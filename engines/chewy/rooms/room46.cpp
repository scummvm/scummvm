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
#include "chewy/rooms/room46.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room46::entry(int16 eib_nr) {
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 45) {
		_G(spieler).PersonRoomNr[P_HOWARD] = 46;
		_G(SetUpScreenFunc) = setup_func;

		if (eib_nr == 79) {
			set_person_pos(140, 90, P_HOWARD, P_LEFT);
			autoMove(3, P_CHEWY);
			set_person_spr(P_LEFT, P_CHEWY);
		}
	}

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 46) {
		_G(spieler_mi)[P_HOWARD].Mode = true;
		_G(SetUpScreenFunc) = setup_func;

		if (_G(spieler).flags33_8) {
			_G(maus_links_click) = false;
			hideCur();
			set_person_pos(175, 73, P_HOWARD, P_LEFT);
			set_person_pos(147, 109, P_CHEWY, P_LEFT);
			_G(spieler).flags33_8 = false;
			_G(spieler).SVal1 = 46;
			wait_show_screen(10);

			start_spz_wait(64, 1, false, P_CHEWY);
			start_spz(65, 255, false, P_CHEWY);
			start_aad_wait(494, -1);
			_G(spieler).SVal2 = 504;
			switch_room(92);
			start_spz(65, 255, false, P_CHEWY);
			start_aad_wait(495, -1);
			_G(spieler).room_e_obj[78].Exit = 90;
			showCur();
		}
	}

	if (_G(spieler).R47SchlossOk && !_G(spieler).R46GetLeder)
		bodo();
}

void Room46::xit() {
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 46) {
		_G(spieler_mi)[P_HOWARD].Mode = false;
	}
}

void Room46::setup_func() {
	calc_person_look();
	int16 y = 64;
	const int16 ch_x = _G(spieler_vector)[P_CHEWY].Xypos[0];

	int16 x;
	if (ch_x > 160) {
		x = 260;
	} else {
		x = 120;
	}

	if (_G(HowardMov) && _G(flags).ExitMov) {
		x = 160;
		y = 200;
	}

	goAutoXy(x, y, P_HOWARD, ANI_GO);
}

void Room46::bodo() {
	hideCur();
	_G(det)->show_static_spr(0);
	_G(person_end_phase)[P_CHEWY] = P_RIGHT;
	start_aad_wait(240, -1);
	_G(spieler_mi)[P_CHEWY].Mode = true;
	goAutoXy(42, 76, P_CHEWY, ANI_WAIT);
	start_aad_wait(250, -1);
	autoMove(1, P_CHEWY);
	set_person_spr(P_RIGHT, P_CHEWY);
	start_aad_wait(251, -1);
	autoMove(2, P_CHEWY);

	_G(spieler_mi)[P_CHEWY].Mode = false;
	start_detail_wait(0, 1, ANI_FRONT);
	_G(det)->start_detail(5, 255, ANI_FRONT);
	start_aad_wait(241, -1);
	_G(det)->stop_detail(5);
	_G(det)->show_static_spr(5);
	start_aad_wait(609, -1);
	_G(det)->hide_static_spr(5);
	start_detail_wait(2, 1, ANI_FRONT);
	start_detail_wait(3, 1, ANI_FRONT);
	_G(det)->start_detail(4, 255, ANI_FRONT);
	start_aad_wait(242, -1);
	_G(det)->stop_detail(4);

	_G(det)->show_static_spr(6);
	start_aad_wait(610, -1);
	_G(det)->hide_static_spr(6);
	start_detail_wait(6, 1, ANI_FRONT);
	_G(det)->show_static_spr(3);
	_G(spieler).PersonHide[P_CHEWY] = true;
	start_detail_wait(1, 1, ANI_FRONT);
	flic_cut(FCUT_065);
	_G(out)->setPointer(nullptr);
	_G(out)->cls();
	register_cutscene(16);
	
	_G(spieler).PersonHide[P_CHEWY] = false;
	_G(det)->hide_static_spr(0);
	_G(det)->hide_static_spr(3);
	load_chewy_taf(CHEWY_ROCKER);
	_G(spieler).R28ChewyPump = false;
	
	set_person_pos(213, 118, P_CHEWY, P_RIGHT);
	start_aad_wait(243, -1);
	invent_2_slot(UHR_INV);
	invent_2_slot(RING_INV);
	_G(spieler).R46GetLeder = true;
	showCur();
}

void Room46::kloppe() {
	for (int16 i = 0; i < 4; i++) {
		_G(mem)->file->select_pool_item(_G(Ci).Handle, FCUT_065);
		_G(flc)->custom_play(&_G(Ci));
		
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		start_aad(244 + i, -1);
		int16 delay = _G(spieler).DelaySpeed * 50;
		_G(atds)->print_aad(0, 0);

		if (_G(spieler).SpeechSwitch) {
			g_engine->_sound->waitForSpeechToFinish();
			continue;
		}
		
		while (_G(in)->get_switch_code() == Common::KEYCODE_INVALID && delay) {
			--delay;
			SHOULD_QUIT_RETURN;
		}
	}

	_G(mem)->file->select_pool_item(_G(Ci).Handle, 66);
	_G(flc)->custom_play(&_G(Ci));
	_G(sndPlayer)->fadeOut(0);
	_G(out)->cls();

	while (_G(sndPlayer)->musicPlaying() && !SHOULD_QUIT) {
		EVENTS_UPDATE;
	}
}

int16 Room46::use_schloss() {
	int16 action_ret = false;

	if (!_G(spieler).inv_cur) {
		action_ret = true;

		if (!_G(spieler).R46GetLeder) {
			autoMove(1, P_CHEWY);
			switch_room(47);
		} else {
			start_aad_wait(252, -1);
		}
	}

	return action_ret;
}

} // namespace Rooms
} // namespace Chewy
