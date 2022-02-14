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
#include "chewy/rooms/room88.h"

namespace Chewy {
namespace Rooms {

void Room88::entry() {
	_G(spieler).scrollx = 0;
	set_person_pos(163, 122, P_CHEWY, P_LEFT);
	set_person_pos(59, 107, P_HOWARD, P_RIGHT);
	set_person_pos(91, 110, P_NICHELLE, P_RIGHT);
	
	_G(det)->show_static_spr(1 + (_G(spieler).flags30_10 ? 1 : 0));
	if (_G(spieler).r88DestRoom == 0)
		_G(spieler).r88DestRoom = 82;

	_G(SetUpScreenFunc) = calc_person_look;

	if (_G(spieler).flags32_10) {
		_G(det)->start_detail(4, 255, false);
		_G(atds)->del_steuer_bit(505, ATS_AKTIV_BIT, ATS_DATEI);
	}
}

void Room88::xit() {
	_G(spieler).flags31_8 = true;
	_G(spieler).PersonRoomNr[P_HOWARD] = _G(spieler).r88DestRoom;

	if (_G(spieler).PersonRoomNr[P_NICHELLE] == 88)
		_G(spieler).PersonRoomNr[P_NICHELLE] = _G(spieler).PersonRoomNr[P_HOWARD];
}

int Room88::proc1() {
	if (_G(spieler).inv_cur)
		return 0;

	hide_cur();
	auto_move(0, P_CHEWY);
	switch_room(_G(spieler).r88DestRoom);
	_G(menu_item) = CUR_WALK;
	cursor_wahl(CUR_WALK);
	show_cur();
	
	return 1;
}

int Room88::proc2() {
	if (_G(spieler).inv_cur)
		return 0;

	hide_cur();
	auto_move(1, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);
	_G(det)->show_static_spr(0);
	start_detail_wait(0, 1, _G(spieler).flags30_10 ? ANI_GO : ANI_VOR);
	_G(det)->hide_static_spr(1 + (_G(spieler).flags30_10 ? 1 : 0));
	_G(spieler).flags31_10 = false;
	_G(det)->show_static_spr(1 + (!_G(spieler).flags30_10 ? 1 : 0));
	start_spz_wait(13, 1, false, P_CHEWY);
	_G(det)->hide_static_spr(0);
	
	show_cur();
	return 1;
}

int Room88::proc3() {
	if (_G(spieler).inv_cur)
		return 0;

	hide_cur();
	auto_move(2, P_CHEWY);

	if (!_G(spieler).flags30_80 || _G(spieler).R88UsedMonkey) {
		hide_cur();
		if (_G(spieler).flags32_10) {
			start_aad_wait(480, -1);
			_G(out)->setze_zeiger(nullptr);
			_G(out)->cls();
			flags.NoPalAfterFlc = true;
			flic_cut(87, CFO_MODE);
			_G(out)->setze_zeiger(nullptr);
			_G(out)->cls();
			_G(spieler).flags32_40 = true;
			switch_room(84);
		} else {
			start_aad_wait(465, -1);
		}
	} else {
		start_aad_wait(466, -1);
		start_spz_wait(13, 1, false, P_CHEWY);

		const int aniNr = 1 + (_G(spieler).flags31_10 ? 1 : 0);

		for (int i = 0; i < 3; ++i) {
			start_detail_wait(aniNr, 1, ANI_VOR);
			start_detail_wait(aniNr, 1, ANI_GO);
		}

		_G(out)->setze_zeiger(nullptr);
		_G(out)->cls();
		flags.NoPalAfterFlc = true;
		flic_cut(88, CFO_MODE);
		_G(spieler).PersonHide[P_CHEWY] = true;
		_G(spieler).flags32_1 = true;
		_G(out)->raster_col(0, 0, 0, 0);
		switch_room(80);
		hide_cur();
		start_detail_wait(1, 1, ANI_VOR);
		_G(out)->setze_zeiger(nullptr);
		_G(out)->cls();
		flags.NoPalAfterFlc = true;
		flic_cut(86, CFO_MODE);
		register_cutscene(24);

		_G(spieler).PersonHide[P_CHEWY] = false;
		_G(spieler).PersonRoomNr[P_HOWARD] = 88;
		_G(spieler).PersonRoomNr[P_NICHELLE] = 88;
		switch_room(88);
		_G(spieler).r88DestRoom = 84;
		_G(spieler).R88UsedMonkey = true;
		_G(spieler).flags32_1 = false;
	}

	show_cur();
	return 1;
}

} // namespace Rooms
} // namespace Chewy
