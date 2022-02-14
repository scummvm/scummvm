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
#include "chewy/rooms/room91.h"

namespace Chewy {
namespace Rooms {

int16 Room91::_click;

void Room91::entry() {
	_G(zoom_horizont) = 110;
	flags.ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(spieler).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;
	_click = 0;
	_G(spieler).ZoomXy[P_HOWARD][0] = _G(spieler).ZoomXy[P_HOWARD][1] = 30;
	spieler_mi[P_HOWARD].Mode = true;

	if (flags.LoadGame)
		return;

	_G(spieler).scrollx = 320;
	hide_cur();

	if (_G(spieler).flags34_1) {
		set_person_pos(499, 106, P_CHEWY, P_RIGHT);
		set_person_pos(536, 90, P_HOWARD, P_RIGHT);
		if (!_G(spieler).flags34_2) {
			_G(spieler).flags34_2 = true;
			start_aad_wait(503, -1);
		}
	} else {
		flags.MainInput = false;
		flags.NoScroll = true;
		_G(spieler).flags34_1 = true;
		set_person_pos(326, 99, P_CHEWY, P_RIGHT);
		set_person_pos(312, 75, P_HOWARD, P_RIGHT);
		_G(spieler).SVal3 = 0;
		_G(spieler).flags34_4 = true;
		start_aad_wait(502, -1);
		_G(spieler).PersonHide[P_CHEWY] = true;
		det->start_detail(0, 255, false);

		for (int i = 0; i < 3; ++i) {
			_G(timer_nr)[i] = room->set_timer(5 + i, 3 + (2 * i));
		}
		det->start_detail(5, 2, false);
	}

	show_cur();
}

void Room91::xit(int16 eib_nr) {
	flags.MainInput = true;
	_G(spieler).ScrollxStep = 1;
	_G(spieler).scrollx = 0;

	if (eib_nr == 136)
		_G(spieler).PersonRoomNr[P_HOWARD] = 90;

	_G(spieler).flags34_4 = false;
	flags.NoScroll = false;
}

void Room91::setup_func() {
	if (!_G(spieler).flags34_4) {
		calc_person_look();
		const int xyPos = spieler_vector[0].Xypos[0];
		if (xyPos < 130)
			go_auto_xy(40, 97, P_HOWARD, ANI_GO);
		else if (xyPos < 312)
			go_auto_xy(221, 94, P_HOWARD, ANI_GO);
		else if (xyPos < 445)
			go_auto_xy(342, 93, P_HOWARD, ANI_GO);
		else
			go_auto_xy(536, 90, P_HOWARD, ANI_GO);
	} else {
		if (_G(menu_display))
			return;
		
		_G(menu_item) = CUR_USE;
		cur_2_inventory();
		cursor_wahl(CUR_22);

		if (_G(maus_links_click) == 0 || _click)
			return;

		const int oldClick = _click;
		_G(maus_links_click) = oldClick;
		_click = 1;
		const int aniNr = 1 + (minfo.y <= 100 ? 1 : 0);
		hide_cur();
		det->stop_detail(0);
		start_detail_wait(aniNr, 1, ANI_VOR);
		_click = oldClick;
		det->start_detail(0, 255, false);
		det->start_detail(aniNr + 2, 1, false);
		det->start_detail(7, 1, false);
		start_spz_wait(62, 1, false, P_HOWARD);
		_G(spieler).SVal3 += 1;
		show_cur();
		if (_G(spieler).SVal3 == 4) {
			det->stop_detail(0);
			_G(spieler).PersonHide[P_CHEWY] = false;
			hide_cur();
			auto_move(1, P_CHEWY);
			stop_spz();
			start_aad_wait(505, -1);
			_G(spieler).PersonHide[P_HOWARD] = true;
			start_detail_wait(9, 1, ANI_VOR);
			_G(spieler).PersonHide[P_HOWARD] = false;
			_G(spieler).PersonRoomNr[P_HOWARD] = 50;
			_G(spieler).flags34_4 = false;
			show_cur();
			switch_room(50);
		}
	}
}

} // namespace Rooms
} // namespace Chewy
