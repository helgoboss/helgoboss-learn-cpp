#pragma once

#include "Tempo.h"
#include "SourceType.h"
#include "SourceCharacter.h"
#include "SourceValue.h"
#include "MidiClockTransportMessageType.h"
#include "math-util.h"

namespace helgoboss {
  class SourceProcessor {
  private:
    SourceType type_ = SourceType::ControlChangeValue;
    int channel_ = 0;
    bool is14Bit_ = false;
    bool isRegistered_ = false;
    int number_ = 0;
    SourceCharacter customCharacter_ = SourceCharacter::Range;
    MidiClockTransportMessageType midiClockTransportMessageType_ = MidiClockTransportMessageType::Start;
  public:
    SourceProcessor() = default;
    SourceProcessor(
        SourceType type,
        int channel,
        bool is14Bit,
        bool isRegistered,
        int number,
        SourceCharacter customCharacter,
        MidiClockTransportMessageType midiClockTransportMessageType
    ) : type_(type),
        channel_(channel),
        is14Bit_(is14Bit),
        isRegistered_(isRegistered),
        number_(number),
        customCharacter_(customCharacter),
        midiClockTransportMessageType_(midiClockTransportMessageType) {
    }
    double getNormalizedValue(const SourceValue& value) const {
      switch (type_) {
        case SourceType::ControlChangeValue: {
          if (is14Bit_) {
            const auto& msg = value.getAsMidi14BitCcMessage();
            return msg.getValue() / 16383.0;
          } else {
            const auto& msg = value.getAsMidiMessage();
            return getNormalizedValueFromControlChange(msg.getControlValue());
          }
        }
        case SourceType::NoteVelocity: {
          const auto& msg = value.getAsMidiMessage();
          if (msg.getType() == MidiMessageType::NoteOff) {
            // Note off
            return 0.0;
          } else {
            // Note on
            return msg.getVelocity() / 127.0;
          }
        }
        case SourceType::NoteKeyNumber: {
          const auto& msg = value.getAsMidiMessage();
          return msg.getKeyNumber() / 127.0;
        }
        case SourceType::PitchBendChangeValue: {
          const auto& msg = value.getAsMidiMessage();
          return util::mapValueInRangeToNormalizedValue(msg.getPitchBendValue(), 0, 16383);
        }
        case SourceType::ChannelPressureAmount: {
          const auto& msg = value.getAsMidiMessage();
          return util::mapValueInRangeToNormalizedValue(
              msg.getPressureAmount(), getMinDiscreteValue(), getMaxDiscreteValue());
        }
        case SourceType::PolyphonicKeyPressureAmount: {
          const auto& msg = value.getAsMidiMessage();
          return util::mapValueInRangeToNormalizedValue(
              msg.getPressureAmount(), getMinDiscreteValue(), getMaxDiscreteValue());
        }
        case SourceType::ProgramChangeNumber: {
          const auto& msg = value.getAsMidiMessage();
          return util::mapValueInRangeToNormalizedValue(
              msg.getProgramNumber(), getMinDiscreteValue(), getMaxDiscreteValue());
        }
        case SourceType::ClockTransport: {
          return 1.0;
        }
        case SourceType::ParameterNumberMessageValue: {
          const auto& msg = value.getAsMidiParameterNumberMessage();
          return util::mapValueInRangeToNormalizedValue(
              msg.getValue(), getMinDiscreteValue(), getMaxDiscreteValue());
        }
        case SourceType::ClockTempo: {
          const auto& msg = value.getAsTempoMessage();
          return Tempo(msg.bpm).normalizedValue();
        }
        default:
          return 0.0;
      }
    }

