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

#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/room.h"
#include "chewy/rooms/room70.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room70::entry(int16 eib_nr) {
	g_engine->_sound->playSound(0, 0);
	g_engine->_sound->playSound(0);
	_G(gameState).ScrollxStep = 2;
	_G(gameState).ZoomXy[P_HOWARD][0] = 70;
	_G(gameState).ZoomXy[P_HOWARD][1] = 100;
	_G(gameState).ZoomXy[P_NICHELLE][0] = 70;
	_G(gameState).ZoomXy[P_NICHELLE][1] = 100;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(zoom_horizont) = 110;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(SetUpScreenFunc) = setup_func;

	if (_G(flags).LoadGame)
		return;

	switch (eib_nr) {
	case 116:
		leftEntry();
		break;
	case 117:
		rightEntry();
		break;
	default:
		_G(gameState).scrollx = 62;
		topEntry();
		break;
	}
}

void Room70::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;

	if (_G(gameState)._personRoomNr[P_HOWARD] == 70) {
		if (eib_nr == 102) {
			_G(gameState)._personRoomNr[P_HOWARD] = 69;
			_G(gameState)._personRoomNr[P_NICHELLE] = 69;
		} else if (eib_nr == 103 || eib_nr == 104) {
			_G(gameState)._personRoomNr[P_HOWARD] = 75;
			_G(gameState)._personRoomNr[P_NICHELLE] = 75;
		}
	}
}

} // namespace Rooms
} // namespace Chewy
