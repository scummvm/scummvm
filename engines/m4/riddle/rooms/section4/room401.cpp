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

#include "m4/riddle/rooms/section4/room401.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const char *const SAID[][2] = {
	{ "POSTCARD RACK", "401R08" },
	{ "POSTER",        "COM002" },
	{ "WINDOW",        "401R10" },
	{ "MAGAZINE",      "COM003" },
	{ "RAILING",       "401R22" },
	{ "DESK",          "401R23" },
	{ "FAN",           "401R24" },
	{ "BUSH",          "401R25" },
	{ "AGENT",         "401R12" },
	{ "CHAIR",         "COM023" },
	{ "SOFA",          "COM023" },
	{ "TELEPHONE",     "COM024" },
	{ "COAT",          "401R41" },
	{ nullptr, nullptr }
};

void Room401::init() {
	digi_preload("950_s01");

	if (_G(game).previous_room != KERNEL_RESTORING_GAME)
		_val1 = _val2 = 0;

	_val4 = 0;
	_digiName = nullptr;

	_401a01 = series_load("401A01");
	_401a02 = series_load("401A02");
	_401a03 = series_load("401A03");
	_401a04 = series_load("401A04");
	_401a05 = series_load("401A05");
	_401a06 = series_load("401A06");
	_401a01 = series_load("401A01");

	_rip1 = series_load("rip trek hand talk pos3");
	_rip2 = series_load("RIP TREK ARMS X POS3");
	_rip3 = series_load("RIP HEAD DOWN TALK OFF TD33");
	_rip4 = series_load("RIP TREK TALKER POS3");
	_401rp01 = series_load("401rp01");

	_agent = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x500, 0,
		triggerMachineByHashCallback, "agent");
	sendWSMessage_10000(1, _agent, _401a01, 1, 1, 100, _401a01, 1, 1, 0);
	_val5 = 1;
	_val6 = 0;

	if (_G(game).previous_room <= 495 && _G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = checkFlags(true);
		_G(flags)[V006]++;

		if (setItemsPlacedFlags()) {
			_digiName = getItemsPlacedDigi();
			++_val1;
		}

		player_set_commands_allowed(false);
		ws_demand_location(66, 266, 3);

		if (_val1 ||
			!_G(flags)[GLB_TEMP_14] ||
			(_G(flags)[V110] && !_G(flags)[GLB_TEMP_10]) ||
			(_G(flags)[GLB_TEMP_11] && player_been_here(407) && !_G(flags)[GLB_TEMP_13]) ||
			(_G(flags)[GLB_TEMP_12] && !_G(flags)[V091])
		) {
			ws_walk(346, 267, 0, 7, 3, 1);
		} else {
			ws_walk(158, 266, 0, 7, 3, 1);
		}
	}

	digi_play_loop("950_s01", 3, 140);
}

