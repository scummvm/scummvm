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

#include "m4/riddle/rooms/section5/room504.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/gui/gui_vmng.h"
#include "m4/m4.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room504::init() {
	_trigger1 = -1;

	_vines1 = nullptr;
	_vines2 = nullptr;
	_vines3 = nullptr;
	_vines4 = nullptr;
	_rope = nullptr;

	_ladder = nullptr;
	_toy = nullptr;
	_shovel = nullptr;
	_driftwood = nullptr;
	_pole = 0;

	_waterfall = series_plain_play("peruvian waterfall", -1, 0, 100, 0xf00, 9, -1, 0);
	digi_preload("504_S01");
	_volume = 1;
	kernel_timing_trigger(1, 501);
	digi_play("504_501", 3, _volume);
	kernel_timing_trigger(828, 754);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 506:
		player_set_commands_allowed(false);
		ws_demand_location(1384, 205, 9);
		ws_hide_walker();
		MoveScreenDelta(_G(game_buff_ptr), -1280, 0);
		_flag1 = true;

		_downSteps = series_load("504 down steps");
		player_update_info();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100,
			_G(player_info).depth, 0, triggerMachineByHashCallbackNegative, "Rp");
		sendWSMessage_10000(1, _ripley, _downSteps, 1, 27, 647,
			_downSteps, 27, 27, 0);
		break;

	default:
		player_set_commands_allowed(false);
		ws_demand_location(50, 226, 3);
		midi_play("MOCAMO", 200, 0, -1, 949);

		if (player_been_here(504)) {
			inv_give_to_player("ROPE");
			_G(flags)[V154] = 3;
			ws_walk(183, 207, nullptr, 762, 3);
		} else {
			_G(flags)[V152] = 6;
			_G(flags)[V153] = 6;
			_G(flags)[V154] = 2;
			_G(flags)[V171] = 3;
			ws_walk(183, 207, nullptr, 655, 3);
		}

		_flag1 = false;
		break;
	}

	setVines();
	setVinesRope();
	setMiscItems();

	if (!_G(flags)[V141])
		kernel_timing_trigger(1, 502);
}

void Room504::daemon() {
}

void Room504::pre_parser() {
	bool useFlag = player_said("gear");
	bool useFlag1 = useFlag && _flag1;
	bool useFlag0 = useFlag && !_flag1;

	if (useFlag1 && player_said("ROPE COIL "))
		intr_freshen_sentence(62);
	else if (useFlag1 && player_said("GREEN VINE COIL "))
		intr_freshen_sentence(63);
	else if (useFlag1 && player_said("BROWN VINE COIL "))
		intr_freshen_sentence(64);
	else if (useFlag1 && player_said("COIL OF VINES "))
		intr_freshen_sentence(107);

	else if (useFlag1 && player_said("ROPE COIL    "))
		intr_freshen_sentence(62);
	else if (useFlag1 && player_said("GREEN VINE COIL    "))
		intr_freshen_sentence(63);
	else if (useFlag1 && player_said("BROWN VINE COIL    "))
		intr_freshen_sentence(64);
	else if (useFlag1 && player_said("COIL OF VINES    "))
		intr_freshen_sentence(107);

	else if (useFlag0 && player_said("ROPE COIL  "))
		intr_freshen_sentence(62);
	else if (useFlag0 && player_said("GREEN VINE COIL  "))
		intr_freshen_sentence(63);
	else if (useFlag0 && player_said("BROWN VINE COIL  "))
		intr_freshen_sentence(64);
	else if (useFlag0 && player_said("COIL OF VINES  "))
		intr_freshen_sentence(107);

	else if (useFlag0 && player_said("ROPE COIL   "))
		intr_freshen_sentence(62);
	else if (useFlag0 && player_said("GREEN VINE COIL   "))
		intr_freshen_sentence(63);
	else if (useFlag0 && player_said("BROWN VINE COIL   "))
		intr_freshen_sentence(64);
	else if (useFlag0 && player_said("COIL OF VINES   "))
		intr_freshen_sentence(107);
}

#define ITEM(NAME) player_said(NAME) && inv_object_is_here(NAME)

