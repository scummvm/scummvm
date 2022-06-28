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
#include "chewy/rooms/room69.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room69::entry(int16 eib_nr) {
	g_engine->_sound->playSound(0, 0);
	g_engine->_sound->playSound(0);
	
	_G(gameState).ScrollxStep = 2;
	_G(gameState).ZoomXy[P_HOWARD][0] = 46;
	_G(gameState).ZoomXy[P_HOWARD][1] = 90;
	_G(gameState).ZoomXy[P_NICHELLE][0] = 46;
	_G(gameState).ZoomXy[P_NICHELLE][1] = 90;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(zoom_horizont) = 110;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(SetUpScreenFunc) = setup_func;
	_G(gameState).DiaAMov = 2;
	if (_G(flags).LoadGame)
		return;

	_G(cur_hide_flag) = false;
	hideCur();

	if (eib_nr == 102)
		proc1();
	else {
		setPersonPos(295, 118, P_CHEWY, P_RIGHT);
		setPersonPos(237, 101, P_NICHELLE, P_RIGHT);
		setPersonPos(347, 119, P_HOWARD, P_RIGHT);
		goAutoXy(351, 97, P_HOWARD, ANI_WAIT);
		showCur();
	}
}

void Room69::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;

	if (_G(gameState)._personRoomNr[P_HOWARD] == 69 && eib_nr == 97) {
		_G(gameState)._personRoomNr[P_HOWARD] = 66;
		_G(gameState)._personRoomNr[P_NICHELLE] = 66;
	}
}

void Room69::setup_func() {
	calc_person_look();
}

void Room69::look_schild() {
	hideCur();
	autoMove(1, P_CHEWY);
	startAadWait(382);
	showCur();
}

int16 Room69::use_bruecke() {
	int16 action_flag = false;
	if (!_G(cur)->usingInventoryCursors()) {
		hideCur();
		action_flag = true;

		if (!_G(gameState).R68KarteDa) {
			calc_person_dia(P_HOWARD);
		} else {
			autoMove(3, P_CHEWY);
			_G(spieler_mi)[P_CHEWY].Mode = true;
			goAutoXy(253, 103, P_CHEWY, ANI_WAIT);
			goAutoXy(141, 103, P_CHEWY, ANI_WAIT);

			if (_G(gameState)._personRoomNr[P_NICHELLE] == 69)
				goAutoXy(255, 84, 2, ANI_GO);
			
			goAutoXy(82, 95, P_CHEWY, ANI_WAIT);
			goAutoXy(46, 83, P_CHEWY, ANI_WAIT);

			if (_G(gameState)._personRoomNr[P_HOWARD] == 69) {
				goAutoXy(258, 86, P_HOWARD, ANI_GO);
				goAutoXy(147, 84, P_NICHELLE, ANI_WAIT);
				goAutoXy(140, 86, P_HOWARD, ANI_GO);
				goAutoXy(94, 77, P_NICHELLE, ANI_WAIT);
				goAutoXy(94, 82, P_HOWARD, ANI_GO);
				goAutoXy(59, 68, P_NICHELLE, ANI_WAIT);
				_G(gameState)._personRoomNr[P_HOWARD] = 70;
				_G(gameState)._personRoomNr[P_NICHELLE] = 70;
			}

			_G(spieler_mi)[P_CHEWY].Mode = false;
			switchRoom(70);
		}
		showCur();
	}
	return action_flag;
}

void Room69::proc1() {
	_G(gameState).scrollx = 0;
	_G(spieler_mi)[P_CHEWY].Mode = true;
	setPersonPos(46, 83, P_CHEWY, P_RIGHT);
	setPersonPos(59, 68, P_NICHELLE, P_RIGHT);
	setPersonPos(94, 82, P_HOWARD, P_RIGHT);
	goAutoXy(59, 68, P_NICHELLE, ANI_GO);
	goAutoXy(94, 82, P_HOWARD, ANI_WAIT);
	goAutoXy(94, 77, P_NICHELLE, ANI_GO);
	goAutoXy(140, 86, P_HOWARD, ANI_WAIT);
	goAutoXy(82, 95, P_CHEWY, _G(gameState)._personRoomNr[P_HOWARD] == 69 ? ANI_GO : ANI_WAIT);
	goAutoXy(147, 84, P_NICHELLE, ANI_GO);
	goAutoXy(278, 86, P_HOWARD, ANI_WAIT);
	goAutoXy(255, 84, P_NICHELLE, ANI_WAIT);
	goAutoXy(237, 101, P_NICHELLE, ANI_GO);
	goAutoXy(351, 97, P_HOWARD, ANI_GO);
	goAutoXy(141, 109, P_CHEWY, ANI_GO);
	goAutoXy(253, 106, P_CHEWY, ANI_WAIT);
	autoMove(3, P_CHEWY);
	_G(spieler_mi)[P_CHEWY].Mode = false;
}

} // namespace Rooms
} // namespace Chewy
