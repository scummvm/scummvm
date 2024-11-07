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

#include "m4/riddle/rooms/section3/room303.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/adv_file.h"
#include "m4/fileio/extensions.h"
#include "m4/fileio/info.h"
#include "m4/gui/gui_vmng.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"
#include "m4/riddle/riddle.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const char *const SAID[][2] = {
	{ "IRON MAIDEN",     "303R29" },
	{ "MUMMY WITH HAIR", "303R30" },
	{ "MAN WITH HAMMER", "303R33" },
	{ "TICKET WINDOW",   "303R34" },
	{ "PICTURE",         "303R35" },
	{ nullptr, nullptr }
};


void Room303::preload() {
	LoadWSAssets("OTHER SCRIPT");

	if (_G(flags)[V000]) {
		_G(art_base_override) = player_been_here(201) ?
			"EXHIBIT HALL-TREK" : "EH TREK NO SNAKE";
		_G(use_alternate_attribute_file) = true;
		_G(player).walker_type = 1;
		_G(player).shadow_type = 1;
		_G(player).walker_in_this_scene = true;

	} else {
		_G(player).walker_type = 0;
		_G(player).shadow_type = 0;
		_G(player).walker_in_this_scene = true;

		if (_G(game).room_id == 352) {
			_G(player).walker_in_this_scene = false;
			_G(player).disable_hyperwalk = true;
		}
	}
}

void Room303::init() {
	_newExhibitsFlag = _lonelyFlag = false;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_digiName1 = nullptr;
		_destTrigger = -1;
		_triggerMode1 = _triggerMode2 = KT_DAEMON;
		_val5 = 0;
		_val6 = 0;
		_val7 = 0;
		_val8 = 0;
	}

	if (player_been_here(301)) {
		hotspot_set_active("MEI CHEN", false);
		hotspot_set_active("FENG LI", false);
		hotspot_set_active("COVER", false);
	}

	if (_G(game).previous_room != 304) {
		_door = series_show_sprite("DOOR", 0, 0xf05);
	}

	int32 status;
	ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
	assert(game_buff_ptr);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		if (!player_been_here(301)) {
			kernel_load_variant(_fengFlag ? "303lock1" : "303lock2");
			setFengActive(_fengFlag);
			setupMei();
			setShadow4(true);

			ws_walk_load_walker_series(S3_NORMAL_DIRS, S3_NORMAL_NAMES);
			ws_walk_load_shadow_series(S3_SHADOW_DIRS, S3_SHADOW_NAMES);
			loadFengLi();

			if (_fengFlag) {
				_fengLi = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 480, 256, 86, 0xc00, 1,
					triggerMachineByHashCallback, "fl");
				setShadow5(true);
			} else {
				_fengLi = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 706, 256, 86, 0xc00, 0,
					triggerMachineByHashCallback, "fl state machine");
				setShadow5(false);
			}

			sendWSMessage_10000(1, _fengLi, _feng4, 1, 1, 400, _feng4, 1, 6, 0);
			_fengShould = _fengMode = 1;
		}

		playSeries();
		break;

	case 301:
		MoveScreenDelta(game_buff_ptr, -641, 0);
		playSeries();

		kernel_timing_trigger(1, 9);
		break;

	case 304:
		if (_G(flags)[V001]) {
			_G(player).disable_hyperwalk = true;
			LoadWSAssets("303 FL SCRIPT");

			_G(camera_reacts_to_player) = true;
			MoveScreenDelta(game_buff_ptr, -110, 0);
			_val12 = KT_DAEMON;

			player_set_commands_allowed(false);
			playSeries(false);

			_door = series_show_sprite("DOOR", 7, 0xf05);
			ws_demand_location(393, 260, 5);

			ws_walk_load_walker_series(S3_NORMAL_DIRS, S3_NORMAL_NAMES);
			ws_walk_load_shadow_series(S3_SHADOW_DIRS, S3_SHADOW_NAMES);
			_fengLi = triggerMachineByHash_3000(8, 2, *S3_NORMAL_DIRS, *S3_SHADOW_DIRS,
				470, 226, 9, triggerMachineByHashCallback3000, "fl");
			_fengFlag = true;

			kernel_load_variant("303lock1");
			setFengActive(true);
			loadFengLi();

			ws_walk_load_walker_series(S3_NORMAL_DIRS2, S3_NORMAL_NAMES2);
			ws_walk_load_shadow_series(S3_SHADOW_DIRS2, S3_SHADOW_NAMES2);

			_mei = triggerMachineByHash_3000(8, 3, *S3_NORMAL_DIRS2, *S3_SHADOW_DIRS2,
				445, 215, 7, triggerMachineByHashCallback3000, "mc");
			_gestTalk4 = series_load("mei ny lft hand gest talk pos4");
			kernel_timing_trigger(1, 107);

		} else {
			player_set_commands_allowed(false);
			kernel_timing_trigger(1, 10);
			_door = series_show_sprite("DOOR", 7, 0xf05);

			if (!player_been_here(301)) {
				kernel_load_variant("303lock1");
				setFengActive(true);
				setupMei();
				setShadow4(true);

				ws_walk_load_walker_series(S3_NORMAL_DIRS, S3_NORMAL_NAMES);
				ws_walk_load_shadow_series(S3_SHADOW_DIRS, S3_SHADOW_NAMES);
				loadFengLi();

				_fengLi = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 480, 256, 86, 0xc00, 1,
					triggerMachineByHashCallback, "fl");
				sendWSMessage_10000(1, _fengLi, _feng4, 1, 1, 400,
					_feng4, 1, 6, 0);

				_fengShould = _fengMode = 1;
				_fengFlag = true;
				setShadow5(true);
			}

			playSeries();
		}
		break;

	case 305:
		player_set_commands_allowed(false);
		ws_demand_location(28, 267, 3);
		playSeries();

		if (!player_been_here(301)) {
			kernel_timing_trigger(1, 6);
			setupMei();
			setShadow4(true);

			ws_walk_load_walker_series(S3_NORMAL_DIRS, S3_NORMAL_NAMES);
			ws_walk_load_shadow_series(S3_SHADOW_DIRS, S3_SHADOW_NAMES);
			loadFengLi();

			_fengLi = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 480, 256, 86, 0xc00, 1,
				triggerMachineByHashCallback, "fl");
			sendWSMessage_10000(1, _fengLi, _feng1, 1, 16, 400, _feng1, 1, 6, 0);
			_fengShould = _fengMode = 1;
			setShadow5(true);
			_fengFlag = true;

			kernel_load_variant("303lock1");
			setFengActive(true);
		} else {
			kernel_timing_trigger(1, 6);
		}
		break;

	case 309:
		interface_show();
		ws_demand_location(230, 258, 10);
		player_set_commands_allowed(false);

		if (!player_been_here(301)) {
			setupMei();
			setShadow4(true);
			ws_walk_load_walker_series(S3_NORMAL_DIRS, S3_NORMAL_NAMES);
			ws_walk_load_shadow_series(S3_SHADOW_DIRS, S3_SHADOW_NAMES);
			_fengFlag = true;
			loadFengLi();

			_fengLi = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 480, 256, 86, 0xc00, 1,
				triggerMachineByHashCallback, "fl");
			sendWSMessage_10000(1, _fengLi, _feng4, 1, 16, 400,
				_feng4, 1, 6, 0);

			_fengShould = _fengMode = 1;
			setShadow5(true);
			kernel_load_variant("303lock1");
			setFengActive(true);
		}

		playSeries();

		if (player_been_here(301)) {
			_ripBends = series_load("rip trek bends to viewer");
			setGlobals1(_ripBends, 17, 17, 17, 17, 1, 17, 1, 1, 1, 1);
			sendWSMessage_110000(3);

		} else {
			_ripBends = series_load("RIP BENDS TO SEE CREATURE");
			setGlobals1(_ripBends, 26, 1, 1, 1);
		}
		break;

	case 352:
		player_set_commands_allowed(false);
		interface_hide();
		digi_preload("303r02");
		digi_preload("303m02");
		digi_preload("303f01");

		AddSystemHotkey(KEY_ESCAPE, escapePressed);
		_G(kernel).call_daemon_every_loop = true;
		digi_stop(3);

		series_plain_play("303cow1", -1, 0, 100, 0, 9);
		series_show_sprite("doorknob relocation", 0, 0xf04);
		series_load("303 final frame");
		_machine2 = series_stream("EVERYTHING IN 303", 5, 17, 21);
		series_stream_break_on_frame(_machine2, 3, 18);

		MoveScreenDelta(game_buff_ptr, -320, 0);
		break;

	default:
		_val12 = KT_PARSE;
		kernel_load_variant("303lock1");
		setFengActive(true);
		ws_demand_location(145, 289, 3);
		setupMei();
		setShadow4(true);

		ws_walk_load_walker_series(S3_NORMAL_DIRS, S3_NORMAL_NAMES);
		ws_walk_load_shadow_series(S3_SHADOW_DIRS, S3_SHADOW_NAMES);

		_fengFlag = true;
		loadFengLi();
		_fengLi = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 480, 256, 86, 0xc00, 1,
			triggerMachineByHashCallback, "fl");
		sendWSMessage_10000(1, _fengLi, _feng4, 1, 1, 400, _feng4, 1, 6, 0);
		_fengShould = _fengMode = 1;
		setShadow5(true);

		playSeries();
		break;
	}
}

