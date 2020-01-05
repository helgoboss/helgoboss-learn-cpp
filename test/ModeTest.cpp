#include <catch.hpp>
#include <helgoboss-learn/Source.h>
#include <helgoboss-learn/SourceContext.h>
#include <helgoboss-learn/Mode.h>
#include <helgoboss-learn/Target.h>
#include "TestSourceContext.h"
#include "TestTarget.h"

namespace helgoboss {
  SCENARIO("Modes") {
    GIVEN("A mode") {
      Mode mode;
      int changeCount = 0;
      mode.changed().subscribe([&changeCount](bool) {
        changeCount += 1;
      });
      auto& modeProcessor = mode.getProcessor();
      WHEN("used") {
        TestSourceContext context;
        Source source;
        TestTarget target;
        mode.type.set(ModeType::Absolute);
        mode.minTargetValue.set(0.2);
        mode.maxTargetValue.set(0.8);
        mode.reverseIsEnabled.set(false);
        mode.eelControlTransformation.set("y = 1 - x");
        modeProcessor.processSourceValue(0.0, source.getProcessor(), target);
        mode.feedback(source, target, context);
        THEN("it should work") {
          REQUIRE(mode.supportsReverseIsEnabled());
          REQUIRE(mode.supportsIgnoreOutOfRangeSourceValuesIsEnabled());
          REQUIRE(mode.supportsTargetJump());
          REQUIRE(mode.supportsEelControlTransformation());
          REQUIRE(mode.supportsEelFeedbackTransformation());
          REQUIRE(mode.supportsEelFeedbackTransformation());
          REQUIRE(mode.supportsRoundTargetValue());
          REQUIRE(mode.supportsScaleModeEnabled());
          REQUIRE(!mode.supportsStepSize());
          REQUIRE(!mode.supportsRotateIsEnabled());
          REQUIRE(changeCount == 3);
          REQUIRE(target.hitCount == 1);
          REQUIRE(target.lastHitValue == 0.8);
          REQUIRE(mode.settingsMakeSense(source, target));
        }
      }
    }
  }
}