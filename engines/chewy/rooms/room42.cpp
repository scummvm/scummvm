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
#include "chewy/rooms/room42.h"

namespace Chewy {
namespace Rooms {

void Room42::entry() {
	if (!_G(spieler).R42BeamterWach) {
		det->play_sound(0, 0);
		det->start_detail(0, 255, 0);
	}

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 42) {
		SetUpScreenFunc = setup_func;

		if (!flags.LoadGame) {
			det->stop_detail(0);
			_G(timer_nr)[0] = room->set_timer(8, 5);
			det->set_static_ani(8, -1);
			_G(spieler).R42BeamterWach = true;
			det->disable_sound(0, 0);

			SetUpScreenFunc = setup_func;

			set_person_pos(80, 43, P_HOWARD, P_LEFT);
			atds->set_ats_str(263, 1, ATS_DATEI);
			atds->set_ats_str(264, 1, ATS_DATEI);
		}

		if (obj->check_inventar(HOTEL_INV) && obj->check_inventar(TICKET_INV) &&
				!_G(spieler).R42BriefOk)
			start_aad_wait(302, -1);

		if (obj->check_inventar(HOTEL_INV) && obj->check_inventar(TICKET_INV) &&
				_G(spieler).R42BriefOk)
			start_aad_wait(301, -1);
	}
}

void Room42::gedAction(int index) {
	if (!index)
		calc_xit();
}

void Room42::setup_func() {
	if (!_G(spieler).R42HoToBeamter)
		calc_person_look();
}

void Room42::calc_xit() {
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 42) {
		atds->set_ats_str(264, 1, ATS_DATEI);
		stop_spz();
		_G(spieler).R42HoToBeamter = false;
		_G(spieler).PersonRoomNr[P_HOWARD] = 40;
	}
}

void Room42::get_kuerbis(int16 aad_nr) {
	hide_cur();
	auto_move(1, P_CHEWY);
	_G(spieler).PersonHide[P_CHEWY] = true;
	start_detail_wait(7, 1, ANI_RUECK);
	_G(spieler).PersonHide[P_CHEWY] = false;
	start_spz(CH_TALK5, 255, ANI_VOR, P_CHEWY);
	start_aad_wait(aad_nr, -1);

	_G(spieler).PersonHide[P_CHEWY] = true;
	start_detail_wait(7, 1, ANI_VOR);
	_G(spieler).PersonHide[P_CHEWY] = false;
	show_cur();
}

int16 Room42::use_psack() {
	int16 action_flag = false;

	if (!_G(spieler).R42BeamterWach && !_G(spieler).inv_cur) {
		action_flag = true;
		get_kuerbis(136);

	} else if (_G(spieler).R42HoToBeamter &&
			!_G(spieler).inv_cur &&
			!_G(spieler).R42MarkeOk) {
		action_flag = true;
		auto_move(3, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(10, 1, ANI_VOR);
		_G(spieler).PersonHide[P_CHEWY] = false;
		start_aad_wait(187, -1);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(9, 1, ANI_VOR);
		_G(spieler).PersonHide[P_CHEWY] = false;
		new_invent_2_cur(BMARKE_INV);
		start_aad_wait(181, -1);
		_G(spieler).R42MarkeOk = true;
		auto_move(4, P_CHEWY);
		start_aad_wait(185, -1);
		_G(spieler).R42HoToBeamter = false;

	} else if (is_cur_inventar(BRIEF2_INV)) {
		action_flag = true;
		auto_move(3, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(10, 1, ANI_VOR);
		_G(spieler).PersonHide[P_CHEWY] = false;
		del_inventar(_G(spieler).AkInvent);
		start_aad_wait(183, -1);
		obj->calc_rsi_flip_flop(SIB_BKASTEN_R28);
		atds->set_ats_str(206, 1, ATS_DATEI);
		_G(spieler).R28Briefkasten = true;
		_G(spieler).R40TrainMove = true;
		_G(spieler).R28PostCar = true;
		_G(spieler).R42BriefOk = true;

	} else if (is_cur_inventar(BRIEF_INV)) {
		action_flag = true;
		start_aad_wait(182, -1);
	}

	return action_flag;
}

int16 Room42::use_beamter() {
	int16 action_flag = false;

	if (menu_item == CUR_HOWARD) {
		action_flag = true;
		atds->set_ats_str(264, 2, ATS_DATEI);
		hide_cur();
		auto_move(4, P_CHEWY);
		start_aad_wait(184, -1);
		_G(spieler).R42HoToBeamter = true;
		menu_item = CUR_WALK;
		cursor_wahl(menu_item);
		show_cur();
		start_spz(HO_TALK_L, 255, ANI_VOR, P_HOWARD);
		flags.MausLinks = false;
	}

	return action_flag;
}

void Room42::talk_beamter() {
	int16 dia_nr;
	auto_move(1, P_CHEWY);

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
	hide_cur();

	if (str_end_nr != 4) {
		while (atds->aad_get_status() != -1) {
			set_up_screen(DO_SETUP);
			SHOULD_QUIT_RETURN;
		}

		_G(spieler).PersonHide[P_CHEWY] = true;
		flags.NoDiaBox = true;

		switch (str_end_nr) {
		case 1:
			start_detail_wait(3, 4, ANI_VOR);
			break;

		case 2:
			det->play_sound(4, 0);
			start_detail_wait(4, 4, ANI_VOR);
			break;

		case 3:
			start_detail_wait(5, 2, ANI_VOR);
			break;

		default:
			break;
		}
	} else {
		while (atds->aad_get_status() < 2) {
			set_up_screen(DO_SETUP);
			SHOULD_QUIT_RETURN;
		}

		det->stop_detail(0);
		det->disable_sound(0, 0);
		start_detail_wait(1, 1, ANI_VOR);
		det->start_detail(2, 255, ANI_VOR);

		while (atds->aad_get_status() != -1) {
			set_up_screen(DO_SETUP);
			SHOULD_QUIT_RETURN;
		}

		det->stop_detail(2);
		det->start_detail(0, 255, ANI_VOR);
		det->enable_sound(0, 0);
		det->play_sound(0, 0);
	}

	_G(spieler).PersonHide[P_CHEWY] = true;
	det->start_detail(6, 255, ANI_VOR);
	start_aad_wait(135, -1);
	det->stop_detail(6);
	flags.NoDiaBox = false;
	_G(spieler).PersonHide[P_CHEWY] = false;
	show_cur();
}

} // namespace Rooms
} // namespace Chewy
