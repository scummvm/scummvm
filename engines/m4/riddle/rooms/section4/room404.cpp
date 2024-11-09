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
#include "m4/riddle/riddle.h"

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
			triggerMachineByHashCallback, "BUTLER talks rip");
		sendWSMessage_10000(1, _butlerTalks, _butlerTalkLoop, 1, 1, -1,
			_butlerTalkLoop, 1, 1, 0);
		player_set_commands_allowed(true);
		break;

	case 405:
		_butlerTalks = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x900, 0,
			triggerMachineByHashCallback, "BUTLER talks rip");
		sendWSMessage_10000(1, _butlerTalks, _butlerTalkLoop, 1, 1, -1,
			_butlerTalkLoop, 1, 1, 0);
		ws_demand_location(58, 347, 3);
		ws_walk(90, 347, nullptr, 50, 3);
		break;

	case 406:
		_butlerTalks = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x900, 0,
			triggerMachineByHashCallback, "BUTLER talks rip");
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

		_machine1 = triggerMachineByHash_3000(8, 10, *NORMAL_DIRS, *SHADOW_DIRS,
			380, 421, 1, triggerMachineByHashCallback3000, "BUTLER_walker");

		if (!_G(kittyScreaming) || !player_been_here(404)) {
			sendWSMessage_10000(_machine1, 410, 332, 1, 21, 1);
			kernel_timing_trigger(120, 20);
			digi_play("404_s01", 2);
		}
		break;
	}
}

