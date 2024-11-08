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
#include "chewy/rooms/room02.h"

namespace Chewy {
namespace Rooms {

#define ANI_5 5
#define GRID_FLASHING 7

static const AniBlock ABLOCK4[2] = {
	{ GRID_FLASHING, 3, ANI_FRONT, ANI_WAIT, 0 },
	{ GRID_FLASHING, 12, ANI_FRONT, ANI_GO, 0 }
};


void Room2::entry() {
	if (!_G(gameState).R2ElectrocutedBork)
		_G(det)->startDetail(5, 255, ANI_FRONT);
}

void Room2::jump_out_r1(int16 nr) {
	_G(gameState)._personHide[P_CHEWY] = true;
	startSetAILWait(nr, 1, ANI_FRONT);
	setupScreen(DO_SETUP);
	_G(det)->stopDetail(6);
	setPersonPos(32, 127, P_CHEWY, P_LEFT);
	_G(gameState)._personHide[P_CHEWY] = false;
	clear_prog_ani();
	switchRoom(1);
	setShadowPalette(2, true);
}

void Room2::electrifyWalkway1() {
	_G(det)->startDetail(ANI_5, 255, ANI_FRONT);
	start_spz(CH_TALK6, 255, false, ANI_FRONT);
	startAadWait(49);
	_G(det)->stopDetail(ANI_5);

	_G(det)->startDetail(GRID_FLASHING, 12, ANI_FRONT);
	_G(gameState).R2ElectrocutedBork = true;
	delInventory(_G(cur)->getInventoryCursor());

	_G(atds)->delControlBit(11, ATS_COUNT_BIT);
	_G(atds)->delControlBit(11, ATS_ACTION_BIT);
	_G(atds)->delControlBit(19, ATS_COUNT_BIT);
	_G(atds)->delControlBit(25, ATS_ACTIVE_BIT);
	_G(atds)->setControlBit(8, ATS_COUNT_BIT);
	_G(atds)->set_all_ats_str(11, 1, ATS_DATA);
}

void Room2::electrifyWalkway2() {
	start_spz(CH_TALK12, 255, false, P_CHEWY);
	startAadWait(47);
}

void Room2::gedAction(int index) {
	if (index == 0) {
		_G(det)->stopDetail(5);
		if (!_G(gameState).R2ElectrocutedBork)
			_G(det)->startDetail(6, 2, ANI_FRONT);
		else
			startAniBlock(2, ABLOCK4);

		jump_out_r1(9);
	}
}

} // namespace Rooms
} // namespace Chewy
