#include <helgoboss-learn/ModeType.h>

namespace helgoboss::util {
  std::string getModeTypeListEntryLabel(ModeType modeType) {
    switch (modeType) {
      case ModeType::Absolute:
        return "Absolute";
      case ModeType::Relative:
        return "Relative";
      case ModeType::Toggle:
        return "Toggle";
      default:
        return "";
    }
  }
}