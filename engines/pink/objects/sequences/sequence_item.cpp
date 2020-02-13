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
#include "pink/objects/actions/action.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/sequences/sequence_item.h"
#include "pink/objects/sequences/sequence.h"
#include "pink/objects/sequences/sequencer.h"
#include "pink/objects/sequences/sequence_context.h"

namespace Pink {

void SequenceItem::deserialize(Archive &archive) {
	_actor = archive.readString();
	_action = archive.readString();
}

void SequenceItem::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\t\t\tSequenceItem: _actor=%s, _action=%s", _actor.c_str(), _action.c_str());
}

bool SequenceItem::execute(uint segment, Sequence *sequence, bool loadingSave) {
	Actor *actor = sequence->getSequencer()->getPage()->findActor(_actor);
	Action *action;
	if (!actor || !(action = actor->findAction(_action)))
		return false;

	actor->setAction(action, loadingSave);

	SequenceContext *context = sequence->getContext();
	SequenceActorState *state = context->findState(_actor);
	if (state)
		state->_segment = segment;
	if (isLeader())
		context->setActor(actor);
	return true;
}

bool SequenceItem::isLeader() {
	return false;
}

bool SequenceItemLeader::isLeader() {
	return true;
}

void SequenceItemLeader::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\t\t\tSequenceItemLeader: _actor=%s, _action=%s", _actor.c_str(), _action.c_str());
}


void SequenceItemLeaderAudio::deserialize(Archive &archive) {
	SequenceItem::deserialize(archive);
	_sample = archive.readDWORD();
}

void SequenceItemLeaderAudio::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\t\t\tSequenceItemLeaderAudio: _actor=%s, _action=%s _sample=%d", _actor.c_str(), _action.c_str(), _sample);
}

bool SequenceItemDefaultAction::execute(uint segment, Sequence *sequence, bool loadingSave) {
	SequenceActorState *state = sequence->getContext()->findState(_actor);
	if (state)
		state->defaultActionName = _action;
	return true;
}

void SequenceItemDefaultAction::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\t\t\tSequenceItemDefaultAction: _actor=%s, _action=%s", _actor.c_str(), _action.c_str());
}

void SequenceItemDefaultAction::skip(Sequence *sequence) {
	execute(0, sequence, 1);
}

} // End of namespace Pink
