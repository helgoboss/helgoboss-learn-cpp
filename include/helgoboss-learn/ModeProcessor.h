#pragma once

#include <string>
#include <memory>
#include <cmath>
#include <boost/algorithm/string.hpp>
#include <eel2/ns-eel.h>
#include "math-util.h"
#include "ModeType.h"
#include "SourceProcessor.h"

namespace helgoboss {
  namespace internal {
    constexpr double DEFAULT_MIN_TARGET_VALUE = 0.0;
    constexpr double DEFAULT_MAX_TARGET_VALUE = 1.0;
    constexpr double DEFAULT_MIN_STEP_SIZE = 0.01;
    constexpr double DEFAULT_MAX_STEP_SIZE = 0.01;
    constexpr bool DEFAULT_REVERSE_IS_ENABLED = false;
    constexpr bool DEFAULT_ROTATE_IS_ENABLED = false;
    constexpr double DEFAULT_MIN_SOURCE_VALUE = 0.0;
    constexpr double DEFAULT_MAX_SOURCE_VALUE = 1.0;
    constexpr double DEFAULT_MIN_TARGET_JUMP = 0.0;
    constexpr double DEFAULT_MAX_TARGET_JUMP = 1.0;
    const std::string DEFAULT_EEL_CONTROL_TRANSFORMATION = std::string();
    constexpr bool DEFAULT_IGNORE_OUT_OF_RANGE_SOURCE_VALUES_IS_ENABLED = false;
    constexpr bool DEFAULT_ROUND_TARGET_VALUE = false;
    constexpr bool DEFAULT_SCALE_MODE_ENABLED = false;

    double alignToStepSize(double value, double stepSize);
  }
  class ModeProcessor {
  private:
    ModeType type_{ModeType::Absolute};
    double minTargetValue_{internal::DEFAULT_MIN_TARGET_VALUE};
    double maxTargetValue_{internal::DEFAULT_MAX_TARGET_VALUE};
    double minSourceValue_{internal::DEFAULT_MIN_SOURCE_VALUE};
    double maxSourceValue_{internal::DEFAULT_MAX_SOURCE_VALUE};
    bool reverseIsEnabled_{internal::DEFAULT_REVERSE_IS_ENABLED};
    bool ignoreOutOfRangeSourceValuesIsEnabled_{internal::DEFAULT_IGNORE_OUT_OF_RANGE_SOURCE_VALUES_IS_ENABLED};
    double minTargetJump_{internal::DEFAULT_MIN_TARGET_JUMP};
    double maxTargetJump_{internal::DEFAULT_MAX_TARGET_JUMP};
    bool roundTargetValue_{internal::DEFAULT_ROUND_TARGET_VALUE};
    bool scaleModeEnabled_{internal::DEFAULT_SCALE_MODE_ENABLED};
    double minStepSize_{internal::DEFAULT_MIN_STEP_SIZE};
    double maxStepSize_{internal::DEFAULT_MAX_STEP_SIZE};
    bool rotateIsEnabled_{internal::DEFAULT_ROTATE_IS_ENABLED};
    std::unique_ptr<void, decltype(&NSEEL_VM_free)> controlVm_{NSEEL_VM_alloc(), NSEEL_VM_free};
    // Only needed for copying
    std::string eelControlTransformation_{internal::DEFAULT_EEL_CONTROL_TRANSFORMATION};
    std::unique_ptr<void, decltype(&NSEEL_code_free)> controlCodeHandle_{nullptr, NSEEL_code_free};
    // Will be deleted together with VM
    double* controlVariableX_{NSEEL_VM_regvar(controlVm_.get(), "x")};
    double* controlVariableY_{NSEEL_VM_regvar(controlVm_.get(), "y")};
  public:
    ModeProcessor() {
      initialize();
    };

