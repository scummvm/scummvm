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
#include "chewy/rooms/room68.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK38[2] = {
	{ 5, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 6, 1, ANI_VOR, ANI_WAIT, 0 },
};

static const AniBlock ABLOCK39[3] = {
	{ 15, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 17, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 15, 1, ANI_RUECK, ANI_WAIT, 0 },
};


void Room68::entry() {
	_G(spieler).ScrollxStep = 2;
	_G(spieler).DiaAMov = 1;
	SetUpScreenFunc = setup_func;
	_G(r68HohesC) = -1;
	if (_G(spieler).R68Papagei) {
		det->show_static_spr(12);
		det->start_detail(21, 255, ANI_VOR);
	}
	if (!_G(spieler).R68DivaWeg) {
		_G(timer_nr)[0] = room->set_timer(255, 10);
		det->set_static_ani(18, -1);
	} else
		det->hide_static_spr(3);
	if (!flags.LoadGame) {
		hide_cur();
		set_person_pos(524, 51, P_HOWARD, P_LEFT);
		set_person_pos(550, 54, P_NICHELLE, P_LEFT);
		auto_move(7, P_CHEWY);
		show_cur();
	}
}

void Room68::xit() {
	_G(spieler).ScrollxStep = 1;
	_G(spieler).PersonRoomNr[P_HOWARD] = 66;
	_G(spieler).PersonRoomNr[P_NICHELLE] = 66;
}

bool Room68::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0])
		Room68::calc_diva();
	else
		return true;

	return false;
}

void Room68::setup_func() {
	int16 ho_x, ho_y;
	int16 ni_x, ni_y;
	int16 ch_x;

	switch (_G(r68HohesC)) {
	case 0:
		if (det->get_ani_status(_G(r68HohesC)) == false) {
			_G(r68HohesC) = 1;
			det->start_detail(_G(r68HohesC), 1, ANI_VOR);
		}
		break;

	case 1:
		if (det->get_ani_status(_G(r68HohesC)) == false) {
			_G(r68HohesC) = 2;
			det->start_detail(_G(r68HohesC), 3, ANI_VOR);
		}
		break;

	case 2:
		if (det->get_ani_status(_G(r68HohesC)) == false) {
			_G(r68HohesC) = -1;
			det->stop_detail(18);
			det->show_static_spr(3);
			uhr->reset_timer(_G(timer_nr)[0], 0);
		}
		break;

	}
	calc_person_look();
	ho_x = spieler_vector[P_HOWARD].Xypos[0];
	ni_x = spieler_vector[P_NICHELLE].Xypos[0];
	ho_y = 75;
	ni_y = 75;
	ch_x = spieler_vector[P_CHEWY].Xypos[0];
	if (ch_x < 130) {
		ho_x = 223;
		ni_x = 260;
	} else {
		ho_x = 320;
		ni_x = 350;
	}
	go_auto_xy(ho_x, ho_y, P_HOWARD, ANI_GO);
	go_auto_xy(ni_x, ni_y, P_NICHELLE, ANI_GO);
}

void Room68::look_kaktus() {
	hide_cur();
	auto_move(6, P_CHEWY);
	start_aad_wait(383, -1);
	show_cur();
}

void Room68::talk_indigo() {
	talk_indigo(-1);
}

void Room68::talk_indigo(int16 aad_nr) {
	hide_cur();
	auto_move(3, P_CHEWY);
	room->set_timer_status(8, TIMER_STOP);
	det->del_static_ani(8);
	if (aad_nr == -1) {
		det->set_static_ani(9, -1);
		start_aad_wait(384 + (int16)_G(spieler).R68IndigoDia, -1);
		_G(spieler).R68IndigoDia ^= 1;
		det->del_static_ani(9);
	} else {
		det->set_static_ani(12, -1);
		start_aad_wait(aad_nr, -1);
		det->del_static_ani(12);
	}
	room->set_timer_status(8, TIMER_START);
	det->set_static_ani(8, -1);
	show_cur();
}

