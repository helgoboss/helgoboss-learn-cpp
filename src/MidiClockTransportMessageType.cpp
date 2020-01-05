#include <helgoboss-learn/MidiClockTransportMessageType.h>
#include <gsl/gsl>

namespace helgoboss {
  namespace util {
    boost::optional<MidiMessageType> mapMidiClockTransportMessageTypeToMidiMessageType(MidiClockTransportMessageType type) {
      switch (type) {
        case MidiClockTransportMessageType::Start:
          return MidiMessageType::Start;
        case MidiClockTransportMessageType::Continue:
          return MidiMessageType::Continue;
        case MidiClockTransportMessageType::Stop:
          return MidiMessageType::Stop;
        default:
          return boost::none;
      }
    }
    std::string getMidiClockTransportMessageTypeLabel(MidiClockTransportMessageType type) {
      switch (type) {
        case MidiClockTransportMessageType::Start:
          return "Start";
        case MidiClockTransportMessageType::Continue:
          return "Continue";
        case MidiClockTransportMessageType::Stop:
          return "Stop";
        default:
          Expects(false);
      }
    }
    std::string serializeMidiClockTransportMessageType(MidiClockTransportMessageType midiClockTransportMessageType) {
      switch (midiClockTransportMessageType) {
        case MidiClockTransportMessageType::Start:
          return "Start";
        case MidiClockTransportMessageType::Continue:
          return "Continue";
        case MidiClockTransportMessageType::Stop:
          return "Stop";
        default:
          Expects(false);
      }
    }
    MidiClockTransportMessageType deserializeMidiClockTransportMessageType(const std::string& text) {
      if (text == "Start") {
        return MidiClockTransportMessageType::Start;
      }
      if (text == "Continue") {
        return MidiClockTransportMessageType::Continue;
      }
      if (text == "Stop") {
        return MidiClockTransportMessageType::Stop;
      }
      Expects(false);
    }
  }
  void to_json(nlohmann::json& j, const MidiClockTransportMessageType& o) {
    j = util::serializeMidiClockTransportMessageType(o);
  }
  void from_json(const nlohmann::json& j, MidiClockTransportMessageType& o) {
    o = util::deserializeMidiClockTransportMessageType(j);
  }
}