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
#include "chewy/ani_dat.h"
#include "chewy/room.h"
#include "chewy/rooms/room7.h"

namespace Chewy {
namespace Rooms {

void Room7::entry() {
	_G(spieler).ScrollxStep = 2;
}

void Room7::haken(int16 sib_nr) {
	int16 dia_nr;
	del_inventar(8);

	_G(spieler).AkInvent = -1;
	menu_item = CUR_WALK;
	cursor_wahl(menu_item);
	if (_G(spieler).R7RHaken) {
		_G(spieler).R7SeilOk = true;
		if (sib_nr == SIB_LHAKEN_R7) {
			obj->calc_rsi_flip_flop(SIB_LHAKEN_R7);
			obj->calc_rsi_flip_flop(SIB_RHAKEN_R7);
		}
		atds->del_steuer_bit(56, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_ats_str(55, TXT_MARK_LOOK, 1, ATS_DATEI);
		dia_nr = 9;
	} else {
		_G(spieler).R7SeilLeft = true;
		dia_nr = 48;
	}
	atds->set_ats_str(54, TXT_MARK_LOOK, 1, ATS_DATEI);
	start_aad(dia_nr);
}

void Room7::klingel() {
	if ((!_G(spieler).R7BellCount) ||
		(_G(spieler).R7BellCount >= 2 && _G(spieler).R7SeilLeft != 0 && !_G(spieler).R7SeilOk)) {
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_aad(5, 0);
		start_ani_block(3, ablock25);

		det->show_static_spr(7);
		start_detail_wait(12, 1, ANI_VOR);
		start_detail_wait(11, 1, ANI_VOR);
		det->hide_static_spr(7);
		det->stop_detail(5);
		set_person_pos(95, 94, P_CHEWY, P_RIGHT);
		_G(spieler).PersonHide[P_CHEWY] = false;
	} else if (_G(spieler).R7BellCount == 1) {
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_aad(6, 0);
		start_ani_block(3, ablock25);
		det->show_static_spr(7);
		start_detail_wait(10, 1, ANI_VOR);
		det->start_detail(13, 1, ANI_VOR);
		set_person_pos(95, 94, P_CHEWY, P_RIGHT);
		det->stop_detail(5);
		_G(spieler).PersonHide[P_CHEWY] = false;
		flags.NoScroll = true;
		auto_move(6, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		det->start_detail(0, 255, ANI_VOR);
		start_detail_wait(13, 1, ANI_VOR);
		flic_cut(FCUT_002, CFO_MODE);
		det->stop_detail(0);
		_G(spieler).scrollx = 0;
		_G(spieler).scrolly = 0;
		start_detail_frame(19, 1, ANI_VOR, 6);
		start_detail_frame(9, 1, ANI_VOR, 4);
		det->show_static_spr(9);
		wait_detail(9);
		det->hide_static_spr(9);
		obj->show_sib(SIB_SCHLOTT_R7);
		obj->calc_rsi_flip_flop(SIB_SCHLOTT_R7);
		set_person_pos(114, 138, P_CHEWY, -1);
		_G(spieler).PersonHide[P_CHEWY] = false;
		flags.NoScroll = false;
		det->hide_static_spr(7);
	} else if (!_G(spieler).R7SeilOk) {
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_aad(7, 0);
		start_ani_block(3, ablock25);
		det->show_static_spr(7);
		det->load_taf_seq(192, 74, 0);
		det->start_detail(14, 1, ANI_VOR);
		set_person_pos(95, 94, P_CHEWY, P_RIGHT);
		det->stop_detail(5);
		_G(spieler).PersonHide[P_CHEWY] = false;
		flags.NoScroll = true;
		auto_move(6, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		det->start_detail(0, 255, ANI_VOR);
		wait_detail(14);
		start_ani_block(4, ablock10);
		det->hide_static_spr(7);
		det->stop_detail(0);
		set_person_pos(181, 130, P_CHEWY, P_RIGHT);
		_G(spieler).PersonHide[P_CHEWY] = false;
		flags.NoScroll = false;
		det->del_taf_tbl(192, 74, 0);
	} else if (_G(spieler).R7SeilOk && !_G(spieler).R7BorkFlug) {
		_G(spieler).R7BorkFlug = true;
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_aad(8, 0);
		start_detail_wait(1, 1, ANI_VOR);
		obj->set_rsi_flip_flop(SIB_TKNOPF2_R7, 255);
		obj->hide_sib(SIB_KLINGEL_R7);
		flags.NoPalAfterFlc = false;
		flic_cut(FCUT_003, CFO_MODE);
		set_person_pos(201, 117, P_CHEWY, P_LEFT);
		_G(spieler).scrollx = 0;
		_G(spieler).scrolly = 0;
		_G(spieler).PersonHide[P_CHEWY] = false;
	}
	++_G(spieler).R7BellCount;
}

} // namespace Rooms
} // namespace Chewy
