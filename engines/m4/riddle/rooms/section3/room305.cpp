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

#include "m4/riddle/rooms/section3/room305.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"
#include "m4/gui/gui_vmng.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room305::preload() {
	if (_G(flags)[V000]) {
		_G(art_base_override) = "CARTOON ROOM-TREK";
		_G(use_alternate_attribute_file) = false;
		_G(player).walker_type = 1;
		_G(player).shadow_type = 1;
	} else {
		_G(player).walker_type = 0;
		_G(player).shadow_type = 0;
	}

	LoadWSAssets("other script");
}

void Room305::init() {
	hotspot_set_active("TURTLE TREATS", false);
	digi_preload("305_s01");
	digi_preload("305_s02");

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = -1;
		_triggerMode1 = KT_DAEMON;
		_triggerMode2 = KT_DAEMON;
		_val2 = 0;
		_val3 = 0;
		_val4 = 0;
	}

	_val5 = 0;
	_ripMedHigh = series_load("RIP MED HIGH REACHER POS2");
	_ripLooksDown = series_load("RIP LOOKS DOWN POS1");

	if (inv_object_is_here("TURTLE"))
		_turtle = series_show_sprite("395turtl", 0, 0xf00);
	else
		hotspot_set_active("TURTLE", false);

	setupSign();

	_shrunkenHead1 = series_load("DISPLAY CASE SHRUNKEN HEAD");
	_incenseHolder1 = series_load("DISPLAY CASE INCENSE HOLDER");
	_crystalSkull1 = series_load("DISPLAY CASE CRYSTAL SKULL");
	_whaleboneHorn1 = series_load("DISPLAY CASE WHALE BONE HORN");
	_wheeledToy1 = series_load("DISPLAY CASE WHEELED TOY");
	_butterfly1 = series_load("DISPLAY CASE BUTTERFLY");
	_amulet1 = series_load("DISPLAY CASE AMULET");
	_knife1 = series_load("DISPLAY CASE YETI HANDLED KNIFE");
	_banknote1 = series_load("DISPLAY CASE GERMAN BANKNOTE");
	_stamp1 = series_load("DISPLAY CASE CHEAPEST STAMP");
	_map1 = series_load("DISPLAY CASE QUARRY STICK MAP");
	_emerald1 = series_load("DISPLAY CASE EMERALD");

	if (_G(flags)[kEasterIslandCartoon]) {
		_easterIslandCartoon = series_load("SMALL EASTER ISLAND CARTOON");
		series_show_sprite("SMALL EASTER ISLAND CARTOON", 0, 0xfff);
	}
	if (_G(flags)[kChinshiCartoon]) {
		_chinshiCartoon = series_load("SMALL CHINSHI CARTOON");
		series_show_sprite("SMALL CHINSHI CARTOON", 0, 0xfff);
	}
	if (_G(flags)[kTabletsCartoon]) {
		_tabletsCartoon = series_load("SMALL TABLETS CARTOON");
		series_show_sprite("SMALL TABLETS CARTOON", 0, 0xfff);
	}
	if (_G(flags)[kEpitaphCartoon]) {
		_epitaphCartoon = series_load("SMALL EPITAPH CARTOON");
		series_show_sprite("SMALL EPITAPH CARTOON", 0, 0xfff);
	}
	if (_G(flags)[kGraveyardCartoon]) {
		_graveyardCartoon = series_load("SMALL GRAVEYARD CARTOON");
		series_show_sprite("SMALL GRAVEYARD CARTOON", 0, 0xfff);
	}
	if (_G(flags)[kCastleCartoon]) {
		_castleCartoon = series_load("SMALL CASTLE CARTOON");
		series_show_sprite("SMALL CASTLE CARTOON", 0, 0xfff);
	}
	if (_G(flags)[kMocaMocheCartoon]) {
		_mocaMocheCartoon = series_load("SMALL MOCA MOCHE CARTOON");
		series_show_sprite("SMALL MOCA MOCHE CARTOON", 0, 0xfff);
	}
	if (_G(flags)[kTempleCartoon]) {
		_templeCartoon = series_load("SMALL TEMPLE CARTOON");
		series_show_sprite("SMALL TEMPLE CARTOON", 0, 0xfff);
	}
	if (_G(flags)[kEmeraldCartoon]) {
		_emeraldCartoon = series_load("SMALL EMERALD CARTOON");
		series_show_sprite("SMALL EMERALD CARTOON", 0, 0xfff);
	}

	#define ITEM(FIELD, NAME, SPR_NAME) \
		if (inv_object_is_here(NAME)) \
			FIELD = series_show_sprite(SPR_NAME, 0, 0xfff); \
		else \
			hotspot_set_active(NAME, false)

	ITEM(_shrunkenHead2, "SHRUNKEN HEAD", "DISPLAY CASE SHRUNKEN HEAD");
	ITEM(_incenseHolder2, "INCENSE BURNER", "DISPLAY CASE INCENSE HOLDER");
	ITEM(_crystalSkull2, "CRYSTAL SKULL", "DISPLAY CASE CRYSTAL SKULL");
	ITEM(_whaleboneHorn2, "WHALE BONE HORN", "DISPLAY CASE WHALE BONE HORN");
	ITEM(_wheeledToy2, "WHEELED TOY", "DISPLAY CASE WHEELED TOY");
	ITEM(_butterfly2, "SILVER BUTTERFLY", "DISPLAY CASE BUTTERFLY");
	ITEM(_amulet2, "REBUS AMULET", "DISPLAY CASE AMULET");
	ITEM(_knife2, "CHISEL", "DISPLAY CASE YETI HANDLED KNIFE");
	ITEM(_banknote2, "GERMAN BANKNOTE", "DISPLAY CASE GERMAN BANKNOTE");
	ITEM(_stamp2, "POSTAGE STAMP", "DISPLAY CASE CHEAPEST STAMP");
	ITEM(_map2, "STICK AND SHELL MAP", "DISPLAY CASE QUARRY STICK MAP");
	ITEM(_emerald2, "ROMANOV EMERALD", "DISPLAY CASE EMERALD");
	#undef ITEM

	series_play(_G(flags)[V000] == 1 ? "395 jelly beans" : "jelly beans",
		0, 0, 3, 7, 0, 100, 0, 0, 49, 61);
	_jellyBeans = series_load("FLTD35");

	if (_G(flags)[V000] == 1) {
		if (player_been_here(201)) {
			_feng1 = series_load("feng trek hands clasped pos5");
			_feng2 = series_load("feng trek bow pos5");
			_feng3 = series_load("feng trek flame loop pos5");
		}

		_rip1 = series_load("RIP TREK HANDS HIPS POS2");
		_rip2 = series_load("RIP TREK HAND ON CHIN POS2");
		_rip3 = series_load("RIP TREK TWO HAND TALK POS2");
	} else {
		_suit1 = series_load("SUIT CHIN IN HAND POS2");
		_suit2 = series_load("RIP SUIT RT HAND GEST TALK POS2");
		_suit3 = series_load("RIP HNDS HIPS GEST TALK");
	}

	int32 status;
	ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		if (player_been_here(201)) {
			_stander = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 494, 278, 73, 0xf00, 1,
				triggerMachineByHashCallback, "fl stander");
			sendWSMessage_10000(1, _stander, _feng3, 1, 1, 400, _feng3, 1, 6, 0);
			_val6 = _val7 = 1;
			setShadow5(true);
		} else {
			hotspot_set_active("FENG LI", false);
		}

		if (_val4) {
			ws_demand_facing(11);
			player_update_info();

			if (_G(flags)[V000]) {
				_rip4 = series_load("RIP TREK MED REACH HAND POS1");
				setGlobals1(_rip4, 5, 5, 5, 5, 0, 5, 1, 1, 1);
				sendWSMessage_110000(1);
			} else {
				ws_hide_walker();
				_rip4 = series_load("SUIT RIP REACHES FOR DRAWER");
				_rip5 = series_show("ripsh1", 0xf00, 0, -1, -1, 0,
					_G(player_info).scale, _G(player_info).x, _G(player_info).y);
				_rip6 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xf00, false,
					triggerMachineByHashCallback, "rip reach");
				sendWSMessage_10000(1, _rip6, _rip4, 5, 5, -1, _rip4, 5, 5, 0);
			}

			hotspot_hide_all();
			mouse_set_sprite(0);
			hotspot_add_dynamic("LOOK AT", " ", 0, 0, 1500, 480, 0);

			if (inv_object_is_here("TURTLE TREATS")) {
				hotspot_add_dynamic("LOOK AT", "TURTLE TREATS", 1105, 208, 1175, 266, 6);
				hotspot_add_dynamic("LOOK AT", "TURTLE TREATS", 1052, 230, 1147, 296, 6);
			}

			_openDrawer = series_show_sprite("open drawer", 0, 0);

			if (inv_object_is_here("TURTLE TREATS"))
				_openDrawerTreats = series_show_sprite("open drawer with treats", 0, 0);
		}
		break;

	case 303:
		player_set_commands_allowed(false);

		if (player_been_here(301)) {
			ws_demand_location(1320, 296, 9);

			if (player_been_here(201)) {
				_stander = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 494, 278, 73, 0xf00, true,
					triggerMachineByHashCallback, "fl stander");
				_G(kernel).trigger_mode = KT_DAEMON;
				sendWSMessage_10000(1, _stander, _feng3, 1, 1, 400, _feng3, 1, 6, 0);
				_val6 = _val7 = 1;
				setShadow5(true);
			} else {
				hotspot_set_active("FENG LI", false);
			}

			MoveScreenDelta(game_buff_ptr, -640, 0);
			kernel_timing_trigger(1, 10);
		} else {
			ws_demand_location(1320, 296, 9);
			hotspot_set_active("FENG LI", false);
			kernel_timing_trigger(1, 10);
			MoveScreenDelta(game_buff_ptr, -640, 0);

		}
		break;

	default:
		ws_demand_location(1180, 320, 9);
		hotspot_set_active("FENG LI", false);
		break;
	}

	digi_play("305_s01", 3, 140);
}

