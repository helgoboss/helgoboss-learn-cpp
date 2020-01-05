#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <boost/optional.hpp>
#include <helgoboss-midi/MidiMessage.h>

namespace helgoboss {
  constexpr int NUM_SOURCE_TYPES = 10;

  // At the moment it's all MIDI sources. Not sure if in future there will be more.
  enum class SourceType {
    ControlChangeValue,
    NoteVelocity,
    NoteKeyNumber,
    PitchBendChangeValue,
    ChannelPressureAmount,
    ProgramChangeNumber,
    ParameterNumberMessageValue,
    PolyphonicKeyPressureAmount,
    ClockTempo,
    ClockTransport
  };
  
  void to_json(nlohmann::json& j, const SourceType& o);
  void from_json(const nlohmann::json& j, SourceType& o);

  namespace util {
    std::string getSourceTypeListEntryLabel(SourceType sourceType);
    std::string getSourceTypeNumberLabel(SourceType sourceType);
    boost::optional<SourceType> getSourceTypeFromMidiMessageType(MidiMessageType midiMessageType);
    std::string serializeSourceType(SourceType sourceType);
    SourceType deserializeSourceType(const std::string& text);
  }
}