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
#include "chewy/rooms/room22.h"

namespace Chewy {
namespace Rooms {

#define BORK_OBJ 0

static const int16 BORK_PHASEN[4][2] = {
	{ 51, 56 },
	{ 36, 41 },
	{ 36, 41 },
	{ 36, 41 }
};

static const MovLine BORK_MPKT[2] = {
	{ { -30, 125, 170} , 1, 6 },
	{ { 155, 125, 170 }, 1, 6 },
};

static const MovLine BORK_MPKT1[2] = {
	{ { 155, 125, 170}, 2, 6 },
	{ { -30, 125, 170}, 0, 6 }
};

static const MovLine BORK_MPKT2[2] = {
	{ { -30, 125, 170 }, 1, 6 },
	{ {  90, 125, 170 }, 1, 6 }
};

void Room22::entry() {
	if (!_G(spieler).R22BorkPlatt) {
		det->load_taf_seq(36, (56 - 36) + 1, 0);
		room->set_timer(255, 15);
	} else if (_G(spieler).R22ChewyPlatt && !_G(spieler).R22GetBork)
		det->show_static_spr(4);
}

bool Room22::timer(int16 t_nr, int16 ani_nr) {
	if (!ani_nr && !flags.ExitMov) {
		bork(t_nr);
	}

	return false;
}

int16 Room22::chewy_amboss() {
	int16 action_flag = false;
	if (!_G(spieler).R22ChewyPlatt && !_G(spieler).inv_cur && !flags.AutoAniPlay) {
		action_flag = true;
		flags.AutoAniPlay = true;
		auto_move(5, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(1, 1, ANI_VOR);
		_G(spieler).PersonHide[P_CHEWY] = false;
		auto_move(2, P_CHEWY);
		flags.NoPalAfterFlc = false;
		flic_cut(FCUT_007, CFO_MODE);
		_G(spieler).R22ChewyPlatt = true;
		atds->set_ats_str(79, 1, ATS_DATEI);
		flags.AutoAniPlay = false;
	}
	return action_flag;
}

void Room22::bork(int16 t_nr) {
	if (!flags.AutoAniPlay) {
		flags.AutoAniPlay = true;
		if (!_G(spieler).R22BorkPlatt) {
			hide_cur();
			start_spz(CH_TALK2, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(10, -1);
			auto_move(3, P_CHEWY);
			_G(auto_obj) = 1;
			mov_phasen[BORK_OBJ].AtsText = 0;
			mov_phasen[BORK_OBJ].Lines = 2;
			mov_phasen[BORK_OBJ].Repeat = 1;
			mov_phasen[BORK_OBJ].ZoomFak = 0;
			auto_mov_obj[BORK_OBJ].Id = AUTO_OBJ0;
			auto_mov_vector[BORK_OBJ].Delay = _G(spieler).DelaySpeed;
			auto_mov_obj[BORK_OBJ].Mode = 1;
			if (!_G(spieler).R22Paint) {
				bork_walk1();
			} else {
				_G(spieler).R22ChewyPlatt = true;
				bork_walk2();
			}
			show_cur();
		}
		uhr->reset_timer(t_nr, 0);
		flags.AutoAniPlay = false;
	}
}

void Room22::bork_walk1() {
	init_auto_obj(BORK_OBJ, &BORK_PHASEN[0][0], mov_phasen[BORK_OBJ].Lines,
		(const MovLine *)BORK_MPKT);
	wait_auto_obj(BORK_OBJ);

	start_detail_wait(2, 1, ANI_VOR);

	mov_phasen[BORK_OBJ].Repeat = 1;
	init_auto_obj(BORK_OBJ, &BORK_PHASEN[0][0], mov_phasen[BORK_OBJ].Lines,
		(const MovLine *)BORK_MPKT1);
	wait_auto_obj(BORK_OBJ);
}

void Room22::bork_walk2() {
	init_auto_obj(BORK_OBJ, &BORK_PHASEN[0][0], mov_phasen[BORK_OBJ].Lines,
		(const MovLine *)BORK_MPKT2);
	wait_auto_obj(BORK_OBJ);

	flic_cut(FCUT_009, CFO_MODE);
	det->show_static_spr(4);
	atds->del_steuer_bit(81, ATS_AKTIV_BIT, ATS_DATEI);
	_G(spieler).R22BorkPlatt = true;
	atds->set_steuer_bit(79, ATS_AKTIV_BIT, ATS_DATEI);
}

void Room22::get_bork() {
	if (!_G(spieler).R22GetBork && _G(spieler).R22BorkPlatt) {
		auto_move(4, P_CHEWY);
		det->hide_static_spr(4);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_ani_block(2, ablock14);
		set_person_pos(171, 120, P_CHEWY, P_LEFT);
		start_aad_wait(11, -1);
		det->stop_detail(3);
		_G(spieler).PersonHide[P_CHEWY] = false;
		atds->set_steuer_bit(81, ATS_AKTIV_BIT, ATS_DATEI);
		invent_2_slot(BORK_INV);

		_G(spieler).R22GetBork = true;
		menu_item = CUR_WALK;
		cursor_wahl(menu_item);
	}
}

int16 Room22::malen() {
	int16 action_flag = false;
	if (!flags.AutoAniPlay && is_cur_inventar(17)) {
		action_flag = true;
		flags.AutoAniPlay = true;
		auto_move(8, P_CHEWY);
		flic_cut(FCUT_008, CFO_MODE);
		atds->set_ats_str(82, TXT_MARK_LOOK, 1, ATS_DATEI);
		_G(spieler).R22Paint = true;
		obj->calc_rsi_flip_flop(SIB_PAINT_R22);
		obj->hide_sib(SIB_PAINT_R22);
		del_inventar(_G(spieler).AkInvent);
		obj->calc_all_static_detail();
		flags.AutoAniPlay = false;
	}
	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
