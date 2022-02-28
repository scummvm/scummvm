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
#include "chewy/dialogs/credits.h"
#include "chewy/rooms/room89.h"
#include "chewy/rooms/room66.h"

namespace Chewy {
namespace Rooms {

void Room89::entry() {
	_G(spieler).ScrollxStep = 2;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(SetUpScreenFunc) = setup_func;
	_G(timer_nr)[0] = _G(room)->set_timer(2, 10);
	if (_G(spieler).flags33_4) {
		_G(det)->showStaticSpr(5);
		if (!_G(spieler).flags32_80)
			_G(det)->showStaticSpr(6);
	}

	if (_G(flags).LoadGame) {
		_G(spieler).SVal2 = 0;
		return;
	}

	if (_G(spieler).scrollx != 5000) {
		_G(spieler).scrollx = 0;
		set_person_pos(116, 114, P_HOWARD, P_RIGHT);
		set_person_pos(93, 98, P_CHEWY, P_RIGHT);
	}

	if (_G(spieler).flags35_2) {
		hideCur();
		_G(spieler).flags35_2 = false;
		_G(spieler).SVal1 = 89;
		_G(spieler).SVal2 = 537;
		switch_room(92);
		start_aad_wait(490, -1);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_103);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_118);
		_G(spieler).SVal1 = 89;
		_G(spieler).SVal2 = 538;
		switch_room(92);
		showCur();
	} else if (_G(spieler).flags35_4) {
		// End sequence
		hideCur();
		set_person_pos(138, 82, P_CHEWY, P_RIGHT);
		set_person_pos(116, 114, P_HOWARD, P_RIGHT);
		set_person_pos(260, 57, P_NICHELLE, P_LEFT);
		_G(spieler).ZoomXy[P_NICHELLE][0] = _G(spieler).ZoomXy[P_NICHELLE][1] = 10;
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_117);
		register_cutscene(35);

		_G(fx_blend) = BLEND3;
		Room66::proc8(-1, 2, 3, 563);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_119);

		// Squash screen into a single point at the center
		int edi = -20;
		int var24 = 0;
		_G(out)->sprite_save(_G(Ci).TempArea, 0, 0, 320, 200, 320);
		for (int esi = 0; esi < 100; ++esi) {
			edi -= 3;
			var24 -= 2;
			_G(out)->setPointer(_G(workptr));
			_G(out)->cls();
			_G(out)->scale_set(_G(Ci).TempArea, ABS(edi) / 2, ABS(var24) / 2, edi, var24, _G(scr_width));
			_G(out)->setPointer(nullptr);
			_G(out)->back2screen(_G(workpage));
			g_events->delay(30);
			SHOULD_QUIT_RETURN;
		}

		// Unsquish out thanks for playing screen
		_G(out)->setPointer(_G(workptr));
		_G(out)->cls();
		// Those strings are also displayed in the the German version
		_G(out)->printxy(70, 80, 15, 0, 0, "Thank you for playing");
		_G(out)->printxy(70, 100, 15, 0, 0, "  CHEWY Esc from F5");
		_G(out)->sprite_save(_G(Ci).TempArea, 0, 0, 320, 200, 320);
		edi = -300;
		int var30 = -200;

		for (int esi = 0; esi < 100; ++esi) {
			edi += 3;
			var30 += 2;
			_G(out)->setPointer(_G(workptr));
			_G(out)->cls();
			_G(out)->scale_set(_G(Ci).TempArea, ABS(edi) / 2, ABS(var30) / 2, edi, var30, _G(scr_width));
			_G(out)->setPointer(nullptr);
			_G(out)->back2screen(_G(workpage));
			g_events->delay(30);
			SHOULD_QUIT_RETURN;
		}

		g_events->delay(3000);
		_G(out)->ausblenden(2);
		_G(out)->setPointer(_G(workptr));
		_G(out)->cls();
		_G(out)->setPointer(nullptr);
		_G(out)->cls();

		Dialogs::Credits::execute();
		
		_G(spieler).SVal4 = 1;
		_G(out)->ausblenden(2);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
	}

	_G(spieler).SVal2 = 0;
}

void Room89::xit() {
	_G(spieler).ScrollxStep = 1;
}

void Room89::setup_func() {
	calc_person_look();
	int destX;
	
	if (_G(spieler_vector)[P_CHEWY].Xypos[0] >= 230)
		destX = 318;
	else
		destX = 116;

	goAutoXy(destX, 114, P_HOWARD, ANI_GO);
}

void Room89::talk1() {
	Room66::proc8(1, 2, 3, 485);
}

int Room89::proc2() {
	if (_G(spieler).inv_cur || _G(spieler.flags33_2))
		return 0;

	hideCur();
	_G(spieler).flags33_2 = true;
	_G(spieler).SVal1 = 89;
	_G(spieler).SVal2 = 489;
	switch_room(92);
	start_aad_wait(490, -1);
	_G(out)->setPointer(nullptr);
	_G(out)->cls();
	_G(flags).NoPalAfterFlc = true;
	flic_cut(FCUT_102);
	register_cutscene(26);

	_G(spieler).SVal1 = 89;
	_G(spieler).SVal2 = 487;
	switch_room(92);
	showCur();

	return 1;
}

int Room89::proc4() {
	if (_G(spieler).inv_cur || _G(spieler).flags32_80)
		return 0;

	hideCur();
	autoMove(2, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);

	if (_G(spieler).flags33_4) {
		_G(atds)->set_ats_str(514, 2, ATS_DATA);
		_G(spieler).flags32_80 = true;
		_G(det)->hideStaticSpr(6);
		new_invent_2_cur(111);
		start_spz(CH_TALK6, 255, false, P_CHEWY);
		start_aad_wait(491, -1);
	} else {
		_G(det)->showStaticSpr(5);
		_G(det)->showStaticSpr(6);
		_G(spieler).flags33_4 = true;
		_G(atds)->set_ats_str(514, 1, ATS_DATA);
	}

	showCur();
	return 1;
}

int Room89::proc5() {
	if (_G(spieler).inv_cur)
		return 0;

	if (!_G(spieler).flags32_80 || !_G(spieler).flags33_1 || !_G(spieler).flags33_2)
		Room66::proc8(-1, 2, 3, 493);
	else {
		hideCur();
		Room66::proc8(-1, 2, 3, 486);
		_G(spieler).SVal1 = 89;
		_G(spieler).SVal2 = 488;
		switch_room(92);
		showCur();
	}
	
	return 1;
}

} // namespace Rooms
} // namespace Chewy
