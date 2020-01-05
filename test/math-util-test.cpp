#include <catch.hpp>
#include <helgoboss-learn/math-util.h>
#include <vector>

using helgoboss::util::mapNormalizedValueToValueInRange;
using helgoboss::util::mapValueInRangeToNormalizedValue;
using helgoboss::util::mapValueInRangeToValueInRange;
using std::vector;

namespace helgoboss {
  struct TestRecord {
    double inRangeValue;
    double normalizedValue;
  };

  struct TestRange {
    double first;
    double second;
    std::vector<TestRecord> innerRecords;

    bool isSwapped() const {
      return first > second;
    }
    double getMin() const {
      return std::min(first, second);
    }
    double getMax() const {
      return std::max(first, second);
    }
    bool isCompletelyPositive() const {
      return first >= 0 && second >= 0;
    }
  };

  void testNormalizeValuesBetweenMinAndMax(const TestRange& range, bool negated) {
    int numTestedRecords = 0;
    for (const TestRecord& rec : range.innerRecords) {
      numTestedRecords += 1;
      if (negated) {
        REQUIRE(mapValueInRangeToNormalizedValue(-rec.inRangeValue, range.getMin(), range.getMax())
            == -rec.normalizedValue);
      } else {
        REQUIRE(
            mapValueInRangeToNormalizedValue(rec.inRangeValue, range.getMin(), range.getMax()) == rec.normalizedValue);
      }
    }
    REQUIRE(numTestedRecords > 0);
  }

  void testDenormalizeValuesBetweenZeroAndOne(const TestRange& range, bool negated) {
    int numTestedRecords = 0;
    for (const TestRecord& rec : range.innerRecords) {
      numTestedRecords += 1;
      if (negated) {
        REQUIRE(mapNormalizedValueToValueInRange(-rec.normalizedValue, range.getMin(), range.getMax())
            == -rec.inRangeValue);
      } else {
        REQUIRE(
            mapNormalizedValueToValueInRange(rec.normalizedValue, range.getMin(), range.getMax()) == rec.inRangeValue);
      }
    }
    REQUIRE(numTestedRecords > 0);
  }

  void testInternal(const TestRange& range) {
    GIVEN("Range [" + std::to_string(range.getMin()) + ", " + std::to_string(range.getMax()) + "]"
              + (range.isSwapped() ? " swapped" : "")) {
      WHEN("normalizing values between min and max") {
        THEN("normalizes to between 0 and 1") {
          testNormalizeValuesBetweenMinAndMax(range, false);
        }
      }
      WHEN("denormalizing values between 0 and 1") {
        THEN("denormalizes to between min and max") {
          testDenormalizeValuesBetweenZeroAndOne(range, false);
        }
      }
      WHEN("normalizing min and max") {
        THEN("normalizes to 0 or 1") {
          REQUIRE(mapValueInRangeToNormalizedValue(range.getMin(), range.getMin(), range.getMax()) == 0);
          REQUIRE(mapValueInRangeToNormalizedValue(range.getMax(), range.getMin(), range.getMax()) == 1);
        }
      }
      WHEN("denormalizing 0 and 1") {
        THEN("denormalizes to min or max") {
          REQUIRE(mapNormalizedValueToValueInRange(0, range.getMin(), range.getMax()) == range.getMin());
          REQUIRE(mapNormalizedValueToValueInRange(1, range.getMin(), range.getMax()) == range.getMax());
        }
      }
      WHEN("normalizing out-of-range values") {
        THEN("normalizes to 0 or 1") {
          if (!range.isCompletelyPositive() || range.getMin() - 0.5 >= 0) {
            REQUIRE(mapValueInRangeToNormalizedValue(range.getMin() - 0.5, range.getMin(), range.getMax()) == 0);
          }
          REQUIRE(mapValueInRangeToNormalizedValue(range.getMax() + 0.5, range.getMin(), range.getMax()) == 1);
        }
      }
      WHEN("denormalizing > 1") {
        THEN("denormalizes to max") {
          REQUIRE(mapNormalizedValueToValueInRange(1.2, range.getMin(), range.getMax()) == range.getMax());
        }
      }
      if (range.isCompletelyPositive()) {
        WHEN("normalizing values between -max and -min") {
          THEN("normalizes to between -1 and 0") {
            testNormalizeValuesBetweenMinAndMax(range, true);
          }
        }
        WHEN("denormalizing values between -1 and 0") {
          THEN("denormalizes to between -max and -min") {
            testDenormalizeValuesBetweenZeroAndOne(range, true);
          }
        }
        WHEN("normalizing -max and -min") {
          THEN("normalizes to -1 or 0") {
            REQUIRE(mapValueInRangeToNormalizedValue(-range.getMax(), range.getMin(), range.getMax()) == -1);
            REQUIRE(mapValueInRangeToNormalizedValue(-range.getMin(), range.getMin(), range.getMax()) == 0);
          }
        }
        WHEN("denormalizing -1") {
          THEN("denormalizes to -max") {
            REQUIRE(mapNormalizedValueToValueInRange(-1, range.getMin(), range.getMax()) == -range.getMax());
          }
        }
        WHEN("normalizing negated out-of-range values") {
          THEN("normalizes to -1 or 0") {
            REQUIRE(mapValueInRangeToNormalizedValue(-range.getMax() - 0.5, range.getMin(), range.getMax()) == -1);
            if (-range.getMin() + 0.5 < 0) {
              REQUIRE(mapValueInRangeToNormalizedValue(-range.getMin() + 0.5, range.getMin(), range.getMax()) == 0);
            }
          }
        }
        WHEN("denormalizing < -1") {
          THEN("denormalizes to -max") {
            REQUIRE(mapNormalizedValueToValueInRange(-1.2, range.getMin(), range.getMax()) == -range.getMax());
          }
        }
      }
    }
  }

  void test(const TestRange& range) {
    testInternal(range);
    TestRange swappedRange = range;
    swappedRange.first = range.second;
    swappedRange.second = range.first;
    testInternal(swappedRange);
  }

  SCENARIO("Mapping methods") {
    test({
        0, 1,
        {
            {+0.5, +0.5},
        }
    });
    test({
        -1, 0,
        {
            {-0.5, 0.5},
        }
    });
    test({
        1, 2,
        {
            {+1.5, +0.5},
        }
    });
    test({
        -2, -1,
        {
            {-1.5, 0.5},
        }
    });
    test({
        -1, 1,
        {
            {-0.5, 0.25},
            {+0.0, 0.50},
            {+0.5, 0.75},
        }
    });
    test({
        -1, +3,
        {
            {+0.0, 0.25},
            {+1.0, 0.50},
            {+2.0, 0.75},
        }
    });
  }
}