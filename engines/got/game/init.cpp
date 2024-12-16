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
#include "common/textconsole.h"
#include "got/game/init.h"
#include "got/game/back.h"
#include "got/game/main.h"
#include "got/gfx/image.h"
#include "got/utils/file.h"
#include "got/events.h"
#include "got/sound.h"
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

	_G(thor)->speed_count = 6;

	_G(new_level) = _G(current_level);

	// Load level data
	Common::MemoryReadStream levelStream(
		_G(sd_data) + _G(new_level) * 512, 512);
	_G(scrn).load(&levelStream);

	show_level(_G(current_level));

	if (_G(auto_load)) {
		if (load_game(0)) {
			setup_load();
		}
		_G(auto_load) = 0;
		if (GAME1 && _G(current_area) == 59 && !_G(setup).game_over) {
			_G(auto_load) = 1;
#ifdef TODO
			fade_in();
			boss_level1();
#else
			error("TODO: boss_level1");
#endif
		}
	} else if (!_G(cheat)) {
//		key_flag[ESC] = 1;
	}

	if (!_G(auto_load)) {
//		fade_in();
		_G(sound).music_play(_G(level_type), 1);
	}
	_G(auto_load) = 0;

	_G(startup) = false;
	if (_G(record))
		memset(_G(demo_key), 0, DEMO_LEN);
}

} // namespace Got
