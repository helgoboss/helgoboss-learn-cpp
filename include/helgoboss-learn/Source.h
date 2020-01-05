#pragma once

#include <rxcpp/rx.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include "SourceValue.h"
#include <helgoboss-midi/MidiMessage.h>
#include "ReactiveProperty.h"
#include "math-util.h"
#include "string-util.h"
#include "Tempo.h"
#include <gsl/gsl>
#include <cmath>
#include <sstream>
#include "SourceType.h"
#include "SourceCharacter.h"
#include "SourceProcessor.h"
#include "MidiClockTransportMessageType.h"

namespace helgoboss {
  class Source {
  public:
    ReactiveProperty<SourceType> type{SourceType::ControlChangeValue};
    ReactiveProperty<int> channel{0};
    ReactiveProperty<bool> is14Bit{false};
    ReactiveProperty<bool> isRegistered{false};
    ReactiveProperty<int> midiMessageNumber{0};
    ReactiveProperty<int> parameterNumberMessageNumber{0};
    ReactiveProperty<SourceCharacter> customCharacter{SourceCharacter::Range};
    ReactiveProperty<MidiClockTransportMessageType> midiClockTransportMessageType{MidiClockTransportMessageType::Start};
  private:
    SourceProcessor processor_ = createProcessor();
  public:
    Source() {
      initialize();
    }
    Source(const Source& other) :
        type(other.type),
        channel(other.channel),
        is14Bit(other.is14Bit),
        isRegistered(other.isRegistered),
        midiMessageNumber(other.midiMessageNumber),
        parameterNumberMessageNumber(other.parameterNumberMessageNumber),
        customCharacter(other.customCharacter),
        midiClockTransportMessageType(other.midiClockTransportMessageType),
        processor_(other.processor_) {
      initialize();
    }
    // TODO Right now move constructor will invoke copy constructor. Maybe optimize later.
    // Default assignment is okay because object and therefore reactive properties stay the same, just not their values.
    Source& operator=(const Source& other) = default;
    Source& operator=(Source&& other) noexcept = default;

    //region Property support queries

    bool supportsChannel() const {
      switch (type.get()) {
        case SourceType::ChannelPressureAmount:
        case SourceType::ControlChangeValue:
        case SourceType::NoteVelocity:
        case SourceType::PolyphonicKeyPressureAmount:
        case SourceType::NoteKeyNumber:
        case SourceType::ParameterNumberMessageValue:
        case SourceType::PitchBendChangeValue:
        case SourceType::ProgramChangeNumber:
          return true;
        default:
          return false;
      }
    }
    bool supports14Bit() const {
      switch (type.get()) {
        case SourceType::ControlChangeValue:
        case SourceType::ParameterNumberMessageValue:
          return true;
        default:
          return false;
      }
    }
    bool supportsIsRegistered() const {
      return type.get() == SourceType::ParameterNumberMessageValue;
    }
    bool supportsMidiMessageNumber() const {
      switch (type.get()) {
        case SourceType::ControlChangeValue:
        case SourceType::NoteVelocity:
        case SourceType::PolyphonicKeyPressureAmount:
          return true;
        default:
          return false;
      }
    }
    bool supportsParameterNumberMessageNumber() const {
      return type.get() == SourceType::ParameterNumberMessageValue;
    }
    bool supportsCustomCharacter() const {
      return type.get() == SourceType::ControlChangeValue && !is14Bit.get();
    }
    bool supportsMidiClockTransportMessageType() const {
      return type.get() == SourceType::ClockTransport;
    }
    //endregion

    //region UI/modification/persistence/management

