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
#include "chewy/rooms/room80.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room80::entry() {
	_G(mouseLeftClick) = false;
	_G(gameState).scrollx = 0;
	_G(gameState).scrolly = 0;
	g_engine->_sound->playSound(0, 0);
	g_engine->_sound->playSound(0, 1);
	g_engine->_sound->playSound(0, 2);
	g_engine->_sound->playSound(0);
	g_engine->_sound->playSound(0, 1, false);
	g_engine->_sound->playSound(0, 2, false);

	if (_G(gameState).gottenDiary) {
		_G(atds)->delControlBit(476, ATS_ACTIVE_BIT);
	} else {
		_G(atds)->setControlBit(476, ATS_ACTIVE_BIT);
	}

	if (_G(gameState).flags32_1) {
		_G(gameState).scrollx = 39;
		return;
	}

	setPersonPos(37, 10, P_CHEWY, P_RIGHT);
	setPersonPos(22, -1, P_HOWARD, P_RIGHT);
	setPersonPos(6, 2, P_NICHELLE, P_RIGHT);
	_G(gameState).scrollx = 10;
	_G(flags).NoScroll = true;
	_G(gameState).ZoomXy[P_HOWARD][0] = 24;
	_G(gameState).ZoomXy[P_HOWARD][1] = 40;
	_G(gameState).ZoomXy[P_NICHELLE][0] = 24;
	_G(gameState).ZoomXy[P_NICHELLE][1] = 40;
	_G(zoom_horizont) = 0;

	if (_G(gameState).r88DestRoom == 84)
		_G(det)->showStaticSpr(3);
	else
		_G(det)->showStaticSpr(4);

	_G(SetUpScreenFunc) = setup_func;
}

void Room80::setup_func() {
	for (int i = 0; i < 3; ++i)
		_G(det)->hideStaticSpr(i);

	if (_G(gameState).flags32_1 || !_G(flags).ShowAtsInvTxt || _G(menu_display))
		return;

	_G(menu_item) = CUR_USE;
	cur_2_inventory();
	cursorChoice(CUR_POINT);
	int vec = _G(det)->maus_vector(_G(gameState).scrollx + g_events->_mousePos.x, g_events->_mousePos.y);
	if (vec == -1)
		return;

	if (vec != 0 && vec != 2) {
		if (vec != 1 || !_G(gameState).gottenDiary)
			return;
	}

	_G(det)->showStaticSpr(vec);
	if (!_G(mouseLeftClick))
		return;

	int nextRoom;
	switch (vec) {
	case 0:
		nextRoom = 82;
		break;
	case 1:
		if (_G(gameState).R88UsedMonkey)
			nextRoom = 85;
		else
			nextRoom = 84;
		break;
	case 2:
		nextRoom = 81;
		break;
	default:
		nextRoom = -1;
		break;
	}

	if (nextRoom == -1)
		return;

	_G(SetUpScreenFunc) = nullptr;
	_G(det)->hideStaticSpr(vec);
	_G(menu_item) = CUR_WALK;
	cursorChoice(CUR_WALK);
	_G(gameState).flags30_1 = true;
	_G(mouseLeftClick) = false;
	setupScreen(DO_SETUP);
	
	for (int i = P_CHEWY; i <= P_NICHELLE; ++i) {
		if (_G(gameState).R79Val[i] != 0) {
			_G(gameState)._personHide[i] = false;
			_G(gameState).R79Val[i] = 0;
		}
	}

	if (_G(gameState)._personRoomNr[P_HOWARD] == 80)
		_G(gameState)._personRoomNr[P_HOWARD] = nextRoom;

	if (_G(gameState)._personRoomNr[P_NICHELLE] == 80)
		_G(gameState)._personRoomNr[P_NICHELLE] = nextRoom;

	_G(flags).NoScroll = false;
	switchRoom(nextRoom);
}

} // namespace Rooms
} // namespace Chewy
