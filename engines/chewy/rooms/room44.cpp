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
#include "chewy/rooms/room44.h"

namespace Chewy {
namespace Rooms {

void Room44::look_news() {
	int16 tmproom = _G(gameState)._personRoomNr[P_CHEWY];
	int16 tmpsx = _G(gameState).scrollx;
	int16 tmpsy = _G(gameState).scrolly;
	_G(gameState).scrollx = 0;
	_G(gameState).scrolly = 0;
	hide_person();
	_G(fx_blend) = BLEND1;
	_G(gameState)._personRoomNr[P_CHEWY] = 44;
	_G(room)->loadRoom(&_G(room_blk), 44, &_G(gameState));

	startAadWait(172);
	_G(gameState).scrollx = tmpsx;
	_G(gameState).scrolly = tmpsy;
	switchRoom(tmproom);
	show_person();
}

} // namespace Rooms
} // namespace Chewy
