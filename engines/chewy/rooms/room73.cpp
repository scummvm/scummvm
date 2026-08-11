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
#include "chewy/rooms/room73.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room73::entry(int16 eib_nr) {
	_G(det)->playSound(0, 0);
	_G(gameState).ScrollxStep = 2;
	_G(gameState).ZoomXy[P_HOWARD][0] = 70;
	_G(gameState).ZoomXy[P_HOWARD][1] = 100;
	_G(gameState).ZoomXy[P_NICHELLE][0] = 70;
	_G(gameState).ZoomXy[P_NICHELLE][1] = 100;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(zoom_horizont) = 110;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(SetUpScreenFunc) = setup_func;

	if (_G(gameState).flags28_20)
		moveBushes();

	if (_G(gameState).flags28_40)
		pickupMachete();

	if (_G(flags).LoadGame)
		return;

	switch (eib_nr) {
	case 106:
		topEntry();
		break;
	case 115:
		leftEntry();
		break;
	default:
		break;
	}
}

void Room73::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;

	if (_G(gameState)._personRoomNr[P_HOWARD] != 73)
		return;

	switch (eib_nr) {
	case 110:
		_G(gameState)._personRoomNr[P_HOWARD] = 71;
		_G(gameState)._personRoomNr[P_NICHELLE] = 71;
		break;
	case 111:
		_G(gameState)._personRoomNr[P_HOWARD] = 75;
		_G(gameState)._personRoomNr[P_NICHELLE] = 75;
		break;
	default:
		break;
	}
}

int Room73::procMoveBushes() {
	if (_G(cur)->usingInventoryCursor() || _G(gameState).flags28_20)
		return 0;

	hideCur();
	_G(gameState).flags28_20 = true;
	autoMove(3, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);
	moveBushes();
	showCur();

	return 1;	
}

int Room73::procPickupMachete() {
	if (_G(cur)->usingInventoryCursor())
		return 0;

	hideCur();
	_G(gameState).flags28_40 = true;
	autoMove(4, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);
	pickupMachete();
	new_invent_2_cur(99);
	showCur();

	return 1;
}

void Room73::moveBushes() {
	_G(atds)->delControlBit(433, ATS_ACTIVE_BIT);	// show machete hotspot
	_G(atds)->delControlBit(432, ATS_ACTIVE_BIT);	// show skeleton hotspot
	_G(det)->showStaticSpr(3);								// show skeleton + machete
	_G(det)->hideStaticSpr(4);								// hide skeleton
	_G(atds)->set_all_ats_str(430, 1, ATS_DATA);
}

void Room73::pickupMachete() {
	_G(atds)->setControlBit(433, ATS_ACTIVE_BIT);	// hide machete hotspot
	_G(atds)->setControlBit(432, ATS_ACTIVE_BIT);	// show skeleton hotspot
	_G(det)->hideStaticSpr(3);								// hide skeleton + machete
	_G(det)->showStaticSpr(4);								// show skeleton
}

} // namespace Rooms
} // namespace Chewy
