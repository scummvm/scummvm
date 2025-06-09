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


#include "snatcher/memory.h"
#include "snatcher/saveload.h"
#include "snatcher/snatcher.h"
#include "snatcher/sound.h"
#include "snatcher/state.h"
#include "snatcher/ui.h"

namespace Snatcher {

uint16 MemAccessHandler::readWord(uint16 addr) {
	int result = 0;

	switch (addr) {
	case 0x7972:
		result = _state->conf.useLightGun ? 1 : 0;
		break;
	case 0x79E0:
		// BURAM status
		return 0;
	case 0x79E2:
	case 0x79E4:
	case 0x79E6:
	case 0x79E8:
		// Save slots
		result = _saveMan->isSaveSlotUsed((addr - 0x79E0) >> 1) ? 1 : 0; // The correct start address is 0x79E2, but we want to skip the autosave slot 0 here.
		break;
	case 0x79F0:
		break;
	case 0x971A:
		break;
	default:
		error("%s(): Unhandled address 0x%04X", __FUNCTION__, addr);
	}

	return result;
}

void MemAccessHandler::writeWord(uint16 addr, uint16 val) {
	switch (addr) {
	case 0x79F0:
		break;
	case 0x971a:
		_ui->setInterpreterMode(val);
		break;
	case 0xFFFF:
		// This is an exception that is caught even in the original code.
		_vm->sound()->reduceVolume2(val);
		break;
	default:
		error("%s(): Unhandled address 0x%04X", __FUNCTION__, addr);
	}
}

void MemAccessHandler::setGameState(GameState *state) {
	_state = state;
}

} // End of namespace Snatcher
