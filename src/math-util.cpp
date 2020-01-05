#include <helgoboss-learn/math-util.h>
#include <cmath>
#include <algorithm>
#include <gsl/gsl>

using std::string;
using std::function;

namespace helgoboss::util {
  double mapNormalizedValueToValueInRange(double value, double rangeMin, double rangeMax) {
    const double actualRangeMin = std::min(rangeMin, rangeMax);
    const double actualRangeMax = std::max(rangeMin, rangeMax);
    // Graceful error handling: In case that normalized value is not actually normalized (not from -1 to +1), return
    // corresponding range maximum
    if (value < -1) {
      return -actualRangeMax;
    } else if (value > 1) {
      return actualRangeMax;
    }
    // Happy path
    const double targetSpan = actualRangeMax - actualRangeMin;
    return (std::signbit(value) ? -1 : 1) * (actualRangeMin + std::abs(value) * targetSpan);
  }

  double mapValueInRangeToNormalizedValue(double value, double rangeMin, double rangeMax) {
    const double actualRangeMin = std::min(rangeMin, rangeMax);
    const double actualRangeMax = std::max(rangeMin, rangeMax);
    const double sourceSpan = actualRangeMax - actualRangeMin;
    if (sourceSpan == 0.0) {
      return 0.0;
    }
    if (actualRangeMin < 0) {
      const double crampedValue = std::min(std::max(actualRangeMin, value), actualRangeMax);
      const double positiveValue = crampedValue + std::abs(actualRangeMin);
      return positiveValue / sourceSpan;
    } else {
      const double crampedAbsValue = std::min(std::max(actualRangeMin, std::abs(value)), actualRangeMax);
      return (std::signbit(value) ? -1 : 1) * ((crampedAbsValue - actualRangeMin) / sourceSpan);
    }
  }

  double mapValueInRangeToValueInRange(double value, double sourceRangeMin, double sourceRangeMax,
      double targetRangeMin, double targetRangeMax) {
    const double actualSourceRangeMin = std::min(sourceRangeMin, sourceRangeMax);
    const double actualSourceRangeMax = std::max(sourceRangeMin, sourceRangeMax);
    const auto positiveValue = std::abs(value);
    if (positiveValue < actualSourceRangeMin || positiveValue > actualSourceRangeMax) {
      return 0.0;
    }
    const double actualTargetRangeMin = std::max(targetRangeMin, targetRangeMax);
    const double actualTargetRangeMax = std::max(targetRangeMin, targetRangeMax);
    return mapNormalizedValueToValueInRange(
        mapValueInRangeToNormalizedValue(value, actualSourceRangeMin, actualSourceRangeMax),
        actualTargetRangeMin,
        actualTargetRangeMax
    );
  }

}