void Room504::parser() {
	bool ropeCoilFlag = player_said_any("ROPE COIL ", "ROPE COIL  ",
		"ROPE COIL   ", "ROPE COIL    ");
	bool greenVineFlag = player_said_any("GREEN VINE COIL ",
		"GREEN VINE COIL  ", "GREEN VINE COIL   ", "GREEN VINE COIL    ");
	bool brownVineFlag = player_said_any("BROWN VINE COIL ",
		"BROWN VINE COIL  ", "BROWN VINE COIL   ", "BROWN VINE COIL    ");
	bool vineCoilFlag = player_said_any("COIL OF VINES ", "COIL OF VINES  ",
		"COIL OF VINES   ", "COIL OF VINES    ");
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool talkFlag = player_said_any("talk", "talk to");
	bool useFlag = player_said("gear");
	bool vineStatueFlag = player_said_any("rope ",
		"green vine ", "brown vine ", "vines ", "spider statue");
	bool menendezFlag = player_said_any("PERSON IN HOLE", "MENENDEZ");

	player_update_info();
	_flag1 = _G(player_info).x > 300;

	if (player_said("GREEN VINE", "BROWN VINE") && inv_player_has("GREEN VINE") &&
			inv_player_has("BROWN VINE")) {
		inv_move_object("GREEN VINE", NOWHERE);
		inv_move_object("BROWN VINE", NOWHERE);
		inv_give_to_player("VINES");
		_G(player).command_ready = false;
		return;
	} else if (((!_flag1 && _G(player).click_x > 300) ||
			(_flag1 && _G(player).click_x <= 300)) &&
			!lookFlag && !takeFlag && !useFlag &&
			checkVinesDistance()) {
		_G(player).command_ready = false;
		return;
	}

	_flag2 = _G(flags)[V154] == 2 ||
		(_G(flags)[V152] == 2 && _G(flags)[V153] == 2);

	if (!lookFlag && !takeFlag && !useFlag && !ropeCoilFlag &&
			!greenVineFlag && !brownVineFlag && !vineCoilFlag &&
			(_G(flags)[V154] == 2 || _G(flags)[V152] == 2 || _G(flags)[V153] == 2) &&
			parser1()) {
		// No implementation
	} else if (player_said("conv504a")) {
		conv504a();
	} else if (_G(kernel).trigger == 747) {
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 509);
	} else if (talkFlag && menendezFlag) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			g_engine->camera_shift_xy(862, 0);
			kernel_timing_trigger(1, 1);
			break;
		case 1:
			if (g_engine->game_camera_panning()) {
				kernel_timing_trigger(5, 1);
			} else {
				_trigger1 = kernel_trigger_create(2);
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(5, 505);
			}
			break;
		case 2:
			_convState2 = 3;
			digi_play("504R51", 1, 255, 3);
			break;
		case 3:
			_convState2 = 1;
			_convState = 2;
			_trigger2 = kernel_trigger_create(4);
			break;
		case 4:
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 508);
			break;
		default:
			break;
		}
	} else if (menendezFlag && player_said("ROMANOV EMERALD")) {
		player_set_commands_allowed(false);
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 511);
	} else if ((menendezFlag && player_said("PERUVIAN INTI")) ||
			(menendezFlag && !lookFlag && !takeFlag && !useFlag)) {
		player_set_commands_allowed(false);
		_flag4 = true;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 548);
	} else if (player_said("WALK UP")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			_upSteps = series_load("504 UP STEPS");
			player_update_info();
			ws_hide_walker();
			_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100,
				_G(player_info).depth, 0, triggerMachineByHashCallbackNegative, "Rp");
			sendWSMessage_10000(1, _ripley, _upSteps, 1, 25, -1, _upSteps, 25, 25, 0);
			kernel_timing_trigger(60, 2);
			break;
		case 2:
			disable_player_commands_and_fade_init(2);
			break;
		case 3:
			_G(game).setRoom(506);
			break;	
		default:
			break;
		}
	} else if (player_said("EXIT") && !_flag1) {
		if (inv_player_has("ROPE")) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				player_update_info();
				ws_walk(_G(player_info).x, _G(player_info).y, nullptr, -1, 9);
				disable_player_commands_and_fade_init(2);
				break;
			case 2:
				_G(game).setRoom(501);
				inv_move_object("ROPE", 504);
				break;
			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				ws_walk(183, 207, nullptr, 2, 3);
				break;
			case 2:
				digi_play("504R44", 1, 255, 3);
				break;
			case 3:
				player_set_commands_allowed(true);
				break;
			default:
				break;
			}
		}
	} else if (lookFlag && player_said("ROPE ") &&
			(_G(flags)[V154] == 1 || _G(flags)[V154] == 4)) {
		digi_play("504R39", 1);
	} else if (lookFlag && player_said("GREEN VINE ") &&
			(_G(flags)[V152] == 1 || _G(flags)[V152] == 4)) {
		digi_play("504R39", 1);
	} else if (lookFlag && player_said("BROWN VINE ") &&
			(_G(flags)[V153] == 1 || _G(flags)[V153] == 4)) {
		digi_play("504R39", 1);
	} else if (lookFlag && player_said("ROPE  ") &&
			(_G(flags)[V154] == 0 || _G(flags)[V154] == 5)) {
		digi_play("504R40", 1);
	} else if (lookFlag && player_said("GREEN VINE  ") &&
			(_G(flags)[V152] == 0 || _G(flags)[V152] == 5)) {
		digi_play("504R40", 1);
	} else if (lookFlag && player_said("BROWN VINE  ") &&
			(_G(flags)[V153] == 0 || _G(flags)[V153] == 5)) {
		digi_play("504R40", 1);
	} else if (lookFlag && ropeCoilFlag) {
		digi_play("504R36", 1);
	} else if (lookFlag && greenVineFlag) {
		digi_play("504R34", 1);
	} else if (lookFlag && brownVineFlag) {
		digi_play("504R34", 1);
	} else if (lookFlag && vineCoilFlag) {
		digi_play("504R35", 1);
	} else if (lookFlag && (
			player_said_any("GREEN VINE ", "GREEN VINE  ") ||
			player_said_any("BROWN VINE ", "BROWN VINE  ") ||
			player_said_any("ROPE ", "ROPE  ", "ROPE   ") ||
			player_said_any("GREEN VINE   ", "BROWN VINE   ")) &&
			lookVines()) {
		// No implementation
	} else if (lookFlag && player_said(" ")) {
		if (_G(flags)[V150]) {
			digi_play("504R02", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				digi_play("504R02", 1, 255, 2);
				break;
			case 2:
				_G(kernel).trigger_mode = KT_DAEMON;
				digi_play("504R02A", 1, 255, 762);
				_G(flags)[V150] = 1;
				break;
			default:
				break;
			}
		}
	} else if (lookFlag && player_said("WATERFALL")) {
		digi_play("504R03", 1);
	} else if (lookFlag && player_said("TREE")) {
		digi_play("504R04", 1);
	} else if (lookFlag && player_said("SPIDER STATUE")) {
		if (_G(flags)[V151] == 0 && _G(flags)[V154] == 2) {
			player_set_commands_allowed(false);
			_G(kernel).trigger_mode = KT_DAEMON;
			digi_play("504R06", 1, 255, 762);
			_G(flags)[V151] = 1;
		} else if (_G(flags)[V154] == 2 || _G(flags)[V154] == 1 ||
				_G(flags)[V154] == 4) {
			digi_play("504R06A", 1);
		} else {
			digi_play("504R06B", 1);
		}
	} else if (lookFlag && player_said("BROWN VINES")) {
		digi_play("504R07", 1);
	} else if (lookFlag && player_said("CHASM")) {
		digi_play("504R08", 1);
	} else if (lookFlag && player_said("STONE WALL")) {
		digi_play("504R09", 1);
	} else if (lookFlag && menendezFlag) {
		digi_play("504R12", 1);
	} else if (lookFlag && player_said("HOLE ")) {
		digi_play("504R33", 1);
	} else if (lookFlag && player_said("WHEELED TOY") &&
			_G(flags)[V141] == 1 && inv_player_has("WHEELED TOY")) {
		digi_play("504R20", 1);
	} else if (lookFlag && ITEM("WHEELED TOY")) {
		digi_play("504R50", 1);
	} else if (lookFlag && player_said("STAIRS")) {
		digi_play("504R25", 1);
	} else if (lookFlag && ITEM("DRIFTWOOD STUMP")) {
		digi_play("504R43", 1);
	} else if (lookFlag && ITEM("POLE")) {
		digi_play("504R43", 1);
	} else if (lookFlag && player_said("WOODEN LADDER ")) {
		digi_play("504R43", 1);
	} else if (lookFlag && ITEM("WOODEN LADDER")) {
		digi_play("COM107", 1, 255, -1, 504);
	} else if (lookFlag && ITEM("SHOVEL")) {
		digi_play("COM106", 1);
	} else if (lookFlag && player_said("SHOVEL ")) {
		digi_play("504R43", 1);
	}

	else if (useFlag && ITEM("SHOVEL")) {
		digi_play("504R19", 1);
	} else if (useFlag && player_said("HOLE ")) {
		digi_play("504R21", 1);
	} else if (vineStatueFlag && player_said("ROPE") &&
			_flag1 && inv_player_has("ROPE")) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V154] = 1;
		hotspot_set_active("ROPE", true);
		hotspot_set_active("ROPE COIL", true);
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 580);
	} else if (vineStatueFlag && player_said("GREEN VINE") &&
			_flag1 && inv_player_has("GREEN VINE")) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(flags)[V152] = 1;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 580);
	} else if (vineStatueFlag && player_said("BROWN VINE") &&
		_flag1 && inv_player_has("BROWN VINE")) {
		player_set_commands_allowed(false);
		_val1 = 3;
		_G(flags)[V153] = 1;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 580);
	} else if (vineStatueFlag && player_said("VINES") &&
		_flag1 && inv_player_has("VINES")) {
		player_set_commands_allowed(false);
		_val1 = 4;
		_G(flags)[V171] = 1;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 580);
	}

	else if (takeFlag && player_said_any("ROPE ", "ROPE COIL ") &&
			_G(flags)[V154] == 1 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V154] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(491, 166, nullptr, 586, 11);
	} else if (takeFlag && player_said_any("ROPE ") &&
		_G(flags)[V154] == 4 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V154] = 3;
		kernel_timing_trigger(1, 603);
	} else if (takeFlag && player_said_any("GREEN VINE ", "GREEN VINE COIL ") &&
			_G(flags)[V152] == 1 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(flags)[V152] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(491, 166, nullptr, 586, 11);
	} else if (takeFlag && player_said("GREEN VINE ") &&
			_G(flags)[V152] == 4 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(flags)[V152] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 603);
	} else if (takeFlag && player_said_any(
			"BROWN VINE ", "BROWN VINE COIL ") && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V153] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(491, 166, nullptr, 586, 11);
	} else if (takeFlag && player_said("BROWN VINE ") &&
			_G(flags)[V153] == 4 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 3;
		_G(flags)[V153] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 603);
	} else if (takeFlag && player_said("VINES ", "COIL OF VINES ") &&
			_G(flags)[V171] == 1 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 4;
		_G(flags)[V171] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(491, 166, nullptr, 586, 11);
	} else if (takeFlag && player_said("VINES ") &&
			_G(flags)[V171] == 4 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 4;
		_G(flags)[V171] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 603);
	} else if (takeFlag && !_flag1 && player_said_any("ROPE ", "ROPE COIL ") &&
			_G(flags)[V154] == 4) {
		digi_play("COM126", 1, 255, -1, 997);
	} else if (takeFlag && !_flag1 && player_said_any("GREEN VINE ", "GREEN VINE COIL ") &&
			_G(flags)[V154] == 4) {
		digi_play("COM126", 1, 255, -1, 997);
	} else if (takeFlag && !_flag1 && player_said_any("BROWN VINE ", "BROWN VINE COIL ") &&
			_G(flags)[V153] == 4) {
		digi_play("COM126", 1, 255, -1, 997);
	} else if (takeFlag && !_flag1 && player_said_any("VINES ", "COIL OF VINES  ") &&
			_G(flags)[V171] == 4) {
		digi_play("COM126", 1, 255, -1, 997);
	}
	// TODO
	else {
		return;
	}

	_G(player).command_ready = false;
}

