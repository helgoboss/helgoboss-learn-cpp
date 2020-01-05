#pragma once

#include <rxcpp/rx.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <eel2/ns-eel.h>
#include "ReactiveProperty.h"
#include "ModeType.h"
#include "Source.h"
#include "ModeProcessor.h"
#include "TargetCharacter.h"
#include <string>
#include <cmath>
#include "math-util.h"
#include <boost/algorithm/string.hpp>

#undef min
#undef max

namespace helgoboss {
  namespace internal {
    const std::string DEFAULT_EEL_FEEDBACK_TRANSFORMATION = std::string();

    void ensureThatMinAlwaysLowerThanMax(ReactiveProperty<double>& minProp, ReactiveProperty<double>& maxProp);
    std::function<double(double)> keepInRange(double min, double max);
  }

  class Mode {
  public:
    ReactiveProperty<ModeType> type{ModeType::Absolute};
    ReactiveProperty<double> minTargetValue{internal::DEFAULT_MIN_TARGET_VALUE, internal::keepInRange(0.0, 1.0)};
    ReactiveProperty<double> maxTargetValue{internal::DEFAULT_MAX_TARGET_VALUE, internal::keepInRange(0.0, 1.0)};
    ReactiveProperty<double> minSourceValue{internal::DEFAULT_MIN_SOURCE_VALUE, internal::keepInRange(0.0, 1.0)};
    ReactiveProperty<double> maxSourceValue{internal::DEFAULT_MAX_SOURCE_VALUE, internal::keepInRange(0.0, 1.0)};
    ReactiveProperty<bool> reverseIsEnabled{internal::DEFAULT_REVERSE_IS_ENABLED};
    ReactiveProperty<bool> ignoreOutOfRangeSourceValuesIsEnabled{
        internal::DEFAULT_IGNORE_OUT_OF_RANGE_SOURCE_VALUES_IS_ENABLED};
    ReactiveProperty<double> minTargetJump{internal::DEFAULT_MIN_TARGET_JUMP, internal::keepInRange(0.0, 1.0)};
    ReactiveProperty<double> maxTargetJump{internal::DEFAULT_MAX_TARGET_JUMP, internal::keepInRange(0.0, 1.0)};
    ReactiveProperty<std::string> eelControlTransformation{internal::DEFAULT_EEL_CONTROL_TRANSFORMATION};
    ReactiveProperty<std::string> eelFeedbackTransformation{internal::DEFAULT_EEL_FEEDBACK_TRANSFORMATION};
    ReactiveProperty<bool> roundTargetValue{internal::DEFAULT_ROUND_TARGET_VALUE};
    ReactiveProperty<bool> scaleModeEnabled{internal::DEFAULT_SCALE_MODE_ENABLED};
    ReactiveProperty<double> minStepSize{internal::DEFAULT_MIN_STEP_SIZE, internal::keepInRange(0.0, 1.0)};
    ReactiveProperty<double> maxStepSize{internal::DEFAULT_MAX_STEP_SIZE, internal::keepInRange(0.0, 1.0)};
    ReactiveProperty<bool> rotateIsEnabled{internal::DEFAULT_ROTATE_IS_ENABLED};
  private:
    ModeProcessor processor_ = createProcessor();
    std::unique_ptr<void, decltype(&NSEEL_VM_free)> feedbackVm_{NSEEL_VM_alloc(), NSEEL_VM_free};
    std::unique_ptr<void, decltype(&NSEEL_code_free)> feedbackCodeHandle_{nullptr, NSEEL_code_free};
    // Will be deleted together with VM
    double* feedbackVariableX_{NSEEL_VM_regvar(feedbackVm_.get(), "x")};
    double* feedbackVariableY_{NSEEL_VM_regvar(feedbackVm_.get(), "y")};

  public:
    Mode() {
      initialize();
    }

