#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace helgoboss {
  enum class SourceCharacter {
    Range,
    Switch,
    Encoder1,
    Encoder2,
    Encoder3
  };

  void to_json(nlohmann::json& j, const SourceCharacter& o);
  void from_json(const nlohmann::json& j, SourceCharacter& o);

  namespace util {
    std::string serializeSourceCharacter(SourceCharacter sourceCharacter);
    SourceCharacter deserializeSourceCharacter(const std::string& text);
  }
}