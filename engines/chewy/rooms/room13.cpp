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
#include "chewy/rooms/room13.h"

namespace Chewy {
namespace Rooms {

#define R13_BORK_OBJ 0

static const int16 R13_BORK_PHASEN[4][2] = {
	{ 92, 97 },
	{ 86, 91 },
	{ 86, 91 },
	{ 68, 73 }
};

static const MovLine R13_BORK_MPKT[3] = {
	{ { 168, 140, 162 }, 0, 6 },
	{ {  50, 140, 162 }, 0, 6 },
	{ {  50, 107, 130 }, 2, 6 }
};

void Room13::entry() {
	if (!_G(spieler).R12ChewyBork && !_G(spieler).R13BorkOk) {
		out->cls();
		flic_cut(FCUT_013, CFO_MODE);
		set_person_pos(106, 65, P_CHEWY, P_RIGHT);
		switch_room(11);
		start_spz(16, 255, 0, 0);
		start_aad_wait(27, -1);
	} else {
		if (_G(spieler).R13MonitorStatus)
			det->show_static_spr(11 - _G(spieler).R13MonitorStatus);

		if (!_G(spieler).R13BorkOk) {
			det->show_static_spr(12);
			obj->hide_sib(SIB_BANDKNOPF_R13);
			_G(spieler).R13Bandlauf = 1;

			atds->set_ats_str(94, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);

			atds->set_ats_str(97, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);

			atds->set_ats_str(93, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);
		}

		if (_G(spieler).R13Band) {
			_G(spieler).room_e_obj[25].Attribut = AUSGANG_OBEN;
			atds->set_steuer_bit(100, 1, 1);
		}

		if (_G(spieler).R21GitterMuell)
			det->hide_static_spr(6);
	}
}

void Room13::xit() {
	_G(spieler).room_e_obj[25].Attribut = AUSGANG_OBEN;
	atds->set_steuer_bit(100, 1, 1);
	_G(spieler).R13Band = false;
}

void Room13::gedAction(int index) {
	switch (index) {
	case 2:
		if (_G(spieler).R12ChewyBork) {
			stop_person(P_CHEWY);
			talk_bork();
		}
		break;

	default:
		break;
	}
}

void Room13::talk_bork() {
	if (!_G(spieler).R13BorkOk) {
		_G(spieler).R13BorkOk = true;
		_G(spieler).R12ChewyBork = false;
		det->show_static_spr(13);
		det->set_detail_pos(10, spieler_vector[P_CHEWY].Xypos[0], spieler_vector[P_CHEWY].Xypos[1]);
		det->set_static_pos(12, spieler_vector[P_CHEWY].Xypos[0], spieler_vector[P_CHEWY].Xypos[1], 0, true);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_aad_wait(33, -1);
		det->stop_detail(9);
		det->load_taf_seq(86, (97 - 86) + 1, 0);
		_G(spieler).PersonHide[P_CHEWY] = false;
		det->hide_static_spr(12);
		_G(auto_obj) = 1;

		mov_phasen[R13_BORK_OBJ].AtsText = 122;
		mov_phasen[R13_BORK_OBJ].Lines = 3;
		mov_phasen[R13_BORK_OBJ].Repeat = 1;
		mov_phasen[R13_BORK_OBJ].ZoomFak = 0;
		auto_mov_obj[R13_BORK_OBJ].Id = AUTO_OBJ0;
		auto_mov_vector[R13_BORK_OBJ].Delay = _G(spieler).DelaySpeed;
		auto_mov_obj[R13_BORK_OBJ].Mode = true;
		init_auto_obj(R13_BORK_OBJ, &R13_BORK_PHASEN[0][0], mov_phasen[R13_BORK_OBJ].Lines,
			(const MovLine *)R13_BORK_MPKT);
		auto_move(9, P_CHEWY);
		wait_auto_obj(R13_BORK_OBJ);
		auto_move(11, P_CHEWY);
		flags.NoScroll = true;
		auto_scroll(41, 0);

		start_spz(68, 255, 0, 0);
		start_aad_wait(248, -1);
		flic_cut(FCUT_014, CFO_MODE);
		load_chewy_taf(CHEWY_NORMAL);
		_G(auto_obj) = 0;
		flags.NoScroll = false;

		atds->set_steuer_bit(122, ATS_AKTIV_BIT, ATS_DATEI);
		atds->del_steuer_bit(92, ATS_AKTIV_BIT, ATS_DATEI);
		obj->show_sib(SIB_BANDKNOPF_R13);
		set_person_pos(153, 138, P_CHEWY, P_LEFT);

		start_aad_wait(34, -1);
		start_aad_wait(249, -1);

		obj->show_sib(SIB_TKNOPF1_R11);
		obj->show_sib(SIB_SCHLITZ_R11);
		obj->show_sib(SIB_TKNOPF2_R11);
		obj->show_sib(SIB_TKNOPF3_R11);

		if (_G(spieler).R6DoorRightB)
			_G(spieler).room_e_obj[20].Attribut = AUSGANG_LINKS;
		_G(spieler).room_e_obj[21].Attribut = AUSGANG_OBEN;
	}
}

void Room13::jmp_band() {
	if (!_G(spieler).R13Band && !_G(spieler).R12ChewyBork) {
		if (!_G(spieler).R13Bandlauf) {
			obj->hide_sib(SIB_BANDKNOPF_R13);
			_G(spieler).room_e_obj[25].Attribut = 255;
			atds->del_steuer_bit(100, ATS_AKTIV_BIT, ATS_DATEI);
			_G(spieler).R13Band = true;
			auto_move(3, P_CHEWY);
			_G(spieler).PersonHide[P_CHEWY] = true;
			start_detail_wait(8, 1, ANI_VOR);
			_G(spieler).PersonHide[P_CHEWY] = false;
			set_person_pos(292, 98, P_CHEWY, P_RIGHT);

		} else {
			_G(spieler).R13Surf = true;
			_G(maus_links_click) = false;
			auto_move(12, P_CHEWY);
			start_aad_wait(117, -1);
			flags.NoScroll = true;
			auto_scroll(76, 0);
			flic_cut(FCUT_015, CFO_MODE);
			flags.NoScroll = false;
			set_person_pos(195, 226, P_CHEWY, P_LEFT);

			_G(spieler).R13Bandlauf = 0;
			atds->set_ats_str(94, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);
			atds->set_ats_str(97, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);
			atds->set_ats_str(93, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);
			obj->calc_rsi_flip_flop(SIB_BANDKNOPF_R13);
			obj->hide_sib(SIB_BANDKNOPF_R13);
			switch_room(14);

			flic_cut(FCUT_018, CFO_MODE);
			_G(spieler).scrollx = 92;
			_G(spieler).scrolly = 120;
			_G(spieler).PersonHide[P_CHEWY] = true;
			wait_show_screen(40);
			_G(spieler).PersonHide[P_CHEWY] = false;
		}
	} else
		start_aad_wait(118, -1);
}

void Room13::jmp_boden() {
	if (_G(spieler).R13Band) {
		if (!_G(spieler).R13Surf)
			obj->show_sib(SIB_BANDKNOPF_R13);

		_G(spieler).room_e_obj[25].Attribut = AUSGANG_OBEN;
		atds->set_steuer_bit(100, ATS_AKTIV_BIT, ATS_DATEI);
		_G(spieler).R13Band = false;
		auto_move(5, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(7, 1, ANI_VOR);
		_G(spieler).PersonHide[P_CHEWY] = false;
		set_person_pos(176, 138, P_CHEWY, P_LEFT);
	}
}

int16 Room13::monitor_knopf() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur) {
		action_flag = true;

		if (_G(spieler).R13Band) {
			start_aad_wait(620, -1);

		} else if (!_G(spieler).R12ChewyBork) {
			auto_move(8, P_CHEWY);
			_G(spieler).PersonHide[P_CHEWY] = true;
			start_detail_wait(6, 1, ANI_VOR);
			_G(spieler).PersonHide[P_CHEWY] = false;

			if (_G(spieler).R13MonitorStatus)
				det->hide_static_spr(11 - _G(spieler).R13MonitorStatus);

			++_G(spieler).R13MonitorStatus;
			if (_G(spieler).R13MonitorStatus > 4)
				_G(spieler).R13MonitorStatus = 0;
			else
				det->show_static_spr(11 - _G(spieler).R13MonitorStatus);

			atds->set_ats_str(96, TXT_MARK_LOOK, _G(spieler).R13MonitorStatus, ATS_DATEI);
		}
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
