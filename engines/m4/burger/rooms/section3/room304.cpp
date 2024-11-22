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

#include "m4/burger/rooms/section3/room304.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {


const char *Room304::SAID[][4] = {
	{ "DOOR",        "304w002", "300w002", nullptr   },
	{ "BED",         "304w007", "300w003", "304w008" },
	{ "PICTURE",     "304w009", "304w010", "300w005" },
	{ "MATCHES ",    "304w003", nullptr,   "304w004" },
	{ "MAP",         nullptr,   "304w013", "304w014" },
	{ "CRATE",       "304w015", "300w004", "300w004" },
	{ "STOVE",       "304w016", "304w017", "304w018" },
	{ "KETTLE",      "304w019", "304w018", "304w018" },
	{ "MUG",         "304w019", nullptr,   nullptr   },
	{ "RECIPE BOOK", nullptr,   "304w021", "304w022" },
	{ "WASHTUB",     "304w023", "304w024", "304w024" },
	{ "POT",         "304w025", "304w018", "304w018" },
	{ "FRYING PAN",  "304w026", "304w027", "304w018" },
	{ "WINDOW",      "304w028", "300w002", "304w029" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room304::PLAY1[] = {
	{  0,  3, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{  4, 16, "304_001", 2, 255, -1, 2048, 0, nullptr, 0 },
	{ 17, -1, "304_002", 2, 255, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room304::PLAY2[] = {
	{  0,  3, nullptr,   1,   0, -1, 2050,  0, nullptr, 0 },
	{  0,  0, "304w020", 1, 255, -1,    0, 10, nullptr, 0 },
	{  0,  6, nullptr,   0,   0, -1,    0,  0, nullptr, 0 },
	{  7,  7, nullptr,   0,   0, -1,    0,  5, nullptr, 0 },
	{  8, 12, nullptr,   0,   0, -1,    0,  0, nullptr, 0 },
	{ 13, -1, "304_004", 2, 255, -1,    0,  0, nullptr, 0 },
	{  7,  7, nullptr,   0,   0, -1,    0,  3, nullptr, 0 },
	{  8, 12, nullptr,   0,   0, -1,    0,  0, nullptr, 0 },
	{ 13, -1, "304_004", 2, 255, -1,    0,  0, nullptr, 0 },
	{  7,  7, nullptr,   0,   0, -1,    0,  5, nullptr, 0 },
	{  7,  3, nullptr,   0,   0, -1,    0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room304::PLAY3[] = {
	{ 0,  3, nullptr, 1, 0, -1, 0, 0, nullptr, 0 },
	{ 4,  5, nullptr, 0, 0,  6, 0, 0, nullptr, 0 },
	{ 6, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room304::PLAY4[] = {
	{  0, 24, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 25, 36, "300_003", 2, 255, -1, 2048, 0, nullptr, 0 },
	{ 37, -1, nullptr,   0,   0,  3,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room304::PLAY5[] = {
	{ 17, 17, nullptr,   0,   0, -1,    0, 5, nullptr, 0 },
	{ 17, 21, "304_002", 2, 255, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const char *Room304::getDigi() {
	return _G(flags)[V130] ? "304_003" : "300_005";
}

void Room304::init() {
	setupDigi();
	set_palette_brightness(70);

	if (inv_player_has("MATCHES")) {
		hotspot_set_active("MATCHES ", false);
	} else {
		_matches = series_show("304match", 0xa00);
		hotspot_set_active("MATCHES ", true);
	}

	if (_G(flags)[V130]) {
		_fire = series_play("304firel", 0xa00, 4, -1, 6, -1);
	} else {
		_fire = series_show("304firel", 0xa00);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 302:
		ws_demand_location(150, 306, 3);
		kernel_trigger_dispatch_now(4);
		break;

	default:
		player_set_commands_allowed(true);
		ws_demand_location(300, 306, 4);
		break;
	}
}

void Room304::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		term_message("Walking into scene....!");

		if (_G(flags)[V129]) {
			ws_walk(300, 306, nullptr, 2, 4);
		} else {
			_G(flags)[V129] = 1;
			player_set_commands_allowed(false);
			_G(wilbur_should) = 5;
			ws_walk(300, 306, nullptr, kCHANGE_WILBUR_ANIMATION, 4);
		}
		break;

	case 2:
		player_set_commands_allowed(true);
		break;

	case 3:
		setupDigi();
		break;

	case 4:
		player_set_commands_allowed(false);
		series_play_with_breaks(PLAY5, "304wi01", 0x600, -1, 2, 6);
		kernel_trigger_dispatch_now(1);
		break;

	case 5:
		_fire = series_play("304fire1", 0xa00, 0, -1, 6, -1);
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 6:
		inv_give_to_player("MATCHES");
		hotspot_set_active("MATCHES ", false);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 10002;
			series_play_with_breaks(PLAY1, "304wi01", 0x600, 3001, 3, 8);
			break;

		case 2:
			player_set_commands_allowed(false);
			ws_hide_walker();
			terminateMachineAndNull(_fire);
			series_play_with_breaks(PLAY4, "304wi05", 0xa00, 5, 3);
			break;

		case 3:
			player_set_commands_allowed(false);
			ws_hide_walker();
			terminateMachineAndNull(_matches);
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY3, "304wb01", 0xa00, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 4:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY2, "304wi03", 0x101, kCHANGE_WILBUR_ANIMATION, 2);
			break;

		case 5:
			_G(wilbur_should) = 10001;
			_G(flags)[V129] = 1;
			wilbur_speech("304w001");
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room304::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;
}

void Room304::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("OPEN", "DOOR") || player_said("GEAR", "DOOR")) {
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("LOOK AT", "MAP")) {
		wilbur_speech(_G(flags)[V135] ? "304w012" : "304w011");
	} else if (player_said("DISTILLED CARROT JUICE") &&
			player_said_any("STOVE", "KETTLE", "FRYING PAN", "POT")) {
		wilbur_speech("300w035");
	} else if (player_said("CARROT JUICE") &&
			player_said_any("STOVE", "KETTLE", "FRYING PAN", "POT")) {
		wilbur_speech("300w051");
	} else if (player_said("CARROT JUICE", "WASHTUB")) {
		wilbur_speech("300w052");
	} else if (player_said("DISTILLED CARROT JUICE", "WASHTUB")) {
		wilbur_speech("300w035z");
	} else if (player_said("MATCHES", "STOVE") && !_G(flags)[V130]) {
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("TAKE", "MATCHES ")) {
		_G(wilbur_should) = 3;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("READ", "RECIPE BOOK") || player_said("LOOK AT", "RECIPE BOOK")) {
		_G(wilbur_should) = 4;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
