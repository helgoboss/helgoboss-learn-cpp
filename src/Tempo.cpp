#include <helgoboss-learn/Tempo.h>
#include <algorithm>
#include <helgoboss-learn/string-util.h>
#include <helgoboss-learn/math-util.h>

namespace helgoboss {
  Tempo Tempo::ofNormalizedValue(double normalizedValue) {
    return Tempo(util::mapNormalizedValueToValueInRange(normalizedValue, 1, 960));
  }

  Tempo::Tempo(double bpm) {
    bpm_ = std::max(1.0, std::min(960.0, bpm));
  }

  double Tempo::normalizedValue() const {
    return util::mapValueInRangeToNormalizedValue(bpm_, 1, 960);
  }

  double Tempo::bpm() const {
    return bpm_;
  }

  std::string Tempo::toString() const {
    return toStringWithoutUnit() + " bpm";
  }

  std::string Tempo::toStringWithoutUnit() const {
    // @closureIsSafe
    return util::toString(10, [this](char* buffer, int maxSize) {
      sprintf(buffer, "%.4f", bpm_);
    });
  }
}