void Room303::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
	case 61:
		ws_demand_location(230, 258);
		player_set_commands_allowed(true);
		break;

	case 6:
		ws_walk(128, 267, nullptr, 75, 3);
		break;

	case 7:
		priestTalk(true, -1);
		setShadow5(true);
		break;

	case 8:
		ws_walk(145, 289, nullptr, -1, 2);
		kernel_timing_trigger(200, 38);
		break;

	case 9:
		ws_demand_location(1350, 276, 9);
		ws_walk(1208, 276, nullptr, 75, 9);
		break;

	case 10:
		ws_demand_location(425, 227, 7);
		ws_walk(399, 260, nullptr, 50, 7);
		break;

	case 18:
		digi_play("303r02", 1);
		series_stream_break_on_frame(_machine2, 20, 19);
		break;

	case 19:
		digi_play("303m02", 1);
		series_stream_break_on_frame(_machine2, 55, 20);
		break;

	case 20:
		digi_play("303f01", 1);
		series_stream_break_on_frame(_machine2, 77, 24);
		break;

	case 21:
		series_plain_play("303 final frame", -1, 0, 100,
			0x100, 3000);
		disable_player_commands_and_fade_init(22);
		break;

	case 22:
		digi_stop(1);
		digi_stop(3);
		adv_kill_digi_between_rooms(false);
		_G(game).setRoom(354);
		break;

	case 24:
		break;

	case 38:
		sendWSMessage_10000(_fengLi, 146, 270, 9, 39, 0);
		break;

	case 39:
		sendWSMessage_10000(_fengLi, 480, 256, 7, 40, 1);
		break;

	case 40:
		sendWSMessage_60000(_fengLi);
		_fengLi = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 480, 256, 86, 0xc00, 1,
			triggerMachineByHashCallback, "fl");
		sendWSMessage_10000(1, _fengLi, _feng1, 1, 16, 400,
			_feng4, 1, 6, 0);
		_fengShould = _fengMode = 1;
		setShadow5(true);
		player_set_commands_allowed(true);
		break;

	case 50:
		terminateMachineAndNull(_door);
		digi_preload("950_s44");
		series_plain_play("DOOR", 1, 2, 100, 0xf05, 10, 51);
		break;

	case 51:
		digi_play("950_s44", 2, 200, 52);
		_door = series_show_sprite("DOOR", 0, 0xf05);
		player_set_commands_allowed(true);
		break;

	case 52:
		digi_unload("950_s44");
		break;

	case 60:
		sendWSMessage_150000(61);
		break;

	case 70:
		kernel_timing_trigger(imath_ranged_rand(300, 600), 71);
		break;

	case 71:
		series_plain_play("CREATURE FEATURE LONG VIEW", 1, 0,
			100, 0xf05, 7, 70);
		break;

	case 75:
		player_set_commands_allowed(true);
		break;

	case 100:
		digi_play("950_s01", 3);
		kernel_timing_trigger(1020, 100);
		break;

	case 103:
		sendWSMessage_10000(_fengLi, 414, 228, 9, 104, 0);
		break;

	case 104:
		sendWSMessage_10000(_fengLi, 399, 256, 7, 105, 0);
		break;

	case 105:
		terminateMachineAndNull(_door);
		digi_preload("950_s44");
		series_plain_play("DOOR", 1, 2, 100, 0xf05, 8, 156);
		sendWSMessage_10000(_fengLi, 480, 256, 7, 106, 1);
		break;

	case 106:
		sendWSMessage_60000(_fengLi);
		priestTalk(true, -1);
		setShadow5(true);
		break;

	case 107:
		ws_walk(700, 305, 0, -1, 3);
		sendWSMessage_10000(_mei, 393, 260, 9, 109, 0);
		break;

	case 109:
		sendWSMessage_10000(_mei, 393, 260, 5, 110, 1);
		break;

	case 110:
		setGlobals3(_gestTalk4, 1, 21);
		sendWSMessage_F0000(_mei, 157);
		digi_play("303m03", 1, 255, 143);
		break;

	case 111:
		_chinTalk4 = series_load("suit hands out to side pos1");
		_suitTalk1 = series_load("suit talk pos1");
		setGlobals1(_chinTalk4, 1, 8, 8, 8);
		sendWSMessage_110000(154);
		digi_play("303r03", 1, 255, 116);
		break;

	case 116:
		_gestTalk4 = series_load("mc ny hand out talk pos4");
		sendWSMessage_10000(_mei, 357, 255, 4, 117, 1);
		kernel_timing_trigger(60, 103);
		series_unload(_suitTalk1);
		series_unload(_chinTalk4);
		ws_walk(700, 305, nullptr, -1, 3);
		break;

	case 117:
		setGlobals1(_gestTalk4, 1, 9, 9, 9);
		sendWSMessage_110000(_mei, 120);
		kernel_timing_trigger(70, 118);
		digi_play("303m04", 1, 255);
		break;

	case 118:
		ws_walk(444, 295, nullptr, 119, 10);
		break;

	case 119:
		_chinTalk4 = series_load("rip suit rt hand gest talk pos2");
		setGlobals1(_chinTalk4, 1, 8, 8, 8);

		_ctr1 = 0;
		sendWSMessage_110000(121);
		digi_play("303r04", 1, 255, 121);
		break;

	case 120:
		sendWSMessage_140000(_mei, 159);
		break;

	case 121:
		if (++_ctr1 == 2)
			sendWSMessage_140000(122);
		break;

	case 122:
		series_unload(_chinTalk4);
		player_update_info();
		ws_walk(_G(player_info).x + 1, _G(player_info).y - 1,
			nullptr, 123, 1);
		break;

	case 123:
		_chinTalk4 = series_load("suit talk pos1");
		digi_play(_G(flags)[V084] == 2 ? "303r05" : "303r06",
			1, 255, 138);
		setGlobals1(_chinTalk4, 1, 5, 5, 5);
		sendWSMessage_110000(144);
		kernel_timing_trigger(200, 124);
		break;

	case 124:
		_G(globals)[GLB_TEMP_1] = 0;
		_G(globals)[GLB_TEMP_2] = 0xFFFF0000;
		sendWSMessage(0x200000, 0, _priestTalk, 0, nullptr, 1);
		break;

	case 125:
		_chinTalk4 = series_load("rip suit lft hand gest talk");
		setGlobals4(_chinTalk4, 6, 6, 13);
		sendWSMessage_C0000(126);
		break;

	case 126:
		digi_play("303r07", 1, 255, 150);
		sendWSMessage_D0000();
		break;

	case 127:
		ws_walk(_G(player_info).x + 50, _G(player_info).y + 10,
			nullptr, -1, 10);
		break;

	case 129:
		series_unload(_chinTalk4);
		_chinTalk4 = series_load("rip suit rt hand gest talk pos2");
		setGlobals1(_chinTalk4, 1, 5, 5, 5);
		digi_play("303r08", 1, 255, 131);
		break;

	case 130:
		sendWSMessage_140000(158);
		break;

	case 131:
		_val12 = KT_DAEMON;
		playSound("303f03", -1, 132);
		break;

	case 132:
		ws_walk(565, 306, nullptr, 170, 10);
		kernel_timing_trigger(1, 133);
		break;

	case 133:
		playSound("303f04", 134, -1);
		break;

	case 134:
		if (_newExhibitsFlag)
			kernel_timing_trigger(1, 172);
		else
			kernel_timing_trigger(60, 134);
		break;

	case 136:
		sendWSMessage_140000(137);
		break;

	case 137:
		sendWSMessage_60000(_mei);
		setupMei();
		setShadow4(true);
		_val11 = 7;
		kernel_timing_trigger(1, 160);
		break;

	case 138:
		series_unload(_chinTalk4);
		ws_walk(500, 300, nullptr, 125, 3);
		break;

	case 143:
		player_update_info();
		sendWSMessage_10000(_G(my_walker),
			_G(player_info).x - 1, _G(player_info).y - 1,
			10, 198, 0);
		break;

	case 144:
	case 154:
		sendWSMessage_140000(-1);
		break;

	case 147:
		sendWSMessage_120000(148);
		break;

	case 148:
		sendWSMessage_110000(-1);
		break;

	case 150:
		sendWSMessage_B0000(127);
		playSound("303f02", -1, 129);
		break;

	case 156:
		digi_play("950_s44", 1, 200, 52);
		_door = series_show_sprite("DOOR", 0, 0xf05);
		break;

	case 157:
	case 159:
		series_unload(_gestTalk4);
		break;

	case 158:
		series_unload(_chinTalk4);
		break;

	case 160:
		series_unload(_chinTalk4);
		series_unload(220);
		series_unload(221);
		series_unload(222);
		series_plain_play("303cow1", -1, 0, 100, 0, 9);

		_G(player).disable_hyperwalk = false;
		_G(camera_reacts_to_player) = true;
		_val12 = KT_PARSE;
		_G(flags)[V001] = 0;

		terminateMachineAndNull(_priestTalk);
		_fengLi = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 480, 256, 86, 0xc00, true,
			triggerMachineByHashCallback, "fl");
		sendWSMessage_10000(1, _fengLi, _feng4, 1, 1, 400,
			_feng4, 1, 6, 0);
		_fengShould = _fengMode = 1;

		player_set_commands_allowed(true);
		break;

	case 170:
		_newExhibitsFlag = true;
		break;

	case 172:
		_chinTalk4 = series_load("RIP HNDS HIPS GEST TALK");
		setGlobals1(_chinTalk4, 1, 15, 15, 15, 0, 16, 25, 25, 25);
		sendWSMessage_110000(-1);
		kernel_timing_trigger(200, 147);
		digi_play("303r09", 1, 255, 136);
		break;

	case 198:
		ws_walk(444, 295, nullptr, 111, 11);
		break;

	case 200:
		if (_val10 == 0 && _val11 == 0 && _destTrigger != -1) {
			kernel_trigger_dispatchx(_destTrigger);
			_destTrigger = -1;

			if (_val5) {
				ws_unhide_walker();
				_val5 = 0;
			}
		}

		kernel_trigger_dispatchx(kernel_trigger_create(201));
		break;

	case 201:
		switch (_val10) {
		case 0:
			switch (_val11) {
			case 0:
				if (_val7) {
					series_unload(_meiLips);
					_val7 = 0;
				}

				if (_digiName1) {
					digi_play(_digiName1, 1, 255, _val18);
					_digiName1 = nullptr;
					_triggerMode2 = KT_DAEMON;
					_G(kernel).trigger_mode = KT_DAEMON;
				}

				++_val8;
				if (imath_ranged_rand(7, 12) < _val8) {
					_val8 = 0;

					switch (imath_ranged_rand(1, 3)) {
					case 1:
						sendWSMessage_10000(1, _mei, _mei2, 1, 14, 200,
							_mei2, 14, 14, 0);
						_val10 = _val11 = 1;
						break;

					case 2:
						sendWSMessage_10000(1, _mei, _mei1, 2, 9, 200,
							_mei1, 9, 9, 0);
						_val10 = _val11 = 1;
						break;

					case 3:
						kernel_timing_trigger(60, 201);
						break;

					default:
						break;
					}
				} else {
					kernel_timing_trigger(60, 201);
				}
				break;

			case 3:
				sendWSMessage_10000(1, _mei, _mei3, 1, 12, 200,
					_mei3, 12, 12, 0);
				_val10 = 3;
				break;

			case 4:
			case 5:
				sendWSMessage_10000(1, _mei, _mei3, 17, 17, 200,
					_mei3, 17, 17, 0);
				_val10 = 4;
				break;

			case 6:
				sendWSMessage_10000(1, _mei, _meiLips, 1, 13, 200,
					_meiLips, 13, 13, 0);
				_val10 = 6;
				break;

			case 7:
				sendWSMessage_10000(1, _mei, _mei2, 1, 14, 200,
					_mei2, 14, 14, 0);
				_val10 = _val11 = 1;
				break;

			default:
				break;
			}
			break;

		case 1:
			if (_val11 == 1) {
				++_val8;
				if (imath_ranged_rand(7, 12) < _val8) {
					_val8 = 0;

					switch (imath_ranged_rand(1, 2)) {
					case 1:
						sendWSMessage_10000(1, _mei, _mei2, 14, 1, 200,
							_mei1, 1, 1, 0);
						_val10 = _val11 = 0;
						break;

					case 2:
						kernel_timing_trigger(60, 201);
						break;

					default:
						break;
					}
				} else {
					kernel_timing_trigger(60, 201);
				}
			} else {
				sendWSMessage_10000(1, _mei, _mei2, 14, 1, 200,
					_mei1, 1, 1, 0);
				_val10 = 0;
			}
			break;

		case 2:
			if (_val11 == 2) {
				++_val8;
				if (imath_ranged_rand(7, 12) < _val8) {
					_val8 = 0;

					switch (imath_ranged_rand(1, 2)) {
					case 1:
						sendWSMessage_10000(1, _mei, _mei1, 9, 2, 200,
							_mei1, 1, 1, 0);
						break;

					case 2:
						kernel_timing_trigger(60, 201);
						break;

					default:
						break;
					}
				} else {
					kernel_timing_trigger(60, 201);
				}
				break;
			} else {
				sendWSMessage_10000(1, _mei, _mei1, 9, 2, 200,
					_mei1, 1, 1, 0);
				_val10 = 0;
			}
			break;

		case 3:
			if (_val11 == 3) {
				sendWSMessage_10000(1, _mei, _mei3, 13, 13, 200,
					_mei3, 13, 16, 0);
			} else {
				sendWSMessage_10000(1, _mei, _mei3, 12, 1, 200,
					_mei1, 1, 1, 0);
				_val10 = 0;
			}
			break;

		case 4:
			switch (_val11) {
			case 4:
				sendWSMessage_10000(1, _mei, _mei3, 17, 17, 200,
					_mei3, 17, 17, 0);
				break;

			case 5:
				frame = imath_ranged_rand(17, 20);
				sendWSMessage_10000(1, _mei, _mei3, frame, frame, 200,
					_mei3, frame, frame, 0);
				break;

			default:
				sendWSMessage_10000(1, _mei, _mei1, 1, 1, 200,
					_mei1, 1, 1, 0);
				_val10 = 0;
				break;
			}
			break;

		case 6:
			if (_val11 == 6) {
				frame = imath_ranged_rand(14, 16);
				sendWSMessage_10000(1, _mei, _meiLips, frame, frame, 200,
					_meiLips, frame, frame, 0);
			} else {
				sendWSMessage_10000(1, _mei, _meiLips, 17, 24, 200,
					_mei1, 1, 1, 0);
				_val10 = 0;
			}
			break;

		default:
			break;
		}
		break;

	case 300:
		if (_val17 == 0 && _val16 == 0 && _destTrigger != -1) {
			kernel_trigger_dispatchx(_destTrigger);
			_destTrigger = -1;

			if (_val5) {
				ws_unhide_walker();
				_val5 = 0;
			}
		}

		kernel_trigger_dispatchx(kernel_trigger_create(301));
		break;

	case 301:
		switch (_val17) {
		case 0:
			switch (_val16) {
			case 0:
				if (_digiName1) {
					digi_play(_digiName1, 1, 255, _val18);
					_digiName1 = nullptr;
					_triggerMode2 = KT_DAEMON;
					_G(kernel).trigger_mode = KT_DAEMON;
				}

				sendWSMessage_10000(1, _machine3, 1, 1, 1, 300, 1, 1, 1, 0);
				break;

			case 1:
				sendWSMessage_10000(1, _machine3, _suit2, 1, 10, 300, _suit2, 10, 10, 0);
				_val17 = 1;
				break;

			case 2:
				sendWSMessage_10000(1, _machine3, _suit1, 1, 17, 300,
					_suit1, 17, 17, 0);
				_val17 = 2;
				break;

			case 3:
			case 4:
				sendWSMessage_10000(1, _machine3, _ripGesture, 1, 14, 300,
					_ripGesture, 1, 14, 0);
				break;

			case 5:
				terminateMachineAndNull(_machine3);
				terminateMachineAndNull(_ripsh2);
				ws_unhide_walker();
				series_unload(_suit1);
				series_unload(_suit2);
				series_unload(_ripGesture);
				player_set_commands_allowed(true);
				return;

			default:
				break;
			}
			break;

		case 1:
			if (_val16 == 1) {
				sendWSMessage_10000(1, _machine3, _suit2, 10, 10, 300,
					_suit2, 10, 10, 0);
			} else {
				sendWSMessage_10000(1, _machine3, _suit2, 11, 18, 300, 1, 1, 1, 0);
				_val17 = 0;
			}
			break;

		case 2:
			if (_val16 == 2) {
				sendWSMessage_10000(1, _machine3, _suit1, 17, 17, 300,
					_suit1, 17, 17, 0);
			} else {
				sendWSMessage_10000(1, _machine3, _suit1, 17, 1, 300, 1, 1, 1, 0);
				_val17 = 0;
			}
			break;

		case 3:
			switch (_val16) {
			case 3:
				sendWSMessage_10000(1, _machine3, _ripGesture, 14, 14, 300,
					_ripGesture, 14, 14, 0);
				break;
			case 4:
				sendWSMessage_10000(1, _machine3, _ripGesture, 15, 25, 300,
					_ripGesture, 25, 25, 0);
				break;
			default:
				sendWSMessage_10000(1, _machine3, _ripGesture, 14,
					1, 300, 1, 1, 1, 0);
				_val17 = 0;
				break;
			}
			break;

		case 4:
			if (_val16 == 4) {
				sendWSMessage_10000(1, _machine3, _ripGesture, 25, 25, 300,
					_ripGesture, 25, 25, 0);
			} else {
				sendWSMessage_10000(1, _machine3, _ripGesture, 25, 15, 300,
					_ripGesture, 14, 14, 0);
				_val17 = 3;
			}
			break;

		default:
			break;
		}
		break;

	case 400:
		if (_fengShould == 0 && _fengMode == 0 && _destTrigger != -1) {
			kernel_trigger_dispatchx(_destTrigger);
			_destTrigger = -1;

			if (_val5) {
				ws_unhide_walker();
				_val5 = 0;
			}
		}

		kernel_trigger_dispatchx(kernel_trigger_create(401));
		break;

	case 401:
		switch (_fengShould) {
		case 0:
			switch (_fengMode) {
			case 0:
				sendWSMessage_10000(1, _fengLi, _feng1, 1, 1, 400,
					_feng1, 1, 1, 0);
				break;

			case 1:
			case 2:
			case 4:
				sendWSMessage_10000(1, _fengLi, _feng1, 1, 18, 400,
					_feng4, 1, 6, 1);
				_fengShould = 1;
				break;

			case 3:
				sendWSMessage_10000(1, _fengLi, _feng2, 1, 13, 400,
					_feng2, 14, 19, 1);
				_fengShould = 3;
				break;

			case 5:
				terminateMachineAndNull(_fengLi);

				if (_fengFlag)
					_fengLi = triggerMachineByHash_3000(8, 2, *S3_NORMAL_DIRS, *S3_SHADOW_DIRS,
						480, 256, 7, triggerMachineByHashCallback3000, "fl walker");
				else
					_fengLi = triggerMachineByHash_3000(8, 2, *S3_NORMAL_DIRS, *S3_SHADOW_DIRS,
						706, 256, 5, triggerMachineByHashCallback3000, "fl walker");

				_G(kernel).trigger_mode = KT_PARSE;
				kernel_timing_trigger(1, 1);
				return;

			default:
				break;
			}
			break;

		case 1:
			switch (_fengMode) {
			case 0:
				sendWSMessage_10000(1, _fengLi, _feng1, 16, 1, 400,
					_feng1, 1, 1, 0);
				_fengShould = 0;
				break;

			case 1:
				frame = imath_ranged_rand(1, 6);
				sendWSMessage_10000(1, _fengLi, _feng4, frame, frame, 400,
					_feng4, frame, frame, 0);
				break;

			case 3:
			case 5:
				sendWSMessage_10000(1, _fengLi, _feng1, 16, 1, 400,
					_feng2, 1, 1, 0);
				_fengShould = 0;
				break;

			case 4:
				frame = imath_ranged_rand(1, 6);
				sendWSMessage_10000(1, _fengLi, _feng4, frame, frame, 400,
					_feng4, frame, frame, 0);
				player_set_commands_allowed(true);
				_fengMode = 1;
				break;

			default:
				sendWSMessage_10000(1, _fengLi, _feng3, 1, 18, 400,
					_feng3, 19, 21, 1);
				_fengShould = 2;
				break;
			}
			break;

		case 2:
			if (_fengMode == 2) {
				frame = imath_ranged_rand(19, 21);
				sendWSMessage_10000(1, _fengLi, _feng3, frame, frame, 400,
					_feng3, frame, frame, 0);
			} else {
				sendWSMessage_10000(1, _fengLi, _feng3, 18, 1, 400,
					_feng4, 1, 6, 0);
				_fengShould = 1;
			}
			break;

		case 3:
			if (_fengMode == 3) {
				frame = imath_ranged_rand(14, 19);
				sendWSMessage_10000(1, _fengLi, _feng2, frame, frame, 400,
					_feng2, frame, frame, 0);
			} else {
				sendWSMessage_10000(1, _fengLi, _feng2, 20, 31, 400,
					_feng2, 1, 1, 0);
				_fengShould = 0;
			}
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	if (_G(game).previous_room == 352) {
		switch (_G(kernel).trigger) {
		case 55:
			_G(game).setRoom(304);
			break;
		case 56:
			_G(game).setRoom(494);
			break;
		default:
			break;
		}

		if (_G(MouseState).ButtonState) {
			_btnFlag = true;
		} else if (_btnFlag) {
			_btnFlag = false;

			disable_player_commands_and_fade_init(55);
		}
	}
}

