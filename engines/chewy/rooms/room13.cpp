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
		_G(out)->cls();
		flic_cut(FCUT_012);
		set_person_pos(106, 65, P_CHEWY, P_RIGHT);
		switch_room(11);
		start_spz(CH_TALK12, 255, false, P_CHEWY);
		start_aad_wait(27, -1);
	} else {
		if (_G(spieler).R13MonitorStatus)
			_G(det)->show_static_spr(11 - _G(spieler).R13MonitorStatus);

		if (!_G(spieler).R13BorkOk) {
			_G(det)->show_static_spr(12);
			_G(obj)->hide_sib(SIB_BANDKNOPF_R13);
			_G(spieler).R13Bandlauf = true;

			_G(atds)->set_ats_str(94, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);

			_G(atds)->set_ats_str(97, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);

			_G(atds)->set_ats_str(93, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);
		}

		if (_G(spieler).R13Bandlauf) {
			for (int i = 0; i < 5; ++i)
				_G(det)->start_detail(i, 255, false);
		}
		
		if (!_G(flags).LoadGame && _G(spieler).R13Band) {
			_G(spieler).room_e_obj[25].Attribut = AUSGANG_OBEN;
			_G(atds)->set_steuer_bit(100, ATS_AKTIV_BIT, ATS_DATEI);
			_G(spieler).R13Band = false;
		}

		if (_G(spieler).R21GitterMuell)
			_G(det)->hide_static_spr(6);
	}
}

void Room13::xit() {
	_G(spieler).room_e_obj[25].Attribut = AUSGANG_OBEN;
	_G(atds)->set_steuer_bit(100, ATS_AKTIV_BIT, ATS_DATEI);
	_G(spieler).R13Band = false;
}

void Room13::gedAction(int index) {
	if (index == 2 && _G(spieler).R12ChewyBork) {
		stop_person(P_CHEWY);
		talk_bork();
	}
}

void Room13::talk_bork() {
	if (!_G(spieler).R13BorkOk) {
		_G(spieler).R13BorkOk = true;
		_G(spieler).R12ChewyBork = false;
		_G(det)->show_static_spr(13);
		_G(det)->set_detail_pos(10, _G(spieler_vector)[P_CHEWY].Xypos[0], _G(spieler_vector)[P_CHEWY].Xypos[1]);
		_G(det)->set_static_pos(12, _G(spieler_vector)[P_CHEWY].Xypos[0], _G(spieler_vector)[P_CHEWY].Xypos[1], false, true);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_aad_wait(33, -1);
		_G(det)->stop_detail(9);
		_G(det)->load_taf_seq(86, 12, nullptr);
		_G(spieler).PersonHide[P_CHEWY] = false;
		_G(det)->hide_static_spr(12);
		_G(auto_obj) = 1;

		_G(mov_phasen)[R13_BORK_OBJ].AtsText = 122;
		_G(mov_phasen)[R13_BORK_OBJ].Lines = 3;
		_G(mov_phasen)[R13_BORK_OBJ].Repeat = 1;
		_G(mov_phasen)[R13_BORK_OBJ].ZoomFak = 0;
		_G(auto_mov_obj)[R13_BORK_OBJ].Id = AUTO_OBJ0;
		_G(auto_mov_vector)[R13_BORK_OBJ].Delay = _G(spieler).DelaySpeed;
		_G(auto_mov_obj)[R13_BORK_OBJ].Mode = true;
		init_auto_obj(R13_BORK_OBJ, &R13_BORK_PHASEN[0][0], 3, (const MovLine *)R13_BORK_MPKT);
		autoMove(9, P_CHEWY);
		wait_auto_obj(R13_BORK_OBJ);
		autoMove(11, P_CHEWY);
		_G(flags).NoScroll = true;
		auto_scroll(41, 0);

		start_spz(68, 255, false, P_CHEWY);
		start_aad_wait(248, -1);
		flic_cut(FCUT_013);
		load_chewy_taf(CHEWY_NORMAL);
		_G(auto_obj) = 0;
		_G(flags).NoScroll = false;

		_G(atds)->set_steuer_bit(122, ATS_AKTIV_BIT, ATS_DATEI);
		_G(atds)->del_steuer_bit(92, ATS_AKTIV_BIT, ATS_DATEI);
		_G(obj)->show_sib(SIB_BANDKNOPF_R13);
		set_person_pos(153, 138, P_CHEWY, P_LEFT);

		start_aad_wait(34, -1);
		start_aad_wait(249, -1);

		_G(obj)->show_sib(SIB_TKNOPF1_R11);
		_G(obj)->show_sib(SIB_SCHLITZ_R11);
		_G(obj)->show_sib(SIB_TKNOPF2_R11);
		_G(obj)->show_sib(SIB_TKNOPF3_R11);

		if (_G(spieler).R6DoorRightB)
			_G(spieler).room_e_obj[20].Attribut = AUSGANG_LINKS;
		_G(spieler).room_e_obj[21].Attribut = AUSGANG_OBEN;
	}
}

