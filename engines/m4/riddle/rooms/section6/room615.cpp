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

#include "m4/riddle/rooms/section6/room615.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room615::preload() {
	Room::preload();
	_G(flags)[V000] = 1;
}

void Room615::init() {
	player_set_commands_allowed(false);
	_val1 = 0;
	_val2 = 0;
	_val3 = 90;

	static const char *DIGI[] = {
		"615r01", "615t01", "615r02", "615r02a",
		"615t02", "615t03", "615r03", "615t04",
		"615r04", "615t05", "615r05", "615t06",
		"615r06", "615r08", "615r12", "615r13",
		"615t10", "615t10a", "615r15", nullptr
	};
	for (static const char **name = DIGI; *name; ++name)
		digi_preload(*name);

	_ripTalking = series_load("615 RIP TALKING");
	_ripTalk = series_load("RIP TREK TALK");
	_loop0 = series_load("615LOOP0");
	_loop1 = series_load("615LOOP1");
	_loop2 = series_load("615LOOP2");
	_loop3 = series_load("615LOOP3");
	_loop4 = series_load("615LOOP4");
	_loop5 = series_load("615LOOP5");
	_loop6 = series_load("615LOOP6");
	_loop7 = series_load("615LOOP7");
	_ear2 = series_load("615ear2");

	_untie = series_stream("615 UNTIE", 7, 0xf00, 46);
	series_stream_break_on_frame(_untie, 25, 10);
	kernel_timing_trigger(1, 9);

	digi_preload("950_s28a");
	sendWSMessage_60000(_G(my_walker));
	_G(player).walker_in_this_scene = false;

	series_unload(0);
	series_unload(1);
	series_unload(2);
	series_unload(3);
	series_unload(4);
	series_unload(10);
	series_unload(11);
	series_unload(12);
	series_unload(13);
	series_unload(14);

	digi_play_loop("950_s28a", 3, 50);
	kernel_timing_trigger(1, 1);
}

