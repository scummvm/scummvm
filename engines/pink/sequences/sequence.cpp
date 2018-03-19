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
#include "sequence.h"
#include "sequencer.h"
#include "../archive.h"
#include "../items/sequence_item.h"

namespace Pink {

void Sequence::deserialize(Archive &archive) {
    NamedObject::deserialize(archive);
    debug("\tSequence %s", _name.c_str());
    _sequencer = static_cast<Sequencer*>(archive.readObject());
    archive >> _items;
}

Common::Array<SequenceItem*> &Sequence::getItems() {
    return _items;
}

SequenceContext::SequenceContext(Sequence *sequence, Sequencer *sequencer)
    : _sequence(sequence), _sequencer(sequencer)
{
    Common::Array<SequenceItem*> &items = sequence->getItems();
    for (uint i = 0; i < items.size(); ++i) {
        bool found = 0;
        for (uint j = 0; j < _states.size(); ++j) {
            if (items[i]->getActor() == _states[j].getActor()){
                found = 1;
                break;
            }
        }
        if (!found) {
            debug(items[i]->getActor().c_str());
            _states.push_back({items[i]->getActor()});
        }
    }
}

SequenceContext::~SequenceContext() {

}

SequenceActorState::SequenceActorState(const Common::String &name)
    :_actor(name)
{}

const Common::String &SequenceActorState::getActor() const {
    return _actor;
}

} // End of namespace Pink