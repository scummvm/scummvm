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
#include "chewy/rooms/room40.h"
#include "chewy/rooms/room43.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

#define SPEED 3
#define POLICE_LEFT 8
#define POLICE_OFFEN 9
#define POLICE_WUERG 10
#define POLICE_RIGHT 11
#define POLICE_FLASCHE 16

void Room40::entry(int16 eib_nr) {
	_G(spieler).R40HoUse = false;
	_G(zoom_horizont) = 130;
	_G(spieler).ScrollxStep = 2;

	if (_G(spieler).R40Geld) {
		det->del_static_ani(6);
		room->set_timer_status(6, TIMER_STOP);
	}

	if (_G(spieler).R40HaendlerOk) {
		det->del_static_ani(4);
		room->set_timer_status(4, TIMER_STOP);
	}

	if (_G(spieler).R40PoliceWeg == false) {
		_G(timer_nr)[0] = room->set_timer(255, 10);
		atds->del_steuer_bit(275, ATS_AKTIV_BIT, ATS_DATEI);
	} else {
		det->hide_static_spr(15);
	}

	_G(spieler).R40PoliceAniStatus = 255;
	_G(spieler).R40PoliceStart = false;
	spieler_mi[P_HOWARD].Mode = true;

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 41)
		_G(spieler).PersonRoomNr[P_HOWARD] = 40;

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 40) {
		_G(spieler).ZoomXy[P_HOWARD][0] = 40;
		_G(spieler).ZoomXy[P_HOWARD][1] = 40;

		if (!flags.LoadGame) {
			switch (eib_nr) {
			case 69:
				set_person_pos(30, 105, P_HOWARD, P_RIGHT);
				go_auto_xy(158, 99, P_HOWARD, ANI_GO);
				break;

			case 73:
			case 74:
			case 87:
				set_person_pos(158, 93, P_HOWARD, P_LEFT);
				break;
			default:
				break;
			}
		}
	}

	SetUpScreenFunc = setup_func;
	if (_G(spieler).R40TrainMove)
		move_train(0);
}

void Room40::xit(int16 eib_nr) {
	hide_cur();
	_G(spieler).ScrollxStep = 1;
	_G(spieler).R40PoliceAb = false;
	_G(spieler).R40HoUse = false;
	stop_spz();
	SetUpScreenFunc = nullptr;

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 40) {
		if (eib_nr == 70 || eib_nr == 77) {
			_G(spieler).PersonRoomNr[P_HOWARD] = 28;

		} else if (eib_nr == 72) {
			if ((obj->check_inventar(HOTEL_INV) && obj->check_inventar(TICKET_INV) && _G(spieler).R42BriefOk && _G(spieler).R28Manuskript)
				|| _G(spieler).R40TrainOk) {
				_G(spieler).R40TrainOk = true;
				_G(spieler).PersonRoomNr[P_HOWARD] = 45;
				_G(spieler).room_e_obj[72].Exit = 45;
				obj->hide_sib(SIB_MUENZE_R40);

				uhr->disable_timer();
				_G(out)->ausblenden(0);
				hide_person();
				set_up_screen(DO_SETUP);
				_G(out)->einblenden(pal, 0);
				uhr->enable_timer();

				_G(maus_links_click) = false;
				start_aad_wait(238, -1);
				move_train(1);
				register_cutscene(15);
				
				flags.NoPalAfterFlc = true;
				flic_cut(FCUT_073, CFO_MODE);

				if (_G(spieler).ChewyAni != CHEWY_ROCKER)
					_G(spieler).PersonGlobalDia[1] = 10023;

				cur_2_inventory();
				remove_inventory(57);
				_G(spieler).PersonDiaRoom[P_HOWARD] = true;
				show_person();

			} else {
				_G(spieler).PersonRoomNr[P_HOWARD] = 42;
			}
		} else {
			_G(spieler).PersonRoomNr[P_HOWARD] = 41;
		}
	}

	spieler_mi[P_HOWARD].Mode = false;
	show_cur();
}

bool Room40::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0])
		_G(spieler).R40PoliceStart = true;
	else
		return true;

	return false;
}

