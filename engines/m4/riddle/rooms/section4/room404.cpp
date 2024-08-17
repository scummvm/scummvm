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

#include "m4/riddle/rooms/section4/room404.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const int16 NORMAL_DIRS[] = { 200, 201, -1 };
static const char *NORMAL_NAMES[] = {
	"butler walker pos1",
	"butler walker pos3"
};

static const int16 SHADOW_DIRS[] = { 200, 201, -1 };
static const char *SHADOW_NAMES[] = {
	"butler walker shadow pos1",
	"wolf walker shadow pos3"
};

static const char *const SAID[][2] = {
	{ "DOOR",           "404r10" },
	{ "BUTLER",         "404r30" },
	{ "HUGE DOOR",      "404r11" },
	{ "LARGE PAINTING", "404r13" },
	{ "PAINTING",       "404r14" },
	{ "RUG",            "404r12" },
	{ nullptr, nullptr }
};

void Room404::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	LoadWSAssets("OTHER SCRIPT");
}

void Room404::init() {
	_door = series_place_sprite("404DOORA", 0, 0, 0, 100, 0xf00);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		player_set_commands_allowed(false);
		_val1 = 0;
		_val2 = -1;
		_val3 = 0;
		_val4 = -1;
		_val5 = 0;
		_val6 = 0;
		_val7 = 0;
		_val8 = 0;
	}

	_safariShadow = series_load("SAFARI SHADOW 3");
	_butlerTurns7 = series_load("Butler turns to pos7");
	_butlerTurns9 = series_load("Butler turns to pos9");
	_butlerTalkLoop = series_load("Butler talk loop");
	_val7 = 2000;
	_val8 = 2100;

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		_butlerTalks = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x900, 0,
			triggerMachineByHashCallbackNegative, "BUTLER talks rip");
		sendWSMessage_10000(1, _butlerTalks, _butlerTalkLoop, 1, 1, -1,
			_butlerTalkLoop, 1, 1, 0);
		player_set_commands_allowed(true);
		break;

	case 405:
		_butlerTalks = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x900, 0,
			triggerMachineByHashCallbackNegative, "BUTLER talks rip");
		sendWSMessage_10000(1, _butlerTalks, _butlerTalkLoop, 1, 1, -1,
			_butlerTalkLoop, 1, 1, 0);
		ws_demand_location(58, 347, 3);
		ws_walk(90, 347, nullptr, 50, 3);
		break;

	case 406:
		_butlerTalks = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x900, 0,
			triggerMachineByHashCallbackNegative, "BUTLER talks rip");
		sendWSMessage_10000(1, _butlerTalks, _butlerTalkLoop, 1, 1, -1,
			_butlerTalkLoop, 1, 1, 0);
		ws_demand_location(174, 268, 3);
		ws_walk(250, 285, nullptr,
			inv_player_has("BILLIARD BALL") ? 70 : 60,
			3, 1);
		break;

	default:
		ws_walk_load_shadow_series(SHADOW_DIRS, SHADOW_NAMES);
		ws_walk_load_walker_series(NORMAL_DIRS, NORMAL_NAMES);
		ws_demand_location(340, 480, 2);

#ifdef KITTY_SCREAMING
		const char *KITTY = "SCREAMING";
#else
		const char *KITTY = nullptr;
#endif
		_machine1 = triggerMachineByHash_3000(8, 10, NORMAL_DIRS, SHADOW_DIRS,
			380, 421, 1, triggerMachineByHashCallback3000, "BUTLER_walker");

		if (strcmp(KITTY, "SCREAMING") || !player_been_here(404)) {
			sendWSMessage_10000(_machine1, 410, 332, 1, 21, 1);
			kernel_timing_trigger(120, 20);
			digi_play("404_s01", 2);
		}
		break;
	}
}

void Room404::daemon() {
	// TODO
}

