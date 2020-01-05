#pragma once

#include <string>

namespace helgoboss {
  constexpr int NUM_MODE_TYPES = 3;

  enum class ModeType {
    Absolute,
    Relative,
    Toggle
  };

  namespace util {
    std::string getModeTypeListEntryLabel(ModeType modeType);
  }
}