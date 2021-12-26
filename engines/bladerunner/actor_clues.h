/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BLADERUNNER_ACTOR_CLUES_H
#define BLADERUNNER_ACTOR_CLUES_H

#include "common/array.h"

namespace BladeRunner {

class BladeRunnerEngine;
class SaveFileReadStream;
class SaveFileWriteStream;

class ActorClues {
	// _vm->_gameInfo->getClueCount()
	static const int kClueCount = 288;

	struct Clue {
		int clueId;
		int weight;
		int fromActorId;
		int field3; // unused (but stored/restored)
		int field4; // Used in Restored Content. Original: unused (but stored/restored)
		int field5; // unused (but stored/restored)
		int field6; // unused (but stored/restored)
		int field7; // unused (but stored/restored)
		int field8; // unused (but stored/restored)
		byte flags; // bit 0 (acquired), bit 1 (unknown), bit 2 (viewed), bit 3 (private)
	};

	BladeRunnerEngine *_vm;

	int                 _count;
	int                 _maxCount;
	Common::Array<Clue> _clues;

public:
	struct CluesUS {
		int clueId;
		int modifier;
	};

public:
	ActorClues(BladeRunnerEngine *_vm, int cluesLimit);

	void add(int actorId, int clueId, int unknown, bool acquired, bool unknownFlag, int fromActorId);
	bool exists(int clueId) const;

	void acquire(int clueId, bool flag2, int fromActorId);
	void lose(int clueId);
	bool isAcquired(int clueId) const;
	int getWeight(int clueId) const;

	int getModifier(int actorId, int otherActorId, int clueId);

	void acquireCluesByRelations(int actorId, int otherActorId);
	int findAcquirableCluesFromActor(int actorId, int targetActorId, CluesUS *list, int size);

	int getFromActorId(int clueId) const;

	bool isFlag2(int clueId) const;

	bool isViewed(int clueId) const;
	void setViewed(int clueId, bool viewed);

	bool isPrivate(int clueId) const;
	void setPrivate(int clueId, bool value);

	// Restored Content method - Checks whether a clue, that McCoy has, was shared with Mainframe
	bool isSharedWithMainframe(int clueId) const;
	// Restored Content method - Marks a clue, that McCoy has, as shared with Mainframe
	void setSharedWithMainframe(int clueId, bool value);

	int getCount() const;
	int getClueIdByIndex(int index) const;

	void removeAll();

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);

private:
	int findClueIndex(int clueId) const;
	void remove(int clueIndex);
};

} // End of namespace BladeRunner

#endif
