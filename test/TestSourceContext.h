#pragma once

#include <helgoboss-learn/SourceContext.h>

namespace helgoboss {
  class TestSourceContext : public SourceContext {
  public:
    int oneCount = 0;
    int twoCount = 0;

    void processMidiFeedback(const void* source, const MidiMessage& message) override {
      oneCount += 1;
    }
    void processMidiFeedbackTwo(const void* source, const std::array<MidiMessage, 2>& messages) override {
      twoCount += 1;
    }
  };
}