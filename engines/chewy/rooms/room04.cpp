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

	// This can't be static because it makes them global objects
	const Common::Rect CONSOLE_HOTSPOTS[] = {
		{  80, 140, 120, 175 },
		{ 140, 140, 170, 175 },
		{ 195, 140, 235, 175 }
	};

	cur_2_inventory();
	_G(gameState)._personHide[P_CHEWY] = true;
	switchRoom(4);
	bool endLoop = false;

	// TODO: The original limited the cursor height to 16 pixels
	//WRITE_LE_INT16(_G(room_blk)._detImage[HAND_NORMAL] + 2, 16);
	//WRITE_LE_INT16(_G(room_blk)._detImage[HAND_CLICK] + 2, 16);

	_G(cur)->setCustomRoomCursor(_G(room_blk)._detImage[HAND_NORMAL]);

	byte curCursor = HAND_NORMAL;
	int16 curX = 1;
	int16 sprNr = RED_FRAME;
	_G(cur)->move(160, 160);

	// Clear any pending keys
	g_events->_kbInfo._keyCode = '\0';
	g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
	_G(minfo).button = 0;
	_G(mouseLeftClick) = false;

	start_aad(46);

	while (!endLoop) {
		// WORKAROUND: The original constrained the mouse area.
		// We don't do that in ScummVM so the below prevents
		// potential crashes caused by the hand sprites being
		// unloaded if the cursor is moved up too high
		if (g_events->_mousePos.y < 135)
			g_events->_mousePos.y = 135;
		mouseAction();

		if (_G(mouseLeftClick)) {
			switch (_G(out)->findHotspot(CONSOLE_HOTSPOTS)) {
			case 0:
				if (curX > 0)
					--curX;
				else
					curX = 2;
				_G(det)->playSound(0, 1);
				break;

			case 1:
				endLoop = true;
				sprNr = YELLOW_FRAME;
				_G(det)->playSound(0, 0);
				break;

			case 2:
				if (curX < 2)
					++curX;
				else
					curX = 0;
				_G(det)->playSound(0, 2);
				break;

			default:
				break;
			}
		}

		_G(spr_info)[0]._image = _G(room_blk)._detImage[sprNr];
		_G(spr_info)[0]._zLevel = 0;
		_G(spr_info)[0]._x = CUR_POS[curX][0];
		_G(spr_info)[0]._y = CUR_POS[curX][1];

		if (_G(minfo).button == 1 || g_events->_kbInfo._keyCode == Common::KEYCODE_RETURN) {
			if (curCursor != HAND_CLICK) {
				_G(cur)->setCustomRoomCursor(_G(room_blk)._detImage[HAND_CLICK]);
				_G(cur)->setAnimation(HAND_CLICK, HAND_CLICK, -1);
				curCursor = HAND_CLICK;
			}
		} else {
			if (curCursor != HAND_NORMAL) {
				_G(cur)->setCustomRoomCursor(_G(room_blk)._detImage[HAND_NORMAL]);
				_G(cur)->setAnimation(HAND_NORMAL, HAND_NORMAL, -1);
				curCursor = HAND_NORMAL;
			}
		}
		cursorChoice(CUR_USER);

		if (g_events->_mousePos.y < 124)
			g_events->_mousePos.y = 123;

		setupScreen(DO_SETUP);
		SHOULD_QUIT_RETURN0;
	}
	g_events->delay(500);

	clear_prog_ani();
	_G(gameState)._personHide[P_CHEWY] = false;
	_G(menu_item) = CUR_WALK;
	cursorChoice(_G(menu_item));

	_G(moveState)[P_CHEWY]._delayCount = 0;
	_G(mouseLeftClick) = false;
	_G(minfo).button = 0;
	_G(gameState)._personRoomNr[P_CHEWY] = 3;
	_G(room)->loadRoom(&_G(room_blk), 3, &_G(gameState));
	setPersonPos(110, 139, P_CHEWY, P_LEFT);

	_G(fx_blend) = BLEND1;
	_G(atds)->stopAad();

	return curX;
}

} // namespace Rooms
} // namespace Chewy