    ModeProcessor(
        ModeType type,
        double minTargetValue,
        double maxTargetValue,
        double minSourceValue,
        double maxSourceValue,
        bool reverseIsEnabled,
        bool ignoreOutOfRangeSourceValuesIsEnabled,
        double minTargetJump,
        double maxTargetJump,
        const std::string& eelControlTransformation,
        bool roundTargetValue,
        bool scaleModeEnabled,
        double minStepSize,
        double maxStepSize,
        bool rotateIsEnabled
    ) : type_(type),
        minTargetValue_(minTargetValue),
        maxTargetValue_(maxTargetValue),
        minSourceValue_(minSourceValue),
        maxSourceValue_(maxSourceValue),
        reverseIsEnabled_(reverseIsEnabled),
        ignoreOutOfRangeSourceValuesIsEnabled_(ignoreOutOfRangeSourceValuesIsEnabled),
        minTargetJump_(minTargetJump),
        maxTargetJump_(maxTargetJump),
        roundTargetValue_(roundTargetValue),
        scaleModeEnabled_(scaleModeEnabled),
        minStepSize_(minStepSize),
        maxStepSize_(maxStepSize),
        rotateIsEnabled_(rotateIsEnabled),
        eelControlTransformation_(boost::trim_copy(eelControlTransformation)) {
      initialize();
    }

    ModeProcessor(const ModeProcessor& other) :
        type_(other.type_),
        minTargetValue_(other.minTargetValue_),
        maxTargetValue_(other.maxTargetValue_),
        minSourceValue_(other.minSourceValue_),
        maxSourceValue_(other.maxSourceValue_),
        reverseIsEnabled_(other.reverseIsEnabled_),
        ignoreOutOfRangeSourceValuesIsEnabled_(other.ignoreOutOfRangeSourceValuesIsEnabled_),
        minTargetJump_(other.minTargetJump_),
        maxTargetJump_(other.maxTargetJump_),
        roundTargetValue_(other.roundTargetValue_),
        scaleModeEnabled_(other.scaleModeEnabled_),
        minStepSize_(other.minStepSize_),
        maxStepSize_(other.maxStepSize_),
        rotateIsEnabled_(other.rotateIsEnabled_),
        eelControlTransformation_(other.eelControlTransformation_) {
      initialize();
    }
    // TODO Right now move constructor will invoke copy constructor. Maybe optimize later.
    ModeProcessor& operator=(ModeProcessor&& other) noexcept = default;
    // Right now not needed
    ModeProcessor& operator=(const ModeProcessor& other) = delete;

    template<typename Target>
    void processSourceValue(double normalizedSourceValue, const SourceProcessor& sourceProcessor, Target& target) {
      switch (type_) {
        case ModeType::Absolute:
          processSourceValueInAbsoluteMode(normalizedSourceValue, sourceProcessor, target);
          break;
        case ModeType::Relative:
          processSourceValueInRelativeMode(normalizedSourceValue, sourceProcessor, target);
          break;
        case ModeType::Toggle:
          processSourceValueInToggleMode(normalizedSourceValue, sourceProcessor, target);
          break;
        default:
          break;
      }
    }