    SourceCharacter getCharacter() const {
      switch (type.get()) {
        case SourceType::ControlChangeValue:
          return is14Bit.get() ? SourceCharacter::Range : customCharacter.get();
        case SourceType::NoteVelocity:
          return SourceCharacter::Switch;
        case SourceType::ClockTransport:
          // TODO Introduce new character "Trigger"
          return SourceCharacter::Switch;
        default:
          return SourceCharacter::Range;
      }
    }
    rxcpp::observable<bool> changed() const {
      return type.changed()
          .merge(channel.changed())
          .merge(is14Bit.changed())
          .merge(isRegistered.changed())
          .merge(midiMessageNumber.changed())
          .merge(parameterNumberMessageNumber.changed())
          .merge(customCharacter.changed())
          .merge(midiClockTransportMessageType.changed());
    }
    void serializeToJson(nlohmann::json& j, bool useStringsForEnums = false) const {
      if (useStringsForEnums) {
        j["type"] = type.get();
      } else {
        j["type"] = static_cast<int>(type.get());
      }
      if (supportsChannel()) {
        j["channel"] = channel.get();
      }
      if (supportsMidiMessageNumber()) {
        j["number"] = midiMessageNumber.get();
      } else if (supportsParameterNumberMessageNumber()) {
        j["number"] = parameterNumberMessageNumber.get();
      }
      if (supportsCustomCharacter()) {
        if (useStringsForEnums) {
          j["character"] = customCharacter.get();
        } else {
          j["character"] = static_cast<int>(customCharacter.get());
        }
      }
      if (supportsIsRegistered()) {
        j["isRegistered"] = isRegistered.get();
      }
      if (supports14Bit()) {
        j["is14Bit"] = is14Bit.get();
      }
      if (supportsMidiClockTransportMessageType()) {
        if (useStringsForEnums) {
          j["message"] = midiClockTransportMessageType.get();
        } else {
          j["message"] = static_cast<int>(midiClockTransportMessageType.get());
        }
      }
    }
    void updateFromJson(const nlohmann::json& j) {
      using nlohmann::json;
      // Important to set type first
      {
        const auto typeJson = j.at("type");
        if (typeJson.type() == json::value_t::string) {
          type.set(typeJson);
        } else {
          const int typeIndex = typeJson;
          type.set(static_cast<SourceType>(typeIndex));
        }
      }
      if (j.count("channel")) {
        channel.set(j.at("channel"));
      }
      if (j.count("number")) {
        const int number = j.at("number");
        if (supportsMidiMessageNumber()) {
          midiMessageNumber.set(number);
        } else if (supportsParameterNumberMessageNumber()) {
          parameterNumberMessageNumber.set(number);
        }
      }
      if (j.count("character")) {
        const auto characterJson = j.at("character");
        if (characterJson.type() == json::value_t::string) {
          customCharacter.set(characterJson);
        } else {
          const int characterIndex = characterJson;
          customCharacter.set(static_cast<SourceCharacter>(characterIndex));
        }
      }
      if (j.count("isRegistered")) {
        isRegistered.set(j.at("isRegistered"));
      }
      if (j.count("is14Bit")) {
        is14Bit.set(j.at("is14Bit"));
      }
      if (j.count("message")) {
        const auto messageJson = j.at("message");
        if (messageJson.type() == json::value_t::string) {
          midiClockTransportMessageType.set(messageJson);
        } else {
          const int index = messageJson;
          midiClockTransportMessageType.set(static_cast<MidiClockTransportMessageType>(index));
        }
      }
    }
    std::string toString() const {
      std::stringstream ss;
      ss << getMainLabel();
      if (supportsChannel()) {
        if (channel.get() == -1) {
          ss << "\nAny channel";
        } else {
          ss << "\nChannel " << channel.get() + 1;
        }
      }
      switch (type.get()) {
        case SourceType::NoteVelocity:
        case SourceType::PolyphonicKeyPressureAmount:
          if (midiMessageNumber.get() == -1) {
            ss << "\nAny note";
          } else {
            ss << "\nNote number " << midiMessageNumber.get();
          }
          break;
        case SourceType::ControlChangeValue:
          if (midiMessageNumber.get() == -1) {
            ss << "\nAny CC";
          } else {
            ss << "\nCC number " << midiMessageNumber.get();
          }
          break;
        case SourceType::ParameterNumberMessageValue:
          ss << "\nNumber " << parameterNumberMessageNumber.get();
          break;
        default:
          break;
      }
      return ss.str();
    }
    std::string formatNormalizedValue(double normalizedValue) const {
      switch (type.get()) {
        case SourceType::ClockTempo:
          return util::toString(8, [normalizedValue](char* buffer, int maxSize) {
            sprintf(buffer, "%.2f", Tempo::ofNormalizedValue(normalizedValue).bpm());
          });
        case SourceType::ClockTransport:
          return "1";
        default:
          return std::to_string(makeDiscrete(normalizedValue));
      }
    }
    double normalizeDiscreteValue(double discreteValue) const {
      return util::mapValueInRangeToNormalizedValue(
          discreteValue,
          processor_.getMinDiscreteValue(),
          processor_.getMaxDiscreteValue()
      );
    }
    void updateFromMidiMessage(const MidiMessage& msg) {
      if (msg.getSuperType() != MidiMessageSuperType::Channel) {
        return;
      }
      if (const auto sourceType = util::getSourceTypeFromMidiMessageType(msg.getType())) {
        type.set(*sourceType);
        channel.set(msg.getChannel());
        if (supportsMidiMessageNumber()) {
          midiMessageNumber.set(msg.getDataByte1());
        }
        is14Bit.set(false);
      }
    }
    void updateFromMidiParameterNumberMessage(const MidiParameterNumberMessage& msg) {
      type.set(SourceType::ParameterNumberMessageValue);
      channel.set(msg.getChannel());
      parameterNumberMessageNumber.set(msg.getNumber());
      isRegistered.set(msg.isRegistered());
      is14Bit.set(msg.is14bit());
    }
    void updateFromMidi14BitCcMessage(const Midi14BitCcMessage& msg) {
      type.set(SourceType::ControlChangeValue);
      channel.set(msg.getChannel());
      midiMessageNumber.set(msg.getMsbControllerNumber());
      is14Bit.set(true);
    }
    bool equals(const Source& rhs, bool ignoreCustomCharacter, bool ignoreChannel) const {
      if (type != rhs.type) {
        return false;
      }
      switch (type.get()) {
        case SourceType::ControlChangeValue:
          return (ignoreChannel || channel == rhs.channel) && midiMessageNumber == rhs.midiMessageNumber
              && is14Bit == rhs.is14Bit
              && (is14Bit.get() || (ignoreCustomCharacter || customCharacter == rhs.customCharacter));
        case SourceType::NoteVelocity:
        case SourceType::PolyphonicKeyPressureAmount:
          return (ignoreChannel || channel == rhs.channel) && midiMessageNumber == rhs.midiMessageNumber;
        case SourceType::NoteKeyNumber:
        case SourceType::PitchBendChangeValue:
        case SourceType::ChannelPressureAmount:
        case SourceType::ProgramChangeNumber:
          return (ignoreChannel || channel == rhs.channel);
        case SourceType::ParameterNumberMessageValue:
          return (ignoreChannel || channel == rhs.channel)
              && parameterNumberMessageNumber == rhs.parameterNumberMessageNumber && is14Bit == rhs.is14Bit
              && isRegistered == rhs.isRegistered;
        case SourceType::ClockTempo:
          return true;
        case SourceType::ClockTransport:
          return midiClockTransportMessageType == rhs.midiClockTransportMessageType;
        default:
          return false;
      }
    }
    friend bool operator==(const Source& lhs, const Source& rhs) {
      return lhs.equals(rhs, false, false);
    }
    friend bool operator!=(const Source& lhs, const Source& rhs) {
      return !(rhs == lhs);
    }
    //endregion