    bool processes(const SourceValue& value) const {
      switch (type_) {
        case SourceType::NoteVelocity: {
          if (value.getType() != SourceValueType::MidiMessage) {
            return false;
          }
          const auto& msg = value.getAsMidiMessage();
          return msg.isNote() && channelMatches(msg) && numberMatches(msg);
        }
        case SourceType::NoteKeyNumber: {
          if (value.getType() != SourceValueType::MidiMessage) {
            return false;
          }
          const auto& msg = value.getAsMidiMessage();
          return msg.getType() == MidiMessageType::NoteOn && channelMatches(msg);
        }
        case SourceType::PitchBendChangeValue: {
          if (value.getType() != SourceValueType::MidiMessage) {
            return false;
          }
          const auto& msg = value.getAsMidiMessage();
          return msg.getType() == MidiMessageType::PitchBendChange && channelMatches(msg);
        }
        case SourceType::ChannelPressureAmount: {
          if (value.getType() != SourceValueType::MidiMessage) {
            return false;
          }
          const auto& msg = value.getAsMidiMessage();
          return msg.getType() == MidiMessageType::ChannelPressure && channelMatches(msg);
        }
        case SourceType::ProgramChangeNumber: {
          if (value.getType() != SourceValueType::MidiMessage) {
            return false;
          }
          const auto& msg = value.getAsMidiMessage();
          return msg.getType() == MidiMessageType::ProgramChange && channelMatches(msg);
        }
        case SourceType::PolyphonicKeyPressureAmount: {
          if (value.getType() != SourceValueType::MidiMessage) {
            return false;
          }
          const auto& msg = value.getAsMidiMessage();
          return msg.getType() == MidiMessageType::PolyphonicKeyPressure && channelMatches(msg) && numberMatches(msg);
        }
        case SourceType::ControlChangeValue: {
          if (is14Bit_) {
            if (value.getType() != SourceValueType::Midi14BitCcMessage) {
              return false;
            }
            const auto& msg = value.getAsMidi14BitCcMessage();
            return channelMatches(msg.getChannel()) && numberMatches(msg.getMsbControllerNumber());
          } else {
            if (value.getType() != SourceValueType::MidiMessage) {
              return false;
            }
            const auto& msg = value.getAsMidiMessage();
            return msg.getType() == MidiMessageType::ControlChange && channelMatches(msg) && numberMatches(msg);
          }
        }
        case SourceType::ClockTransport: {
          if (value.getType() != SourceValueType::MidiMessage) {
            return false;
          }
          const auto& msg = value.getAsMidiMessage();
          return msg.getType() ==
              util::mapMidiClockTransportMessageTypeToMidiMessageType(midiClockTransportMessageType_);
        }
        case SourceType::ParameterNumberMessageValue: {
          if (value.getType() != SourceValueType::MidiParameterNumberMessage) {
            return false;
          }
          const auto& msg = value.getAsMidiParameterNumberMessage();
          return channelMatches(msg.getChannel()) && numberMatches(msg.getNumber())
              && msg.isRegistered() == isRegistered_
              && msg.is14bit() == is14Bit_;
        }
        case SourceType::ClockTempo:
          return value.getType() == SourceValueType::TempoMessage;
        default:
          return false;
      }
    }

    // Only has to be implemented for sources whose events are composed of multiple MIDI messages
    bool consumes(const MidiMessage& msg) const {
      switch (type_) {
        case SourceType::ControlChangeValue: {
          if (!is14Bit_) {
            return false;
          }
          return msg.getType() == MidiMessageType::ControlChange && channelMatches(msg) &&
              (msg.getControllerNumber() == number_ || msg.getControllerNumber() == number_ + 32);
        }
        case SourceType::ParameterNumberMessageValue: {
          return channelMatches(msg) && helgoboss::util::couldBePartOfParameterNumberMessage(msg);
        }
        default:
          return false;
      }
    }

    // TODO Maybe use numPossibleValues instead
    int getMaxStepCount() const {
      // All encoders support up to 63 different step counts
      return 63;
    }

