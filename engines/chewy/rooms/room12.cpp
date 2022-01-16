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
	int16 i;
	_G(zoom_horizont) = 150;
	_G(timer_nr)[1] = room->set_timer(255, 20);
	if (!_G(spieler).R12Betreten) {
		_G(spieler).R12Betreten = true;
		for (i = 7; i < 10; i++)
			det->show_static_spr(i);
		flags.NoScroll = true;
		auto_scroll(60, 0);
		flic_cut(FCUT_016, CFO_MODE);
		flags.NoScroll = false;
		for (i = 7; i < 10; i++)
			det->hide_static_spr(i);
		obj->show_sib(SIB_TALISMAN_R12);
		obj->calc_rsi_flip_flop(SIB_TALISMAN_R12);
		obj->calc_all_static_detail();
		auto_move(5, P_CHEWY);
		start_aad_wait(109, -1);
	} else {
		if (_G(spieler).R12Talisman == true && !_G(spieler).R12BorkInRohr)
			_G(timer_nr)[0] = room->set_timer(255, 20);
		else if (_G(spieler).R12BorkInRohr && !_G(spieler).R12RaumOk)
			det->show_static_spr(12);

	}
}

bool Room12::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0]) {
		if (!is_chewy_busy())
			init_bork();
	} else if (t_nr == _G(timer_nr)[1]) {
		if (_G(spieler).R12TransOn) {
			_G(spieler).R12TransOn = false;
			start_aad_wait(30, -1);
		}
	}

	return false;
}

void Room12::init_bork() {
	if (!auto_obj_status(R12_BORK_OBJ) &&
		!_G(spieler).R12BorkTalk) {

		if (!_G(auto_obj))
			det->load_taf_seq(62, (85 - 62) + 1, 0);
		if (!flags.AutoAniPlay && !flags.ChAutoMov) {
			_G(auto_obj) = 1;
			mov_phasen[R12_BORK_OBJ].AtsText = 120;
			mov_phasen[R12_BORK_OBJ].Lines = 5;
			mov_phasen[R12_BORK_OBJ].Repeat = 1;
			mov_phasen[R12_BORK_OBJ].ZoomFak = (int16)room->room_info->ZoomFak + 20;
			auto_mov_obj[R12_BORK_OBJ].Id = AUTO_OBJ0;
			auto_mov_vector[R12_BORK_OBJ].Delay = _G(spieler).DelaySpeed;
			auto_mov_obj[R12_BORK_OBJ].Mode = true;
			init_auto_obj(R12_BORK_OBJ, &R12_BORK_PHASEN[0][0], mov_phasen[R12_BORK_OBJ].Lines,
				(const MovLine *)R12_BORK_MPKT);
			if (!_G(spieler).R12TalismanOk) {
				hide_cur();
				auto_mov_vector[R12_BORK_OBJ].DelayCount = 1000;
				auto_move(5, P_CHEWY);
				auto_mov_vector[R12_BORK_OBJ].DelayCount = 0;
				if (_G(spieler).R12BorkCount < 3) {
					++_G(spieler).R12BorkCount;
					uhr->reset_timer(_G(timer_nr)[0], 0);
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
		uhr->reset_timer(_G(timer_nr)[0], 0);
	}
}

void Room12::talk_bork() {
	if (!_G(spieler).R12TalismanOk) {

		start_aad_wait(28, -1);

	}
}

void Room12::bork_ok() {
	hide_cur();
	flags.MausLinks = true;
	auto_mov_vector[R12_BORK_OBJ].DelayCount = 1000;
	auto_move(5, P_CHEWY);
	auto_mov_vector[R12_BORK_OBJ].DelayCount = 0;
	_G(spieler).R12BorkTalk = true;
	mov_phasen[R12_BORK_OBJ].Repeat = 1;
	mov_phasen[R12_BORK_OBJ].Lines = 2;
	init_auto_obj(R12_BORK_OBJ, &R12_BORK_PHASEN[0][0], mov_phasen[R12_BORK_OBJ].Lines,
		(const MovLine *)R12_BORK_MPKT1);
	wait_auto_obj(R12_BORK_OBJ);
	_G(spieler).R12BorkInRohr = true;
	det->set_detail_pos(3, 170, 145);
	det->start_detail(3, 255, ANI_VOR);
	start_aad_wait(57, -1);
	det->stop_detail(3);
	mov_phasen[R12_BORK_OBJ].Repeat = 1;
	mov_phasen[R12_BORK_OBJ].Lines = 3;
	init_auto_obj(R12_BORK_OBJ, &R12_BORK_PHASEN[0][0], mov_phasen[R12_BORK_OBJ].Lines,
		(const MovLine *)R12_BORK_MPKT2);
	wait_auto_obj(R12_BORK_OBJ);
	det->hide_static_spr(10);
	start_detail_wait(4, 1, ANI_VOR);
	talk_hide_static = -1;
	det->show_static_spr(12);
	atds->set_ats_str(118, TXT_MARK_LOOK, 2, ATS_DATEI);
	obj->calc_rsi_flip_flop(SIB_ROEHRE_R12);
	flags.MausLinks = false;
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
				start_aad_wait(112, -1);
				flags.NoScroll = true;
				auto_scroll(46, 0);
				flic_cut(FCUT_017, CFO_MODE);

				load_chewy_taf(CHEWY_BORK);
				flags.NoScroll = false;
				atds->set_ats_str(118, 0, ATS_DATEI);
				det->hide_static_spr(12);
				menu_item = CUR_WALK;
				cursor_wahl(menu_item);
				set_person_pos(108, 90, P_CHEWY, -1);
				_G(spieler).R12ChewyBork = true;
				_G(spieler).R12RaumOk = true;
				auto_move(4, P_CHEWY);
				start_aad_wait(113, 0);
			} else if (_G(spieler).R12TalismanOk && !_G(spieler).R12RaumOk) {
				_G(spieler).R12TalismanOk = false;
				_G(spieler).R12KetteLinks = true;
				uhr->disable_timer();
				obj->calc_rsi_flip_flop(SIB_L_ROEHRE_R12);
				obj->calc_rsi_flip_flop(SIB_ROEHRE_R12);
				obj->calc_all_static_detail();
				atds->set_ats_str(118, TXT_MARK_LOOK, 0, ATS_DATEI);
				atds->set_ats_str(117, TXT_MARK_LOOK, 1, ATS_DATEI);
				start_aad(111, 0);
			} else {
				_G(spieler).R12TransOn = true;
				uhr->reset_timer(_G(timer_nr)[1], 0);
			}
		} else
			start_aad(114, 0);
	}

	return action_flag;
}

