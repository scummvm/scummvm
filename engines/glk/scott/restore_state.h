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

#ifndef GLK_SCOTT_RESTORESTATE_H
#define GLK_SCOTT_RESTORESTATE_H

#include "glk/scott/types.h"

namespace Glk {
namespace Scott {

struct SavedState {
	int _counters[16];
	int _roomSaved[16];
	uint32_t _bitFlags = 0;
	int _currentLoc = 0;
	int _currentCounter = 0;
	int _savedRoom = 0;
	int _lightTime = 0;
	int _autoInventory = 0;
	uint8_t *_itemLocations = nullptr;
	SavedState *_previousState = nullptr;
	SavedState *_nextState = nullptr;
};

void saveUndo();
void restoreUndo();
void ramSave();
void ramRestore();
SavedState *saveCurrentState();
void recoverFromBadRestore(SavedState *state);
void restoreState(SavedState *state);

} // End of namespace Scott
} // End of namespace Glk

#endif
