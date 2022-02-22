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
#include "chewy/room.h"
#include "chewy/rooms/room54.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room54::entry(int16 eib_nr) {
	_G(spieler).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;
	_G(zoom_horizont) = 106;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(spieler).ZoomXy[P_HOWARD][0] = 30;
	_G(spieler).ZoomXy[P_HOWARD][1] = 66;

	if (_G(spieler).R54FputzerWeg)
		_G(det)->show_static_spr(9);

	_G(spieler_mi)[P_HOWARD].Mode = true;

	if (!_G(flags).LoadGame) {
		if (_G(spieler).R48TaxiEntry) {
			_G(spieler).R48TaxiEntry = false;

			if (_G(spieler).PersonRoomNr[P_HOWARD] == 54) {
				_G(spieler).R54HowardVorne = 0;
				set_person_pos(290, 61, P_HOWARD, P_RIGHT);
			}

			set_person_pos(300, 80, P_CHEWY, P_LEFT);
			_G(maus_links_click) = false;
			_G(spieler).scrollx = 134;
			_G(spieler).scrolly = 0;

		} else if (_G(spieler).R55Location) {
			aufzug_ab();

		} else if (eib_nr == 90 || _G(spieler).R55ExitDia) {
			if (_G(spieler).PersonRoomNr[P_HOWARD] == 54) {
				_G(spieler).R54HowardVorne = 0;
				set_person_pos(212, 61, P_HOWARD, P_RIGHT);
			}

			_G(spieler).scrollx = 66;
			set_person_pos(241, 85, P_CHEWY, P_LEFT);
		}

		if (_G(spieler).R55ExitDia) {
			start_aad_wait(_G(spieler).R55ExitDia, -1);
			showCur();
			_G(spieler).R55ExitDia = false;
		}

		if (_G(spieler).R55R54First) {
			_G(spieler).R55R54First = false;
		}
	}
}

void Room54::xit(int16 eib_nr) {
	_G(spieler).ScrollxStep = 1;

	if (eib_nr == 89 && _G(spieler).PersonRoomNr[P_HOWARD] == 54) {
		_G(spieler).PersonRoomNr[P_HOWARD] = 55;
		_G(spieler_mi)[P_HOWARD].Mode = false;
	}
}

void Room54::setup_func() {
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 54) {
		calc_person_look();

		if (_G(spieler).R54HowardVorne != 255) {
			const int16 ch_x = _G(spieler_vector)[P_CHEWY].Xypos[0];
			int16 x, y;
			if (_G(spieler).R54HowardVorne) {
				if (ch_x < 218) {
					x = 150;
					y = 113;
				} else {
					x = 287;
					y = 115;
				}
			} else {
				y = 61;
				if (ch_x < 167) {
					x = 122;
				} else if (ch_x < 251) {
					x = 212;
				} else {
					x = 290;
				}
			}

			if (!_G(flags).SaveMenu)
				go_auto_xy(x, y, P_HOWARD, ANI_GO);
		}
	}
}