    Mode(const Mode& other) :
        type(other.type),
        minTargetValue(other.minTargetValue),
        maxTargetValue(other.maxTargetValue),
        minSourceValue(other.minSourceValue),
        maxSourceValue(other.maxSourceValue),
        reverseIsEnabled(other.reverseIsEnabled),
        ignoreOutOfRangeSourceValuesIsEnabled(other.ignoreOutOfRangeSourceValuesIsEnabled),
        roundTargetValue(other.roundTargetValue),
        scaleModeEnabled(other.scaleModeEnabled),
        minTargetJump(other.minTargetJump),
        maxTargetJump(other.maxTargetJump),
        eelControlTransformation(other.eelControlTransformation),
        eelFeedbackTransformation(other.eelFeedbackTransformation),
        minStepSize(other.minStepSize),
        maxStepSize(other.maxStepSize),
        rotateIsEnabled(other.rotateIsEnabled.get()),
        processor_(createProcessor()) {
      initialize();
    }
    // TODO Right now move constructor will invoke copy constructor. Maybe optimize later.
    // Default move assignment is okay because object and therefore reactive properties stay the same, just not their
    // values.
    Mode& operator=(Mode&& other) noexcept = default;
    // Right now not needed
    Mode& operator=(const Mode& other) = delete;


    //region Property support queries

    bool supportsReverseIsEnabled() const {
      switch (type.get()) {
        case ModeType::Absolute:
        case ModeType::Relative:
          return true;
        default:
          return false;
      }
    }
    bool supportsIgnoreOutOfRangeSourceValuesIsEnabled() const {
      return type.get() == ModeType::Absolute;
    }
    bool supportsTargetJump() const {
      return type.get() == ModeType::Absolute;
    }
    bool supportsEelControlTransformation() const {
      return type.get() == ModeType::Absolute;
    }
    bool supportsEelFeedbackTransformation() const {
      return type.get() == ModeType::Absolute;
    }
    bool supportsRoundTargetValue() const {
      return type.get() == ModeType::Absolute;
    }
    bool supportsScaleModeEnabled() const {
      return type.get() == ModeType::Absolute;
    }
    bool supportsStepSize() const {
      return type.get() == ModeType::Relative;
    }
    bool supportsRotateIsEnabled() const {
      return type.get() == ModeType::Relative;
    }
    //endregion

    //region UI/modification/persistence/management

