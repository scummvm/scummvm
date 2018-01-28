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

#include "bladerunner/bladerunner.h"
#include "bladerunner/game_info.h"
#include "bladerunner/crimes_database.h"

#include "common/debug.h"

namespace BladeRunner {

ActorClues::ActorClues(BladeRunnerEngine *vm, int cluesType) {
	_vm = vm;
	_count = 0;
	_maxCount = 0;
	switch (cluesType) {
	case 4:
		_maxCount = _vm->_gameInfo->getClueCount();
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
	_clues[clueIndex].flags |= 0x01;
	_clues[clueIndex].flags = (_clues[clueIndex].flags & ~0x02) | ((flag2 << 1) & 0x02);
	_clues[clueIndex].fromActorId = fromActorId;

	debug("Actor acquired clue: \"%s\" from %d", _vm->_crimesDatabase->getClueText(clueId), fromActorId);
}

void ActorClues::lose(int clueId) {
	int clueIndex = findClueIndex(clueId);
	_clues[clueIndex].flags = 0;
}

bool ActorClues::isAcquired(int clueId) const {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return false;
	}
#if BLADERUNNER_DEBUG_GAME
	return true;
#else
	return _clues[clueIndex].flags & 0x01;
#endif
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

int ActorClues::getField1(int clueId) const {
	if (!_count) {
		return 0;
	}

	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return 0;
	}

	return _clues[clueIndex].weight;
}

int ActorClues::getCount() const {
	return _count;
}

void ActorClues::removeAll() {
	_count = 0;
	for (int i = 0; i < _maxCount; ++i) {
		remove(i);
	}
}

int ActorClues::findClueIndex(int clueId) const {
	for (int i = 0; i < _count; i++) {
		if (clueId == _clues[i].clueId) {
			return i;
		}
	}
	return -1;
}

void ActorClues::add(int actorId, int clueId, int weight, bool acquired, bool unknownFlag, int fromActorId) {
	assert(_count < _maxCount);

	//debug("Actor %d added clue: \"%s\" from %d", actorId, _vm->_crimesDatabase->getClueText(clueId), fromActorId);

	_clues[_count].clueId = clueId;
	_clues[_count].weight = weight;

	_clues[_count].flags = 0;
	_clues[_count].flags = (_clues[_count].flags & ~0x01) | (acquired & 0x01);
	_clues[_count].flags = (_clues[_count].flags & ~0x02) | ((unknownFlag << 1) & 0x02);

	_clues[_count].fromActorId = fromActorId;
	++_count;
}

void ActorClues::remove(int index) {
	if (_vm->_crimesDatabase) {
		debug("Actor removed clue: \"%s\"", _vm->_crimesDatabase->getClueText(_clues[index].clueId));
	}

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

bool ActorClues::exists(int clueId) const {
	return findClueIndex(clueId) != -1;
}

} // End of namespace BladeRunner
