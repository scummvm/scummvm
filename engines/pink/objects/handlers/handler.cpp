/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "handler.h"
#include "engines/pink/archive.h"
#include "engines/pink/objects/side_effect.h"
#include <engines/pink/objects/condition.h>
#include <engines/pink/objects/sequences/sequencer.h>
#include <engines/pink/objects/sequences/sequence.h>
#include <engines/pink/objects/actors/lead_actor.h>
#include <engines/pink/objects/pages/game_page.h>
#include <engines/pink/pink.h>

namespace Pink {

void Handler::deserialize(Archive &archive) {
    archive >> _conditions;
    archive >> _sideEffects;
}

bool Handler::isSuitable(LeadActor *actor) {
    for (int i = 0; i < _conditions.size(); ++i) {
        if (!_conditions[i]->evaluate(actor)){
            return false;
        }
    }
    return true;
}

void Handler::executeSideEffects(LeadActor *actor) {
    for (int i = 0; i < _sideEffects.size(); ++i) {
        _sideEffects[i]->execute(actor);
    }
}

void Handler::onMessage(LeadActor *actor) {
    executeSideEffects(actor);
}

void HandlerSequences::deserialize(Archive &archive) {
    Handler::deserialize(archive);
    archive >> _sequences;
}

void HandlerSequences::onMessage(LeadActor *actor) {
    Handler::onMessage(actor);
    Sequencer *sequencer = actor->getSequencer();

    assert(!_sequences.empty());

    Common::RandomSource &rnd = actor->getPage()->getGame()->getRnd();
    uint index = rnd.getRandomNumber(_sequences.size() - 1);

    Sequence *sequence = sequencer->findSequence(_sequences[index]);

    assert(sequence);
    sequencer->authorSequence(sequence, 0);

    handle(sequence);
}

void HandlerStartPage::handle(Sequence *sequence) {
    sequence->_unk = 1;
}

void HandlerStartPage::toConsole() {
    debug("HandlerStartPage:");

    debug("\tSideEffects:");
    for (int i = 0; i < _sideEffects.size(); ++i) {
        _sideEffects[i]->toConsole();
    }

    debug("\tConditions:");
    for (int i = 0; i < _conditions.size(); ++i) {
        _conditions[i]->toConsole();
    }

    debug("\tSequences:");
    for (int i = 0; i < _sequences.size(); ++i) {
        debug("\t\t%s", _sequences[i].c_str());
    }
}

void HandlerLeftClick::toConsole() {
    debug("HandlerLeftClick:");

    debug("\tSideEffects:");
    for (int i = 0; i < _sideEffects.size(); ++i) {
        _sideEffects[i]->toConsole();
    }

    debug("\tConditions:");
    for (int i = 0; i < _conditions.size(); ++i) {
        _conditions[i]->toConsole();
    }

    debug("\tSequences:");
    for (int i = 0; i < _sequences.size(); ++i) {
        debug("\t\t%s", _sequences[i].c_str());
    }
}

void HandlerUseClick::deserialize(Archive &archive) {
    HandlerSequences::deserialize(archive);
    archive >> _inventoryItem >> _recepient;
}

void HandlerUseClick::toConsole() {
    debug("HandlerUseClick: _inventoryItem=%s, _recepient=%s", _inventoryItem.c_str(), _recepient.c_str());
    debug("\tSideEffects:");
    for (int i = 0; i < _sideEffects.size(); ++i) {
        _sideEffects[i]->toConsole();
    }

    debug("\tConditions:");
    for (int i = 0; i < _conditions.size(); ++i) {
        _conditions[i]->toConsole();
    }

    debug("\tSequences:");
    for (int i = 0; i < _sequences.size(); ++i) {
        debug("\t\t%s", _sequences[i].c_str());
    }
}

void HandlerUseClick::handle(Sequence *sequence) {

}

} // End of namespace Pink