void Room303::pre_parser() {
	if (player_said("open") && player_been_here(301)) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room303::parser() {
	bool lookFlag = player_said_any("look", "look at");

	if (player_said("conv303b")) {
		conv303b();
	} else if (player_said("conv303a")) {
		conv303a();
	} else if (lookFlag && _G(flags)[V000] == 0 && _fengFlag &&
			player_said_any("cow", "giant matchstick")) {
		switch (_G(kernel).trigger) {
		case -1:
			_fengMode = 5;
			player_set_commands_allowed(false);
			break;

		case 1:
			setShadow5(false);
			if (player_said("giant matchstick"))
				ws_demand_location(610, 256);

			sendWSMessage_10000(_fengLi, 706, 256, 5, 2, 1);
			break;

		case 2:
			setShadow5Alt(true);
			_fengLi = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 706, 256, 86, 0xc00, 0,
				triggerMachineByHashCallback, "fl state machine");

			_G(kernel).trigger_mode = KT_DAEMON;
			sendWSMessage_10000(1, _fengLi, _feng1, 1, 16, 400,
				_feng4, 1, 6, 0);
			_fengShould = _fengMode = 1;
			setFengActive(false);
			_fengFlag = false;

			kernel_load_variant("303lock2");
			_G(kernel).trigger_mode = KT_PARSE;
			kernel_timing_trigger(1, 666);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("torso tomato") &&
			!_G(flags)[V081] && !_G(flags)[V000] && !_fengFlag) {
		switch (_G(kernel).trigger) {
		case 1:
			setShadow5Alt(false);
			sendWSMessage_10000(_fengLi, 480, 256, 7, 2, 1);
			break;

		case 2:
			setShadow5(true);
			sendWSMessage_60000(_fengLi);
			_fengLi = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 480, 256, 86, 0xc00, 1,
				triggerMachineByHashCallback, "fl state machine");

			_G(kernel).trigger_mode = KT_DAEMON;
			sendWSMessage_10000(1, _fengLi, _feng1, 1, 16, 400,
				_feng4, 1, 6, 0);
			_fengShould = _fengMode = 1;
			setFengActive(true);
			_fengFlag = true;

			kernel_load_variant("303lock1");
			_G(kernel).trigger_mode = KT_PARSE;
			kernel_timing_trigger(1, 666);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("puffball")) {
		if (_G(flags)[V000] == 1) {
			digi_play("303r37", 1);
		} else if (_G(flags)[V080]) {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				_chinTalk4 = series_load("suit chin in hand pos4");
				setGlobals4(_chinTalk4, 15, 15, 15);
				sendWSMessage_C0000(0);
				digi_play("303r37", 1, 255, 2);
				break;

			case 2:
				sendWSMessage_B0000(3);
				break;

			case 3:
				series_unload(_chinTalk4);
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
				_chinTalk4 = series_load("rip hand to chin talk pos4");
				setGlobals4(_chinTalk4, 15, 16, 19);
				sendWSMessage_C0000(0);
				digi_play("303F13", 1, 255, 1);
				_fengMode = 2;
				break;

			case 1:
				_fengMode = 1;
				kernel_timing_trigger(30, 2);
				break;

			case 2:
				sendWSMessage_D0000();
				digi_play("303r26", 1, 255, 3);
				break;

			case 3:
				sendWSMessage_E0000();
				sendWSMessage(0xe0000, 0, _G(my_walker), 0, nullptr, 1);
				kernel_timing_trigger(30, 4);
				break;

			case 4:
				digi_play("303F14", 1, 255, 5);
				_fengMode = 2;
				break;

			case 5:
				_fengMode = 1;
				kernel_timing_trigger(30, 6);
				break;

			case 6:
				sendWSMessage_D0000();
				digi_play("303r27", 1, 255, 7);
				break;

			case 7:
				sendWSMessage_B0000(8);
				break;

			case 8:
				series_unload(_chinTalk4);
				player_set_commands_allowed(true);
				_G(flags)[V080] = 1;
				break;

			default:
				break;
			}
		}
	} else if (lookFlag && player_said("copper tank") &&
			(_G(flags)[V000] == 1 || _fengFlag)) {
		if (_G(flags)[V000] == 1) {
			digi_play("303r31", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				_chinTalk4 = series_load("suit hand in pocket pos2");
				setGlobals4(_chinTalk4, 19, 19, 19);
				sendWSMessage_C0000(0);
				digi_play("303r31", 1, 255, 2);
				break;

			case 2:
				sendWSMessage_B0000(3);
				break;

			case 3:
				series_unload(_chinTalk4);
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
		}
	} else if (lookFlag && player_said("torso tomato")) {
		if (_G(flags)[V000] == 1) {
			digi_play("303r28a", 1);
		} else if (_G(flags)[V081]) {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				_chinTalk4 = series_load("suit chin in hand pos4");
				setGlobals4(_chinTalk4, 15, 15, 15);
				sendWSMessage_C0000(-2);
				digi_play("303r28a", 1, 255, 1);
				break;

			case 1:
				sendWSMessage_B0000(3);
				break;

			case 3:
				series_unload(_chinTalk4);
				break;

			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				_chinTalk4 = series_load("rip suit talker pos4");
				setGlobals4(_chinTalk4, 1, 1, 5);
				sendWSMessage_C0000(-1);
				digi_play("303F15", 1, 255, 1);
				_fengMode = 2;
				break;

			case 2:
				sendWSMessage_D0000();
				digi_play("303r28", 1, 255, 3);
				break;

			case 3:
				sendWSMessage_B0000(4);
				break;

			case 4:
				series_unload(_chinTalk4);
				player_set_commands_allowed(true);
				_G(flags)[V081] = 1;
				break;

			default:
				break;
			}
		}
	} else if (lookFlag && player_said("cow")) {
		if (_G(flags)[V086] == 2) {
			_G(flags)[V086] = 1;
		} else if (_G(flags)[V086] == 3) {
			_G(flags)[V086] = 0;
		}

		if (_G(flags)[V000] == 1) {
			digi_play("303r23", 1);
		} else if (_G(flags)[V082]) {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				digi_play("303F09", 1, 255, 1);
				_fengMode = 2;
				break;

			case 1:
				_fengMode = 1;
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
				_chinTalk4 = series_load("rip hand to chin talk pos4");
				setGlobals4(_chinTalk4, 15, 16, 19);
				sendWSMessage_C0000(0);
				digi_play("303F10", 1, 255, 6);
				_fengMode = 2;
				break;

			case 1:
				digi_play("303r24", 1, 255, 3);
				sendWSMessage_D0000();
				_fengMode = 0;
				break;

			case 3:
				kernel_timing_trigger(30, 4);
				break;

			case 4:
				sendWSMessage_B0000(7);
				_fengMode = 3;
				digi_play("303f11", 1, 255, 5);
				break;

			case 5:
				_fengMode = 1;
				player_set_commands_allowed(true);
				_G(flags)[V082] = 1;
				break;

			case 6:
				_fengMode = 1;
				kernel_timing_trigger(30, 1);
				break;

			case 7:
				series_unload(_chinTalk4);
				break;

			default:
				break;
			}
		}
	} else if (lookFlag && player_said("giant matchstick")) {
		if (_G(flags)[V000] == 1) {
			digi_play("303r36", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				_chinTalk4 = series_load("rip suit talker pos4");
				setGlobals4(_chinTalk4, 1, 1, 5);
				sendWSMessage_C0000(1);
				break;

			case 2:
				sendWSMessage_D0000();
				digi_play("303r25", 1, 255, 3);
				break;

			case 3:
				digi_play("303F12", 1, 255, 4);
				_fengMode = 2;
				sendWSMessage_B0000(0);
				break;

			case 4:
				_fengMode = 1;
				series_unload(_chinTalk4);
				break;

			default:
				break;
			}
		}
	} else if (player_said("open")) {
		if (player_been_here(301)) {
			switch (_G(kernel).trigger) {
			case -1:
				ws_walk(409, 266, nullptr, 1, 1);
				break;

			case 1:
				player_set_commands_allowed(false);
				_med1 = series_load("RIP TREK MED REACH HAND POS1");
				setGlobals1(_med1, 1, 10, 10, 10, 1);
				sendWSMessage_110000(2);
				break;

			case 2:
				sendWSMessage_140000(4);
				terminateMachineAndNull(_door);
				series_plain_play("DOOR", 1, 0, 100, 0xf05, 7, 3);
				digi_play("303_s01", 1);
				break;

			case 3:
				series_show_sprite("DOOR", 7, 0xf05);
				break;

			case 4:
				ws_walk(417, 232, nullptr, -1, 2);
				break;

			case 5:
				_G(game).setRoom(304);
				break;

			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				player_update_info();
				ws_hide_walker();
				_ripsh1 = series_place_sprite("ripsh1", 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale,
					_G(player_info).depth + 1);
				_med1 = series_load("SUIT DOOR OPENER POS1");

				series_play_xy("SUIT DOOR OPENER POS1", 1, 0, 405, 266,
					_G(player_info).scale, _G(player_info).depth, 7, 1);
				break;

			case 2:
				series_show_sprite("DOOR", 7, 0xf05);
				break;

			case 3:
				ws_unhide_walker();
				terminateMachineAndNull(_ripsh1);
				kernel_timing_trigger(10, 4);
				break;

			case 4:
				ws_walk(417, 232, nullptr, -1, 2);
				disable_player_commands_and_fade_init(6);
				break;

			case 6:
				digi_stop(3);
				_G(game).setRoom(304);
				break;

			default:
				break;
			}
		}
	} else if (lookFlag && player_said("cobra case")) {
		digi_play(player_been_here(201) ? "203r38" : "303r32", 1);
	} else if ((lookFlag || player_said("peer into")) &&
			player_said("copper tank viewer")) {
		if (player_been_here(301)) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_ripBends = series_load("rip trek bends to viewer");
				setGlobals1(_ripBends, 1, 17, 17, 17, 1);
				sendWSMessage_110000(-1);
				disable_player_commands_and_fade_init(1);
				break;

			case 1:
				_G(game).setRoom(309);
				interface_hide();
				break;

			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_ripBends = series_load("RIP BENDS TO SEE CREATURE");
				setGlobals1(_ripBends, 1, 26, 26, 26);
				sendWSMessage_110000(1);
				break;

			case 1:
				disable_player_commands_and_fade_init(2);
				break;

			case 2:
				_G(game).setRoom(309);
				interface_hide();
				break;
			default:
				break;
			}
		}
	} else if (player_said("TALK MEI CHEN")) {
		player_set_commands_allowed(false);
		_suit1 = series_load("SUIT CHIN IN HAND POS2");
		_suit2 = series_load("RIP SUIT RT HAND GEST TALK POS2");
		_meiLips = series_load("MEI PUTS FINGER TO LIPS");
		_ripGesture = series_load("RIP HNDS HIPS GEST TALK");
		player_update_info();
		ws_hide_walker();

		_ripsh2 = series_show("ripsh2", 0xf00, 128, -1, -1, 0,
			_G(player_info).scale, _G(player_info).x, _G(player_info).y);
		_machine3 = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale + 1,
			0x500, 1, triggerMachineByHashCallback, "rip");

		_G(kernel).trigger_mode = KT_DAEMON;
		sendWSMessage_10000(1, _machine3, 1, 1, 1, 300, 1, 1, 1, 0);

		_val17 = _val16 = 0;
		_G(kernel).trigger_mode = KT_PARSE;

		conv_load("conv303b", 10, 10, 747);
		conv_play(conv_get_handle());
		_val11 = 4;
		_val16 = 0;

	} else if (_G(kernel).trigger == 747) {
		midi_fade_volume(0, 120);
		kernel_timing_trigger(120, 749);
		_lonelyFlag = false;
		_val7 = 1;
		_val11 = 0;
		_val16 = 5;

	} else if (_G(kernel).trigger == 749) {
		midi_stop();

	} else if (player_said("TALK FL")) {
		_suit1 = series_load("SUIT CHIN IN HAND POS2");
		_suit2 = series_load("RIP SUIT RT HAND GEST TALK POS2");
		_ripGesture = series_load("RIP HNDS HIPS GEST TALK");
		player_update_info();
		ws_hide_walker();

		if (_fengFlag) {
			_ripsh2 = series_show("ripsh2", 0xf00, 0, -1, -1, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			_machine3 = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
				_G(player_info).x, _G(player_info).y, _G(player_info).scale + 1,
				0x500, 0, triggerMachineByHashCallback, "rip");

		} else {
			_ripsh2 = series_show("ripsh2", 0xf00, 128, -1, -1, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			_machine3 = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
				_G(player_info).x, _G(player_info).y, _G(player_info).scale + 1,
				0x500, 1, triggerMachineByHashCallback, "rip");
		}

		_G(kernel).trigger_mode = KT_DAEMON;
		sendWSMessage_10000(1, _machine3, 1, 1, 1, 300, 1, 1, 1, 0);
		_val16 = _val17 = 0;

		_G(kernel).trigger_mode = KT_PARSE;
		player_set_commands_allowed(false);
		conv_load("conv303a", 10, 10, 748);

		conv_export_value(conv_get_handle(), _G(flags)[V086], 0);
		conv_play();
		_fengMode = 1;
		_val16 = 0;

	} else if (_G(kernel).trigger == 748) {
		_G(flags)[V082] = 1;
		_G(flags)[V083] = 1;
		midi_fade_volume(0, 120);
		kernel_timing_trigger(120, 749);
		_lonelyFlag = false;
		_fengMode = 4;
		_val16 = 5;

	} else if (player_said("exit left")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;

		case 1:
			digi_stop(3);
			_G(game).setRoom(305);
			break;

		default:
			break;
		}
	} else if (lookFlag && _G(walker).ripley_said(SAID)) {
		// Already handled
	} else if (player_said("exit right")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;

		case 1:
			digi_stop(3);
			_G(game).setRoom(301);
			break;

		default:
			break;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room303::setupMei() {
	_mei1 = series_load("MC NY hands behind back pos4");
	_mei2 = series_load("MC NY hand on hip pos4");
	_mei3 = series_load("MC NY hand out talk pos4");
	_mei = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 357, 255, 86, 0xf00, 0,
		triggerMachineByHashCallback, "mc");

	_G(kernel).trigger_mode = KT_DAEMON;
	sendWSMessage_10000(1, _mei, _mei1, 1, 1, 200, _mei1, 1, 1, 0);
	_val10 = _val11 = 0;
}

