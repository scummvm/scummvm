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
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/burger/rooms/section9/room902.h"
#include "m4/graphics/gr_series.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const Entry ENTRIES1[] = {
	{ "902bk", 168 },
	{ "902bi", 168 },
	{ "902bj", 168 },
	{ "902bb", 168 },
	{ "902bs", 168 },
	{ "902bg", 168 },
	{ "902bc", 168 },
	{ "902bl", 168 },
	{ "902ba", 168 },
	{ "902bw", 168 },
	{ "902bq", 168 },
	{ "902bn", 168 },
	{ nullptr, 0 }
};

static const Entry ENTRIES2[] = {
	{ "902wa", 300 },
	{ nullptr, 0 }
};

void Room902::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room902::init() {
	_vol = 255;
	digi_preload("902music");
	digi_play("902music", 1, _vol, -1);
	mouse_hide();

	if (_G(room902Flag) <= 0) {
		_entries = ENTRIES1;
		series_load("902ob", -1, _G(master_palette));
		kernel_timing_trigger(1, 3);
	} else if (_G(room902Flag) == 1) {
		_entries = ENTRIES2;
	}

	_index = 0;
	_name = _entries[_index]._name;
	_duration = _entries[_index]._duration;

	_G(kernel).suppress_fadeup = true;
	pal_fade_set_start(_G(master_palette), 0);
	pal_fade_init(_G(master_palette), 0, 255, 100, 60, 1);
}

void Room902::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		pal_fade_set_start(_G(master_palette), 0);

		if (_name) {
			if (_index <= 0)
				series_unload(_seriesIndex);

			_seriesIndex = series_load(_name, -1, _G(master_palette));
			series_show(_name, 1, 64, 1, _duration + 60, 0, 100, 0, 0);
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 100, 30, 5);
			kernel_timing_trigger(_duration + 30, 2);

			++_index;
			_name = _entries[_index]._name;
			_duration = _entries[_index]._duration;
		}
		break;

	case 2:
		if (_name) {
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, -1);
		} else {
			pal_fade_init(_G(master_palette), 0, 255, 0, 30, -1);
			kernel_trigger_dispatch_now(4);
		}
		break;

	case 3:
		series_show("902ob", 2, 64, -1, -1, 0, 100, 320, 430);
		break;

	case 4:
		_vol -= 20;
		if (_vol > 0) {
			digi_change_volume(1, _vol);
			kernel_timing_trigger(6, 4);
		} else if (_G(executing) == WHOLE_GAME) {
			_G(game).setRoom(903);
		} else {
			_G(game).setRoom(901);
		}
		break;

	case 5:
		if (_entries[_index]._name)
			series_load(_entries[_index]._name, -1, nullptr);
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
