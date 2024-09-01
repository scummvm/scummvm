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

#include "m4/riddle/rooms/section4/room405.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const int16 NORMAL_DIRS[] = { 220, 221, -1 };
static const char *NORMAL_NAMES[] = {
	"baron walker pos1",
	"baron walker pos7"
};

static const int16 SHADOW_DIRS[] = { 230, 231, -1 };
static const char *SHADOW_NAMES[] = {
	"baron walker shadow pos1",
	"baron walker shadow pos7"
};

static const char *const SAID[][2] = {
	{ "FIREPLACE",  "405r05" },
	{ "WINDOW",     "405r06" },
	{ "ZEBRA SKIN", "405r07" },
	{ "DEER HEAD",  "405r08" },
	{ "BATTLE AXE", "405r09" },
	{ "SHIELD",     "405r09" },
	{ "CHANDELIER", "405r10" },
	{ "BOOKSHELF",  "405r11" },
	{ "RUG",        "405r16" },
	{ "CHAIR",      "405r13" },
	{ nullptr, nullptr }
};

void Room405::init() {
	player_set_commands_allowed(false);
	_door = series_place_sprite("405doora", 0, 0, 0, 100, 0);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 0;
		_val2 = -1;
		_val3 = 0;
		_val4 = -1;
		_val5 = 0;
		_val6 = 0;
		_val7 = 0;
		_val8 = 0;
		_val9 = 0;
	}

	if (!_G(flags)[V338] || !inv_object_is_here("GERMAN BANKNOTE"))
		hotspot_set_active("GERMAN BANKNOTE", false);

	_safariShadow = series_load("SAFARI SHADOW 3");

	if (inv_object_is_here("GERMAN BANKNOTE"))
		_bankNote = series_place_sprite("405 BANK NOTE UNDER RUG", 0, 0, 0, 100, 0xf00);

	_candlesBurning = series_load("TWO CANDLES BURNING");
	_candles = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
		triggerMachineByHashCallbackNegative, "candles");
	sendWSMessage_10000(1, _candles, _candlesBurning, 1, 8, -1,
		_candlesBurning, 1, 8, 0);

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		player_set_commands_allowed(true);
		return;
	}

	_G(flags)[V110] = 1;
	if (!inv_player_has("TURTLE"))
		inv_move_object("TURTLE", 305);

	ws_demand_location(155, 370, 9);

	if (player_been_here(405) || _G(kittyScreaming)) {
		ws_walk(230, 345, nullptr, 50, 2);
	} else {
		ws_walk_load_shadow_series(SHADOW_DIRS, SHADOW_NAMES);
		ws_walk_load_walker_series(NORMAL_DIRS, NORMAL_NAMES);
		_baron = triggerMachineByHash_3000(8, 11, NORMAL_DIRS, SHADOW_DIRS, 185, 365, 1,
			triggerMachineByHashCallback3000, "BARON_walker");
		ws_walk(329, 320, nullptr, 20, 9);
	}
}

