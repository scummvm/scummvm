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

#include "bladerunner/actor_clues.h"
#include "bladerunner/actor.h"
#include "bladerunner/script/ai_script.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/game_info.h"
#include "bladerunner/crimes_database.h"
#include "bladerunner/savefile.h"

#include "common/debug.h"

namespace BladeRunner {

ActorClues::ActorClues(BladeRunnerEngine *vm, int cluesLimit) {
	_vm = vm;
	_count = 0;
	_maxCount = 0;
	switch (cluesLimit) {
	case 4:
		_maxCount = kClueCount;
		break;
	case 3:
		_maxCount = 100;
		break;
	case 2:
		_maxCount = 50;
		break;
	case 1:
		_maxCount = 25;
		break;
	case 0:
		_maxCount = 0;
		break;
	default:
		return;
	}

	if (_maxCount > 0) {
		_clues.resize(_maxCount);
	}

	removeAll();
}

void ActorClues::acquire(int clueId, bool flag2, int fromActorId) {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) { // prevent assertion fault
	//	debug("Actor could not acquire clue: \"%s\" from %d", _vm->_crimesDatabase->getClueText(clueId), fromActorId);
		return;
	} else {
		_clues[clueIndex].flags |= 0x01;
		_clues[clueIndex].flags = (_clues[clueIndex].flags & ~0x02) | (((flag2? 1:0) << 1) & 0x02);
		_clues[clueIndex].fromActorId = fromActorId;
	// debug("Actor acquired clue: \"%s\" from %d", _vm->_crimesDatabase->getClueText(clueId), fromActorId);
	}
}

void ActorClues::lose(int clueId) {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) { // prevent assertion fault
	//	debug("Actor could not lose clue: \"%s\"", _vm->_crimesDatabase->getClueText(clueId));
		return;
	} else {
		_clues[clueIndex].flags = 0;
	}
}

bool ActorClues::isAcquired(int clueId) const {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return false;
	}
	return _clues[clueIndex].flags & 0x01;
}

int ActorClues::getWeight(int clueId) const {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return 0;
	}
	return _clues[clueIndex].weight;
}

int ActorClues::getModifier(int actorId, int otherActorId, int clueId) {
	Actor *actor = _vm->_actors[actorId];
	Actor *otherActor = _vm->_actors[otherActorId];
	int modifier1, modifier2, modifier3, modifier4;

	int friendliness = actor->getFriendlinessToOther(otherActorId);
	int clueWeight = otherActor->_clues->getWeight(clueId);

	if (actor->_clues->isFlag2(clueId)) {
		modifier1 = 100 - actor->getHonesty() - friendliness;
	} else {
		modifier1 = 0;
	}

	modifier2 = 0;
	for (int i = 0; i < (int)_vm->_gameInfo->getActorCount(); ++i) {
		if (i != actorId && i != otherActorId) {
			modifier2 += (friendliness - 50) * _vm->_aiScripts->callGetFriendlinessModifierIfGetsClue(i, otherActorId, clueId) / 100;
		}
	}

	modifier3 = _vm->_aiScripts->callGetFriendlinessModifierIfGetsClue(otherActorId, actorId, clueId);

	modifier4 = _vm->_rnd.getRandomNumberRng(0, (100 - actor->getIntelligence()) / 10);

	if (_vm->_rnd.getRandomNumberRng(0, 1) == 1) {
		modifier4 = -modifier4;
	}

	return modifier1 + modifier2 + modifier3 + modifier4 + clueWeight;
}

static int cluesCompare(const void *p1, const void *p2) {
	const ActorClues::CluesUS *clue1 = (const ActorClues::CluesUS *)p1;
	const ActorClues::CluesUS *clue2 = (const ActorClues::CluesUS *)p2;

	if (clue1->modifier > clue2->modifier)
		return -1;

	return (clue1->modifier < clue2->modifier);
}

