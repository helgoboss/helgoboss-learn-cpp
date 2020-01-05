#pragma once

#include "rxcpp/rx.hpp"

namespace helgoboss::util {
  /**
   * Lets the first item through and after that only items which are similar to the first item.
   *
   * The similarity is decided by the passed function.
   */
  template<typename Item>
  std::function<rxcpp::subscriber<Item>(rxcpp::subscriber<Item>)> limitToItemsLikeFirstOne(
      std::function<bool(Item itemOne, Item itemTwo)> isSimilar) {
    return [isSimilar](rxcpp::subscriber<Item> dest) {
      std::shared_ptr<std::unique_ptr<Item>> firstItem = std::make_shared<std::unique_ptr<Item>>();
      return rxcpp::make_subscriber<Item>(
          [dest, firstItem, isSimilar](Item item) {
            if (*firstItem == nullptr) {
              *firstItem = std::unique_ptr<Item>(new Item(item));
              dest.on_next(item);
            } else {
              if (isSimilar(item, **firstItem)) {
                dest.on_next(item);
              }
            }
          }
      ).as_dynamic(); // VS2013 deduction issue requires dynamic (type-forgetting)
    };
  }
}