void Room615::daemon() {
	switch (_G(kernel).trigger) {
	case 9:
		digi_play("615r01", 1, 255, 10);
		break;

	case 10:
		if (_flag1) {
			_flag1 = false;
			series_stream_break_on_frame(_untie, 79, 11);
			digi_play("615t01", 1, 255, 11);
		} else {
			_flag1 = true;
		}
		break;

	case 11:
		if (_flag1) {
			_flag1 = false;
			series_set_frame_rate(_untie, 30000);
			digi_play("615r02", 1, 255, 15);
		} else {
			_flag1 = true;
		}
		break;

	case 15:
		_pu = series_stream("615PU01", 6, 0, 21);
		series_stream_break_on_frame(_pu, 6, 17);
		break;

	case 17:
		digi_play("615r02a", 1, 255, 21);
		break;

	case 21:
		if (_flag1) {
			_flag1 = false;
			_pu = series_stream("615PU02", 6, 0x100, 24);
			series_stream_break_on_frame(_pu, 5, 22);
			series_set_frame_rate(_pu, 4);
		} else {
			_flag1 = true;
		}
		break;

	case 22:
		digi_play("615t02", 1, 255, 24);
		break;

	case 24:
		if (_flag1) {
			_flag1 = false;
			series_set_frame_rate(_untie, 7);
			ws_OverrideCrunchTime(_untie);
			series_stream_break_on_frame(_untie, 144, 26);
			digi_play("615t03", 1, 255, 25);
		} else {
			_flag1 = true;
		}
		break;

	case 25:
		_flag1 = false;
		_ripTalker = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 393, 361, 97, 256, 1,
			triggerMachineByHashCallback, "rip talker");
		sendWSMessage_10000(1, _ripTalker, _ripTalk, 1, 1, 40, _ripTalk, 1, 7, 4);
		sendWSMessage_1a0000(_ripTalker, 11);
		digi_play("615r03", 1, 255, 40);
		break;

	case 26:
		if (!_val1)
			series_set_frame_rate(_untie, 3000);
		break;

	case 40:
		if (_flag1) {
			_flag1 = false;
			digi_unload("615r01");
			digi_unload("615t01");
			digi_unload("615r02");
			digi_unload("615r02a");
			digi_unload("615t02");
			digi_unload("615t03");
			digi_unload("615r03");

			_val1 = 1;
			terminateMachineAndNull(_ripTalker);
			series_set_frame_rate(_untie, 7);
			ws_OverrideCrunchTime(_untie);
	
		} else {
			_flag1 = true;
		}
		break;

	case 46:
		if (_ripTalker) {
			kernel_timing_trigger(10, 46);
		} else {
			_ctr1 = 0;
			series_load("RIP SAFARI WALKER POSITION 1");
			series_load("RIP SAFARI WALKER POSITION 2", 1);
			series_load("RIP SAFARI WALKER POSITION 3", 2);
			series_load("SAFARI SHADOW 1", 10);
			series_load("SAFARI SHADOW 1", 11);
			series_load("SAFARI SHADOW 1", 12);

			_pu = triggerMachineByHash_3000(8, 0, *RIPLEY_SERIES_DIRS, *RIPLEY_SHADOWS_DIRS,
				392, 361, 10, triggerMachineByHashCallback3000, "rip");
			sendWSMessage_10000(_pu, 426, 347, 9, 50, 1);

			_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
				triggerMachineByHashCallback, "untie tt");
			sendWSMessage_10000(1, _tt, _loop7, 1, 1, 50, _loop7, 1, 5, 1);
			sendWSMessage_1a0000(_tt, 15);
			digi_play("615t04", 1, 255, 50);
		}
		break;

	case 50:
		if (_ctr1 >= 2) {
			_ctr1 = 0;
			player_update_info(_pu, &_G(player_info));
			ws_hide_walker(_pu);

			series_play("SAFARI SHADOW 3", 0xf00, 128, -1, 600, -1,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y, 0, 0);
			terminateMachineAndNull(_tt);

			_untie = series_stream("615ALL1", 6, 0x100, 57);
			series_stream_break_on_frame(_untie, 47, 55);
			digi_play("615r04", 1);
		} else {
			++_ctr1;
		}
		break;

	case 55:
		digi_play("615t05", 1, 255, 60);
		break;

	case 57:
		_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "x");
		sendWSMessage_10000(1, _tt, _loop0, 1, 1, 60, _loop0, 1, 5, 1);
		sendWSMessage_1a0000(_tt, 13);
		break;

	case 60:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			terminateMachineAndNull(_tt);
			_untie = series_stream("615ALL1a", 6, 0x100, 65);
			digi_play("615r05", 1, 255, 62);
		} else {
			++_ctr1;
		}
		break;

	case 62:
		digi_play("615t06", 1, 255, 75);
		break;

	case 65:
		_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "spleen");
		sendWSMessage_10000(1, _tt, _loop1, 1, 1, 75, _loop1, 1, 7, 1);
		sendWSMessage_1a0000(_tt, 13);
		break;

	case 75:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			terminateMachineAndNull(_tt);
			_untie = series_stream("615ALL2", 6, 0x100, 80);
			digi_play("615r06", 1, 255, 80);
		} else {
			++_ctr1;
		}
		break;

	case 80:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
				triggerMachineByHashCallback, "spleen");
			sendWSMessage_10000(1, _tt, _loop2, 1, 1, 85, _loop2, 1, 9, 1);
			sendWSMessage_1a0000(_tt, 13);
			digi_play("615t07", 1, 255, 85);
		} else {
			++_ctr1;
		}
		break;

	case 85:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			sendWSMessage_10000(1, _tt, _loop3, 1, 17, 87, _loop3, 18, 18, 1);
		} else {
			++_ctr1;
		}
		break;

	case 87:
		sendWSMessage_10000(1, _tt, _loop3, 19, 19, 90, _loop3, 19, 23, 1);
		sendWSMessage_1a0000(_tt, 13);
		digi_play("615r07", 1, 255, 90);
		break;

	case 90:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			sendWSMessage_10000(1, _tt, _loop4, 1, 13, 93, _loop4, 14, 14, 1);
		} else {
			++_ctr1;
		}
		break;

	case 93:
		sendWSMessage_10000(1, _tt, _loop4, 15, 15, 96, _loop4, 15, 20, 1);
		sendWSMessage_1a0000(_tt, 13);
		digi_play("615t08", 1, 255, 96);
		break;

	case 96:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			sendWSMessage_10000(1, _tt, _loop4, 21, 50, 98, _loop4, 50, 50, 1);
			sendWSMessage_190000(_tt, 7);
			digi_play("615t08a", 1, 255, 98);
		} else {
			++_ctr1;
		}
		break;

	case 98:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			terminateMachineAndNull(_tt);
			_untie = series_stream("615all3", 6, 0x100, 102);
			digi_play("615r08", 1, 255, 102);
		} else {
			++_ctr1;
		}
		break;

	case 102:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			_untie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
				triggerMachineByHashCallback, "spleen");
			sendWSMessage_10000(1, _untie, _loop5, 1, 1, 105, _loop5, 1, 5, 1);
			sendWSMessage_1a0000(_untie, 15);

			_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
				triggerMachineByHashCallback, "spleen");
			sendWSMessage_10000(1, _tt, _loop6, 1, 1, 105, _loop6, 1, 1, 1);
			digi_play("615r09", 1, 255, 105);
		} else {
			++_ctr1;
		}
		break;

	case 105:
		if (_ctr1 >= 2) {
			_ctr1 = 0;
			sendWSMessage_10000(1, _untie, _loop5, 1, 1, 108, _loop5, 1, 1, 1);
			sendWSMessage_10000(1, _tt, _loop6, 1, 1, 108, _loop6, 1, 5, 1);
			sendWSMessage_1a0000(_tt, 13);
			digi_play("615t09", 1, 255, 108);
		} else {
			++_ctr1;
		}
		break;

	case 108:
		if (_ctr1 >= 2) {
			_ctr1 = 0;
			sendWSMessage_10000(1, _untie, _loop5, 1, 1, 112, _loop5, 1, 5, 1);
			sendWSMessage_1a0000(_untie, 13);
			sendWSMessage_10000(1, _tt, _loop6, 1, 1, 112, _loop6, 1, 1, 1);
			digi_play("615r10", 1, 255, 112);
		} else {
			++_ctr1;
		}
		break;

	case 112:
		if (_ctr1 >= 2) {
			_ctr1 = 0;
			inv_move_object("RONGORONGO TABLET", 615);
			terminateMachineAndNull(_untie);
			terminateMachineAndNull(_tt);
			_G(flags)[V089] = 1;
			_untie = series_stream("615ALL4", 6, 0x100, 115);

			digi_play(_G(flags)[V288] ? "615r11" : "615r12", 1, 255, 115);
			_G(flags)[V288] = 1;
		} else {
			++_ctr1;
		}
		break;

	case 115:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			_untie = series_stream("615ALL5", 6, 0x100, 120);
			series_stream_break_on_frame(_untie, 30, 116);
			digi_play("615r13", 1, 255, 118);
		} else {
			++_ctr1;
		}
		break;

	case 116:
		series_stream_break_on_frame(_untie, 58, 150);
		series_set_frame_rate(_untie, 30000);
		break;

	case 118:
		series_set_frame_rate(_untie, 6);
		ws_OverrideCrunchTime(_untie);
		digi_play("615t10", 1, 255, 119);
		break;

	case 119:
		_val2 = 1;

		if (_val2) {
			series_set_frame_rate(_untie, 6);
			ws_OverrideCrunchTime(_untie);
		} else {
			kernel_timing_trigger(10, 151);
		}
		break;

	case 120:
		midi_play("love", 255, 0, -1, 949);
		_untie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "spleen");
		sendWSMessage_10000(1, _untie, _loop5, 1, 1, 125, _loop5, 1, 1, 1);
		sendWSMessage_190000(_untie, 15);
		sendWSMessage_1a0000(_untie, 15);

		_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "spleen");
		sendWSMessage_10000(1, _tt, _loop7, 1, 1, 125, _loop7, 1, 5, 1);
		sendWSMessage_1a0000(_tt, 13);
		digi_play("615t10a", 1, 255, 125);
		break;

	case 125:
		if (_ctr1 >= 2) {
			_ctr1 = 0;
			sendWSMessage_10000(1, _untie, _loop5, 1, 1, 127, _loop5, 1, 5, 1);
			sendWSMessage_1a0000(_untie, 13);
			sendWSMessage_10000(1, _tt, _loop7, 1, 1, 127, _loop7, 1, 1, 1);
			digi_play("615r14", 1, 255, 127);
		} else {
			++_ctr1;
		}
		break;

	case 127:
		if (_ctr1 >= 2) {
			_ctr1 = 0;
			sendWSMessage_10000(1, _untie, _loop5, 1, 1, 130, _loop5, 1, 1, 1);
			sendWSMessage_10000(1, _tt, _loop7, 1, 1, 130, _loop7, 1, 5, 1);
			sendWSMessage_1a0000(_tt, 13);
			digi_play("615t11", 1, 255, 130);
		} else {
			++_ctr1;
		}
		break;

	case 130:
		if (_ctr1 >= 2) {
			_ctr1 = 0;
			sendWSMessage_10000(1, _tt, _loop7, 1, 1, 133, _loop7, 1, 1, 1);
			_untie = series_stream("615PU03", 6, 256, -1);
			series_stream_break_on_frame(_untie, 5, 133);
		} else {
			++_ctr1;
		}
		break;

	case 133:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			series_stream_break_on_frame(_untie, 94, 135);
			digi_play("615r15", 1);
		} else {
			++_ctr1;
		}
		break;

	case 135:
		series_set_frame_rate(_untie, 400);
		kernel_timing_trigger(60, 140);
		break;

	case 140:
		disable_player_commands_and_fade_init(141);
		break;

	case 141:
		inv_move_object("SPARK PLUG TOOL", NOWHERE);
		_G(flags)[V203] = 9;
		_G(flags)[V191] = 0;
		_G(flags)[kTravelDest] = 4;
		_G(game).setRoom(495);
		break;

	case 150:
		if (!_val2) {
			series_set_frame_rate(_untie, 30000);
			kernel_timing_trigger(10, 151);
		}
		break;

	case 151:
		if (_val2) {
			series_set_frame_rate(_untie, 6);
			ws_OverrideCrunchTime(_untie);
		} else {
			kernel_timing_trigger(10, 151);
		}
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