void Room40::move_train(int16 mode) {
	_G(spieler).R40TrainMove = false;
	hide_cur();
	auto_move(9, P_CHEWY);
	flags.NoScroll = true;
	auto_scroll(232, 0);

	if (!mode)
		start_aad_wait(206, -1);

	int16 lx = -40;
	int16 ax = -230;

	det->start_detail(7, 20, ANI_VOR);
	det->show_static_spr(11);

	if (mode && _G(spieler).ChewyAni == CHEWY_PUMPKIN)
		det->show_static_spr(12);

	g_engine->_sound->playSound(7, 0);
	int16 delay = 0;

	while (ax < 560) {
		det->set_detail_pos(7, lx, 46);
		det->set_static_pos(11, ax, 62, false, false);

		if (mode && _G(spieler).ChewyAni == CHEWY_PUMPKIN)
			det->set_static_pos(12, ax, 62, false, true);

		if (!delay) {
			lx += SPEED;
			ax += SPEED;
			delay = _G(spieler).DelaySpeed / 2;
		} else {
			--delay;
		}

		set_up_screen(DO_SETUP);
		SHOULD_QUIT_RETURN;
	}

	det->stop_detail(7);
	det->hide_static_spr(11);
	det->hide_static_spr(12);

	if (!mode)
		start_aad_wait(207, -1);
	if (!mode)
		auto_scroll(180, 0);

	flags.NoScroll = false;
	show_cur();
}

void Room40::setup_func() {
	if (!_G(spieler).R40HoUse && _G(spieler).PersonRoomNr[P_HOWARD] == 40) {
		calc_person_look();

		int16 x, y;
		const int16 sp_x = spieler_vector[P_CHEWY].Xypos[0];
		if (sp_x > 170 && sp_x < 255) {
			x = 248;
			y = 97;
		} else if (sp_x > 255 && sp_x < 350) {
			x = 310;
			y = 90;
		} else if (sp_x > 350) {
			x = 428;
			y = 90;
		} else if (sp_x < 170) {
			x = 166;
			y = 99;
		} else {
			x = spieler_vector[P_HOWARD].Xypos[0];
			y = spieler_vector[P_HOWARD].Xypos[1];
		}

		go_auto_xy(x, y, P_HOWARD, ANI_GO);
	}

	if (_G(spieler).R40PoliceWeg == false) {
		if (_G(spieler).R40PoliceStart) {
			_G(spieler).R40PoliceStart = false;
			_G(spieler).R40PoliceAniStatus = POLICE_LEFT;
			room->set_timer_status(255, TIMER_STOP);
			uhr->reset_timer(_G(timer_nr)[0], 0);
			det->hide_static_spr(15);
			det->start_detail(POLICE_LEFT, 1, ANI_VOR);
			atds->set_steuer_bit(275, ATS_AKTIV_BIT, ATS_DATEI);
		}

		switch (_G(spieler).R40PoliceAniStatus) {
		case POLICE_LEFT:
			if (det->get_ani_status(POLICE_LEFT) == false) {
				det->start_detail(POLICE_OFFEN, 1, ANI_VOR);
				_G(spieler).R40PoliceAniStatus = POLICE_OFFEN;
			}
			break;

		case POLICE_OFFEN:
			if (det->get_ani_status(POLICE_OFFEN) == false) {
				det->show_static_spr(0);
				det->start_detail(POLICE_FLASCHE, 1, ANI_VOR);
				_G(spieler).R40PoliceAniStatus = POLICE_FLASCHE;
			}
			break;

		case POLICE_RIGHT:
			if (det->get_ani_status(POLICE_RIGHT) == false) {
				det->show_static_spr(15);
				_G(spieler).R40PoliceAniStatus = 255;
				room->set_timer_status(255, TIMER_START);
				uhr->reset_timer(_G(timer_nr)[0], 0);
				atds->del_steuer_bit(275, ATS_AKTIV_BIT, ATS_DATEI);
			}
			break;

		case POLICE_FLASCHE:
			if (det->get_ani_status(POLICE_FLASCHE) == false) {
				det->hide_static_spr(0);
				if (_G(spieler).R40DuengerTele) {
					hide_cur();
					_G(spieler).R40PoliceWeg = true;
					det->start_detail(17, 255, ANI_VOR);
					start_aad_wait(226, -1);
					det->stop_detail(17);
					_G(spieler).R40HoUse = true;
					person_end_phase[P_CHEWY] = P_RIGHT;
					start_detail_wait(10, 1, ANI_VOR);
					person_end_phase[P_HOWARD] = P_RIGHT;
					start_aad_wait(224, -1);
					_G(spieler).R40PoliceWeg = true;
					show_cur();

					flags.MausLinks = false;
					flags.MainInput = true;
					_G(spieler).R40HoUse = false;
					atds->set_steuer_bit(276, ATS_AKTIV_BIT, ATS_DATEI);

				} else {
					det->start_detail(POLICE_RIGHT, 1, ANI_VOR);
					_G(spieler).R40PoliceAniStatus = POLICE_RIGHT;
				}
			}
			break;

		default:
			break;
		}
	}
}

