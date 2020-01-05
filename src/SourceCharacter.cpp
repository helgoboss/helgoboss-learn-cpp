#include <helgoboss-learn/SourceCharacter.h>
#include <gsl/gsl>

namespace helgoboss {

  namespace util {
    std::string serializeSourceCharacter(SourceCharacter sourceCharacter) {
      switch (sourceCharacter) {
        case SourceCharacter::Range:
          return "Range";
        case SourceCharacter::Switch:
          return "Switch";
        case SourceCharacter::Encoder1:
          return "Encoder1";
        case SourceCharacter::Encoder2:
          return "Encoder2";
        case SourceCharacter::Encoder3:
          return "Encoder3";
        default:
          Expects(false);
      }
    }

    SourceCharacter deserializeSourceCharacter(const std::string& text) {
      if (text == "Range") {
        return SourceCharacter::Range;
      }
      if (text == "Switch") {
        return SourceCharacter::Switch;
      }
      if (text == "Encoder1") {
        return SourceCharacter::Encoder1;
      }
      if (text == "Encoder2") {
        return SourceCharacter::Encoder2;
      }
      if (text == "Encoder3") {
        return SourceCharacter::Encoder3;
      }
      Expects(false);
    }
  }

  void to_json(nlohmann::json& j, const SourceCharacter& o) {
    j = util::serializeSourceCharacter(o);
  }

  void from_json(const nlohmann::json& j, SourceCharacter& o) {
    o = util::deserializeSourceCharacter(j);
  }
}