void Room401::daemon() {
	int val;

	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(true);
		break;

	case 7:
		setGlobals1(_rip1, 1, 7, 7, 7, 1);
		sendWSMessage_110000(9);
		digi_play("401r01", 1, 255, 11);
		break;

	case 8:
		sendWSMessage_140000(-1);
		break;

	case 11:
		_val6 = 5;
		digi_play(_G(player).been_here_before ? "401x08" : "401x01",
			1, 255, 13);
		break;

	case 13:
		kernel_timing_trigger(1, 100);
		_val6 = 0;
		kernel_timing_trigger(2, 999);
		break;

	case 100:
		kernel_trigger_dispatchx(kernel_trigger_create(101));
		break;

	case 101:
		switch (_val5) {
		case 1:
			switch (_val6) {
			case 0:
				val = imath_ranged_rand(1, 2);
				++_ctr1;

				if (imath_ranged_rand(20, 80) > _ctr1)
					val = 1;
				else
					_ctr1 = 0;

				if (val == 1) {
					sendWSMessage_10000(1, _agent, _401a01, 1, 1, 100, _401a01, 1, 1, 0);
				} else {
					sendWSMessage_10000(1, _agent, _401a01, 1, 3, 100, _401a01, 3, 3, 0);
					_val5 = 4;
				}
				break;

			case 7:
				sendWSMessage_10000(1, _agent, _401a02, 1, 57, 100, _401a01, 1, 1, 0);
				_val6 = 4;
				_ctr1 = 0;
				break;

			case 8:
				terminateMachineAndNull(_ripMach);
				sendWSMessage_10000(1, _agent, _401a04, 1, 62, 108, _401a01, 1, 1, 0);
				break;

			case 9:
				terminateMachineAndNull(_ripMach);
				sendWSMessage_10000(1, _agent, _401a05, 1, 53, 105, _401a01, 53, 53, 0);
				break;

			case 10:
				terminateMachineAndNull(_ripMach);
				sendWSMessage_10000(1, _agent, _401a03, 1, 47, 103, _401a03, 1, 1, 0);
				break;

			default:
				sendWSMessage_10000(1, _agent, _401a01, 1, 3, 100, _401a01, 3, 3, 0);
				_val5 = 4;
				_ctr1 = 0;
				break;
			}
			break;

		case 4:
			switch (_val6) {
			case 0:
				val = imath_ranged_rand(1, 3);
				++_ctr1;

				if (imath_ranged_rand(20, 80) > _ctr1)
					val = 1;
				else
					_ctr1 = 0;

				switch (val) {
				case 1:
					sendWSMessage_10000(1, _agent, _401a01, 3, 3, 100, _401a01, 3, 3, 0);
					break;

				case 2:
					sendWSMessage_10000(1, _agent, _401a01, 11, 28, 100, _401a01, 28, 28, 0);
					_val5 = 11;
					_ctr1 = 0;
					break;

				case 3:
					sendWSMessage_10000(1, _agent, _401a01, 3, 1, 100, _401a01, 1, 1, 0);
					_val5 = 1;
					_ctr1 = 0;
					break;

				default:
					break;
				}
				break;

			case 4:
				sendWSMessage_10000(1, _agent, _401a01, 3, 3, 100, _401a01, 3, 3, 0);
				break;

			case 5:
				sendWSMessage_10000(1, _agent, _401a01, 4, 4, -1, _401a01, 4, 10, 4);
				sendWSMessage_1a0000(_agent, 13);
				break;

			case 6:
				sendWSMessage_10000(1, _agent, _401a01, 29, 43, 100, _401a01, 3, 3, 0);
				sendWSMessage_190000(_agent, 13);
				_val6 = 4;
				break;

			case 11:
				sendWSMessage_10000(1, _agent, _401a01, 11, 28, 100, _401a01, 28, 28, 0);
				_val5 = 11;
				break;

			default:
				sendWSMessage_10000(1, _agent, _401a01, 3, 1, 100, _401a01, 1, 1, 0);
				_val5 = 1;
				_ctr1 = 0;
				break;
			}
			break;

		case 11:
			switch (_val6) {
			case 0:
				val = imath_ranged_rand(1, 2);
				++_ctr1;
				if (imath_ranged_rand(20, 80) < _ctr1)
					val = 1;
				else
					_ctr1 = 0;

				if (val == 1) {
					sendWSMessage_10000(1, _agent, _401a01, 28, 28, 100, _401a01, 28, 28, 0);
				} else {
					sendWSMessage_10000(1, _agent, _401a01, 28, 11, 100, _401a01, 3, 3, 0);
					_val5 = 4;
					_ctr1 = 0;
				}
				break;

			case 11:
				sendWSMessage_10000(1, _agent, _401a01, 28, 28, 100, _401a01, 28, 28, 0);
				break;

			default:
				sendWSMessage_10000(1, _agent, _401a01, 28, 11, 100, _401a01, 3, 3, 0);
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 102:
		_ripMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "rip");
		sendWSMessage_10000(1, _ripMach, _401rp01, 11, 11, 200, _401rp01, 11, 11, 0);
		_val7 = _val3 = 0;

		sendWSMessage_10000(1, _agent, _401a01, 1, 1, 100, _401a01, 1, 1, 0);
		_val6 = 4;
		_ctr1 = 0;

		_G(kernel).trigger_mode = KT_PARSE;
		digi_play(conv_sound_to_play(), 255, 1);
		_G(kernel).trigger_mode = KT_DAEMON;
		break;

	case 103:
		sendWSMessage_10000(1, _agent, _401a03, 48, 67, 102, _401a01, 1, 1, 0);
		digi_play(imath_ranged_rand(1, 2) == 1 ? "950_s09" : "950_s10", 2);
		break;

	case 104:
		series_show("401a06", 0);
		_ripMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "rip");
		sendWSMessage_10000(1, _ripMach, _401rp01, 11, 11, 200, _401rp01, 11, 11, 0);
		_val7 = _val3 = 0;

		sendWSMessage_10000(1, _agent, _401a01, 1, 1, 100, _401a01, 1, 1, 0);
		_val6 = 4;
		_ctr1 = 0;

		conv_resume();
		break;

	case 105:
		sendWSMessage_10000(1, _agent, _401a05, 53, 1, 106, _401a01, 1, 1, 0);
		break;

	case 106:
		_ripMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "rip");
		sendWSMessage_10000(1, _ripMach, _401rp01, 11, 11, 200, _401rp01, 11, 11, 0);
		_val7 = _val3 = 0;

		sendWSMessage_10000(1, _agent, _401a01, 1, 1, 100, _401a01, 1, 1, 0);
		_val6 = 0;
		_ctr1 = 0;

		inv_give_to_player("POMERANIAN MARKS");
		conv_resume();
		break;

	case 108:
		sendWSMessage_10000(1, _agent, _401a04, 63, 90, 104, _401a01, 1, 1, 0);
		digi_play("950_s35", 2);
		break;

	case 200:
		kernel_timing_trigger(1, 201);
		break;

	case 201:
		if (!_val7) {
			switch (_val3) {
			case 0:
				sendWSMessage_10000(1, _ripMach, _401rp01, 11, 11, 200, _401rp01, 11, 11, 0);
				break;
			case 1:
				sendWSMessage_10000(1, _ripMach, _401rp01, 12, 12, -1, _401rp01, 12, 19, 4);
				sendWSMessage_1a0000(_ripMach, 13);
				break;
			case 2:
				sendWSMessage_10000(1, _ripMach, _401rp01, 20, 36, 200, _401rp01, 11, 11, 0);
				sendWSMessage_190000(_ripMach, 13);
				_val3 = 0;
				break;
			case 3:
				sendWSMessage_10000(1, _ripMach, _401rp01, 11, 1, 202, _401rp01, 1, 1, 0);
				break;
			default:
				break;
			}
		}
		break;

	case 202:
		terminateMachineAndNull(_ripMach);
		ws_unhide_walker();
		player_set_commands_allowed(true);
		break;

	case 300:
		ws_hide_walker();
		_ripMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "rip");
		sendWSMessage_10000(1, _ripMach, _401rp01, 1, 11, 305, _401rp01, 11, 11, 0);
		_val6 = 5;

		digi_play((_val1 == 1) ? "401x02" : "401x03", 1);
		break;

	case 305:
		terminateMachineAndNull(_agent);
		sendWSMessage_10000(1, _ripMach, _401a02, 1, 47, 306, _401a02, 47, 47, 0);
		break;

	case 306:
		kernel_timing_trigger(1, 990);
		inv_give_to_player("MESSAGE LOG");
		break;

	case 320:
		sendWSMessage_10000(1, _ripMach, _401a02, 48, 57, 322, _401a02, 57, 57, 0);
		break;

	case 322:
		sendWSMessage_10000(1, _ripMach, _401rp01, 11, 1, 324, _401rp01, 1, 1, 0);
		_agent = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x600, 0,
			triggerMachineByHashCallback, "agent");
		sendWSMessage_10000(1, _agent, _401a01, 1, 1, 100, _401a01, 1, 1, 0);
		break;

	case 324:
	case 520:
	case 620:
	case 720:
	case 826:
		terminateMachineAndNull(_ripMach);
		ws_unhide_walker();
		kernel_timing_trigger(1, 1000);
		break;

	case 400:
		_G(flag1) = 0;
		setGlobals1(_rip4, 1, 1, 1, 5, 1);
		sendWSMessage_110000(405);
		_val6 = 4;
		digi_play("401r02", 1, 255, 405);
		break;

	case 405:
		if (_G(flag1) >= 1) {
			_G(flag1) = 0;
			sendWSMessage_140000(-1);
			_val6 = 5;
			digi_play("401x04", 1, 255, 407);
		} else {
			++_G(flag1);
		}
		break;

	case 407:
		_val6 = 4;
		kernel_timing_trigger(1, 100);

		setGlobals1(_rip4, 1, 1, 1, 5, 1);
		sendWSMessage_110000(410);
		digi_play("401r03", 1, 255, 410);
		break;

	case 410:
		if (_G(flag1) >= 1) {
			_G(flag1) = 0;
			sendWSMessage_140000(-1);
			_val6 = 5;
			digi_play("401x05", 1, 255, 412);
		} else {
			++_G(flag1);
		}
		break;

	case 412:
		_val6 = 0;
		kernel_timing_trigger(1, 100);
		_G(flags)[GLB_TEMP_14] = 1;
		kernel_timing_trigger(1, 1000);
		break;

	case 500:
		setGlobals1(_rip2, 1, 15, 15, 15, 0, 1, 2, 3, 5, 1, 2, 1, 1, 1);
		_val6 = 4;
		sendWSMessage_110000(503);
		digi_play("401r29", 1, 255, 505);
		break;

	case 503:
	case 603:
		sendWSMessage_29a0000(_rip3);
		sendWSMessage_120000(-1);
		break;

	case 505:
		sendWSMessage_130000(-1);
		digi_play("401x09", 1, 255, 507);
		break;

	case 507:
		_val6 = 4;
		kernel_timing_trigger(1, 100);
		sendWSMessage_120000(-1);
		digi_play("401r30", 1, 255, 510);
		break;

	case 510:
		sendWSMessage_110000(511);
		_val6 = 5;
		digi_play("401x10", 1, 255, 512);
		break;

	case 511:
		sendWSMessage_29a0000(_rip2);
		sendWSMessage_140000(514);
		break;

	case 512:
		sendWSMessage_10000(1, _agent, _401a01, 3, 3, 100, _401a01, 3, 3, 0);
		break;

	case 514:
		kernel_timing_trigger(60, 515);
		break;

	case 515:
		_ripMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "rip");
		sendWSMessage_10000(1, _ripMach, _401rp01, 1, 11, 516, _401rp01, 11, 11, 0);
		break;

	case 516:
		terminateMachineAndNull(_ripMach);
		sendWSMessage_10000(1, _agent, _401a04, 1, 63, 517, _401a04, 63, 63, 0);
		break;

	case 517:
		sendWSMessage_10000(1, _agent, _401a04, 64, 90, 518, _401a01, 1, 1, 0);
		digi_play("950_s35", 2);
		break;

	case 518:
		series_show("401a06", 0x600, 16);
		sendWSMessage_10000(1, _agent, _401a01, 1, 1, 100, _401a01, 1, 1, 0);
		_G(flags)[GLB_TEMP_10] = 1;
		_G(flags)[V366] = 1;

		_ripMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "rip");
		sendWSMessage_10000(1, _ripMach, _401rp01, 11, 1, 520, _401rp01, 1, 1, 0);
		_val6 = 0;
		_ctr1 = 0;
		break;

	case 600:
		setGlobals1(_rip2, 1, 15, 15, 15, 0, 1, 2, 3, 5, 1, 2, 1, 1, 1);
		_val6 = 4;
		sendWSMessage_110000(603);
		digi_play("401r32", 1, 255, 605);
		break;

	case 605:
		sendWSMessage_110000(612);
		break;

	case 612:
		sendWSMessage_29a0000(_rip2);
		sendWSMessage_140000(613);
		break;

	case 613:
		ws_hide_walker();
		_ripMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "rip");
		sendWSMessage_10000(1, _agent, _401rp01, 1, 11, 616, _401rp01, 11, 11, 0);
		break;

	case 616:
		terminateMachineAndNull(_agent);
		sendWSMessage_10000(1, _ripMach, _401a04, 1, 63, 617, _401a04, 63, 63, 0);
		break;

	case 617:
		sendWSMessage_10000(1, _ripMach, _401a04, 1, 90, 618, _401a01, 1, 1, 0);
		digi_play("950_s35", 2);
		break;

	case 618:
		series_show("401a06", 0x600, 16);
		_agent = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x600, 0,
			triggerMachineByHashCallback, "agent");
		sendWSMessage_10000(1, _agent, _401a01, 1, 1, 100, _401a01, 1, 1, 0);
		sendWSMessage_10000(1, _ripMach, _401rp01, 11, 1, 620, _401rp01, 1, 1, 0);
		_G(flags)[GLB_TEMP_13] = 1;
		_val6 = 0;
		_ctr1 = 0;
		break;

	case 700:
		setGlobals1(_rip4, 1, 1, 1, 5, 1);
		sendWSMessage_110000(705);
		_val6 = 4;
		digi_play(_G(flags)[GLB_TEMP_13] ? "401r33" : "401r34", 1, 255, 705);
		break;

	case 705:
		if (_G(flag1) >= 1) {
			_G(flag1) = 0;
			sendWSMessage_140000(-1);
			_val6 = 5;
			digi_play("401x12", 1, 255, 707);
		} else {
			++_G(flag1);
		}
		break;

	case 707:
		_val6 = 4;
		kernel_timing_trigger(1, 100);
		setGlobals1(_rip4, 1, 1, 1, 5, 1);
		sendWSMessage_110000(710);
		digi_play("401r35", 1, 255, 710);
		_G(flags)[V373] = 1;
		break;

	case 710:
		if (_G(flag1) >= 1) {
			_G(flag1) = 0;
			sendWSMessage_140000(-1);
			_val6 = 5;
			digi_play("401x13", 1, 255, 712);
		} else {
			++_G(flag1);
		}
		break;

	case 711:
		sendWSMessage_10000(1, _agent, _401a01, 3, 3, -1, _401a01, 3, 3, 0);
		kernel_timing_trigger(1, 715);
		break;

	case 715:
		ws_hide_walker();
		_ripMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "rip");
		sendWSMessage_10000(1, _ripMach, _401a04, 1, 11, 716, _401rp01, 11, 11, 0);
		break;

	case 716:
		terminateMachineAndNull(_ripMach);
		sendWSMessage_10000(1, _agent, _401a03, 1, 67, 718, _401a01, 1, 1, 0);
		break;

	case 718:
		sendWSMessage_10000(1, _agent, _401a01, 1, 1, 100, _401a01, 1, 1, 0);
		inv_move_object("TURTLE", 305);
		inv_move_object("TURTLE TREATS", 305);

		_ripMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "rip");
		sendWSMessage_10000(1, _ripMach, _401rp01, 11, 1, 720, _401rp01, 1, 1, 0);
		_ctr1 = 0;
		break;

	case 800:
		sendWSMessage_10000(1, _ripMach, _401a02, 48, 57, 822, _401a02, 57, 57, 0);
		break;

	case 822:
		sendWSMessage_10000(1, _ripMach, _401a03, 67, 1, 824, _401rp01, 11, 11, 0);
		break;

	case 824:
		sendWSMessage_10000(1, _ripMach, _401rp01, 11, 1, 826, _401rp01, 1, 1, 0);
		inv_give_to_player("ROMANOV EMERALD");
		_agent = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x600, 0,
			triggerMachineByHashCallback, "agent");
		sendWSMessage_10000(1, _agent, _401a01, 1, 1, 100, _401a01, 1, 1, 0);
		_val6 = 0;
		_ctr1 = 0;
		break;

	case 990:
		_digiName1 = nullptr;
		_digiName2 = nullptr;
		_digiName3 = nullptr;
		_digiName4 = nullptr;

		if (_val1 <= 0) {
			kernel_timing_trigger(1, 320);
		} else if (_digiName) {
			_digiName1 = _digiName;
			_digiName = nullptr;
			kernel_timing_trigger(1, 991);
		} else if (_G(flags)[V364] == 1) {
			_digiName1 = "201R26";
			_G(flags)[V364] = 0;
			kernel_timing_trigger(1, 991);
		} else if (_G(flags)[V365] == 1) {
			_digiName1 = "201R61";
			_G(flags)[V365] = 0;
			kernel_timing_trigger(1, 991);
		} else if (_G(flags)[V366] == 1) {
			_digiName1 = "401R31";
			_G(flags)[V366] = 0;
			kernel_timing_trigger(1, 991);
		} else if (_G(flags)[V371] == 1) {
			_digiName = "501R03C";
			_G(flags)[V371] = 0;
			kernel_timing_trigger(1, 991);
		} else if (_G(flags)[V372] == 1) {
			_digiName1 = "701R39";
			_digiName2 = "701R39A";
			_G(flags)[V372] = 0;
			kernel_timing_trigger(1, 991);
		} else if (_G(flags)[V367] == 1) {
			_digiName1 = "401R37";
			_G(flags)[V367] = 0;
			_val2 = 1;
			kernel_timing_trigger(1, 991);
		} else if (_G(flags)[V368] == 1) {
			_digiName1 = "401R38";
			_G(flags)[V368] = 0;
			_val2 = 1;
			kernel_timing_trigger(1, 991);
		} else if (_G(flags)[V369] == 1) {
			_digiName1 = "401R39";
			_G(flags)[V369] = 0;
			_val2 = 1;
			kernel_timing_trigger(1, 991);
		}
		break;

	case 991:
		if (_digiName1) {
			digi_play(_digiName1, 1, 255, 993);
		} else {
			kernel_timing_trigger(1, 993);
		}
		break;

	case 993:
		if (_digiName2) {
			digi_play(_digiName2, 1, 255, 994);
		} else {
			kernel_timing_trigger(1, 994);
		}
		break;

	case 994:
		if (_digiName3) {
			digi_play(_digiName3, 1, 255, 995);
		} else {
			kernel_timing_trigger(1, 995);
		}
		break;

	case 995:
		if (_digiName4) {
			digi_play(_digiName4, 1, 255, 996);
		} else {
			kernel_timing_trigger(1, 996);
		}
		break;

	case 996:
		if (_val2) {
			_val2 = 0;
			kernel_timing_trigger(1, 800);
		} else {
			if (--_val1 == 0) {
				kernel_timing_trigger(1, 320);
			} else {
				kernel_timing_trigger(1, 800);
			}
		}
		break;

	case 999:
		kernel_timing_trigger(1, _val1 ? 300 : 1000);
		break;

	case 1000:
		if (_G(flags)[GLB_TEMP_14]) {
			kernel_timing_trigger(30, 400);
		} else if (_G(flags)[V110] && !_G(flags)[GLB_TEMP_10]) {
			kernel_timing_trigger(30, 500);
		} else if (_G(flags)[GLB_TEMP_11] && player_been_here(407) &&
				!_G(flags)[GLB_TEMP_13]) {
			kernel_timing_trigger(30, 600);
		} else if (_G(flags)[GLB_TEMP_12] && !_G(flags)[V091]) {
			kernel_timing_trigger(30, 700);
		} else {
			player_set_commands_allowed(true);
		}
		break;

	default:
		break;
	}
}

