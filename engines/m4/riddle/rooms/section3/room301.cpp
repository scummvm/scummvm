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

#include "m4/riddle/rooms/section3/room301.h"

#include "m4/adv_r/other.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room301::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	_G(player).walker_in_this_scene = true;
	
	LoadWSAssets("OTHER SCRIPT");
}

void Room301::init() {
	_G(flags)[V000] = 1;
	digi_preload("950_s01");

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_travelDest = 0;
		_soundName = nullptr;
		_nextSound = nullptr;
		_ripleyTrigger = -1;
		_val6_triggerMode = KT_DAEMON;
		_georgeTriggerMode = KT_DAEMON;
		_georgeRandomActionCounter = 0;
		_showWalkerFlag = false;
		_msgRipleyFlag = false;
		_convResumeFlag = false;
		_val12 = 0;
	}

	_ripTrekHandTalk3 = series_load("rip trek hand talk pos3");
	_ripTrekTalker3 = series_load("RIP TREK TALKER POS3");
	_ripTrekTravel = series_load("rip trek travel disp");
	_agentStander = series_load("agent stander / talk disp");
	_agentCheckingList = series_load("agent checking list on desk");
	_agentSlidesPaper = series_load("AGENT SLIDES PAPER TO RIP");
	_agentTakesMoney = series_load("agent takes money from rip");
	_agentTakesTelegram = series_load("agent takes telegram from slot");
	_agentTalk = series_load("agent animated talk disp");
	_agentSalutes = series_load("agent salutes rip");
	_georgeMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, false,
		triggerMachineByHashCallback, "guy behind desk");
	_georgeMode = 0;
	_georgeShould = 0;

	sendWSMessage(1, _georgeMach, _agentStander, 1, 1, 10, _agentStander, 1, 1, 0);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		if (inv_object_in_scene("ROMANOV EMERALD", 305)) {
			setFlag45();
			inv_move_object("ROMANOV EMERALD", NOWHERE);
		}

		_val12 = checkFlags(false);
		player_set_commands_allowed(false);
		ws_demand_location(_G(my_walker), 55, 270, 3);

		if (_val12 || (!player_been_here(401) && _G(flags)[V092] && !_G(flags)[V093])) {
			ws_walk(_G(my_walker), 352, 269, nullptr, 60, 3);
		} else if (!player_been_here(301)) {
			digi_preload("301r37");
			digi_play("301r37", 1, 255, 51);
			ws_walk(_G(my_walker), 165, 270, nullptr, -1, 3);
		} else {
			ws_walk(_G(my_walker), 165, 270, nullptr, 51, 3);
		}
	}

	digi_play_loop("950_s01", 3, 140);
}

