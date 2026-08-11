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
#include "chewy/rooms/room77.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room77::entry() {
	_G(det)->playSound(0, 0);
	_G(det)->playSound(0, 1);
	_G(gameState).ScrollxStep = 2;
	if (_G(gameState).r76State == 1) {
		_G(gameState).flags29_4 = true;
		_G(atds)->set_all_ats_str(452, 1, ATS_DATA);
	}

	if (_G(gameState).flags29_4) {
		_G(det)->set_static_ani(0, -1);

		for (int i = 0; i < 3; ++i) {
			_G(det)->showStaticSpr(2 + i);
			_G(atds)->delControlBit(460 + i, ATS_ACTIVE_BIT);
		}
	}

	// WORKAROUND: Loading saved game in this room don't properly
	// reset the flag used for cutscenes when returning to village
	if (_G(flags).LoadGame) {
		_G(gameState).r76State = -1;

	} else if (_G(gameState).r76State == 1) {
		_G(gameState).r76State = -1;
		hideCur();
		_G(gameState).scrollx = 62;
		setPersonPos(158, 127, P_CHEWY, P_RIGHT);
		startAadWait(440);
		showCur();
	}
}

void Room77::xit() {
	_G(gameState).ScrollxStep = 1;
}

int Room77::procOpenBoatHoleWithCorkscrew() {
	if (_G(gameState).flags29_4 || !isCurInventory(103))
		return 0;

	hideCur();
	autoMove(1, P_CHEWY);
	int diaNr, aniId;
	if (_G(gameState).flags29_8) {
		start_spz_wait(14, 1, false, P_CHEWY);
		_G(atds)->set_all_ats_str(463, 1, ATS_DATA);
		_G(gameState).flags29_10 = true;
		_G(gameState).flags29_40 = true;
		diaNr = 441;
		aniId = 6;
	} else {
		diaNr = 439;
		aniId = 4;
	}

	cur_2_inventory();
	start_spz(aniId, 255, false, P_CHEWY);
	startAadWait(diaNr);
	showCur();
	if (_G(gameState).flags29_10)
		remove_inventory(103);

	return 1;
}

int Room77::procPlugBoatHoleWithRubber() {
	if (_G(gameState).flags29_4 || !isCurInventory(100))
		return 0;

	hideCur();
	autoMove(2, P_CHEWY);

	int diaNr, aniId;
	if (_G(gameState).flags29_8) {
		start_spz_wait(14, 1, false, P_CHEWY);
		delInventory(_G(cur)->getInventoryCursor());
		_G(atds)->set_all_ats_str(464, 1, ATS_DATA);
		_G(gameState).flags29_20 = true;
		_G(gameState).flags29_40 = true;
		diaNr = 442;
		aniId = 6;
	} else {
		diaNr = 439;
		aniId = 4;
	}

	start_spz(aniId, 255, false, P_CHEWY);
	startAadWait(diaNr);

	showCur();
	return 1;
}

} // namespace Rooms
} // namespace Chewy
