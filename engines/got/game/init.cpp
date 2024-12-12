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

#include "common/textconsole.h"
#include "got/game/init.h"
#include "got/gfx/image.h"
#include "got/utils/file.h"
#include "got/events.h"
#include "got/vars.h"

namespace Got {

int setup_level() {
	_G(bgPics).setArea(_G(area));

	if (_G(load_game_flag) != 1) {
		if (!load_sd_data())
			return 0;
	}

	return 1;
}

int setup_player() {
	memset(&_G(thor_info), 0, sizeof(_G(thor_info)));
	_G(thor_info).inventory = 0;
	if (_G(area) > 1) _G(thor_info).inventory |= APPLE_MAGIC + LIGHTNING_MAGIC;
	if (_G(area) > 2) _G(thor_info).inventory |= BOOTS_MAGIC + WIND_MAGIC;

	_G(thor)->health = 150;
	_G(thor_info).magic = 0;
	_G(thor_info).jewels = 0;
	_G(thor_info).score = 0;
	_G(thor_info).keys = 0;
	_G(thor_info).last_item = 0;
	_G(thor_info).object = 0;
	_G(thor_info).object_name = nullptr;
	_G(thor)->x = 152;
	_G(thor)->y = 96;
	_G(thor)->last_x[0] = _G(thor)->x;
	_G(thor)->last_x[1] = _G(thor)->x;
	_G(thor)->last_y[0] = _G(thor)->y;
	_G(thor)->last_y[1] = _G(thor)->y;
	_G(thor_info).last_icon = (6 * 20) + 8;
	_G(thor_info).last_screen = 23;
	_G(thor)->dir = 1;

	return 1;
}

void initialize() {
	load_standard_actors();
	if (!setup_player())
		error("setup_player failed");
	if (!setup_level())
		error("setup_level failed");

	// Handle loading demo key
	if (_G(rdemo)) {
		Common::File f;
		if (f.open("demo.got"))
			f.read(_G(demo_key), DEMO_LEN);
	}

	if (_G(record))
		Common::fill(_G(demo_key), _G(demo_key) + DEMO_LEN, 0);

	if (_G(demo) || _G(record)) {
		_G(thor)->health = 100;
		_G(thor_info).magic = 100;
		_G(thor_info).jewels = 463;
		_G(thor_info).score = 12455;
		_G(setup).skill = 0;
		_G(thor_info).inventory = 1 + 2;
		_G(current_level) = 54;
		_G(thor_info).item = 2;
	}

	_G(new_level) = _G(current_level);
}

} // namespace Got