void Room504::setVines() {
	freeVines();

	bool ropeFlag = _G(flags)[V152] == 2 || _G(flags)[V153] == 2 ||
		_G(flags)[V154] == 2;
	bool tiedFlag = _G(flags)[V152] == 0 || _G(flags)[V153] == 0 ||
		_G(flags)[V154] == 0 || _G(flags)[V171] == 0;
	bool rightVine = _G(flags)[V152] == 1 || _G(flags)[V153] == 1 ||
		_G(flags)[V154] == 1 || _G(flags)[V171] == 1;
	bool acrossFlag = _G(flags)[V152] == 5 || _G(flags)[V153] == 5 ||
		_G(flags)[V154] == 5 || _G(flags)[V171] == 5;
	bool hangingFlag = _G(flags)[V152] == 4 || _G(flags)[V153] == 4 ||
		_G(flags)[V154] == 4 || _G(flags)[V171] == 4;

	if (hangingFlag)
		_vines1 = series_place_sprite("504 R VINE HANGING ",
			0, 0, 0, 100, 0xe00);
	if (acrossFlag)
		_vines2 = series_place_sprite("VINE SPRITE ACROSS CHASM",
			0, 0, 0, 100, 0xe00);
	if (tiedFlag)
		_vines3 = series_place_sprite("VINE SPRITE AFTER FIRST TIED",
			0, 0, 0, 100, 0xe00);
	if (rightVine)
		_vines4 = series_place_sprite("504 R VINE SPRITE", 0, 0, 0, 100, 0xe00);
	if (ropeFlag)
		_rope = series_place_sprite("ROPE SPRITE", 0, 0, 0, 100, 0xe00);
}