void Room305::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case 2:
		series_play(_G(flags)[V000] == 1 ? "395 jelly beans" : "jelly beans",
			0, 0, 3, 7, 0, 100, 0, 0, 49, 61);
		digi_play("305_s01", 3, 140);
		break;

	case 3:
		series_play(_G(flags)[V000] == 1 ? "395 jelly beans" : "jelly beans",
			0, 0, 2, 7, 0, 100, 0, 0, 0, 48);
		break;

	case 10:
		ws_walk(1220, 296, 0, 50, 9);
		break;

	case 40:
		sendWSMessage_10000(_stander, 1199, 296, 9, 41, 0);
		break;

	case 41:
		sendWSMessage_10000(_stander, 494, 278, 8, 42, 1);
		break;

	case 42:
		sendWSMessage_60000(_stander);
		_stander = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 494, 278, 73, 0xf00, 1,
			triggerMachineByHashCallback, "fl stander");
		_G(kernel).trigger_mode = KT_DAEMON;
		sendWSMessage_10000(1, _stander, _feng3, 1, 1, 400, _feng3, 1, 6, 1);
		_val6 = _val7 = 1;
		setShadow5(true);
		_G(player).disable_hyperwalk = false;
		break;

	case 50:
		player_set_commands_allowed(true);
		break;

	case 200:
		if (!_val8 && !_conv1 && _val1 != -1) {
			kernel_trigger_dispatchx(_val1);
			_val1 = -1;

			if (_val2) {
				ws_unhide_walker();
				_val2 = 0;
			}
		}

		kernel_trigger_dispatchx(kernel_trigger_create(201));
		break;

	case 201:
		switch (_val8) {
		case 0:
			switch (_conv1) {
			case 0:
				sendWSMessage_10000(1, _rip6, 1, 1, 1, 200, 1, 1, 1, 0);
				break;

			case 1:
				sendWSMessage_10000(1, _rip6, _rip3, 1, 6, 200, _rip3, 6, 6, 0);
				_val8 = 1;
				break;

			case 3:
				sendWSMessage_10000(1, _rip6, _rip2, 1, 16, 200, _rip2, 16, 16, 0);
				_val8 = 3;
				break;

			case 5:
				sendWSMessage_10000(1, _rip6, _rip1, 1, 12, 200, _rip1, 12, 12, 0);
				_val8 = 5;
				break;

			case 7:
				terminateMachineAndNull(_rip6);
				terminateMachineAndNull(_rip5);
				ws_unhide_walker();
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
			break;

		case 1:
			if (_conv1 == 1) {
				sendWSMessage_10000(1, _rip6, _rip3, 6, 6, 200, _rip3, 6, 6, 0);
			} else {
				sendWSMessage_10000(1, _rip6, _rip3, 6, 1, 200, 1, 1, 1, 0);
				_val8 = 5;
			}
			break;

		case 3:
			if (_conv1 == 3) {
				sendWSMessage_10000(1, _rip6, _rip2, 16, 16, 200, _rip2, 16, 16, 0);
			} else {
				sendWSMessage_10000(1, _rip6, _rip2, 16, 1, 200, 1, 1, 1, 0);
				_val8 = 0;
			}
			break;

		case 5:
			if (_conv1 == 5) {
				sendWSMessage_10000(1, _rip6, _rip1, 12, 12, 200, _rip1, 12, 12, 0);
			} else {
				sendWSMessage_10000(1, _rip6, _rip1, 12, 1, 200, 1, 1, 1, 0);
				_val8 = 0;
			}
			break;

		default:
			break;
		}
		break;

	case 300:
		if (!_val8 && !_conv1 && _val1 != -1) {
			kernel_trigger_dispatchx(_val1);
			_val1 = -1;

			if (_val2) {
				ws_unhide_walker();
				_val2 = 0;
			}
		}

		kernel_trigger_dispatchx(kernel_trigger_create(301));
		break;

	case 301:
		switch (_val8) {
		case 0:
			switch (_conv1) {
			case 0:
				sendWSMessage_10000(1, _rip6, 1, 1, 1, 300, 1, 1, 1, 0);
				break;

			case 1:
				sendWSMessage_10000(1, _rip6, _suit2, 1, 10, 300, _suit2, 10, 10, 0);
				_val8 = 1;
				break;

			case 3:
				sendWSMessage_10000(1, _rip6, _suit1, 1, 17, 300, _suit1, 17, 17, 0);
				_val8 = 3;
				break;

			case 5:
			case 6:
				sendWSMessage_10000(1, _rip6, _suit3, 1, 14, 300, _suit3, 14, 14, 0);
				_val8 = 5;
				break;

			case 7:
				terminateMachineAndNull(_rip6);
				terminateMachineAndNull(_rip5);
				ws_unhide_walker();
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
			break;

		case 1:
			if (_conv1 == 1) {
				sendWSMessage_10000(1, _rip6, _suit2, 10, 10, 300, _suit2, 10, 10, 0);
			} else {
				sendWSMessage_10000(1, _rip6, _suit2, 11, 18, 300, 1, 1, 1, 0);
				_val8 = 0;
			}
			break;

		case 3:
			if (_conv1 == 3) {
				sendWSMessage_10000(1, _rip6, _suit1, 17, 17, 300, _suit1, 17, 17, 0);
			} else {
				sendWSMessage_10000(1, _rip6, _suit1, 17, 1, 300, 1, 1, 1, 0);
				_val8 = 0;
			}
			break;

		case 5:
			switch (_conv1) {
			case 5:
				sendWSMessage_10000(1, _rip6, _suit3, 14, 14, 300, _suit3, 14, 14, 0);
				break;

			case 6:
				sendWSMessage_10000(1, _rip6, _suit3, 15, 25, 300, _suit3, 25, 25, 0);
				_val8 = 6;
				break;

			default:
				sendWSMessage_10000(1, _rip6, _suit3, 14, 1, 300, 1, 1, 1, 0);
				_val8 = 0;
				break;
			}
			break;

		case 6:
			if (_conv1 == 6) {
				sendWSMessage_10000(1, _rip6, _suit3, 25, 25, 300, _suit3, 25, 25, 0);
			} else {
				sendWSMessage_10000(1, _rip6, _suit3, 25, 15, 300, _suit3, 14, 14, 0);
				_val8 = 5;
			}
			break;

		default:
			break;
		}
		break;

	case 400:
		if (!_val6 && !_val7 && _val1 != -1) {
			kernel_trigger_dispatchx(_val1);
			_val1 = -1;

			if (_val2) {
				ws_unhide_walker();
				_val2 = 0;
			}
		}

		kernel_trigger_dispatchx(kernel_trigger_create(401));
		break;

	case 401:
		switch (_val6) {
		case 0:
			switch (_val7) {
			case 0:
				sendWSMessage_10000(1, _stander, _jellyBeans, 1, 1, 400, _jellyBeans, 1, 1, 0);
				break;

			case 1:
			case 2:
			case 4:
				sendWSMessage_10000(1, _stander, _feng1, 16, 1, 400, _feng3, 1, 6, 1);
				_val6 = 1;
				break;

			case 3:
				sendWSMessage_10000(1, _stander, _jellyBeans, 1, 13, 400, _jellyBeans, 14, 19, 1);
				_val6 = 3;
				break;

			default:
				break;
			}
			break;

		case 1:
			switch (_val7) {
			case 0:
				sendWSMessage_10000(1, _stander, _feng1, 16, 1, 400, _jellyBeans, 1, 1, 0);
				_val6 = 0;
				break;

			case 1:
				frame = imath_ranged_rand(1, 6);
				sendWSMessage_10000(1, _stander, _feng3, frame, frame, 400, _feng3, frame, frame, 0);
				break;

			case 2:
				sendWSMessage_10000(1, _stander, _feng2, 1, 18, 400, _feng2, 19, 21, 1);
				_val6 = 2;
				break;

			case 3:
				sendWSMessage_10000(1, _stander, _feng1, 16, 1, 400, _jellyBeans, 1, 1, 0);
				_val6 = 0;
				break;

			case 4:
				frame = imath_ranged_rand(1, 6);
				sendWSMessage_10000(1, _stander, _feng3, frame, frame, 400, _feng3, frame, frame, 0);
				_val7 = 1;
				break;

			default:
				sendWSMessage_10000(1, _stander, _feng2, 1, 18, 400, _feng2, 19, 21, 1);
				_val6 = 2;
				break;
			}
			break;

		case 2:
			if (_val7 == 2) {
				frame = imath_ranged_rand(19, 21);
				sendWSMessage_10000(1, _stander, _feng2, frame, frame, 400, _feng2, frame, frame, 0);
			} else {
				sendWSMessage_10000(1, _stander, _feng2, 18, 1, 400, _feng3, 1, 6, 0);
				_val6 = 1;
				break;
			}
			break;

		case 3:
			if (_val7 == 3) {
				frame = imath_ranged_rand(14, 19);
				sendWSMessage_10000(1, _stander, _jellyBeans, frame, frame, 400, _jellyBeans, frame, frame, 0);
			} else {
				sendWSMessage_10000(1, _stander, _jellyBeans, 20, 31, 400, _jellyBeans, 1, 1, 0);
				_val6 = 0;
			}
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}

void Room305::pre_parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");

	if (_val4 && !(takeFlag && player_said("turtle treats"))
			&& !(lookFlag && player_said("turtle treats"))) {
		player_set_commands_allowed(false);
		Common::strcpy_s(_G(player).verb, "close");
		Common::strcpy_s(_G(player).noun, "drawer");
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	#define SAID(NAME) (player_said(NAME) && inv_player_has(NAME))
	if (SAID("SHRUNKEN HEAD") || SAID("INCENSE BURNER") ||
			SAID("CRYSTAL SKULL") || SAID("CRYSTAL SKULL)") ||
			SAID("WHALE BONE HORN") || SAID("WHEELED TOY") ||
			SAID("CHISEL") || SAID("ROMANOV EMERALD") ||
			SAID("GERMAN BANKNOTE") || SAID("REBUS AMULET") ||
			SAID("SILVER BUTTERFLY") || SAID("POSTAGE STAMP") ||
			SAID("STICK AND SHELL MAP")) {
		if (player_said("DISPLAY CASE")) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	}
	#undef SAID

	if (lookFlag && player_said("cartoon"))
		_G(camera_reacts_to_player) = false;

	if (_val5) {
		_G(camera_reacts_to_player) = true;
		terminateMachineAndNull(_cartoonMach);
		series_unload(_cartoon);
		intr_cancel_sentence();
		_val5 = 0;
		hotspot_restore_all();
		interface_show();
	}
}

void Room305::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");
	bool itemFlag = false;
	bool chiselFlag = false;
	bool caseFlag = false;

	if (player_said("conv305a")) {
		conv305a();
	} else if (player_said("TALK FL")) {
		g_engine->camera_shift_xy(160, 0);
		player_update_info();
		ws_hide_walker();

		_rip6 = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale + 1,
			0x500, 0, triggerMachineByHashCallback, "rip");
		_G(kernel).trigger_mode = KT_DAEMON;
		sendWSMessage_10000(1, _rip6, 1, 1, 1,
			_G(flags)[V000] == 1 ? 200 : 300,
			1, 1, 1, 0);

		_val8 = 0;
		_conv1 = 0;
		_G(kernel).trigger_mode = KT_PARSE;
		player_set_commands_allowed(false);

		_rip5 = series_show("safari shadow 2", 0xf00, 0, -1, -1, 0,
			_G(player_info).scale, _G(player_info).x, _G(player_info).y);
		conv_load("conv305a", 10, 10, 747);
		conv_export_value_curr(_G(flags)[V088], 0);
		conv_export_value_curr(_G(flags)[V089], 1);
		conv_export_value_curr(
			getNumKeyItemsPlaced() != 0 || inv_object_is_here("ROMANOV EMERALD") ? 1 : 0,
			2);
		conv_export_value_curr(inv_object_in_scene("TURTLE", 305) ? 1 : 0, 3);

		if (_G(flags)[V090] == 0 || inv_player_has("ROMANOV EMERALD") ||
			inv_object_is_here("ROMANOV EMERALD")) {
			conv_export_value_curr(0, 7);
		} else if (_G(flags)[V090] == 1) {
			conv_export_value_curr(1, 7);
		} else if (_G(flags)[V090] == 2) {
			conv_export_value_curr(2, 7);
		} else if (_G(flags)[V090] == 3) {
			conv_export_value_curr(2, 7);
		}

		conv_export_value_curr(_G(flags)[V141], 8);
		conv_play();
		_val7 = 1;
		_conv1 = 0;
	} else if (_G(kernel).trigger == 747) {
		_val7 = 4;
		_conv1 = 7;
	} else if (player_said("close", "drawer")) {
		switch (_G(kernel).trigger) {
		case -1:
			if (inv_object_is_here("TURTLE TREATS"))
				terminateMachineAndNull(_openDrawerTreats);

			terminateMachineAndNull(_openDrawer);
			hotspot_restore_all();

			if (_G(flags)[V000]) {
				sendWSMessage_120000(1);
			} else {
				sendWSMessage_10000(1, _rip6, _rip4, 5, 1, 1, _rip4, 1, 1, 0);
			}
			break;

		case 1:
			if (_G(flags)[V000]) {
				sendWSMessage_150000(-1);
			} else {
				terminateMachineAndNull(_rip6);
				terminateMachineAndNull(_rip5);
				ws_unhide_walker();
			}
			kernel_timing_trigger(1, 2);
			break;

		case 2:
			series_unload(_rip4);
			player_set_commands_allowed(true);
			_val4 = 0;
			break;

		default:
			break;
		}
	} else if (useFlag && player_said("drawer")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			player_update_info();

			if (_G(flags)[V000]) {
				_rip4 = series_load("RIP TREK MED REACH HAND POS1");
				setGlobals1(1, 5, 5, 5, 0, 5, 1, 1, 1);
				sendWSMessage_110000(1);
			} else {
				ws_hide_walker();
				_rip4 = series_load("SUIT RIP REACHES FOR DRAWER");
				_rip5 = series_show("ripsh1", 0xf00, 128, -1, -1, 0,
					_G(player_info).scale, _G(player_info).x, _G(player_info).y);
				_rip6 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xf00, false,
					triggerMachineByHashCallback, "rip reach");

				sendWSMessage_10000(1, _rip6, _rip4, 1, 5, 1, _rip4, 5, 5, 0);
			}
			break;

		case 1:
			hotspot_hide_all();
			intr_cancel_sentence();
			mouse_set_sprite(0);
			hotspot_add_dynamic("LOOK AT", " ", 0, 0, 1500, 374, 0);

			if (inv_object_is_here("TURTLE TREATS")) {
				hotspot_add_dynamic("LOOK AT", "TURTLE TREATS", 1105, 208, 1175, 266, 6);
				hotspot_add_dynamic("LOOK AT", "TURTLE TREATS", 1052, 230, 1147, 296, 6);
			}

			_val4 = 1;
			_openDrawer = series_show_sprite("open drawer", 0, 0);

			if (inv_object_is_here("TURTLE TREATS")) {
				_openDrawerTreats = series_show_sprite("open drawer with treats", 0, 0);
			}

			player_set_commands_allowed(true);
			digi_play("305_s02", 2);
			break;

		default:
			break;
		}
	} else if (takeFlag && player_said("turtle treats") && _G(kernel).trigger == -1
			&& inv_object_is_here("TURTLE TREATS")) {
		terminateMachineAndNull(_openDrawerTreats);
		terminateMachineAndNull(_openDrawer);
		hotspot_restore_all();

		if (_G(flags)[V000])
			sendWSMessage_120000(1);
		else
			sendWSMessage_10000(1, _rip6, _rip4, 5, 1, 1, _rip4, 1, 1, 0);

		digi_play("305r20", 1);

	} else if (takeFlag && player_said("turtle treats") && _G(kernel).trigger == 1) {
		inv_give_to_player("TURTLE TREATS");
		terminateMachineAndNull(_rip6);
		terminateMachineAndNull(_rip5);
		ws_unhide_walker();
		kernel_timing_trigger(1, 2);

	} else if (takeFlag && player_said("turtle treats") && _G(kernel).trigger == 2) {
		series_unload(_rip4);
		player_set_commands_allowed(true);
		_val4 = 0;

	} else if (takeFlag && player_said("turtle")) {
		if (_G(flags)[GLB_TEMP_12]) {
			digi_play("305r55", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				if (inv_object_is_here("TURTLE")) {
					player_set_commands_allowed(false);
					g_engine->camera_shift_xy(640, 0);
					setGlobals1(_ripLooksDown, 1, 9, 9, 9);
					sendWSMessage_110000(1);
				} else {
					goto next1;
				}
				break;

			case 1:
				kernel_examine_inventory_object("ping turtle",
					_G(master_palette), 5, 1, 407, 25, 2,
					"305R19", -1);
				terminateMachineAndNull(_turtle);
				break;

			case 2:
				inv_give_to_player("TURTLE");
				sendWSMessage_140000(3);
				break;

			case 3:
				hotspot_set_active("turtle", false);
				player_set_commands_allowed(true);
				break;
			default:
				goto next1;
				break;
			}
		}
	} else {
		goto next1;
	}
	goto exit;

