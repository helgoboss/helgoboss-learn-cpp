#pragma once

#include <vector>
#include <memory>
#include <rxcpp/rx.hpp>
#include <helgoboss-midi/MidiMessage.h>
#include <helgoboss-midi/Midi14BitCcMessage.h>
#include <helgoboss-midi/MidiParameterNumberMessage.h>
#include "SourceCharacter.h"
#include "Source.h"
#include "rx/limit_to_items_like_first_one.h"
#include "rx/wait_for_some_more_items.h"

namespace helgoboss::util {
  SourceCharacter guessSourceCharacter(const std::vector<MidiMessage>& relatedCcMessages);

  template<typename Coordination>
  rxcpp::observable<std::shared_ptr<Source>> parseSources(
      rxcpp::observable<MidiMessage> midiMessages,
      rxcpp::observable<Midi14BitCcMessage> midi14BitCcMessages,
      rxcpp::observable<MidiParameterNumberMessage> midiParameterNumberMessages,
      Coordination coordination,
      bool firstCcMessageSetsTheAgenda = true
  ) {
    const auto ccMidiMessages = midiMessages
        .filter([](MidiMessage msg) {
          return msg.getType() == MidiMessageType::ControlChange;
        });
    const auto nonCcMidiMessages = midiMessages
        .filter([](MidiMessage msg) {
          return msg.getType() != MidiMessageType::ControlChange;
        });
    const rxcpp::observable<MidiMessage> lockedCcMidiMessages = ccMidiMessages.template lift<MidiMessage>(
        limitToItemsLikeFirstOne<MidiMessage>([](MidiMessage itemOne, MidiMessage itemTwo) {
          return itemOne.getChannel() == itemTwo.getChannel() &&
              itemOne.getControllerNumber() == itemTwo.getControllerNumber();
        })
    );
    const auto actualCcMidiMessages = firstCcMessageSetsTheAgenda ? lockedCcMidiMessages.as_dynamic()
                                                                  : ccMidiMessages.as_dynamic();
    const auto ccValueSources = actualCcMidiMessages
        .template lift<std::vector<MidiMessage >>(
            waitForSomeMoreItems<MidiMessage>(
                10, // wait for 10 items
                std::chrono::milliseconds(250), // for a maximum of n ms
                coordination // best on audio thread
            )
        )
        .map([](std::vector<MidiMessage> msgs) {
          auto source = std::make_shared<Source>();
          source->updateFromMidiMessage(msgs.at(0));
          source->customCharacter.set(guessSourceCharacter(msgs));
          return source;
        });
    const auto nonCcValueSources = nonCcMidiMessages
        .map([](MidiMessage msg) {
          auto source = std::make_shared<Source>();
          source->updateFromMidiMessage(msg);
          return source;
        });
    const auto parameterMessageValueSources = midiParameterNumberMessages
        .map([](MidiParameterNumberMessage msg) {
          auto source = std::make_shared<Source>();
          source->updateFromMidiParameterNumberMessage(msg);
          return source;
        });
    const auto midi14BitCcMessageSources = midi14BitCcMessages
        .map([](Midi14BitCcMessage msg) {
          auto source = std::make_shared<Source>();
          source->updateFromMidi14BitCcMessage(msg);
          return source;
        });
    return ccValueSources
        .merge(nonCcValueSources)
        .merge(parameterMessageValueSources)
        .merge(midi14BitCcMessageSources);
  }
}