void Room504::freeVines() {
	terminateMachineAndNull(_vines1);
	terminateMachineAndNull(_vines2);
	terminateMachineAndNull(_vines3);
	terminateMachineAndNull(_vines4);
	terminateMachineAndNull(_rope);
}

void Room504::setVinesRope() {
	disableVinesRope();

	if (_G(flags)[V154] == 2) {
		hotspot_set_active("ROPE ", true);
		hotspot_set_active("ROPE  ", true);
		addRope();
	}

	if (_G(flags)[V152] == 2) {
		hotspot_set_active("GREEN VINE ", true);
		hotspot_set_active("GREEN VINE  ", true);
		addGreenVine();
	}

	if (_G(flags)[V153] == 2) {
		hotspot_set_active("BROWN VINE ", true);
		hotspot_set_active("BROWN VINE  ", true);
		addBrownVine();
	}

	if (_G(flags)[154] == 1) {
		hotspot_set_active("ROPE ", true);
		hotspot_set_active("ROPE COIL ", true);
	}

	if (_G(flags)[V152] == 1) {
		hotspot_set_active("GREEN VINE ", true);
		hotspot_set_active("GREEN VINE COIL ", true);
	}

	if (_G(flags)[V153] == 1) {
		hotspot_set_active("BROWN VINE ", true);
		hotspot_set_active("BROWN VINE COIL ", true);
	}

	if (_G(flags)[V171] == 1) {
		hotspot_set_active("VINES ", true);
		hotspot_set_active("COIL OF VINES ", true);
	}

	if (_G(flags)[V154] == 0) {
		hotspot_set_active("ROPE  ", true);
		hotspot_set_active("ROPE COIL   ", true);
	}

	if (_G(flags)[V152] == 0) {
		hotspot_set_active("GREEN VINE  ", true);
		hotspot_set_active("GREEN VINE COIL   ", true);
	}

	if (_G(flags)[V153] == 0) {
		hotspot_set_active("BROWN VINE  ", true);
		hotspot_set_active("BROWN VINE COIL   ", true);
	}

	if (_G(flags)[V171] == 0) {
		hotspot_set_active("VINES  ", true);
		hotspot_set_active("COIL OF VINES   ", true);
	}

	if (_G(flags)[V154] == 4) {
		hotspot_set_active("ROPE ", true);
		hotspot_set_active("ROPE COIL  ", true);
	}

	if (_G(flags)[V152] == 4) {
		hotspot_set_active("GREEN VINE ", true);
		hotspot_set_active("GREEN VINE  ", true);
	}

	if (_G(flags)[V153] == 4) {
		hotspot_set_active("BROWN VINE ", true);
		hotspot_set_active("BROWN VINE COIL  ", true);
	}

	if (_G(flags)[V171] == 4) {
		hotspot_set_active("VINES ", true);
		hotspot_set_active("COIL OF VINES  ", true);
	}

	if (_G(flags)[V154] == 5) {
		hotspot_set_active("ROPE  ", true);
		hotspot_set_active("ROPE COIL    ", true);
	}

	if (_G(flags)[V152] == 5) {
		hotspot_set_active("GREEN VINE  ", true);
		hotspot_set_active("GREEN VINE COIL    ", true);
	}

	if (_G(flags)[V153] == 5) {
		hotspot_set_active("BROWN VINE  ", true);
		hotspot_set_active("BROWN VINE COIL    ", true);
	}

	if (_G(flags)[V171] == 5) {
		hotspot_set_active("VINES  ", true);
		hotspot_set_active("COIL OF VINES    ", true);
	}
}