void ActorClues::acquireCluesByRelations(int actorId, int otherActorId) {
	CluesUS clues1[kClueCount], clues2[kClueCount];

	int count1 = findAcquirableCluesFromActor(actorId, otherActorId, clues1, kClueCount);
	int count2 = findAcquirableCluesFromActor(otherActorId, actorId, clues2, kClueCount);

	if (count1 || count2) {
		for (int i = 0; i < count1; ++i) {
			clues1[i].modifier = getModifier(actorId, otherActorId, clues1[i].clueId);
		}
		qsort(clues1, count1, sizeof(CluesUS), cluesCompare);

		for (int i = 0; i < count2; ++i) {
			clues2[i].modifier = getModifier(otherActorId, actorId, clues2[i].clueId);
		}
		qsort(clues2, count2, sizeof(CluesUS), cluesCompare);

		Actor *actor = _vm->_actors[actorId];
		Actor *otherActor = _vm->_actors[otherActorId];

		uint avgParameters = (otherActor->getHonesty() + otherActor->getIntelligence() + actor->getFriendlinessToOther(otherActorId)) / 3;
		int clue1count = avgParameters * count1 / 100;

		if (avgParameters >= 50 && clue1count == 0 && count1 == 1) {
			clue1count = 1;
		}

		avgParameters = (actor->getHonesty() + actor->getIntelligence() + otherActor->getFriendlinessToOther(actorId)) / 3;
		int clue2count = avgParameters * count2 / 100;

		if (avgParameters >= 50 && clue2count == 0 && count2 == 1) {
			clue2count = 1;
		}

		for (int i = 0; i < clue2count; ++i) {
			bool flag = false;
			if (otherActor->_clues->isFlag2(clues2[i].clueId)) {
				avgParameters = (2 * otherActor->getFriendlinessToOther(actorId) + otherActor->getHonesty()) / 3;

				if (avgParameters > 70) {
					avgParameters = 100;
				} else if (avgParameters < 30) {
					avgParameters = 0;
				}
				if (_vm->_rnd.getRandomNumberRng(1, 100) <= avgParameters) {
					flag = true;
				}
			}

			actor->_clues->acquire(clues2[i].clueId, flag, otherActorId);
		}

		for (int i = 0; i < clue1count; ++i) {
			bool flag = false;
			if (actor->_clues->isFlag2(clues1[i].clueId)) {
				avgParameters = (2 * actor->getFriendlinessToOther(otherActorId) + actor->getHonesty()) / 3;

				if (avgParameters > 70) {
					avgParameters = 100;
				} else if (avgParameters < 30) {
					avgParameters = 0;
				}
				if (_vm->_rnd.getRandomNumberRng(1, 100) <= avgParameters) {
					flag = true;
				}
			}

			otherActor->_clues->acquire(clues1[i].clueId, flag, actorId);
		}
	}
}

int ActorClues::findAcquirableCluesFromActor(int actorId, int targetActorId, ActorClues::CluesUS *list, int size) {
	Actor *actor = _vm->_actors[actorId];
	Actor *otherActor = _vm->_actors[targetActorId];
	int count = 0;
	int cluesCount = actor->_clues->getCount();

	for (int i = 0; i < cluesCount; ++i) 	{
		int clueId = actor->_clues->getClueIdByIndex(i);

		if (actor->_clues->isAcquired(clueId)
				&& otherActor->_clues->getWeight(clueId) > 0
				&& !otherActor->_clues->isAcquired(clueId)) {
			list[count].clueId = clueId;
			list[count].modifier = 0;

			++count;
		}
	}

	return count;
}

int ActorClues::getFromActorId(int clueId) const {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return  -1;
	}

	return _clues[clueIndex].fromActorId;
}

bool ActorClues::isFlag2(int clueId) const {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return false;
	}

	return _clues[clueIndex].flags & 0x02;
}