int16 Room68::use_indigo() {
	int16 action_flag = false;
	hide_cur();
	if (is_cur_inventar(CLINT_500_INV)) {
		action_flag = true;
		if (_G(spieler).R68Lied) {
			hide_cur();
			auto_move(3, P_CHEWY);
			auto_scroll(78, 0);
			del_inventar(_G(spieler).AkInvent);
			talk_indigo(394);
			_G(cur_hide_flag) = false;
			hide_cur();
			room->set_timer_status(8, TIMER_STOP);
			det->del_static_ani(8);
			det->stop_detail(8);
			start_detail_wait(13, 3, ANI_VOR);
			start_detail_wait(25, 1, ANI_VOR);
			det->set_static_ani(12, -1);
			talk_indigo(398);
			room->set_timer_status(8, TIMER_STOP);
			det->del_static_ani(8);
			det->stop_detail(8);
			start_detail_wait(26, 1, ANI_VOR);
			room->set_timer_status(8, TIMER_START);
			det->set_static_ani(8, -1);
			new_invent_2_cur(KARTE_INV);
			_G(spieler).R68KarteDa = true;
		} else {
			talk_indigo(397);
		}
	} else if (is_cur_inventar(CLINT_1500_INV) || is_cur_inventar(CLINT_3000_INV)) {
		action_flag = true;
		start_aad_wait(393, -1);
	}
	show_cur();
	return action_flag;
}

void Room68::talk_keeper() {
	hide_cur();
	auto_move(2, P_CHEWY);
	room->set_timer_status(20, TIMER_STOP);
	det->del_static_ani(20);
	start_detail_wait(15, 1, ANI_VOR);
	det->set_static_ani(16, -1);
	show_cur();
	ssi[3].X = spieler_vector[P_CHEWY].Xypos[0] - _G(spieler).scrollx + spieler_mi[P_CHEWY].HotX;;
	ssi[3].Y = spieler_vector[P_CHEWY].Xypos[1] - _G(spieler).scrolly;
	atds->set_split_win(3, &ssi[3]);
	start_ads_wait(20);
	_G(cur_hide_flag) = false;
	hide_cur();
	det->del_static_ani(16);
	start_detail_wait(15, 1, ANI_RUECK);
	room->set_timer_status(20, TIMER_START);
	det->set_static_ani(20, -1);
	show_cur();
}

