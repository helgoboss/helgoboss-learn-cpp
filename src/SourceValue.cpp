#include <helgoboss-learn/SourceValue.h>
#include <gsl/gsl>

namespace helgoboss {
  SourceValue::SourceValue(helgoboss::MidiMessage content) : content_(std::move(content)) {
  }
  SourceValue::SourceValue(helgoboss::MidiParameterNumberMessage content) : content_(std::move(content)) {

  }
  SourceValue::SourceValue(helgoboss::Midi14BitCcMessage content) : content_(std::move(content)) {

  }
  SourceValue::SourceValue(TempoMessage content) : content_(std::move(content)) {

  }
  SourceValueType SourceValue::getType() const {
    switch (content_.which()) {
      case 0:
        return SourceValueType::MidiMessage;
      case 1:
        return SourceValueType::MidiParameterNumberMessage;
      case 2:
        return SourceValueType::Midi14BitCcMessage;
      case 3:
        return SourceValueType::TempoMessage;
      default:
        Expects(false);
    }
  }
  const helgoboss::MidiMessage& SourceValue::getAsMidiMessage() const {
    return boost::get<MidiMessage>(content_);
  }
  const helgoboss::MidiParameterNumberMessage& SourceValue::getAsMidiParameterNumberMessage() const {
    return boost::get<MidiParameterNumberMessage>(content_);
  }
  const helgoboss::Midi14BitCcMessage& SourceValue::getAsMidi14BitCcMessage() const {
    return boost::get<Midi14BitCcMessage>(content_);
  }
  const TempoMessage& SourceValue::getAsTempoMessage() const {
    return boost::get<TempoMessage>(content_);
  }
}