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
#include "chewy/room.h"
#include "chewy/dialogs/credits.h"
#include "chewy/rooms/room89.h"
#include "chewy/rooms/room66.h"

namespace Chewy {
namespace Rooms {

void Room89::entry() {
	_G(spieler).ScrollxStep = 2;
	spieler_mi[P_HOWARD].Mode = true;
	SetUpScreenFunc = setup_func;
	_G(timer_nr)[0] = room->set_timer(2, 10);
	if (_G(spieler).flags33_4) {
		det->show_static_spr(5);
		if (!_G(spieler).flags32_80)
			det->show_static_spr(6);
	}

	if (flags.LoadGame) {
		_G(spieler).SVal2 = 0;
		return;
	}

	if (_G(spieler).scrollx != 5000) {
		_G(spieler).scrollx = 0;
		set_person_pos(116, 114, P_HOWARD, P_RIGHT);
		set_person_pos(93, 98, P_CHEWY, P_RIGHT);
	}

	if (_G(spieler).flags35_2) {
		hide_cur();
		_G(spieler).flags35_2 = false;
		_G(spieler).SVal1 = 89;
		_G(spieler).SVal2 = 537;
		switch_room(92);
		start_aad_wait(490, -1);
		out->setze_zeiger(nullptr);
		out->cls();
		flags.NoPalAfterFlc = true;
		flic_cut(103, CFO_MODE);
		out->setze_zeiger(nullptr);
		out->cls();
		flags.NoPalAfterFlc = true;
		flic_cut(118, CFO_MODE);
		_G(spieler).SVal1 = 89;
		_G(spieler).SVal2 = 538;
		switch_room(92);
		show_cur();
	} else if (_G(spieler).flags35_4) {
		// End sequence
		hide_cur();
		set_person_pos(138, 82, P_CHEWY, P_RIGHT);
		set_person_pos(116, 114, P_HOWARD, P_RIGHT);
		set_person_pos(260, 57, P_NICHELLE, P_LEFT);
		_G(spieler).ZoomXy[P_NICHELLE][0] = _G(spieler).ZoomXy[P_NICHELLE][1] = 10;
		out->setze_zeiger(nullptr);
		out->cls();
		flags.NoPalAfterFlc = true;
		flic_cut(117, CFO_MODE);
		register_cutscene(35);

		fx_blend = BLEND3;
		Room66::proc8(-1, 2, 3, 563);
		out->setze_zeiger(nullptr);
		out->cls();
		flags.NoPalAfterFlc = true;
		flic_cut(119, CFO_MODE);

		// Squash screen into a single point at the center
		int edi = -20;
		int var24 = 0;
		out->sprite_save(Ci.TempArea, 0, 0, 320, 200, 320);
		for (int esi = 0; esi < 100; ++esi) {
			edi -= 3;
			var24 -= 2;
			out->setze_zeiger(workptr);
			out->cls();
			out->scale_set(Ci.TempArea, ABS(edi) / 2, ABS(var24) / 2, edi, var24, scr_width);
			out->setze_zeiger(nullptr);
			out->back2screen(workpage);
			g_events->delay(30);
			SHOULD_QUIT_RETURN;
		}

		// Unsquish out thanks for playing screen
		out->setze_zeiger(workptr);
		out->cls();
		out->printxy(70, 80, 15, 0, 0, "Thank you for playing");
		out->printxy(70, 100, 15, 0, 0, "  CHEWY ESC from F5");
		out->sprite_save(Ci.TempArea, 0, 0, 320, 200, 320);
		edi = -300;
		int var30 = -200;

		for (int esi = 0; esi < 100; ++esi) {
			edi += 3;
			var30 += 2;
			out->setze_zeiger(workptr);
			out->cls();
			out->scale_set(Ci.TempArea, ABS(edi) / 2, ABS(var30) / 2, edi, var30, scr_width);
			out->setze_zeiger(nullptr);
			out->back2screen(workpage);
			g_events->delay(30);
			SHOULD_QUIT_RETURN;
		}

		g_events->delay(3000);
		out->ausblenden(2);
		out->setze_zeiger(workptr);
		out->cls();
		out->setze_zeiger(nullptr);
		out->cls();

		Dialogs::Credits::execute();
		
		_G(spieler).SVal4 = 1;
		out->ausblenden(2);
		out->setze_zeiger(nullptr);
		out->cls();
	}

	_G(spieler).SVal2 = 0;
}

void Room89::xit() {
	_G(spieler).ScrollxStep = 1;
}

void Room89::setup_func() {
	calc_person_look();
	int destX;
	
	if (spieler_vector[P_CHEWY].Xypos[0] >= 230)
		destX = 318;
	else
		destX = 116;

	go_auto_xy(destX, 114, P_HOWARD, ANI_GO);
}

void Room89::talk1() {
	Room66::proc8(1, 2, 3, 485);
}

int Room89::proc2() {
	if (_G(spieler).inv_cur || _G(spieler.flags33_2))
		return 0;

	hide_cur();
	_G(spieler).flags33_2 = true;
	_G(spieler).SVal1 = 89;
	_G(spieler).SVal2 = 489;
	switch_room(92);
	start_aad_wait(490, -1);
	out->setze_zeiger(nullptr);
	out->cls();
	flags.NoPalAfterFlc = true;
	flic_cut(102, CFO_MODE);
	register_cutscene(26);

	_G(spieler).SVal1 = 89;
	_G(spieler).SVal2 = 487;
	switch_room(92);
	show_cur();

	return 1;
}

int Room89::proc4() {
	if (_G(spieler).inv_cur || _G(spieler).flags32_80)
		return 0;

	hide_cur();
	auto_move(2, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);

	if (_G(spieler).flags33_4) {
		atds->set_ats_str(514, 2, ATS_DATEI);
		_G(spieler).flags32_80 = true;
		det->hide_static_spr(6);
		new_invent_2_cur(111);
		start_spz(5, 255, false, P_CHEWY);
		start_aad_wait(491, -1);
	} else {
		det->show_static_spr(5);
		det->show_static_spr(6);
		_G(spieler).flags33_4 = true;
		atds->set_ats_str(514, 1, ATS_DATEI);
	}

	show_cur();
	return 1;
}

int Room89::proc5() {
	if (_G(spieler).inv_cur)
		return 0;

	if (!_G(spieler).flags32_80 || !_G(spieler).flags33_1 || !_G(spieler).flags33_2)
		Room66::proc8(-1, 2, 3, 493);
	else {
		hide_cur();
		Room66::proc8(-1, 2, 3, 486);
		_G(spieler).SVal1 = 89;
		_G(spieler).SVal2 = 488;
		switch_room(92);
		show_cur();
	}
	
	return 1;
}

} // namespace Rooms
} // namespace Chewy
