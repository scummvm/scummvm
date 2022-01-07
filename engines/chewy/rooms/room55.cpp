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
#include "chewy/rooms/room55.h"

namespace Chewy {
namespace Rooms {

void Room55::entry() {
	_G(spieler).ScrollxStep = 2;
	SetUpScreenFunc = setup_func;
	_G(zoom_horizont) = 140;
	flags.ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(spieler).R55ExitDia = 0;
	_G(spieler).ZoomXy[P_HOWARD][0] = 20;
	_G(spieler).ZoomXy[P_HOWARD][1] = 22;
	if (!_G(spieler).R54FputzerWeg)
		det->start_detail(6, 255, ANI_VOR);

	if ((_G(spieler).R55RaumOk) || (!_G(spieler).R55RaumOk && _G(spieler).R55ScriptWeg))
		det->hide_static_spr(0);
	if (!_G(spieler).R55SekWeg) {
		det->start_detail(8, 255, ANI_VOR);
		det->start_detail(9, 255, ANI_VOR);
		_G(spieler).R55Entry ^= 1;
		_G(timer_nr)[0] = room->set_timer(19 + (int16)_G(spieler).R55Entry, 10);
		det->set_static_ani(19 + (int16)_G(spieler).R55Entry, -1);
	}
	spieler_mi[P_HOWARD].Mode = true;
	if (_G(spieler).R55Job) {
		rock2mans();
		_G(spieler).PersonRoomNr[P_HOWARD] = 54;
	}
	if (_G(spieler).ChewyAni == CHEWY_JMANS) {
		set_person_pos(51, 75, P_CHEWY, P_RIGHT);
		room->set_zoom(10);
	}
	if (_G(spieler).R55EscScriptOk && !_G(spieler).R55RaumOk)
		det->show_static_spr(0);
	if (!flags.LoadGame) {
		if (_G(spieler).R55Location) {
			_G(spieler).scrollx = 136;
			set_person_pos(400, 82, P_CHEWY, P_RIGHT);
			start_detail_wait(3, 1, ANI_VOR);
			_G(timer_nr)[0] = room->set_timer(4, 10);
			det->set_static_ani(4, -1);
			atds->del_steuer_bit(340, ATS_AKTIV_BIT, ATS_DATEI);
		} else {
			_G(spieler).scrollx = 0;
			if (_G(spieler).PersonRoomNr[P_HOWARD] == 55) {
				set_person_pos(21, 77, P_HOWARD, P_RIGHT);
			}
			det->show_static_spr(8);
			auto_move(1, P_CHEWY);
			hide_cur();
			det->hide_static_spr(8);
			if (_G(spieler).R55SekWeg && !_G(spieler).R55Job) {

				set_person_spr(P_LEFT, P_CHEWY);
				if (_G(spieler).ChewyAni == CHEWY_ROCKER) {
					verleger_mov(0);
					talk_line();
					_G(spieler).R54Schild = true;
				} else {
					verleger_mov(1);
					get_job();
				}
			}
			show_cur();
		}
	}
}

void Room55::xit(int16 eib_nr) {
	if (eib_nr == 90) {
		mans2rock();
		if (_G(spieler).PersonRoomNr[P_HOWARD] == 55) {
			_G(spieler).PersonRoomNr[P_HOWARD] = 54;
			spieler_mi[P_HOWARD].Mode = false;
		}
	}
	_G(spieler).ScrollxStep = 1;
}

int16 Room55::use_stapel1() {
	int16 action_ret = false;
	if (!_G(spieler).inv_cur) {
		if (!_G(spieler).R55ScriptWeg) {
			action_ret = true;
			_G(spieler).R55ScriptWeg = true;
			auto_move(4, P_CHEWY);
			det->hide_static_spr(0);
			auto_move(5, P_CHEWY);
			det->show_static_spr(1);
			atds->set_ats_str(354, 1, ATS_DATEI);
			atds->set_ats_str(355, 1, ATS_DATEI);
		} else if (_G(spieler).R55EscScriptOk && !_G(spieler).R55RaumOk) {
			action_ret = true;
			start_aad_wait(333, -1);
		}
	} else if (is_cur_inventar(MANUSKRIPT_INV)) {
		action_ret = true;
		if (_G(spieler).R55ScriptWeg) {
			auto_move(4, P_CHEWY);
			_G(spieler).R55EscScriptOk = true;
			det->show_static_spr(0);
			del_inventar(_G(spieler).AkInvent);
			atds->set_ats_str(354, 2, ATS_DATEI);
		} else
			start_aad_wait(326, -1);
	}
	return action_ret;
}

int16 Room55::use_stapel2() {
	int16 action_ret = false;
	if (is_cur_inventar(MANUSKRIPT_INV)) {
		action_ret = true;
		start_aad_wait(327, -1);
	}
	return action_ret;
}

int16 Room55::use_telefon() {
	int16 i;
	int16 delay;
	int16 tmp_delay;
	int16 action_ret = false;
	if (!_G(spieler).inv_cur) {
		action_ret = true;
		if (_G(spieler).R55EscScriptOk) {
			if (!_G(spieler).R55RaumOk) {
				hide_cur();
				_G(spieler).R55RaumOk = true;
				auto_move(6, P_CHEWY);
				_G(spieler).PersonHide[P_CHEWY] = true;
				start_detail_wait(10, 1, ANI_VOR);
				det->start_detail(11, 255, ANI_VOR);
				start_aad_wait(329, -1);
				det->stop_detail(11);
				start_detail_wait(10, 1, ANI_RUECK);
				_G(spieler).PersonHide[P_CHEWY] = false;
				auto_move(7, P_CHEWY);
				flags.NoScroll = true;
				auto_scroll(0, 0);
				start_aad_wait(330, -1);
				det->show_static_spr(8);
				start_detail_wait(0, 1, ANI_VOR);
				det->start_detail(1, 255, ANI_VOR);
				start_aad_wait(331, -1);
				det->stop_detail(1);
				start_detail_wait(2, 1, ANI_VOR);
				det->hide_static_spr(0);
				det->hide_static_spr(8);
				atds->set_ats_str(354, 1, ATS_DATEI);
				flic_cut(FCUT_071, FLC_MODE);
				_G(spieler).PersonRoomNr[P_HOWARD] = 55;
				set_person_pos(178, 75, P_CHEWY, P_LEFT);
				set_person_pos(66, 73, P_HOWARD, P_RIGHT);
				SetUpScreenFunc = 0;
				det->set_static_ani(18, -1);
				start_aad_wait(334, -1);
				start_aad_wait(335, -1);
				start_aad_wait(336, -1);
				tmp_delay = _G(spieler).DelaySpeed;
				for (i = 0; i < 7; i++) {
					delay = tmp_delay;
					if (!_G(spieler).scrollx)
						_G(spieler).scrollx = 8;
					else
						_G(spieler).scrollx = 0;
					while (delay) {
						out->skip_frame(1);
						--delay;
					}
					set_up_screen(DO_SETUP);
				}
				flic_cut(FCUT_072, FLC_MODE);
				flags.NoScroll = false;
				invent_2_slot(SACKGELD_INV);
				invent_2_slot(EINLAD_INV);
				del_invent_slot(LEDER_INV);
				load_chewy_taf(CHEWY_ROCKER);
				_G(spieler).PersonRoomNr[P_HOWARD] = 54;
				spieler_mi[P_HOWARD].Mode = false;
				show_cur();
				_G(spieler).R55R54First = true;
				_G(spieler).R55ExitDia = 337;
				_G(spieler).room_e_obj[89].Attribut = 255;
				switch_room(54);
			} else {
				start_aad_wait(332, -1);
			}
		} else {
			start_aad_wait(328, -1);
		}
	}
	return action_ret;
}

void Room55::get_job() {
	int16 tmp_scrollx;
	int16 tmp_scrolly;
	int16 r_nr;
	_G(spieler).PersonHide[P_CHEWY] = true;
	tmp_scrollx = _G(spieler).scrollx;
	tmp_scrolly = _G(spieler).scrolly;
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	switch_room(61);
	show_cur();
	start_ads_wait(15);
	_G(spieler).PersonHide[P_CHEWY] = false;
	flags.LoadGame = true;
	_G(spieler).scrollx = tmp_scrollx;
	_G(spieler).scrolly = tmp_scrolly;
	if (_G(spieler).R55Job) {
		r_nr = 55;
		atds->del_steuer_bit(357, ATS_AKTIV_BIT, ATS_DATEI);
		atds->del_steuer_bit(354, ATS_AKTIV_BIT, ATS_DATEI);
		atds->del_steuer_bit(355, ATS_AKTIV_BIT, ATS_DATEI);
	} else {
		r_nr = 54;
		mans2rock();
	}
	_G(spieler).PersonRoomNr[P_HOWARD] = r_nr;
	spieler_mi[P_HOWARD].Mode = true;
	if (r_nr == 54)
		flags.LoadGame = false;
	set_person_pos(118, 96, P_CHEWY, P_LEFT);
	switch_room(r_nr);
	flags.LoadGame = false;

}

void Room55::mans2rock() {
	if (_G(spieler).ChewyAni == CHEWY_JMANS) {
		_G(spieler).R55ExitDia = 317;
		load_chewy_taf(CHEWY_ROCKER);
		del_invent_slot(LEDER_INV);
		invent_2_slot(JMKOST_INV);
	}
}

void Room55::rock2mans() {
	if (_G(spieler).ChewyAni == CHEWY_ROCKER) {
		load_chewy_taf(CHEWY_JMANS);
		del_invent_slot(JMKOST_INV);
		invent_2_slot(LEDER_INV);
	}
}

void Room55::verleger_mov(int16 mode) {
	start_detail_wait(7, 1, ANI_VOR);
	det->show_static_spr(11);
	start_detail_wait(13, 1, ANI_VOR);
	start_detail_wait(14, 1, ANI_VOR);
	if (_G(spieler).ChewyAni == CHEWY_JMANS)
		start_spz(CH_JM_TITS, 1, ANI_VOR, P_CHEWY);
	if (mode)
		start_detail_wait(15, 1, ANI_VOR);
	det->set_static_ani(16, -1);
}

void Room55::strasse(int16 mode) {
	if (!_G(spieler).inv_cur || mode) {
		room->set_timer_status(4, TIMER_STOP);
		det->del_static_ani(4);
		det->stop_detail(4);
		atds->set_steuer_bit(340, ATS_AKTIV_BIT, ATS_DATEI);
		start_detail_wait(3, 1, ANI_RUECK);

		_G(spieler).scrollx = 0;
		switch_room(54);
	}
}

int16 Room55::use_kammeraus() {
	int16 action_ret = false;
	if (_G(spieler).R55Location) {
		if (is_cur_inventar(KILLER_INV)) {
			action_ret = true;
			if (_G(spieler).R52KakerWeg) {
				if (!_G(spieler).R55SekWeg) {
					hide_cur();
					room->set_timer_status(4, TIMER_STOP);
					det->del_static_ani(4);
					det->stop_detail(4);
					start_detail_wait(5, 1, ANI_VOR);
					det->show_static_spr(10);
					flc->set_flic_user_function(cut_serv);
					flic_cut(FCUT_070, FLC_MODE);
					flc->remove_flic_user_function();
					room->set_timer_status(19 + (int16)_G(spieler).R55Entry, TIMER_STOP);
					det->stop_detail(19 + (int16)_G(spieler).R55Entry);
					det->del_static_ani(19 + (int16)_G(spieler).R55Entry);
					det->hide_static_spr(10);
					_G(spieler).R55ExitDia = 322;
					_G(spieler).R55SekWeg = true;
					atds->set_ats_str(KILLER_INV, 1, INV_ATS_DATEI);
					atds->set_steuer_bit(352, ATS_AKTIV_BIT, ATS_DATEI);
					strasse(1);
					show_cur();
				}
			} else {
				start_aad_wait(325, -1);
			}
		}
	}
	return action_ret;
}

int16 Room55::cut_serv(int16 frame) {
	if (frame < 10)
		det->plot_static_details(136, 0, 10, 10);
	return 0;
}

void Room55::setup_func() {
	int16 x, y;
	int16 ch_x;
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 55) {
		calc_person_look();
		ch_x = spieler_vector[P_CHEWY].Xypos[0];
		if (ch_x < 100) {
			x = 62;
			y = 101;
		} else if (ch_x < 187) {
			x = 139;
			y = 119;
		} else if (ch_x < 276) {
			x = 235;
			y = 119;
		} else {
			x = 318;
			y = 110;
		}
		go_auto_xy(x, y, P_HOWARD, ANI_GO);
	}
}

void Room55::talk_line() {
	int16 aad_nr = 0;
	if (_G(spieler).R55Location == 0) {
		if (_G(spieler).ChewyAni == CHEWY_ROCKER) {
			hide_cur();
			if (!_G(spieler).R55SekWeg) {
				auto_move(2, P_CHEWY);
				flags.NoScroll = true;
				auto_scroll(136, 0);
				aad_nr = 320;
				_G(spieler).R55ExitDia = 321;
				room->set_timer_status(19 + (int16)_G(spieler).R55Entry, TIMER_STOP);
				det->stop_detail(19 + (int16)_G(spieler).R55Entry);
				det->del_static_ani(19 + (int16)_G(spieler).R55Entry);
				start_detail_wait(22, 1, ANI_VOR);
				det->set_static_ani(21, -1);
			} else if (!_G(spieler).R55RaumOk) {
				auto_move(3, P_CHEWY);
				aad_nr = 323;
				_G(spieler).R55ExitDia = 324;
			}
			start_aad_wait(aad_nr, -1);
			if (_G(spieler).R55SekWeg) {
				det->del_static_ani(16);
				start_detail_wait(14, 1, ANI_VOR);
			} else {
				det->del_static_ani(21);
				start_detail_wait(22, 1, ANI_VOR);
			}
			flags.NoScroll = false;
			_G(spieler).PersonRoomNr[P_HOWARD] = 54;
			spieler_mi[P_HOWARD].Mode = false;
			switch_room(54);
			show_cur();
		}
	}
}

} // namespace Rooms
} // namespace Chewy