next1:
	if (player_said_any("SHRUNKEN HEAD", "INCENSE BURNER",
			"CRYSTAL SKULL", "ROMANOV EMERALD", "WHALE BONE HORN",
			"WHEELED TOY")) {
		if (player_said("DISPLAY CASE"))
			itemFlag = true;
	}

	if (itemFlag || (takeFlag && player_said_any("SHRUNKEN HEAD",
			"INCENSE BURNER", "CRYSTAL SKULL", "ROMANOV EMERALD",
			"WHALE BONE HORN"))) {
		switch (_G(kernel).trigger) {
		case -1:
			if ((itemFlag && inv_player_has(_G(player).verb)) || takeFlag) {
				if (itemFlag) {
					if (!walkToObject())
						goto next2;
				} else {
					kernel_timing_trigger(1, 1);
				}
			}
			break;

		default:
			break;
		}
	} else {
		goto next2;
	}
	goto exit;

next2:
	chiselFlag = player_said("CHISEL") && player_said("DISPLAY CASE");
	if (chiselFlag || (takeFlag && player_said("CHISEL"))) {
		switch (_G(kernel).trigger) {
		case -1:
			if ((chiselFlag && inv_player_has(_G(player).verb)) ||
				(takeFlag && inv_object_is_here(_G(player).noun))) {
				if (chiselFlag) {
					if (player_said("CHISEL")) {
						ws_walk(186, 279, 0, 1, 10);
					}
				} else {
					kernel_timing_trigger(1, 1);
				}
			}
			break;

		case 1:
			player_set_commands_allowed(false);
			setGlobals1(_ripMedHigh, 1, 12, 12, 12);
			sendWSMessage_110000(3);
			break;

		case 3:
			if (chiselFlag) {
				hotspot_set_active(_G(player).verb, false);

				if (player_said("CHISEL")) {
					_knife2 = series_show_sprite("DISPLAY CASE YETI HANDLED KNIFE", 0, 0xf00);
					inv_move_object("CHISEL", 305);
				}
			} else {
				hotspot_set_active(_G(player).noun, false);

				if (player_said("CHISEL")) {
					terminateMachineAndNull(_knife2);
					inv_give_to_player("CHISEL");
				}
			}

			sendWSMessage_140000(5);
			break;

		case 5:
			player_set_commands_allowed(true);
			break;

		default:
			goto next3;
			break;
		}
	} else {
		goto next3;
	}
	goto exit;