    template<typename Target>
    void reset(const Source& source, const Target& target) {
      minSourceValue.set(internal::DEFAULT_MIN_SOURCE_VALUE);
      maxSourceValue.set(internal::DEFAULT_MAX_SOURCE_VALUE);
      minTargetValue.set(internal::DEFAULT_MIN_TARGET_VALUE);
      maxTargetValue.set(internal::DEFAULT_MAX_TARGET_VALUE);
      minTargetJump.set(internal::DEFAULT_MIN_TARGET_JUMP);
      maxTargetJump.set(internal::DEFAULT_MAX_TARGET_JUMP);
      eelControlTransformation.set(internal::DEFAULT_EEL_CONTROL_TRANSFORMATION);
      eelFeedbackTransformation.set(internal::DEFAULT_EEL_FEEDBACK_TRANSFORMATION);
      ignoreOutOfRangeSourceValuesIsEnabled.set(internal::DEFAULT_IGNORE_OUT_OF_RANGE_SOURCE_VALUES_IS_ENABLED);
      roundTargetValue.set(internal::DEFAULT_ROUND_TARGET_VALUE);
      scaleModeEnabled.set(internal::DEFAULT_SCALE_MODE_ENABLED);
      rotateIsEnabled.set(internal::DEFAULT_ROTATE_IS_ENABLED);
      reverseIsEnabled.set(internal::DEFAULT_REVERSE_IS_ENABLED);
      setPreferredValues(source, target);
    }
    // Changes settings if there are some preferred ones for a certain source or target.
    template<typename Target>
    void setPreferredValues(const Source& source, const Target& target) {
      minStepSize.set(getDefaultMinStepSize(target));
      maxStepSize.set(getDefaultMaxStepSize(target));
    }
    template<typename Target>
    void setPreferredTypeAndValues(const Source& source, const Target& target) {
      type.set(getPreferredModeType(source, target));
      setPreferredValues(source, target);
    }
    rxcpp::observable<bool> changed() const {
      return type.changed()
          .merge(minTargetValue.changed())
          .merge(maxTargetValue.changed())
          .merge(minSourceValue.changed())
          .merge(maxSourceValue.changed())
          .merge(reverseIsEnabled.changed())
          .merge(ignoreOutOfRangeSourceValuesIsEnabled.changed())
          .merge(minTargetJump.changed())
          .merge(maxTargetJump.changed())
          .merge(eelControlTransformation.changed())
          .merge(eelFeedbackTransformation.changed())
          .merge(roundTargetValue.changed())
          .merge(scaleModeEnabled.changed())
          .merge(minStepSize.changed())
          .merge(maxStepSize.changed())
          .merge(rotateIsEnabled.changed());
    }
    void serializeToJson(nlohmann::json& j) const {
      j["type"] = static_cast<int>(type.get());
      j["minSourceValue"] = minSourceValue.get();
      j["maxSourceValue"] = maxSourceValue.get();
      j["minTargetValue"] = minTargetValue.get();
      j["maxTargetValue"] = maxTargetValue.get();
      if (supportsReverseIsEnabled()) {
        j["reverseIsEnabled"] = reverseIsEnabled.get();
      }
      if (supportsIgnoreOutOfRangeSourceValuesIsEnabled()) {
        j["ignoreOutOfRangeSourceValuesIsEnabled"] = ignoreOutOfRangeSourceValuesIsEnabled.get();
      }
      if (supportsRoundTargetValue()) {
        j["roundTargetValue"] = roundTargetValue.get();
      }
      if (supportsScaleModeEnabled()) {
        j["scaleModeEnabled"] = scaleModeEnabled.get();
      }
      if (supportsTargetJump()) {
        j["minTargetJump"] = minTargetJump.get();
        j["maxTargetJump"] = maxTargetJump.get();
      }
      if (supportsEelControlTransformation()) {
        j["eelControlTransformation"] = eelControlTransformation.get();
      }
      if (supportsEelFeedbackTransformation()) {
        j["eelFeedbackTransformation"] = eelFeedbackTransformation.get();
      }
      if (supportsStepSize()) {
        j["minStepSize"] = minStepSize.get();
        j["maxStepSize"] = maxStepSize.get();
      }
      if (supportsRotateIsEnabled()) {
        j["rotateIsEnabled"] = rotateIsEnabled.get();
      }
    }
    void updateFromJson(const nlohmann::json& j) {
      {
        const int typeIndex = j.at("type");
        type.set(static_cast<ModeType>(typeIndex));
      }
      minSourceValue.set(j.at("minSourceValue"));
      maxSourceValue.set(j.at("maxSourceValue"));
      minTargetValue.set(j.at("minTargetValue"));
      maxTargetValue.set(j.at("maxTargetValue"));
      if (j.count("reverseIsEnabled")) {
        reverseIsEnabled.set(j.at("reverseIsEnabled"));
      }
      if (j.count("ignoreOutOfRangeSourceValuesIsEnabled")) {
        ignoreOutOfRangeSourceValuesIsEnabled.set(j.at("ignoreOutOfRangeSourceValuesIsEnabled"));
      }
      if (j.count("roundTargetValue")) {
        roundTargetValue.set(j.at("roundTargetValue"));
      }
      if (j.count("scaleModeEnabled")) {
        scaleModeEnabled.set(j.at("scaleModeEnabled"));
      }
      if (j.count("minTargetJump")) {
        minTargetJump.set(j.at("minTargetJump"));
      }
      if (j.count("maxTargetJump")) {
        maxTargetJump.set(j.at("maxTargetJump"));
      }
      if (j.count("eelControlTransformation")) {
        eelControlTransformation.set(j.at("eelControlTransformation"));
      }
      if (j.count("eelFeedbackTransformation")) {
        eelFeedbackTransformation.set(j.at("eelFeedbackTransformation"));
      }
      if (j.count("minStepSize")) {
        minStepSize.set(j.at("minStepSize"));
      }
      if (j.count("maxStepSize")) {
        maxStepSize.set(j.at("maxStepSize"));
      }
      if (j.count("rotateIsEnabled")) {
        rotateIsEnabled.set(j.at("rotateIsEnabled"));
      }
    }
    template<typename Target>
    bool settingsMakeSense(const Source& source, const Target& target) const {
      switch (source.getCharacter()) {
        case SourceCharacter::Range:
          return type.get() == ModeType::Absolute;
        case SourceCharacter::Switch: {
          switch (type.get()) {
            case ModeType::Absolute:
            case ModeType::Toggle:
              return !target.wantsToBeHitWithStepCounts();
            case ModeType::Relative: {
              if (target.wantsToBeHitWithStepCounts()) {
                return true;
              }
              switch (target.getCharacter()) {
                case TargetCharacter::Continuous:
                case TargetCharacter::Discrete:
                  return true;
                case TargetCharacter::Switch:
                case TargetCharacter::Trigger:
                  return false;
              }
            }
          }
        }
        case SourceCharacter::Encoder1:
        case SourceCharacter::Encoder2:
        case SourceCharacter::Encoder3:
          return type.get() == ModeType::Relative;
        default:
          return true;
      }
    }
    //endregion

