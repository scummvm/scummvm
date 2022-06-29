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
#include "chewy/rooms/room16.h"
#include "chewy/rooms/room23.h"

namespace Chewy {
namespace Rooms {

void Room16::entry() {
	_G(zoom_horizont) = 140;

	if (!_G(gameState).R16F5Exit) {
		_G(det)->showStaticSpr(4);
		_G(gameState).room_e_obj[32].Attribut = 255;
		_G(atds)->delControlBit(124, ATS_ACTIVE_BIT);
	} else {
		_G(det)->hideStaticSpr(4);
		_G(gameState).room_e_obj[32].Attribut = EXIT_LEFT;
		_G(atds)->setControlBit(124, ATS_ACTIVE_BIT);
	}
}

int16 Room16::use_gleiter() {
	int16 action_flag = false;

	if (!_G(cur)->usingInventoryCursor()) {
		action_flag = true;
		autoMove(6, P_CHEWY);
		_G(gameState).R23GleiterExit = 16;
		Room23::cockpit();
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