int16 Room40::use_mr_pumpkin() {
	int16 action_ret = false;

	if (menu_item != CUR_HOWARD) {
		hide_cur();

		if (!_G(spieler).inv_cur) {
			action_ret = use_schalter(205);

		} else {
			switch (_G(spieler).AkInvent) {
			case CENT_INV:
				action_ret = true;
				auto_move(5, P_CHEWY);
				del_inventar(_G(spieler).AkInvent);
				start_detail_wait(15, 1, ANI_VOR);
				start_spz(CH_PUMP_TALK, 255, ANI_VOR, P_CHEWY);
				start_aad_wait(200, -1);
				break;

			case RECORDER_INV:
				action_ret = true;

				if (_G(spieler).R39TvRecord == 6) {
					if (_G(spieler).R40PoliceWeg == false)
						use_schalter(227);
					else {
						hide_cur();
						auto_move(8, P_CHEWY);
						start_spz_wait(CH_PUMP_GET1, 1, false, P_CHEWY);
						del_inventar(_G(spieler).AkInvent);
						_G(out)->ausblenden(1);
						Room43::catch_pg();
						remove_inventory(LIKOER_INV);
						obj->add_inventar(LIKOER2_INV, &room_blk);
						inventory_2_cur(LIKOER2_INV);
						switch_room(40);
						start_aad_wait(236, -1);
					}
				} else {
					start_aad_wait(228 + _G(spieler).R39TvRecord, -1);
				}
				break;

			default:
				break;
			}
		}

		show_cur();
	}

	return action_ret;
}

