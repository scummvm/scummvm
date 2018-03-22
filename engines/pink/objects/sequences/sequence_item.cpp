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

#include "sequence_item.h"
#include <common/debug.h>
#include <engines/pink/objects/sequences/sequence.h>
#include <engines/pink/objects/sequences/sequencer.h>
#include <engines/pink/objects/actions/action.h>
#include "engines/pink/archive.h"
#include "engines/pink/objects/pages/game_page.h"
#include "engines/pink/objects/actors/actor.h"

namespace Pink {

void SequenceItem::deserialize(Archive &archive) {
    archive >> _actor >> _action;
}

void SequenceItem::toConsole() {
    debug("\t\t\t\tSequenceItem: _actor=%s, _action=%s", _actor.c_str(), _action.c_str());
}

const Common::String &SequenceItem::getActor() const {
    return _actor;
}

const Common::String &SequenceItem::getAction() const {
    return _action;
}

bool SequenceItem::execute(int unk, Sequence *sequence, bool unk2) {
    Actor *actor;
    Action *action;
    if (!(actor = sequence->_sequencer->_page->findActor(_actor)) ||
        !(action = actor->findAction(_action))) {
        assert(0);
        return false;
    }

    actor->setAction(action, unk2);
    Common::Array<SequenceActorState> &states = sequence->_context->_states;
    for (int i = 0; i < sequence->_context->_states.size(); ++i) {
        if (states[i]._actorName == _actor){
            states[i]._unk = unk;
            sequence->_context->_actor = isLeader() ? actor : sequence->_context->_actor;
            break;
        }
    }

    return true;
}

bool SequenceItem::isLeader() {
    return false;
}

bool SequenceItemLeader::isLeader() {
    return true;
}

void SequenceItemLeader::toConsole() {
    debug("\t\t\t\tSequenceItemLeader: _actor=%s, _action=%s", _actor.c_str(), _action.c_str());
}


void SequenceItemLeaderAudio::deserialize(Archive &archive) {
    SequenceItem::deserialize(archive);
    archive.readDWORD();
}

void SequenceItemLeaderAudio::toConsole() {
    debug("\t\t\t\tSequenceItemLeaderAudio: _actor=%s, _action=%s", _actor.c_str(), _action.c_str());
}

bool SequenceItemDefaultAction::execute(int unk, Sequence *sequence, bool unk2) {
    Common::Array<SequenceActorState> &actorStates = sequence->_context->_states;
    for (int i = 0; i < actorStates.size(); ++i) {
        if (actorStates[i]._actorName == _actor){
            actorStates[i]._actionName = _action;
            break;
        }
    }
    return true;
}

void SequenceItemDefaultAction::toConsole() {
    debug("\t\t\t\tSequenceItemDefaultAction: _actor=%s, _action=%s", _actor.c_str(), _action.c_str());
}

} // End of namespace Pink