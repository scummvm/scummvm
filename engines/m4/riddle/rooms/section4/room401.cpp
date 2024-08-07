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

void Room401::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	LoadWSAssets("other script");
}

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
		triggerMachineByHashCallbackNegative, "agent");
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

}

void Room401::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said("gear");

	if (player_said("conv401a")) {
		conv401a();
	}
	// TODO
	else {
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

} // namespace Rooms
} // namespace Riddle
} // namespace M4
