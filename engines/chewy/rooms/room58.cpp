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
#include "chewy/rooms/room58.h"

namespace Chewy {
namespace Rooms {

void Room58::entry() {
	_G(SetUpScreenFunc) = setup_func;
}

void Room58::xit() {
	_G(gameState).scrollx = _G(gameState).R58TmpSx;
	_G(gameState).scrolly = _G(gameState).R58TmpSy;
	_G(spieler_vector)[P_CHEWY].Xypos[0] = _G(gameState).R58TmpX;
	_G(spieler_vector)[P_CHEWY].Xypos[1] = _G(gameState).R58TmpY;
	_G(spieler_mi)[P_CHEWY].XyzStart[0] = _G(gameState).R58TmpX;
	_G(spieler_mi)[P_CHEWY].XyzStart[1] = _G(gameState).R58TmpY;
	_G(flags).MainInput = true;
	_G(flags).LoadGame = true;
	show_person();
	set_person_rnr();
	const int16 tmp = _G(gameState).R58TmpRoom1;
	_G(gameState)._personRoomNr[P_CHEWY] = _G(gameState).R58TmpRoom;

	switchRoom(tmp);
}

void Room58::look_cut_mag(int16 r_nr) {
	_G(gameState).R58TmpRoom = r_nr;
	_G(gameState).R58TmpRoom1 = _G(gameState)._personRoomNr[P_CHEWY];

	_G(gameState).R58TmpSx = _G(gameState).scrollx;
	_G(gameState).R58TmpSy = _G(gameState).scrolly;
	_G(gameState).R58TmpX = _G(spieler_vector)[P_CHEWY].Xypos[0];
	_G(gameState).R58TmpY = _G(spieler_vector)[P_CHEWY].Xypos[1];
	_G(spieler_vector)[P_CHEWY].Xypos[0] = 160;
	_G(spieler_vector)[P_CHEWY].Xypos[1] = 100;
	_G(gameState).scrollx = 0;
	_G(gameState).scrolly = 0;
	_G(flags).MainInput = false;
	hide_person();
	switchRoom(r_nr);
}

void Room58::setup_func() {
	if (_G(menu_item) != CUR_LOOK) {
		_G(menu_item) = CUR_LOOK;
		cursorChoice(_G(menu_item));
	}
}

} // namespace Rooms
} // namespace Chewy
