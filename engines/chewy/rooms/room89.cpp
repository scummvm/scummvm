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

#include "chewy/cursor.h"
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
	_G(gameState).ScrollxStep = 2;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(SetUpScreenFunc) = setup_func;
	_G(timer_nr)[0] = _G(room)->set_timer(2, 10);
	if (_G(gameState).flags33_4) {
		_G(det)->showStaticSpr(5);
		if (!_G(gameState).flags32_80)
			_G(det)->showStaticSpr(6);
	}

	if (_G(flags).LoadGame) {
		_G(gameState).SVal2 = 0;
		return;
	}

	if (_G(gameState).scrollx != 5000) {
		_G(gameState).scrollx = 0;
		setPersonPos(116, 114, P_HOWARD, P_RIGHT);
		setPersonPos(93, 98, P_CHEWY, P_RIGHT);
	}

	if (_G(gameState).flags35_2) {
		hideCur();
		_G(gameState).flags35_2 = false;
		_G(gameState).SVal1 = 89;
		_G(gameState).SVal2 = 537;
		switchRoom(92);
		startAadWait(490);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_103);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_118);
		_G(gameState).SVal1 = 89;
		_G(gameState).SVal2 = 538;
		switchRoom(92);
		showCur();
	} else if (_G(gameState).flags35_4) {
		// End sequence
		hideCur();
		setPersonPos(138, 82, P_CHEWY, P_RIGHT);
		setPersonPos(116, 114, P_HOWARD, P_RIGHT);
		setPersonPos(260, 57, P_NICHELLE, P_LEFT);
		_G(gameState).ZoomXy[P_NICHELLE][0] = _G(gameState).ZoomXy[P_NICHELLE][1] = 10;
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
		_G(out)->spriteSave(_G(tempArea), 0, 0, 320, 200);
		for (int esi = 0; esi < 100; ++esi) {
			edi -= 3;
			var24 -= 2;
			_G(out)->setPointer(_G(workptr));
			_G(out)->cls();
			_G(out)->scale_set(_G(tempArea), ABS(edi) / 2, ABS(var24) / 2, edi, var24, _G(scr_width));
			_G(out)->setPointer(nullptr);
			_G(out)->copyToScreen();
			g_events->delay(30);
			SHOULD_QUIT_RETURN;
		}

		// Unsquish out thanks for playing screen
		_G(out)->setPointer(_G(workptr));
		_G(out)->cls();
		// Those strings are also displayed in the German version
		_G(out)->printxy(70, 80, 15, 0, 0, "Thank you for playing");
		_G(out)->printxy(70, 100, 15, 0, 0, "  CHEWY Esc from F5");
		_G(out)->spriteSave(_G(tempArea), 0, 0, 320, 200);
		edi = -300;
		int var30 = -200;

		for (int esi = 0; esi < 100; ++esi) {
			edi += 3;
			var30 += 2;
			_G(out)->setPointer(_G(workptr));
			_G(out)->cls();
			_G(out)->scale_set(_G(tempArea), ABS(edi) / 2, ABS(var30) / 2, edi, var30, _G(scr_width));
			_G(out)->setPointer(nullptr);
			_G(out)->copyToScreen();
			g_events->delay(30);
			SHOULD_QUIT_RETURN;
		}

		g_events->delay(3000);
		_G(out)->fadeOut();
		_G(out)->setPointer(_G(workptr));
		_G(out)->cls();
		_G(out)->setPointer(nullptr);
		_G(out)->cls();

		Dialogs::Credits::execute();
		
		_G(gameState).SVal4 = 1;
		_G(out)->fadeOut();
		_G(out)->setPointer(nullptr);
		_G(out)->cls();

		// Quit the game
		g_engine->quitGame();
	}

	_G(gameState).SVal2 = 0;
}

void Room89::xit() {
	_G(gameState).ScrollxStep = 1;
}

void Room89::setup_func() {
	calc_person_look();
	int destX;
	
	if (_G(moveState)[P_CHEWY].Xypos[0] >= 230)
		destX = 318;
	else
		destX = 116;

	goAutoXy(destX, 114, P_HOWARD, ANI_GO);
}

void Room89::talk1() {
	Room66::proc8(1, 2, 3, 485);
}

int Room89::proc2() {
	if (_G(cur)->usingInventoryCursor() || _G(gameState).flags33_2)
		return 0;

	hideCur();
	_G(gameState).flags33_2 = true;
	_G(gameState).SVal1 = 89;
	_G(gameState).SVal2 = 489;
	switchRoom(92);
	startAadWait(490);
	_G(out)->setPointer(nullptr);
	_G(out)->cls();
	_G(flags).NoPalAfterFlc = true;
	flic_cut(FCUT_102);
	register_cutscene(26);

	_G(gameState).SVal1 = 89;
	_G(gameState).SVal2 = 487;
	switchRoom(92);
	showCur();

	return 1;
}

int Room89::proc4() {
	if (_G(cur)->usingInventoryCursor() || _G(gameState).flags32_80)
		return 0;

	hideCur();
	autoMove(2, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);

	if (_G(gameState).flags33_4) {
		_G(atds)->set_all_ats_str(514, 2, ATS_DATA);
		_G(gameState).flags32_80 = true;
		_G(det)->hideStaticSpr(6);
		new_invent_2_cur(111);
		start_spz(CH_TALK6, 255, false, P_CHEWY);
		startAadWait(491);
	} else {
		_G(det)->showStaticSpr(5);
		_G(det)->showStaticSpr(6);
		_G(gameState).flags33_4 = true;
		_G(atds)->set_all_ats_str(514, 1, ATS_DATA);
	}

	showCur();
	return 1;
}

int Room89::proc5() {
	if (_G(cur)->usingInventoryCursor())
		return 0;

	if (!_G(gameState).flags32_80 || !_G(gameState).flags33_1 || !_G(gameState).flags33_2)
		Room66::proc8(-1, 2, 3, 493);
	else {
		hideCur();
		Room66::proc8(-1, 2, 3, 486);
		_G(gameState).SVal1 = 89;
		_G(gameState).SVal2 = 488;
		switchRoom(92);
		showCur();
	}
	
	return 1;
}

} // namespace Rooms
} // namespace Chewy
