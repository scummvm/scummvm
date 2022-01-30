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
#include "chewy/rooms/room70.h"
#include "chewy/rooms/room73.h"

namespace Chewy {
namespace Rooms {

void Room73::entry(int16 eib_nr) {
	det->enable_sound(0, 0);
	det->play_sound(0, 0);
	_G(spieler).ScrollxStep = 2;
	_G(spieler).ZoomXy[P_HOWARD][0] = 70;
	_G(spieler).ZoomXy[P_HOWARD][1] = 100;
	_G(spieler).ZoomXy[P_NICHELLE][0] = 70;
	_G(spieler).ZoomXy[P_NICHELLE][1] = 100;
	spieler_mi[P_HOWARD].Mode = true;
	spieler_mi[P_NICHELLE].Mode = true;
	_G(zoom_horizont) = 110;
	flags.ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	SetUpScreenFunc = Room70::setup_func;
	if (_G(spieler).flags28_20)
		det->show_static_spr(3+ (_G(spieler).R63Feuer ? 1 : 0)); // flags25_40

	if (flags.LoadGame)
		return;

	switch (eib_nr) {
	case 106:
		Room70::topEntry();
		break;
	case 115:
		Room70::leftEntry();
	default:
		break;
	}
}

void Room73::xit(int16 eib_nr) {
	_G(spieler).ScrollxStep = 1;

	if (_G(spieler).PersonRoomNr[P_HOWARD] != 73)
		return;

	switch (eib_nr) {
	case 110:
		_G(spieler).PersonRoomNr[P_HOWARD] = 71;
		_G(spieler).PersonRoomNr[P_NICHELLE] = 71;
		break;
	case 111:
		_G(spieler).PersonRoomNr[P_HOWARD] = 75;
		_G(spieler).PersonRoomNr[P_NICHELLE] = 75;
		break;
	default:
		break;
	}
}

int Room73::proc1() {
	if (_G(spieler).inv_cur && !_G(spieler).flags28_20)
		return 0;

	hide_cur();
	_G(spieler).flags28_20 = true;
	auto_move(3, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);
	atds->del_steuer_bit(433, ATS_AKTIV_BIT, ATS_DATEI);
	atds->del_steuer_bit(432, ATS_AKTIV_BIT, ATS_DATEI);
	atds->set_ats_str(430, 1, ATS_DATEI);
	det->show_static_spr(3);
	show_cur();

	return 1;	
}

int Room73::proc2() {
	if (_G(spieler).inv_cur)
		return 0;

	hide_cur();
	_G(spieler).flags28_40 = true;
	auto_move(4, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);
	det->hide_static_spr(3);
	det->hide_static_spr(4);
	atds->set_steuer_bit(433, ATS_AKTIV_BIT, ATS_DATEI);
	new_invent_2_cur(99);
	show_cur();

	return 1;
}

} // namespace Rooms
} // namespace Chewy
