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
#include "pink/objects/actors/lead_actor.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/sequences/sequencer.h"
#include "pink/objects/sequences/sequence.h"
#include "pink/objects/sequences/sequence_context.h"
#include "pink/objects/sequences/seq_timer.h"

namespace Pink {

Sequencer::Sequencer(GamePage *page)
	: _context(nullptr), _page(page), _time(0) {}

Sequencer::~Sequencer() {
	for (uint i = 0; i < _sequences.size(); ++i) {
		delete _sequences[i];
	}
	for (uint i = 0; i < _timers.size(); ++i) {
		delete _timers[i];
	}
	removeContext(_context);
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
	assert(!_context);

	if (sequence) {
		SequenceContext *context = new SequenceContext(sequence, this);

		SequenceContext *confilct;
		while(confilct = isContextConflicts(context))
			confilct->_sequence->end();

		_context = context;
		_currentSequenceName = sequence->getName();
		sequence->init(unk);
	}
}

void Sequencer::authorParallelSequence(Sequence *seqeunce, bool unk) {
	if (_context && _context->_sequence == seqeunce)
		return;

	for (uint i = 0; i < _parrallelContexts.size(); ++i) {
		if (_parrallelContexts[i]->_sequence == seqeunce)
			return;
	}

	const Common::String leadName = _page->getLeadActor()->getName();

	SequenceContext *context = new SequenceContext(seqeunce, this);

	for (uint i = 0; i < context->_states.size(); ++i) {
		if (context->_states[i].getActor() == leadName) {
			delete context;
			return;
		}
	}

	for (uint i = 0; i < context->_states.size(); ++i) {
		if (findMainSequenceActorState(context->_states[i].getActor())) {
			delete context;
			return;
		}
	}

	for (uint i = 0; i < context->_states.size(); ++i) {
		if (findParralelSequenceActorState(context->_states[i].getActor())) {
			delete context;
			return;
		}
	}

	_parrallelContexts.push_back(context);
	seqeunce->start(unk);
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

	for (uint i = 0; i < _parrallelContexts.size(); ++i) {
		_parrallelContexts[i]->_sequence->update();
	}

	updateTimers();
}

void Sequencer::removeContext(SequenceContext *context) {
	if (context == _context) {
		delete _context;
		_context = nullptr;
		return;
	}

	for (uint i = 0; i < _parrallelContexts.size(); ++i) {
		if (context == _parrallelContexts[i]->_sequence->_context) {
			_parrallelContexts.remove_at(i);
			break;
		}
	}
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

SequenceActorState *Sequencer::findMainSequenceActorState(const Common::String &name) {
	if (!_context)
		return nullptr;

	for (uint i = 0; i < _context->_states.size(); ++i) {
	   if (_context->_states[i].getActor() == name)
		   return &_context->_states[i];
	}

	return nullptr;
}

SequenceActorState *Sequencer::findParralelSequenceActorState(const Common::String &name) {
	for (uint i = 0; i < _parrallelContexts.size(); ++i) {
		for (uint j = 0; j < _parrallelContexts[i]->_states.size(); ++j) {
			if (_parrallelContexts[i]->_states[j].getActor() == name)
				return &_parrallelContexts[i]->_states[j];
		}
	}

	return nullptr;
}


void Sequencer::loadState(Archive &archive) {
	Sequence *sequence = findSequence(archive.readString());
	authorSequence(sequence, 1);
}

void Sequencer::saveState(Archive &archive) {
	Common::String sequenceName;
	if (_context)
		sequenceName = _context->_sequence->getName();
	archive.writeString(sequenceName);
	// add pokus specific
}

SequenceContext * Sequencer::isContextConflicts(SequenceContext *context) {
	for (uint i = 0; i < _parrallelContexts.size(); ++i) {
		for (uint j = 0; j < _parrallelContexts[i]->_states.size(); ++j) {
			for (int k = 0; k < context->_states.size(); ++k) {
				if (_parrallelContexts[i]->_states[j].getActor() == context->_states[k].getActor())
					return _parrallelContexts[i];
			}
		}
	}
	return nullptr;
}

} // End of namespace Pink
