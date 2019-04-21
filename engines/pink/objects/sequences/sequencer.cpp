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
#include "pink/objects/actions/action_play_with_sfx.h"
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
	delete _context;
	for (uint i = 0; i < _parrallelContexts.size(); ++i) {
		delete _parrallelContexts[i];
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

void Sequencer::authorSequence(Sequence *sequence, bool loadingSave) {
	if (_context)
		_context->getSequence()->forceEnd();

	if (sequence) {
		SequenceContext *context = new SequenceContext(sequence);
		SequenceContext *confilct;
		while((confilct = findConfilictingContextWith(context)) != nullptr)
			confilct->getSequence()->forceEnd();

		_context = context;
		sequence->init(loadingSave);
		debugC(5, kPinkDebugScripts, "Main Sequence %s started", sequence->getName().c_str());
	}
}

void Sequencer::authorParallelSequence(Sequence *sequence, bool loadingSave) {
	if (_context && _context->getSequence() == sequence)
		return;

	for (uint i = 0; i < _parrallelContexts.size(); ++i) {
		if (_parrallelContexts[i]->getSequence() == sequence)
			return;
	}

	const Common::String leadName = _page->getLeadActor()->getName();
	SequenceContext *context = new SequenceContext(sequence);

	if (!context->findState(leadName) && !findConfilictingContextWith(context)) {
		_parrallelContexts.push_back(context);
		sequence->init(loadingSave);
		debugC(6, kPinkDebugScripts, "Parallel Sequence %s started", sequence->getName().c_str());
	} else
		delete context;
}


void Sequencer::toConsole() {
	debugC(6, kPinkDebugLoadingObjects, "Sequencer:");
	for (uint i = 0; i < _sequences.size(); ++i) {
		_sequences[i]->toConsole();
	}
	for (uint i = 0; i < _timers.size(); ++i) {
		_timers[i]->toConsole();
	}
}

void Sequencer::update() {
	if (_context)
		_context->getSequence()->update();

	for (uint i = 0; i < _parrallelContexts.size(); ++i) {
		_parrallelContexts[i]->getSequence()->update();
	}

	uint time = _page->getGame()->getTotalPlayTime();
	if (time - _time > kTimersUpdateTime) {
		_time = time;
		for (uint i = 0; i < _timers.size(); ++i) {
			_timers[i]->update();
		}
	}
}

void Sequencer::removeContext(SequenceContext *context) {
	if (context == _context) {
		delete _context;
		_context = nullptr;
		return;
	}

	for (uint i = 0; i < _parrallelContexts.size(); ++i) {
		if (context == _parrallelContexts[i]) {
			delete _parrallelContexts[i];
			_parrallelContexts.remove_at(i);
			break;
		}
	}
}

void Sequencer::skipSubSequence() {
	if (_context) {
		g_skipping = true;
		_context->getSequence()->skipSubSequence();
		g_skipping = false;
	}
}

void Sequencer::restartSequence() {
	if (_context) {
		g_skipping = true;
		_context->getSequence()->restart();
		g_skipping = false;
	}
}

void Sequencer::skipSequence() {
	if (_context && _context->getSequence()->isSkippingAllowed()) {
		g_skipping = true;
		_context->getSequence()->skip();
		g_skipping = false;
	}
}

void Sequencer::loadState(Archive &archive) {
	Sequence *sequence = findSequence(archive.readString());
	authorSequence(sequence, 1);

	uint size = archive.readWORD();
	for (uint i = 0; i < size; ++i) {
		sequence = findSequence(archive.readString());
		authorParallelSequence(sequence, 1);
	}
}

void Sequencer::saveState(Archive &archive) {
	Common::String sequenceName;
	if (_context)
		sequenceName = _context->getSequence()->getName();
	archive.writeString(sequenceName);

	archive.writeWORD(_parrallelContexts.size());
	for (uint i = 0; i < _parrallelContexts.size(); ++i) {
		archive.writeString(_parrallelContexts[i]->getSequence()->getName());
	}
}

SequenceContext *Sequencer::findConfilictingContextWith(SequenceContext *context) {
	if (_context && _context->isConflictingWith(context)) {
		return _context;
	}
	for (uint i = 0; i < _parrallelContexts.size(); ++i) {
		if (_parrallelContexts[i]->isConflictingWith(context))
			return _parrallelContexts[i];
	}
	return nullptr;
}

SequenceActorState *Sequencer::findState(const Common::String &name) {
	SequenceActorState *state = nullptr;
	if (_context && (state = _context->findState(name)))
		return state;

	for (uint i = 0; i < _parrallelContexts.size(); ++i) {
		state = _parrallelContexts[i]->findState(name);
		if (state)
			break;
	}
	return state;
}

} // End of namespace Pink
