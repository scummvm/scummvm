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
#include "chewy/rooms/room04.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

#define HAND_NORMAL 68
#define HAND_CLICK 69
#define RED_FRAME 70
#define YELLOW_FRAME 71

int16 Room4::comp_probe() {
	static const int16 CUR_POS[3][2] = {
		{  83, 106 },
		{ 136, 103 },
		{ 188, 101 }
	};
	static const int16 CONSOLE[3][4] = {
		{  82, 158, 143, 199 },
		{ 150, 159, 194, 193 },
		{ 201, 154, 262, 193 }
	};

	cur_2_inventory();
	_G(spieler)._personHide[P_CHEWY] = true;
	_G(cur_display) = false;
	switchRoom(4);
	_G(cur_display) = true;
	bool endLoop = false;
	_G(curblk).sprite = _G(room_blk)._detImage;
	int16 curX = 1;
	int16 sprNr = RED_FRAME;
	_G(cur)->move(160, 160);

	start_aad(46);
	while (!endLoop) {
		mouseAction();
		if (_G(maus_links_click)) {
			switch (_G(in)->mouseVector(_G(minfo).x + 17, _G(minfo).y + 7, &CONSOLE[0][0], 3)) {
			case 0:
				if (curX > 0)
					--curX;
				else
					curX = 2;
				g_engine->_sound->playSound(0, 1);
				break;

			case 1:
				endLoop = true;
				sprNr = YELLOW_FRAME;
				g_engine->_sound->playSound(0, 0);
				break;

			case 2:
				if (curX < 2)
					++curX;
				else
					curX = 0;
				g_engine->_sound->playSound(0, 2);
				break;

			default:
				break;
			}
		}

		_G(spr_info)[0]._image = _G(room_blk)._detImage[sprNr];
		_G(spr_info)[0]._zLevel = 0;
		_G(spr_info)[0]._x = CUR_POS[curX][0];
		_G(spr_info)[0]._y = CUR_POS[curX][1];

		if (_G(minfo)._button == 1 || _G(kbinfo)._keyCode == Common::KEYCODE_RETURN) {
			_G(curani)._start = HAND_CLICK;
			_G(curani)._end = HAND_CLICK;
		} else {
			_G(curani)._start = HAND_NORMAL;
			_G(curani)._end = HAND_NORMAL;
		}
		cursorChoice(CUR_USER);
		_G(spieler)._curHeight = 16;

		if (_G(minfo).y < 124)
			_G(minfo).y = 123;

		setupScreen(DO_SETUP);
		SHOULD_QUIT_RETURN0;
	}
	g_events->delay(500);

	clear_prog_ani();
	_G(spieler)._personHide[P_CHEWY] = false;
	_G(menu_item) = CUR_WALK;
	cursorChoice(_G(menu_item));

	_G(spieler_vector)[P_CHEWY]._delayCount = 0;
	_G(maus_links_click) = false;
	_G(minfo)._button = 0;
	_G(spieler)._personRoomNr[P_CHEWY] = 3;
	_G(room)->loadRoom(&_G(room_blk), 3, &_G(spieler));
	setPersonPos(110, 139, P_CHEWY, P_LEFT);

	_G(fx_blend) = BLEND1;
	_G(atds)->stopAad();

	return curX;
}

} // namespace Rooms
} // namespace Chewy
