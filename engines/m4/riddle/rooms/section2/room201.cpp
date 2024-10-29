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

#include "m4/riddle/rooms/section2/room201.h"
#include "m4/riddle/rooms/section2/section2.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const char *LOOK[][2] = {
	{ "VASE",          "201r08" },
	{ "POSTCARD RACK", "201r09" },
	{ "POSTER",        "201r10" },
	{ "WINDOW",        "201r11" },
	{ "MAGAZINES",     "201r12" },
	{ "AGENT",         "201r13" },
	{ "RUG",           "201r14" },
	{ "LAMP",          "201r37" },
	{ "PLANT",         "201r38" },
	{ "SOFA",          "201r39" },
	{ "CHAIR",         "201r39" },
	{ "PHONE",         "201r40" },
	{ nullptr, nullptr }
};

static const char *TAKE[][2] = {
	{ "POSTCARD RACK", "201r19" },
	{ "MAGAZINES",     "201r20" },
	{ "VASE",          "201r41" },
	{ "RUG",           "201r42" },
	{ nullptr, nullptr }
};

static const char *USE[][2] = {
	{ "MONEY WITH AGENT", "201r43" },
	{ "SOFA",             "201r44" },
	{ "CHAIR",            "201r44" },
	{ "PHONE",            "201r45" },
	{ nullptr, nullptr }
};

Room201::Room201() : Section2Room() {
	Common::fill(_items, _items + 12, 0);
	Common::fill(_itemFlags, _itemFlags + 12, 0);
}

void Room201::init() {
	if (keyCheck()) {
		_flag1 = true;
		player_set_commands_allowed(false);
	} else {
		_flag1 = false;
	}

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 0;
		_givenYuan = false;
	}
	_itemDigi = nullptr;

	digi_preload("950_s02");
	_nod = series_load("HEAD NOD Y/N");
	_guyWriting = series_load("GUY WRITING");
	_guyParcel = series_load("GUY TAKE PARCEL FROM RIPLEY");

	if (_flag1) {
		_series1 = series_load("GUY GET ATTENTION / WAVE");
		_series2 = series_load("201rp99");
		_series3 = series_load("201rp02");
		_series4 = series_load("201mc01");
	} else {
		_series5 = series_load("GUY TAKE LETTER FROM RIPLEY");
		_series6 = series_load("RIP TREK HAND TALK POS3");
		_series7 = series_load("RIP TREK TELEGRAM DISPLACEMENT");
		_series8 = series_load("GUY TELEGRAM FROM SLOT");
	}

	if (_G(game).previous_room != KERNEL_RESTORING_GAME)
		_val4 = 0;
	_doc = nullptr;

	kernel_timing_trigger(1, 507);
	digi_play_loop("950_s02", 3, 50);

	if (_flag1) {
		_mei1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 512, 0,
			triggerMachineByHashCallback, "mc");
		sendWSMessage(1, _mei1, _series4, 1, 1, -1, _series4, 1, 1, 0);

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 512, 0,
			triggerMachineByHashCallback, "rip");
		sendWSMessage(1, _ripley, _series2, 1, 21, -1, _series2, 21, 21, 0);
		kernel_timing_trigger(10, 100);

	} else {
		if (_val4 == 1)
			_doc = series_place_sprite("201DOC", 0, 0, 0, 100, 0x410);

		if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
			if (_G(flags)[V053] == 1) {
				_meiHandHip = series_load("MEI TREK HAND ON HIP POS4");
				_meiTalker = series_load("MEI TREK TALKER POS4");
				_meiWalk = series_load("MEI CHEN TREK WALK POS4");
				_mei2 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 200, 238, 73, 0x900, 0,
					triggerMachineByHashCallback, "MC");
				series_place_sprite("candleman shadow4", 0, 200, 238, 73, 0xf00);
				sendWSMessage(1, _mei2, _meiHandHip, 22, 22, 2000, _meiHandHip, 22, 22, 0);
				_val5 = 2;
				_val6 = 2;
				_trigger9 = -1;
				_val8 = -1;
				_val9 = -1;

			} else {
				hotspot_set_active("MEI CHEN", false);
			}
		} else {
			player_set_commands_allowed(false);
			ws_demand_location(79, 257, 3);

			if (_G(flags)[V053] == 2 && _G(game).previous_room != KERNEL_RESTORING_GAME) {
				++_G(flags)[V006];

				if (setItemsPlacedFlags()) {
					_itemDigi = getItemsPlacedDigi();
					++_val1;
				}
			}

			if (_G(flags)[V053] > 0) {
				hotspot_set_active("MEI CHEN", false);
				ws_walk(356, 256, nullptr, 1000, 3);
				_G(flags)[V068] = 1;

				if (_G(flags)[V053] == 1)
					_G(flags)[V364] = 1;

			} else {
				digi_preload("201R01");
				digi_preload("201M03");
				_G(flags)[V068] = 1;
				_G(flags)[V053] = 1;
				setupMei();
				_G(flags)[V059] = 0;
				sendWSMessage_10000(_mei0, 180, 257, 3, 180, 1);
				kernel_timing_trigger(1, 8000);
			}
		}	
	}
}

