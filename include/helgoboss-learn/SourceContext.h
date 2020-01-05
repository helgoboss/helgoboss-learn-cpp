#pragma once

#include <array>
#include <helgoboss-midi/MidiMessage.h>

namespace helgoboss {
  class SourceContext {
  public:
    virtual void processMidiFeedback(const void* source, const MidiMessage& message) = 0;
    // Good for 14-bit CC
    virtual void processMidiFeedbackTwo(const void* source, const std::array<MidiMessage, 2>& messages) = 0;
    // TODO Add variant with four messages (for parameter messages)
  };
}