    bool emitsStepCounts() const {
      switch (type_) {
        case SourceType::ControlChangeValue: {
          if (is14Bit_) {
            return false;
          }
          switch (customCharacter_) {
            case SourceCharacter::Encoder1:
            case SourceCharacter::Encoder2:
            case SourceCharacter::Encoder3:
              return true;
            default:
              return false;
          }
        }
        default:
          return false;
      }
    }

    double getMinDiscreteValue() const {
      switch (type_) {
        case SourceType::ClockTempo:
          return 1.0;
        default:
          return isCentered() ? -getNumPossibleValues() / 2 : 0;
      }
    }
    double getMaxDiscreteValue() const {
      switch (type_) {
        case SourceType::ClockTempo:
          return 960.0;
        default:
          return isCentered() ? getNumPossibleValues() / 2 - 1 : getNumPossibleValues() - 1;
      }
    }

  private:
    bool channelMatches(const MidiMessage& msg) const {
      return channelMatches(msg.getChannel());
    }

    bool channelMatches(int channel) const {
      return channel_ == -1 || channel == channel_;
    }

    bool numberMatches(const MidiMessage& msg) const {
      return numberMatches(msg.getDataByte1());
    }

    bool numberMatches(int number) const {
      return number_ == -1 || number == number_;
    }

    double getNormalizedValueFromControlChange(int controlValue) const {
      switch (customCharacter_) {
        case SourceCharacter::Encoder1: {
          // 127 = decrement; 0 = none; 1 = increment
          // 127 > value > 63 results in higher decrement step sizes (64 possible decrement step sizes)
          // 1 < value <= 63 results in higher increment step sizes (63 possible increment step sizes)
          if (controlValue <= 63) {
            // Zero and increment
            return controlValue;
          } else {
            // Decrement
            return -1 * (128 - controlValue);
          }
        }
        case SourceCharacter::Encoder2: {
          // 63 = decrement; 64 = none; 65 = increment
          // 63 > value >= 0 results in higher decrement step sizes (64 possible decrement step sizes)
          // 65 < value <= 127 results in higher increment step sizes (63 possible increment step sizes)
          if (controlValue >= 64) {
            // Zero and increment
            return controlValue - 64;
          } else {
            // Decrement
            return -1 * (64 - controlValue);
          }
        }
        case SourceCharacter::Encoder3: {
          // 65 = decrement; 0 = none; 1 = increment
          // 65 < value <= 127 results in higher decrement step sizes (63 possible decrement step sizes)
          // 1 < value <= 64 results in higher increment step sizes (64 possible increment step sizes)
          if (controlValue <= 64) {
            // Zero and increment
            return controlValue;
          } else {
            // Decrement
            return -1 * (controlValue - 64);
          }
        }
        default: {
          // Absolute
          return controlValue / 127.0;
        }
      }
    }
    bool isCentered() const {
      switch (type_) {
        case SourceType::PitchBendChangeValue:
          return true;
        default:
          return false;
      }
    }
    int getNumPossibleValues() const {
      switch (type_) {
        case SourceType::ChannelPressureAmount:
        case SourceType::NoteVelocity:
        case SourceType::PolyphonicKeyPressureAmount:
        case SourceType::NoteKeyNumber:
        case SourceType::ProgramChangeNumber:
          return 128;
        case SourceType::ControlChangeValue: {
          if (is14Bit_) {
            return 16384;
          } else {
            switch (customCharacter_) {
              case SourceCharacter::Encoder1:
              case SourceCharacter::Encoder2:
              case SourceCharacter::Encoder3:
                return 64;
              default:
                return 128;
            }
          }
        }
        case SourceType::ParameterNumberMessageValue:
          return is14Bit_ ? 16384 : 128;
        case SourceType::PitchBendChangeValue:
          return 16384;
        case SourceType::ClockTempo:
          return 960;
        case SourceType::ClockTransport:
          return 1;
        default:
          return 0;
      }
    }
  };
}