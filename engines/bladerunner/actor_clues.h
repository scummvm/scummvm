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

#ifndef BLADERUNNER_ACTOR_CLUES_H
#define BLADERUNNER_ACTOR_CLUES_H

#include "bladerunner/bladerunner.h"

#include "bladerunner/gameinfo.h"

namespace BladeRunner {

struct ActorClue {
	int _clueId;
	int _weight;
	int _fromActorId;
	int _field3;
	int _field4;
	int _field5;
	int _field6;
	int _field7;
	int _field8;
	unsigned char _flags;
};

class ActorClues {
	BladeRunnerEngine *_vm;

private:
	int _count;
	int _maxCount;
	ActorClue *_clues;

public:
	ActorClues(BladeRunnerEngine *_vm, int cluesType);
	~ActorClues();

	void add(int actorId, int clueId, int unknown, bool acquired, bool unknownFlag, int fromActorId);
	void acquire(int clueId, bool flag2, int fromActorId);
	void lose(int clueId);
	bool isAcquired(int clueId);
	int getFromActorId(int clueId);
	bool isFlag2(int clueId);
	bool isViewed(int clueId);
	bool isFlag4(int clueId);
	int getField1(int clueId);

	int getCount();

	void removeAll();

	//savegame
	//loadgame

private:
	bool exists(int clueId);
	int findClueIndex(int clueId);
	void remove(int clueIndex);
};

} // End of namespace BladeRunner

#endif