int16 Room12::use_linke_rohr() {
	int16 action_flag = false;
	if (!_G(spieler).inv_cur) {
		action_flag = true;
		if (!_G(spieler).R12KetteLinks) {
			auto_move(7, P_CHEWY);
			start_aad_wait(29, -1);
		} else {
			_G(spieler).R12KetteLinks = false;
			uhr->enable_timer();
			atds->set_ats_str(117, TXT_MARK_LOOK, 0, ATS_DATEI);
		}
	}
	return action_flag;
}

int16 Room12::chewy_trans() {
	int16 action_flag = false;
	if (!_G(spieler).inv_cur) {
		if (_G(spieler).R12TransOn) {
			action_flag = true;
			flags.AutoAniPlay = true;
			auto_move(9, P_CHEWY);
			_G(spieler).PersonHide[P_CHEWY] = true;
			start_ani_block(2, ABLOCK16);
			set_person_pos(108, 82, P_CHEWY, P_RIGHT);
			_G(spieler).PersonHide[P_CHEWY] = false;
			_G(spieler).R12TransOn = false;
			flags.AutoAniPlay = false;
		}
	}
	return action_flag;
}

int16 Room12::proc1() {
	bool result = false;

	if (!_G(spieler).inv_cur) {
		result = true;

		if (_G(spieler).R12KetteLinks) {
			_G(spieler).R12KetteLinks = false;
			uhr->enable_timer();
			atds->set_ats_str(117, 1, 0);
		} else {
			auto_move(7, P_CHEWY);
			start_aad_wait(29, -1);
		}
	}

	return result;
}

int16 Room12::cut_serv(int16 frame) {
	atds->print_aad(_G(spieler).scrollx, _G(spieler).scrolly);
	if (frame == 43)
		start_aad(106, 0);

	return 0;
}

} // namespace Rooms
} // namespace Chewy