void Room303::loadFengLi() {
	_feng1 = series_load("hands clasped pos5");
	_feng2 = series_load("shrug head shake pos5");
	_feng3 = series_load("hands clasped bow pos5");
	_feng4 = series_load("hands clasped flame loop pos5");
}

void Room303::setFengActive(bool flag) {
	for (HotSpotRec *hotspot = _G(currentSceneDef).hotspots;
		hotspot; hotspot = hotspot->next) {
		if (!strcmp(hotspot->vocab, "FENG LI")) {
			if (flag) {
				hotspot->active = hotspot->lr_x < 600;
			} else {
				hotspot->active = hotspot->lr_x > 600;
			}
			break;
		}
	}
}

void Room303::setShadow4(bool active) {
	if (active) {
		_shadow4 = series_place_sprite("candleman shadow4", 0, 360, 252, -86, 0xe06);
	} else {
		terminateMachineAndNull(_shadow4);
	}
}

void Room303::setShadow5(bool active) {
	if (active) {
		_shadow5 = series_place_sprite("candleman shadow5", 0, 480, 256, -84, 0xe06);
	} else {
		terminateMachineAndNull(_shadow5);
	}
}

void Room303::setShadow5Alt(bool active) {
	if (active) {
		_shadow5 = series_place_sprite("candleman shadow5", 0, 706, 256, 84, 0xe06);
	} else {
		terminateMachineAndNull(_shadow5);
	}
}