void Room201::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case 50:
		player_set_commands_allowed(true);
		break;

	case 75:
		ws_walk(237, 247, nullptr, 205, 10);
		break;

	case 100:
		digi_play("201r30", 1, 255, 102);
		break;

	case 102:
		sendWSMessage_10000(1, _mei1, _series4, 1, 6, -1, _series4, 3, 6, 1);
		sendWSMessage_190000(_mei1, 11);
		sendWSMessage_1a0000(_mei1, 11);
		digi_play("201m09", 1, 255, 104);
		break;

	case 104:
		sendWSMessage_10000(1, _mei1, _series4, 7, 18, -1, _series4, 18, 18, 1);
		sendWSMessage_10000(1, _ripley, _series2, 21, 42, 105, _series2, 42, 42, 1);
		digi_play("201r31", 1, 255, 110);
		break;

	case 105:
		sendWSMessage_10000(1, _ripley, _series2, 43, 52, 106, _series2, 52, 52, 1);
		digi_play("950_s35", 2);
		break;

	case 106:
		sendWSMessage_10000(1, _ripley, _series2, 52, 21, 108, _series2, 21, 21, 1);
		sendWSMessage_10000(1, _mei1, _series4, 18, 7, -1, _series4, 7, 7, 1);
		break;

	case 108:
		sendWSMessage_10000(1, _ripley, _series2, 13, 13, -1, _series2, 13, 21, 1);
		sendWSMessage_1a0000(_ripley, 11);
		break;

	case 110:
		sendWSMessage_10000(1, _ripley, _series2, 21, 21, -1, _series2, 21, 21, 1);
		sendWSMessage_10000(1, _mei1, _series4, 33, 61, -1, _series4, 61, 63, 1);
		sendWSMessage_1a0000(_mei1, 11);
		digi_play("201m06", 1, 255, 112);
		break;

	case 112:
		sendWSMessage_10000(1, _mei1, _series4, 61, 61, -1, _series4, 61, 61, 1);
		sendWSMessage_10000(1, _ripley, _series2, 13, 13, -1, _series2, 13, 21, 1);
		digi_play("201r32", 1, 255, 116);
		break;

	case 116:
		sendWSMessage_10000(1, _ripley, _series2, 21, 21, -1, _series2, 21, 21, 1);
		sendWSMessage_10000(1, _mei1, _series4, 61, 61, -1, _series4, 61, 63, 1);
		sendWSMessage_1a0000(_mei1, 11);
		digi_play("201m07", 1, 255, 120);
		break;

	case 120:
		sendWSMessage_10000(1, _mei1, _series4, 61, 61, -1, _series4, 61, 61, 1);
		sendWSMessage_10000(1, _ripley, _series2, 53, 61, -1, _series2, 61, 63, 1);
		sendWSMessage_1a0000(_ripley, 11);
		digi_play("201r33", 1, 255, 123);
		kernel_timing_trigger(150, 121);
		break;

	case 121:
		sendWSMessage_10000(1, _ripley, _series2, 61, 53, -1, _series2, 13, 21, 1);
		sendWSMessage_1a0000(_ripley, 11);
		break;

	case 123:
		sendWSMessage_10000(1, _ripley, _series2, 13, 13, -1, _series2, 13, 13, 1);
		sendWSMessage_10000(1, _mei1, _series4, 64, 78, -1, _series4, 78, 81, 1);
		sendWSMessage_1a0000(_mei1, 11);
		digi_play("201m08", 1, 255, 126);
		break;

	case 126:
		sendWSMessage_10000(1, _mei1, _series4, 78, 64, -1, _series4, 64, 64, 1);
		sendWSMessage_10000(1, _ripley, _series2, 73, 82, 127, _series2, 82, 82, 1);
		digi_play("201r34", 1, 255, 129);
		break;

	case 127:
		terminateMachineAndNull(_mei1);
		sendWSMessage_10000(1, _ripley, _series3, 1, 8, -1, _series3, 8, 12, 1);
		break;

	case 129:
		sendWSMessage_10000(1, _ripley, _series3, 12, 12, -1, _series3, 12, 12, 1);
		kernel_timing_trigger(60, 130);
		break;

	case 130:
		_agentShould = 0;
		sendWSMessage_10000(1, _ripley, _series3, 13, 27, 162, _series3, 27, 27, 1);
		break;

	case 131:
		sendWSMessage_10000(1, _ripley, _series3, 29, 42, 160, _series3, 42, 42, 1);
		sendWSMessage_190000(_ripley, 30);
		break;

	case 132:
		_agentShould = 0;
		sendWSMessage_10000(1, _ripley, _series3, 51, 73, 161, _series3, 73, 73, 1);
		digi_play("com119", 1, 190, -1);
		break;

	case 137:
		_agentShould = 0;
		sendWSMessage_10000(1, _ripley, _series3, 74, 93, 138, _series3, 93, 93, 1);
		break;

	case 138:
		sendWSMessage_10000(1, _ripley, _series3, 94, 98, -1, _series3, 98, 102, 11);
		digi_play("201r36", 1, 255, 140);
		kernel_timing_trigger(160, 139);
		break;

	case 139:
		sendWSMessage_10000(1, _ripley, _series3, 103, 113, -1, _series3, 113, 113, 1);
		break;

	case 140:
		sendWSMessage_10000(1, _ripley, _series3, 114, 115, 144, _series3, 115, 115, 1);
		break;

	case 144:
		kernel_timing_trigger(50, 145);
		break;

	case 145:
		sendWSMessage_10000(1, _ripley, _series3, 116, 128, -1, _series3, 128, 128, 1);
		digi_play("201r35", 1, 255, 146);
		break;

	case 146:
		kernel_timing_trigger(100, 147);
		break;

	case 147:
		interface_hide();
		disable_player_commands_and_fade_init(150);
		break;

	case 150:
		_G(game).setRoom(203);
		break;

	case 160:
		sendWSMessage_10000(1, _ripley, _series3, 43, 50, 132, _series3, 50, 50, 1);
		break;

	case 161:
		sendWSMessage_10000(1, _ripley, _series3, 73, 73, -1, _series3, 73, 73, 1);
		_agentShould = 10;
		digi_play("201x08", 1, 255, 137);
		break;

	case 162:
		sendWSMessage_10000(1, _ripley, _series3, 28, 28, -1, _series3, 28, 28, 1);
		kernel_timing_trigger(120, 131);
		break;

	case 180:
		sendWSMessage_10000(_mei0, 200, 238, 4, -1, 1);
		break;

	case 205:
		_ripTalk = series_load("RIP TREK TALK");
		setGlobals1(_ripTalk, 1, 7, 1, 7, 1);
		sendWSMessage_110000(9139);
		digi_play("201r01", 1, 255, 210);
		kernel_timing_trigger(1, 1999);
		break;

	case 210:
		_val6 = 3;
		digi_play("201m01", 1, 255, 215);
		break;

	case 215:
		_ctr1 = 0;
		_val6 = 0;
		setGlobals1(_ripTalk, 1, 7, 1, 7, 1);
		sendWSMessage_110000(217);
		digi_play("201r02", 1, 255, 217);
		break;

	case 217:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			sendWSMessage_150000(220);
		} else {
			++_ctr1;
		}
		break;

	case 220:
		series_unload(_ripTalk);
		_val6 = 3;
		digi_play("201m02", 1, 255, 230);
		break;

	case 230:
		_val6 = 0;
		kernel_timing_trigger(120, 235);
		break;

	case 235:
		player_update_info();
		ws_walk(_G(player_info).x, _G(player_info).y, nullptr, 240, 3);
		break;

	case 240:
		setGlobals1(_series6, 1, 7, 7, 7);
		sendWSMessage_110000(241);
		break;

	case 241:
		digi_play("201r03", 1, 255, 242);
		break;

	case 242:
		sendWSMessage_140000(-1);
		_agentShould = 0;
		_trigger10 = kernel_trigger_create(244);
		break;

	case 244:
		_agentShould = 10;
		digi_play("201x01", 1, 255, 245);
		break;

	case 245:
		_agentShould = 3;
		_trigger10 = kernel_trigger_create(260);
		break;

	case 260:
		_agentShould = 0;
		_val6 = 4;
		digi_play("201m03", 1, 255, 270);
		break;

	case 270:
		_val6 = 0;
		_trigger9 = kernel_trigger_create(279);
		break;

	case 279:
		_val6 = 2;
		ws_walk(356, 256, nullptr, 280, 3);
		break;

	case 280:
		setGlobals1(_series6, 1, 7, 7, 7);
		sendWSMessage_110000(290);
		digi_play("201r04", 1, 255, 290);
		break;

	case 290:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			sendWSMessage_140000(-1);
			_agentShould = 0;
			_trigger10 = kernel_trigger_create(300);

		} else {
			++_ctr1;
		}
		break;

	case 300:
		_agentShould = 2;
		digi_play("201x02", 1, 255, 310);
		break;

	case 310:
		_ripHandChin = series_load("RIP TREK HAND CHIN POS3");
		setGlobals1(_ripHandChin, 1, 14, 14, 14);
		sendWSMessage_110000(320);
		digi_play(_G(flags)[GLB_TEMP_2] == 1 ? "201r05" : "201r06", 1, 255, 320);
		break;

	case 320:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			sendWSMessage_140000(325);
			_agentShould = 8;
		} else {
			++_ctr1;
		}
		break;

	case 325:
		series_unload(_ripHandChin);
		digi_unload("201R01");
		digi_unload("201M03");
		player_set_commands_allowed(true);
		break;

	case 507:
		_agent = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "201 guy behind desk 1");
		_agentMode = 9;
		_agentShould = 8;
		_trigger10 = _trigger11 = _trigger12 = -1;
		_flag3 = false;
		kernel_timing_trigger(1, 511);
		break;

	case 509:
		_agent = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallback, "201 guy behind desk 2");
		sendWSMessage_10000(1, _agent, _nod, 1, 1, 510, _nod, 1, 1, 0);
		_agentMode = 0;
		_agentShould = 0;
		_trigger10 = _trigger11 = _trigger12 = -1;
		_flag3 = false;
		break;

	case 510:
		if (_agentMode == 0 && _agentShould == 0 && _trigger10 != -1) {
			kernel_trigger_dispatchx(_trigger10);
			_trigger10 = -1;
		}

		if (_agentMode == 9 && _agentShould == 8 && _trigger11 != -1) {
			kernel_trigger_dispatchx(_trigger11);
			_trigger11 = -1;
		}

		if (_agentMode == 9 && _agentShould == 8 && _trigger11 != -1) {
			kernel_trigger_dispatchx(_trigger11);
			_trigger11 = -1;
		}

		if (_flag3) {
			terminateMachineAndNull(_agent);
		} else {
			kernel_timing_trigger(1, 511);
		}
		break;

	case 511:
		if (_agentMode <= 0) {
			switch (_agentShould) {
			case 0:
				sendWSMessage_10000(1, _agent, _nod, 1, 1, 510, _nod, 1, 1, 0);
				break;

			case 1:
				sendWSMessage_10000(1, _agent, _nod, 1, 7, 510, _nod, 1, 1, 0);
				_agentShould = 0;
				_agentMode = 0;
				break;

			case 2:
				sendWSMessage_10000(1, _agent, _nod, 7, 29, 510, _nod, 1, 1, 0);
				_agentShould = 0;
				_agentMode = 0;
				break;

			case 3:
				_guyBow = series_load("GUY BOW");
				sendWSMessage_10000(1, _agent, _guyBow, 1, 32, 510, _guyBow, 1, 1, 0);
				_agentShould = 4;
				break;

			case 4:
				sendWSMessage_10000(1, _agent, _nod, 1, 1, 510, _nod, 1, 1, 0);
				series_unload(_guyBow);
				_agentShould = 0;
				break;

			case 5:
				sendWSMessage_10000(1, _agent, _guySeries1, 1, 49, 510, _nod, 1, 1, 0);
				_agentShould = 0;
				_agentMode = 0;
				break;

			case 8:
			case 9:
				sendWSMessage_10000(1, _agent, _guyWriting, 1, 22, 510, _guyWriting, 22, 22, 0);
				_agentMode = 9;
				break;

			case 10:
				frame = imath_ranged_rand(1, 10) + 3;
				sendWSMessage_10000(1, _agent, _nod, frame, frame, 510, _nod, frame, frame, 1);
				_agentMode = 0;
				break;

			default:
				break;
			}
		} else if (_agentMode == 9) {
			switch (_agentShould) {
			case 8:
				sendWSMessage_10000(1, _agent, _guyWriting, 23, 32, 510, _guyWriting, 32, 23, 1);
				break;

			case 9:
				sendWSMessage_10000(1, _agent, _guyWriting, 22, 22, 510, _guyWriting, 22, 22, 0);
				break;

			default:
				sendWSMessage_10000(1, _agent, _guyWriting, 22, 2, 510, _guyWriting, 1, 1, 0);
				_agentMode = 0;
				break;
			}
		}
		break;

	case 1000:
		animateRipley();
		kernel_timing_trigger(2, 1003);
		break;

	case 1003:
		_conv2 = 6;
		_trigger4 = kernel_trigger_create(1004);
		break;

	case 1004:
		_conv2 = 7;
		_trigger5 = kernel_trigger_create(1005);
		break;

	case 1005:
		_conv2 = 8;
		digi_play("201r03", 1, 255, 1010);
		break;

	case 1010:
		_conv2 = 7;
		_agentShould = 0;
		_trigger10 = kernel_trigger_create(1030);
		break;

	case 1030:
		_agentShould = 10;
		digi_play("201x03", 1, 255, 1035);
		break;

	case 1035:
		_agentShould = 3;
		_trigger10 = kernel_trigger_create(1040);
		break;

	case 1040:
		checkFlags();

		if (_val1 == 0) {
			_agentShould = 0;
			kernel_timing_trigger(15, 1065);
		} else if (_val1 == 1) {
			_agentShould = 10;
			digi_play("201x04", 1, 255, 9160);
		} else {
			_agentShould = 10;
			digi_play("201x05", 1, 255, 9160);
		}
		break;

	case 1065:
		_trigger10 = kernel_trigger_create(1070);
		break;

	case 1070:
		if (_G(flags)[V079] == 0 && _G(flags)[V053] == 1) {
			kernel_timing_trigger(1, 9200);
		} else {
			kernel_timing_trigger(15, 1073);
			_conv2 = 6;
		}
		break;

	case 1073:
		_agentShould = 0;
		_trigger10 = kernel_trigger_create(1075);
		break;

	case 1075:
		_trigger4 = kernel_trigger_create(1080);
		break;

	case 1080:
		_agentShould = 8;
		player_set_commands_allowed(true);

		if (_G(flags)[V053] == 1)
			_G(flags)[V053] = 2;

		_trigger11 = kernel_trigger_create(1200);
		break;

	case 1200:
		if (_G(flags)[V053] == 1)
			_G(flags)[V053] = 2;
		if (_G(flags)[V056] == 1 && _G(flags)[V066] == 0) {
			kernel_timing_trigger(10, 3000);
		} else {
			kernel_timing_trigger(10, 50);
		}

		_num2 = 1;
		break;

	case 1999:
		ws_get_walker_info(_mei0, &_G(player_info).x, &_G(player_info).y,
			&_G(player_info).scale, &_G(player_info).depth, &_G(player_info).facing);
		_mei2 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _G(player_info).x, _G(player_info).y,
			_G(player_info).scale, 0x900, 0, triggerMachineByHashCallback, "Mei Chen other states machine");
		series_place_sprite("candleman shadow4", 0, _G(player_info).x, _G(player_info).y,
			_G(player_info).scale, 0xf00);
		sendWSMessage_10000(1, _mei2, _meiWalk, 1, 1, 2000, _meiWalk, 1, 1, 0);
		sendWSMessage_60000(_mei0);
		kernel_timing_trigger(1, 9300);
		_val5 = _val6 = 0;
		_trigger9 = _val8 = _val9 = -1;
		break;

	case 2000:
		if (_val5 == 0 && _val6 == 0 && _trigger9 != -1) {
			kernel_trigger_dispatchx(_trigger9);
			_trigger9 = -1;
		}

		if (_val5 == 2 && _val6 == 2 && _val8 != -1) {
			kernel_trigger_dispatchx(_val8);
			_val8 = -1;
		}

		if (_val5 == 5 && _val6 == 5 && _val9 != -1) {
			kernel_trigger_dispatchx(_val9);
			_val9 = -1;
		}

		kernel_timing_trigger(1, 2010);
		break;

	case 2010:
		switch (_val5) {
		case 0:
			switch (_val6) {
			case 0:
				sendWSMessage_10000(1, _mei2, _meiWalk, 1, 1, 2000, _meiWalk, 1, 1, 0);
				break;

			case 2:
				_meiHandHip = series_load("MEI TREK HAND ON HIP POS4");
				sendWSMessage_10000(1, _mei2, _meiHandHip, 1, 21, 2000, _meiHandHip, 22, 22, 0);
				_val5 = 2;
				break;

			case 3:
				sendWSMessage_10000(1, _mei2, _meiTalker, 1, 1, 2000, _meiTalker, 1, 4, 1);
				_val5 = 3;
				break;

			case 4:
				_meiHeadTurn = series_load("MEI TREK HEAD TRN POS4");
				sendWSMessage_10000(1, _mei2, _meiHeadTurn, 8, 9, 2000, _meiHeadTurn, 10, 10, 0);
				_val5 = 4;
				break;

			case 5:
			case 6:
				_meiTalkLoop = series_load("MEI TREK TALK LOOP POS4");
				sendWSMessage_10000(1, _mei2, _meiTalkLoop, 1, 14, 2000, _meiTalkLoop, 15, 15, 0);
				_val5 = 5;
				break;

			default:
				break;
			}
			break;

		case 2:
			switch (_val6) {
			case 0:
				sendWSMessage_10000(1, _mei2, _meiHandHip, 21, 1, 2000, _meiWalk, 1, 1, 0);
				_val6 = 1;
				break;

			case 1:
				sendWSMessage_10000(1, _mei2, _meiWalk, 1, 1, 2000, _meiWalk, 1, 1, 0);
				series_unload(_meiHandHip);
				_val6 = 0;
				_val5 = 0;
				break;

			case 2:
				sendWSMessage_10000(1, _mei2, _meiHandHip, 22, 22, 2000, _meiHandHip, 22, 22, 0);
				break;

			default:
				break;
			}
			break;

		case 3:
			switch (_val6) {
			case 0:
				sendWSMessage_10000(1, _mei2, _meiTalker, 1, 1, 2000, _meiWalk, 1, 1, 0);
				_val5 = 0;
				break;

			case 3:
				frame = imath_ranged_rand(1, 4);
				sendWSMessage_10000(1, _mei2, _meiTalker, frame, frame, 2000,
					_meiTalker, frame, frame, 1);
				break;

			default:
				break;
			}
			break;

		case 4:
			switch (_val6) {
			case 0:
				sendWSMessage_10000(1, _mei2, _meiHeadTurn, 10, 8, 2000, _meiWalk, 1, 1, 0);
				_val6 = 1;
				break;

			case 1:
				sendWSMessage_10000(1, _mei2, _meiWalk, 1, 1, 2000, _meiWalk, 1, 1, 0);
				_val5 = _val6 = 0;
				series_unload(_meiHeadTurn);
				break;

			case 4:
				sendWSMessage_10000(1, _mei2, _meiHeadTurn, 10, 10, 2000,
					_meiHeadTurn, 10, 10, 0);
				break;

			default:
				break;
			}
			break;

		case 5:
			switch (_val6) {
			case 0:
				sendWSMessage_10000(1, _mei2, _meiTalkLoop, 15, 1, 2000, _meiWalk, 1, 1, 0);
				_val6 = 1;
				break;

			case 1:
				sendWSMessage_10000(1, _mei2, _meiWalk, 1, 1, 2000, _meiWalk, 1, 1, 0);
				_val6 = _val5 = 0;
				series_unload(_meiTalkLoop);
				break;

			case 5:
				sendWSMessage_10000(1, _mei2, _meiTalkLoop, 15, 15, 2000, _meiTalkLoop, 15, 15, 0);
				break;

			case 6:
				sendWSMessage_10000(1, _mei2, _meiTalkLoop, 15, 29, 2000, _meiTalkLoop, 29, 29, 0);
				_val6 = 7;
				break;

			case 7:
				sendWSMessage_10000(1, _mei2, _meiTalkLoop, 29, 15, 2000, _meiTalkLoop, 15, 15, 0);
				_val6 = 5;
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 3000:
		player_set_commands_allowed(false);
		animateRipley();
		_agentShould = 0;
		kernel_timing_trigger(2, 3003);
		break;

	case 3003:
		_conv2 = 7;
		_trigger5 = kernel_trigger_create(3005);
		break;

	case 3005:
		_trigger10 = kernel_trigger_create(3010);
		break;

	case 3010:
		_conv2 = 8;
		digi_play("201r27", 1, 255, 3020);
		break;

	case 3020:
		_conv2 = 7;
		_agentShould = 10;
		digi_play("201x06", 1, 255, 3030);
		break;

	case 3030:
		_conv2 = 8;
		_agentShould = 0;
		digi_play("201r28", 1, 255, 3040);
		break;

	case 3040:
		_conv2 = 7;
		_agentShould = 10;
		digi_play("201x07", 1, 255, 3050);
		break;

	case 3050:
		_conv2 = 8;
		_agentShould = 0;
		digi_play("201r29", 1, 255, 3060);
		break;

	case 3060:
		_conv2 = 6;
		_agentShould = 0;
		_trigger10 = kernel_trigger_create(3070);
		break;

	case 3070:
		_agentShould = 8;
		_trigger4 = kernel_trigger_create(3080);
		break;

	case 3080:
		_trigger11 = kernel_trigger_create(3090);
		break;

	case 3090:
		_G(flags)[V066] = 1;
		_num2 = 1;
		player_set_commands_allowed(true);
		break;

	case 4000:
		_num1 = 1;
		_conv2 = 1;
		_trigger1 = _trigger2 = _trigger3 = _trigger4 = -1;
		_trigger5 = _trigger6 = _trigger7 = _trigger8 = -1;

		player_update_info();
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale,
			_G(player_info).depth, false, triggerMachineByHashCallback, "Rip Machine State");
		sendWSMessage_10000(1, _ripley, _series6, 1, 1, 4010, _series6, 1, 1, 0);
		_shadow3 = series_place_sprite("SAFARI SHADOW 3", 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00);
		break;

	case 4010:
		if (_num1 == 1 && _conv2 == 1 && _trigger1 != -1) {
			kernel_trigger_dispatchx(_trigger1);
			_trigger1 = -1;
		}

		if (_num1 == 3 && _conv2 == 3 && _trigger2 != -1) {
			kernel_trigger_dispatchx(_trigger2);
			_trigger2 = -1;
		}

		if (_num1 == 4 && _conv2 == 4 && _trigger3 != -1) {
			kernel_trigger_dispatchx(_trigger3);
			_trigger3 = -1;
		}

		if (_num1 == 6 && _conv2 == 6 && _trigger4 != -1) {
			kernel_trigger_dispatchx(_trigger4);
			_trigger4 = -1;
		}

		if (_num1 == 7 && _conv2 == 7 && _trigger5 != -1) {
			kernel_trigger_dispatchx(_trigger5);
			_trigger5 = -1;
		}

		if (_num1 == 7 && _conv2 == 16 && _trigger6 != -1) {
			kernel_trigger_dispatchx(_trigger6);
			_trigger6 = -1;
		}

		if (_num1 == 2 && _conv2 == 22 && _trigger7 != -1) {
			kernel_trigger_dispatchx(_trigger7);
			_trigger7 = -1;
		}

		if (_num1 == 7 && _conv2 == 13 && _trigger8 != -1) {
			kernel_trigger_dispatchx(_trigger8);
			_trigger8 = -1;
		}

		if (_num2) {
			terminateMachineAndNull(_ripley);
			ws_unhide_walker();
			terminateMachineAndNull(_shadow3);

		} else {
			kernel_timing_trigger(1, 4020);
		}
		break;

	case 4020:
		switch (_num1) {
		case 1:
			switch (_conv2) {
			case 1:
				sendWSMessage_10000(1, _ripley, _series6, 1, 1, 4010, _series6, 1, 1, 0);
				break;

			case 2:
			case 3:
				sendWSMessage_10000(1, _ripley, _series6, 1, 5, 4010, _series6, 5, 5, 0);
				_num1 = 3;
				break;

			case 4:
			case 5:
				sendWSMessage_10000(1, _ripley, _ripHandChin, 1, 12, 4010, _ripHandChin, 12, 12, 0);
				_num1 = 4;
				break;

			default:
				break;
			}
			break;

		case 3:
			switch (_conv2) {
			case 1:
				sendWSMessage_10000(1, _ripley, _series6, 5, 1, 4010, _series6, 1, 1, 0);
				_num1 = 1;
				break;

			case 2:
				sendWSMessage_10000(1, _ripley, _series6, 6, 9, 4010, _series6, 8, 5, 0);
				break;

			case 3:
				sendWSMessage_10000(1, _ripley, _series6, 5, 5, 4010, _series6, 5, 5, 0);
				break;

			default:
				break;
			}
			break;

		case 4:
			switch (_conv2) {
			case 1:
				sendWSMessage_10000(1, _ripley, _ripHandChin, 12, 1, 4010, _ripHandChin, 1, 1, 0);
				_num1 = 1;
				break;

			case 4:
				sendWSMessage_10000(1, _ripley, _ripHandChin, 12, 12, 4010, _ripHandChin, 12, 12, 0);
				break;

			case 5:
				sendWSMessage_10000(1, _ripley, _ripHandChin, 11, 7, 4010, _ripHandChin, 7, 7, 0);
				_num1 = 5;
				break;

			default:
				break;
			}
			break;

		case 5:
			switch (_conv2) {
			case 4:
				sendWSMessage_10000(1, _ripley, _ripHandChin, 7, 12, 4010, _ripHandChin, 12, 12, 0);
				_num1 = 4;
				break;

			case 5:
				sendWSMessage_10000(1, _ripley, _ripHandChin, 7, 7, 4010, _ripHandChin, 7, 7, 0);
				break;

			default:
				break;
			}
			break;

		case 6:
			switch (_conv2) {
			case 6:
				sendWSMessage_10000(1, _ripley, _series7, 1, 1, 4010, _series7, 1, 1, 0);
				break;

			case 7:
				sendWSMessage_10000(1, _ripley, _series7, 1, 9, 4010, _series7, 10, 10, 0);
				_num1 = 7;
				break;

			case 9:
				_conv2 = 7;
				_trigger5 = kernel_trigger_create(6500);
				kernel_timing_trigger(1, 4010);
				break;

			default:
				break;
			}
			break;

		case 7:
			switch (_conv2) {
			case 6:
				sendWSMessage_10000(1, _ripley, _series7, 9, 1, 4010, _series7, 1, 1, 0);
				_num1 = 6;
				break;

			case 7:
				sendWSMessage_10000(1, _ripley, _series7, 10, 10, 4010, _series7, 10, 10, 0);
				break;

			case 8:
				frame = imath_ranged_rand(11, 19);
				sendWSMessage_10000(1, _ripley, _series7, frame, frame, 4010, _series7, frame, frame, 0);
				break;

			case 10:
				_flag3 = true;
				sendWSMessage_10000(1, _ripley, _guyParcel, 1, 25, 4010, _guyParcel, 25, 25, 0);
				_conv2 = 11;
				break;

			case 11:
				digi_play("COM090", 1);
				sendWSMessage_10000(1, _ripley, _guyParcel, 26, 68, 4010, _guyParcel, 1, 1, 0);
				_conv2 = 13;
				break;

			case 12:
				player_set_commands_allowed(true);
				_conv2 = 7;
				sendWSMessage_10000(1, _ripley, _guyParcel, 1, 1, 4010, _guyParcel, 1, 1, 0);
				kernel_timing_trigger(1, 6500);
				break;

			case 13:
				sendWSMessage_10000(1, _ripley, _guyParcel, 1, 1, 4010, _guyParcel, 1, 1, 0);
				break;

			case 14:
				_conv2 = 6;
				_agentShould = 8;
				_trigger4 = kernel_trigger_create(6000);
				kernel_timing_trigger(1, 4010);

				if (_travelDest) {
					player_set_commands_allowed(false);
					disable_player_commands_and_fade_init(6700);
				}
				break;

			case 15:
				_flag3 = true;
				sendWSMessage_10000(1, _ripley, _series5, 1, 35, 4010, _series5, 35, 35, 0);
				_conv2 = 16;
				break;

			case 16:
				sendWSMessage_10000(1, _ripley, _series5, 35, 35, 4010, _series5, 35, 35, 0);
				break;

			case 17:
				sendWSMessage_10000(1, _ripley, _series5, 35, 1, 4010, _series5, 1, 1, 0);
				_conv2 = 7;
				break;

			case 18:
				player_set_commands_allowed(false);
				_flag3 = true;
				digi_play(conv_sound_to_play(), 1);
				sendWSMessage_10000(1, _ripley, _guyPassForm, 1, 77, 4010,
					_guyPassForm, 77, 77, 0);
				_conv2 = 19;
				break;

			case 19:
				sendWSMessage_10000(1, _ripley, _series5, 48, 1, 4010, _series5, 1, 1, 0);
				_conv2 = 7;
				_trigger5 = kernel_trigger_create(7100);
				series_unload(_guyPassForm);
				break;

			case 20:
				_flag3 = true;
				digi_preload("201R63");
				sendWSMessage_10000(1, _ripley, _series8, 1, 50, 4010, _series8, 50, 50, 0);
				_conv2 = 21;
				break;

			case 21:
				digi_play("201R63", 1);
				sendWSMessage_10000(1, _ripley, _series8, 51, 69, 4010, _series8, 69, 69, 0);
				_conv2 = 22;
				_num1 = 22;
				break;

			case 24:
				_flag3 = true;
				sendWSMessage_10000(1, _ripley, _guyPassForm, 1, 50, 4010, _guyPassForm, 50, 50, 0);
				_conv2 = 25;
				break;

			case 25:
				digi_play("950_S34", 2);

				if (_doc) {
					_doc = series_place_sprite("201DOC", 0, 0, 0, 100, 0x410);
					_val4 = 1;
				}

				sendWSMessage_10000(1, _ripley, _guyPassForm, 51, 57, 4010, _guyPassForm, 57, 57, 0);
				_conv2 = 26;
				break;

			case 26:
				digi_stop(2);
				sendWSMessage_10000(1, _ripley, _guyPassForm, 58, 77, 4010, _guyPassForm, 77, 77, 0);
				kernel_timing_trigger(1, 509);
				_conv2 = 27;
				break;

			case 27:
				_G(flags)[V079] = 1;
				_G(flags)[V365] = 1;
				sendWSMessage_10000(1, _ripley, _series7, 10, 10, 4010, _series7, 10, 10, 0);
				_conv2 = 7;
				break;

			case 28:
				_flag3 = true;
				sendWSMessage_10000(1, _ripley, _guyParcel, 68, 1, 4010, _guyParcel, 1, 1, 0);
				_conv2 = 29;
				break;

			case 29:
				kernel_timing_trigger(1, 509);
				sendWSMessage_10000(1, _ripley, _guyParcel, 1, 1, 4010, _guyParcel, 1, 1, 0);
				_conv2 = 7;
				_trigger5 = kernel_trigger_create(9250);
				break;

			default:
				break;
			}
			break;

		case 22:
			switch (_conv2) {
			case 7:
				sendWSMessage_10000(1, _ripley, _series8, 70, 79, 4010, _series7, 10, 10, 0);
				_num1 = 7;
				kernel_timing_trigger(1, 509);
				break;

			case 22:
				sendWSMessage_10000(1, _ripley, _series8, 69, 69, 4010, _series8, 69, 69, 0);
				break;

			case 23:
				sendWSMessage_10000(1, _ripley, _series8, 69, 70, 4010, _series8, 70, 69, 0);
				_conv2 = 22;
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 6000:
		player_set_commands_allowed(true);
		_num2 = 1;
		break;

	case 6500:
		_G(kernel).trigger_mode = KT_PARSE;
		conv_load("conv201a", 10, 10, 747);
		_hasKeyItems = buildKeyItemsArray();
		conv_export_pointer_curr(&_hasKeyItems, 0);
		conv_export_value_curr(_G(flags)[V088] >= 3 ? 1 : 0, 1);

		for (int i = 0; i < 12; ++i)
			conv_export_pointer_curr(&_itemFlags[i], i + 2);

		if (_G(flags)[V056] != 1)
			conv_export_value_curr(0, 15);
		else if (_G(flags)[V110] != 1)
			conv_export_value_curr(1, 15);
		else
			conv_export_value_curr(2, 15);

		_travelDest = 0;
		conv_export_pointer_curr(&_travelDest, 16);
		conv_export_pointer_curr(&_givenYuan, 17);
		conv_export_value_curr((_G(flags)[V053] == 1) ? 1 : 0, 19);
		conv_play();
		break;

	case 6700:
		digi_stop(2);
		digi_stop(3);

		if (_G(flags)[V056] == 1)
			_G(flags)[V049] = 1;

		_G(flags)[kTravelDest] = _travelDest;
		_G(game).setRoom(495);
		break;

	case 7010:
		_conv2 = 17;
		_trigger5 = kernel_trigger_create(7020);
		break;

	case 7020:
		if (!_givenYuan) {
			inv_give_to_player("CHINESE YUAN");
			_givenYuan = true;
		}

		kernel_timing_trigger(1, 509);
		break;

	case 7100:
		player_set_commands_allowed(true);
		inv_give_to_player("US DOLLARS");
		kernel_timing_trigger(1, 509);
		break;

	case 8000:
		kernel_timing_trigger(120, 75);
		break;

	case 9010:
		kernel_timing_trigger(1, 509);
		kernel_timing_trigger(5, 9020);
		break;

	case 9020: {
		static const char *ITEMS[12] = {
			"CRYSTAL SKULL", "STICK AND SHELL MAP", "WHEELED TOY",
			"REBUS AMULET", "SHRUNKEN HEAD", "SILVER BUTTERFLY",
			"POSTAGE STAMP", "GERMAN BANKNOTE", "WHALE BONE HORN",
			"CHISEL", "INCENSE BURNER", "ROMANOV EMERALD"
		};
		assert((uint)conv_current_entry() <= 12);
		inv_move_object(ITEMS[conv_current_entry()], 305);
		player_set_commands_allowed(true);
		conv_resume();
		break;
	}

	case 9050:
		_itemDigi2 = _itemDigi3 = _itemDigi4 = _itemDigi5 = nullptr;
		inv_give_to_player("MESSAGE LOG");

		if (_val1 <= 0) {
			kernel_timing_trigger(1, 1080);
		} else {
			if (_itemDigi) {
				_itemDigi2 = _itemDigi;
				_itemDigi = nullptr;
			} else if (_G(flags)[V364] == 1) {
				_itemDigi2 = "201R26";
				_G(flags)[V364] = 0;
			} else if (_G(flags)[V365] == 1) {
				_itemDigi2 = "201R61";
				_G(flags)[V365] = 0;
			} else if (_G(flags)[V366] == 1) {
				_itemDigi2 = "401R31";
				_G(flags)[V366] = 0;
			} else if (_G(flags)[V373] == 1) {
				_itemDigi2 = "401R36";
				_G(flags)[V373] = 0;
			} else if (_G(flags)[V370] == 1) {
				_itemDigi2 = "501R02B";
				_itemDigi3 = _itemDigi4 = nullptr;
				_G(flags)[V370] = 0;
			} else if (_G(flags)[V371] == 1) {
				_itemDigi2 = "501R03C";
				_itemDigi3 = _itemDigi4 = _itemDigi5 = nullptr;
				_G(flags)[V371] = 0;
			} else if (_G(flags)[V372] == 1) {
				_itemDigi2 = "701R39";
				_itemDigi3 = "701R39A";
				_G(flags)[V372] = 0;
			} else if (_G(flags)[V367] == 1) {
				_itemDigi2 = "401R37";
				_G(flags)[V367] = 0;
				_flag2 = true;
			} else if (_G(flags)[V368] == 1) {
				_itemDigi2 = "401R38";
				_G(flags)[V368] = 0;
				_flag2 = true;
			} else if (_G(flags)[V369] == 1) {
				_itemDigi2 = "401R39";
				_G(flags)[V369] = 0;
				_flag2 = true;
			}

			kernel_timing_trigger(1, 9060);
		}
		break;

	case 9060:
		_conv2 = 22;
		_trigger7 = kernel_trigger_create(9085);
		break;

	case 9085:
		if (_itemDigi2)
			digi_play(_itemDigi2, 1, 255, 9090);
		else
			kernel_timing_trigger(1, 9090);
		break;

	case 9090:
		if (_itemDigi3)
			digi_play(_itemDigi3, 1, 255, 9100);
		else
			kernel_timing_trigger(1, 9100);
		break;

	case 9100:
		if (_itemDigi4)
			digi_play(_itemDigi4, 1, 255, 9110);
		else
			kernel_timing_trigger(1, 9110);
		break;

	case 9110:
		if (_itemDigi5)
			digi_play(_itemDigi5, 1, 255, 9120);
		else
			kernel_timing_trigger(1, 9120);
		break;

	case 9120:
		if (_flag2) {
			_flag2 = false;
			kernel_timing_trigger(1, 9230);
		} else {
			if (--_val1 > 0) {
				_conv2 = 23;
				_trigger7 = kernel_trigger_create(9050);
			} else {
				kernel_timing_trigger(1, 9220);
			}
		}
		break;

	case 9139:
		sendWSMessage_150000(-1);
		break;

	case 9160:
		_conv2 = 20;
		_trigger7 = kernel_trigger_create(9050);
		break;

	case 9200:
		_guyPassForm = series_load("GUY PASS FORM TO RIPLEY");
		digi_preload("950_S34");
		_conv2 = 8;
		digi_play("201R60", 1, 255, 9210);
		break;

	case 9210:
		_conv2 = 24;
		_trigger5 = kernel_trigger_create(9215);
		break;

	case 9215:
		series_unload(_guyPassForm);
		digi_unload("950_S34");
		kernel_timing_trigger(5, 1070);
		break;

	case 9220:
		_conv2 = 7;
		_trigger5 = kernel_trigger_create(1070);
		break;

	case 9230:
		_conv2 = 7;
		_trigger5 = kernel_trigger_create(9240);
		break;

	case 9240:
		_conv2 = 28;
		break;

	case 9250:
		_conv2 = 8;
		digi_play("COM084", 1, 255, 9220, 997);

		if (!inv_player_has("ROMANOV EMERALD"))
			inv_give_to_player("ROMANOV EMERALD");
		break;

	case 9300:
		series_unload(S2_MEI_NORMAL_DIRS[4]);
		series_unload(S2_MEI_NORMAL_DIRS[2]);
		series_unload(S2_MEI_NORMAL_DIRS[1]);
		series_unload(S2_MEI_NORMAL_DIRS[0]);
		break;

	default:
		if (_G(kernel).trigger < 9999)
			error("Unhandled trigger");
		break;
	}
}

void Room201::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool talkFlag = player_said_any("talk", "talk to");
	bool useFlag = player_said("gear");

	if (player_said("conv201a")) {
		conv201a();
	} else if (lookFlag && player_said(" ")) {
		digi_play("201R07", 1);
	} else if (lookFlag && _G(walker).ripley_said(LOOK)) {
		// Handled
	} else if (takeFlag && _G(walker).ripley_said(TAKE)) {
		// Handled
	} else if (useFlag && _G(walker).ripley_said(USE)) {
		// Handled
	} else if (takeFlag && player_said("AGENT")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			_agentShould = 0;
			_G(kernel).trigger_mode = KT_DAEMON;
			animateRipley();
			_G(kernel).trigger_mode = KT_PARSE;
			kernel_timing_trigger(2, 3);
			break;

		case 3:
			_conv2 = 9;
			_agentShould = 0;
			break;
		default:
			break;
		}
	} else if (_G(kernel).trigger == 747) {
		_conv2 = 14;
	} else if (talkFlag && player_said("MEI CHEN")) {
		if (_G(flags)[V059] == 0) {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				_ctr2 = 0;
				player_set_commands_allowed(false);
				_ripTalk = series_load("RIP TREK TALK");
				_val6 = 0;
				setGlobals1(_ripTalk, 1, 7, 1, 7, 1);
				sendWSMessage_110000(2);
				digi_play("201r15", 1, 255, 2);
				break;

			case 2:
				if (_ctr2 >= 1) {
					_ctr2 = 0;
					sendWSMessage_150000(3);
				} else {
					++_ctr2;
				}
				break;

			case 3:
				series_unload(_ripTalk);
				_val6 = 3;
				digi_play("201m04", 1, 255, 4);
				break;

			case 4:
				_val6 = 0;
				_trigger9 = kernel_trigger_create(5);
				break;

			case 5:
				_val6 = 2;
				_val8 = kernel_trigger_create(6);
				break;

			case 6:
				_G(flags)[V059] = 1;
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				_ripTalk = series_load("RIP TREK TALK");
				_val6 = 0;
				_trigger9 = kernel_trigger_create(2);
				break;

			case 2:
				_val6 = 5;
				_val9 = kernel_trigger_create(3);
				break;

			case 3:
			{
				_ctr2 = 0;
				setGlobals1(_ripTalk, 1, 7, 1, 7, 1);
				sendWSMessage_110000(4);

				static const char *DIGI[3] = { "201r16", "201r17", "201r18" };
				digi_play(DIGI[imath_ranged_rand(0, 2)], 1, 255, 4);
				break;
			}

			case 4:
				if (_ctr2 >= 1) {
					_ctr2 = 0;
					sendWSMessage_150000(5);
				} else {
					++_ctr2;
				}
				break;

			case 5:
				series_unload(_ripTalk);
				_val6 = 6;
				_val9 = kernel_trigger_create(6);
				break;

			case 6:
				_val6 = 0;
				_trigger9 = kernel_trigger_create(7);
				break;

			case 7:
				_val6 = 2;
				_val8 = kernel_trigger_create(8);
				break;

			case 8:
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
		}
	} else if (player_said("walk through")) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(66, 254, nullptr, 2, 7);
			break;

		case 2:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(3);
			break;

		case 3:
			_G(game).setRoom(203);
			break;

		default:
			break;
		}
	} else if (player_said("journal") && !takeFlag && !lookFlag) {
		digi_play("201r25", 1);
	} else if (useFlag && !inv_player_has(_G(player).noun)) {
		digi_play("201r46", 1);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room201::setupMei() {
	_meiTalker = series_load("MEI TREK TALKER POS4");
	_meiWalk = series_load("MEI CHEN TREK WALK POS4");
	ws_walk_load_walker_series(S2_MEI_NORMAL_DIRS, S2_MEI_NORMAL_NAMES);
	ws_walk_load_shadow_series(S2_MEI_SHADOW_DIRS, S2_MEI_SHADOW_NAMES);
	_mei0 = triggerMachineByHash_3000(8, 4, *S2_MEI_NORMAL_DIRS,
		*S2_MEI_SHADOW_DIRS, 79, 257, 3, triggerMachineByHashCallback3000, "mc");
}

void Room201::conv201a() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	player_set_commands_allowed(false);

	if (_G(kernel).trigger == 1) {
		if (who <= 0) {
			_agentShould = 0;

		} else if (who == 1) {
			_conv2 = 7;

			if (node == 1 && entry == 1) {
				int32 x1, y1, x2, y2;
				conv_set_box_xy(490, -4);
				conv_get_dlg_coords(&x1, &y1, &x2, &y2);
				conv_set_dlg_coords(x1, y1 - 10, x2, y2 - 10);
			} else {
				conv_set_box_xy(10, 10);
			}

			if (node == 1 && entry == 3) {
				player_set_commands_allowed(false);
				kernel_timing_trigger(1, 7000, KT_DAEMON, KT_PARSE);
				return;
			}

			if (node == 14 && entry != 12) {
				player_set_commands_allowed(false);
				kernel_timing_trigger(1, 9000, KT_DAEMON, KT_PARSE);
				return;
			}
		}

		player_set_commands_allowed(true);
		conv_resume();

	} else {
		if (who <= 0) {
			if (node == 3 && entry == 0) {
				player_set_commands_allowed(false);
				_conv2 = 18;
				_guyPassForm = series_load("GUY PASS FORM TO RIPLEY");
				return;
			} else {
				_agentShould = 10;
			}
		} else if (who == 1) {
			if (node != 14)
				_conv2 = 8;
		}

		if (sound)
			digi_play(sound, 1, 255, 1);
	}
}

