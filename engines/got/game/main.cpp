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

void setup_load() {
	_G(thor)->_active = 1;
	_G(new_level) = _G(thor_info).last_screen;
	_G(thor)->_x = (_G(thor_info).last_icon % 20) * 16;
	_G(thor)->_y = ((_G(thor_info).last_icon / 20) * 16) - 1;
	if (_G(thor)->_x < 1)
		_G(thor)->_x = 1;
	if (_G(thor)->_y < 0)
		_G(thor)->_y = 0;
	_G(thor)->_lastX[0] = _G(thor)->_x;
	_G(thor)->_lastX[1] = _G(thor)->_x;
	_G(thor)->_lastY[0] = _G(thor)->_y;
	_G(thor)->_lastY[1] = _G(thor)->_y;
	_G(thor)->_dir = _G(thor_info).last_dir;
	_G(thor)->_lastDir = _G(thor_info).last_dir;
	_G(thor)->_health = _G(thor_info).last_health;
	_G(thor_info).magic = _G(thor_info).last_magic;
	_G(thor_info).jewels = _G(thor_info).last_jewels;
	_G(thor_info).keys = _G(thor_info).last_keys;
	_G(thor_info).score = _G(thor_info).last_score;
	_G(thor_info).item = _G(thor_info).last_item;
	_G(thor_info).inventory = _G(thor_info).last_inventory;
	_G(thor_info).object = _G(thor_info).last_object;
	_G(thor_info).object_name = _G(thor_info).last_object_name;
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
	_G(actor[1])._active = 0;
	_G(actor[2])._active = 0;
	_G(thor)->_moveCountdown = 6;

	_G(scrn).load(_G(new_level));

	_G(current_level) = _G(new_level);
	show_level(_G(new_level));
}

void pause(int delay) {
	g_system->delayMillis(delay);
}

} // namespace Got