void Room303::escapePressed(void *, void *) {
	_G(kernel).trigger_mode = KT_DAEMON;
	disable_player_commands_and_fade_init(56);
}

void Room303::playSeries(bool cow) {
	series_plain_play("SPINNING TOMATO MAN", -1, 0, 100, 0, 7);
	series_plain_play("PUFFBALL", -1, 0, 100, 0, 8);
	series_plain_play("CREATURE FEATURE LONG VIEW", 1, 0, 100, 0xf05, 7, 70);
	series_plain_play("303cow1", -1, 0, 100, 0, 9);
}

void Room303::conv303a() {
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();
	const char *sound = conv_sound_to_play();

	if (_G(kernel).trigger == 1) {
		if (who <= 0) {
			_fengMode = node != 3 || entry ? 1 : 0;

		} else if (who == 1) {
			if ((node == 0 && entry != 0) || (node == 0 && entry == 1)) {
				if (!_lonelyFlag) {
					midi_play("ppstreet", 140, 1, -1, 949);
				}
			}

			if (node != 1 || entry != 1) {
				if (node != 3 || entry != 1)
					_val16 = 0;
			} else {
				_val16 = 3;
			}
		}

		conv_resume();

	} else if (!conv_sound_to_play()) {
		conv_resume();

	} else {
		if (who <= 0) {
			if (node == 3 && !entry)
				_val16 = 2;
			if ((node != 3 || entry != 1) && (node != 3 || entry))
				_fengMode = 2;
			else
				_fengMode = 3;

		} else if (who == 1) {
			if (node != 1 || entry != 1) {
				if (node != 5 || entry) {
					if (node != 3 || entry != 1)
						_val16 = 1;
				} else {
					_val16 = 0;
				}
			} else {
				_val16 = 4;
			}
		}

		digi_play(sound, 1, 255, 1);
	}
}