next3:
	caseFlag = player_said_any("GERMAN BANKNOTE", "REBUS AMULET", "SILVER BUTTERFLY",
		"POSTAGE STAMP", "STICK AND SHELL MAP") && player_said("DISPLAY CASE");
	// The second or parameter looks redundant, but I'm keeping it in as a reminder,
	// just in case there's something actually different in the original disassembly
	if (caseFlag || (takeFlag && caseFlag)) {
		switch (_G(kernel).trigger) {
		case -1:
			if ((caseFlag && inv_player_has(_G(player).verb)) ||
					(takeFlag && inv_object_is_here(_G(player).noun))) {
				if (caseFlag) {
					if (player_said("GERMAN BANKNOTE"))
						ws_walk(88, 305, nullptr, 1, 10);
					else if (player_said("REBUS AMULET"))
						ws_walk(66, 319, nullptr, 1, 10);
					else if (player_said("SILVER BUTTERFLY"))
						ws_walk(124, 302, nullptr, 1, 10);
					else if (player_said("POSTAGE STAMP"))
						ws_walk(164, 288, nullptr, 1, 10);
					else if (player_said("STICK AND SHELL MAP"))
						ws_walk(120, 299, nullptr, 1, 10);
				} else {
					kernel_timing_trigger(1, 1);
				}
			}
			break;

		case 1:
			player_set_commands_allowed(false);
			_rip4 = series_load("RIP HIGH REACHER POS2");

			if (player_said_any("REBUS AMULET", "SILVER BUTTERFLY"))
				setGlobals1(_rip4, 1, 7, 7, 7);
			else
				setGlobals1(_rip4, 1, 14, 14, 14);

			sendWSMessage_110000(3);
			break;

		case 3:
			if (useFlag) {
				hotspot_set_active(_G(player).verb, true);

#define DROP(FIELD, NAME, CASE) if (player_said(NAME)) { \
	FIELD = series_show_sprite(CASE, 0, 0xf00); \
		inv_move_object(NAME, 305); }
				DROP(_amulet2, "REBUS AMULET", "DISPLAY CASE AMULET")
				DROP(_butterfly2, "SILVER BUTTERFLY", "DISPLAY CASE BUTTERFLY")
				DROP(_banknote2, "GERMAN BANKNOTE", "DISPLAY CASE GERMAN BANKNOTE")
				DROP(_stamp2, "POSTAGE STAMP", "DISPLAY CASE CHEAPEST STAMP")
				DROP(_map2, "STICK AND SHELL MAP", "DISPLAY CASE QUARRY STICK MAP")
#undef ITEM
				sendWSMessage_140000(3);

			} else {
				hotspot_set_active(_G(player).noun, false);

#define TAKE(FIELD, NAME) if (player_said(NAME)) { \
	terminateMachineAndNull(FIELD); inv_give_to_player(NAME); }
				TAKE(_amulet2, "REBUS AMULET")
				TAKE(_butterfly2, "SILVER BUTTERFLY")
				TAKE(_banknote2, "GERMAN BANKNOTE")
				TAKE(_stamp2, "POSTAGE STAMP")
				TAKE(_map2, "STICK AND SHELL MAP")
#undef TAKE

				sendWSMessage_140000(5);
			}
			break;

		case 5:
			series_unload(_rip4);
			player_set_commands_allowed(true);
			break;

		default:
			goto next4;
			break;
		}
	} else {
		goto next4;
	}
	goto exit;