    //region Processing
    const SourceProcessor& getProcessor() const {
      return processor_;
    }

    template<typename SourceContext>
    void feedback(double normalizedTargetValue, SourceContext& context) {
      Expects(normalizedTargetValue >= 0 && normalizedTargetValue <= 1);
      if (!supportsChannel()) {
        return;
      }
      if (channel.get() == -1) {
        return;
      }
      if (supportsMidiMessageNumber() && midiMessageNumber.get() == -1) {
        return;
      }
      switch (type.get()) {
        case SourceType::NoteVelocity: {
          context.processMidiFeedback(this,
              MidiMessage::noteOn(channel.get(), midiMessageNumber.get(), makeDiscrete(normalizedTargetValue)));
          break;
        }
        case SourceType::NoteKeyNumber: {
          context.processMidiFeedback(this,
              MidiMessage::noteOn(channel.get(), makeDiscrete(normalizedTargetValue), 127));
          break;
        }
        case SourceType::ProgramChangeNumber: {
          context.processMidiFeedback(this,
              MidiMessage::programChange(channel.get(), makeDiscrete(normalizedTargetValue)));
          break;
        }
        case SourceType::PitchBendChangeValue: {
          context.processMidiFeedback(this,
              MidiMessage::pitchBendChange(channel.get(), makeDiscrete(normalizedTargetValue)));
          break;
        }
        case SourceType::ChannelPressureAmount: {
          context.processMidiFeedback(this,
              MidiMessage::channelPressure(channel.get(), makeDiscrete(normalizedTargetValue)));
          break;
        }
        case SourceType::PolyphonicKeyPressureAmount: {
          context.processMidiFeedback(this,
              MidiMessage::polyphonicKeyPressure(channel.get(),
                  midiMessageNumber.get(),
                  makeDiscrete(normalizedTargetValue)));
          break;
        }
        case SourceType::ControlChangeValue: {
          if (is14Bit.get()) {
            const auto midi14BitMsg =
                Midi14BitCcMessage(channel.get(), midiMessageNumber.get(), makeDiscrete(normalizedTargetValue));
            context.processMidiFeedbackTwo(this, midi14BitMsg.buildMidiMessages());
          } else {
            context.processMidiFeedback(
                this,
                MidiMessage::controlChange(channel.get(), midiMessageNumber.get(), determine7BitCcFeedbackValue(normalizedTargetValue))
            );
          }
          break;
        }
        default:
          break;
      }
    }
    //endregion

