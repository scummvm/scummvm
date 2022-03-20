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
#include "chewy/rooms/room96.h"

namespace Chewy {
namespace Rooms {

void Room96::entry() {
	_G(zoom_horizont) = 140;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 1;
	_G(gameState).ScrollxStep = 2;
	_G(gameState).ZoomXy[P_HOWARD][0] = 30;
	_G(gameState).ZoomXy[P_HOWARD][1] = 50;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(SetUpScreenFunc) = setup_func;
	_G(spieler_mi)[P_CHEWY].Mode = true;

	if (_G(flags).LoadGame)
		return;

	hideCur();
	setPersonPos(93, 62, P_HOWARD, P_RIGHT);
	setPersonPos(116, 74, P_CHEWY, P_RIGHT);
	autoMove(1, P_CHEWY);
	showCur();
}

void Room96::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;

	if (eib_nr == 141)
		_G(gameState)._personRoomNr[P_HOWARD] = 95;
}

void Room96::setup_func() {
	calc_person_look();
	int destX, destY = 62;
	
	if (_G(spieler_vector)[P_CHEWY].Xypos[0] >= 120)
		destX = 121;
	else
		destX = 93;

	if (_G(HowardMov) == 1) {
		destX = 49;
		destY = 60;
	}

	goAutoXy(destX, destY, P_HOWARD, ANI_GO);
}

} // namespace Rooms
} // namespace Chewy
