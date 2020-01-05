#include <helgoboss-learn/Mode.h>

void NSEEL_HOSTSTUB_EnterMutex() {}
void NSEEL_HOSTSTUB_LeaveMutex() {}

namespace helgoboss::internal {
  void ensureThatMinAlwaysLowerThanMax(ReactiveProperty<double>& minProp, ReactiveProperty<double>& maxProp) {
    minProp.changedToValue().subscribe([&maxProp](double v) {
      if (maxProp.get() < v) {
        maxProp.set(v);
      }
    });
    maxProp.changedToValue().subscribe([&minProp](double v) {
      if (minProp.get() > v) {
        minProp.set(v);
      }
    });
  }

  std::function<double(double)> keepInRange(double min, double max) {
    return [min, max](double v) {
      return std::min(max, std::max(min, v));
    };
  }
}