void Room401::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said("gear");

	if (player_said("conv401a")) {
		conv401a();
	} else if (_G(kernel).trigger == 747) {
		if (_val4) {
			switch (_val4) {
			case 1:
				_G(flags)[kTravelDest] = 1;
				break;
			case 2:
				_G(flags)[kTravelDest] = 0;
				break;
			case 3:
				_G(flags)[kTravelDest] = 4;
				break;
			case 4:
				_G(flags)[kTravelDest] = 3;
				break;
			default:
				break;
			}

			disable_player_commands_and_fade_init(748);
		}

		_val6 = 0;
		_val3 = 3;
	} else if (_G(kernel).trigger == 748) {
		_G(flags)[V322] = 0;
		_G(game).setRoom(495);
	} else if (player_said("talk to", "agent")) {
		ws_hide_walker();
		_G(kernel).trigger_mode = KT_DAEMON;
		_ripMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "rip");
		sendWSMessage_10000(1, _ripMach, _401rp01, 1, 11, 200, _401rp01, 11, 11, 0);

		_val7 = 0;
		_val3 = 0;
		_G(kernel).trigger_mode = KT_PARSE;

		conv_load("conv401a", 10, 10, 747);
		conv_export_value_curr(_G(flags)[V088] < 3 ? 0 : 1, 1);
		conv_export_pointer_curr(&_val4, 2);
		conv_export_value_curr(inv_player_has("POMERANIAN MARKS") ? 1 : 0, 3);
		conv_export_value_curr(_G(flags)[V110], 4);
		conv_export_value_curr(getItemCount() ? 1 : 0, 5);

		#define EXPORT(INDEX, NAME) conv_export_value_curr(inv_player_has(NAME) ? 1 : 0, INDEX)
		EXPORT(6, "CRYSTAL SKULL");
		EXPORT(7, "STICK AND SHELL MAP");
		EXPORT(8, "WHEELED TOY");
		EXPORT(9, "REBUS AMULET");
		EXPORT(10, "SHRUNKEN HEAD");
		EXPORT(11, "SILVER BUTTERFLY");
		EXPORT(12, "POSTAGE STAMP");
		EXPORT(13, "GERMAN BANKNOTE");
		EXPORT(14, "WHALE BONE HORN");
		EXPORT(15, "CHISEL");
		EXPORT(16, "INCENSE BURNER");
		EXPORT(17, "ROMANOV EMERALD");
		#undef EXPORT

		conv_play();
		_G(kernel).trigger_mode = KT_DAEMON;
		_val6 = 4;
	} else if (lookFlag && player_said(" ")) {
		digi_play("COM001", 1, 255, -1, 997);
	} else if (takeFlag && player_said("BEER STEIN")) {
		digi_play("203r58", 1);
	} else if (lookFlag && player_said("BEER STEIN")) {
		if (_G(flags)[GLB_TEMP_9]) {
			digi_play("401R07", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				digi_play("401R05", 1, 255, 3);
				kernel_timing_trigger(1, 230);
				break;
			case 1:
				player_update_info();
				ws_walk(_G(player_info).x + 1, _G(player_info).y,
					nullptr, 2, 3);
				_val6 = 4;
				break;
			case 2:
				setGlobals1(_rip1, 1, 7, 8, 9, 1);
				sendWSMessage_110000(-1);
				break;
			case 3:
				sendWSMessage_140000(-1);
				_val6 = 5;
				digi_play("401x06", 1, 255, 4);
				break;
			case 4:
				setGlobals1(_rip1, 1, 7, 8, 9, 1);
				sendWSMessage_110000(-1);
				_val6 = 4;

				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 100);
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play("401r06", 1, 255, 5);
				break;
			case 5:
				sendWSMessage_140000(-1);
				_val6 = 5;
				digi_play("401x07", 1, 255, 6);
				break;
			case 6:
				_val6 = 0;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 100);
				_G(kernel).trigger_mode = KT_PARSE;
				_G(flags)[GLB_TEMP_9] = 1;
				player_set_commands_allowed(true);
				break;
			default:
				break;
			}
		}
	} else if (lookFlag && _G(walker).ripley_said(SAID)) {
		// No implementation
	} else if (useFlag && player_said_any("SOFA", "CHAIR")) {
		digi_play("COM025", 1, 255, -1, 997);
	} else if (useFlag && player_said("TELEPHONE")) {
		digi_play("COM026", 1, 255, -1, 997);
	} else if ((useFlag || takeFlag) && player_said("COAT")) {
		digi_play("203R58", 1);
	} else if (takeFlag && player_said("POSTCARD RACK")) {
		digi_play("COM004", 1, 255, -1, 997);
	} else if (takeFlag && player_said("MAGAZINE")) {
		digi_play("COM005", 1, 255, -1, 997);
	} else if (player_said("exit")) {
		switch (_G(kernel).trigger) {
		case -1:
			disable_player_commands_and_fade_init(3);
			break;
		case 3:
			_G(game).setRoom(410);
			break;
		default:
			break;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room401::conv401a() {
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();
	const char *sound = conv_sound_to_play();

	if (_G(kernel).trigger == 1) {
		if (who <= 0) {
			if (node == 3)
				return;
			
			_val6 = 4;
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 100);
			_G(kernel).trigger_mode = KT_PARSE;
		} else if (who == 1) {
			if (node == 1 && entry == 1) {

			} else {
				conv_set_box_xy(10, 10);
			}

			if (node == 1 && entry == 3) {
				_val6 = 9;
				_val3 = 0;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 200);
				_G(kernel).trigger_mode = KT_PARSE;
				return;
			}

			_val3 = 0;
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 200);
			_G(kernel).trigger_mode = KT_PARSE;
		}

		conv_resume();

	} else if (!sound) {
		conv_resume();

	} else {
		if (who <= 0) {
			if (node == 3) {
				_val6 = 8;
				_G(flags)[GLB_TEMP_8] = 0;
			} else if (node == 2 && entry == 0) {
				_val6 = 10;
				return;
			} else {
				_val6 = 5;
			}
		} else if (who == 1) {
#define ITEM(NODE, ENTRY, ITEM) if (node == NODE && entry == ENTRY) inv_move_object(ITEM, 305)
			ITEM(11, 0, "CRYSTAL SKULL");
			ITEM(11, 1, "STICK AND SHELL MAP");
			ITEM(11, 2, "WHEELED TOY");
			ITEM(11, 3, "REBUS AMULET");
			ITEM(11, 4, "SHRUNKEN HEAD");
			ITEM(11, 5, "SILVER BUTTERFLY");
			ITEM(11, 6, "POSTAGE STAMP");
			ITEM(11, 7, "GERMAN BAKNOTE");
			ITEM(11, 8, "WHALE BONE HORN");
			ITEM(11, 9, "CHISEL");
			ITEM(11, 10, "INCENSE BURNER");
			ITEM(11, 11, "ROMANOV EMERALD");
#undef ITEM

			if (node != 11)
				_val3 = 1;
		}

		digi_play(sound, 1, 255, 1);
	}
}

int Room401::getItemCount() const {
	int count = 0;
	static const char *NAMES[12] = {
		"SHRUNKEN HEAD", "INCENSE BURNER", "CRYSTAL SKULL",
		"WHALE BONE HORN", "WHEELED TOY", "SILVER BUTTERFLY",
		"REBUS AMULET", "CHISEL", "GERMAN BANKNOTE",
		"POSTAGE STAMP", "STICK AND SHELL MAP", "ROMANOV EMERALD"
	};
	for (int i = 0; i < 12; ++i) {
		if (inv_player_has(NAMES[i]))
			++count;
	}

	return count;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
