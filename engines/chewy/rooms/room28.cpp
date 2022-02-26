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
#include "chewy/rooms/room28.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room28::entry(int16 eib_nr) {
	_G(zoom_horizont) = 140;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(spieler).ScrollxStep = 2;

	if (_G(spieler).R28RKuerbis)
		_G(det)->show_static_spr(6);

	if (_G(spieler).R28Briefkasten) {
		_G(det)->show_static_spr(8);
		_G(det)->show_static_spr(9);
	} else {
		_G(det)->show_static_spr(7);
	}

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 28) {
		_G(spieler).ZoomXy[P_HOWARD][0] = 40;
		_G(spieler).ZoomXy[P_HOWARD][1] = 40;
		hideCur();
		_G(SetUpScreenFunc) = setup_func;

		if (_G(spieler).R40Wettbewerb) {
			_G(spieler).scrollx = 320;
			_G(spieler).PersonHide[P_CHEWY] = false;
			haendler();
			_G(spieler).R40Wettbewerb = false;
		} else {
			switch (eib_nr) {
			case 55:
				if (!_G(spieler).R28EntryHaus) {
					_G(spieler).R28EntryHaus = true;
					set_person_pos(232, 100, P_HOWARD, P_RIGHT);
					autoMove(3, P_HOWARD);
					goAutoXy(320, _G(spieler_vector)[P_HOWARD].Xypos[1], P_HOWARD, ANI_WAIT);
					start_aad_wait(176, -1);
				} else if (_G(spieler).R31SurFurz && !_G(spieler).R28RKuerbis) {
					_G(spieler).R28RKuerbis = true;
					_G(spieler).ScrollxStep = 2;
					autoMove(6, P_CHEWY);
					wait_show_screen(10);
					start_aad_wait(194, -1);
					_G(spieler).room_e_obj[62].Attribut = 255;
					_G(atds)->set_ats_str(208, 1, ATS_DATEI);
					_G(flags).NoScroll = true;

					auto_scroll(0, 0);
					flic_cut(FCUT_064);
					_G(det)->show_static_spr(6);

					if (!_G(spieler).R40TeilKarte) {
						_G(out)->ausblenden(0);
						_G(out)->set_teilpalette(_G(pal), 255, 1);
						start_aad_wait(607, -1);
						_G(fx_blend) = BLEND3;
					}

					_G(flags).NoScroll = false;

					if (_G(spieler).R40TeilKarte) {
						haendler();
					} else {
						wait_show_screen(60);
						start_aad_wait(195, -1);
					}

					_G(spieler).ScrollxStep = 2;
				}
				break;

			case 70:
			case 77:
				if (_G(spieler).R28PostCar) {
					_G(spieler).R28PostCar = false;
					_G(out)->setPointer(nullptr);
					_G(out)->cls();
					flic_cut(FCUT_063);
					_G(spieler).R28ChewyPump = false;
					invent_2_slot(K_MASKE_INV);
					_G(atds)->set_ats_str(209, 0, ATS_DATEI);
					_G(spieler).room_e_obj[69].Attribut = 255;
					load_chewy_taf(CHEWY_NORMAL);

					set_person_pos(480, 118, P_HOWARD, P_LEFT);
					set_person_pos(440, 146, P_CHEWY, P_RIGHT);
					_G(fx_blend) = BLEND_NONE;
					_G(flags).ExitMov = false;
					_G(spieler_mi)[P_HOWARD].Mode = true;
					goAutoXy(420, 113, P_HOWARD, ANI_WAIT);
					_G(spieler_mi)[P_HOWARD].Mode = false;
					set_person_pos(440, 146, P_CHEWY, P_LEFT);
					start_aad_wait(193, -1);
				} else {
					set_person_pos(480, 100, P_HOWARD, P_LEFT);
					goAutoXy(420, 113, P_HOWARD, ANI_GO);
					_G(flags).ExitMov = false;
					autoMove(6, P_CHEWY);
				}
				break;

			default:
				break;
			}
		}

		showCur();
	}
}

