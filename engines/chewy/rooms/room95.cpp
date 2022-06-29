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
#include "chewy/rooms/room95.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room95::entry(int16 eib_nr) {
	g_engine->_sound->playSound(0, 0);
	g_engine->_sound->playSound(0);
	_G(SetUpScreenFunc) = setup_func;	
	_G(zoom_horizont) = 140;
	_G(gameState).ScrollxStep = 2;
	_G(gameState).ZoomXy[P_HOWARD][0] = _G(gameState).ZoomXy[P_HOWARD][1] = 30;
	_G(spieler_mi)[P_HOWARD].Mode = true;

	if (_G(gameState).flags35_20)
		_G(det)->showStaticSpr(10);

	if (_G(flags).LoadGame)
		return;

	hideCur();
	if (eib_nr == 138) {
		setPersonPos(-20, 120, P_HOWARD, ANI_GO);
		autoMove(2, P_CHEWY);
	} else {
		setPersonPos(473, 83, P_HOWARD, P_RIGHT);
	}

	showCur();
}

void Room95::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;

	if (eib_nr == 139)
		_G(gameState)._personRoomNr[P_HOWARD] = 94;
	else if (eib_nr == 140)
		_G(gameState)._personRoomNr[P_HOWARD] = 96;
}

void Room95::setup_func() {
	calc_person_look();
	int posX = _G(moveState)[P_CHEWY].Xypos[0];
	int destX;
	int destY = 120;
	
	if (posX < 130)
		destX = 40;
	else if (posX < 312)
		destX = 221;
	else if (posX < 445)
		destX = 342;
	else
		destX = 536;

	if (_G(HowardMov) == 1) {
		destX = 473;
		destY = 83;
	}

	goAutoXy(destX, destY, P_HOWARD, ANI_GO);
}

int Room95::proc2() {
	hideCur();
	autoMove(3, P_CHEWY);
	if (isCurInventory(113)) {
		delInventory(_G(cur)->getInventoryCursor());

		start_spz_wait(14, 1, false, P_CHEWY);
		_G(gameState).flags35_20 = true;
		_G(det)->showStaticSpr(10);

		showCur();
		return 1;
	}

	if (_G(cur)->usingInventoryCursor()) {
		showCur();
		return 0;
	}

	if (_G(gameState).flags35_20) {
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_133);
		register_cutscene(33);
		_G(gameState)._personRoomNr[P_HOWARD] = 97;
		switchRoom(97);
	} else {
		start_spz(CH_TALK1, 255, false, P_CHEWY);
		startAadWait(548);
	}

	showCur();
	return 1;
}

} // namespace Rooms
} // namespace Chewy
