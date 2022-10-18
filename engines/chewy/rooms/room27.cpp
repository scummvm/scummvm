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
#include "chewy/ani_dat.h"
#include "chewy/rooms/room27.h"

namespace Chewy {
namespace Rooms {

void Room27::entry() {
	if (_G(gameState)._personRoomNr[P_HOWARD] == 27) {
		_G(timer_nr)[0] = _G(room)->set_timer(0, 5);
		_G(det)->set_static_ani(0, -1);
		_G(atds)->delControlBit(274, ATS_ACTIVE_BIT);
	} else {
		_G(atds)->setControlBit(274, ATS_ACTIVE_BIT);
	}

	_G(gameState)._personHide[P_HOWARD] = true;
	_G(gameState).ScrollxStep = 2;
}

void Room27::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;
	hideCur();

	if (_G(gameState)._personRoomNr[P_HOWARD] == 27) {
		if (eib_nr == 55) {
			startAadWait(175);
			_G(room)->set_timer_status(0, TIMER_STOP);
			_G(det)->del_static_ani(0);
			startSetAILWait(2, 1, ANI_FRONT);
			_G(gameState)._personRoomNr[P_HOWARD] = 28;
			_G(spieler_mi)[P_HOWARD].Id = HOWARD_OBJ;
		} else if (_G(gameState).R27HowardGed < 3 && eib_nr != -1) {
			++_G(gameState).R27HowardGed;
			startAadWait(174);
		}
	}

	showCur();
}

void Room27::get_surimy() {
	_G(obj)->calc_all_static_detail();
	hideCur();
	autoMove(4, P_CHEWY);

	if (_G(gameState)._personRoomNr[P_HOWARD] == 27) {
		startAadWait(171);
	}

	showCur();
	_G(obj)->hide_sib(SIB_SURIMY_R27);
}

void Room27::talk_howard() {
	hideCur();
	autoMove(4, P_CHEWY);
	startAadWait(173);
	showCur();
}

void Room27::howard_ged() {
	if (_G(gameState).R27HowardGed < 3) {
		++_G(gameState).R27HowardGed;
	}
}

} // namespace Rooms
} // namespace Chewy