void Room303::conv303b() {
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();
	const char *sound = conv_sound_to_play();

	switch (_G(kernel).trigger) {
	case 1:
		if (who <= 0) {
			_val11 = 4;

			if (node == 1 && entry == 0) {
				digi_unload("08_01n01");
				digi_unload("08_02n01");
			} else if (node == 2 && entry == 2) {
				_val16 = 0;
			} else if (node == 1 && entry == 2) {
				digi_preload("com119");
				_ripPonders = series_stream("303 rip reacts", 4, 0, 667);
				series_stream_break_on_frame(_ripPonders, 5, 7);
				return;
			}

		} else if (who == 1) {
			if ((node == 0 && entry == 0) || (node == 0 && entry == 1)) {
				if (!_lonelyFlag) {
					midi_play("lonelyme", 140, 1, -1, 949);
					_lonelyFlag = true;
				}
			}

			if (node == 0 && entry == 0) {
				series_unload(2);
				series_unload(3);
				series_unload(4);
				_ripPonders = series_stream("303pu01", 4, 0x100, 666);
				series_stream_break_on_frame(_ripPonders, 5, 700);
			} else if (node == 1 && entry == 2) {
				// No implementation
			} else if ((node == 2 && entry == 0) || (node == 2 && entry == 2)) {
				_val16 = 3;
			} else {
				_val16 = 0;
			}
		}
		break;

	case 2:
		series_set_frame_rate(_ripPonders, 20);
		series_stream_break_on_frame(_ripPonders, 27, 3);
		return;

	case 3:
		series_set_frame_rate(_ripPonders, 5);
		return;

	case 4:
		_val11 = 5;
		return;

	case 5:
		digi_play("com119", 1, 255, 6);
		return;

	case 6:
		digi_unload("com119");
		return;

	case 7:
		_val16 = 0;
		series_stream_break_on_frame(_ripPonders, 22, 5);
		return;

	case 666:
		kernel_timing_trigger(1, 668);
		break;

	case 667:
		kernel_timing_trigger(1, 670);
		break;

	case 668:
		conv_resume();
		digi_preload("08_01n01");
		digi_preload("08_02n01");
		_ripPonders = series_stream("303 rip ponders", 5, 0, -1);
		series_stream_break_on_frame(_ripPonders, 5, 2);
		return;

	case 670:
		series_load("test1");
		series_load("test3");
		series_load("test4");
		series_load("test5");
		break;

	case 700:
		_val16 = 0;
		break;

	default:
		if (sound) {
			if (who <= 0) {
				if (node != 2 || entry != 1)
					_val11 = 5;
			} else if (who == 1) {
				if (node == 1 && entry == 2) {
					_val16 = 2;
				} else if (node == 2 && entry == 1) {
					_val11 = 6;
					kernel_timing_trigger(150, 4);
				} else if ((node == 2 && entry == 0) ||
						(node == 2 && entry == 2)) {
					_val16 = 4;
				} else {
					_val16 = 1;
				}
			}

			digi_play(sound, 1, 255, 1);
			return;
		}
		break;
	}

	conv_resume();
}