  private:
    template<typename Target>
    void processSourceValueInRelativeMode(
        double normalizedSourceValue, const SourceProcessor& sourceProcessor, Target& target) {
      if (sourceProcessor.emitsStepCounts()) {
        // Classic relative mode
        const auto stepCount = static_cast<int>(normalizedSourceValue);
        if (stepCount != 0) {
          if (target.wantsToBeHitWithStepCounts()) {
            // Target wants step counts
            const int peppedUpStepCount = pepUpStepCount(stepCount, sourceProcessor, target);
            if (peppedUpStepCount != 0) {
              target.hit(peppedUpStepCount, true);
            }
          } else {
            // Target wants absolute values
            const double targetStepSize = target.getStepSize();
            if (targetStepSize == -1) {
              // Continuous target
              const double stepSize = util::mapValueInRangeToValueInRange(
                  stepCount,
                  getMinSourceStepCount(sourceProcessor),
                  getMaxSourceStepCount(sourceProcessor),
                  minStepSize_,
                  maxStepSize_
              );
              hitTargetAbsolutelyWithStepSize(target, getReverseFactor() * stepSize);
            } else {
              // Discrete target
              const int peppedUpStepCount = pepUpStepCount(stepCount, sourceProcessor, target);
              hitTargetAbsolutelyWithStepSize(target, peppedUpStepCount * targetStepSize);
            }
          }
        }
      } else if (normalizedSourceValue > 0 && normalizedSourceValue >= minSourceValue_
          && normalizedSourceValue <= maxSourceValue_) {
        // Relative one direction mode
        if (target.wantsToBeHitWithStepCounts()) {
          // Target wants step counts
          const double intermediateStepCount = util::mapValueInRangeToValueInRange(
              normalizedSourceValue,
              minSourceValue_,
              maxSourceValue_,
              getMinStepCount(target),
              getMaxStepCount(target)
          );
          const double peppedUpStepCount = getReverseFactor() * static_cast<int>(std::round(intermediateStepCount));
          if (peppedUpStepCount != 0) {
            target.hit(peppedUpStepCount, true);
          }
        } else {
          // Target wants absolute values
          const double targetStepSize = target.getStepSize();
          if (targetStepSize == -1) {
            // Continuous target
            const double stepSize = util::mapValueInRangeToValueInRange(
                normalizedSourceValue,
                minSourceValue_,
                maxSourceValue_,
                minStepSize_,
                maxStepSize_
            );
            hitTargetAbsolutelyWithStepSize(target, getReverseFactor() * stepSize);
          } else {
            // Discrete target
            const double alignedMinStepSize = internal::alignToStepSize(minStepSize_, targetStepSize);
            const double alignedMaxStepSize = internal::alignToStepSize(maxStepSize_, targetStepSize);
            const double mappedStepSize = util::mapValueInRangeToValueInRange(
                normalizedSourceValue,
                minSourceValue_,
                maxSourceValue_,
                alignedMinStepSize,
                alignedMaxStepSize
            );
            const double alignedMappedStepSize = internal::alignToStepSize(mappedStepSize, targetStepSize);
            hitTargetAbsolutelyWithStepSize(target, getReverseFactor() * alignedMappedStepSize);
          }
        }
      }
    }
    template<typename Target>
    void processSourceValueInAbsoluteMode(
        double normalizedSourceValue, const SourceProcessor& sourceProcessor, Target& target) {
      if (normalizedSourceValue >= minSourceValue_ && normalizedSourceValue <= maxSourceValue_) {
        const double mappedSourceValue =
            util::mapValueInRangeToNormalizedValue(normalizedSourceValue, minSourceValue_, maxSourceValue_);
        const double transformedSourceValue = transformControlValue(mappedSourceValue);
        const double tmpValue =
            util::mapNormalizedValueToValueInRange(transformedSourceValue, minTargetValue_, maxTargetValue_);
        const double absoluteValue = reverseIsEnabled_ ? 1 - tmpValue : tmpValue;
        const double potentiallyDiscreteValue = roundValueIfNecessary(absoluteValue, target);
        hitTargetAbsolutelyConsideringMaxJump(target, potentiallyDiscreteValue);
      } else if (!ignoreOutOfRangeSourceValuesIsEnabled_) {
        if (normalizedSourceValue < minSourceValue_) {
          hitTargetAbsolutelyConsideringMaxJump(target, minTargetValue_);
        } else {
          hitTargetAbsolutelyConsideringMaxJump(target, maxTargetValue_);
        }
      }
    }
    template<typename Target>
    void processSourceValueInToggleMode(
        double normalizedSourceValue, const SourceProcessor& sourceProcessor, Target& target) {
      if (normalizedSourceValue > 0.0) {
        const double centerTargetValue = (minTargetValue_ + maxTargetValue_) / 2;
        const double absoluteValue = target.getCurrentValue() > centerTargetValue ? minTargetValue_ : maxTargetValue_;
        target.hit(absoluteValue, false);
      }
    }
    template<typename Target>
    int pepUpStepCount(int stepCount, const SourceProcessor& sourceProcessor, const Target& target) const {
      if (stepCount == 0) {
        return 0;
      }
      const auto intermediateStepCount = util::mapValueInRangeToValueInRange(
          stepCount,
          getMinSourceStepCount(sourceProcessor),
          getMaxSourceStepCount(sourceProcessor),
          getMinStepCount(target),
          getMaxStepCount(target)
      );
      return getReverseFactor() * static_cast<int>(std::round(intermediateStepCount));
    }
    int getReverseFactor() const {
      return reverseIsEnabled_ ? -1 : 1;
    }
    template<typename Target>
    int getMinStepCount(const Target& target) const {
      return convertStepSizeToStepCount(minStepSize_, target);
    }
    template<typename Target>
    int getMaxStepCount(const Target& target) const {
      return convertStepSizeToStepCount(maxStepSize_, target);
    }
    int getMinSourceStepCount(const SourceProcessor& sourceProcessor) const {
      return convertSourceValueToStepCount(minSourceValue_, sourceProcessor);
    }
    int getMaxSourceStepCount(const SourceProcessor& sourceProcessor) const {
      return convertSourceValueToStepCount(maxSourceValue_, sourceProcessor);
    }
    int convertSourceValueToStepCount(double normalizedValue, const SourceProcessor& sourceProcessor) const {
      return static_cast<int>(
          std::round(util::mapNormalizedValueToValueInRange(normalizedValue, 0, sourceProcessor.getMaxStepCount()))
      );
    }
    template<typename Target>
    int convertStepSizeToStepCount(double normalizedValue, const Target& target) const {
      return static_cast<int>(
          std::round(util::mapNormalizedValueToValueInRange(normalizedValue, 0, target.getMaxStepCount()))
      );
    }
    template<typename Target>
    void hitTargetAbsolutelyWithStepSize(Target& target, double stepSize) {
      if (stepSize != 0) {
        const double absoluteTargetValue = target.getCurrentValue() + stepSize;
        const double peppedUpAbsoluteTargetValue = pepUpAbsoluteTargetValue(absoluteTargetValue, target);
        target.hit(peppedUpAbsoluteTargetValue, false);
      }
    }
    template<typename Target>
    double pepUpAbsoluteTargetValue(double absoluteTargetValue, Target& target) const {
      const double alignedValue = internal::alignToStepSize(absoluteTargetValue, target.getStepSize());
      double tmpResult = alignedValue;
      if (rotateIsEnabled_) {
        if (tmpResult < minTargetValue_) {
          tmpResult = maxTargetValue_;
        } else if (tmpResult > maxTargetValue_) {
          tmpResult = minTargetValue_;
        }
      }
      return std::max(minTargetValue_, std::min(maxTargetValue_, tmpResult));
    }
    double transformControlValue(double normalizedValue) const {
      if (controlCodeHandle_ == nullptr) {
        return normalizedValue;
      }
      *controlVariableX_ = normalizedValue;
      *controlVariableY_ = normalizedValue;
      NSEEL_code_execute(controlCodeHandle_.get());
      return *controlVariableY_;
    }
    template<typename Target>
    double roundValueIfNecessary(double absoluteValue, const Target& target) {
      if (roundTargetValue_ && target.canBeDiscrete()) {
        const int targetValueSpan = static_cast<int>(1 / target.getStepSize());
        return std::round(absoluteValue * targetValueSpan) / targetValueSpan;
      } else {
        return absoluteValue;
      }
    }
    template<typename Target>
    void hitTargetAbsolutelyConsideringMaxJump(Target& target, double absoluteValue) {
      const double currentTargetValue = target.getCurrentValue();
      const double jump = std::abs(absoluteValue - currentTargetValue);
      if (jump <= maxTargetJump_) {
        if (jump >= minTargetJump_) {
          target.hit(absoluteValue, false);
        }
      } else if (scaleModeEnabled_) {
        const double approachJump = util::mapNormalizedValueToValueInRange(jump, minTargetJump_, maxTargetJump_);
        if (absoluteValue < currentTargetValue) {
          target.hit(currentTargetValue - approachJump, false);
        } else {
          target.hit(currentTargetValue + approachJump, false);
        }
      }
    }

    void initialize() {
      initEelTransformation();
    }

    void initEelTransformation() {
      if (!eelControlTransformation_.empty()) {
        NSEEL_CODEHANDLE raw = NSEEL_code_compile(controlVm_.get(), eelControlTransformation_.c_str(), 0);
        controlCodeHandle_.reset(raw);
      }
    }
  };
}