void Room404::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case 20:
		ws_walk(370, 347, nullptr, -1, 1);
		break;

	case 21:
		sendWSMessage_60000(_machine1);
		_butlerTalks = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x900, 0,
			triggerMachineByHashCallback, "BUTLER talks rip");
		sendWSMessage_10000(1, _butlerTalks, _butlerTurns7, 1, 10, 23,
			_butlerTalkLoop, 1, 1, 0);
		break;

	case 23:
		if (!_G(flags)[V337]) {
			digi_play("404r03", 1, 255, 24);
			_G(flags)[V337] = 1;
			_val7 = 2000;
			_val8 = 2102;
			kernel_timing_trigger(1, 110);
		} else {
			digi_play("404r02", 1, 255, 30);
		}
		break;

	case 24:
		_val8 = 2102;
		digi_play("404u01", 1, 255, 25);
		break;

	case 25:
		_val8 = 2102;
		digi_play("404r04", 1, 255, 26);
		break;

	case 26:
		_val8 = 2101;
		digi_play("404u02", 1, 255, 27);
		break;

	case 27:
		_val8 = 2102;
		digi_play("404r05", 1, 255, 28);
		break;

	case 28:
		_val8 = 2101;
		digi_play("404u03", 1, 255, 29);
		break;

	case 29:
		_val8 = 2103;
		digi_play("404r06", 1, 255, 30);
		break;

	case 30:
		digi_play("404u04", 1);
		sendWSMessage_10000(1, _butlerTalks, _butlerTurns9, 1, 43, 32,
			_butlerTurns9, 43, 43, 0);
		break;

	case 32:
		terminateMachineAndNull(_butlerTalks);
		_machine1 = triggerMachineByHash_3000(8, 10, *NORMAL_DIRS, *SHADOW_DIRS,
			390, 332, 9, triggerMachineByHashCallback3000, "BUTLER_walker");
		kernel_timing_trigger(270, 33);
		break;

	case 33:
		ws_walk(58, 347, nullptr, -1, 9);
		kernel_timing_trigger(90, 34);
		break;

	case 34:
		disable_player_commands_and_fade_init(35);
		break;

	case 35:
		midi_stop();
		digi_stop(3);
		_G(game).setRoom(405);
		break;

	case 40:
		ws_walk(370, 347, nullptr, -1, 1);
		break;

	case 42:
		sendWSMessage_60000(_machine1);
		_butlerTalks = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x900, 0,
			triggerMachineByHashCallback, "BUTLER talks rip");
		sendWSMessage_10000(1, _butlerTalks, _butlerTurns7, 1, 10, 43,
			_butlerTalkLoop, 1, 1, 0);
		break;

	case 43:
		if (!_G(flags)[GLB_TEMP_12] || _G(flags)[V334]) {
			player_set_commands_allowed(true);
		} else {
			_G(flags)[V334] = 1;
			kernel_timing_trigger(1, 44);
		}
		break;

	case 44:
		digi_play("404r20", 1, 255, 45);
		_val7 = 2000;
		_val8 = 2102;
		kernel_timing_trigger(1, 110);
		break;

	case 45:
		_val8 = 2101;
		digi_play("404u10", 1, 255, 46);
		break;

	case 46:
		_val8 = 2102;
		digi_play("404r21", 1, 255, 47);
		break;

	case 47:
		_val8 = 2101;
		digi_play("404u11", 1, 255, 48);
		break;

	case 48:
		_val8 = 2102;
		digi_play("404r22", 1, 255, 49);
		ws_walk(368, 349, nullptr, -1, 8);
		break;

	case 49:
	case 50:
	case 60:
		player_set_commands_allowed(true);
		break;

	case 70:
		if (++_G(flags)[V128] == 1)
			digi_play("404u12", 1, 255, 72);
		else
			digi_play("404u13", 1, 255, 72);

		_val8 = 2101;
		break;

	case 72:
		_val8 = 2102;
		ws_walk(174, 268, nullptr, 73, 9);

		if (_G(flags)[V128] == 1)
			digi_play("404r23", 1);
		break;

	case 73:
		disable_player_commands_and_fade_init(74);
		break;

	case 74:
		midi_stop();
		digi_stop(3);
		_G(game).setRoom(406);
		break;

	case 100:
		kernel_timing_trigger(1, 102);
		break;

	case 101:
		_val5 = 1000;
		_val6 = 1105;
		break;

	case 102:
		if (_val2 != -1) {
			kernel_timing_trigger(1, _val2);
			_val2 = -1;
		} else {
			kernel_timing_trigger(1, 103);
		}
		break;

	case 103:
		if (_val5 == 1000) {
			switch (_val6) {
			case 1100:
				player_set_commands_allowed(false);
				_val7 = 2000;
				_val8 = 2100;
				kernel_timing_trigger(1, 110);
				_val6 = 1103;
				kernel_timing_trigger(1, 102);

				conv_load("conv404a", 10, 10, 101);
				conv_export_pointer_curr(&_G(flags)[V135], 0);
				conv_export_value_curr(player_been_here(407) ? 1 : 0, 1);
				conv_play();
				break;

			case 1102:
			case 1103:
				kernel_timing_trigger(10, 102);
				break;

			case 1105:
				_val8 = 2103;
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
		}
		break;

	case 110:
		if (_val7 == 2000) {
			switch (_val8) {
			case 2100:
			case 2101:
			case 2102:
			case 2103:
				if (_val4 != -1) {
					kernel_timing_trigger(1, _val4);
					_val4 = -1;
				} else {
					kernel_timing_trigger(1, 111);
				}
				break;
			default:
				break;
			}
		}
		break;

	case 111:
		if (_val7 == 2000) {
			switch (_val8) {
			case 2100:
			case 2102:
				sendWSMessage_10000(1, _butlerTalks, _butlerTalkLoop, 1, 1, 110,
					_butlerTalkLoop, 1, 1, 0);
				break;

			case 2101:
				frame = imath_ranged_rand(12, 21);
				sendWSMessage_10000(1, _butlerTalks, _butlerTalkLoop, frame, frame, 110,
					_butlerTalkLoop, frame, frame, 0);
				break;

			case 2103:
				sendWSMessage_10000(1, _butlerTalks, _butlerTalkLoop, 1, 1, -1,
					_butlerTalkLoop, 1, 1, 0);
				break;

			default:
				break;
			}
		}
		break;

	default:
		break;
	}
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
			sketchInJournal("com015");
		} else {
			sketchInJournal("");
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