void Room404::pre_parser() {
	bool takeFlag = player_said("take");
	bool lookFlag = player_said_any("look", "look at");
	bool useFlag = player_said_any("push", "pull", "gear",
		"open", "close");

	if ((player_said("SITTING ROOM") && (lookFlag || useFlag || takeFlag)) ||
			(lookFlag && player_said(" "))) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (player_said("journal") && !takeFlag && !lookFlag &&
			_G(kernel).trigger == -1) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room404::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool talkFlag = player_said_any("talk", "talk to");
	bool takeFlag = player_said("take");
	bool enterFlag = player_said("enter");
	bool useFlag = player_said_any("push", "pull", "gear",
		"open", "close");

	if (player_said("conv404a")) {
		if (_G(kernel).trigger == 1) {
			_val6 = 1103;
			_val8 = 2102;
			conv_resume();
		} else {
			conv404a();
		}
	} else if (player_said("-", " ")) {
		// No implementation
	} else if (talkFlag && player_said("butler")) {
		player_set_commands_allowed(false);
		_val4 = -1;
		_val5 = 1000;
		_val6 = 1100;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 102);
		_G(kernel).trigger_mode = KT_PARSE;
	} else if (enterFlag && player_said("BILLIARDS ROOM")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;
		case 1:
			midi_stop();
			digi_stop(3);
			_G(game).setRoom(406);
			break;
		default:
			break;
		}
	} else if (player_said("WALK TO", "OUTSIDE CASTLE")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			player_update_info();
			ws_walk(_G(player_info).x, _G(player_info).y + 50, nullptr, -1, 5);
			disable_player_commands_and_fade_init(1);
			break;
		case 1:
			midi_stop();
			digi_stop(3);
			_G(game).setRoom(402);
			break;
		default:
			break;
		}
	} else if (player_said("WALK TO", "OUTSIDE CASTLE")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_walk(58, 347, nullptr, -1, 9);
			disable_player_commands_and_fade_init(1);
			break;
		case 1:
			midi_stop();
			digi_stop(3);
			_G(game).setRoom(402);
			break;
		default:
			break;
		}
	} else if (enterFlag && player_said("SITTING ROOM")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_walk(58, 347, nullptr, -1, 9);
			disable_player_commands_and_fade_init(1);
			break;
		case 1:
			midi_stop();
			digi_stop(3);
			_G(game).setRoom(405);
			break;
		default:
			break;
		}
	} else if (player_said("GO", "STAIRS")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);

			if (!_G(flags)[V126]) {
				_G(flags)[V126] = 1;
				digi_play("404u05", 1, 255, 2);
				_val7 = 2000;
				_val8 = 2102;
				_G(kernel).trigger = KT_DAEMON;
				kernel_timing_trigger(1, 110);
				_G(kernel).trigger_mode = KT_PARSE;
			} else {
				digi_play("404r15", 1, 255, 2);
			}
			break;
		case 2:
			_val8 = 2103;
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("BILLIARDS ROOM")) {
		digi_play(player_been_here(405) ? "404r08a" : "404r08", 1);
	} else if (player_said("SITTING ROOM") && (useFlag || takeFlag) &&
			_G(kernel).trigger >= -1) {
		ws_walk(115, 350, nullptr, 2, 9);
	} else if (lookFlag && player_said("SITTING ROOM")) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(115, 350, nullptr, 2, 9);
			break;
		default:
			digi_play("404r09", 1);
			break;
		}
	} else if (lookFlag && _G(walker).ripley_said(SAID)) {
		// No implementation
	} else if (lookFlag && player_said(" ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("404r07", 1, 255, 2);
			break;
		case 2:
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("journal") && !takeFlag && !lookFlag &&
			!inv_player_has(_G(player).noun)) {
		if (_G(flags)[kCastleCartoon]) {
			digi_play("com016", 1);
		} else if (_G(kernel).trigger == 6) {
			_G(flags)[kCastleCartoon] = 1;
			sendWSMessage_multi("com015");
		} else {
			sendWSMessage_multi("");
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room404::conv404a() {
	int who = conv_whos_talking();
	_currentNode = conv_current_node();
	const char *sound = conv_sound_to_play();

	if (sound) {
		if (who <= 0)
			_val8 = 2101;
		else if (who == 1)
			_val6 = 1102;

		digi_play(sound, 1, 255, 1);

	} else {
		conv_resume();
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