next4:
	if (player_said("TURTLE", "FENG LI")) {
		digi_play("305r40", 1);
	} else if (inv_player_has(_G(player).verb) && player_said("FENG LI")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("305f08a", 1, 255, 1);
			break;

		case 1:
			player_set_commands_allowed(true);
			_val7 = 1;
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("cartoon") && _G(kernel).trigger >= -1
			&& _G(kernel).trigger <= 5) {
		switch (_G(kernel).trigger) {
		case -1:
		case 1: {
			player_set_commands_allowed(true);
			int area = getXAreaNum();
			const int32 flags[8] = {
				_G(flags)[kEasterIslandCartoon], _G(flags)[kChinshiCartoon],
				_G(flags)[kTabletsCartoon], _G(flags)[kEpitaphCartoon],
				_G(flags)[kGraveyardCartoon], _G(flags)[kCastleCartoon],
				_G(flags)[kMocaMocheCartoon], _G(flags)[kTempleCartoon]
			};
			bool flag = flags[area - 1] != 0;
			if (flag && (area == 3 || area == 9))
				player_set_commands_allowed(true);

			interface_hide();
			hotspot_hide_all();
			mouse_set_sprite(0);
			hotspot_add_dynamic("LOOK AT", " ", 0, 0, 1500, 480, 0);

			Common::String digiName;
			if (flag) {
				_G(flags)[V089] = 2;
				_cartoon = series_load(getXAreaCartoon(), -1, _G(master_palette));
				_cartoonMach = series_place_sprite(getXAreaCartoon(),
					0, getItemX(_cartoon), getItemY(_cartoon),
					100, 0x100);
				digiName = getXAreaDigi();

			} else {
				_cartoon = series_load(getXAreaSeries(), -1, _G(master_palette));
				_cartoonMach = series_place_sprite(getXAreaSeries(), 0,
					getItemX(_cartoon), getItemY(_cartoon), 100, 0x100);
				digiName = getXAreaDigi2();
			}

			if (getXAreaNum() == 3 && caseFlag) {
				digi_play("305r23", 1, 255, 2);
			} else if (getXAreaNum() == 9 && caseFlag) {
				digi_play("305r29", 1, 255, 3);
			} else {
				digi_play(digiName.c_str(), 1);
			}

			_val5 = 1;
			break;
		}

		case 2:
			digi_play("305r23a", 1);
			player_set_commands_allowed(true);
			break;

		case 3:
			if (_G(flags)[V141]) {
				digi_play("305r29a", 1, 255, 4);
			} else {
				digi_play("305r29a", 1);
			}
			break;

		case 4:
			digi_play("305r29a", 1);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("mona lisa")) {
		if (_G(flags)[V086] == 2) {
			_G(flags)[V086] = 3;
		} else if (_G(flags)[V086] == 1) {
			_G(flags)[V086] = 0;
		}

		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			if (_G(flags)[V000] == 1)
				setGlobals4(_rip2, 16, 16, 16);
			else
				setGlobals4(_suit1, 17, 17, 17);

			sendWSMessage_C0000(0);

			if (_G(flags)[V087]) {
				digi_play("305r31", 1, 255, 2);
			} else if (_G(flags)[V083]) {
				digi_play("305r11", 1, 255, 2);
			} else {
				digi_play("305r10", 1, 255, 2);
			}

			_G(flags)[V087] = 1;
			break;

		case 2:
			sendWSMessage_B0000(3);
			break;

		case 3:
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("hourglass")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);

			if (_G(flags)[V000] == 1) {
				_lookUp = series_load("rip looks up pos3");
				setGlobals4(_lookUp, 6, 6, 6);
			} else {
				_lookUp = series_load("rpsd103");
				setGlobals4(_lookUp, 7, 7, 7);
			}

			sendWSMessage_C0000(0);
			digi_play("305r13", 1, 255, 2);
			break;

		case 2:
			sendWSMessage_B0000(3);
			break;

		case 3:
			series_unload(_lookUp);
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("display case")) {
		switch (_G(kernel).trigger) {
		case -1:
			if (_G(flags)[V000]) {
				if (getNumKeyItemsPlaced() != 0 || inv_object_is_here("ROMANOV EMERALD")) {
					digi_play("305r32", 1);
				} else if (player_been_here(201)) {
					digi_play("305f03", 1, 255, 1);
					_val7 = 2;
					player_set_commands_allowed(false);
				} else {
					digi_play("305r15", 1);
				}
			} else {
				digi_play("305r15", 1);
			}
			break;

		case 1:
			player_set_commands_allowed(true);
			_val7 = 1;
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("terrarium")) {
		if (inv_player_has("TURTLE")) {
			digi_play("305r16a", 1);
		} else if (_G(flags)[GLB_TEMP_12]) {
			digi_play("305r16b", 1);
		} else if (inv_object_is_here("TURTLE")) {
			digi_play("305r16c", 1);
		} else if (_G(flags)[V085]) {
			digi_play("305r16b", 1);
		} else {
			_G(flags)[V085] = 1;
			digi_play("305r16", 1);
		}
	} else if (lookFlag && player_said("turtle") &&
			inv_object_is_here("turtle")) {
		if (_G(flags)[V085]) {
			digi_play("305r16b", 1);
		} else {
			_G(flags)[V085] = 1;
			digi_play("305r16", 1);
		}
	} else if (lookFlag && player_said("turtle treats") &&
			inv_object_is_here("TURTLE TREATS")) {
		digi_play("305r18", 1);
	} else if (lookFlag && player_said("drawer")) {
		digi_play("305r17", 1);
	} else if (lookFlag && player_said("button display case")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);

			if (_G(flags)[V000] == 1)
				setGlobals4(_rip1, 12, 12, 12);
			else
				setGlobals4(_suit1, 17, 17, 17);

			sendWSMessage_C0000(0);
			digi_play("305r14", 1, 255, 2);
			break;

		case 2:
			sendWSMessage_B0000(3);
			break;

		case 3:
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("hammerhead shark")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);

			if (_G(flags)[V000] == 1) {
				_lookUp = series_load("rip looks up pos3");
				setGlobals4(_lookUp, 6, 6, 6);
			} else {
				_lookUp = series_load("rpsd103");
				setGlobals4(_lookUp, 7, 7, 7);
			}

			sendWSMessage_C0000(0);
			digi_play("305r12", 1, 255, 2);
			break;

		case 2:
			sendWSMessage_B0000(3);
			break;

		case 3:
			series_unload(_lookUp);
			break;

		default:
			break;
		}
	}
