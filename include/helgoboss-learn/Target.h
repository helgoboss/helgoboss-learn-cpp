#pragma once

#include "TargetCharacter.h"

namespace helgoboss {
  class Target {
  public:

    virtual TargetCharacter getCharacter() const = 0;

    // Returns -1 if no minimum step size. Usually if the target character is not discrete. But some targets are
    // continuous in nature but it still makes sense to have discrete steps, for example tempo in bpm
    // - see canBeDiscrete()).
    // This value should be something from 0 to 1. Although 1 doesn't really make sense because that would mean the
    // step size covers the whole range, so the target has just one possible value.
    virtual double getStepSize() const = 0;

    virtual bool wantsToBeHitWithStepCounts() const = 0;

    virtual void hit(double normalizedValue, bool isStepCount) = 0;

    // Returns normalized value
    virtual double getCurrentValue() const = 0;

    // Should be a rather high value like e.g. 63 (meaning one target has 63 different discrete values)
    virtual int getMaxStepCount() const = 0;

    virtual bool canBeDiscrete() const = 0;
  };
}