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

#include "got/game/special_tile.h"
#include "got/game/back.h"
#include "got/vars.h"

namespace Got {

int open_door1(int y, int x);
int cash_door1(int y, int x, int amount);

int special_tile_thor(int x, int y, int icon) {
	int cx, cy, f;

	f = 0;
	switch (icon) {
	case 201:
		return open_door1(x, y);
	case 202:
		if (_G(thor)->x > 300)
			_G(end_tile) = 1;     // ending bridge
		return 1;
	case 203:
		return 0;
	case 204:
		if (GAME2) {
			if (!_G(setup).f19)
				_G(slip_flag) = true;
			return 1;
		}
		return 0;
	case 205:
		if (!_G(diag) && _G(thor)->dir != 1)
			return 1;
		break;
	case 206:
		if (!_G(diag) && _G(thor)->dir != 0)
			return 1;
		break;
	case 207:
		if (!_G(diag) && _G(thor)->dir != 3)
			return 1;
		break;
	case 208:
		if (!_G(diag) && _G(thor)->dir != 2)
			return 1;
		break;
	case 209:
		return cash_door1(x, y, 10);
	case 210:
		return cash_door1(x, y, 100);
	case 211:
		place_tile(y, x, 79);
		_G(exit_flag) = 2;
		return 1;
	case 212:
	case 213:
		return 0;
	case 214:        //teleport tiles
	case 215:
	case 216:
	case 217:
		return 0;
	case 218:
	case 219:
		f = 1;
	case 220:     //hole tiles
	case 221:
	case 222:
	case 223:
	case 224:
	case 225:
	case 226:
	case 227:
	case 228:
	case 229:
		cx = (_G(thor_x1) + 7) / 16;
		cy = (_G(thor_real_y1) + 8) / 16;
		if (_G(scrn).icon[cy][cx] == icon) {
			_G(thor)->vunerable = STAMINA;
			if (icon < 224 && icon>219) play_sound(FALL, 0);
			_G(new_level) = _G(scrn).new_level[icon - 220 + (f * 6)];
			_G(warp_scroll) = 0;
			if (_G(new_level) > 119) {
				_G(warp_scroll) = 1;
				_G(new_level) -= 128;
			}

			_G(new_level_tile) = _G(scrn).new_level_loc[icon - 220 + (f * 6)];
			_G(warp_flag) = 1;

			if (_G(warp_scroll)) {
				if (_G(thor)->dir == 0) _G(thor)->y = 175;
				else if (_G(thor)->dir == 1) _G(thor)->y = 0;
				else if (_G(thor)->dir == 2) _G(thor)->x = 304;
				else if (_G(thor)->dir == 3) _G(thor)->x = 0;
			} else {
				_G(thor)->x = (_G(new_level_tile) % 20) * 16;
				_G(thor)->y = ((_G(new_level_tile) / 20) * 16) - 2;
			}
			_G(thor)->last_x[0] = _G(thor)->x;
			_G(thor)->last_x[1] = _G(thor)->x;
			_G(thor)->last_y[0] = _G(thor)->y;
			_G(thor)->last_y[1] = _G(thor)->y;
			return 0;
		}

		return 1;
	}
	return 0;
}

int special_tile(ACTOR *actr, int x, int y, int icon) {
	switch (icon) {
	case 201:
	case 202:
	case 203:
	case 204:
		break;
	case 205:
	case 206:
	case 207:
	case 208:
		return 1;
	case 209:
	case 210:
		return 0;
	case 214:
	case 215:
	case 216:
	case 217:
		return 0;
	case 224:
	case 225:
	case 226:
	case 227:
		if (!actr->flying) return 0;
		return 1;
	default:
		return 1;

	}

	return 0;
}

void erase_door(int x, int y) {
	play_sound(DOOR, 0);
	_G(scrn).icon[y][x] = _G(scrn).bg_color;

	//x = x << 4;
	//y = y << 4;
	//xfput(x, y, PAGE2, (char far *) (bg_pics + (_G(scrn).bg_color * 262)));
	//xcopyd2d(x, y, x + 16, y + 16, x, y, PAGE2, draw_page, 320, 320);
	//xcopyd2d(x, y, x + 16, y + 16, x, y, PAGE2, display_page, 320, 320);
}

int open_door1(int y, int x) {
	if (_G(thor_info).keys > 0) {
		erase_door(x, y);
		_G(thor_info).keys--;
		//display_keys();
		return 1;
	} else {
		if (!_G(door_inform)) {
			odin_speaks(2003, 0);
			_G(door_inform) = 1;
		}
	}

	return 0;
}

int cash_door1(int y, int x, int amount) {
	if (_G(thor_info).jewels >= amount) {
		erase_door(x, y);
		_G(thor_info).jewels -= amount;
		//display_jewels();
		return 1;
	} else {
		if (amount == 10 && !_G(cash1_inform)) {
			odin_speaks(2005, 0);
			_G(cash1_inform) = 1;
		}
		if (amount == 100 && !_G(cash2_inform)) {
			odin_speaks(2004, 0);
			_G(cash2_inform) = 1;
		}
	}

	return 0;
}

} // namespace Got