void Room405::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case 20:
		digi_preload("405b01a");
		_baronShakeSit = series_load("BARON SHAKES AND SITS");
		_baronTalkLoop = series_load("BARON TALK LOOP");
		_baronLeanForward = series_load("BARON LEAN FORWARD");
		_baronTurnWalk = series_load("BARON TURNWALK");
		_ripHandLetter = series_load("RIP HANDS BARON LETTER");
		_ripTalkGesture = series_load("RIPLEY TALKGESTURE");
		_ripLeanForward = series_load("RIPLEY LEAN FORWARD");
		_lowReacher = series_load("RIP TREK MED REACH HAND POS1");
		_ripHeadTurn = series_load("RIP TREK HEAD TURN POS3");

		digi_preload("13_05n03");
		digi_preload("13_03n01");
		digi_preload("13_09p03");
		setGlobals1(_ripHeadTurn, 6, 9, 9, 9, 0, 9, 6, 6, 6);
		sendWSMessage_110000(-1);
		kernel_timing_trigger(50, 21);
		sendWSMessage_10000(_baron, 284, 324, 1, 22, 1);
		break;

	case 21:
		sendWSMessage_120000(-1);
		break;

	case 22:
		ws_demand_location(286, 324, 7);
		ws_hide_walker();
		sendWSMessage_150000(-1);
		_baronWalker = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x600, 0,
			triggerMachineByHashCallbackNegative, "BARON talks rip");
		sendWSMessage_10000(1, _baronWalker, _baronShakeSit, 1, 48, 23,
			_baronShakeSit, 48, 48, 0);
		digi_play("405b01", 1);
		break;

	case 23:
		sendWSMessage_10000(1, _baronWalker, _baronShakeSit, 48, 100, 24,
			_baronShakeSit, 100, 100, 0);
		break;
		digi_play("405b01a", 1, 255, 25);
		break;

	case 24:
		_ripTalksBaron = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x600, 0,
			triggerMachineByHashCallbackNegative, "rip talks baron");
		_val5 = 1000;
		_val6 = 1103;
		kernel_timing_trigger(1, 102);
		_val7 = 2000;
		_val8 = 2101;
		kernel_timing_trigger(1, 110);
		break;


	case 25:
		digi_play("405b02", 1, 255, 26);
		break;

	case 26:
		_val8 = 2102;
		_val6 = 1102;
		digi_play("405r01", 1, 255, 27);
		break;

	case 27:
		_val6 = 1104;
		_val8 = 2101;
		digi_play("405b03", 1, 255, 28);
		break;

	case 28:
		_val8 = 2103;
		kernel_timing_trigger(10, 29);
		break;

	case 29:
		terminateMachineAndNull(_baronWalker);
		sendWSMessage_10000(1, _ripTalksBaron, _ripHandLetter, 1, 15, 30,
			_ripHandLetter, 15, 15, 0);
		break;

	case 30:
		digi_preload("950_s06");
		sendWSMessage_10000(1, _ripTalksBaron, _ripHandLetter, 16, 29, -1,
			_ripHandLetter, 29, 29, 0);
		digi_play("405r02", 1, 255, 31);
		digi_play("950_s06", 2, 255, -1, 950);
		break;

	case 31:
		sendWSMessage_10000(1, _ripTalksBaron, _ripHandLetter, 30, 84, -1,
			_ripHandLetter, 84, 84, 0);
		break;

	case 32:
		digi_play("405b04", 1, 255, 33);
		break;

	case 33:
		digi_play("405b03", 1, 255, 34);
		break;

	case 34:
		digi_play("405b05", 1, 255, 35);
		break;

	case 35:
		digi_play("950_s06", 2, 255, -1, 950);
		sendWSMessage_10000(1, _ripTalksBaron, _ripHandLetter, 85, 98, 36,
			_ripHandLetter, 98, 98, 0);
		break;

	case 36:
		_baronWalker = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x600, 0,
			triggerMachineByHashCallbackNegative, "BARON talks rip");
		sendWSMessage_10000(1, _baronWalker, _baronTalkLoop, 1, 1, 110,
			_baronTalkLoop, 1, 1, 0);
		kernel_timing_trigger(1, 37);
		break;

	case 37:
		digi_play("405b05a", 1, 255, 38);
		_val8 = 2101;
		kernel_timing_trigger(1, 110);
		break;

	case 38:
		_val8 = 2102;
		sendWSMessage_10000(1, _ripTalksBaron, _ripHandLetter, 15, 1, 39,
			_ripHandLetter, 1, 1, 0);
		break;

	case 39:
		_val5 = 1000;
		_val6 = 1100;
		kernel_timing_trigger(1, 102);
		break;

	case 40:
		kernel_timing_trigger(1, 60);
		break;

	case 50:
	case 69:
		player_set_commands_allowed(true);
		break;

	case 61:
		digi_play("13_15n07", 1, 255, 62);
		break;

	case 62:
		sendWSMessage_10000(1, _baronWalker, _baronTurnWalk, 58, 79, 63,
			_baronTurnWalk, 80, 82, 4);
		sendWSMessage_1a0000(_baronWalker, 11);
		break;

	case 63:
		digi_play("13_20p01", 1, 255, 64);
		break;

	case 64:
		sendWSMessage_10000(1, _baronWalker, _baronTurnWalk, 83, 99, 65,
			_baronTurnWalk, 99, 99, 0);
		break;

	case 65:
		ws_unhide_walker(_baron);
		ws_unhide_walker();
		setGlobals1(_ripHeadTurn, 6, 9, 9, 9, 0, 9, 6, 6, 6);
		sendWSMessage_110000(-1);
		terminateMachineAndNull(_baronWalker);
		sendWSMessage_10000(_baron, 185, 365, 9, 66, 1);
		break;

	case 66:
		sendWSMessage_60000(_baron);
		digi_play("13_20n01", 1, 255, 67);
		sendWSMessage_120000(68);
		break;

	case 67:
		digi_play("405_s01", 2, 255, 69);
		break;

	case 68:
		sendWSMessage_150000(-1);
		break;

	case 100:
		kernel_timing_trigger(1, 102);
		break;

	case 101:
		_val8 = 2163;
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
				ws_hide_walker();
				player_set_commands_allowed(false);
				sendWSMessage_10000(1, _ripTalksBaron, _ripTalkGesture, 13, 13, 102,
					_ripTalkGesture, 13, 13, 0);
				_val6 = 1101;
				_val7 = 2000;
				_val8 = 2100;
				kernel_timing_trigger(1, 110);
				break;

			case 1101:
				_val6 = 1103;
				kernel_timing_trigger(1, 102);

				conv_load("conv405a", 10, 10, 101);
				conv_export_pointer_curr(&_G(flags)[V115], 0);
				conv_export_pointer_curr(&_G(flags)[V314], 1);
				conv_play();
				break;

			case 1102:
				frame = imath_ranged_rand(13, 16);
				sendWSMessage_10000(1, _ripTalksBaron, _ripTalkGesture, frame, frame, 102,
					_ripTalkGesture, frame, frame, 0);
				break;

			case 1103:
				sendWSMessage_10000(1, _ripTalksBaron, _ripTalkGesture, 13, 13, 102,
					_ripTalkGesture, 13, 13, 0);
				break;

			case 1104:
				sendWSMessage_10000(1, _ripTalksBaron, _ripTalkGesture, 13, 13, -1,
					_ripTalkGesture, 13, 13, 0);
				break;

			case 1220:
				_G(kernel).call_daemon_every_loop = true;
				_response = series_stream("RIP DONT I KNOW", 4, 0, 103);
				series_stream_break_on_frame(_response, 9, 103);
				_val6 = 1221;
				break;

			case 1221:
				if (!_sound2.empty()) {
					digi_play(_sound2.c_str(), 1);
					_sound2.clear();
				}

				_G(kernel).call_daemon_every_loop = false;
				_val6 = 1222;
				break;

			case 1222:
				_val6 = 1103;
				kernel_timing_trigger(1, 102);
				conv_resume();
				break;

			case 1230:
				sendWSMessage_10000(1, _ripTalksBaron, _ripLeanForward, 1, 16, 103,
					_ripLeanForward, 16, 16, 0);
				_val6 = 1232;
				break;

			case 1232:
				_val6 = 1233;

				if (!_sound2.empty()) {
					digi_play(_sound2.c_str(), 1, 255, 103);
					_sound2.clear();
				}
				break;

			case 1233:
				sendWSMessage_10000(1, _ripTalksBaron, _ripLeanForward, 16, 1, 102,
					_ripLeanForward, 1, 1, 0);
				_val6 = 1103;
				conv_resume();
				break;

			case 2110:
				sendWSMessage_10000(1, _ripTalksBaron, _ripTalkGesture, 13, 42, 103,
					_ripTalkGesture, 42, 42, 0);
				_val6 = 2112;

				if (!_sound2.empty()) {
					_G(kernel).trigger_mode = KT_PARSE;
					digi_play(_sound2.c_str(), 1);
					_G(kernel).trigger_mode = KT_DAEMON;
					_sound2.clear();
				}
				break;

			case 2112:
				_val6 = 1102;
				kernel_timing_trigger(1, 102);
				conv_resume();
				break;

			default:
				break;
			}
		}
		break;

	case 110:
		if (_val7 == 2000) {
			if (_val4 != -1) {
				kernel_timing_trigger(1, _val4);
				_val4 = -1;
			} else {
				kernel_timing_trigger(1, 111);
			}
		}
		break;

	case 111:
		if (_val7 == 2000) {
			switch (_val8) {
			case 2100:
			case 2102:
				sendWSMessage_10000(1, _baronWalker, _baronTalkLoop, 1, 1, 110,
					_baronTalkLoop, 1, 1, 0);
				_val8 = 2102;
				break;

			case 2101:
				frame = imath_ranged_rand(4, 9);
				sendWSMessage_10000(1, _baronWalker, _baronTalkLoop, frame, frame, 110,
					_baronTalkLoop, frame, frame, 0);
				break;

			case 2103:
				sendWSMessage_10000(1, _baronWalker, _baronTalkLoop, 1, 1, -1,
					_baronTalkLoop, 1, 1, 0);
				break;

			case 2110:
				_G(kernel).call_daemon_every_loop = true;
				_response = series_stream("Baron sport is life", 5, 0, 111);
				series_stream_break_on_frame(_response, 3, 111);
				_val8 = 2111;
				break;

			case 2111:
				if (!_sound1.empty()) {
					digi_play(_sound1.c_str(), 1);
					_sound1.clear();
				}

				_G(kernel).call_daemon_every_loop = false;
				_val8 = 2112;
				break;

			case 2112:
			case 2153:
				_val8 = 2102;
				kernel_timing_trigger(1, 110);
				conv_resume();
				break;

			case 2120:
				_G(kernel).call_daemon_every_loop = true;
				_response = series_stream("BARON WORLD WAR", 6, 0, 111);
				series_stream_break_on_frame(_response, 4, 111);
				_val8 = 2121;
				break;

			case 2121:
				if (!_sound1.empty()) {
					digi_play(_sound1.c_str(), 1);
					_sound1.clear();
				}

				_val8 = 2122;
				_G(kernel).call_daemon_every_loop = false;
				break;

			case 2122:
				kernel_timing_trigger(1, 666);
				break;

			case 2123:
				_val8 = 2152;
				kernel_timing_trigger(1, 110);
				_val6 = 1103;
				kernel_timing_trigger(1, 102);
				break;

			case 2140:
				sendWSMessage_10000(1, _baronWalker, _baronLeanForward, 1, 11, -1,
					_baronLeanForward, 12, 18, 1);
				_val8 = 2142;

				if (!_sound1.empty()) {
					digi_play(_sound1.c_str(), 1, 255, 111);
					_sound1.clear();
				}
				break;

			case 2142:
				sendWSMessage_10000(1, _baronWalker, _baronLeanForward, 11, 11, 111,
					_baronLeanForward, 11, 11, 0);
				_val8 = 2162;
				break;

			case 2150:
				sendWSMessage_10000(1, _baronWalker, _baronLeanForward, 11, 11, -1,
					_baronLeanForward, 12, 18, 1);

				if (!_sound1.empty()) {
					digi_play(_sound1.c_str(), 1, 255, 111);
					_sound1.clear();
				}
				break;
				
			case 2152:
				sendWSMessage_10000(1, _baronWalker, _baronLeanForward, 12, 1, 111,
					_baronLeanForward, 1, 1, 0);
				_val8 = 2153;
				break;

			case 2161:
				frame = imath_ranged_rand(12, 18);
				sendWSMessage_10000(1, _baronWalker, _baronLeanForward, frame, frame, 110,
					_baronLeanForward, frame, frame, 0);
				break;

			case 2162:
				sendWSMessage_10000(1, _baronWalker, _baronLeanForward, 11, 11, 111,
					_baronLeanForward, 11, 11, 0);
				_val8 = 2162;
				break;

			case 2163:
				terminateMachineAndNull(_ripTalksBaron);
				sendWSMessage_10000(1, _baronWalker, _baronTurnWalk, 1, 57, 61,
					_baronTurnWalk, 77, 79, 4);
				sendWSMessage_1a0000(_baronWalker, 11);
				break;

			case 2171:
				sendWSMessage_10000(1, _baronWalker, _baronLeanForward, 12, 1, 111,
					_baronLeanForward, 1, 1, 0);
				_val8 = 2102;
				break;

			default:
				break;
			}
		}
		break;

	case 666:
		_response = series_stream("RIP WAR REACTION", 5, 0, 111);
		series_stream_break_on_frame(_response, 4, 667);
		_val8 = 2123;
		break;

	case 667:
		series_stream_check_series(_response, 15);
		series_stream_break_on_frame(_response, 10, 668);
		break;

	case 668:
		series_stream_check_series(_response, 5);
		ws_OverrideCrunchTime(_response);
		break;

	default:
		break;
	}
}

