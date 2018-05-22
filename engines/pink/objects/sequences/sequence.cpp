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

#include "common/debug.h"

#include "pink/archive.h"
#include "pink/sound.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/sequences/sequence.h"
#include "pink/objects/sequences/sequence_context.h"
#include "pink/objects/sequences/sequence_item.h"
#include "pink/objects/sequences/sequencer.h"

namespace Pink {

Sequence::Sequence()
    : _unk(0), _context(nullptr),
      _sequencer(nullptr) {}

Sequence::~Sequence() {
    for (uint i = 0; i < _items.size(); ++i) {
        delete _items[i];
    }
}

void Sequence::deserialize(Archive &archive) {
    NamedObject::deserialize(archive);
    _sequencer = static_cast<Sequencer*>(archive.readObject());
    _items.deserialize(archive);
}

void Sequence::toConsole() {
    debug("\t\tSequence %s", _name.c_str());
    debug("\t\t\tItems:");
    for (uint i = 0; i < _items.size(); ++i) {
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

void Sequence::start(int unk) {
    if (_context->_nextItemIndex >= _items.size() || !_items[_context->_nextItemIndex]->execute(_context->_index, this, unk)){
        debug("Sequence %s ended", _name.c_str());
        end();
        return;
    }

    uint i;
    for (i = _context->_nextItemIndex + 1; i <_items.size(); ++i){
        if (_items[i]->isLeader())
            break;
        _items[i]->execute(_context->_index, this, unk);
    }
    _context->_nextItemIndex = i;


    Common::Array<SequenceActorState> &states = _context->_states;
    for (uint j = 0; j < states.size(); ++j) {
        states[j].check(_context->_index, this, unk);
    }
    _context->_index++;
}

void Sequence::update() {
    if (!_context->_actor->isPlaying()){
        debug("Sequence step ended");
        start(0);
    }
}

void Sequence::end() {
    _context->_actor = 0;
    _unk = 1;
    _sequencer->removeContext(_context);
}

void Sequence::restart() {
    _context->setNextItemIndex(0);
    _context->clearActionsFromActorStates();
    start(0);
}

void Sequence::skipToLastSubSequence() {
    if (_unk && _context->getNextItemIndex() < _items.size()){
        int i = _items.size() - 1;
        while(i >= 0 && !_items[--i]->isLeader());
        assert(i >= 0);
        _context->setNextItemIndex(i);
        _context->clearActionsFromActorStates();
        skipItemsTo(i);
        start(0);
    }
}

void Sequence::skipItemsTo(int index) {
    for(int i = 0; i < index; ++i){
        _items[i]->skip(this);
    }
}

void Sequence::skipSubSequence() {
    if (_context->getNextItemIndex() < _context->getSequence()->getItems().size())
        _context->getSequence()->start(0);
}

void SequenceAudio::deserialize(Archive &archive) {
    Sequence::deserialize(archive);
    _soundName = archive.readString();
}

void SequenceAudio::toConsole() {
    debug("\t\tSequenceAudio %s : _sound = %s", _name.c_str(), _soundName.c_str());
    debug("\t\t\tItems:");
    for (uint i = 0; i < _items.size(); ++i) {
        _items[i]->toConsole();
    }
}

void SequenceAudio::start(int unk) {
    Sequence::start(unk);
    uint index = _context->getNextItemIndex();
    if (index < _items.size()) {
        SequenceItemLeaderAudio* leaderAudio = (SequenceItemLeaderAudio*) _items[index];
        _sample = leaderAudio->getSample();
    }
}

void SequenceAudio::end() {
    delete _sound;
    _sound = nullptr;
    Sequence::end();
}

void SequenceAudio::update() {
    if (!_sound->isPlaying())
        end();
    else if (_sample <= _sound->getCurrentSample()){
        start(0);
    }
}

void SequenceAudio::init(int unk) {
    _sample = 0;
    _sound = _sequencer->_page->loadSound(_soundName);
    _sound->play(Audio::Mixer::SoundType::kMusicSoundType, 100, 0);
    Sequence::init(unk);
}

void SequenceAudio::restart() {
    _sound->play(Audio::Mixer::SoundType::kMusicSoundType, 100, 0);
    Sequence::restart();
}

void SequenceAudio::skipToLastSubSequence() {
    end();
}

} // End of namespace Pink
