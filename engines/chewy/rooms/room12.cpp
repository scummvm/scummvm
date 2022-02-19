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
#include "chewy/rooms/room12.h"

namespace Chewy {
namespace Rooms {

#define R12_BORK_OBJ 0

static const int16 R12_BORK_PHASEN[4][2] = {
	{ 74, 79 },
	{ 80, 85 },
	{ 62, 67 },
	{ 68, 73 }
};

static const MovLine R12_BORK_MPKT[5] = {
	{ { 207, 220, 199 }, 2, 6 },
	{ { 207, 145, 199 }, 2, 6 },
	{ {  30, 145, 199 }, 0, 6 },
	{ { 207, 145, 199 }, 1, 6 },
	{ { 207, 220, 199 }, 3, 6 }
};

static const MovLine R12_BORK_MPKT1[2] = {
	{ { 207, 220, 199 }, 0, 6 },
	{ { 170, 145, 199 }, 0, 6 }
};

static const MovLine R12_BORK_MPKT2[3] = {
	{ { 170, 145, 199 }, 1, 8 },
	{ { 180, 145, 120 }, 1, 8 },
	{ { 300,  80, 120 }, 1, 8 }
};

static const AniBlock ABLOCK16[2] = {
	{ 0, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 1, 1, ANI_VOR, ANI_WAIT, 0 },
};


void Room12::entry() {
	_G(zoom_horizont) = 150;
	_G(timer_nr)[1] = _G(room)->set_timer(254, 20);

	if (!_G(spieler).R12Betreten) {
		_G(spieler).R12Betreten = true;
		hide_cur();

		for (int16 i = 7; i < 10; i++)
			_G(det)->show_static_spr(i);

		_G(flags).NoScroll = true;
		auto_scroll(60, 0);
		flic_cut(FCUT_016);
		_G(flags).NoScroll = false;

		for (int16 i = 7; i < 10; i++)
			_G(det)->hide_static_spr(i);

		_G(obj)->show_sib(SIB_TALISMAN_R12);
		_G(obj)->calc_rsi_flip_flop(SIB_TALISMAN_R12);
		_G(obj)->calc_all_static_detail();
		auto_move(5, P_CHEWY);
		start_spz(CH_TALK12, 255, false, 0);
		start_aad_wait(109, -1);
		show_cur();

	} else if (_G(spieler).R12Talisman && !_G(spieler).R12BorkInRohr)
		_G(timer_nr)[0] = _G(room)->set_timer(255, 20);
	else if (_G(spieler).R12BorkInRohr && !_G(spieler).R12RaumOk)
		_G(det)->show_static_spr(12);
}

bool Room12::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0]) {
		if (!is_chewy_busy())
			init_bork();
	} else if (t_nr == _G(timer_nr)[1] && _G(spieler).R12TransOn) {
		_G(spieler).R12TransOn = false;
		start_aad_wait(30, -1);
	}

	return false;
}

void Room12::init_bork() {
	if (!auto_obj_status(R12_BORK_OBJ) && !_G(spieler).R12BorkTalk) {
		if (!_G(auto_obj))
			_G(det)->load_taf_seq(62, 24, nullptr);

		if (!_G(flags).AutoAniPlay && !_G(flags).ChAutoMov) {
			_G(auto_obj) = 1;
			_G(mov_phasen)[R12_BORK_OBJ].AtsText = 120;
			_G(mov_phasen)[R12_BORK_OBJ].Lines = 5;
			_G(mov_phasen)[R12_BORK_OBJ].Repeat = 1;
			_G(mov_phasen)[R12_BORK_OBJ].ZoomFak = (int16)_G(room)->_roomInfo->ZoomFak + 20;
			_G(auto_mov_obj)[R12_BORK_OBJ].Id = AUTO_OBJ0;
			_G(auto_mov_vector)[R12_BORK_OBJ].Delay = _G(spieler).DelaySpeed;
			_G(auto_mov_obj)[R12_BORK_OBJ].Mode = true;
			init_auto_obj(R12_BORK_OBJ, &R12_BORK_PHASEN[0][0], 5, (const MovLine *)R12_BORK_MPKT);

			if (!_G(spieler).R12TalismanOk) {
				hide_cur();
				_G(auto_mov_vector)[R12_BORK_OBJ].DelayCount = 1000;
				auto_move(5, P_CHEWY);
				_G(auto_mov_vector)[R12_BORK_OBJ].DelayCount = 0;

				if (_G(spieler).R12BorkCount < 3) {
					++_G(spieler).R12BorkCount;
					_G(uhr)->reset_timer(_G(timer_nr)[0], 0);
					wait_show_screen(10);
					start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
					start_aad_wait(14, -1);
				}

				wait_auto_obj(R12_BORK_OBJ);
				show_cur();
			} else {
				bork_ok();
			}
		}

		_G(uhr)->reset_timer(_G(timer_nr)[0], 0);
	}
}

void Room12::talk_bork() {
	if (!_G(spieler).R12TalismanOk) {
		start_aad_wait(28, -1);
	}
}

