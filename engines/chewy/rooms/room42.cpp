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
#include "chewy/rooms/room42.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room42::entry() {
	if (!_G(spieler).R42BeamterWach) {
		g_engine->_sound->playSound(0);
		_G(det)->start_detail(0, 255, ANI_FRONT);
	}

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 42) {
		_G(SetUpScreenFunc) = setup_func;

		if (!_G(flags).LoadGame) {
			_G(det)->stop_detail(0);
			_G(timer_nr)[0] = _G(room)->set_timer(8, 5);
			_G(det)->set_static_ani(8, -1);
			_G(spieler).R42BeamterWach = true;
			g_engine->_sound->stopSound(0);

			_G(SetUpScreenFunc) = setup_func;

			set_person_pos(80, 43, P_HOWARD, P_LEFT);
			_G(atds)->set_ats_str(263, 1, ATS_DATEI);
			_G(atds)->set_ats_str(264, 1, ATS_DATEI);
		}

		if (_G(obj)->checkInventory(HOTEL_INV) && _G(obj)->checkInventory(TICKET_INV) && !_G(spieler).R42BriefOk)
			start_aad_wait(302, -1);

		if (_G(obj)->checkInventory(HOTEL_INV) && _G(obj)->checkInventory(TICKET_INV) && _G(spieler).R42BriefOk)
			start_aad_wait(301, -1);
	}
}

void Room42::xit() {
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 42) {
		_G(atds)->set_ats_str(264, 1, ATS_DATEI);
		stop_spz();
		_G(spieler).R42HoToBeamter = false;
		_G(spieler).PersonRoomNr[P_HOWARD] = 40;
	}
}

void Room42::setup_func() {
	if (!_G(spieler).R42HoToBeamter)
		calc_person_look();
}

void Room42::get_kuerbis(int16 aad_nr) {
	hideCur();
	autoMove(1, P_CHEWY);
	_G(spieler).PersonHide[P_CHEWY] = true;
	start_detail_wait(7, 1, ANI_BACK);
	_G(spieler).PersonHide[P_CHEWY] = false;
	start_spz(CH_TALK5, 255, ANI_FRONT, P_CHEWY);
	start_aad_wait(aad_nr, -1);

	_G(spieler).PersonHide[P_CHEWY] = true;
	start_detail_wait(7, 1, ANI_FRONT);
	_G(spieler).PersonHide[P_CHEWY] = false;
	showCur();
}

int16 Room42::use_psack() {
	int16 action_flag = false;

	if (_G(menu_item) == CUR_HOWARD)
		return action_flag;

	hideCur();
	if (!_G(spieler).R42BeamterWach && !_G(spieler).inv_cur) {
		action_flag = true;
		get_kuerbis(136);
	} else if (_G(spieler).R42HoToBeamter && !_G(spieler).inv_cur && !_G(spieler).R42MarkeOk) {
		action_flag = true;
		autoMove(3, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(10, 1, ANI_FRONT);
		_G(spieler).PersonHide[P_CHEWY] = false;
		start_aad_wait(187, -1);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(9, 1, ANI_FRONT);
		_G(spieler).PersonHide[P_CHEWY] = false;
		new_invent_2_cur(BMARKE_INV);
		start_aad_wait(181, -1);
		_G(spieler).R42MarkeOk = true;
		autoMove(4, P_CHEWY);
		start_aad_wait(185, -1);
		_G(spieler).R42HoToBeamter = false;
	} else if (is_cur_inventar(BRIEF2_INV)) {
		action_flag = true;
		autoMove(3, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(10, 1, ANI_FRONT);
		_G(spieler).PersonHide[P_CHEWY] = false;
		del_inventar(_G(spieler).AkInvent);
		start_aad_wait(183, -1);
		_G(obj)->calc_rsi_flip_flop(SIB_BKASTEN_R28);
		_G(atds)->set_ats_str(206, 1, ATS_DATEI);
		_G(spieler).R28Briefkasten = true;
		_G(spieler).R40TrainMove = true;
		_G(spieler).R28PostCar = true;
		_G(spieler).R42BriefOk = true;

	} else if (is_cur_inventar(BRIEF_INV)) {
		action_flag = true;
		start_aad_wait(182, -1);
	}

	showCur();
	return action_flag;
}

int16 Room42::use_beamter() {
	int16 action_flag = false;

	if (_G(menu_item) == CUR_HOWARD) {
		action_flag = true;
		_G(atds)->set_ats_str(264, 2, ATS_DATEI);
		hideCur();
		autoMove(4, P_CHEWY);
		start_aad_wait(184, -1);
		_G(spieler).R42HoToBeamter = true;
		_G(menu_item) = CUR_WALK;
		cursorChoice(_G(menu_item));
		showCur();
		start_spz(HO_TALK_L, 255, ANI_FRONT, P_HOWARD);
		_G(flags).MausLinks = false;
	}

	return action_flag;
}

void Room42::talk_beamter() {
	int16 dia_nr;
	autoMove(1, P_CHEWY);

	if (!_G(spieler).R42BeamterWach) {
		dia_nr = 10;
	} else if (!_G(spieler).R42MarkeOk) {
		dia_nr = 13;
	} else {
		dia_nr = 14;
	}

	start_ads_wait(dia_nr);
}

void Room42::dia_beamter(int16 str_end_nr) {
	hideCur();

	if (str_end_nr != 4) {
		while (_G(atds)->aad_get_status() != -1) {
			set_up_screen(DO_SETUP);
			SHOULD_QUIT_RETURN;
		}

		_G(spieler).PersonHide[P_CHEWY] = true;
		_G(flags).NoDiaBox = true;

		switch (str_end_nr) {
		case 1:
			start_detail_wait(3, 4, ANI_FRONT);
			break;

		case 2:
			g_engine->_sound->playSound(4);
			start_detail_wait(4, 13, ANI_FRONT);
			break;

		case 3:
			start_detail_wait(5, 2, ANI_FRONT);
			break;

		default:
			break;
		}
	} else {
		while (_G(atds)->aad_get_status() < 2) {
			set_up_screen(DO_SETUP);
			SHOULD_QUIT_RETURN;
		}

		_G(det)->stop_detail(0);
		g_engine->_sound->stopSound(0);
		start_detail_wait(1, 1, ANI_FRONT);
		_G(det)->start_detail(2, 255, ANI_FRONT);

		while (_G(atds)->aad_get_status() != -1) {
			set_up_screen(DO_SETUP);
			SHOULD_QUIT_RETURN;
		}

		_G(det)->stop_detail(2);
		_G(det)->start_detail(0, 255, ANI_FRONT);
		g_engine->_sound->playSound(0, 0);
		g_engine->_sound->playSound(0);
	}

	_G(spieler).PersonHide[P_CHEWY] = true;
	_G(det)->start_detail(6, 255, ANI_FRONT);
	start_aad_wait(135, -1);
	_G(det)->stop_detail(6);
	_G(flags).NoDiaBox = false;
	_G(spieler).PersonHide[P_CHEWY] = false;
	showCur();
}

} // namespace Rooms
} // namespace Chewy
