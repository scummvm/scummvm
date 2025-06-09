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


#ifndef SNATCHER_MEMORY_H
#define SNATCHER_MEMORY_H

#include "common/scummsys.h"

namespace Snatcher {

class SaveLoadManager;
class SnatcherEngine;
class UI;
struct GameState;

// The script engine has opcodes that allow reading and writing into memory, at least for 16-bit addresses.
// We could just create a 64K buffer and let that happen, but eventually we would have to route the values
// to the affected parts of the engine, anyway.

class MemAccessHandler {
public:
	MemAccessHandler(SnatcherEngine *vm, UI *ui, SaveLoadManager *saveMan) : _vm(vm), _ui(ui), _saveMan(saveMan) {}
	~MemAccessHandler() {}

	uint16 readWord(uint16 addr);
	void writeWord(uint16 addr, uint16 val);

	void setGameState(GameState *state);

private:
	SnatcherEngine *_vm;
	UI *_ui;
	SaveLoadManager *_saveMan;
	GameState *_state;
};

} // End of namespace Snatcher

#endif // SNATCHER_MEMORY_H