int16 Room40::use_schalter(int16 aad_nr) {
	int16 action_flag = false;

	if (menu_item != CUR_HOWARD &&_G(spieler).R40PoliceWeg == false) {
		action_flag = true;

		hide_cur();
		auto_move(8, P_CHEWY);

		if (_G(spieler).R40PoliceAniStatus != 255) {
			start_spz(CH_PUMP_TALK, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(204, -1);

			while (_G(spieler).R40PoliceAniStatus != 255) {
				set_up_screen(DO_SETUP);
				SHOULD_QUIT_RETURN0;
			}
		}

		room->set_timer_status(255, TIMER_STOP);
		_G(spieler).R40PoliceStart = false;
		stop_spz();
		start_spz_wait(CH_PUMP_GET1, 1, false, P_CHEWY);

		if (_G(spieler).R40PoliceAb) {
			_G(spieler).R40PoliceAb = false;
			stop_spz();
			go_auto_xy(308, 100, P_HOWARD, ANI_WAIT);
			_G(spieler).R40HoUse = false;
		}

		det->hide_static_spr(15);
		start_detail_wait(12, 1, ANI_VOR);
		det->set_static_ani(14, -1);
		start_aad_wait(aad_nr, -1);
		det->del_static_ani(14);
		start_detail_wait(13, 1, ANI_VOR);
		det->show_static_spr(15);
		room->set_timer_status(255, TIMER_START);
		uhr->reset_timer(_G(timer_nr)[0], 0);
		show_cur();
	}

	return action_flag;
}

void Room40::talk_police() {
	if (!_G(spieler).R40PoliceWeg && _G(spieler).R40PoliceAniStatus == 255) {
		hide_cur();
		_G(spieler).R40PoliceStart = false;
		room->set_timer_status(255, TIMER_STOP);
		auto_move(7, P_CHEWY);
		start_aad_wait(203, -1);
		room->set_timer_status(255, TIMER_START);
		uhr->reset_timer(_G(timer_nr)[0], 0);
		show_cur();
	}
}

void Room40::talk_handler() {
	if (!_G(spieler).R40HaendlerOk) {
		_G(spieler).flags38_2 = true;
		hide_cur();
		auto_move(6, P_CHEWY);
		det->del_static_ani(4);
		room->set_timer_status(4, TIMER_STOP);
		det->set_static_ani(3, -1);
		start_aad_wait(202, -1);
		det->set_static_ani(4, -1);
		det->del_static_ani(3);
		room->set_timer_status(4, TIMER_START);
		show_cur();
	}
}

int16 Room40::use_haendler() {
	int16 action_flag = false;

	if (menu_item == CUR_HOWARD && !_G(spieler).R40HaendlerOk) {
		action_flag = true;
		if (!_G(spieler).flags38_2) {
			start_aad_wait(612, -1);
		} else {
			hide_cur();
			invent_2_slot(DUENGER_INV);
			_G(spieler).R40HoUse = true;
			_G(spieler).R40HaendlerOk = true;
			_G(spieler).R40TeilKarte = true;
			_G(spieler).R40DuengerMit = true;
			atds->set_steuer_bit(283, ATS_AKTIV_BIT, ATS_DATEI);
			auto_move(10, P_CHEWY);
			auto_move(11, P_HOWARD);
			start_aad_wait(208, -1);
			auto_move(6, P_HOWARD);
			flags.NoScroll = true;

			auto_scroll(270, 0);
			det->del_static_ani(4);
			room->set_timer_status(4, TIMER_STOP);
			det->set_static_ani(3, -1);
			start_aad_wait(209, -1);
			det->del_static_ani(3);
			det->set_static_ani(5, -1);
			start_aad_wait(213, -1);

			if (_G(spieler).R28RKuerbis) {
				det->del_static_ani(5);
				det->set_static_ani(3, -1);
				start_aad_wait(211, -1);
				_G(out)->ausblenden(0);
				_G(out)->set_teilpalette(pal, 255, 1);
				start_aad_wait(212, -1);
				_G(out)->ausblenden(0);
				_G(spieler).R40Wettbewerb = true;
				_G(spieler).PersonRoomNr[P_HOWARD] = 28;
				flags.NoScroll = false;
				_G(out)->setze_zeiger(nullptr);
				_G(out)->cls();
				switch_room(28);
				SetUpScreenFunc = setup_func;

			} else {
				auto_move(11, P_HOWARD);
				start_aad_wait(210, -1);
			}

			menu_item = CUR_WALK;
			cursor_wahl(menu_item);
			show_cur();
			flags.NoScroll = false;
			flags.MausLinks = false;
			_G(spieler).R40HoUse = false;
		}
	}

	return action_flag;
}

int16 Room40::use_bmeister() {
	short action_flag = false;

	if (menu_item == CUR_HOWARD) {
		action_flag = true;
		hide_cur();
		_G(spieler).R40HoUse = true;
		auto_move(9, P_CHEWY);
		auto_move(11, P_HOWARD);
		start_aad_wait(214, -1);
		bmeister_dia(215);
		start_aad_wait(216, -1);
		_G(spieler).R40HoUse = false;
		flags.NoScroll = false;
		menu_item = CUR_WALK;
		cursor_wahl(menu_item);
		show_cur();

	} else if (is_cur_inventar(LIKOER2_INV)) {
		action_flag = true;
		if (_G(spieler).flags37_80) {
			start_aad_wait(605, -1);
		} else {
			hide_cur();
			_G(spieler).R40HoUse = true;
			new_invent_2_cur(HOTEL_INV);
			bmeister_dia(237);
			_G(spieler).flags37_80 = true;
			flags.NoScroll = false;
			show_cur();
		}
	}

	return action_flag;
}

void Room40::bmeister_dia(int16 aad_nr) {
	auto_move(7, P_CHEWY);
	auto_move(12, P_HOWARD);
	flags.NoScroll = true;
	auto_scroll(206, 0);
	det->del_static_ani(0);
	room->set_timer_status(0, TIMER_STOP);
	det->stop_detail(0);
	start_detail_wait(1, 1, ANI_VOR);
	det->set_static_ani(2, -1);

	if (aad_nr == 237) {
		start_aad_wait(aad_nr, -1);
		flags.NoPalAfterFlc = true;
		flic_cut(FCUT_062, CFO_MODE);
		fx_blend = BLEND3;
		start_aad_wait(375, -1);
	} else {
		start_aad_wait(aad_nr, -1);
	}

	flags.NoPalAfterFlc = false;
	room->set_timer_status(0, TIMER_START);
	det->set_static_ani(0, -1);
	det->del_static_ani(2);
	auto_move(11, P_HOWARD);
	auto_move(9, P_CHEWY);
}

bool Room40::use_police() {
	bool result = false;

	if (menu_item == CUR_HOWARD) {
		if (!_G(spieler).R40PoliceWeg && _G(spieler).R40PoliceAniStatus == 255) {
			result = true;
			_G(spieler).R40PoliceAb = true;
			hide_cur();
			_G(spieler).R40PoliceStart = false;
			room->set_timer_status(255, TIMER_STOP);
			_G(spieler).R40HoUse = true;
			auto_move(9, P_CHEWY);
			auto_move(11, P_HOWARD);
			start_aad_wait(217, -1);
			auto_move(8, P_CHEWY);
			go_auto_xy(300, 120, P_HOWARD, ANI_WAIT);
			person_end_phase[P_HOWARD] = P_RIGHT;
			start_aad_wait(218, -1);
			start_spz(HO_TALK_L, 255, ANI_VOR, P_HOWARD);
			menu_item = CUR_WALK;
			cursor_wahl(menu_item);
			show_cur();
			flags.MausLinks = false;
		}
	} else {
		start_aad_wait(225, -1);
	}

	return result;
}

int16 Room40::use_tele() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur && _G(spieler).R40PoliceWeg == false) {
		action_flag = true;
		hide_cur();

		int16 dia_nr1 = -1;
		if (!_G(spieler).R40PoliceAb) {
			start_aad_wait(219, -1);
			_G(spieler).R40HoUse = false;

		} else {
			auto_move(13, P_CHEWY);
			det->show_static_spr(0);

			int16 dia_nr;
			if (!_G(spieler).R40DuengerMit) {
				dia_nr = 220;
				dia_nr1 = 222;
			} else {
				dia_nr = 221;
				dia_nr1 = 223;
			}

			start_aad_wait(dia_nr, -1);
			auto_move(11, P_HOWARD);
			det->hide_static_spr(0);
			auto_move(9, P_CHEWY);
			start_aad_wait(dia_nr1, -1);
			_G(spieler).R40HoUse = false;
			int16 timer_wert = 0;

			if (dia_nr1 == 223) {
				if (is_cur_inventar(DUENGER_INV)) {
					del_inventar(_G(spieler).AkInvent);
				} else {
					remove_inventory(DUENGER_INV);
				}

				invent_2_slot(LIKOER_INV);
				auto_move(1, P_CHEWY);
				_G(spieler).R40DuengerMit = false;
				_G(spieler).R40DuengerTele = true;
				flags.MausLinks = true;
				flags.MainInput = false;
				timer_wert = 3;
			}

			_G(spieler).R40PoliceAb = false;
			room->set_timer_status(255, TIMER_START);
			uhr->reset_timer(_G(timer_nr)[0], timer_wert);
		}

		if (dia_nr1 != 223)
			show_cur();
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
