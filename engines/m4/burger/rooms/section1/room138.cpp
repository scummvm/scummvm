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

#include "m4/burger/rooms/section1/room138.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "DONUTS",          "138W002", "138W003", "138W003" },
	{ "IGNITION SWITCH", "138W006", "138W007", "138W006" },
	{ "HORN",            "138W008", "138W007", nullptr   },
	{ "RADIO",           "138W009", "138W007", nullptr   },
	{ "POLICE RADIO",    "138W010", "138W007", "138W004" },
	{ "RADAR",           "138W011", "138W007", "138W004" },
	{ "SHERIFF",         "138W012", "138W013", "138W004" },
	{ "DEPUTY",          "138W014", "138W013", "138W004" },
	{ "PATROL CAR",      "138W015", "138W013", "138W016" },
	{ "EXIT",            nullptr,   "138W017", "138W017" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{ 0,  3, 0, 1, 255, -1, 0, 0, 0, 0 },
	{ 4,  4, 0, 1, 255,  8, 0, 0, 0, 0 },
	{ 5, -1, 0, 1, 255, 10, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{  0, 11, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{ 12, 12, "138_003", 1, 255, -1, 0, 0, 0, 0 },
	{ 12, 12, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{ 12, 12, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{ 12, 12, nullptr,   1, 255,  5, 0, 0, 0, 0 },
	{ 12, 12, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{ 11,  0, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{  6, 8, nullptr,   1, 255, 12,    0, 0, 0, 0 },
	{  7, 6, "138_006", 1, 255, -1,    0, 0, 0, 0 },
	{  7, 8, nullptr,   1, 255, -1,    0, 0, 0, 0 },
	{  8, 6, nullptr,   1, 255, -1,    0, 0, 0, 0 },
	{  6, 7, nullptr,   1, 255, -1,    0, 0, 0, 0 },
	{  6, 7, nullptr,   1, 255, -1,    0, 0, 0, 0 },
	{  7, 6, nullptr,   1, 255, -1,    0, 0, 0, 0 },
	{  6, 8, nullptr,   1, 255, -1,    0, 0, 0, 0 },
	{  8, 6, nullptr,   1, 255, -1, 2048, 0, 0, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY4[] = {
	{ 25, 5, 0, 1, 255, -1, 0, 0, 0, 0 },
	{  4, 0, 0, 1, 255, 11, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

void Room138::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room138::init() {
	static const char *NAMES[12] = {
		"138_004", "137_012", "137_013", "137_020", "137_021",
		"137_022", "137_023", "137_024", "137_025", "137_026",
		"137_027", "137_028"
	};
	for (int i = 0; i < 12; ++i)
		digi_preload(NAMES[i]);

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		if (_G(flags)[V048])
			_G(flags)[V048] = 1;
		break;

	default:
		break;
	}

	if (inv_object_is_here("keys")) {
		digi_preload("138_001");
		digi_play_loop("138_001", 3, 255);

	} else {
		digi_preload("138_002");
		digi_play_loop("138_002", 3, 255);
	}

	if (inv_object_is_here("keys")) {
		hotspot_set_active("ignition switch", false);
		_series1 = series_play("138keys", 0xa00, 0, -1, 7, -1, 100, 0, 0, 0, 0);
	} else {
		hotspot_set_active("keys", false);
	}

	_val1 = 22;
	_val2 = 18;
	_val3 = 18;
	kernel_trigger_dispatch_now(1);
	_val4 = -1;
	_val5 = 26;
	_val6 = 26;
	kernel_trigger_dispatch_now(2);
	_val7 = 2;
	_val8 = 1;
	kernel_trigger_dispatch_now(3);
	_series2 = series_play("138donut", 0x400, 0, -1, 7, -1, 100, 0, 0, 0, 0);

	if (!inv_object_is_here("keys"))
		kernel_timing_trigger(10, 13);

	if (!player_been_here(138)) {
		player_set_commands_allowed(false);
		wilbur_speech("138w001");
	}

	digi_play("137_021", 2);
}

void Room138::daemon() {
}

void Room138::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Nothing needed
	} else if (player_said("keys", "ignition switch")) {
		_val8 = 16;
		player_set_commands_allowed(false);

	} else if ((player_said("donuts") || player_said("keys") ||
			player_said("ignition switch") || player_said("horn") ||
			player_said("radio") || player_said("police radio") ||
			player_said("radar") || player_said("deputy") ||
			player_said("patrol car")) && inv_player_has(_G(player).verb)) {
		wilbur_speech("138w004");

	} else if (player_said("gear", "horn")) {
		player_set_commands_allowed(false);
		_G(flags)[V047] = 1;
		_val8 = 10;

	} else if (player_said("gear", "radio")) {
		player_set_commands_allowed(false);

		if (inv_object_is_here("keys"))
			_G(flags)[V047] = 2;

		_val8 = 11;

	} else if (player_said("talk to") &&
			(player_said("sherrif") || player_said("deputy"))) {
		player_set_commands_allowed(false);
		_G(flags)[V047] = 3;
		wilbur_speech("138w610");
		_val8 = 15;

	} else if (player_said("take", "keys") && inv_player_has("keys")) {
		_val8 = 7;
		player_set_commands_allowed(false);

	} else if (player_said("look at", "keys") && inv_object_is_here("keys")) {
		wilbur_speech("138w005");

	} else if (player_said("gear", "keys")) {
		wilbur_speech("138w004");

	} else if (player_said("keys") && inv_player_has(_G(player).verb)) {
		wilbur_speech("138w004");

	} else if (player_said("exit") || player_said("look at", "exit")) {
		player_set_commands_allowed(false);
		pal_fade_init(1010);

	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
