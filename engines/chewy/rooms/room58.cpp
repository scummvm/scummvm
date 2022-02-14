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
#include "chewy/global.h"
#include "chewy/room.h"
#include "chewy/rooms/room58.h"

namespace Chewy {
namespace Rooms {

void Room58::entry() {
	SetUpScreenFunc = setup_func;
}

void Room58::xit() {
	_G(spieler).scrollx = _G(spieler).R58TmpSx;
	_G(spieler).scrolly = _G(spieler).R58TmpSy;
	spieler_vector[P_CHEWY].Xypos[0] = _G(spieler).R58TmpX;
	spieler_vector[P_CHEWY].Xypos[1] = _G(spieler).R58TmpY;
	spieler_mi[P_CHEWY].XyzStart[0] = _G(spieler).R58TmpX;
	spieler_mi[P_CHEWY].XyzStart[1] = _G(spieler).R58TmpY;
	flags.MainInput = true;
	flags.LoadGame = true;
	show_person();
	set_person_rnr();
	const int16 tmp = _G(spieler).R58TmpRoom1;
	_G(spieler).PersonRoomNr[P_CHEWY] = _G(spieler).R58TmpRoom;

	switch_room(tmp);
}

void Room58::look_cut_mag(int16 r_nr) {
	_G(spieler).R58TmpRoom = r_nr;
	_G(spieler).R58TmpRoom1 = _G(spieler).PersonRoomNr[P_CHEWY];;

	_G(spieler).R58TmpSx = _G(spieler).scrollx;
	_G(spieler).R58TmpSy = _G(spieler).scrolly;
	_G(spieler).R58TmpX = spieler_vector[P_CHEWY].Xypos[0];
	_G(spieler).R58TmpY = spieler_vector[P_CHEWY].Xypos[1];
	spieler_vector[P_CHEWY].Xypos[0] = 160;
	spieler_vector[P_CHEWY].Xypos[1] = 100;
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	flags.MainInput = false;
	hide_person();
	switch_room(r_nr);
}

void Room58::setup_func() {
	if (_G(menu_item) != CUR_LOOK) {
		_G(menu_item) = CUR_LOOK;
		cursor_wahl(_G(menu_item));
	}
}

} // namespace Rooms
} // namespace Chewy
