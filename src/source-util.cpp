#include <helgoboss-learn/source-util.h>

using helgoboss::SourceCharacter;
using helgoboss::MidiMessage;

namespace {
  enum class Direction {
    Decrement,
    None,
    Increment
  };

  SourceCharacter guessEncoderType(const std::vector<MidiMessage>& relatedCcMessages) {
    const auto v = relatedCcMessages.at(0).getDataByte2();
    if ((v >= 1 && v <= 7) || (v >= 121 && v <= 127)) {
      return SourceCharacter::Encoder1;
    } else if (v >= 57 && v <= 71) {
      return SourceCharacter::Encoder2;
    } else {
      return SourceCharacter::Encoder3;
    }
  }

  Direction determineDirection(int firstValue, int secondValue) {
    if (firstValue == secondValue) {
      return Direction::None;
    } else if (firstValue < secondValue) {
      return Direction::Increment;
    } else {
      return Direction::Decrement;
    }
  }
}

namespace helgoboss::util {
  SourceCharacter guessSourceCharacter(const std::vector<MidiMessage>& relatedCcMessages) {
    if (relatedCcMessages.size() == 1) {
      // Only one message received. Looks like a switch has been pressed and not released.
      return SourceCharacter::Switch;
    } else if (relatedCcMessages.size() == 2 && relatedCcMessages.at(1).getDataByte2() == 0) {
      // Two messages received and second message has value 0. Looks like a switch has been pressed and released.
      return SourceCharacter::Switch;
    } else {
      // Multiple messages received. Switch character is ruled out already. Check continuity.
      auto previousDirection = Direction::None;
      for (std::size_t i = 1; i < relatedCcMessages.size(); i++) {
        const auto currentDirection =
            determineDirection(relatedCcMessages.at(i - 1).getDataByte2(), relatedCcMessages.at(i).getDataByte2());
        if (currentDirection == Direction::None) {
          // Same value twice. Not continuous so it's probably an encoder.
          return guessEncoderType(relatedCcMessages);
        } else if (i > 1 && currentDirection != previousDirection) {
          // Direction changed. Not continuous so it's probably an encoder.
          return guessEncoderType(relatedCcMessages);
        }
        previousDirection = currentDirection;
      }
      // Was continuous until now so it's probably a knob/fader
      return SourceCharacter::Range;
    }
  }
}