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
#include "chewy/rooms/room37.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK31[3] = {
	{ 0, 1, ANI_VOR, ANI_GO, 0 },
	{ 1, 2, ANI_VOR, ANI_GO, 0 },
	{ 5, 1, ANI_RUECK, ANI_WAIT, 0 },
};


void Room37::entry() {
	_G(zoom_horizont) = 100;
	flags.ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	SetUpScreenFunc = setup_func;
	obj->show_sib(74);
	obj->show_sib(75);

	if (!flags.LoadGame) {
		_G(spieler).scrollx = 124;
		set_person_pos(219, 66, P_CHEWY, P_RIGHT);
	}

	if (!_G(spieler).R37Kloppe) {
		_G(timer_nr)[1] = room->set_timer(7, 5);
		det->set_static_ani(7, -1);

		if (!_G(spieler).R37HundScham) {
			_G(timer_nr)[0] = room->set_timer(3, 4);
			det->set_static_ani(3, -1);
		}
	}

	if (_G(spieler).R37Gebiss) {
		det->hide_static_spr(9);

		if (_G(spieler).R37Kloppe) {
			det->hide_static_spr(8);
		} else if (_G(spieler).R37HundScham) {
			det->show_static_spr(0);
		}
	}
}

void Room37::gedAction(int index) {
	if (!index) {
		dog_bell();

	} else if (index == 1) {
		if (_G(spieler).R37Kloppe && !_G(spieler).R37Mes) {
			stop_person(P_CHEWY);
			_G(spieler).R37Mes = true;
			start_spz(CH_TALK6, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(142, -1);
		}
	}
}

void Room37::setup_func() {
	if (_G(maus_links_click) &&
			!_G(spieler).R37Kloppe &&
			menu_item == CUR_WALK) {
		if ((minfo.x + _G(spieler).scrollx > 380 && minfo.y > 120) ||
			(minfo.x + _G(spieler).scrollx > 482)) {
			auto_move(7, P_CHEWY);
			_G(maus_links_click) = false;
		}
	}
}

short Room37::use_wippe() {
	int16 action_flag = false;

	if (_G(spieler).inv_cur) {
		action_flag = true;

		if (is_cur_inventar(H_FUTTER_INV)) {
			hide_cur();
			auto_move(0, P_CHEWY);
			flags.NoScroll = true;
			auto_scroll(129, 0);
			start_spz(CH_TALK6, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(159, -1);
			del_inventar(_G(spieler).AkInvent);
			flc->set_flic_user_function(cut_serv1);
			flic_cut(FCUT_047, FLC_MODE);
			flc->remove_flic_user_function();
			flags.NoScroll = false;
			show_cur();
			_G(spieler).scrollx = 269;
			set_person_pos(388, 119, P_CHEWY, P_RIGHT);
			switch_room(29);
			_G(maus_links_click) = false;

		} else {
			start_spz(CH_TALK5, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(160, -1);
		}
	}

	return action_flag;
}

int16 Room37::cut_serv1(int16 frame) {
	int16 static_nr;
	int16 static_nr1;

	if (!_G(spieler).R37Kloppe) {
		if (!_G(spieler).R37Gebiss) {
			static_nr = 9;
			static_nr1 = 11;
			det->show_static_spr(11);
		} else {
			static_nr = 8;
			static_nr1 = 0;
		}

		det->plot_static_details(_G(spieler).scrollx, _G(spieler).scrolly, static_nr, static_nr);
		det->plot_static_details(_G(spieler).scrollx, _G(spieler).scrolly, static_nr1, static_nr1);
	}

	det->plot_static_details(_G(spieler).scrollx, _G(spieler).scrolly, 7, 7);
	det->plot_static_details(_G(spieler).scrollx, _G(spieler).scrolly, 14, 14);
	return 0;
}

int16 Room37::cut_serv2(int16 frame) {
	static const int16 STATIC_NR[] = { 7, 14, 12, 10 };
	short i;

	det->show_static_spr(12);
	det->show_static_spr(10);
	for (i = 0; i < 4; i++)
		det->plot_static_details(_G(spieler).scrollx, _G(spieler).scrolly, STATIC_NR[i], STATIC_NR[i]);

	return 0;
}

int16 Room37::use_glas() {
	int16 action_flag = false;

	if (!_G(spieler).R37Gebiss) {
		if (is_cur_inventar(ANGEL2_INV)) {
			action_flag = true;
			flags.NoScroll = true;
			hide_cur();
			auto_move(5, P_CHEWY);
			flags.NoScroll = true;
			auto_scroll(146, 0);
			start_spz(CH_TALK6, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(147, -1);
			del_inventar(_G(spieler).AkInvent);
			flc->set_flic_user_function(cut_serv2);
			flic_cut(FCUT_048, FLC_MODE);
			flc->remove_flic_user_function();
			flic_cut(FCUT_049, FLC_MODE);
			invent_2_slot(GEBISS_INV);
			det->hide_static_spr(9);
			atds->set_ats_str(250, 1, ATS_DATEI);
			atds->set_ats_str(256, 1, ATS_DATEI);
			atds->del_steuer_bit(251, ATS_AKTIV_BIT, ATS_DATEI);
			obj->show_sib(SIB_HFUTTER2_R37);
			_G(spieler).R37Gebiss = true;
			start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(146, -1);
			show_cur();
			flags.NoScroll = false;
			det->play_sound(3, 0);
		} else {
			auto_move(4, P_CHEWY);
		}
	}

	return action_flag;
}

void Room37::dog_bell() {
	int16 dia_nr = -1;
	int16 ani_nr = 0;
	hide_cur();

	if (!flags.AutoAniPlay) {
		flags.AutoAniPlay = true;

		if (!_G(spieler).R37Gebiss) {
			stop_person(P_CHEWY);
			flags.ChAutoMov = false;
			set_person_spr(P_LEFT, P_CHEWY);
			flags.NoScroll = true;
			auto_scroll(178, 0);
			disable_timer();
			det->stop_detail(3);
			det->del_static_ani(3);
			start_detail_wait(5, 1, ANI_VOR);
			det->hide_static_spr(9);
			start_detail_wait(6, 1, ANI_VOR);
			_G(spieler).PersonHide[P_CHEWY] = true;
			det->start_detail(11, 255, ANI_VOR);
			flic_cut(FCUT_050, FLC_MODE);
			start_detail_wait(6, 1, ANI_RUECK);
			det->stop_detail(11);
			set_person_pos(326, 85, P_CHEWY, P_LEFT);
			_G(spieler).PersonHide[P_CHEWY] = false;
			det->show_static_spr(9);
			start_ani_block(3, ABLOCK31);
			det->set_static_ani(3, -1);
			det->enable_sound(3, 0);
			det->play_sound(3, 0);
			enable_timer();
			dia_nr = 149;
			ani_nr = CH_TALK12;

		} else if (!_G(spieler).R37HundScham) {
			stop_person(P_CHEWY);
			set_person_spr(P_LEFT, P_CHEWY);
			flags.NoScroll = true;
			auto_scroll(178, 0);
			room->set_timer_status(3, TIMER_STOP);
			det->del_static_ani(3);
			det->stop_detail(3);
			start_detail_wait(4, 1, ANI_VOR);
			flic_cut(FCUT_051, FLC_MODE);
			_G(spieler).scrollx = 104;
			flic_cut(FCUT_054, FLC_MODE);
			det->show_static_spr(0);

			_G(spieler).R37HundScham = true;
			dia_nr = 148;
			ani_nr = CH_TALK6;
		}

		flags.AutoAniPlay = false;

		if (dia_nr != -1) {
			start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(dia_nr, -1);
		}
	}

	flags.NoScroll = false;
	show_cur();
}

void Room37::talk_hahn() {
	hide_cur();
	auto_move(7, P_CHEWY);
	show_cur();

	if (!_G(spieler).R37TransHahn) {
		_G(cur_hide_flag) = 0;
		hide_cur();
		start_aad_wait(145, -1);
		show_cur();
	} else {
		hahn_dia();
	}
}

void Room37::use_hahn() {
	if (is_cur_inventar(TRANSLATOR_INV)) {
		hide_cur();
		auto_move(7, P_CHEWY);
		_G(spieler).R37TransHahn = true;
		start_spz_wait(CH_TRANS, 1, false, P_CHEWY);
		flic_cut(FCUT_052, FLC_MODE);
		cur_2_inventory();
		menu_item = CUR_TALK;
		cursor_wahl(menu_item);
		show_cur();
		hahn_dia();

	} else if (_G(spieler).R37TransHahn) {
		if (is_cur_inventar(GEBISS_INV)) {
			_G(spieler).R37Kloppe = true;
			hide_cur();
			auto_move(6, P_CHEWY);
			load_room_music(256);
			room->set_timer_status(7, TIMER_STOP);
			det->stop_detail(7);
			det->del_static_ani(7);
			det->start_detail(9, 1, ANI_VOR);
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			del_inventar(GEBISS_INV);
			flags.NoScroll = true;
			auto_scroll(177, 0);

			while (det->get_ani_status(9)) {
				set_up_screen(DO_SETUP);
				SHOULD_QUIT_RETURN;
			}

			det->start_detail(4, 1, ANI_VOR);
			det->hide_static_spr(0);
			det->start_detail(10, 10, ANI_VOR);
			auto_move(8, P_CHEWY);
			flic_cut(FCUT_053, FLC_MODE);
			det->stop_detail(10);
			_G(spieler).scrollx = 320;
			flags.NoScroll = false;
			atds->set_steuer_bit(251, ATS_AKTIV_BIT, ATS_DATEI);
			atds->set_steuer_bit(250, ATS_AKTIV_BIT, ATS_DATEI);
			atds->set_steuer_bit(256, ATS_AKTIV_BIT, ATS_DATEI);
			det->hide_static_spr(8);
			start_spz(CH_TALK5, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(141, -1);
			obj->add_inventar(EIER_INV, &room_blk);
			inventory_2_cur(EIER_INV);
			show_cur();
		}
	} else if (_G(spieler).inv_cur) {
		start_aad_wait(143, -1);
	}
}

void Room37::hahn_dia() {
	int16 tmp_scrollx;
	int16 tmp_scrolly;

	_G(spieler).PersonHide[P_CHEWY] = true;
	tmp_scrollx = _G(spieler).scrollx;
	tmp_scrolly = _G(spieler).scrolly;
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	switch_room(38);
	start_ads_wait(9);
	_G(spieler).PersonHide[P_CHEWY] = false;
	flags.LoadGame = true;
	_G(spieler).scrollx = tmp_scrollx;
	_G(spieler).scrolly = tmp_scrolly;
	switch_room(37);
	flags.LoadGame = false;
}

} // namespace Rooms
} // namespace Chewy