    //region Processing
    ModeProcessor& getProcessor() {
      return processor_;
    }

    const ModeProcessor& getProcessor() const {
      return processor_;
    }

    template<typename SourceContext, typename Target>
    void feedback(Source& source, const Target& target, SourceContext& sourceContext) {
      switch (type.get()) {
        case ModeType::Absolute:
          feedbackInAbsoluteMode(source, target, sourceContext);
          break;
        case ModeType::Relative:
          feedbackInRelativeMode(source, target, sourceContext);
          break;
        case ModeType::Toggle:
          feedbackInToggleMode(source, target, sourceContext);
          break;
        default:
          break;
      }
    }
    //endregion
  private:
    void initialize() {
      ensureThatMinValsAlwaysLowerThanMaxVals();
      initEelTransformation();
      keepProcessorInSync();
    }

    void keepProcessorInSync() {
      changed().subscribe([this](bool) {
        processor_ = createProcessor();
      });
    }

    template<typename Target>
    double getDefaultMinStepSize(const Target& target) const {
      if (target.getCharacter() == TargetCharacter::Discrete) {
        return target.getStepSize();
      } else {
        return internal::DEFAULT_MIN_STEP_SIZE;
      }
    }
    template<typename Target>
    double getDefaultMaxStepSize(const Target& target) const {
      if (target.getCharacter() == TargetCharacter::Discrete) {
        return target.getStepSize();
      } else {
        return internal::DEFAULT_MAX_STEP_SIZE;
      }
    }
    template<typename SourceContext, typename Target>
    void feedbackInAbsoluteMode(Source& source, const Target& target, SourceContext& sourceContext) {
      const double absoluteValue = target.getCurrentValue();
      if (absoluteValue != -1) {
        const double tmpValue = reverseIsEnabled.get() ? 1 - absoluteValue : absoluteValue;
        const double transformedSourceValue = transformFeedbackValue(tmpValue);
        const double mappedSourceValue =
            util::mapValueInRangeToNormalizedValue(
                transformedSourceValue, minTargetValue.get(), maxTargetValue.get());
        const double normalizedSourceValue =
            util::mapNormalizedValueToValueInRange(mappedSourceValue, minSourceValue.get(), maxSourceValue.get());
        source.feedback(normalizedSourceValue, sourceContext);
      }
    }
    template<typename SourceContext, typename Target>
    void feedbackInRelativeMode(Source& source, const Target& target, SourceContext& sourceContext) {
      const double absoluteValue = target.getCurrentValue();
      if (absoluteValue != -1) {
        const double tmpValue = reverseIsEnabled.get() ? 1 - absoluteValue : absoluteValue;
        const double mappedSourceValue =
            util::mapValueInRangeToNormalizedValue(tmpValue, minTargetValue.get(), maxTargetValue.get());
        source.feedback(mappedSourceValue, sourceContext);
      }
    }
    template<typename SourceContext, typename Target>
    void feedbackInToggleMode(Source& source, const Target& target, SourceContext& sourceContext) {
      auto absoluteValue = target.getCurrentValue();
      if (absoluteValue != -1) {
        // Toggle switches between min and max target value and when doing feedback we want this to translate
        // to min source and max source value. But we also allow feedback of values inbetween. Then users can detect
        // whether a parameter is somewhere between target min and max.
        const double mappedSourceValue =
            util::mapValueInRangeToNormalizedValue(absoluteValue, minTargetValue.get(), maxTargetValue.get());
        const double sourceValue =
            util::mapNormalizedValueToValueInRange(mappedSourceValue, minSourceValue.get(), maxSourceValue.get());
        source.feedback(sourceValue, sourceContext);
      }
    }
    double transformFeedbackValue(double normalizedValue) const {
      if (feedbackCodeHandle_ == nullptr) {
        return normalizedValue;
      }
      *feedbackVariableX_ = normalizedValue;
      *feedbackVariableY_ = normalizedValue;
      NSEEL_code_execute(feedbackCodeHandle_.get());
      return *feedbackVariableX_;
    }
    void ensureThatMinValsAlwaysLowerThanMaxVals() {
      internal::ensureThatMinAlwaysLowerThanMax(minTargetValue, maxTargetValue);
      internal::ensureThatMinAlwaysLowerThanMax(minSourceValue, maxSourceValue);
      internal::ensureThatMinAlwaysLowerThanMax(minTargetJump, maxTargetJump);
      internal::ensureThatMinAlwaysLowerThanMax(minStepSize, maxStepSize);
    }
    void initEelTransformation() {
      compileEelFeedbackTransformation();
      // @closureIsSafe
      eelFeedbackTransformation.changed().subscribe([this](bool) {
        compileEelFeedbackTransformation();
      });
    }
    void compileEelFeedbackTransformation() {
      if (boost::trim_copy(eelFeedbackTransformation.get()).empty()) {
        feedbackCodeHandle_.reset(nullptr);
      } else {
        NSEEL_CODEHANDLE raw = NSEEL_code_compile(feedbackVm_.get(), eelFeedbackTransformation.get().c_str(), 0);
        feedbackCodeHandle_.reset(raw);
      }
    }
    template<typename Target>
    ModeType getPreferredModeType(const Source& source, const Target& target) {
      switch (source.getCharacter()) {
        case SourceCharacter::Range:
          return ModeType::Absolute;
        case SourceCharacter::Switch: {
          if (target.wantsToBeHitWithStepCounts()) {
            return ModeType::Relative;
          } else {
            switch (target.getCharacter()) {
              case TargetCharacter::Continuous:
              case TargetCharacter::Trigger:
                return ModeType::Absolute;
              case TargetCharacter::Discrete:
                return ModeType::Relative;
              case TargetCharacter::Switch:
                return ModeType::Toggle;
            }
          }
        }
        case SourceCharacter::Encoder1:
        case SourceCharacter::Encoder2:
        case SourceCharacter::Encoder3:
          return ModeType::Relative;
        default:
          return ModeType::Absolute;
      }
    }
    ModeProcessor createProcessor() const {
      return ModeProcessor(
          type.get(),
          minTargetValue.get(),
          maxTargetValue.get(),
          minSourceValue.get(),
          maxSourceValue.get(),
          reverseIsEnabled.get(),
          ignoreOutOfRangeSourceValuesIsEnabled.get(),
          minTargetJump.get(),
          maxTargetJump.get(),
          eelControlTransformation.get(),
          roundTargetValue.get(),
          scaleModeEnabled.get(),
          minStepSize.get(),
          maxStepSize.get(),
          rotateIsEnabled.get()
      );
    }
  };
}