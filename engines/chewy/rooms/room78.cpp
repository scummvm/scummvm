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
#include "chewy/rooms/room78.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room78::entry() {
	_G(gameState).r76State = -1;
	_G(det)->playSound(0, 1);
	hideCur();
	_G(gameState).ScrollxStep = 1;
	_G(gameState)._personHide[P_CHEWY] = true;
	_G(gameState)._personHide[P_HOWARD] = true;
	_G(gameState)._personHide[P_NICHELLE] = true;
	_G(gameState).scrollx = 320;
	setPersonPos(0, 0, P_CHEWY, P_RIGHT);

	int delay = 0;
	int destRoom;

	if (_G(gameState).flags29_80) {
		_G(det)->startDetail(3, 255, false);

		int destX = 592;
		while (destX > 0) {
			_G(det)->setDetailPos(3, destX, 77);
			if (delay)
				--delay;
			else {
				delay = _G(gameState).DelaySpeed / 2;
				destX -= 4;
			}
			setupScreen(DO_SETUP);
		}
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_101);
		destRoom = 79;
	} else {
		_G(det)->startDetail(0, 255, false);
		_G(det)->startDetail(4, 255, false);
		int det0DestX = 608;
		int det4DestX = 570;
		bool exitLoopFlag = false;
		bool flag1 = false;
		bool flag2 = false;

		while (exitLoopFlag == 0) {
			_G(det)->setDetailPos(0, det0DestX, 93);
			_G(det)->setDetailPos(4, det4DestX, 57);
			
			if (delay)
				--delay;
			else {
				det0DestX -= 4;
				if (det0DestX <= 276 && flag1 == 0) {
					if (_G(gameState).flags29_20) {
						if (det0DestX > 0)
							flag1 = true;
					} else {
						flag1 = true;
						_G(det)->stopDetail(0);
						_G(det)->startDetail(1, 1, false);
					}
				}

				det4DestX -= 4;
				
				if (det4DestX <= 222 && flag2 == 0) {
					if (_G(gameState).flags29_10) {
						flag2 = true;
						_G(det)->stopDetail(4);
						if (flag1 == 0) {
							_G(det)->startDetail(5, 1, false);
						} else {
							startSetAILWait(5, 1, ANI_FRONT);
						}
					} else {
						if (det4DestX <= 0)
							flag2 = true;
					}
				}

				delay = _G(gameState).DelaySpeed / 3;
			}

			if (flag1 && flag2)
				exitLoopFlag = true;

			setupScreen(DO_SETUP);
		}

		if (_G(gameState).flags29_10 && _G(gameState).flags29_20) {
			_G(gameState).r76State = 1;
			destRoom = 77;
		} else {
			_G(gameState).r76State = 0;
			destRoom = 76;
		}		
	}

	switchRoom(destRoom);
	showCur();
}

void Room78::xit() {
	_G(gameState).ScrollxStep = 1;
	_G(gameState)._personHide[P_CHEWY] = false;
	_G(gameState)._personHide[P_HOWARD] = false;
	_G(gameState)._personHide[P_NICHELLE] = false;
}

} // namespace Rooms
} // namespace Chewy