void Room201::animateRipley() {
	_trigger1 = _trigger2 = _trigger3 = _trigger4 = -1;
	_trigger5 = _trigger6 = _trigger7 = _trigger8 = -1;
	_num1 = 6;
	_conv2 = 6;
	_num2 = 0;

	player_update_info();
	ws_hide_walker();
	_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100,
		_G(player_info).depth, 0, triggerMachineByHashCallback,
		"Rip Delta Machine State");
	sendWSMessage_10000(1, _ripley, _series7, 1, 1, 4010, _series7, 1, 1, 0);
	_shadow3 = series_place_sprite("SAFARI SHADOW 3", 0,
		_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00);
}

void Room201::checkFlags() {
	if (_G(flags)[V364] == 1) {
		_val1++;

		if (_G(flags)[V053] == 1) {
			_G(flags)[V350] = 1;
		} else {
			switch (_G(flags)[V005]) {
			case 1: _G(flags)[V351] = 1; break;
			case 2: _G(flags)[V352] = 1; break;
			case 3: _G(flags)[V353] = 1; break;
			case 4: _G(flags)[V354] = 1; break;
			default: break;
			}
		}
	}

	for (int i = V365; i <= V373; ++i) {
		if (_G(flags)[i] == 1) {
			_G(flags)[i - 10] = 1;
			++_val1;
		}
	}
}

bool Room201::buildKeyItemsArray() {
	static const char *ITEMS[12] = {
		"CRYSTAL SKULL", "STICK AND SHELL MAP", "WHEELED TOY",
		"REBUS AMULET", "SHRUNKEN HEAD", "SILVER BUTTERFLY",
		"POSTAGE STAMP", "GERMAN BANKNOTE", "WHALE BONE HORN",
		"CHISEL", "INCENSE BURNER", "ROMANOV EMERALD"
	};

	// Build up an array of the key items the player has
	Common::fill(_items, _items + 12, 0);
	Common::fill(_itemFlags, _itemFlags + 12, 0);
	_totalItems = 0;

	for (int i = 0; i < 12; ++i) {
		_itemFlags[i] = inv_player_has(ITEMS[i]) ? 1 : 0;
		if (_itemFlags[i])
			_items[_totalItems++] = i + 1;
	}

	return _totalItems > 0;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
