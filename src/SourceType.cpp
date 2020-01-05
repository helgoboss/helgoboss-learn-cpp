#include <helgoboss-learn/SourceType.h>
#include <gsl/gsl>

namespace helgoboss {
  namespace util {
    std::string getSourceTypeListEntryLabel(SourceType sourceType) {
      switch (sourceType) {
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
          return "(N)RPN value (no feedback)";
        case SourceType::PolyphonicKeyPressureAmount:
          return "Polyphonic after touch";
        case SourceType::ClockTempo:
          return "MIDI clock tempo (experimental)";
        case SourceType::ClockTransport:
          return "MIDI clock transport";
        default:
          return "";
      }
    }
    std::string getSourceTypeNumberLabel(SourceType sourceType) {
      switch (sourceType) {
        case SourceType::ControlChangeValue:
          return "CC number";
          break;
        case SourceType::ParameterNumberMessageValue:
          return "Number";
        case SourceType::NoteVelocity:
        case SourceType::PolyphonicKeyPressureAmount:
          return "Note number";
        default:
          return "";
      }
    }
    boost::optional<SourceType> getSourceTypeFromMidiMessageType(MidiMessageType midiMessageType) {
      switch (midiMessageType) {
        case MidiMessageType::NoteOff:
        case MidiMessageType::NoteOn:
          return SourceType::NoteVelocity;
        case MidiMessageType::PolyphonicKeyPressure:
          return SourceType::PolyphonicKeyPressureAmount;
        case MidiMessageType::ControlChange:
          return SourceType::ControlChangeValue;
        case MidiMessageType::ProgramChange:
          return SourceType::ProgramChangeNumber;
        case MidiMessageType::ChannelPressure:
          return SourceType::ChannelPressureAmount;
        case MidiMessageType::PitchBendChange:
          return SourceType::PitchBendChangeValue;
        case MidiMessageType::TimingClock:
          return SourceType::ClockTempo;
        case MidiMessageType::Start:
        case MidiMessageType::Continue:
        case MidiMessageType::Stop:
          return SourceType::ClockTransport;
        default:
          return boost::none;
      }
    }

    std::string serializeSourceType(SourceType sourceType) {
      switch (sourceType) {
        case SourceType::ControlChangeValue:
          return "ControlChangeValue";
        case SourceType::NoteVelocity:
          return "NoteVelocity";
        case SourceType::NoteKeyNumber:
          return "NoteKeyNumber";
        case SourceType::PitchBendChangeValue:
          return "PitchBendChangeValue";
        case SourceType::ChannelPressureAmount:
          return "ChannelPressureAmount";
        case SourceType::ProgramChangeNumber:
          return "ProgramChangeNumber";
        case SourceType::ParameterNumberMessageValue:
          return "ParameterNumberMessageValue";
        case SourceType::PolyphonicKeyPressureAmount:
          return "PolyphonicKeyPressureAmount";
        case SourceType::ClockTempo:
          return "ClockTempo";
        case SourceType::ClockTransport:
          return "ClockTransport";
        default:
          Expects(false);
      }
    }
    SourceType deserializeSourceType(const std::string& text) {
      if (text == "ControlChangeValue") {
        return SourceType::ControlChangeValue;
      }
      if (text == "NoteVelocity") {
        return SourceType::NoteVelocity;
      }
      if (text == "NoteKeyNumber") {
        return SourceType::NoteKeyNumber;
      }
      if (text == "PitchBendChangeValue") {
        return SourceType::PitchBendChangeValue;
      }
      if (text == "ChannelPressureAmount") {
        return SourceType::ChannelPressureAmount;
      }
      if (text == "ProgramChangeNumber") {
        return SourceType::ProgramChangeNumber;
      }
      if (text == "ParameterNumberMessageValue") {
        return SourceType::ParameterNumberMessageValue;
      }
      if (text == "PolyphonicKeyPressureAmount") {
        return SourceType::PolyphonicKeyPressureAmount;
      }
      if (text == "ClockTempo") {
        return SourceType::ClockTempo;
      }
      if (text == "ClockTransport") {
        return SourceType::ClockTransport;
      }
      Expects(false);
    }
  }
  void to_json(nlohmann::json& j, const SourceType& o) {
    j = util::serializeSourceType(o);
  }
  void from_json(const nlohmann::json& j, SourceType& o) {
    o = util::deserializeSourceType(j);
  }
}