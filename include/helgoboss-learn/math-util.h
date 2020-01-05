#pragma once

#include <string>
#include <functional>

namespace helgoboss::util {
  /**
   * Denormalizes the given value with regard to the given range. This is the reverse function of
   * mapValueInRangeToNormalizedValue for all normalized values in [0, 1].
   *
   * In particular, it maps a
   * - value within [0, 1] to [rangeMin, rangeMax]
   * - value > 1 to rangeMax
   *
   * If rangeMin > 0 and rangeMax > 0, it also maps a
   * - value within [-1, 0) to [-rangeMax, -rangeMin)
   * - value < -1 to -rangeMax
   */
  double mapNormalizedValueToValueInRange(double value, double rangeMin, double rangeMax);

  /**
   * Normalizes the given value with regard to the given range. This is the reverse function of
   * mapNormalizedValueToValueInRange for all in-range values.
   *
   * In particular, it maps a
   * - value within [rangeMin, rangeMax] to [0, 1]
   *
   * If rangeMin < 0 and/or rangeMax < 0, it maps a
   * - value < rangeMin to 0
   * - value > rangeMax to 1
   *
   * If rangeMin > 0 and rangeMax > 0, it maps a
   * - value within [-rangeMax, -rangeMin) to [-1, 0)
   * - negative value < -rangeMax to -1
   * - negative value > -rangeMin to 0
   * - positive value < rangeMin to 0
   * - positive value > rangeMax to 1
   */
  double mapValueInRangeToNormalizedValue(double value, double rangeMin, double rangeMax);

  /**
   * Maps value in [sourceRangeMin, sourceRangeMax] to [targetRangeMin, targetRangeMax] or
   * [-sourceRangeMax, -sourceRangeMin] to [-targetRangeMax, -targetRangeMin].
   */
  double mapValueInRangeToValueInRange(double value, double sourceRangeMin, double sourceRangeMax,
      double targetRangeMin, double targetRangeMax);
}