int16 Room54::use_schalter() {
	int16 action_ret = false;

	if (!_G(spieler).inv_cur) {
		hideCur();
		action_ret = true;

		if (!_G(spieler).R54FputzerWeg) {
			auto_move(1, P_CHEWY);
			_G(spieler).R54HowardVorne = 255;
			go_auto_xy(127, 76, P_HOWARD, ANI_WAIT);
			go_auto_xy(14, 72, P_HOWARD, ANI_GO);
			start_spz_wait(CH_ROCK_GET2, 1, false, P_CHEWY);
			_G(det)->show_static_spr(0);
			auto_move(2, P_CHEWY);
			g_engine->_sound->playSound(1, 0);
			g_engine->_sound->playSound(0, 1);
			g_engine->_sound->stopSound(2);

			start_detail_wait(1, 1, ANI_VOR);
			_G(det)->start_detail(3, 255, ANI_VOR);
			start_aad_wait(292 + _G(spieler).R54LiftCount, -1);
			_G(det)->stop_detail(3);
			++_G(spieler).R54LiftCount;

			int16 aad_nr;
			if (_G(spieler).R54LiftCount < 3) {
				start_detail_wait(2, 1, ANI_VOR);
				_G(det)->hide_static_spr(0);
				g_engine->_sound->stopSound(0);
				g_engine->_sound->stopSound(1);
				g_engine->_sound->playSound(1, 2);
				start_detail_wait(1, 1, ANI_RUECK);
				g_engine->_sound->stopSound(2);
				aad_nr = 295;

			} else {
				start_detail_wait(5, 1, ANI_VOR);
				_G(det)->show_static_spr(9);
				start_detail_wait(4, 1, ANI_VOR);
				aad_nr = 296;
				_G(spieler).R54FputzerWeg = true;
				_G(atds)->del_steuer_bit(345, ATS_AKTIV_BIT, ATS_DATEI);
				_G(atds)->set_ats_str(349, 1, ATS_DATEI);
				_G(atds)->set_ats_str(351, 1, ATS_DATEI);
			}

			start_aad_wait(aad_nr, -1);
			_G(spieler).R54HowardVorne = 0;

		} else {
			start_aad_wait(297, -1);
		}

		showCur();
	}

	return action_ret;
}

void Room54::talk_verkauf() {
	hideCur();

	if (!_G(spieler).R54HotDogOk) {
		if (_G(spieler).R45MagOk) {
			if (_G(spieler).AkInvent == DOLLAR175_INV)
				del_inventar(DOLLAR175_INV);
			else
				remove_inventory(DOLLAR175_INV);
		}

		_G(spieler).R54HotDogOk = true;
		auto_move(3, P_CHEWY);
		_G(spieler).R54HowardVorne = 1;
		start_aad_wait(299, -1);
		_G(room)->set_timer_status(6, TIMER_STOP);
		_G(det)->del_static_ani(6);
		start_detail_wait(7, 1, ANI_VOR);
		_G(det)->start_detail(8, 255, ANI_VOR);
		start_aad_wait(310, -1);

		_G(det)->stop_detail(8);
		start_detail_wait(9, 1, ANI_VOR);
		start_detail_wait(10, 1, ANI_VOR);
		_G(det)->start_detail(11, 255, ANI_VOR);
		start_aad_wait(311, -1);
		_G(det)->stop_detail(11);
		_G(room)->set_timer_status(6, TIMER_START);
		_G(det)->set_static_ani(6, -1);
		auto_move(4, P_CHEWY);

		start_aad(_G(spieler).R45MagOk ? 312 : 578, -1);
		_G(obj)->addInventory(BURGER_INV, &_G(room_blk));
		inventory_2_cur(BURGER_INV);
	} else {
		start_aad_wait(313, -1);
	}

	showCur();
}

int16 Room54::use_zelle() {
	int16 action_ret = false;
	hideCur();

	if (_G(spieler).inv_cur) {
		if (is_cur_inventar(JMKOST_INV)) {
			action_ret = true;

			if (!_G(spieler).R54Schild) {
				start_aad_wait(318, -1);
			} else {
				auto_move(5, P_CHEWY);
				_G(spieler).R54HowardVorne = 1;
				_G(SetUpScreenFunc) = nullptr;
				auto_scroll(176, 0);
				go_auto_xy(239, 101, P_HOWARD, ANI_WAIT);
				flic_cut(FCUT_069);

				del_inventar(_G(spieler).AkInvent);
				invent_2_slot(LEDER_INV);
				load_chewy_taf(CHEWY_JMANS);
				_G(zoom_horizont) = 90;

				set_person_pos(283, 93, P_CHEWY, P_LEFT);
				set_person_pos(238, 99, P_HOWARD, P_RIGHT);
				start_aad_wait(315, -1);
				go_auto_xy(241, 74, P_CHEWY, ANI_WAIT);
				start_aad_wait(316, -1);

				_G(spieler_mi)[P_HOWARD].Mode = true;
				switch_room(55);
			}
		}
	} else {
		action_ret = true;
		auto_move(6, P_CHEWY);
		_G(spieler).R54HowardVorne = 1;
		start_aad_wait(319, -1);
	}

	showCur();
	return action_ret;
}