void Room303::priestTalkCallback(frac16 myMessage, machine *sender) {
	Room303 *room = (Room303 *)g_engine->_activeRoom;
	auto oldMode = _G(kernel).trigger_mode;
	int trigger = myMessage >> 16;

	if (trigger > 0) {
		_G(kernel).trigger_mode = room->_val12;
		kernel_timing_trigger(1, trigger);
		_G(kernel).trigger_mode = oldMode;
	}
}

void Room303::priestTalk(bool flag, int trigger) {
	ws_hide_walker(_fengLi);
	_G(globals)[GLB_TEMP_1] = _feng1 << 24;
	_G(globals)[GLB_TEMP_2] = 0xD << 24;
	_G(globals)[GLB_TEMP_3] = _feng3 << 24;
	_G(globals)[GLB_TEMP_4] = 0xD << 24;
	_G(globals)[GLB_TEMP_9] = _feng4 << 24;
	_G(globals)[GLB_TEMP_10] = 0xD << 24;
	_G(globals)[GLB_TEMP_5] = (flag ? 480 : 706) << 16;
	_G(globals)[GLB_TEMP_6] = 1 << 24;
	_G(globals)[GLB_TEMP_7] = MulSF16((1 << 24) - _G(globals)[GLB_MIN_Y],
		_G(globals)[GLB_SCALER]) + _G(globals)[GLB_MIN_SCALE];
	_G(globals)[GLB_TEMP_8] = (flag ? 1 : 0) << 16;
	_G(globals)[GLB_TEMP_11] = trigger << 16;
	_G(globals)[GLB_TEMP_12] = 0xdc28;

	_priestTalk = TriggerMachineByHash(32, nullptr, -1, -1,
		priestTalkCallback, false, "fl priest/talk");
}

int Room303::getSize(const Common::String &assetName, int roomNum) {
	Common::String name = expand_name_2_RAW(assetName, roomNum);
	size_t fileSize = f_info_get_file_size(Common::Path(name));

	return static_cast<int>((double)fileSize * 0.000090702946);
}

void Room303::playSound(const Common::String &assetName, int trigger1, int trigger2) {
	if (!trigger2)
		trigger2 = -1;

	int size = MAX(getSize(assetName), 0);
	_G(globals)[GLB_TEMP_1] = size << 16;
	_G(globals)[GLB_TEMP_2] = trigger2 << 16;
	sendWSMessage(0x200000, 0, _priestTalk, 0, nullptr, 1);

	digi_play(assetName.c_str(), 1, 255, trigger1);
}

void Room303::syncGame(Common::Serializer &s) {
	s.syncAsByte(_fengFlag);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
