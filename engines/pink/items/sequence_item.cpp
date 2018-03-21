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

#include <common/debug.h>
#include <engines/pink/sequences/sequence.h>
#include <engines/pink/sequences/sequencer.h>
#include <engines/pink/actions/action.h>
#include "sequence_item.h"
#include "../archive.h"
#include "sequence_item_leader.h"
#include "sequence_item_default_action.h"
#include "../page.h"
#include "../actors/actor.h"

namespace Pink {

void SequenceItem::deserialize(Archive &archive) {
    archive >> _actorName >> _actionName;
    if (!dynamic_cast<SequenceItemLeader*>(this) && !dynamic_cast<SequenceItemDefaultAction*>(this))
        debug("\t\tSequenceItem: _actor = %s, _action = %s", _actorName.c_str(), _actionName.c_str());
}

const Common::String &SequenceItem::getActor() const {
    return _actorName;
}

const Common::String &SequenceItem::getAction() const {
    return _actionName;
}

bool SequenceItem::execute(int unk, Sequence *sequence, bool unk2) {
    Actor *actor;
    Action *action;
    if (!(actor = sequence->_sequencer->_page->findActor(_actorName)) ||
        !(action = actor->findAction(_actionName))) {
        return false;
    }

    actor->setAction(action, unk2);
    Common::Array<SequenceActorState> &states = sequence->_context->_states;
    for (int i = 0; i < sequence->_context->_states.size(); ++i) {
        if (states[i]._actorName == _actorName){
            states[i]._unk = unk;
            sequence->_context->_actor = dynamic_cast<SequenceItemLeader*>(this) ?
                                         actor : sequence->_context->_actor;
            // TODO change to virt call
        }
    }

    return true;
}

} // End of namespace Pink