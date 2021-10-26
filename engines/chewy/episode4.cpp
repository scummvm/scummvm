/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "chewy/defines.h"
#include "chewy/global.h"
#include "chewy/ani_dat.h"
#include "chewy/episode4.h"

namespace Chewy {

int16 r68HohesC;

void r66_entry(int16 eib_nr) {
	spieler.ScrollxStep = 2;
	spieler_mi[P_HOWARD].Mode = true;
	spieler_mi[P_NICHELLE].Mode = true;
}

void r66_exit(int16 eib_nr) {
	spieler.ScrollxStep = 1;
	switch (eib_nr) {
	case 98:
		spieler.PersonRoomNr[P_HOWARD] = 69;
		spieler.PersonRoomNr[P_NICHELLE] = 69;
		break;

	case 99:
		spieler.PersonRoomNr[P_HOWARD] = 68;
		spieler.PersonRoomNr[P_NICHELLE] = 68;
		break;

	case 100:
		spieler.PersonRoomNr[P_HOWARD] = 67;
		spieler.PersonRoomNr[P_NICHELLE] = 67;
		break;

	}
}

void r67_entry() {
	spieler.ScrollxStep = 2;
	SetUpScreenFunc = r67_setup_func;
	spieler_mi[P_HOWARD].Mode = true;
	spieler.DiaAMov = 7;
	if (spieler.R67KommodeAuf)
		det->show_static_spr(9);
	if (!spieler.R67PapageiWeg) {
		timer_nr[0] = room->set_timer(1, 10);
		det->set_static_ani(1, -1);
	} else
		det->show_static_spr(0);
	if (!flags.LoadGame) {
		hide_cur();
		set_person_pos(102, 132, P_CHEWY, P_RIGHT);
		set_person_pos(12, 100, P_HOWARD, P_RIGHT);
		set_person_pos(47, 106, P_NICHELLE, P_RIGHT);
		go_auto_xy(214, 112, P_NICHELLE, ANI_GO);
		auto_move(7, P_CHEWY);
		show_cur();
	}
}

void r67_exit() {
	spieler.ScrollxStep = 1;
	spieler.PersonRoomNr[P_HOWARD] = 66;
	spieler.PersonRoomNr[P_NICHELLE] = 66;
}

void r67_setup_func() {
	int16 x, y;
	calc_person_look();
	y = 117;
	if (spieler_vector[P_CHEWY].Xypos[0] > 320) {
		x = 178;
	} else {
		x = 137;
	}
	go_auto_xy(x, y, P_HOWARD, ANI_GO);
	go_auto_xy(214, 112, P_NICHELLE, ANI_GO);
}

int16 r67_use_grammo() {
	int16 action_flag = false;
	if (is_cur_inventar(SCHALL_INV)) {
		hide_cur();
		action_flag = true;
		auto_move(6, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		det->start_detail(0, 255, ANI_VOR);
		start_aad_wait(376, -1);
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		det->stop_detail(0);
		show_cur();
	}
	return action_flag;
}

void r67_look_brief() {
	hide_cur();
	auto_move(3, P_CHEWY);
	start_aad_wait(379, -1);
	show_cur();
}

int16 r67_use_kommode() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		hide_cur();
		if (!spieler.R67KommodeAuf) {
			action_flag = true;
			spieler.R67KommodeAuf = true;
			auto_move(6, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
			det->show_static_spr(9);
			atds->set_ats_str(400, 1, ATS_DATEI);
		} else if (!spieler.R67KostuemWeg) {
			action_flag = true;
			spieler.R67KostuemWeg = true;
			auto_move(6, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
			atds->set_ats_str(400, 2, ATS_DATEI);
			new_invent_2_cur(GALA_INV);
		}
		show_cur();
	}
	return action_flag;
}

void r67_kostuem_aad(int16 aad_nr) {
	hide_cur();
	if (spieler.DiaAMov != -1) {
		auto_move(spieler.DiaAMov, P_CHEWY);
	}
	start_aad_wait(aad_nr, -1);
	show_cur();
}

int16 r67_talk_papagei() {
	int16 action_flag = false;
	if (!spieler.R67PapageiWeg) {
		if (!spieler.inv_cur) {
			action_flag = true;
			hide_cur();
			room->set_timer_status(1, TIMER_STOP);
			if (menu_item == CUR_HOWARD) {
				show_cur();

				ssi[2].X = 270 - spieler.scrollx;
				ssi[2].Y = 10;
				atds->set_split_win(2, &ssi[2]);
				start_ads_wait(19);
				room->set_timer_status(1, TIMER_START);
			} else if (menu_item == CUR_NICHELLE) {
				start_aad_wait(380, -1);
				room->set_timer_status(1, TIMER_START);
			} else if (menu_item == CUR_TALK) {
				auto_move(5, P_CHEWY);
				show_cur();

				ssi[2].X = 270 - spieler.scrollx;
				ssi[2].Y = 10;
				atds->set_split_win(2, &ssi[2]);
				start_ads_wait(18);
				room->set_timer_status(1, TIMER_START);
			} else if (menu_item == CUR_USE) {
				hide_cur();
				action_flag = true;
				auto_move(4, P_CHEWY);
				start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
				spieler.R67PapageiWeg = true;
				det->stop_detail(1);
				det->del_static_ani(1);
				det->show_static_spr(0);
				invent_2_slot(PAPAGEI_INV);
				show_cur();
				atds->set_steuer_bit(394, ATS_AKTIV_BIT, ATS_DATEI);
			}
			show_cur();
		}
	}
	return action_flag;
}

void r68_entry() {
	spieler.ScrollxStep = 2;
	spieler.DiaAMov = 1;
	SetUpScreenFunc = r68_setup_func;
	r68HohesC = -1;
	if (spieler.R68Papagei) {
		det->show_static_spr(12);
		det->start_detail(21, 255, ANI_VOR);
	}
	if (!spieler.R68DivaWeg) {
		timer_nr[0] = room->set_timer(255, 10);
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

void r68_exit() {
	spieler.ScrollxStep = 1;
	spieler.PersonRoomNr[P_HOWARD] = 66;
	spieler.PersonRoomNr[P_NICHELLE] = 66;
}

void r68_setup_func() {
	int16 ho_x, ho_y;
	int16 ni_x, ni_y;
	int16 ch_x;

	switch (r68HohesC) {
	case 0:
		if (det->get_ani_status(r68HohesC) == false) {
			r68HohesC = 1;
			det->start_detail(r68HohesC, 1, ANI_VOR);
		}
		break;

	case 1:
		if (det->get_ani_status(r68HohesC) == false) {
			r68HohesC = 2;
			det->start_detail(r68HohesC, 3, ANI_VOR);
		}
		break;

	case 2:
		if (det->get_ani_status(r68HohesC) == false) {
			r68HohesC = -1;
			det->stop_detail(18);
			det->show_static_spr(3);
			uhr->reset_timer(timer_nr[0], 0);
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

void r68_look_kaktus() {
	hide_cur();
	auto_move(6, P_CHEWY);
	start_aad_wait(383, -1);
	show_cur();
}

void r68_talk_indigo() {
	r68_talk_indigo(-1);
}

void r68_talk_indigo(int16 aad_nr) {
	hide_cur();
	auto_move(3, P_CHEWY);
	room->set_timer_status(8, TIMER_STOP);
	det->del_static_ani(8);
	if (aad_nr == -1) {
		det->set_static_ani(9, -1);
		start_aad_wait(384 + (int16)spieler.R68IndigoDia, -1);
		spieler.R68IndigoDia ^= 1;
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

int16 r68_use_indigo() {
	int16 action_flag = false;
	hide_cur();
	if (is_cur_inventar(CLINT_500_INV)) {
		action_flag = true;
		if (spieler.R68Lied) {
			hide_cur();
			auto_move(3, P_CHEWY);
			auto_scroll(78, 0);
			del_inventar(spieler.AkInvent);
			r68_talk_indigo(394);
			cur_hide_flag = false;
			hide_cur();
			room->set_timer_status(8, TIMER_STOP);
			det->del_static_ani(8);
			det->stop_detail(8);
			start_detail_wait(13, 3, ANI_VOR);
			start_detail_wait(25, 1, ANI_VOR);
			det->set_static_ani(12, -1);
			r68_talk_indigo(398);
			room->set_timer_status(8, TIMER_STOP);
			det->del_static_ani(8);
			det->stop_detail(8);
			start_detail_wait(26, 1, ANI_VOR);
			room->set_timer_status(8, TIMER_START);
			det->set_static_ani(8, -1);
			new_invent_2_cur(KARTE_INV);
			spieler.R68KarteDa = true;
		} else {
			r68_talk_indigo(397);
		}
	} else if (is_cur_inventar(CLINT_1500_INV) || is_cur_inventar(CLINT_3000_INV)) {
		action_flag = true;
		start_aad_wait(393, -1);
	}
	show_cur();
	return action_flag;
}

void r68_talk_keeper() {
	hide_cur();
	auto_move(2, P_CHEWY);
	room->set_timer_status(20, TIMER_STOP);
	det->del_static_ani(20);
	start_detail_wait(15, 1, ANI_VOR);
	det->set_static_ani(16, -1);
	show_cur();
	ssi[3].X = spieler_vector[P_CHEWY].Xypos[0] - spieler.scrollx + spieler_mi[P_CHEWY].HotX;;
	ssi[3].Y = spieler_vector[P_CHEWY].Xypos[1] - spieler.scrolly;
	atds->set_split_win(3, &ssi[3]);
	start_ads_wait(20);
	cur_hide_flag = false;
	hide_cur();
	det->del_static_ani(16);
	start_detail_wait(15, 1, ANI_RUECK);
	room->set_timer_status(20, TIMER_START);
	det->set_static_ani(20, -1);
	show_cur();
}

int16 r68_use_papagei() {
	int16 action_flag = false;
	if (is_cur_inventar(PAPAGEI_INV)) {
		hide_cur();
		action_flag = true;
		spieler.R68Papagei = true;
		del_inventar(spieler.AkInvent);
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

void r68_calc_diva() {
	if (!spieler.R68DivaWeg) {
		if (!spieler.R68Papagei) {
			if (r68HohesC == -1) {
				r68HohesC = 0;
				det->hide_static_spr(3);
				det->start_detail(r68HohesC, 1, ANI_RUECK);
				det->start_detail(18, 255, ANI_VOR);
			}
		} else {
			if (!spieler.R68Gutschein) {

				if (!is_chewy_busy()) {
					hide_cur();
					spieler.R68Gutschein = true;
					auto_move(4, P_CHEWY);
					start_aad_wait(386, -1);
					start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
					new_invent_2_cur(BAR_GUT_INV);
					uhr->reset_timer(timer_nr[0], 0);
					show_cur();
				}
			}
		}
	}
}

int16 r68_use_keeper() {
	int16 action_flag = false;
	if (is_cur_inventar(BAR_GUT_INV)) {
		hide_cur();
		del_inventar(spieler.AkInvent);
		action_flag = true;
		auto_move(2, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		room->set_timer_status(20, TIMER_STOP);
		det->del_static_ani(20);
		start_ani_block(3, ablock39);
		room->set_timer_status(20, TIMER_START);
		det->set_static_ani(20, -1);
		new_invent_2_cur(B_MARY_INV);
		show_cur();
	}
	return action_flag;
}

int16 r68_use_diva() {
	short action_flag = false;
	hide_cur();
	if (is_cur_inventar(B_MARY_INV)) {
		del_inventar(spieler.AkInvent);
		action_flag = true;
		auto_move(4, P_CHEWY);
		start_detail_wait(4, 1, ANI_VOR);
		spieler.R68Gutschein = false;
	} else if (is_cur_inventar(B_MARY2_INV)) {
		del_inventar(spieler.AkInvent);
		action_flag = true;
		auto_move(4, P_CHEWY);
		det->hide_static_spr(3);
		spieler.R68DivaWeg = true;
		start_ani_block(2, ablock38);
		flic_cut(FCUT_083, FLC_MODE);
		flic_cut(FCUT_083, FLC_MODE);
		det->del_static_ani(18);
		start_detail_wait(7, 1, ANI_VOR);
		atds->set_steuer_bit(407, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_steuer_bit(412, ATS_AKTIV_BIT, ATS_DATEI);
		set_person_spr(P_RIGHT, P_CHEWY);
		start_aad_wait(402, -1);
	} else
		action_flag = r68_use_papagei();
	show_cur();
	return action_flag;
}

void r68_kostuem_aad(int16 aad_nr) {
	hide_cur();
	if (spieler.DiaAMov != -1) {
		auto_move(spieler.DiaAMov, P_CHEWY);
	}
	start_aad_wait(aad_nr, -1);
	if (!spieler.R68DivaWeg)
		start_aad_wait(388, -1);
	else if (!spieler.R67LiedOk)
		start_aad_wait(389, -1);
	else {
		SetUpScreenFunc = 0;
		del_inventar(spieler.AkInvent);
		go_auto_xy(150, -13, P_NICHELLE, ANI_WAIT);
		spieler.PersonHide[P_NICHELLE] = true;
		go_auto_xy(161, 59, P_HOWARD, ANI_GO);
		auto_move(4, P_CHEWY);
		start_aad_wait(390, -1);
		start_detail_wait(22, 1, ANI_VOR);
		spieler.PersonHide[P_HOWARD] = true;
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
		spieler.PersonHide[P_HOWARD] = false;
		det->stop_detail(27);
		det->set_static_ani(8, -1);
		start_aad_wait(392, -1);
		spieler.R68Lied = true;
		det->stop_detail(24);
		SetUpScreenFunc = r68_setup_func;
		auto_move(1, P_CHEWY);
		spieler.PersonHide[P_NICHELLE] = false;
		set_person_pos(150, -13, P_NICHELLE, P_RIGHT);
	}
	show_cur();
}

void r68_talk_papagei() {
	hide_cur();
	auto_move(5, P_CHEWY);
	show_cur();

	ssi[2].X = 60;
	ssi[2].Y = 80;
	atds->set_split_win(2, &ssi[2]);
	start_ads_wait(18);
}

void r69_entry(int16 eib_nr) {
	spieler.ScrollxStep = 2;
	spieler.ZoomXy[P_HOWARD][0] = 46;
	spieler.ZoomXy[P_HOWARD][1] = 90;
	spieler.ZoomXy[P_NICHELLE][0] = 46;
	spieler.ZoomXy[P_NICHELLE][1] = 90;
	zoom_horizont = 110;
	flags.ZoomMov = true;
	zoom_mov_fak = 3;
	SetUpScreenFunc = r69_setup_func;
	spieler.DiaAMov = 2;
	if (!flags.LoadGame) {
		hide_cur();
		set_person_pos(295, 118, P_CHEWY, P_RIGHT);
		set_person_pos(237, 101, P_NICHELLE, P_RIGHT);
		set_person_pos(347, 119, P_HOWARD, P_RIGHT);
		go_auto_xy(351, 97, P_HOWARD, ANI_WAIT);
		show_cur();
	}
}

void r69_exit(int16 eib_nr) {
	spieler.ScrollxStep = 1;
	switch (eib_nr) {
	case 97:
		spieler.PersonRoomNr[P_HOWARD] = 66;
		spieler.PersonRoomNr[P_NICHELLE] = 66;
		break;

	}
}

void r69_setup_func() {
	calc_person_look();
}

void r69_look_schild() {
	hide_cur();
	auto_move(1, P_CHEWY);
	start_aad_wait(382, -1);
	show_cur();
}

int16 r69_use_bruecke() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		hide_cur();
		action_flag = true;

		if (!spieler.R68KarteDa) {
			calc_person_dia(P_HOWARD);
		} else {
			auto_move(3, P_CHEWY);
			spieler_mi[P_CHEWY].Mode = true;
			go_auto_xy(253, 103, P_CHEWY, ANI_WAIT);
			go_auto_xy(141, 103, P_CHEWY, ANI_WAIT);
			go_auto_xy(82, 95, P_CHEWY, ANI_WAIT);
			go_auto_xy(46, 83, P_CHEWY, ANI_WAIT);
			go_auto_xy(255, 84, P_NICHELLE, ANI_WAIT);
			go_auto_xy(258, 86, P_HOWARD, ANI_GO);
			go_auto_xy(147, 84, P_NICHELLE, ANI_WAIT);
			go_auto_xy(140, 86, P_HOWARD, ANI_GO);
			go_auto_xy(94, 77, P_NICHELLE, ANI_WAIT);
			go_auto_xy(94, 82, P_HOWARD, ANI_GO);
			go_auto_xy(59, 68, P_NICHELLE, ANI_WAIT);
			spieler_mi[P_CHEWY].Mode = false;
			switch_room(66);
		}
		show_cur();
	}
	return action_flag;
}

} // namespace Chewy