#define LOOK(ITEM) lookFlag && player_said(ITEM) && inv_object_is_here(ITEM)
	else if (LOOK("SHRUNKEN HEAD")) {
		digi_play("305r41", 1);
	} else if (LOOK("INCENSE BURNER")) {
		digi_play("305r50", 1);
	} else if (LOOK("CRYSTAL SKULL")) {
		digi_play("3055r51", 1);
	} else if (LOOK("ROMANOV EMERALD")) {
		digi_play("305r29a", 1);
	} else if (LOOK("WHALE BONE HORN")) {
		digi_play(_G(flags)[V042] ? "305r47" : "305r47a", 1);
	} else if (LOOK("WHEELED TOY")) {
		digi_play("305r42", 1);
	} else if (LOOK("SILVER BUTTERFLY")) {
		digi_play("305r46", 1);
	} else if (LOOK("REBUS AMULET")) {
		digi_play("305r39", 1);
	} else if (LOOK("CHISEL")) {
		digi_play("305r49", 1);
	} else if (LOOK("GERMAN BANKNOTE")) {
		digi_play("305r53", 1);
	} else if (LOOK("POSTAGE STAMP")) {
		digi_play("305r52", 1);
	} else if (LOOK("STICK AND SHELL MAP")) {
		digi_play("305r48", 1);
	} else if (player_said("exit")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;

		case 1:
			_G(game).setRoom(303);
			break;

		default:
			break;
		}
	} else {
		return;
	}