void Room301::daemon() {
	int frame, val;

	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(true);
		break;

	case 9:
		_georgeMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, false,
			triggerMachineByHashCallback, "guy behind desk");
		_georgeMode = _georgeShould = 0;
		sendWSMessage_10000(1, _georgeMach, _agentStander, 1,
			1, 10, _agentStander, 1, 1, 0);
		break;

	case 10:
		if (_georgeMode == 0 && _georgeShould == 0) {
			if (_ripleyTrigger != -1) {
				kernel_trigger_dispatchx(_ripleyTrigger);
				_ripleyTrigger = -1;

				if (_showWalkerFlag) {
					ws_unhide_walker(_G(my_walker));
					_showWalkerFlag = false;
				}
			}

			if (_convResumeFlag) {
				conv_resume();
				_convResumeFlag = false;
			}

			if (_msgRipleyFlag) {
				sendWSMessage_80000(_ripleyMach);
				sendWSMessage_10000(1, _ripleyMach, _ripTrekTravel, 10,
					10, 20, _ripTrekTravel, 10, 10, 0);

				_msgRipleyFlag = false;
			}
		}

		kernel_timing_trigger(1, 11);
		break;

	case 11:
		switch (_georgeMode) {
		case 0:
			switch (_georgeShould) {
			case 0:
				val = imath_ranged_rand(1, 3);
				++_georgeRandomActionCounter;
				if (imath_ranged_rand(10, 40) <= _georgeRandomActionCounter)
					_georgeRandomActionCounter = 0;
				else
					val = 1;

				switch (val) {
				case 1:
					sendWSMessage_10000(1, _georgeMach, _agentStander, 1, 1, 10,
						_agentStander, 1, 1, 0);
					break;
				case 2:
					sendWSMessage_10000(1, _georgeMach, _agentStander, 1, 7, 10,
						_agentStander, 7, 7, 0);
					_georgeMode = _georgeShould = 3;
					break;
				case 3:
					sendWSMessage_10000(1, _georgeMach, _agentTalk, 1, 6, 10,
						_agentTalk, 6, 6, 0);
					_georgeMode = _georgeShould = 7;
					_georgeRandomActionCounter = 0;

					digi_play((imath_ranged_rand(1, 2) == 1) ? "950_s06" : "950_s07",
						2, 200);
					break;
				default:
					break;
				}
				break;

			case 1:
			case 2:
				sendWSMessage_10000(1, _georgeMach, _agentCheckingList,
									1, 26, 10, _agentCheckingList, 27, 27, 0);
				_georgeMode = 1;
				
				break;

			case 3:
			case 4:
			case 5:
			case 6:
				sendWSMessage_10000(1, _georgeMach, _agentStander, 1, 7, 10,
					_agentStander, 7, 7, 0);
				_georgeMode = 3;
				_georgeRandomActionCounter = 0;
				break;

			case 7:
			case 8:
			case 9:
				sendWSMessage_10000(1, _georgeMach, _agentTalk, 1, 6, 10,
					_agentTalk, 6, 6, 0);
				_georgeMode = 7;
				_georgeRandomActionCounter = 0;
				break;

			case 10:
				sendWSMessage_10000(1, _georgeMach, _agentSalutes, 1, 28, 10,
					_agentStander, 1, 1, 0);
				_georgeMode = _georgeShould = 0;
				_georgeRandomActionCounter = 0;
				break;

			case 12:
				// George takes payment from Ripley
				terminateMachineAndNull(_ripleyMach);
				_convResumeFlag = true;
				sendWSMessage_10000(1, _georgeMach, _agentTakesMoney, 1, 52, 13,
					_agentStander, 1, 1, 0);
				_georgeMode = _georgeShould = 0;
				_georgeRandomActionCounter = 0;
				_ripleyShould = 0;
				break;

			case 13:
				terminateMachineAndNull(_ripleyMach);
				sendWSMessage_10000(1, _georgeMach, _agentSlidesPaper, 1, 49, 14,
					_agentStander, 1, 1, 0);
				break;

			case 15:
				terminateMachineAndNull(_ripleyMach);
				sendWSMessage_10000(1, _georgeMach, _agentSlidesPaper, 1, 49, 25,
					_agentStander, 1, 1, 0);
				break;

			default:
				break;
			}
			break;

		case 1:
			switch (_georgeShould) {
			case 1:
				sendWSMessage_10000(1, _georgeMach, _agentCheckingList, 27, 27, 10,
					_agentCheckingList, 27, 27, 0);
				break;

			case 2:
				if (_soundName) {
					_G(kernel).trigger_mode = KT_PARSE;
					digi_play(_soundName, 1, 255, _georgeTrigger);
					_soundName = nullptr;
					_G(kernel).trigger_mode = KT_DAEMON;
				}

				frame = imath_ranged_rand(27, 31);
				sendWSMessage_10000(1, _georgeMach, _agentCheckingList, frame, frame, 10,
					_agentCheckingList, frame, frame, 0);
				break;

			default:
				sendWSMessage_10000(1, _georgeMach, _agentCheckingList, 27, 22, 100,
					_agentCheckingList, 21, 21, 0);
				_georgeRandomActionCounter = 0;
				break;
			}
			break;

		case 3:
			switch (_georgeShould) {
			case 3:
				val = imath_ranged_rand(1, 2);
				++_georgeRandomActionCounter;

				if (imath_ranged_rand(10, 40) <= _georgeRandomActionCounter)
					_georgeRandomActionCounter = 0;
				else
					val = 1;

				if (val == 1) {
					sendWSMessage_10000(1, _georgeMach, _agentStander, 7, 7, 10,
						_agentStander, 7, 7, 0);
				} else if (val == 2) {
					sendWSMessage_10000(1, _georgeMach, _agentStander, 7, 1, 10,
						_agentStander, 1, 1, 0);
					_georgeMode = _georgeShould = 0;
					_georgeRandomActionCounter = 0;
				}
				break;

			case 4:
			case 5:
			case 6:
				sendWSMessage_10000(1, _georgeMach, _agentStander, 8, 12, 10,
					_agentStander, 12, 12, 0);
				_georgeMode = 4;
				break;
			default:
				sendWSMessage_10000(1, _georgeMach, _agentStander, 7, 1, 10,
					_agentStander, 1, 1, 0);
				_georgeMode = 0;
				_georgeRandomActionCounter = 0;
				break;
			}
			break;

		case 4:
			switch (_georgeShould) {
			case 4:
				sendWSMessage_10000(1, _georgeMach, _agentStander, 12,
					12, 10, _agentStander, 12, 12, 0);
				break;
			case 5:
				if (_soundName) {
					_G(kernel).trigger_mode = _georgeTriggerMode;
					digi_play(_soundName, 1, 255, _georgeTrigger);
					_soundName = nullptr;
					_georgeTriggerMode = KT_DAEMON;
					_G(kernel).trigger_mode = KT_DAEMON;
				}

				_G(kernel).trigger_mode = _val6_triggerMode;
				frame = imath_ranged_rand(12, 17);
				sendWSMessage_10000(1, _georgeMach, _agentStander, frame, frame,
					10, _agentStander, frame, frame, 1);
				sendWSMessage_190000(_georgeMach, 13);
				sendWSMessage_1a0000(_georgeMach, 13);
				_G(kernel).trigger_mode = KT_DAEMON;
				_val6_triggerMode = KT_DAEMON;
				break;
			case 6:
				if (_soundName) {
					_G(kernel).trigger_mode = _georgeTriggerMode;
					digi_play(_soundName, 1, 255, _georgeTrigger);
					_soundName = nullptr;
					_G(kernel).trigger_mode = KT_DAEMON;
				}

				frame = imath_ranged_rand(18, 21);
				sendWSMessage_10000(1, _georgeMach, _agentStander, frame, frame,
					10, _agentStander, frame, frame, 1);
				sendWSMessage_190000(_georgeMach, 13);
				sendWSMessage_1a0000(_georgeMach, 13);
				break;
			default:
				sendWSMessage(1, _georgeMach, _agentStander, 12,
					8, 10, _agentStander, 7, 7, 0);
				_georgeMode = 3;
				_georgeRandomActionCounter = 0;
				break;
			}
			break;

		case 7:
			switch (_georgeShould) {
			case 7:
				val = imath_ranged_rand(1, 2);
				if (imath_ranged_rand(10, 40) <= _georgeRandomActionCounter)
					_georgeRandomActionCounter = 0;
				else
					val = 1;

				if (val == 1) {
					sendWSMessage_10000(1, _georgeMach, _agentTalk, 6, 6, 10,
						_agentTalk, 6, 6, 0);
				} else if (val == 2) {
					sendWSMessage_10000(1, _georgeMach, _agentTalk, 6, 1, 10,
						_agentStander, 1, 1, 0);
					_georgeMode = _georgeShould = 0;
					_georgeRandomActionCounter = 0;

					digi_play((imath_ranged_rand(1, 2) == 1) ? "950_s06" : "950_s07",
						2, 200);
				}
				break;

			case 8:
			case 9:
				sendWSMessage_10000(1, _georgeMach, _agentTalk, 6, 15, 10,
					_agentTalk, 15, 15, 0);
				_georgeMode = 8;
				
				break;

			default:
				sendWSMessage_10000(1, _georgeMach, _agentTalk, 6, 1, 10,
					_agentStander, 1, 1, 0);
				_georgeMode = 0;
				_georgeRandomActionCounter = 0;
				
				break;
			}
			break;

		case 8:
			switch (_georgeShould) {
			case 8:
				sendWSMessage_10000(1, _georgeMach, _agentTalk, 15, 15, 10,
					_agentTalk, 15, 15, 0);
				break;
				
			case 9:
				if (_soundName) {
					digi_play(_soundName, 1, 255, _georgeTrigger);
					_soundName = nullptr;
				}

				sendWSMessage_10000(1, _georgeMach, _agentTalk, 16, 32, 10,
					_agentTalk, 15, 15, 0);
				_georgeMode = 8;
				
				break;
			default:
				sendWSMessage_10000(1, _georgeMach, _agentTalk, 15, 6, 10,
					_agentTalk, 6, 6, 0);
				_georgeMode = 7;
				_georgeRandomActionCounter = 0;
				
				break;
			}
			break;

		case 14:
			if (_georgeShould == 14) {
				if (_soundName) {
					_G(kernel).trigger_mode = KT_PARSE;
					digi_play(_soundName, 1, 255, _georgeTrigger);
					_soundName = nullptr;
					_G(kernel).trigger_mode = KT_DAEMON;
				}

				sendWSMessage(1, _georgeMach, _agentSlidesPaper, 54, 54, 10,
					_agentSlidesPaper, 54, 55, 1);

			} else {
				sendWSMessage_10000(1, _georgeMach, _agentSlidesPaper, 57, 76, 10,
					_agentSlidesPaper, 76, 76, 0);
				_georgeMode = _georgeShould = 0;
				_georgeRandomActionCounter = 0;
				_msgRipleyFlag = true;
				_convResumeFlag = true;
			}
			break;
			
		default:
			break;
		}
		break;

	case 12:
		_ripleyMach = TriggerMachineByHash(triggerMachineByHashCallback, "rip");
		sendWSMessage_10000(1, _ripleyMach, _ripTrekTravel, 10, 10, 20,
			_ripTrekTravel, 10, 10, 0);
		_ripleyShould = _ripleyMode = 0;
		kernel_timing_trigger(10, 10);
		_georgeShould = 4;
		_georgeRandomActionCounter = 0;
		conv_resume();
		break;

	case 13:
		_ripleyMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, false,
			triggerMachineByHashCallback, "rip in conv");
		sendWSMessage_10000(1, _ripleyMach, _ripTrekTravel, 10, 10, 20,
			_ripTrekTravel, 10, 10, 0);
		_ripleyShould = _ripleyMode = 0;
		kernel_timing_trigger(10, 10);
		break;

	case 14:
		sendWSMessage_10000(1, _georgeMach, _agentSlidesPaper, 50, 63, 15,
			_agentStander, 1, 1, 0);
		digi_play("950_s35", 2);
		break;

	case 15:
		_G(flags)[V008] = 1;
		sendWSMessage_10000(1, _georgeMach, _agentSlidesPaper, 49, 1, 12,
			_agentStander, 1, 1, 0);
		break;

	case 20:
		if (!_ripleyMode && _ripleyShould == 0 && _ripleyTrigger != -1) {
			kernel_trigger_dispatchx(_ripleyTrigger);
			_ripleyTrigger = -1;

			if (_showWalkerFlag) {
				ws_unhide_walker();
				_showWalkerFlag = false;
			}
			if (_msgRipleyFlag) {
				sendWSMessage_80000(_ripleyMach);
				_msgRipleyFlag = false;
			}
		}

		kernel_timing_trigger(1, 21);
		break;

	case 21:
		if (!_ripleyMode) {
			switch (_ripleyShould) {
			case 0:
				sendWSMessage_10000(1, _ripleyMach, _ripTrekTravel,
					10, 10, 20, _ripTrekTravel, 10, 10, 0);
				break;

			case 1:
				frame = imath_ranged_rand(11, 19);
				sendWSMessage_10000(1, _ripleyMach, _ripTrekTravel,
					frame, frame, 20, _ripTrekTravel, frame, frame, 0);
				sendWSMessage_190000(_ripleyMach, 13);
				sendWSMessage_1a0000(_ripleyMach, 13);
				break;

			case 2:
				sendWSMessage_10000(1, _ripleyMach, _ripTrekTravel,
					20, 26, 20, _ripTrekTravel, 10, 10, 0);
				break;

			case 3:
				sendWSMessage_10000(1, _ripleyMach, _ripTrekTravel,
					37, 50, 20, _ripTrekTravel, 10, 10, 0);
				break;

			case 4:
				sendWSMessage_10000(1, _ripleyMach, _ripTrekTravel,
					9, 1, 22, _ripTrekTravel, 1, 1, 0);
				break;

			case 5:
				_G(kernel).trigger_mode = KT_PARSE;
				conv_load("conv301a", 10, 10, 747, player_commands_allowed());
				_travelDest = 0;

				conv_export_value(conv_get_handle(), _G(flags)[V088] >= 3 ? 1 : 0, 0);
				conv_export_value(conv_get_handle(), _G(flags)[V056], 1);
				conv_export_value(conv_get_handle(), _G(flags)[V110], 2);
				conv_export_pointer(conv_get_handle(), &_travelDest, 5);

				if (player_been_here(201))
					conv_export_value(conv_get_handle(), 2, 6);

				conv_play(conv_get_handle());
				_ripleyShould = 0;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 20);
				break;

			default:
				break;
			}
		}
		break;

	case 22:
		terminateMachineAndNull(_ripleyMach);
		terminateMachineAndNull(_shadowMach);
		ws_unhide_walker();
		player_set_commands_allowed(true);
		break;

	case 25:
		sendWSMessage_10000(1, _georgeMach, _agentSlidesPaper, 50, 64, 26,
			_agentSlidesPaper, 64, 64, 0);
		digi_play("950_s35", 2);
		break;

	case 26:
		sendWSMessage_10000(1, _georgeMach, _agentSlidesPaper, 49, 1, 27,
			_agentStander, 1, 1, 0);
		break;

	case 27:
		sendWSMessage_10000(1, _georgeMach, _agentTakesMoney, 52, 9, 28,
			_agentTakesMoney, 9, 9, 0);
		_georgeShould = 4;
		_georgeMode = 4;
		_georgeRandomActionCounter = 0;
		_ripleyShould = 0;
		break;

	case 28:
		conv_resume();
		kernel_timing_trigger(1, 13);
		break;

	case 51:
		digi_unload("301r37");
		setGlobals1(_ripTrekHandTalk3, 1, 7, 7, 7, 1);
		sendWSMessage_110000(52);
		digi_play("301r01", 1, 255, 53);
		break;

	case 52:
		sendWSMessage_140000(-1);
		break;

	case 53:
		_georgeShould = 6;
		_soundName = "301a01";
		_georgeTrigger = 56;
		break;

	case 56:
		_georgeShould = 3;
		kernel_timing_trigger(1, 11);
		player_set_commands_allowed(true);
		break;

	case 60:
		setGlobals1(_ripTrekHandTalk3, 1, 7, 7, 7, 1);
		sendWSMessage_110000(62);
		digi_play("301r01", 1);
		break;

	case 62:
		sendWSMessage_140000(63);
		break;

	case 63:
		_georgeShould = 6;
		_georgeTriggerMode = KT_DAEMON;
		_soundName = "301a01";
		_georgeTrigger = 72;
		break;

	case 64:
		if (_val12) {
			_georgeShould = 5;
			_soundName = (_val12 == 1) ? "301a03" : "301a04";
			_georgeTriggerMode = KT_DAEMON;
			_georgeTrigger = 71;
		} else if (!player_been_here(401) && _G(flags)[V092] &&
				!_G(flags)[V093]) {
			kernel_timing_trigger(1, 1000);
		}
		break;

	case 65:
		_G(global301) = 0;
		setGlobals1(_ripTrekTalker3, 1, 1, 1, 5, 1);
		sendWSMessage_110000(68);
		_georgeShould = 8;
		digi_play("301r01a", 1, 255, 68);
		break;

	case 68:
		if (_G(global301) >= 1) {
			_G(global301) = 0;
			sendWSMessage_140000(-1);
			_georgeShould = 9;
			_soundName = "301a04a";
			_georgeTrigger = 70;
		} else {
			++_G(global301);
		}
		break;

	case 70:
		_georgeShould = 0;
		player_set_commands_allowed(true);
		break;

	case 71:
		_georgeShould = 0;
		kernel_timing_trigger(1, 999);
		break;

	case 72:
		_georgeShould = 4;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 11);
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(10, 64);
		break;

	case 100:
		sendWSMessage_10000(1, _georgeMach, _agentCheckingList, 8,
			1, 10, _agentStander, 1, 1, 0);
		_georgeMode = 0;
		
		break;

	case 200:
		if (_marshallMode || _marshallShould || _ripleyTrigger == -1) {
			kernel_timing_trigger(1, 201);
		} else {
			kernel_trigger_dispatchx(_ripleyTrigger);
			_ripleyTrigger = -1;
		}
		break;

	case 201:
		if (_marshallMode <= 0) {
			if (_marshallShould <= 0) {
				kernel_timing_trigger(30, 201);
			} if (_marshallShould == 1) {
				sendWSMessage_10000(1, _marshallMach, _marshalMatt, 17, 51, 201,
					_marshalMatt, 51, 51, 0);
				_marshallMode = 1;
			}
		} else if (_marshallMode == 1) {
			if (_marshallShould == 1) {
				frame = imath_ranged_rand(52, 55);
				sendWSMessage_10000(1, _marshallMach, _marshalMatt, frame, frame, 201,
					_marshalMatt, frame, frame, 0);
				sendWSMessage_190000(_marshallMach, 13);
				sendWSMessage_1a0000(_marshallMach, 13);

			} else {
				sendWSMessage_10000(1, _marshallMach, _marshalMatt, 52, 52, 200,
					_marshalMatt, 52, 52, 0);
			}
		}
		break;

	case 202:
		digi_play("301s01", 1, 255, 203);
		sendWSMessage_10000(1, _marshallMach, _marshalMatt, 4, 16, 201,
			_marshalMatt, 16, 16, 0);
		_marshallShould = 0;
		_marshallMode = 0;
		break;

	case 203:
		setGlobals1(_ripTrekArms, 1, 15, 15, 15, 1);
		sendWSMessage_110000(-1);
		digi_play("301r22", 1, 255, 204);
		break;

	case 204:
		digi_play("301s02", 1, 255, 205);
		_marshallShould = 1;
		break;

	case 205:
		_marshallShould = 0;
		kernel_timing_trigger(60, 206);
		break;

	case 206:
		disable_player_commands_and_fade_init(207);
		break;

	case 207:
		sendWSMessage_150000(208);
		break;

	case 208:
		ws_demand_facing(_G(my_walker), 3);
		kernel_timing_trigger(1, 209);
		break;

	case 209:
		other_save_game_for_resurrection();
		_G(game).setRoom(413);
		break;

	case 300:
		terminateMachineAndNull(_georgeMach);
		_ripleyMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, false,
			triggerMachineByHashCallback, "rip");
		sendWSMessage_10000(1, _ripleyMach, _agentStander, 7,
			1, 304, _agentStander, 1, 1, 0);
		break;

	case 304:
		ws_hide_walker();
		sendWSMessage(1, _ripleyMach, _agentTakesTelegram,
			1, 53, 305, _agentTakesTelegram, 53, 53, 0);
		break;

	case 305:
		kernel_timing_trigger(1, 990);
		inv_give_to_player("MESSAGE LOG");
		break;

	case 320:
		sendWSMessage(1, _ripleyMach, _agentTakesTelegram,
			54, 63, 322, _agentTakesTelegram, 63, 63, 0);
		break;

	case 322:
		sendWSMessage_10000(1, _ripleyMach, _ripTrekTravel,
			10, 1, 324, _ripTrekTravel, 1, 1, 0);
		_georgeMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, false,
			triggerMachineByHashCallback, "guy behind desk");
		_georgeMode = 0;
		_georgeShould = 0;
		sendWSMessage_10000(1, _georgeMach, _agentStander, 1,
			1, 10, _agentStander, 1, 1, 0);
		break;

	case 324:
		terminateMachineAndNull(_ripleyMach);
		ws_unhide_walker();
		kernel_timing_trigger(1, 1000);
		break;

	case 990:
		_digiSound1 = _digiSound2 = nullptr;
		_digiSound3 = _digiSound4 = nullptr;

		if (_val12 <= 0) {
			kernel_timing_trigger(1, 320);
		} else if (_nextSound) {
			_digiSound1 = _nextSound;
			_nextSound = nullptr;
			kernel_timing_trigger(1, 991);
		} else  if (_G(flags)[V365] == 1) {
			_digiSound1 = "201R61";
			_G(flags)[V365] = 0;
			kernel_timing_trigger(1, 991);
		} else if (_G(flags)[V366] == 1) {
			_digiSound1 = "401R31";
			_G(flags)[V366] = 0;
			kernel_timing_trigger(1, 991);
		} else if (_G(flags)[V373] == 1) {
			_digiSound1 = "401R36";
			_G(flags)[V373] = 0;
			kernel_timing_trigger(1, 991);
		} else if (_G(flags)[V371] == 1) {
			_digiSound1 = "501R03C";
			_G(flags)[V371] = 0;
			kernel_timing_trigger(1, 991);
		} else if (_G(flags)[V372] == 1) {
			_digiSound1 = "701R39";
			_digiSound2 = "701R39A";
			_G(flags)[V372] = 0;
			kernel_timing_trigger(1, 991);
		}
		break;

	case 991:
		if (_digiSound1)
			digi_play(_digiSound1, 1, 255, 993);
		else
			kernel_timing_trigger(1, 993);
		break;

	case 993:
		if (_digiSound2)
			digi_play(_digiSound2, 1, 255, 994);
		else
			kernel_timing_trigger(1, 994);
		break;

	case 994:
		if (_digiSound3)
			digi_play(_digiSound3, 1, 255, 995);
		else
			kernel_timing_trigger(1, 995);
		break;

	case 995:
		if (_digiSound4)
			digi_play(_digiSound4, 1, 255, 996);
		else
			kernel_timing_trigger(1, 996);
		break;

	case 996:
		if (--_val12 == 0)
			kernel_timing_trigger(1, 320);
		else
			kernel_timing_trigger(1, 990);
		break;

	case 999:
		kernel_timing_trigger(1, !_val12 ? 1000 : 300);
		break;

	case 1000:
		if (!player_been_here(401) && _G(flags)[V092]
			&& !_G(flags)[V093]) {
			_G(flags)[V092] = 0;
			_G(flags)[V093] = 1;
			kernel_timing_trigger(30, 65);
		}

		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room301::pre_parser() {
	if (player_said("exit") && _G(flags)[V007]) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room301::parser() {
	const KernelTriggerType oldMode = _G(kernel).trigger_mode;
	const bool lookFlag = player_said_any("look", "look at");		// ecx
	const bool talkFlag = player_said_any("talk", "talk to");
	const bool takeFlag = player_said("take");					// edi
	const bool useFlag = player_said_any("push", "pull", "gear", "open", "close"); // esi

	if (player_said("conv301a")) {
		conv301a();
	} else if (player_said("exit")) {
		if (_G(flags)[V007]) {
			if (_G(kernel).trigger == -1) {
				player_set_commands_allowed(false);
				_marshalMatt = series_load("marshall matt");
				digi_preload("301s01");
				_ripTrekArms = series_load("rip trek arms x pos3");
				ws_walk(_G(my_walker), 200, 269, nullptr, 1, 9);
			} else if (_G(kernel).trigger == 1) {
				_G(kernel).trigger_mode = KT_DAEMON;
				_marshallMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, false,
					triggerMachineByHashCallback, "marshal");
				sendWSMessage(1, _marshallMach, _marshalMatt, 1, 2, 202, _marshalMatt, 3, 3, 0);
				_marshallShould = 0;
				_marshallMode = 0;
			}
		} else {
			if (_G(kernel).trigger == -1) {
				player_set_commands_allowed(false);
				disable_player_commands_and_fade_init(1);
			} else if (_G(kernel).trigger == 1) {
				_G(game).setRoom(303);
			}
		}
	} else if (lookFlag && player_said("poster")) {
		digi_play("301r04", 1);
	} else if (lookFlag && player_said("window")) {
		digi_play(_G(player).been_here_before ? "301r15" : "301r05", 1);
	} else if (lookFlag && player_said("magazines")) {
		digi_play("301r06", 1);
	} else if (lookFlag && player_said("water cooler")) {
		digi_play(_G(player).been_here_before ? "301r16" : "301r07", 1);
	} else if (lookFlag && player_said("agent")) {
		digi_play("301r08", 1);
	} else if (lookFlag && player_said("telephone")) {
		digi_play("301r27", 1);
	} else if (lookFlag && player_said("plant")) {
		digi_play("301r28", 1);
	} else if (lookFlag && player_said("postcards")) {
		if (_G(flags)[V033]) {
			digi_play("301r14", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				digi_play("301r03", 1, 255, 3);
				kernel_timing_trigger(260, 1);
				break;
			case 1:
				player_update_info();
				ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y,
					nullptr, 2, 3);
				break;
			case 2:
				setGlobals1(_ripTrekHandTalk3, 1, 7, 8, 9, 1, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(-1);
				break;
			case 3:
				sendWSMessage_140000(-1);
				_georgeShould = 5;
				_val6_triggerMode = KT_PARSE;
				_georgeTriggerMode = KT_PARSE;
				_georgeTrigger = 4;
				_soundName = "301a02";
				break;
			case 4:
				_georgeShould = 0;
				_ripleyTrigger = kernel_trigger_create(5);
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 11);
				_G(kernel).trigger_mode = oldMode;
				break;
			case 5:
				player_set_commands_allowed(true);
				_G(flags)[V033] = 1;
				break;
			default:
				break;
			}
		}
	} else if (player_said_any("AMERICAN CURRENCY", "CHINESE CURRENCY") &&
			player_said("AGENT")) {
		digi_play("301r30", 1);
	} else if (lookFlag && player_said_any("sofa", "chair")) {
		digi_play("301r29", 1);
	} else if (_G(kernel).trigger == 747) {
		player_set_commands_allowed(false);
		_georgeShould = 0;
		_ripleyShould = 4;

		if (_travelDest) {
			switch (_travelDest) {
			case 1:
				_G(flags)[kTravelDest] = 0;
				break;
			case 2:
				_G(flags)[kTravelDest] = 2;
				break;
			case 3:
				_G(flags)[kTravelDest] = 3;
				break;
			case 4:
				_G(flags)[kTravelDest] = 4;
				break;
			default:
				break;
			}

			disable_player_commands_and_fade_init(748);
		}

	} else if (_G(kernel).trigger == 748) {
		_G(game).setRoom(495);

	} else if (talkFlag && player_said("agent")) {
		ws_hide_walker();
		player_set_commands_allowed(false);
		player_update_info();

		_shadowMach = series_show("safari shadow 3", 0xf00, 0, -1, -1, 0,
			_G(player_info).scale, _G(player_info).x, _G(player_info).y);
		_ripleyMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, false,
			triggerMachineByHashCallback, "rip");

		_G(kernel).trigger_mode = KT_DAEMON;
		_ripleyMode = 0;
		_ripleyShould = 5;
		sendWSMessage_10000(1, _ripleyMach, _ripTrekTravel, 1, 9, 20,
			_ripTrekTravel, 10, 10, 0);
		_georgeShould = 4;
	} else if (takeFlag && player_said("postcards")) {
		digi_play("301r09", 1);
	} else if (takeFlag && player_said("water cooler")) {
		digi_play("301r10", 1);
	} else if (takeFlag && player_said("magazines")) {
		digi_play("301r11", 1);
	} else if (useFlag && player_said_any("sofa", "chair")) {
		digi_play("301r31", 1);
	} else if (useFlag && player_said("telephone")) {
		digi_play("301r32", 1);
	} else if (useFlag && _G(player).click_y <= 374 &&
			!inv_player_has(_G(player).noun)) {
		digi_play("301r33", 1);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room301::conv301a() {
	const char *sound = conv_sound_to_play();
	const int who = conv_whos_talking();
	const int node = conv_current_node();

	if (_G(kernel).trigger == 1) {
		if (who <= 0) {
			if (node != 3) {
				_georgeShould = 4;
				conv_resume();
			}
		} else if (who == 1) {
			if (node == 11) {
				// Take payment
				_georgeShould = 12;
			} else if (node != 13) {
				_ripleyShould = 0;
				conv_resume();
			}
		} else {
			conv_resume();
		}
	} else if (sound) {
		if (who <= 0) {
			if (node == 3) {
				_georgeShould = 15;
			} else if (node != 2) {
				_georgeShould = imath_ranged_rand(5, 6);
			} else {
				_georgeShould = 2;
				_soundName = sound;
				_georgeTrigger = 1;
				return;
			}
		} else if (who == 1) {
			if (node == 13)
				_georgeShould = 13;
			else
				_ripleyShould = 1;
		}

		digi_play(sound, 1, 255, 1);

	} else {
		conv_resume();
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