int16 Room54::cut_serv(int16 frame) {
	_G(det)->plot_static_details(176, 0, 9, 9);
	return 0;
}

int16 Room54::use_azug() {
	int16 action_ret = false;

	if (!_G(spieler).inv_cur) {
		action_ret = true;
		hideCur();

		if (!_G(spieler).R54LiftOk) {
			if (_G(spieler).R54FputzerWeg) {
				auto_move(8, P_CHEWY);
				_G(spieler).R54HowardVorne = 0;
				start_aad_wait(298, -1);
				_G(spieler).PersonHide[P_CHEWY] = true;
				_G(det)->show_static_spr(12);
				_G(spieler).R55Location = true;
				_G(SetUpScreenFunc) = nullptr;
				go_auto_xy(91, 62, P_HOWARD, ANI_WAIT);
				g_engine->_sound->playSound(1, 0);
				g_engine->_sound->playSound(1);

				int16 ch_y = 68;
				int16 ay = 0;
				int16 delay = 0;

				while (ch_y > -48) {
					_G(det)->set_static_pos(12, 125, ch_y, false, false);
					_G(det)->set_static_pos(9, 122, ay, false, false);

					if (!delay) {
						ch_y -= 3;
						ay -= 3;
						delay = _G(spieler).DelaySpeed / 2;
					} else {
						--delay;
					}

					set_up_screen(DO_SETUP);
					SHOULD_QUIT_RETURN0;
				}

				switch_room(55);
			}
		} else {
			start_aad_wait(314, -1);
		}

		showCur();
	}

	return action_ret;
}

void Room54::aufzug_ab() {
	set_person_pos(91, 62, P_HOWARD, P_RIGHT);
	set_person_pos(99, 82, P_CHEWY, P_RIGHT);
	_G(spieler).scrollx = 0;
	_G(SetUpScreenFunc) = setup_func;
	_G(det)->show_static_spr(12);
	g_engine->_sound->playSound(1, 0);
	g_engine->_sound->playSound(1);

	int16 ch_y = -40;
	int16 ay = -108;
	int16 delay = 0;

	while (ch_y < 68) {
		_G(det)->set_static_pos(12, 125, ch_y, false, false);
		_G(det)->set_static_pos(9, 122, ay, false, false);

		if (!delay) {
			ch_y += 3;
			ay += 3;
			delay = _G(spieler).DelaySpeed / 2;
		} else {
			--delay;
		}

		set_up_screen(DO_SETUP);
		SHOULD_QUIT_RETURN;
	}

	g_engine->_sound->stopSound(0);
	_G(det)->hide_static_spr(12);
	set_person_pos(99, 82, P_CHEWY, P_RIGHT);
	_G(spieler).PersonHide[P_CHEWY] = false;
	_G(maus_links_click) = false;
	stop_person(P_CHEWY);
	_G(spieler).R55Location = false;
}

short Room54::use_taxi() {
	int16 action_ret = false;

	if (!_G(spieler).inv_cur) {
		action_ret = true;
		hideCur();
		auto_move(7, P_CHEWY);
		_G(spieler).R48TaxiPerson[P_CHEWY] = true;

		if (_G(spieler).PersonRoomNr[P_HOWARD] == 54) {
			if (_G(spieler).R54HowardVorne) {
				_G(spieler).R54HowardVorne = 0;
				go_auto_xy(290, 61, P_HOWARD, ANI_WAIT);
			}

			_G(spieler).PersonHide[P_HOWARD] = true;
			_G(spieler).R48TaxiPerson[P_HOWARD] = true;
			_G(spieler).PersonRoomNr[P_HOWARD] = 48;
		}

		_G(spieler).PersonHide[P_CHEWY] = true;
		switch_room(48);
	}

	return action_ret;
}

} // namespace Rooms
} // namespace Chewy
