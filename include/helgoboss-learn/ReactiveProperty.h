#pragma once

#include <rxcpp/rx.hpp>
#include <functional>

namespace helgoboss {
  /**
   * A reactive property which has the following characteristics:
   *
   * - It can be initialized with a transformer, which is a lambda that transforms the value passed to the setter
   *   before actually setting the value. It's good for restricting the value range of that property. It's not
   *   good for maintaining object-wide invariants because transformers which enclose over surrounding state are
   *   not advisable (see the constructor which takes a transformer).
   * - It's copyable (copying it copies the value and the transformer, not the change listeners)
   * - It's movable (values, transformers and change listeners are moved)
   * - Equality operators are based just on the value, not on transformers and listeners
   */
  template<typename T>
  class ReactiveProperty {
  private:
    rxcpp::subjects::behavior<T> behavior_;
    std::function<T(T)> transformer_;

  public:
    /**
     * Creates the property with an initial value and without a special transformer.
     */
    explicit ReactiveProperty(T initialValue)
        : ReactiveProperty(std::move(initialValue), [](T v) { return v; }) {
    }
    /**
     * Creates the property with an initial value and a custom transformer. The transformer is not applied to
     * the initial value.
     * The transformer lambda is copied when this property is copied so be aware of any pointer/references-type
     * captures! You probably want to be the copy of this property to be something which is completely isolated
     * from the original property. So for example a "this" capture to gain access to other members of the object
     * which contains this property is most likely not a good idea. If you want to do that, you should write
     * the copy constructor (and copy assignment operator) yourself and take care to create new independent
     * ReactiveProperty objects with the same value!
     */
    ReactiveProperty(T initialValue, std::function<T(T)> transformer)
        : behavior_(std::move(initialValue)), transformer_(std::move(transformer)) {
    }

    ReactiveProperty(const ReactiveProperty<T>& other)
        : behavior_(other.get()), transformer_(other.transformer_) {
    }

    ReactiveProperty& operator=(const ReactiveProperty<T>& other) {
      if (this != &other) {
        set(other.get());
        transformer_ = other.transformer_;
      }
      return *this;
    }

    ReactiveProperty(ReactiveProperty<T>&& other) noexcept :
        behavior_(std::move(other.get())), transformer_(std::move(other.transformer_)) {
    }

    ReactiveProperty& operator=(ReactiveProperty<T>&& other) noexcept {
      if (this != &other) {
        set(other.get());
        transformer_ = other.transformer_;
      }
      return *this;
    }

    /**
     * Returns the current value of this property.
     */
    T get() const {
      return behavior_.get_value();
    }

    /**
     * Sets this property to the given value. If a transformer has been defined, the given value might be changed into
     * another one before.
     */
    void set(T value) {
      behavior_.get_subscriber().on_next(transformer_(value));
    }

    /**
     * Fires whenever the value is changed. Event contains the new value.
     */
    rxcpp::observable<T> changedToValue() const {
      return behavior_.get_observable()
          .distinct_until_changed()
          .skip(1);
    }

    /**
     * Fires whenever the value is changed. Event always contains a boolean "true" instead of the new value.
     * This is perfect for combining observables because observables can be combined much easier if they have
     * the same type. UI event handlers for example are often not interested in the new value anyway because they
     * will just call some reusable invalidation code that queries the new value itself.
     */
    rxcpp::observable<bool> changed() const {
      return changedToValue()
          .map([](T value) { return true; });
    }
    friend bool operator==(const ReactiveProperty& lhs, const ReactiveProperty& rhs) {
      return lhs.get() == rhs.get();
    }
    friend bool operator!=(const ReactiveProperty& lhs, const ReactiveProperty& rhs) {
      return !(rhs == lhs);
    }
  };
}