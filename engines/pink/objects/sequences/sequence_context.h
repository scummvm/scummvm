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

#ifndef PINK_SEQUENCE_CONTEXT_H
#define PINK_SEQUENCE_CONTEXT_H

#include "common/array.h"

namespace Pink {

class Sequence;
class Sequencer;

struct SequenceActorState {
	SequenceActorState(const Common::String &actor)
			: actorName(actor), _segment(0) {}

	void execute(uint segment, Sequence *sequence, bool loadingSave) const;

	Common::String actorName;
	Common::String defaultActionName;
	uint _segment;
};

class Actor;

class SequenceContext {
public:
	SequenceContext(Sequence *sequence);

	void execute(uint nextItemIndex, bool loadingSave);

	bool isConflictingWith(SequenceContext *context);

	void clearDefaultActions();

	SequenceActorState *findState(const Common::String &actor);

	Sequence *getSequence() const { return _sequence; }
	Actor *getActor() const { return _actor; }
	uint getNextItemIndex() const { return _nextItemIndex; }
	uint getSegment() const { return _segment; }

	void setActor(Actor *actor) { _actor = actor; }
	void setNextItemIndex(uint index) { _nextItemIndex = index; }

private:
	Sequence *_sequence;
	Actor *_actor;
	Common::Array<SequenceActorState> _states;
	uint _nextItemIndex;
	uint _segment;
};

}

#endif
