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
	_G(spieler).r76State = -1;
	g_engine->_sound->playSound(0, 1);
	g_engine->_sound->playSound(0, 1, false);
	hideCur();
	_G(spieler).ScrollxStep = 1;
	_G(spieler).PersonHide[P_CHEWY] = true;
	_G(spieler).PersonHide[P_HOWARD] = true;
	_G(spieler).PersonHide[P_NICHELLE] = true;
	_G(spieler).scrollx = 320;
	set_person_pos(0, 0, P_CHEWY, P_RIGHT);

	int delay = 0;
	int destRoom;

	if (_G(spieler).flags29_80) {
		_G(det)->start_detail(3, 255, false);

		int destX = 592;
		while (destX > 0) {
			_G(det)->set_detail_pos(3, destX, 77);
			if (delay)
				--delay;
			else {
				delay = _G(spieler).DelaySpeed / 2;
				destX -= 4;
			}
			set_up_screen(DO_SETUP);
		}
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_101);
		destRoom = 79;
	} else {
		_G(det)->start_detail(0, 255, false);
		_G(det)->start_detail(4, 255, false);
		int det0DestX = 608;
		int det4DestX = 570;
		bool exitLoopFlag = false;
		bool flag1 = false;
		bool flag2 = false;

		while (exitLoopFlag == 0) {
			_G(det)->set_detail_pos(0, det0DestX, 93);
			_G(det)->set_detail_pos(4, det4DestX, 57);
			
			if (delay)
				--delay;
			else {
				det0DestX -= 4;
				if (det0DestX <= 276 && flag1 == 0) {
					if (_G(spieler).flags29_20) {
						if (det0DestX > 0)
							flag1 = true;
					} else {
						flag1 = true;
						_G(det)->stop_detail(0);
						_G(det)->start_detail(1, 1, false);
					}
				}

				det4DestX -= 4;
				
				if (det4DestX <= 222 && flag2 == 0) {
					if (_G(spieler).flags29_10) {
						flag2 = true;
						_G(det)->stop_detail(4);
						if (flag1 == 0) {
							_G(det)->start_detail(5, 1, false);
						} else {
							start_detail_wait(5, 1, ANI_VOR);
						}
					} else {
						if (det4DestX <= 0)
							flag2 = true;
					}
				}

				delay = _G(spieler).DelaySpeed / 3;
			}

			if (flag1 && flag2)
				exitLoopFlag = true;

			set_up_screen(DO_SETUP);
		}

		if (_G(spieler).flags29_10 && _G(spieler).flags29_20) {
			_G(spieler).r76State = 1;
			destRoom = 77;
		} else {
			_G(spieler).r76State = 0;
			destRoom = 76;
		}		
	}

	switch_room(destRoom);
	showCur();
}

void Room78::xit() {
	_G(spieler).ScrollxStep = 1;
	_G(spieler).PersonHide[P_CHEWY] = false;
	_G(spieler).PersonHide[P_HOWARD] = false;
	_G(spieler).PersonHide[P_NICHELLE] = false;
}

} // namespace Rooms
} // namespace Chewy
