#pragma once

#include "rxcpp/rx.hpp"

namespace helgoboss::util {
  /**
   * Waits for requestedItemCount more items within the given time frame as soon as an item has arrived.
   */
  template<typename Item, typename Coordination>
  std::function<rxcpp::subscriber<Item>(rxcpp::subscriber<std::vector<Item>>)> waitForSomeMoreItems(
      int requestedItemCount,
      rxcpp::schedulers::scheduler::clock_type::duration maxWaitingTime,
      Coordination coordination
  ) {
    // Restrict template parameter Coordination
    static_assert(rxcpp::is_coordination<Coordination>::value,
        "Coordination parameter must satisfy the requirements for a Coordination");

    // Define class that holds the behavior for this operator
    class Operator {
    public:
      // Define class that holds the state for this operator
      struct OperatorState {
      public:
        std::vector<Item> bufferedItems;
        rxcpp::composite_subscription bufferSubmissionSubscription;
        rxcpp::schedulers::worker worker;
        typename Coordination::coordinator_type coordinator;
        rxcpp::composite_subscription cs;
        int requestedItemCount;
        rxcpp::subscriber<std::vector<Item>> destination;
        rxcpp::schedulers::scheduler::clock_type::duration maxWaitingTime;
      public:
        OperatorState(
            rxcpp::composite_subscription cs,
            typename Coordination::coordinator_type coordinator,
            rxcpp::subscriber<std::vector<Item>> destination,
            int requestedItemCount,
            rxcpp::schedulers::scheduler::clock_type::duration maxWaitingTime) :
            worker(coordinator.get_worker()),
            coordinator(std::move(coordinator)),
            cs(std::move(cs)),
            requestedItemCount(requestedItemCount),
            destination(std::move(destination)),
            maxWaitingTime(maxWaitingTime) {
        }

        void dispose() {
          unscheduleBufferSubmission();
          cs.unsubscribe();
          destination.unsubscribe();
          stopWorker();
        }

        void unscheduleBufferSubmission() {
          bufferSubmissionSubscription.unsubscribe();
        }

        void stopWorker() {
          worker.unsubscribe();
        }
      };

    private:
      std::shared_ptr<OperatorState> state_;
    public:
      Operator(std::shared_ptr<OperatorState> state) : state_(state) {
        // Save state into local variable so we don't depend on "this" in the disposer lambda.
        // "this" can be already gone at the time of disposal.
        auto localState = state_;
        // Stop scheduling as soon as unsubscribed
        auto disposer = [localState]() {
          localState->dispose();
        };
        localState->destination.add(disposer);
        localState->cs.add(disposer);
      }

      void onNext(Item item) {
        auto localState = state_;
        // Buffer item
        localState->bufferedItems.emplace_back(item);
        if (localState->bufferedItems.size() == 1) {
          // This was the first item in a row
          scheduleBufferSubmission(localState);
        } else if (localState->bufferedItems.size() == localState->requestedItemCount) {
          // Requested item count reached
          localState->unscheduleBufferSubmission();
          submitBuffer(localState);
        }
      }

    private:
      static void submitBuffer(const std::shared_ptr<OperatorState>& localState) {
        localState->destination.on_next(std::move(localState->bufferedItems));
        localState->bufferedItems = std::vector<Item>();
      }

      static void scheduleBufferSubmission(std::shared_ptr<OperatorState>& localState) {
        auto executionTime = localState->worker.now() + localState->maxWaitingTime;
        localState->bufferSubmissionSubscription = rxcpp::composite_subscription();
        localState->worker.schedule(
            executionTime,
            make_schedulable(
                localState->worker,
                localState->bufferSubmissionSubscription,
                [localState](const rxcpp::schedulers::schedulable&) {
                  submitBuffer(localState);
                }
            )
        );
      }
    };

    // Return function that is capable of creating the operator
    return [requestedItemCount, maxWaitingTime, coordination](rxcpp::subscriber<std::vector<Item>> destination) {
      // Create operator state
      auto cs = rxcpp::composite_subscription();
      auto state = std::make_shared<typename Operator::OperatorState>(
          cs, coordination.create_coordinator(), destination, requestedItemCount, maxWaitingTime);
      auto op = std::make_shared<Operator>(state);

      // Return operator
      return rxcpp::make_subscriber<Item>(cs, [op](Item item) { op->onNext(item); })
          .as_dynamic(); // VS2013 deduction issue requires dynamic (type-forgetting)
    };
  }
}

