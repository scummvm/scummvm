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

namespace Got {

void setupLoad() {
	_G(thor)->_active = true;
	_G(new_level) = _G(thorInfo)._lastScreen;
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
	_G(apple_flag) = false;
	_G(bomb_flag) = false;
	_G(thunder_flag) = 0;
	_G(lightning_used) = false;
	_G(tornado_used) = false;
	_G(shield_on) = false;
	_G(actor[1])._active = false;
	_G(actor[2])._active = false;
	_G(thor)->_moveCountdown = 6;

	_G(scrn).load(_G(new_level));

	_G(current_level) = _G(new_level);
	showLevel(_G(new_level));
}

void pause(int delay) {
	g_system->delayMillis(delay);
}

} // namespace Got
