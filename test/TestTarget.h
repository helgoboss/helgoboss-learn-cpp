#pragma once

#include <helgoboss-learn/Target.h>

namespace helgoboss {
  class TestTarget : public Target {
  public:
    int hitCount = 0;
    double lastHitValue = -1.0;

    TargetCharacter getCharacter() const override {
      return TargetCharacter::Continuous;
    }
    double getStepSize() const override {
      return -1;
    }
    bool wantsToBeHitWithStepCounts() const override {
      return false;
    }
    void hit(double normalizedValue, bool isStepCount) override {
      hitCount += 1;
      lastHitValue = normalizedValue;
    }
    double getCurrentValue() const override {
      return 0.5;
    }
    int getMaxStepCount() const override {
      return -1;
    }
    bool canBeDiscrete() const override {
      return false;
    }
  };
}