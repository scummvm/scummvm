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
				triggerMachineByHashCallbackNegative, "fl stander");
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
					triggerMachineByHashCallbackNegative, "rip reach");
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
					triggerMachineByHashCallbackNegative, "fl stander");
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

	if (lookFlag && player_said("cartoon"))
		_G(camera_reacts_to_player) = false;

	if (_val5) {
		_G(camera_reacts_to_player) = true;
		terminateMachineAndNull(_machine1);
		terminateMachineAndNull(_machine2);
		intr_cancel_sentence();
		_val5 = 0;
		hotspot_restore_all();
		interface_show();
	}
}

void Room305::parser() {

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

} // namespace Rooms
} // namespace Riddle
} // namespace M4
