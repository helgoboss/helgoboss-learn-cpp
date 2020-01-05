#pragma once

#include <string>
#include <memory>
#include <functional>

namespace helgoboss::util {
  /**
   * Executes the given fillBuffer function and converts the filled buffer to a string.
   */
  std::string toString(int maxSize, const std::function<void(char*, int)>& fillBuffer);
}