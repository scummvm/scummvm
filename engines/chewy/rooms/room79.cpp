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
#include "chewy/rooms/room79.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room79::entry() {
	g_engine->_sound->playSound(0, 0);
	g_engine->_sound->playSound(0);
	_G(gameState).ScrollxStep = 2;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(gameState)._personHide[P_CHEWY] = false;
	_G(gameState)._personHide[P_HOWARD] = false;
	_G(gameState)._personHide[P_NICHELLE] = false;
	_G(gameState)._personRoomNr[P_HOWARD] = 79;
	_G(gameState)._personRoomNr[P_NICHELLE] = 79;
	if (!_G(flags).LoadGame) {
		setPersonPos(459, 114, P_CHEWY, P_LEFT);
		setPersonPos(568, 65, P_HOWARD, P_LEFT);
		setPersonPos(534, 75, P_NICHELLE, P_LEFT);
		_G(gameState).scrollx = 300;
		hideCur();
		startAadWait(484);
		_G(gameState).PersonGlobalDia[P_HOWARD] = 10026;
		_G(gameState).PersonDiaRoom[P_HOWARD] = true;
		showCur();
	}

	_G(SetUpScreenFunc) = setup_func;
}

void Room79::xit() {
	_G(gameState).R79Val[P_CHEWY] = 1;
	_G(gameState).R79Val[P_HOWARD] = 1;
	_G(gameState)._personRoomNr[P_HOWARD] = 80;
	_G(gameState).R79Val[P_NICHELLE] = 1;
	_G(gameState)._personRoomNr[P_NICHELLE] = 80;
}

void Room79::setup_func() {
	calc_person_look();
	const int posX = _G(moveState)[P_CHEWY].Xypos[0];

	int howDestX, howDestY, nicDestX, nicDestY;

	if (posX < 160) {
		howDestX = 228;
		howDestY = 102;
		nicDestX = 191;
		nicDestY = 97;
	} else if (posX < 280) {
		howDestX = 346;
		howDestY = 98;
		nicDestX = 307;
		nicDestY = 100;
	} else if (posX < 390) {
		howDestX = 463;
		howDestY = 96;
		nicDestX = 424;
		nicDestY = 94;
	} else {
		howDestX = 568;
		howDestY = 65;
		nicDestX = 534;
		nicDestY = 75;
	}

	goAutoXy(howDestX, howDestY, P_HOWARD, ANI_GO);
	goAutoXy(nicDestX, nicDestY, P_NICHELLE, ANI_GO);
}

} // namespace Rooms
} // namespace Chewy