void Room12::bork_ok() {
	hide_cur();
	_G(flags).MausLinks = true;
	_G(auto_mov_vector)[R12_BORK_OBJ].DelayCount = 1000;
	auto_move(5, P_CHEWY);
	_G(auto_mov_vector)[R12_BORK_OBJ].DelayCount = 0;
	_G(spieler).R12BorkTalk = true;

	_G(mov_phasen)[R12_BORK_OBJ].Repeat = 1;
	_G(mov_phasen)[R12_BORK_OBJ].Lines = 2;
	init_auto_obj(R12_BORK_OBJ, &R12_BORK_PHASEN[0][0], 2, (const MovLine *)R12_BORK_MPKT1);
	wait_auto_obj(R12_BORK_OBJ);

	_G(spieler).R12BorkInRohr = true;
	_G(det)->set_detail_pos(3, 170, 145);
	_G(det)->start_detail(3, 255, ANI_VOR);
	start_aad_wait(57, -1);
	_G(det)->stop_detail(3);

	_G(mov_phasen)[R12_BORK_OBJ].Repeat = 1;
	_G(mov_phasen)[R12_BORK_OBJ].Lines = 3;
	init_auto_obj(R12_BORK_OBJ, &R12_BORK_PHASEN[0][0], 3, (const MovLine *)R12_BORK_MPKT2);
	wait_auto_obj(R12_BORK_OBJ);
	_G(det)->hide_static_spr(10);
	start_detail_wait(4, 1, ANI_VOR);
	_G(talk_hide_static) = -1;
	_G(det)->show_static_spr(12);
	_G(atds)->set_ats_str(118, TXT_MARK_LOOK, 2, ATS_DATEI);
	_G(obj)->calc_rsi_flip_flop(SIB_ROEHRE_R12);

	_G(flags).MausLinks = false;
	show_cur();
}

int16 Room12::use_terminal() {
	int16 action_flag = false;
	if (!_G(spieler).inv_cur) {
		action_flag = true;

		if (!_G(spieler).R12ChewyBork) {
			auto_move(6, P_CHEWY);
			start_aad_wait(110, -1);

			if (_G(spieler).R12BorkInRohr && !_G(spieler).R12RaumOk) {
				start_spz(CH_TALK5, 255, false, P_CHEWY);
				start_aad_wait(112, -1);
				_G(flags).NoScroll = true;
				auto_scroll(46, 0);
				flic_cut(FCUT_017);
				register_cutscene(5);

				load_chewy_taf(CHEWY_BORK);
				_G(flags).NoScroll = false;
				_G(atds)->set_ats_str(118, 0, ATS_DATEI);
				_G(det)->hide_static_spr(12);
				_G(menu_item) = CUR_WALK;
				cursor_wahl(_G(menu_item));
				set_person_pos(108, 90, P_CHEWY, -1);
				_G(spieler).R12ChewyBork = true;
				_G(spieler).R12RaumOk = true;
				auto_move(4, P_CHEWY);
				start_spz(68, 255, false, P_CHEWY);
				start_aad_wait(113, 0);

			} else if (_G(spieler).R12TalismanOk && !_G(spieler).R12RaumOk) {
				use_linke_rohr();

			} else {
				_G(spieler).R12TransOn = true;
				_G(uhr)->reset_timer(_G(timer_nr)[1], 0);
			}
		} else {
			start_aad(114, 0);
		}
	}

	return action_flag;
}

void Room12::use_linke_rohr() {
	_G(spieler).R12TalismanOk = false;
	_G(spieler).R12KetteLinks = true;
	_G(uhr)->disable_timer();
	_G(obj)->calc_rsi_flip_flop(SIB_L_ROEHRE_R12);
	_G(obj)->calc_rsi_flip_flop(SIB_ROEHRE_R12);
	_G(obj)->calc_all_static_detail();
	_G(atds)->set_ats_str(118, TXT_MARK_LOOK, 0, ATS_DATEI);
	_G(atds)->set_ats_str(117, TXT_MARK_LOOK, 1, ATS_DATEI);
	start_aad(111, 0);
}

int16 Room12::chewy_trans() {
	int16 action_flag = false;
	if (!_G(spieler).inv_cur && _G(spieler).R12TransOn) {
		action_flag = true;
		_G(flags).AutoAniPlay = true;
		auto_move(9, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_ani_block(2, ABLOCK16);
		set_person_pos(108, 82, P_CHEWY, P_RIGHT);
		_G(spieler).PersonHide[P_CHEWY] = false;
		_G(spieler).R12TransOn = false;
		_G(flags).AutoAniPlay = false;
	}
	return action_flag;
}

int16 Room12::useTransformerTube() {
	bool result = false;

	if (!_G(spieler).inv_cur) {
		result = true;

		if (_G(spieler).R12KetteLinks) {
			_G(spieler).R12KetteLinks = false;
			_G(uhr)->enable_timer();
			_G(atds)->set_ats_str(117, 1, AAD_DATEI);
		} else {
			auto_move(7, P_CHEWY);
			start_aad_wait(29, -1);
		}
	}

	return result;
}

} // namespace Rooms
} // namespace Chewy