void Room405::pre_parser() {
	bool takeFlag = player_said("take");
	bool lookFlag = player_said_any("look", "look at");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said("DOOR") && (lookFlag || useFlag || takeFlag)) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (player_said("journal") && !takeFlag && !lookFlag && _G(kernel).trigger == -1) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room405::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool talkFlag = player_said_any("talk", "talk to");
	bool takeFlag = player_said("take");
	bool enterFlag = player_said("enter");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said("conv405a")) {
		if (_G(kernel).trigger == 1)
			conv405a1();
		else
			conv405a();
	} else if (talkFlag && player_said("baron")) {
		player_set_commands_allowed(false);
		_val4 = -1;
		_val5 = 1000;
		_val6 = 1100;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 102);
		_G(kernel).trigger_mode = KT_PARSE;
	} else if (enterFlag) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;
		case 1:
			midi_stop();
			digi_stop(3);
			_G(game).setRoom(404);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("DOOR") && lookDoor()) {
		// No implementation
	} else if (useFlag && player_said("DOOR") && useDoor()) {
		// No implementation
	} else if (takeFlag && player_said("DOOR") && takeDoor()) {
		// No implementation
	} else if (lookFlag && _G(walker).ripley_said(SAID)) {
		// No implementation
	} else if ((lookFlag && player_said("LIBRARY TABLE")) ||
			(lookFlag && player_said("JOURNAL "))) {
		if (_val9) {
			doAction("405r14");
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				digi_play("405r14", 1, 255, 1);
				break;
			case 1:
				digi_play("405r14a", 1);
				_val9 = 1;
				break;
			default:
				break;
			}
		}
	} else if (lookFlag && player_said("GERMAN BAKNOTE") && inv_object_is_here("GERMAN BAKNOTE")) {
		doAction("405r17");
	} else if (lookFlag && player_said(" ")) {
		doAction("405r04");
	} else if (takeFlag && player_said("ZEBRA SKIN")) {
		doAction("405r18");
	} else if (takeFlag && player_said("DEER HEAD")) {
		doAction("405r19");
	} else if (takeFlag && player_said("BATTLE AXE")) {
		doAction("405r20");
	} else if (takeFlag && player_said("SHIELD")) {
		doAction("405r20");
	} else if (takeFlag && player_said("BOOKSHELF")) {
		doAction("405r21");
	} else if (takeFlag && player_said("SOFA")) {
		doAction("405r22");
	} else if (takeFlag && player_said("CHAIR")) {
		doAction("405r22");
	} else if (takeFlag && player_said("JOURNAL ")) {
		doAction("405r23");
	} else if (takeFlag && player_said("GERMAN BANKNOTE") && takeBanknote()) {
		// No implementation
	} else if (player_said("journal") && !takeFlag && !lookFlag && !inv_player_has(_G(player).noun)) {
		if (_G(flags)[kCastleCartoon]) {
			digi_play("com016", 1);
		} else {
			if (_G(kernel).trigger == 6)
				_G(flags)[kCastleCartoon] = 1;
			sendWSMessage_multi("com015");
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room405::conv405a() {
	int who = conv_whos_talking();
	_currentNode = conv_current_node();
	int entry = conv_current_entry();
	const char *sound = conv_sound_to_play();

	if (sound) {
		if (who <= 0) {
			switch (_currentNode) {
			case 2:
				if (entry == 0) {
					_val8 = 2110;
					_sound1 = sound;
				} else {
					_val8 = 2101;
					digi_play(sound, 1, 255, 1);
				}
				break;

			case 4:
				if (entry == 1) {
					_val8 = 2140;
					_sound1 = sound;
				} else if (entry == 2) {
					_val8 = 2120;
					_sound1 = sound;
				} else {
					_val8 = 2101;
					digi_play(sound, 1, 255, 1);
				}
				break;

			case 14:
				if (entry == 4) {
					_val8 = 2140;
					_sound1 = sound;
				} else {
					_val8 = 2101;
					digi_play(sound, 1, 255, 1);
				}
				break;

			default:
				_val8 = 2101;
				digi_play(sound, 1, 255, 1);
				break;
			}
		} else if (who == 1) {
			switch (_currentNode) {
			case 1:
				if (entry == 3) {
					_val6 = 2110;
					_sound2 = sound;
				} else {
					_val6 = 1102;
					digi_play(sound, 1, 255, 1);
				}
				break;

			case 8:
				if (entry == 2) {
					_val6 = 1220;
					_sound2 = sound;
				} else {
					_val6 = 1102;
					digi_play(sound, 1, 255, 1);
				}
				break;

			case 14:
				if (entry == 1) {
					_val6 = 1230;
					_sound2 = sound;
				} else if (entry == 5) {
					_val8 = 2171;
					_val6 = 1102;
					digi_play(sound, 1, 255, 1);
				}
				break;

			default:
				break;
			}
		}
	} else {
		conv_resume();
	}
}

void Room405::conv405a1() {
	int who = conv_whos_talking();

	if (who <= 0)
		_val8 = 2102;
	else if (who == 1)
		_val6 = 1103;

	conv_resume();
}

bool Room405::lookDoor() {
	switch (_G(kernel).trigger) {
	case -1:
		ws_walk(245, 367, nullptr, 2, 9);
		return true;
	case 2:
		digi_play("405r30", 1);
		return true;
		
	default:
		break;
	}

	return false;
}

bool Room405::useDoor() {
	switch (_G(kernel).trigger) {
	case -1:
		ws_walk(245, 367, nullptr, 2, 9);
		return true;
	case 2:
		digi_play("405r31", 1);
		return true;

	default:
		break;
	}

	return false;
}

bool Room405::takeDoor() {
	if (_G(kernel).trigger == 1) {
		ws_walk(245, 367, nullptr, 2, 9);
		return true;
	}

	return false;
}

bool Room405::takeBanknote() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("GERMAN BANKNOTE")) {
			player_set_commands_allowed(false);
			_lowReacher = series_load("RIP TREK LOW REACHER POS1");
			setGlobals1(_lowReacher, 1, 16, 16, 16, 0, 16, 1, 1, 1);
			sendWSMessage_110000(1);
			return true;
		}
		break;

	case 1:
		kernel_examine_inventory_object("PING GERMAN BANKNOTE", _G(master_palette),
			5, 1, 265, 270, 2, "405r24", -1);
		return true;

	case 2:
		terminateMachineAndNull(_bankNote);
		inv_give_to_player("GERMAN BANKNOTE");
		sendWSMessage_120000(3);
		return true;

	case 3:
		hotspot_set_active("GERMAN BANKNOTE", false);
		sendWSMessage_150000(4);
		return true;

	case 4:
		series_unload(_lowReacher);
		player_set_commands_allowed(true);
		return true;

	default:
		break;
	}

	return false;
}

void Room405::doAction(const char *name) {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		digi_play(name, 1, 255, 1);
		break;
	case 1:
		player_set_commands_allowed(true);
		break;
	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
