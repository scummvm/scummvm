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

#include "m4/riddle/rooms/section2/room202.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const char *SAID[][2] = {
	{ "rope and hook", "202r10" },
	{ "lantern",       "202r11" },
	{ "shovel",        "202r12" },
	{ "bunch of keys", "202r13" },
	{ "map",           "202r14" },
	{ "axe",           "202r15" },
	{ "gong",          "202r20" },
	{ "note",          "202r24" },
	{ "wooden box",    "202r22" },
	{ nullptr, nullptr }
};


void Room202::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room202::init() {
	_G(player).disable_hyperwalk = true;
	_G(player).walker_in_this_scene = false;

	if (_G(flags)[V068] == 1 && _G(flags)[V065] <= 5) {
		_G(flags)[V068] = 0;
		_G(flags)[V065]++;
	}

	digi_preload("950_s02");
	digi_preload("950_s03");
	digi_preload("950_s04");
	player_set_commands_allowed(true);
	digi_play("950_s02", 2, 30);
	kernel_timing_trigger(1500, 105);

	if (_G(flags)[V065] >= 1)
		series_show_sprite("202 FIRST NASTY SIGN", 0, 0x400);
	if (_G(flags)[V065] >= 2)
		series_show_sprite("202 SECOND NASTY SIGN", 0, 0x400);
	if (_G(flags)[V065] >= 3)
		series_show_sprite("202 THIRD NASTY SIGN", 0, 0x400);
	if (_G(flags)[V065] >= 4)
		series_show_sprite("202 FORTH NASTY SIGN", 0, 0x400);
	if (_G(flags)[V065] >= 5)
		series_show_sprite("202 FIFTH NASTY SIGN", 0, 0x400);
}

void Room202::daemon() {
	if (_G(kernel).trigger == 105) {
		switch (imath_ranged_rand(1, 3)) {
		case 1:
			digi_play("950_s02", 2, 30);
			kernel_timing_trigger(1500, 105);
			break;
		case 2:
			digi_play("950_s03", 2, 30);
			kernel_timing_trigger(1020, 105);
			break;
		case 3:
			digi_play("950_s04", 2, 30);
			kernel_timing_trigger(540, 105);
			break;
		default:
			break;
		}
	}
}

void Room202::pre_parser() {
	_G(player).resetWalk();
}

void Room202::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (lookFlag && player_said("TRADER'S STALL") && _G(flags)[V065] == 1) {
		digi_play("202r01", 1);
	} else if (lookFlag && player_said("TRADER'S STALL") && _G(flags)[V065] == 2) {
		digi_play("202r02", 1);
	} else if (lookFlag && player_said("TRADER'S STALL") && _G(flags)[V065] == 3) {
		digi_play("202r03", 1);
	} else if (lookFlag && player_said("TRADER'S STALL") && _G(flags)[V065] == 4) {
		digi_play("202r04", 1);
	} else if (lookFlag && player_said("TRADER'S STALL") && _G(flags)[V065] == 5) {
		digi_play("202r05", 1);
	} else if (useFlag && player_said("WOODEN BOX")) {
		digi_play("202r23", 1);
	} else if (takeFlag && player_said_any("WOODEN BOX", "GONG")) {
		digi_play("202r21", 1);
	} else if (takeFlag && player_said("NOTE")) {
		digi_play("202r27", 1);
	} else if (player_said("GONG") && inv_player_has(_G(player).verb) &&
			player_said_any("SEVEN SPOKES", "SOLDIER'S HELMET", "BUCKET", "LEAD PIPE")) {
		digi_play("202_S01", 1);
	} else if (player_said("GEAR", "GONG")) {
		digi_play("202_S01", 1);
	} else if (takeFlag) {
		digi_play("202r09", 1);
	} else if (player_said("GEAR")) {
		digi_play("202r09", 1);
	} else if (player_said("exit") && _G(kernel).trigger == -1) {
		player_set_commands_allowed(false);
		disable_player_commands_and_fade_init(1);
	} else if (player_said("exit") && _G(kernel).trigger == 1) {
		digi_stop(2);
		interface_show();
		_G(game).setRoom(203);
	} else if (lookFlag && _G(walker).ripley_said(SAID)) {
		// No implementation
	} else if (player_said("journal") && !takeFlag && !lookFlag) {
		digi_play("202r16", 1);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