#undef LOOK

exit:
	_G(player).command_ready = false;
}

void Room305::setupSign() {
	if (!inv_object_is_here("SHRUNKEN HEAD")
			&& !inv_object_is_here("INCENSE BURNER")
			&& !inv_object_is_here("CRYSTAL SKULL")
			&& !inv_object_is_here("WHALE BONE HORN")
			&& !inv_object_is_here("WHEELED TOY")
			&& !inv_object_is_here("SILVER BUTTERFLY")
			&& !inv_object_is_here("REBUS AMULET")
			&& !inv_object_is_here("CHISEL")
			&& !inv_object_is_here("ROMANOV EMERALD")
			&& !inv_object_is_here("GERMAN BANKNOTE")
			&& !inv_object_is_here("POSTAGE STAMP")
			&& !inv_object_is_here("STICK AND SHELL MAP")
			&& !inv_object_is_here("305 NEW EXHIBIT SIGN")) {
		series_load("305 NEW EXHIBIT SIGN");
		series_show_sprite("305 NEW EXHIBIT SIGN", 0, 0xfff);
	}
}

void Room305::setShadow5(bool active) {
	if (active)
		_shadow5 = series_place_sprite("candleman shadow5", 0, 494, 278, -84, 0xe06);
	else
		terminateMachineAndNull(_shadow5);
}

