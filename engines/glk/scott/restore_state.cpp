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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "glk/scott/scott.h"
#include "glk/scott/globals.h"
#include "glk/scott/definitions.h"
#include "glk/scott/restore_state.h"

namespace Glk {
namespace Scott {

#define MAX_UNDOS 100

void saveUndo() {
	if (_G(_justUndid)) {
		_G(_justUndid) = 0;
		return;
	}
	if (_G(_lastUndo) == nullptr) {
		_G(_lastUndo) = saveCurrentState();
		_G(_oldestUndo) = _G(_lastUndo);
		_G(_numberOfUndos) = 1;
		return;
	}

	if (_G(_numberOfUndos) == 0)
		g_scott->fatal("Number of undos == 0 but _G(_lastUndo) != nullptr!");

	_G(_lastUndo)->_nextState = saveCurrentState();
	SavedState *current = _G(_lastUndo)->_nextState;
	current->_previousState = _G(_lastUndo);
	_G(_lastUndo) = current;
	if (_G(_numberOfUndos) == MAX_UNDOS) {
		SavedState *oldest = _G(_oldestUndo);
		_G(_oldestUndo) = _G(_oldestUndo)->_nextState;
		_G(_oldestUndo)->_previousState = nullptr;
		delete[] oldest->_itemLocations;
		delete oldest;
	} else {
		_G(_numberOfUndos)++;
	}
}

void restoreUndo() {
	if (_G(_justStarted)) {
		g_scott->output(_G(_sys)[CANT_UNDO_ON_FIRST_TURN]);
		return;
	}
	if (_G(_lastUndo) == nullptr || _G(_lastUndo)->_previousState == nullptr) {
		g_scott->output(_G(_sys)[NO_UNDO_STATES]);
		return;
	}
	SavedState *current = _G(_lastUndo);
	_G(_lastUndo) = current->_previousState;
	if (_G(_lastUndo)->_previousState == nullptr)
		_G(_oldestUndo) = _G(_lastUndo);
	restoreState(_G(_lastUndo));
	g_scott->output(_G(_sys)[MOVE_UNDONE]);
	delete[] current->_itemLocations;
	delete current;
	_G(_numberOfUndos)--;
	_G(_justUndid) = 1;
}

void ramSave() {
	if (_G(_ramSave) != nullptr) {
		delete[] _G(_ramSave)->_itemLocations;
		delete _G(_ramSave);
	}

	_G(_ramSave) = saveCurrentState();
	g_scott->output(_G(_sys)[STATE_SAVED]);
}

void ramRestore() {
	if (_G(_ramSave) == nullptr) {
		g_scott->output(_G(_sys)[NO_SAVED_STATE]);
		return;
	}

	restoreState(_G(_ramSave));
	g_scott->output(_G(_sys)[STATE_RESTORED]);
	saveUndo();
}

SavedState *saveCurrentState() {
	SavedState *s = new SavedState;
	for (int ct = 0; ct < 16; ct++) {
		s->_counters[ct] = _G(_counters)[ct];
		s->_roomSaved[ct] = _G(_roomSaved)[ct];
	}

	s->_bitFlags = _G(_bitFlags);
	s->_currentLoc = MY_LOC;
	s->_currentCounter = _G(_currentCounter);
	s->_savedRoom = _G(_savedRoom);
	s->_lightTime = _G(_gameHeader)->_lightTime;
	s->_autoInventory = _G(_autoInventory);

	s->_itemLocations = new uint8_t[_G(_gameHeader)->_numItems + 1];

	for (int ct = 0; ct <= _G(_gameHeader)->_numItems; ct++) {
		s->_itemLocations[ct] = _G(_items)[ct]._location;
	}

	s->_previousState = nullptr;
	s->_nextState = nullptr;

	return s;
}

void recoverFromBadRestore(SavedState *state) {
	g_scott->output(_G(_sys)[BAD_DATA]);
	restoreState(state);
	delete state;
}

void restoreState(SavedState *state) {
	for (int ct = 0; ct < 16; ct++) {
		_G(_counters)[ct] = state->_counters[ct];
		_G(_roomSaved)[ct] = state->_roomSaved[ct];
	}

	_G(_bitFlags) = state->_bitFlags;

	MY_LOC = state->_currentLoc;
	_G(_currentCounter) = state->_currentCounter;
	_G(_savedRoom) = state->_savedRoom;
	_G(_gameHeader)->_lightTime = state->_lightTime;
	_G(_autoInventory) = state->_autoInventory;

	for (int ct = 0; ct <= _G(_gameHeader)->_numItems; ct++) {
		_G(_items)[ct]._location = state->_itemLocations[ct];
	}

	_G(_stopTime) = 1;
}

} // End of namespace Scott
} // End of namespace Glk
