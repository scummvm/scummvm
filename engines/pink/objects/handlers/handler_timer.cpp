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
#include "pink/objects/side_effect.h"
#include "pink/objects/condition.h"
#include "pink/objects/actors/lead_actor.h"
#include "pink/objects/handlers/handler_timer.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/sequences/sequence.h"
#include "pink/objects/sequences/sequencer.h"

namespace Pink {

void HandlerTimerActions::deserialize(Archive &archive) {
	Handler::deserialize(archive);
	_actions.deserialize(archive);
}

void HandlerTimerActions::toConsole() {
	debugC(6, kPinkDebugLoadingObjects, "HandlerTimerActions:");

	debugC(6, kPinkDebugLoadingObjects, "\tSideEffects:");
	for (uint i = 0; i < _sideEffects.size(); ++i) {
		_sideEffects[i]->toConsole();
	}

	debugC(6, kPinkDebugLoadingObjects, "\tConditions:");
	for (uint i = 0; i < _conditions.size(); ++i) {
		_conditions[i]->toConsole();
	}

	debugC(6, kPinkDebugLoadingObjects, "\tActions:");
	for (uint i = 0; i < _actions.size(); ++i) {
		debugC(6, kPinkDebugLoadingObjects, "\t\t%s", _actions[i].c_str());
	}
}

void HandlerTimerActions::handle(Actor *actor) {
	Handler::handle(actor);
	if (!actor->isPlaying() && !_actions.empty()) {
		Common::RandomSource &rnd = actor->getPage()->getGame()->getRnd();
		uint index = rnd.getRandomNumber(_actions.size() - 1);
		Action *action = actor->findAction(_actions[index]);
		assert(action);
		actor->setAction(action);
	}
}

void HandlerTimerSequences::toConsole() {
	debugC(6, kPinkDebugLoadingObjects, "HandlerTimerSequences:");

	debugC(6, kPinkDebugLoadingObjects, "\tSideEffects:");
	for (uint i = 0; i < _sideEffects.size(); ++i) {
		_sideEffects[i]->toConsole();
	}

	debugC(6, kPinkDebugLoadingObjects, "\tConditions:");
	for (uint i = 0; i < _conditions.size(); ++i) {
		_conditions[i]->toConsole();
	}

	debugC(6, kPinkDebugLoadingObjects, "\tSequences:");
	for (uint i = 0; i < _sequences.size(); ++i) {
		debugC(6, kPinkDebugLoadingObjects, "\t\t%s", _sequences[i].c_str());
	}
}

void HandlerTimerSequences::handle(Actor *actor) {
	Handler::handle(actor);
	Sequencer *sequencer = actor->getPage()->getSequencer();

	assert(!_sequences.empty());

	Common::RandomSource &rnd = actor->getPage()->getGame()->getRnd();
	uint index = rnd.getRandomNumber(_sequences.size() - 1);

	Sequence *sequence = sequencer->findSequence(_sequences[index]);

	assert(sequence);

	sequencer->authorParallelSequence(sequence, 0);
}

} // End of namespace Pink