void Room305::conv305a() {
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();
	const char *sound = conv_sound_to_play();

	if (_G(kernel).trigger == 1) {
		if (who <= 0) {
			_val7 = 1;
		} else if (_G(kernel).trigger == 1) {
			if (node != 1 || (entry != 0 && entry != 3 && entry != 5
				&& entry != 6 && entry != 7))
				_conv1 = 0;
		}

		conv_resume();

	} else if (!sound) {
		conv_resume();

	} else {
		if (who <= 0) {
			_val7 = 2;
		} else if (who == 1) {
			if ((node == 1 && entry == 0) || (node == 1 && entry == 6) ||
					(node == 1 && entry == 7)) {
				_conv1 = 5;
			} else if (node == 1 && (entry == 3 || entry == 5)) {
				_conv1 = 3;
			} else if ((node == 7 && entry == 0) || (node == 8 && entry == 0) ||
					(node == 9 && entry == 0) || (node == 10 && entry == 0) ||
					(node == 12 && entry == 0)) {
				_conv1 = 0;
			} else if (node == 1 && entry == 1) {
				_G(flags)[V089] = 2;
			} else {
				_conv1 = 1;
			}
		}

		digi_play(sound, 1, 255, 1);
	}
}

bool Room305::walkToObject() {
	if (player_said("SHRUNKEN HEAD")) {
		ws_walk(98, 313, 0, 1, 10, 1);
		return true;
	} else if (player_said("INCENSE BURNER")) {
		ws_walk(171, 285, 0, 1, 10, 1);
		return true;
	} else if (player_said("CRYSTAL SKULL")) {
		ws_walk(70, 320, 0, 1, 10, 1);
		return true;
	} else if (player_said("WHALE BONE HORN")) {
		ws_walk(116, 304, 0, 1, 10, 1);
		return true;
	} else if (player_said("WHEELED TOY")) {
		ws_walk(151, 296, 0, 1, 10, 1);
		return true;
	} else if (player_said("ROMANOV EMERALD")) {
		if (_G(flags)[V090] == 3) {
			digi_play("305f08", 1, 255, 6);
			_val7 = 2;
		} else {
			ws_walk(183, 288, 0, 1, 10, 1);
		}
		return true;
	}

	return false;
}

int Room305::getXAreaNum() const {
	int x = _G(player).click_x;

	if (x < 300)
		return 1;
	if (x < 380)
		return 2;
	if (x < 580)
		return 3;
	if (x < 670)
		return 4;
	if (x < 760)
		return 5;
	if (x < 860)
		return 6;
	if (x < 1040)
		return 7;
	if (x < 1140)
		return 8;
	return 9;
}

const char *Room305::getXAreaCartoon() const {
	static const char *CARTOONS[9] = {
		"395 easter island cartoon",
		"395 chin shih huang ti cartoon",
		"395 tablets of history cartoon",
		"395 epitaph a cartoon",
		"395 epitaph b cartoon",
		"395 castle cartoon",
		"395 moca moche cartoon",
		"395 temple cartoon",
		"395 romanov emerald cartoon"
	};
	return CARTOONS[getXAreaNum() - 1];
}

const char *Room305::getXAreaSeries() const {
	if (_G(flags)[V000]) {
		static const char *NAMES[9] = {
			"395car01", "395car08", "395car03", "395car06",
			"395car05", "395car04", "395car07", "395car02",
			"395car09"
		};
		return NAMES[getXAreaNum() - 1];

	} else {
		static const char *NAMES[9] = {
			"jack dempsey", "parrot", "kerosene",
			"chimney tree", "prof bingo", "blind men",
			"lemon", "hollow log", "restaurant"
		};
		return NAMES[getXAreaNum() - 1];
	}
}

Common::String Room305::getXAreaDigi() const {
	return Common::String::format("305r%d", getXAreaNum() + 20);
}

Common::String Room305::getXAreaDigi2() const {
	static const int NUMS[9] = { 5, 1, 4, 3, 8, 7, 2, 9, 6 };
	return Common::String::format("305r0%d", NUMS[getXAreaNum() - 1]);
}

int Room305::getItemX(int seriesHash) const {
	int w = ws_get_sprite_width(seriesHash, 0);
	int result = (640 - w) / 2;

	int sx1 = _G(game_buff_ptr)->x1;
	result += imath_abs(sx1);

	return result;
}

int Room305::getItemY(int seriesHash) const {
	int h = ws_get_sprite_height(seriesHash, 0);
	return (374 - h) / 2;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