  private:
    int determine7BitCcFeedbackValue(double normalizedTargetValue) const {
      switch (customCharacter.get()) {
        case SourceCharacter::Encoder1:
        case SourceCharacter::Encoder2:
        case SourceCharacter::Encoder3:
          return static_cast<int>(std::ceil(util::mapNormalizedValueToValueInRange(normalizedTargetValue, 0, 127)));
        default:
          return makeDiscrete(normalizedTargetValue);
      }
    }

    int makeDiscrete(double normalizedValue) const {
      // We use ceil because e.g. pitch bend center is not an integer (because there's an even number of possible
      // values) and the official center is considered as the next higher value.
      // Example uncentered: Possible pitch bend values go from 0 to 16383. Exact center would be 8191.5. Official
      // center is 8192.
      // Example centered: Possible pitch bend values go from -8192 to 8191. Exact center would be -0.5. Official
      // center is 0.
      return static_cast<int>(std::ceil(util::mapNormalizedValueToValueInRange(
          normalizedValue,
          processor_.getMinDiscreteValue(),
          processor_.getMaxDiscreteValue()
      )));
    }
    std::string getMainLabel() const {
      switch (type.get()) {
        case SourceType::ControlChangeValue:
          return "CC value";
        case SourceType::NoteVelocity:
          return "Note velocity";
        case SourceType::NoteKeyNumber:
          return "Note number";
        case SourceType::PitchBendChangeValue:
          return "Pitch wheel";
        case SourceType::ChannelPressureAmount:
          return "Channel after touch";
        case SourceType::ProgramChangeNumber:
          return "Program change";
        case SourceType::ParameterNumberMessageValue:
          return isRegistered.get() ? "RPN" : "NRPN";
        case SourceType::PolyphonicKeyPressureAmount:
          return "Poly after touch";
        case SourceType::ClockTempo:
          return "MIDI clock\nTempo";
        case SourceType::ClockTransport:
          return std::string("MIDI clock\n")
              + util::getMidiClockTransportMessageTypeLabel(midiClockTransportMessageType.get());
        default:
          return util::getSourceTypeListEntryLabel(type.get());
      }
    }

    SourceProcessor createProcessor() const {
      return SourceProcessor(
          type.get(),
          channel.get(),
          is14Bit.get(),
          isRegistered.get(),
          supportsMidiMessageNumber() ? midiMessageNumber.get() : parameterNumberMessageNumber.get(),
          customCharacter.get(),
          midiClockTransportMessageType.get()
      );
    }

    void initialize() {
      keepProcessorInSync();
    }

    void keepProcessorInSync() {
      changed().subscribe([this](bool) {
        processor_ = createProcessor();
      });
    }

  };

  void to_json(nlohmann::json& j, const Source& o);
  void from_json(const nlohmann::json& j, Source& o);
}