void Room504::setMiscItems() {
	hotspot_set_active("WHEELED TOY", false);

	hotspot_set_active("SHOVEL", false);
	deleteHotspot("SHOVEL ");

	hotspot_set_active("WOODEN LADDER", false);
	hotspot_set_active("WOODEN LADDER ", false);
	hotspot_set_active("DRIFTWOOD STUMP", false);
	hotspot_set_active("POLE", false);
	hotspot_set_active("HOLE", false);
	hotspot_set_active("HOLE ", false);
	hotspot_set_active("PERSON IN HOLE", false);
	hotspot_set_active("MENENDEZ", false);

	terminateMachineAndNull(_ladder);
	terminateMachineAndNull(_toy);
	terminateMachineAndNull(_shovel);
	terminateMachineAndNull(_driftwood);
	terminateMachineAndNull(_pole);

	if (_G(flags)[V155] == 0) {
		_ladder = series_place_sprite("MENENDEZ LADDER", 0, 640, 0, 100, 0xf00);
		hotspot_set_active("WOODEN LADDER", true);
	}

	if (_G(flags)[V155] == 1 && inv_object_is_here("WOODEN LADDER")) {
		_ladder = series_place_sprite("504 DROPPED LADDER", 0, 0, 0, 100, 0xe00);
		hotspot_set_active("WOODEN LADDER ", true);
	}

	if (inv_object_is_here("WHEELED TOY")) {
		_toy = series_place_sprite("504TOY", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("WHEELED TOY", true);
	}

	if (_G(flags)[V044] == 0 && inv_object_is_here("SHOVEL") &&
			_G(flags)[V141] == 1) {
		_shovel = series_place_sprite("504shov", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("SHOVEL", true);
	}

	if (_G(flags)[V044] == 1 && inv_object_is_here("SHOVEL") &&
			_G(flags)[V141] == 1) {
		_shovel = series_place_sprite("504 DROPPED SHOVEL", 0, 0, 0, 100, 0xff0);
		addShovel();
	}

	if (inv_object_is_here("DRIFTWOOD STUMP")) {
		_shovel = series_place_sprite("504 DROPPED DRIFTWOOD", 0, 0, 0, 100, 0x750);
		hotspot_set_active("DRIFTWOOD STUMP", true);
	}

	if (inv_object_is_here("POLE")) {
		_shovel = series_place_sprite("504 DROPPED POLE", 0, 0, 0, 100, 0x750);
		hotspot_set_active("POLE", true);
	}

	if (_G(flags)[V141] != 0)
		hotspot_set_active("HOLE ", true);
	else if (_G(flags)[V040] != 1)
		hotspot_set_active("PERSON IN HOLE", true);
	else
		hotspot_set_active("MENENDEZ", true);
}

void Room504::disableVinesRope() {
	static const char *HOTSPOTS[] = {
		"ROPE ", "GREEN VINE ", "BROWN VINE ", "VINES ",
		"ROPE  ", "GREEN VINE  ", "BROWN VINE  "
		"VINES  ", "ROPE COIL ", "GREEN VINE COIL ",
		"BROWN VINE COIL ", "COIL OF VINES ",
		"ROPE COIL  ", "GREEN VINE COIL  ",
		"BROWN VINE COIL  ", "COIL OF VINES  ",
		"ROPE COIL   ", "GREEN VINE COIL   ",
		"BROWN VINE COIL   ", "COIL OF VINES   ",
		"ROPE COIL    ", "GREEN VINE COIL    ",
		"BROWN VINE COIL    ", "COIL OF VINES    ",
		nullptr
	};
	for (const char **hs = HOTSPOTS; *hs; ++hs)
		hotspot_set_active(*hs, false);

	deleteHotspot("ROPE   ");
	deleteHotspot("GREEN VINE   ");
	deleteHotspot("BROWN VINE   ");
}

void Room504::deleteHotspot(const char *hotspotName) {
	for (auto *hs = _G(currentSceneDef).hotspots; hs; hs = hs->next) {
		if (!strcmp(hs->vocab, hotspotName)) {
			_G(currentSceneDef).hotspots = hotspot_delete_record(
				_G(currentSceneDef).hotspots, hs);
			break;
		}
	}
}

void Room504::addRope() {
	if (_G(flags)[V152] != 2 && _G(flags)[V153] != 2)
		addHotspot(145, 73, 460, 118, "LOOK AT", "ROPE   ");
	if (_G(flags)[V152] == 2 && _G(flags)[V153] != 2)
		addHotspot(145, 73, 460, 95, "LOOK AT", "ROPE   ");
	if (_G(flags)[V152] == 2 && _G(flags)[V153] == 2)
		addHotspot(145, 73, 460, 88, "LOOK AT", "ROPE   ");
}

void Room504::addGreenVine() {
	if (_G(flags)[V154] != 2 && _G(flags)[V153] != 2)
		addHotspot(145, 73, 460, 118, "LOOK AT", "GREEN VINE   ");
	if (_G(flags)[V154] == 2 && _G(flags)[V153] != 2)
		addHotspot(145, 96, 460, 118, "LOOK AT", "GREEN VINE   ");
	if (_G(flags)[V154] != 2 && _G(flags)[V153] == 2)
		addHotspot(145, 73, 460, 95, "LOOK AT", "GREEN VINE   ");
	if (_G(flags)[V154] == 2 && _G(flags)[V153] == 2)
		addHotspot(145, 89, 460, 104, "LOOK AT", "GREEN VINE   ");
}

void Room504::addBrownVine() {
	if (_G(flags)[V154] != 2 && _G(flags)[V152] != 2)
		addHotspot(145, 73, 460, 118, "LOOK AT", "BROWN VINE   ");
	if (_G(flags)[V154] == 2 && _G(flags)[V152] != 2)
		addHotspot(145, 96, 460, 118, "LOOK AT", "BROWN VINE   ");
	if (_G(flags)[V154] != 2 && _G(flags)[V152] == 2)
		addHotspot(145, 96, 460, 118, "LOOK AT", "BROWN VINE   ");
	if (_G(flags)[V154] == 2 && _G(flags)[V152] == 2)
		addHotspot(145, 104, 460, 118, "LOOK AT", "BROWN VINE   ");
}

void Room504::addHotspot(int x1, int y1, int x2, int y2,
		const char *verb, const char *vocab) {
	HotSpotRec *hs = hotspot_new(x1, y1, x2, y2);
	hotspot_newVerb(hs, verb);
	hotspot_newVocab(hs, vocab);
	hs->cursor_number = 6;
	hotspot_add(_G(currentSceneDef).hotspots, hs, true);
}

void Room504::addShovel() {
	HotSpotRec *hs = hotspot_new(529, 109, 546, 143);
	hotspot_newVerb(hs, "LOOK AT");
	hotspot_newVocab(hs, "SHOVEL ");
	hs->cursor_number = 6;
	hs->feet_x = 543;
	hs->feet_y = 142;
	hs->facing = 11;

	_G(currentSceneDef).hotspots = hotspot_add(_G(currentSceneDef).hotspots, hs, true);
}

bool Room504::checkVinesDistance() {
	if (player_said("ROPE") && inv_player_has("ROPE"))
		digi_play("504R49", 1);
	else if (player_said("GREEN VINE") && inv_player_has("GREEN VINE"))
		digi_play("504R49", 1);
	else if (player_said("BROWN VINE") && inv_player_has("BROWN VINE"))
		digi_play("504R49", 1);
	else if (player_said("VINES") && inv_player_has("VINES"))
		digi_play("504R49", 1);
	else
		return false;

	return true;
}

bool Room504::parser1() {
	if (!_flag1 && _G(player).click_x > 300) {
		_G(kernel).trigger_mode = KT_DAEMON;
		if (inv_player_has("DRIFTWOOD STUMP") || inv_player_has("POLE")) {
			kernel_timing_trigger(1, 707);
		} else if (!_flag2) {
			ws_walk(200, 153, nullptr, 714, 3);
		} else {
			ws_walk(200, 153, nullptr, 570, 3);
		}
	} else if (_flag1 && _G(player).click_x <= 300) {
		_flag3 = player_said("EXIT") && _flag2;
		_G(kernel).trigger_mode = KT_DAEMON;

		if (inv_player_has("WOODEN LADDER") ||
				inv_player_has("LADDER/ROPE") ||
				inv_player_has("LADDER/GREEN VINE") ||
				inv_player_has("LADDER/BROWN VINE") ||
				inv_player_has("LADDER/VINES")) {
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 725);
		} else if (inv_player_has("SHOVEL")) {
			kernel_timing_trigger(1, 755);
		} else if (_flag2) {
			ws_walk(528, 168, nullptr, 558, 8);
		} else {
			ws_walk(528, 168, nullptr, 734, 8);
		}
	} else {
		return false;
	}

	return true;
}

void Room504::conv504a() {
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();
	const char *sound = conv_sound_to_play();

	if (_G(kernel).trigger == 1) {
		if (who <= 0)
			_convState = 2;
		else if (who == 1)
			_convState2 = 1;

		conv_resume();
	} else {
		if (who <= 0) {
			if (node == 8 && entry == 3) {
				_convState = 10;
			} else if (node == 19 && entry == 2) {
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 669);
			} else {
				_convState = 4;

				if (sound)
					digi_play(sound, 1, 255, 1);
				else
					conv_resume();
			}
		} else if (who == 1) {
			if ((node == 15 && entry == 0) || (node == 18 && entry == 0)) {
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 657);
			} else {
				_convState2 = 3;

				if (sound)
					digi_play(sound, 1, 255, 1);
				else
					conv_resume();
			}
		}
	}
}

bool Room504::lookVines() {
	if (_G(flags)[V152] == 2 && _G(flags)[V153] == 2 &&
		_G(flags)[V154] == 2) {
		digi_play("504R38", 1);
	} else if (_G(flags)[V154] == 2 && _G(flags)[V152] == 2) {
		digi_play("504R37", 1);
	} else if (_G(flags)[V154] == 2 && _G(flags)[V153] == 2) {
		digi_play("504R37", 1);
	} else if (_G(flags)[V154] == 2) {
		if (_G(flags)[V167]) {
			digi_play("504R05A", 1);
		} else {
			digi_play("504R05", 1);
			_G(flags)[V167] = 1;
		}
	} else if (_G(flags)[V152] == 2 && _G(flags)[V153] == 2) {
		digi_play("504R42", 1);
	} else if (_G(flags)[V152] == 2 || _G(flags)[V153] == 2) {
		digi_play("504R41", 1);
	} else {
		return false;
	}

	return true;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
