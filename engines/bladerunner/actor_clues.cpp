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

#include "bladerunner/crimes_database.h"

#include "common/debug.h"

namespace BladeRunner {

ActorClues::ActorClues(BladeRunnerEngine *vm, int cluesType) {
	_vm = vm;
	_count = 0;
	_maxCount = 0;
	_clues = 0;
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
		_clues = new ActorClue[_maxCount];
	} else {
		_clues = nullptr;
	}

	if (_clues) {
		removeAll();
	} else {
		_maxCount = 0;
	}
}

ActorClues::~ActorClues() {
	delete[] _clues;

	_maxCount = 0;
	_count = 0;
}

void ActorClues::acquire(int clueId, bool flag2, int fromActorId) {
	int clueIndex = findClueIndex(clueId);
	_clues[clueIndex]._flags |= 0x01;
	_clues[clueIndex]._flags = (_clues[clueIndex]._flags & ~0x02) | ((flag2 << 1) & 0x02);
	_clues[clueIndex]._fromActorId = fromActorId;

	debug("Actor acquired clue: \"%s\" from %d", _vm->_crimesDatabase->getClueText(clueId), fromActorId);
}

void ActorClues::lose(int clueId) {
	int clueIndex = findClueIndex(clueId);
	_clues[clueIndex]._flags = 0;
}

bool ActorClues::isAcquired(int clueId) {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return false;
	}
	return _clues[clueIndex]._flags & 0x01;
}

int ActorClues::getFromActorId(int clueId) {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return  -1;
	}

	return _clues[clueIndex]._fromActorId;
}

bool ActorClues::isFlag2(int clueId) {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return false;
	}

	return (_clues[clueIndex]._flags & 0x02) >> 1;
}

bool ActorClues::isViewed(int clueId) {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return false;
	}

	return (_clues[clueIndex]._flags & 0x04) >> 2;
}

bool ActorClues::isFlag4(int clueId) {
	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return false;
	}

	return (_clues[clueIndex]._flags & 0x08) >> 3;
}

int ActorClues::getField1(int clueId) {
	if (!_count) {
		return 0;
	}

	int clueIndex = findClueIndex(clueId);
	if (clueIndex == -1) {
		return 0;
	}

	return _clues[clueIndex]._weight;
}

int ActorClues::getCount() {
	return _count;
}

void ActorClues::removeAll() {
	_count = 0;
	for (int i = 0; i < _maxCount; ++i) {
		remove(i);
	}
}

int ActorClues::findClueIndex(int clueId) {
	for (int i = 0; i < _count; i++) {
		if (clueId == _clues[i]._clueId) {
			return i;
		}
	}
	return -1;
}

void ActorClues::add(int actorId, int clueId, int weight, bool acquired, bool unknownFlag, int fromActorId) {
	assert(_count < _maxCount);

	//debug("Actor %d added clue: \"%s\" from %d", actorId, _vm->_crimesDatabase->getClueText(clueId), fromActorId);

	_clues[_count]._clueId = clueId;
	_clues[_count]._weight = weight;

	_clues[_count]._flags = 0;
	_clues[_count]._flags = (_clues[_count]._flags & ~0x01) | (acquired & 0x01);
	_clues[_count]._flags = (_clues[_count]._flags & ~0x02) | ((unknownFlag << 1) & 0x02);

	_clues[_count]._fromActorId = fromActorId;
	++_count;
}

void ActorClues::remove(int index) {
	if (_vm->_crimesDatabase)
		debug("Actor removed clue: \"%s\"", _vm->_crimesDatabase->getClueText(_clues[index]._clueId));

	_clues[index]._clueId = -1;
	_clues[index]._weight = 0;
	_clues[index]._flags = 0;
	_clues[index]._fromActorId = -1;

	_clues[index]._field3 = -1;
	_clues[index]._field4 = 0;
	_clues[index]._field5 = -1;
	_clues[index]._field6 = 0;
	_clues[index]._field7 = -1;
	_clues[index]._field8 = 0;
}

bool ActorClues::exists(int clueId) {
	return findClueIndex(clueId) != -1;
}

} // End of namespace BladeRunner
