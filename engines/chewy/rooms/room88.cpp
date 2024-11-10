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
#include "chewy/detail.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/room.h"
#include "chewy/rooms/room88.h"

namespace Chewy {
namespace Rooms {

void Room88::entry() {
	_G(gameState).scrollx = 0;
	setPersonPos(163, 122, P_CHEWY, P_LEFT);
	setPersonPos(59, 107, P_HOWARD, P_RIGHT);
	setPersonPos(91, 110, P_NICHELLE, P_RIGHT);
	
	_G(det)->showStaticSpr(1 + (_G(gameState).flags30_10 ? 1 : 0));

	_G(SetUpScreenFunc) = calc_person_look;

	if (_G(gameState).flags32_10) {
		_G(det)->startDetail(4, 255, false);
		_G(atds)->delControlBit(505, ATS_ACTIVE_BIT);
	}
}

void Room88::xit() {
	_G(gameState).flags31_8 = true;
	_G(gameState)._personRoomNr[P_HOWARD] = _G(gameState).R88UsedMonkey ? 84 : 82;
	// Don't move Nichelle if she has been abducted in room 86
	if (_G(gameState)._personRoomNr[P_NICHELLE] != 0)
		_G(gameState)._personRoomNr[P_NICHELLE] = _G(gameState)._personRoomNr[P_HOWARD];
}

int Room88::proc1() {
	if (_G(cur)->usingInventoryCursor())
		return 0;

	hideCur();
	autoMove(0, P_CHEWY);
	switchRoom(_G(gameState).R88UsedMonkey ? 84 : 82);
	_G(menu_item) = CUR_WALK;
	cursorChoice(CUR_WALK);
	showCur();
	
	return 1;
}

int Room88::proc2() {
	if (_G(cur)->usingInventoryCursor())
		return 0;

	hideCur();
	autoMove(1, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);
	_G(det)->showStaticSpr(0);
	startSetAILWait(0, 1, _G(gameState).flags30_10 ? ANI_GO : ANI_FRONT);
	_G(det)->hideStaticSpr(1 + (_G(gameState).flags30_10 ? 1 : 0));
	_G(gameState).flags31_10 = false;
	_G(det)->showStaticSpr(1 + (!_G(gameState).flags30_10 ? 1 : 0));
	start_spz_wait(13, 1, false, P_CHEWY);
	_G(det)->hideStaticSpr(0);
	
	showCur();
	return 1;
}

int Room88::proc3() {
	if (_G(cur)->usingInventoryCursor())
		return 0;

	hideCur();
	autoMove(2, P_CHEWY);

	if (!_G(gameState).flags30_80 || _G(gameState).R88UsedMonkey) {
		hideCur();
		if (_G(gameState).flags32_10) {
			startAadWait(480);
			_G(out)->setPointer(nullptr);
			_G(out)->cls();
			_G(flags).NoPalAfterFlc = true;
			flic_cut(FCUT_087);
			_G(out)->setPointer(nullptr);
			_G(out)->cls();
			_G(gameState).flags32_40 = true;
			switchRoom(84);
		} else {
			startAadWait(465);
		}
	} else {
		startAadWait(466);
		start_spz_wait(13, 1, false, P_CHEWY);

		const int aniNr = 1 + (_G(gameState).flags31_10 ? 1 : 0);

		for (int i = 0; i < 3; ++i) {
			startSetAILWait(aniNr, 1, ANI_FRONT);
			startSetAILWait(aniNr, 1, ANI_GO);
		}

		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_088);
		_G(gameState)._personHide[P_CHEWY] = true;
		_G(gameState).flags32_1 = true;
		_G(out)->raster_col(0, 0, 0, 0);
		switchRoom(80);
		hideCur();
		startSetAILWait(1, 1, ANI_FRONT);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_086);
		register_cutscene(24);

		_G(gameState)._personHide[P_CHEWY] = false;
		_G(gameState)._personRoomNr[P_HOWARD] = 88;
		_G(gameState)._personRoomNr[P_NICHELLE] = 88;
		switchRoom(88);
		_G(gameState).R88UsedMonkey = true;
		_G(gameState).flags32_1 = false;
	}

	showCur();
	return 1;
}

} // namespace Rooms
} // namespace Chewy
