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

#include "common/memstream.h"
#include "got/game/back.h"
#include "got/vars.h"
#include "got/gfx/image.h" // loadNewThor

namespace Got {

void setupLoad() {
	_G(thor)->_active = true;
	_G(newLevel) = _G(thorInfo)._lastScreen;
	_G(thor)->_x = (_G(thorInfo)._lastIcon % 20) * 16;
	_G(thor)->_y = ((_G(thorInfo)._lastIcon / 20) * 16) - 1;
	if (_G(thor)->_x < 1)
		_G(thor)->_x = 1;
	if (_G(thor)->_y < 0)
		_G(thor)->_y = 0;
	_G(thor)->_lastX[0] = _G(thor)->_x;
	_G(thor)->_lastX[1] = _G(thor)->_x;
	_G(thor)->_lastY[0] = _G(thor)->_y;
	_G(thor)->_lastY[1] = _G(thor)->_y;
	_G(thor)->_dir = _G(thorInfo)._lastDir;
	_G(thor)->_lastDir = _G(thorInfo)._lastDir;
	_G(thor)->_health = _G(thorInfo)._lastHealth;
	_G(thorInfo)._magic = _G(thorInfo)._lastMagic;
	_G(thorInfo)._jewels = _G(thorInfo)._lastJewels;
	_G(thorInfo)._keys = _G(thorInfo)._lastKeys;
	_G(thorInfo)._score = _G(thorInfo)._lastScore;
	_G(thorInfo)._selectedItem = _G(thorInfo)._lastItem;
	_G(thorInfo)._inventory = _G(thorInfo)._lastInventory;
	_G(thorInfo)._object = _G(thorInfo)._lastObject;
	_G(thorInfo)._objectName = _G(thorInfo)._lastObjectName;
	_G(thor)->_numMoves = 1;
	_G(thor)->_vulnerableCountdown = 60;
	_G(thor)->_show = 60;
	_G(appleFlag) = false;
	_G(thunderSnakeCounter) = 0;
	_G(tornadoUsed) = false;
	_G(shieldOn) = false;
	_G(actor[1])._active = false;
	_G(actor[2])._active = false;
	_G(thor)->_moveCountdown = 6;

	_G(scrn).load(_G(newLevel));

	_G(currentLevel) = _G(newLevel);
	showLevel(_G(newLevel));

	if (_G(area) == 2 && _G(currentLevel) == 105) { // Shovel Maze
		_G(thorInfo)._armor = 2; // eyeballs mode
		loadNewThor();
		_G(eyeballs) = 1;
	}
}

void pause(int delay) {
	g_system->delayMillis(delay);
}

} // namespace Got
