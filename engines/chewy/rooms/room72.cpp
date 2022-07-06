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
#include "chewy/rooms/room72.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room72::entry(int16 eib_nr) {
	_G(det)->playSound(0, 0);
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	
	if (_G(gameState).flags28_10) {
		_G(zoom_horizont) = 110;
		_G(det)->hideStaticSpr(0);
	} else {
		_G(gameState).ZoomXy[P_HOWARD][0] = 20;
		_G(gameState).ZoomXy[P_HOWARD][1] = 40;
		_G(gameState).ZoomXy[P_NICHELLE][0] = 20;
		_G(gameState).ZoomXy[P_NICHELLE][1] = 40;
		_G(zoom_horizont) = 140;
		_G(spieler_mi)[P_CHEWY].Mode = true;
		hideCur();
	}

	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;

	if (_G(flags).LoadGame || _G(gameState).flags28_10)
		return;

	switch (eib_nr) {
	case 112:
		setPersonPos(50, 129, P_CHEWY, P_RIGHT);
		setPersonPos(15, 103, P_HOWARD, P_RIGHT);
		setPersonPos(-15, 102, P_NICHELLE, P_RIGHT);
		goAutoXy(185, 130, P_CHEWY, ANI_GO);
		goAutoXy(147, 103, P_HOWARD, ANI_GO);
		goAutoXy(120, 104, P_NICHELLE, ANI_WAIT);
		proc1(84);
		register_cutscene(23);
		break;
	case 113:
		setPersonPos(213, 129, P_CHEWY, P_LEFT);
		setPersonPos(262, 103, P_HOWARD, P_LEFT);
		setPersonPos(293, 102, P_NICHELLE, P_LEFT);
		goAutoXy(120, 130, P_CHEWY, ANI_GO);
		goAutoXy(154, 102, P_HOWARD, ANI_GO);
		goAutoXy(186, 102, P_NICHELLE, ANI_WAIT);
		proc1(85);
		register_cutscene(23);
		break;
	default:
		break;
	}
}

void Room72::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;
}

void Room72::proc1(int16 flicNr) {
	_G(atds)->setControlBit(389, ATS_ACTIVE_BIT);
	_G(gameState).flags28_10 = true;
	waitShowScreen(7);
	startAadWait(437);
	flic_cut(flicNr);
	_G(gameState)._personRoomNr[P_HOWARD] = 76;
	_G(gameState)._personRoomNr[P_NICHELLE] = 76;
	_G(gameState)._personHide[P_CHEWY] = true;
	_G(det)->hideStaticSpr(0);
	setupScreen(DO_SETUP);
	_G(gameState)._personHide[P_CHEWY] = false;
	_G(spieler_mi)[P_CHEWY].Mode = false;
	showCur();
	switchRoom(76);
}

} // namespace Rooms
} // namespace Chewy
