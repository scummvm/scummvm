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

#include "pink/pink.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/sequences/sequence.h"
#include "pink/objects/sequences/sequence_context.h"
#include "pink/objects/sequences/sequencer.h"

namespace Pink {

void SequenceActorState::execute(uint segment, Sequence *sequence, bool loadingSave) const {
	Actor *actor = sequence->getSequencer()->getPage()->findActor(this->actorName);
	if (actor && _segment != segment && !defaultActionName.empty()) {
		Action *action = actor->findAction(defaultActionName);
		if (action && actor->getAction() != action) {
			actor->setAction(action, loadingSave);
		}
	}
}

SequenceContext::SequenceContext(Sequence *sequence)
		: _sequence(sequence), _nextItemIndex(0),
		  _segment(1), _actor(nullptr) {
	sequence->setContext(this);
	Common::Array<SequenceItem *> &items = sequence->getItems();
	debug(kPinkDebugScripts, "SequenceContext for %s", _sequence->getName().c_str());

	for (uint i = 0; i < items.size(); ++i) {
		bool found = 0;
		for (uint j = 0; j < _states.size(); ++j) {
			if (items[i]->getActor() == _states[j].actorName) {
				found = 1;
				break;
			}
		}
		if (!found) {
			debug(kPinkDebugScripts, "%s", items[i]->getActor().c_str());
			_states.push_back(SequenceActorState(items[i]->getActor()));
		}
	}
}

void SequenceContext::execute(uint nextItemIndex, bool loadingSave) {
	for (uint j = 0; j < _states.size(); ++j) {
		_states[j].execute(_segment, _sequence, loadingSave);
	}

	_nextItemIndex = nextItemIndex;
	_segment++;
}


void SequenceContext::clearDefaultActions() {
	for (uint i = 0; i < _states.size(); ++i) {
		_states[i].defaultActionName.clear();
	}
}

SequenceActorState *SequenceContext::findState(const Common::String &actor) {
	for (uint i = 0; i < _states.size(); ++i) {
		if (_states[i].actorName == actor)
			return &_states[i];
	}
	return nullptr;
}

bool SequenceContext::isConflictingWith(SequenceContext *context) {
	for (uint i = 0; i < _states.size(); ++i) {
		if (context->findState(_states[i].actorName))
			return true;
	}
	return false;
}

} // End of namespace Pink