void Room28::xit(int16 eib_nr) {
	_G(spieler).R28PostCar = false;
	_G(spieler).ScrollxStep = 1;
	hideCur();

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 28 && eib_nr == 69) {
		_G(SetUpScreenFunc) = nullptr;
		if (!_G(spieler).R28ExitTown) {
			start_aad_wait(178, -1);
			_G(spieler).R28ExitTown = true;
		}

		_G(flags).ExitMov = false;
		autoMove(6, P_HOWARD);
		_G(spieler).PersonRoomNr[P_HOWARD] = 40;
	}

	showCur();
}

void Room28::gedAction(int index) {
	if (!index)
		get_pump();
}

void Room28::haendler() {
	hideCur();
	_G(SetUpScreenFunc) = nullptr;
	_G(spieler).R28ChewyPump = true;
	del_inventar(K_MASKE_INV);
	_G(atds)->set_ats_str(209, 1, ATS_DATEI);
	_G(spieler).room_e_obj[69].Attribut = AUSGANG_OBEN;
	load_chewy_taf(CHEWY_PUMPKIN);

	set_person_pos(480, 113, P_HOWARD, P_RIGHT);
	set_person_pos(490, 146, P_CHEWY, P_RIGHT);
	_G(det)->set_static_ani(2, -1);

	while (_G(spieler).scrollx < 300) {
		SHOULD_QUIT_RETURN;
		set_up_screen(DO_SETUP);
	}

	start_aad_wait(196, -1);
	_G(det)->del_static_ani(2);
	_G(det)->set_static_ani(3, -1);
	start_aad_wait(197, -1);
	_G(det)->del_static_ani(3);
	_G(det)->set_static_ani(4, -1);
	start_aad_wait(198, -1);
	_G(SetUpScreenFunc) = setup_func;
	autoMove(4, P_CHEWY);
	hideCur();
	autoMove(3, P_CHEWY);
	set_person_spr(P_RIGHT, P_CHEWY);
	_G(det)->del_static_ani(4);
	wait_show_screen(28);
	start_aad_wait(199, -1);
	invent_2_slot(DOLLAR175_INV);
}

void Room28::setup_func() {
	calc_person_look();

	int16 x = (_G(spieler_vector)[P_CHEWY].Xypos[0] > 350) ? 420 : 320;
	goAutoXy(x, 113, P_HOWARD, ANI_GO);
}

void Room28::use_surimy() {
	int16 dia_nr;
	int16 ani_nr;

	if (!_G(spieler).R28SurimyCar) {
		hideCur();
		_G(flags).NoScroll = true;
		auto_scroll(0, 0);
		_G(spieler).R28SurimyCar = true;
		start_spz_wait(CH_LGET_U, 1, false, P_CHEWY);
		flic_cut(FCUT_055);
		flic_cut(FCUT_056);
		ani_nr = CH_TALK3;
		dia_nr = 140;
		_G(atds)->set_ats_str(205, 1, ATS_DATEI);
		_G(atds)->set_ats_str(222, 1, ATS_DATEI);

	} else {
		ani_nr = CH_TALK5;
		dia_nr = 139;
		_G(obj)->calc_rsi_flip_flop(SIB_AUTO_R28);
	}

	_G(obj)->calc_all_static_detail();
	start_spz(ani_nr, 255, ANI_FRONT, P_CHEWY);
	start_aad_wait(dia_nr, -1);
	_G(flags).NoScroll = false;
	showCur();
}