void Room13::jmp_band() {
	if (!_G(spieler).R13Band && !_G(spieler).R12ChewyBork) {
		if (!_G(spieler).R13Bandlauf) {
			_G(obj)->hide_sib(SIB_BANDKNOPF_R13);
			_G(spieler).room_e_obj[25].Attribut = 255;
			_G(atds)->del_steuer_bit(100, ATS_AKTIV_BIT, ATS_DATEI);
			_G(spieler).R13Band = true;
			autoMove(3, P_CHEWY);
			_G(spieler).PersonHide[P_CHEWY] = true;
			start_detail_wait(8, 1, ANI_FRONT);
			_G(spieler).PersonHide[P_CHEWY] = false;
			set_person_pos(292, 98, P_CHEWY, P_RIGHT);

		} else {
			_G(spieler).R13Surf = true;
			_G(maus_links_click) = false;
			autoMove(12, P_CHEWY);
			start_aad_wait(117, -1);
			_G(flags).NoScroll = true;
			auto_scroll(76, 0);
			flic_cut(FCUT_014);
			_G(flags).NoScroll = false;
			set_person_pos(195, 226, P_CHEWY, P_LEFT);

			_G(spieler).R13Bandlauf = false;
			_G(atds)->set_ats_str(94, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);
			_G(atds)->set_ats_str(97, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);
			_G(atds)->set_ats_str(93, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);
			_G(obj)->calc_rsi_flip_flop(SIB_BANDKNOPF_R13);
			_G(obj)->hide_sib(SIB_BANDKNOPF_R13);
			switch_room(14);

			flic_cut(FCUT_017);
			register_cutscene(6);
			_G(spieler).scrollx = 92;
			_G(spieler).scrolly = 120;
			_G(spieler).PersonHide[P_CHEWY] = true;
			wait_show_screen(20);
			_G(spieler).PersonHide[P_CHEWY] = false;
		}
	}
}

void Room13::jmp_boden() {
	if (_G(spieler).R13Band) {
		if (!_G(spieler).R13Surf)
			_G(obj)->show_sib(SIB_BANDKNOPF_R13);

		_G(spieler).room_e_obj[25].Attribut = AUSGANG_OBEN;
		_G(atds)->set_steuer_bit(100, ATS_AKTIV_BIT, ATS_DATEI);
		_G(spieler).R13Band = false;
		autoMove(5, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(7, 1, ANI_FRONT);
		_G(spieler).PersonHide[P_CHEWY] = false;
		set_person_pos(176, 138, P_CHEWY, P_LEFT);
	}
}

int16 Room13::monitor_knopf() {
	int16 action_flag = false;
	
	if (!_G(spieler).inv_cur) {
		if (_G(spieler).R13Band) {
			action_flag = true;
			start_aad_wait(620, -1);

		} else if (!_G(spieler).R12ChewyBork) {
			action_flag = true;
			autoMove(8, P_CHEWY);
			_G(spieler).PersonHide[P_CHEWY] = true;
			start_detail_wait(6, 1, ANI_FRONT);
			_G(spieler).PersonHide[P_CHEWY] = false;

			if (_G(spieler).R13MonitorStatus)
				_G(det)->hide_static_spr(11 - _G(spieler).R13MonitorStatus);

			++_G(spieler).R13MonitorStatus;
			if (_G(spieler).R13MonitorStatus > 4)
				_G(spieler).R13MonitorStatus = 0;
			else
				_G(det)->show_static_spr(11 - _G(spieler).R13MonitorStatus);

			_G(atds)->set_ats_str(96, TXT_MARK_LOOK, _G(spieler).R13MonitorStatus, ATS_DATEI);
		}
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