int16 Room68::use_papagei() {
	int16 action_flag = false;
	if (is_cur_inventar(PAPAGEI_INV)) {
		hide_cur();
		action_flag = true;
		_G(spieler).R68Papagei = true;
		del_inventar(_G(spieler).AkInvent);
		auto_move(5, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		det->show_static_spr(12);
		det->start_detail(21, 255, ANI_VOR);
		atds->del_steuer_bit(408, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_ats_str(407, 1, ATS_DATEI);
		show_cur();
	}
	return action_flag;
}

void Room68::calc_diva() {
	if (!_G(spieler).R68DivaWeg) {
		if (!_G(spieler).R68Papagei) {
			if (_G(r68HohesC) == -1) {
				_G(r68HohesC) = 0;
				det->hide_static_spr(3);
				det->start_detail(_G(r68HohesC), 1, ANI_RUECK);
				det->start_detail(18, 255, ANI_VOR);
			}
		} else {
			if (!_G(spieler).R68Gutschein) {

				if (!is_chewy_busy()) {
					hide_cur();
					_G(spieler).R68Gutschein = true;
					auto_move(4, P_CHEWY);
					start_aad_wait(386, -1);
					start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
					new_invent_2_cur(BAR_GUT_INV);
					uhr->reset_timer(_G(timer_nr)[0], 0);
					show_cur();
				}
			}
		}
	}
}

int16 Room68::use_keeper() {
	int16 action_flag = false;
	if (is_cur_inventar(BAR_GUT_INV)) {
		hide_cur();
		del_inventar(_G(spieler).AkInvent);
		action_flag = true;
		auto_move(2, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		room->set_timer_status(20, TIMER_STOP);
		det->del_static_ani(20);
		start_ani_block(3, ABLOCK39);
		room->set_timer_status(20, TIMER_START);
		det->set_static_ani(20, -1);
		new_invent_2_cur(B_MARY_INV);
		show_cur();
	}
	return action_flag;
}

int16 Room68::use_diva() {
	short action_flag = false;
	hide_cur();
	if (is_cur_inventar(B_MARY_INV)) {
		del_inventar(_G(spieler).AkInvent);
		action_flag = true;
		auto_move(4, P_CHEWY);
		start_detail_wait(4, 1, ANI_VOR);
		_G(spieler).R68Gutschein = false;
	} else if (is_cur_inventar(B_MARY2_INV)) {
		del_inventar(_G(spieler).AkInvent);
		action_flag = true;
		auto_move(4, P_CHEWY);
		det->hide_static_spr(3);
		_G(spieler).R68DivaWeg = true;
		start_ani_block(2, ABLOCK38);
		flic_cut(FCUT_083, FLC_MODE);
		flic_cut(FCUT_083, FLC_MODE);
		det->del_static_ani(18);
		start_detail_wait(7, 1, ANI_VOR);
		atds->set_steuer_bit(407, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_steuer_bit(412, ATS_AKTIV_BIT, ATS_DATEI);
		set_person_spr(P_RIGHT, P_CHEWY);
		start_aad_wait(402, -1);
	} else
		action_flag = use_papagei();
	show_cur();
	return action_flag;
}

void Room68::kostuem_aad(int16 aad_nr) {
	hide_cur();
	if (_G(spieler).DiaAMov != -1) {
		auto_move(_G(spieler).DiaAMov, P_CHEWY);
	}
	start_aad_wait(aad_nr, -1);
	if (!_G(spieler).R68DivaWeg)
		start_aad_wait(388, -1);
	else if (!_G(spieler).R67LiedOk)
		start_aad_wait(389, -1);
	else {
		SetUpScreenFunc = 0;
		del_inventar(_G(spieler).AkInvent);
		go_auto_xy(150, -13, P_NICHELLE, ANI_WAIT);
		_G(spieler).PersonHide[P_NICHELLE] = true;
		go_auto_xy(161, 59, P_HOWARD, ANI_GO);
		auto_move(4, P_CHEWY);
		start_aad_wait(390, -1);
		start_detail_wait(22, 1, ANI_VOR);
		_G(spieler).PersonHide[P_HOWARD] = true;
		det->start_detail(27, 255, ANI_VOR);
		start_detail_wait(23, 3, ANI_VOR);
		det->start_detail(24, 255, ANI_VOR);
		set_person_pos(26, 40, P_NICHELLE, P_RIGHT);
		start_aad_wait(391, -1);
		room->set_timer_status(8, TIMER_STOP);
		det->del_static_ani(8);
		start_detail_wait(10, 1, ANI_VOR);
		det->start_detail(11, 255, ANI_VOR);
		start_aad_wait(396, -1);
		det->stop_detail(11);
		det->start_detail(14, 255, ANI_VOR);
		start_aad_wait(395, -1);
		det->stop_detail(14);
		room->set_timer_status(8, TIMER_START);
		_G(spieler).PersonHide[P_HOWARD] = false;
		det->stop_detail(27);
		det->set_static_ani(8, -1);
		start_aad_wait(392, -1);
		_G(spieler).R68Lied = true;
		det->stop_detail(24);
		SetUpScreenFunc = setup_func;
		auto_move(1, P_CHEWY);
		_G(spieler).PersonHide[P_NICHELLE] = false;
		set_person_pos(150, -13, P_NICHELLE, P_RIGHT);
	}
	show_cur();
}

void Room68::talk_papagei() {
	hide_cur();
	auto_move(5, P_CHEWY);
	show_cur();

	ssi[2].X = 60;
	ssi[2].Y = 80;
	atds->set_split_win(2, &ssi[2]);
	start_ads_wait(18);
}

} // namespace Rooms
} // namespace Chewy
