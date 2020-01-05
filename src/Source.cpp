#include <helgoboss-learn/Source.h>

namespace helgoboss {
  void to_json(nlohmann::json& j, const Source& o) {
    o.serializeToJson(j, true);
  }
  void from_json(const nlohmann::json& j, Source& o) {
    o.updateFromJson(j);
  }
}