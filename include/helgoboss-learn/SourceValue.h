#pragma once

#include <boost/variant.hpp>
#include <helgoboss-midi/MidiMessage.h>
#include <helgoboss-midi/MidiParameterNumberMessage.h>
#include <helgoboss-midi/Midi14BitCcMessage.h>
#include <helgoboss-learn/TempoMessage.h>

namespace helgoboss {
  enum class SourceValueType {
    MidiMessage,
    MidiParameterNumberMessage,
    Midi14BitCcMessage,
    TempoMessage
  };

  class SourceValue {
  private:
    boost::variant<MidiMessage,
                   MidiParameterNumberMessage,
                   Midi14BitCcMessage,
                   TempoMessage> content_ = MidiMessage::empty();
  public:
    SourceValue() = default;
    explicit SourceValue(MidiMessage content);
    explicit SourceValue(MidiParameterNumberMessage content);
    explicit SourceValue(Midi14BitCcMessage content);
    explicit SourceValue(TempoMessage content);

    SourceValueType getType() const;
    const MidiMessage& getAsMidiMessage() const;
    const MidiParameterNumberMessage& getAsMidiParameterNumberMessage() const;
    const Midi14BitCcMessage& getAsMidi14BitCcMessage() const;
    const TempoMessage& getAsTempoMessage() const;
  };
}