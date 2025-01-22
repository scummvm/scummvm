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
#include "got/events.h"
#include "got/game/back.h"
#include "got/game/object.h"
#include "got/gfx/image.h"
#include "got/vars.h"

namespace Got {

int openDoor1(int y, int x);
int cashDoor1(int y, int x, int amount);
void eraseDoor(int x, int y);

int specialTileThor(const int x, const int y, const int icon) {
	int cx, cy;

	int f = 0;
	switch (icon) {
	case 201:
		return openDoor1(x, y);
	case 202:
		if (GAME3) {
			if (_G(thor_info)._inventory & 64) {
				if (_G(thor_info)._object == 4) {
					eraseDoor(y, x);
					deleteObject();
					return 1;
				}
			}
			return 0;
		}

		if (_G(thor)->_x > 300)
			// Ending bridge
			_G(end_tile) = true;
		return 1;
	case 203:
		if (!GAME1) {
			if ((_G(thor_info)._inventory & 64) && _G(thor_info)._object == 5) {
				odinSpeaks(2012, 0);
				deleteObject();
				_G(setup).f10 = true;
			} else if (!_G(setup).f10) {
				odinSpeaks(2011, 0);
				_G(setup).f10 = true;
			}
			return 1;
		}
		return 0;
	case 204:
		if (GAME2) {
			if (!_G(setup).f19)
				_G(slipFlag) = true;
			return 1;
		}

		if (GAME3) {
			if (_G(thor)->_x < 4)
				_G(end_tile) = true;
			return 1;
		}
		return 0;
	case 205:
		if (!_G(diag) && _G(thor)->_dir != 1)
			return 1;
		break;
	case 206:
		if (!_G(diag) && _G(thor)->_dir != 0)
			return 1;
		break;
	case 207:
		if (!_G(diag) && _G(thor)->_dir != 3)
			return 1;
		break;
	case 208:
		if (!_G(diag) && _G(thor)->_dir != 2)
			return 1;
		break;
	case 209:
		return cashDoor1(x, y, 10);
	case 210:
		return cashDoor1(x, y, 100);
	case 211:
		if (GAME1) {
			placeTile(y, x, 79);
			_G(exit_flag) = 2;
		} else if (GAME2) {
			if (_G(thor)->_dir == 0 && _G(setup).f29 && _G(setup).f21 && !_G(setup).f22) {
				actorVisible(1);
				actorVisible(2);
				actorVisible(3);
				actorVisible(4);
				actorVisible(5);
				Common::fill(_G(scrn)._actorInvis, _G(scrn)._actorInvis + 16, 0);
				_G(thunder_flag) = 60;
				playSound(THUNDER, true);
				_G(setup).f22 = true;
			}
		} else {
			// Game 3
			return 0;
		}
		return 1;
	case 212:
	case 213:
		return 0;
	case 214: // Teleport tiles
	case 215:
	case 216:
	case 217:
		if ((GAME2 && icon == 217) || GAME3) {
			cx = (_G(thor_x1) + 7) / 16;
			cy = (_G(thor_real_y1) + 8) / 16;
			if (_G(scrn)._iconGrid[cy][cx] == icon) {
				_G(thor)->_vulnerableCountdown = STAMINA;
				playSound(WOOP, false);

				const int nt = _G(scrn)._newLevelLocation[icon - 214];
				const int displayPage = _G(pge);
				const int drawPage = _G(pge) ^ 1;

				_G(thor)->_lastX[displayPage] = _G(thor)->_x;
				_G(thor)->_lastY[displayPage] = _G(thor)->_y;
				_G(thor)->_x = (nt % 20) * 16;
				_G(thor)->_y = ((nt / 20) * 16) - 2;
				_G(thor)->_lastX[drawPage] = _G(thor)->_x;
				_G(thor)->_lastY[drawPage] = _G(thor)->_y;
				return 0;
			}
			return 1;
		}
		return 0;
	case 218:
	case 219:
		// Hole tiles
		f = 1;
		// fall through
	case 220:
	case 221:
	case 222:
	case 223:
	case 224:
	case 225:
	case 226:
	case 227:
	case 228:
	case 229:
		// Hole tiles
		cx = (_G(thor_x1) + 7) / 16;
		cy = (_G(thor_real_y1) + 8) / 16;
		if (_G(scrn)._iconGrid[cy][cx] == icon) {
			_G(thor)->_vulnerableCountdown = STAMINA;
			if (icon < 224 && icon > 219)
				playSound(FALL, false);

			_G(new_level) = _G(scrn)._newLevel[icon - 220 + (f * 6)];
			_G(warp_scroll) = false;
			if (_G(new_level) > 119) {
				_G(warp_scroll) = true;
				_G(new_level) -= 128;
			}

			_G(new_level_tile) = _G(scrn)._newLevelLocation[icon - 220 + (f * 6)];
			_G(warp_flag) = true;

			if (_G(warp_scroll)) {
				if (_G(thor)->_dir == 0)
					_G(thor)->_y = 175;
				else if (_G(thor)->_dir == 1)
					_G(thor)->_y = 0;
				else if (_G(thor)->_dir == 2)
					_G(thor)->_x = 304;
				else if (_G(thor)->_dir == 3)
					_G(thor)->_x = 0;
			} else {
				_G(thor)->_x = (_G(new_level_tile) % 20) * 16;
				_G(thor)->_y = ((_G(new_level_tile) / 20) * 16) - 2;
			}
			_G(thor)->_lastX[0] = _G(thor)->_x;
			_G(thor)->_lastX[1] = _G(thor)->_x;
			_G(thor)->_lastY[0] = _G(thor)->_y;
			_G(thor)->_lastY[1] = _G(thor)->_y;
			return 0;
		}

		return 1;

	default:
		break;
	}
	return 0;
}

int specialTile(const Actor *actor, int x, int y, const int icon) {
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
	case 214:
	case 215:
	case 216:
	case 217:
		return 0;
	case 224:
	case 225:
	case 226:
	case 227:
		if (!actor->_flying)
			return 0;
		return 1;
	default:
		return 1;
	}

	return 0;
}

void eraseDoor(const int x, const int y) {
	playSound(DOOR, false);
	_G(scrn)._iconGrid[y][x] = _G(scrn)._backgroundColor;
}

int openDoor1(const int y, const int x) {
	if (_G(thor_info)._keys > 0) {
		eraseDoor(x, y);
		_G(thor_info)._keys--;

		return 1;
	}

	if (!_G(door_inform)) {
		odinSpeaks(2003, 0);
		_G(door_inform) = true;
	}

	return 0;
}

int cashDoor1(const int y, const int x, const int amount) {
	if (_G(thor_info)._jewels >= amount) {
		eraseDoor(x, y);
		_G(thor_info)._jewels -= amount;

		return 1;
	}

	if (amount == 10 && !_G(cash1_inform)) {
		odinSpeaks(2005, 0);
		_G(cash1_inform) = true;
	}

	if (amount == 100 && !_G(cash2_inform)) {
		odinSpeaks(2004, 0);
		_G(cash2_inform) = true;
	}

	return 0;
}

} // namespace Got