bool ActorClues::isViewed(int clueId) const {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return false;
	}

	return _clues[clueIndex].flags & 0x04;
}

void ActorClues::setViewed(int clueId, bool viewed) {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return;
	}

	if (viewed) {
		_clues[clueIndex].flags |= 0x04;
	} else {
		_clues[clueIndex].flags &= ~0x04;
	}
}

bool ActorClues::isPrivate(int clueId) const {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return false;
	}

	return _clues[clueIndex].flags & 0x08;
}

void ActorClues::setPrivate(int clueId, bool value) {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return;
	}

	if (value) {
		_clues[clueIndex].flags |= 0x08;
	} else {
		_clues[clueIndex].flags &= ~0x08;
	}
}

int ActorClues::getCount() const {
	return _count;
}

int ActorClues::getClueIdByIndex(int index) const {
	assert(index < _count);

	if (index < 0 || index >= _count) {
		return -1;
	}
	return _clues[index].clueId;
}

void ActorClues::removeAll() {
	_count = 0;
	for (int i = 0; i < _maxCount; ++i) {
		remove(i);
	}
}

int ActorClues::findClueIndex(int clueId) const {
	for (int i = 0; i < _count; ++i) {
		if (clueId == _clues[i].clueId) {
			return i;
		}
	}
	return -1;
}

void ActorClues::add(int actorId, int clueId, int weight, bool acquired, bool unknownFlag, int fromActorId) {
	assert(_count < _maxCount);

	_clues[_count].clueId = clueId;
	_clues[_count].weight = weight;

	_clues[_count].flags = 0;
	_clues[_count].flags = (_clues[_count].flags & ~0x01) | ((acquired? 1:0) & 0x01);
	_clues[_count].flags = (_clues[_count].flags & ~0x02) | (((unknownFlag? 1:0) << 1) & 0x02);

	_clues[_count].fromActorId = fromActorId;
	++_count;
}

bool ActorClues::exists(int clueId) const {
	return findClueIndex(clueId) != -1;
}

void ActorClues::remove(int index) {
	// if (_vm->_crimesDatabase) {
	// 	debug("Actor removed clue: \"%s\"", _vm->_crimesDatabase->getClueText(_clues[index].clueId));
	// }

	_clues[index].clueId      = -1;
	_clues[index].weight      = 0;
	_clues[index].flags       = 0;
	_clues[index].fromActorId = -1;

	_clues[index].field3 = -1;
	_clues[index].field4 = 0;
	_clues[index].field5 = -1;
	_clues[index].field6 = 0;
	_clues[index].field7 = -1;
	_clues[index].field8 = 0;
}

void ActorClues::save(SaveFileWriteStream &f) {
	f.writeInt(_count);
	f.writeInt(_maxCount);
	for (int i = 0; i < _maxCount; ++i) {
		Clue &c = _clues[i];
		f.writeInt(c.clueId);
		f.writeInt(c.weight);
		f.writeInt(c.fromActorId);
		f.writeInt(c.field3);
		f.writeInt(c.field4);
		f.writeInt(c.field5);
		f.writeInt(c.field6);
		f.writeInt(c.field7);
		f.writeInt(c.field8);
		f.writeByte(c.flags);
	}
}

void ActorClues::load(SaveFileReadStream &f) {
	_count = f.readInt();
	_maxCount = f.readInt();
	_clues.clear();
	_clues.resize(_maxCount);
	for (int i = 0; i < _maxCount; ++i) {
		Clue &c = _clues[i];
		c.clueId = f.readInt();
		c.weight = f.readInt();
		c.fromActorId = f.readInt();
		c.field3 = f.readInt();
		c.field4 = f.readInt();
		c.field5 = f.readInt();
		c.field6 = f.readInt();
		c.field7 = f.readInt();
		c.field8 = f.readInt();
		c.flags = f.readByte();
	}
}

} // End of namespace BladeRunner
