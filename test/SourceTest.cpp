#include <catch.hpp>
#include <helgoboss-learn/Source.h>
#include <helgoboss-learn/SourceContext.h>
#include <helgoboss-learn/Mode.h>
#include <helgoboss-learn/Target.h>
#include <helgoboss-learn/source-util.h>
#include "TestSourceContext.h"

using rxcpp::observable;

namespace helgoboss {

  SCENARIO("Sources") {
    GIVEN("A source") {
      TestSourceContext context;
      Source source;
      int changeCount = 0;
      source.changed().subscribe([&changeCount](bool) {
        changeCount += 1;
      });
      WHEN("used") {
        source.type.set(SourceType::PitchBendChangeValue);
        source.channel.set(13);
        source.feedback(1.0, context);
        const auto& sourceProcessor = source.getProcessor();
        THEN("it should work") {
          REQUIRE(changeCount == 2);
          REQUIRE(source.type.get() == SourceType::PitchBendChangeValue);
          REQUIRE(source.channel.get() == 13);
          REQUIRE(source.supportsChannel());
          REQUIRE(!source.supports14Bit());
          REQUIRE(!source.supportsIsRegistered());
          REQUIRE(!source.supportsMidiMessageNumber());
          REQUIRE(!source.supportsParameterNumberMessageNumber());
          REQUIRE(!source.supportsCustomCharacter());
          REQUIRE(!source.supportsMidiClockTransportMessageType());
          REQUIRE(source.getCharacter() == SourceCharacter::Range);
          REQUIRE(source.toString().find("Channel") != std::string::npos);
          REQUIRE(source.formatNormalizedValue(0.5) == "0");
          REQUIRE(context.oneCount == 1);
          // TODO How can we achieve also in the discrete => normalized direction that 0 (or uncentered 8192) is 0.5
          // and not a bit more? Mmh. See explanation of std::ceil in Source.h
//        REQUIRE(source.normalizeDiscreteValue(0) == 0.5);
          const auto badSourceValue = SourceValue(MidiMessage::noteOn(13, 100, 100));
          REQUIRE(!sourceProcessor.processes(badSourceValue));
          const auto goodSourceValue = SourceValue(MidiMessage::pitchBendChange(13, 8192));
          REQUIRE(sourceProcessor.processes(goodSourceValue));
          // TODO See above
//        REQUIRE(source.getNormalizedValue(goodSourceValue) == 0.5);
          REQUIRE(!sourceProcessor.consumes(MidiMessage::noteOn(13, 100, 100)));
          REQUIRE(!sourceProcessor.consumes(MidiMessage::pitchBendChange(13, 8192)));
          REQUIRE(sourceProcessor.getMaxStepCount() == 63);
          REQUIRE(!sourceProcessor.emitsStepCounts());
        }
      }
    }
  }

  SCENARIO("Guess source character as range") {
    GIVEN("Some pretty continuous MIDI CC messages") {
      const std::vector<MidiMessage> messages{
          MidiMessage::controlChange(0, 5, 80),
          MidiMessage::controlChange(0, 5, 81),
          MidiMessage::controlChange(0, 5, 82),
          MidiMessage::controlChange(0, 5, 83)
      };
      WHEN("guessing source character") {
        const auto character = util::guessSourceCharacter(messages);
        THEN("it should guess range") {
          REQUIRE(character == SourceCharacter::Range);
        }
      }
    }
  }

  SCENARIO("Guess source character as encoder2") {
    GIVEN("Some repetitive MIDI CC messages around 64") {
      const std::vector<MidiMessage> messages{
          MidiMessage::controlChange(0, 5, 65),
          MidiMessage::controlChange(0, 5, 65),
          MidiMessage::controlChange(0, 5, 66)
      };
      WHEN("guessing source character") {
        const auto character = util::guessSourceCharacter(messages);
        THEN("it should guess encoder2") {
          REQUIRE(character == SourceCharacter::Encoder2);
        }
      }
    }
  }

  SCENARIO("Guess source character as switch (released)") {
    GIVEN("Two on-off-style messages") {
      const std::vector<MidiMessage> messages{
          MidiMessage::controlChange(0, 5, 70),
          MidiMessage::controlChange(0, 5, 0)
      };
      WHEN("guessing source character") {
        const auto character = util::guessSourceCharacter(messages);
        THEN("it should guess switch") {
          REQUIRE(character == SourceCharacter::Switch);
        }
      }
    }
  }

  SCENARIO("Guess source character as switch (non-released)") {
    GIVEN("One message") {
      const std::vector<MidiMessage> messages{
          MidiMessage::controlChange(0, 5, 70)
      };
      WHEN("guessing source character") {
        const auto character = util::guessSourceCharacter(messages);
        THEN("it should guess switch") {
          REQUIRE(character == SourceCharacter::Switch);
        }
      }
    }
  }

  SCENARIO("Parse source from MIDI CC messages") {
    GIVEN("Some CC value messages") {
      WHEN("parsing next source") {
        const auto sources = util::parseSources(
            observable<>::from(
                MidiMessage::controlChange(2, 5, 70),
                MidiMessage::controlChange(2, 5, 0)
            ),
            observable<>::empty<Midi14BitCcMessage>(),
            observable<>::empty<MidiParameterNumberMessage>(),
            rxcpp::identity_current_thread()
        );
        const std::shared_ptr<Source> nextSource = sources
            .take(1)
            .as_blocking()
            .first();
        THEN("it should parse a CC source with custom character switch") {
          REQUIRE(nextSource->type.get() == SourceType::ControlChangeValue);
          REQUIRE(nextSource->channel.get() == 2);
          REQUIRE(nextSource->midiMessageNumber.get() == 5);
          REQUIRE(nextSource->customCharacter.get() == SourceCharacter::Switch);
          REQUIRE(!nextSource->is14Bit.get());
        }
      }
    }
  }
}