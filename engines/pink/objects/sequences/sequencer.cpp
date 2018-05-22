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
#include "pink/pink.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/sequences/sequencer.h"
#include "pink/objects/sequences/sequence.h"
#include "pink/objects/sequences/sequence_context.h"
#include "pink/objects/sequences/seq_timer.h"

namespace Pink {

Sequencer::Sequencer(GamePage *page)
    : _context(nullptr), _page(page), _time(0)
{}

Sequencer::~Sequencer() {
    for (uint i = 0; i < _sequences.size(); ++i) {
        delete _sequences[i];
    }
}

void Sequencer::deserialize(Archive &archive) {
    _sequences.deserialize(archive);
    _timers.deserialize(archive);
}

Sequence *Sequencer::findSequence(const Common::String &name) {
	for (uint i = 0; i < _sequences.size(); ++i) {
		if (_sequences[i]->getName() == name)
			return _sequences[i];
	}
	return nullptr;
}

void Sequencer::authorSequence(Sequence *sequence, bool unk) {
    if (_context){

    }

    if (sequence){
        _context = new SequenceContext(sequence, this);
        //unload array of unknown objects
        _currentSequenceName = sequence->getName();
        sequence->init(unk);
    }
    else _currentSequenceName.clear();
}

void Sequencer::toConsole() {
    debug("Sequencer:");
    for (uint i = 0; i < _sequences.size(); ++i) {
        _sequences[i]->toConsole();
    }
    for (uint i = 0; i < _timers.size(); ++i) {
        _timers[i]->toConsole();
    }
}

void Sequencer::update() {
    if (_context)
        _context->_sequence->update();
    updateTimers();
}

void Sequencer::removeContext(SequenceContext *context) {
    delete _context;
    _context = nullptr;
}

void Sequencer::skipSubSequence() {
    if (_context)
        _context->getSequence()->skipSubSequence();
}

void Sequencer::restartSequence() {
    _context->getSequence()->restart();
}

void Sequencer::skipToLastSubSequence() {
    _context->getSequence()->skipToLastSubSequence();
}

void Sequencer::updateTimers() {
    uint time = _page->getGame()->getTotalPlayTime();
    if (time - _time <= kTimersUpdateTime) {
        return;
    }

    _time = time;
    for (uint i = 0; i < _timers.size(); ++i) {
        _timers[i]->update();
    }
}

SequenceActorState *Sequencer::findSequenceActorState(const Common::String &name) {
    if (!_context)
        return nullptr;

    for (uint i = 0; i < _context->_states.size(); ++i) {
       if (_context->_states[i].getActor() == name)
           return &_context->_states[i];
    }

    return nullptr;
}

} // End of namespace Pink
