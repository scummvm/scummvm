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
	_G(new_level) = _G(thor_info)._lastScreen;
	_G(thor)->_x = (_G(thor_info)._lastIcon % 20) * 16;
	_G(thor)->_y = ((_G(thor_info)._lastIcon / 20) * 16) - 1;
	if (_G(thor)->_x < 1)
		_G(thor)->_x = 1;
	if (_G(thor)->_y < 0)
		_G(thor)->_y = 0;
	_G(thor)->_lastX[0] = _G(thor)->_x;
	_G(thor)->_lastX[1] = _G(thor)->_x;
	_G(thor)->_lastY[0] = _G(thor)->_y;
	_G(thor)->_lastY[1] = _G(thor)->_y;
	_G(thor)->_dir = _G(thor_info)._lastDir;
	_G(thor)->_lastDir = _G(thor_info)._lastDir;
	_G(thor)->_health = _G(thor_info)._lastHealth;
	_G(thor_info)._magic = _G(thor_info)._lastMagic;
	_G(thor_info)._jewels = _G(thor_info)._lastJewels;
	_G(thor_info)._keys = _G(thor_info)._lastKeys;
	_G(thor_info)._score = _G(thor_info)._lastScore;
	_G(thor_info)._selectedItem = _G(thor_info)._lastItem;
	_G(thor_info)._inventory = _G(thor_info)._lastInventory;
	_G(thor_info)._object = _G(thor_info)._lastObject;
	_G(thor_info)._objectName = _G(thor_info)._lastObjectName;
	_G(thor)->_numMoves = 1;
	_G(thor)->_vulnerableCountdown = 60;
	_G(thor)->_show = 60;
	_G(hourglass_flag) = 0;
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
