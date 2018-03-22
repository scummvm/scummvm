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
#include "sequence_item.h"
#include "sequence.h"
#include "sequencer.h"
#include "engines/pink/archive.h"
#include "engines/pink/objects/pages/game_page.h"
#include "engines/pink/objects/actors/actor.h"

namespace Pink {

Sequence::Sequence()
    : _unk(0), _context(nullptr),
      _sequencer(nullptr) {}

Sequence::~Sequence() {
    for (int i = 0; i < _items.size(); ++i) {
        delete _items[i];
    }
}

void Sequence::deserialize(Archive &archive) {
    NamedObject::deserialize(archive);
    _sequencer = static_cast<Sequencer*>(archive.readObject());
    archive >> _items;
}

void Sequence::toConsole() {
    debug("\t\tSequence %s", _name.c_str());
    debug("\t\t\tItems:");
    for (int i = 0; i < _items.size(); ++i) {
        _items[i]->toConsole();
    }
}

Common::Array<SequenceItem*> &Sequence::getItems() {
    return _items;
}

void Sequence::setContext(SequenceContext *context) {
    _context = context;
}

void Sequence::init(int unk) {
    assert(_items.size());
    assert(dynamic_cast<SequenceItemLeader*>(_items[0])); // first item must always be a leader
    start(unk);
}

class Action;

void Sequence::start(int unk) {
    if (_context->_nextItemIndex > _items.size()){
        debug("Sequence %s ended", _name);
        //TODO destroy context
        return;
    }

    if (!_items[_context->_nextItemIndex]->execute(_context->_unk, this, unk)){
        //destroy context;
    }

    uint i;
    for (i = _context->_nextItemIndex + 1; i <_items.size(); ++i){
        if (_items[i]->isLeader())
            break;
        _items[i]->execute(_context->_unk, this, unk);
    }
    _context->_nextItemIndex = i;


    Common::Array<SequenceActorState> &states = _context->_states;
    for (uint j = 0; j < states.size(); ++j) {
        if (states[j]._unk != _context->_unk &&
            !states[j]._actionName.empty()) {
            Actor *actor;
            Action *action;
            actor = _sequencer->_page->findActor(states[j]._actorName);
            assert(actor);
            action = actor->findAction(states[j]._actionName);
            assert(action);
            if (actor->getAction() != action)
                actor->setAction(action, unk);
        }
    }
    _context->_unk++;
}

SequenceContext::SequenceContext(Sequence *sequence, Sequencer *sequencer)
    : _sequence(sequence), _sequencer(sequencer),
      _nextItemIndex(0), _unk(1), _actor(nullptr)
{
    sequence->setContext(this);
    Common::Array<SequenceItem*> &items = sequence->getItems();
    debug("SequenceContext for %s", _sequence->getName().c_str());

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
    :_actorName(name), _unk(0)
{}

const Common::String &SequenceActorState::getActor() const {
    return _actorName;
}

} // End of namespace Pink