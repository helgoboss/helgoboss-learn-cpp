#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <boost/optional.hpp>
#include <helgoboss-midi/MidiMessage.h>

namespace helgoboss {
  enum class MidiClockTransportMessageType {
    Start,
    Continue,
    Stop
  };
  
  void to_json(nlohmann::json& j, const MidiClockTransportMessageType& o);
  void from_json(const nlohmann::json& j, MidiClockTransportMessageType& o);

  namespace util {
    std::string getMidiClockTransportMessageTypeLabel(MidiClockTransportMessageType type);
    boost::optional<MidiMessageType> mapMidiClockTransportMessageTypeToMidiMessageType(MidiClockTransportMessageType type);
    std::string serializeMidiClockTransportMessageType(MidiClockTransportMessageType midiClockTransportMessageType);
    MidiClockTransportMessageType deserializeMidiClockTransportMessageType(const std::string& text);
  }
}