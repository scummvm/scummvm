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
#include "chewy/rooms/room41.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK32[2] = {
	{ 6, 255, ANI_VOR, ANI_GO, 0 },
	{ 2, 1, ANI_VOR, ANI_WAIT, 0 },
};


void Room41::entry() {
	hide_cur();

	if (!_G(spieler).R41LolaOk) {
		det->play_sound(6, 0);

		if (!flags.LoadGame) {
			room->set_timer_status(0, TIMER_STOP);
			det->del_static_ani(0);
			start_ani_block(2, ABLOCK32);
			room->set_timer_status(0, TIMER_START);
			det->set_static_ani(0, -1);
			start_aad_wait(127, -1);
		} else {
			det->start_detail(6, 255, ANI_VOR);
		}
	}

	if (_G(spieler).R41Einbruch) {
		atds->del_steuer_bit(271, ATS_AKTIV_BIT, ATS_DATEI);
		det->show_static_spr(6);
		det->show_static_spr(7);
	}

	if (_G(spieler).ChewyAni == 5) {
		atds->set_steuer_bit(269, 1, 1);
	}

	if (!flags.LoadGame)
		set_person_pos(241, 113, 1, 1);

	SetUpScreenFunc = setup_func;
	show_cur();
}

void Room41::xit() {
	if (_G(spieler).R41TrainCount == 0) {
		_G(spieler).R41TrainCount = 3;
		_G(spieler).R40TrainMove = true;
	}

	_G(spieler).R41TrainCount--;
}

void Room41::setup_func() {
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 41) {
		calc_person_look();
		if (spieler_vector->Xypos[P_HOWARD] == 160) {
			go_auto_xy(258, 75, 1, 1);
		} else {
			go_auto_xy(246, 120, 1, 1);
		}
	}
}

void Room41::talk_hoggy1() {
	stop_hoggy();
	auto_move(1, P_CHEWY);

	if (!_G(spieler).R41FirstTalk) {
		first_talk();

	} else if (!_G(spieler).R41Einbruch) {
		show_cur();
		start_ads_wait(11);

	} else if (_G(spieler).R41Einbruch) {
		if (!_G(spieler).R41BruchInfo) {
			_G(spieler).R41BruchInfo = true;
			start_aad_wait(132, -1);
			start_aad_wait(128, -1);
		} else if (_G(spieler).R31SurFurz &&
			!_G(spieler).R41KuerbisInfo) {
			_G(spieler).R41KuerbisInfo = true;
			start_aad_wait(131, -1);
			auto_move(5, P_CHEWY);
			new_invent_2_cur(TICKET_INV);
		} else {
			start_aad_wait(130, -1);
		}
	} else {
		start_aad_wait(130, -1);
	}

	start_hoggy();
}

void Room41::talk_hoggy2() {
	stop_hoggy();
	auto_move(2, P_CHEWY);

	if (!_G(spieler).R41FirstTalk) {
		first_talk();

	} else if (_G(spieler).R41BruchInfo) {
		if (_G(spieler).R31SurFurz &&
			!_G(spieler).R41KuerbisInfo) {
			_G(spieler).R41KuerbisInfo = true;
			start_aad_wait(131, -1);
			auto_move(5, P_CHEWY);
			new_invent_2_cur(TICKET_INV);

		} else {
			start_aad_wait(129, -1);
		}
	} else {
		start_aad_wait(129, -1);
	}

	start_hoggy();
}

void Room41::first_talk() {
	_G(spieler).R41FirstTalk = true;
	start_aad_wait(134, -1);
	atds->set_ats_str(266, TXT_MARK_NAME, 1, ATS_DATEI);
	atds->set_ats_str(265, TXT_MARK_NAME, 1, ATS_DATEI);
}

void Room41::start_hoggy() {
	int16 i;
	show_cur();

	for (i = 0; i < 2; i++) {
		room->set_timer_status(i, TIMER_START);
		det->set_static_ani(i, -1);
		det->del_static_ani(i + 3);
	}
}

void Room41::stop_hoggy() {
	int16 i;
	hide_cur();

	for (i = 0; i < 2; i++) {
		room->set_timer_status(i, TIMER_STOP);
		det->del_static_ani(i);
		det->stop_detail(i);
		det->set_static_ani(i + 3, -1);
	}
}

int16 Room41::use_kasse() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur) {
		action_flag = true;
		stop_hoggy();
		auto_move(1, P_CHEWY);
		start_aad_wait(133, -1);
		start_hoggy();
	}

	return action_flag;
}

int16 Room41::use_lola() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur) {
		if (!_G(spieler).R41LolaOk && _G(spieler).R41RepairInfo) {
			hide_cur();
			action_flag = true;
			_G(spieler).R41LolaOk = true;
			auto_move(4, P_CHEWY);
			det->disable_sound(6, 0);
			flic_cut(FCUT_057, FLC_MODE);
			set_person_pos(127, 112, P_CHEWY, P_LEFT);
			det->stop_detail(6);

			atds->del_steuer_bit(267, 1, 1);
			atds->set_ats_str(267, 1, ATS_DATEI);
			atds->hide_item(11, 0, 3);
			show_cur();
		}
	}

	return action_flag;
}

int16 Room41::use_brief() {
	int16 action_flag = false;

	if (is_cur_inventar(BRIEF_INV)) {
		action_flag = true;
		stop_hoggy();
		auto_move(6, P_CHEWY);
		start_aad_wait(126, -1);
		start_hoggy();

	} else if (is_cur_inventar(BRIEF2_INV)) {
		action_flag = true;
		auto_move(6, P_CHEWY);
		del_inventar(_G(spieler).AkInvent);
		stop_hoggy();
		start_aad_wait(186, -1);
		start_hoggy();
		atds->set_ats_str(206, 1, ATS_DATEI);
		_G(spieler).R28Briefkasten = true;
		_G(spieler).R40TrainMove = true;
		_G(spieler).R28PostCar = true;
		_G(spieler).R42BriefOk = true;
	}

	show_cur();
	return action_flag;
}

void Room41::sub_dia() {
	aad_wait(-1);
	start_aad_wait(161, -1);

	if (_G(spieler).R41LolaOk) {
		start_aad_wait(163, -1);
		atds->hide_item(11, 0, 2);
		stop_ads_dialog();
		auto_move(5, P_CHEWY);
		new_invent_2_cur(PAPIER_INV);

	} else {
		start_aad_wait(162, -1);
	}
}

} // namespace Rooms
} // namespace Chewy