void Room28::set_pump() {
	hideCur();

	if (_G(spieler).PersonRoomNr[P_CHEWY] == 28) {
		if (!_G(flags).AutoAniPlay) {
			_G(flags).AutoAniPlay = true;
			if (_G(spieler_vector)[P_CHEWY].Xypos[0] < 380)
				autoMove(5, P_CHEWY);

			_G(spieler).PersonHide[P_CHEWY] = true;
			int16 tmp = (_G(person_end_phase)[P_CHEWY] == P_RIGHT) ? 1 : 0;
			del_inventar(K_MASKE_INV);
			_G(det)->set_detail_pos(tmp, _G(spieler_vector)[P_CHEWY].Xypos[0], _G(spieler_vector)[P_CHEWY].Xypos[1]);

			if (_G(spieler).R28PumpTxt1 < 3) {
				start_aad(137);
				++_G(spieler).R28PumpTxt1;
			}

			start_detail_wait(tmp, 1, ANI_FRONT);
			_G(spieler).PersonHide[P_CHEWY] = false;

			load_chewy_taf(CHEWY_PUMPKIN);
			_G(spieler).R28ChewyPump = true;

			if (_G(spieler).R39TranslatorUsed) {
				_G(spieler).room_e_obj[69].Attribut = AUSGANG_OBEN;
			} else {
				_G(atds)->set_ats_str(209, 1, ATS_DATEI);
			}

			if (_G(spieler).PersonRoomNr[P_HOWARD] == 28 && _G(spieler).R28PumpTxt < 3) {
				stop_person(P_HOWARD);
				_G(SetUpScreenFunc) = nullptr;
				g_engine->_sound->waitForSpeechToFinish();

				start_aad_wait(177, -1);
				_G(SetUpScreenFunc) = setup_func;
				++_G(spieler).R28PumpTxt;
			}

			_G(flags).AutoAniPlay = false;
		}
	} else {
		start_ats_wait(20, TXT_MARK_USE, 14, INV_USE_DEF);
	}

	showCur();
}

void Room28::get_pump() {

	if (_G(spieler).R28ChewyPump) {
		hideCur();

		stop_person(P_CHEWY);
		_G(spieler).R28ChewyPump = false;
		_G(spieler).PersonHide[P_CHEWY] = true;
		int16 tmp = (_G(person_end_phase)[P_CHEWY] == P_RIGHT) ? 1 : 0;
		_G(det)->set_detail_pos(tmp, _G(spieler_vector)[P_CHEWY].Xypos[0], _G(spieler_vector)[P_CHEWY].Xypos[1]);
		start_detail_wait(tmp, 1, ANI_BACK);

		invent_2_slot(K_MASKE_INV);
		_G(atds)->set_ats_str(209, 0, ATS_DATEI);
		_G(spieler).room_e_obj[69].Attribut = 255;
		_G(spieler).PersonHide[P_CHEWY] = false;
		load_chewy_taf(CHEWY_NORMAL);

		if (_G(spieler).R28PumpTxt1 < 3) {
			start_spz(CH_TALK3, 255, ANI_FRONT, P_CHEWY);
			start_aad_wait(138, -1);
		}

		showCur();
	}
}

int16 Room28::use_breifkasten() {
	int16 action_flag = false;

	if (_G(spieler).R28Briefkasten && !_G(spieler).inv_cur) {
		action_flag = true;
		hideCur();
		_G(spieler).R28Briefkasten = false;
		autoMove(7, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		_G(det)->hide_static_spr(8);
		_G(det)->hide_static_spr(9);
		_G(det)->show_static_spr(7);
		autoMove(8, P_CHEWY);
		start_spz(CH_LGET_O, 1, ANI_FRONT, P_CHEWY);
		start_aad_wait(179, -1);
		_G(atds)->set_ats_str(206, 0, ATS_DATEI);
		showCur();
		invent_2_slot(MANUSKRIPT_INV);
		_G(spieler).R28Manuskript = true;
	}

	return action_flag;
}

int16 Room28::cut_serv1(int16 frame) {
	if (_G(spieler).R28Briefkasten) {
		_G(det)->plot_static_details(0, 0, 8, 9);
	} else {
		_G(det)->plot_static_details(0, 0, 7, 7);
	}
	return 0;
}

int16 Room28::cut_serv2(int16 frame) {
	if (frame < 23)
		cut_serv1(frame);

	return 0;
}

} // namespace Rooms
} // namespace Chewy
