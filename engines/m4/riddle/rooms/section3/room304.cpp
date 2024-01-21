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

#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"
#include "m4/riddle/rooms/section3/room304.h"
#include "m4/riddle/vars.h"
#include "m4/riddle/riddle.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room304::init() {
	interface_show();
	_val1 = 0;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME)
		_roomVal1 = 0;

	if (_G(game).previous_room == 303 && _G(flags)[V001])
		_G(game).previous_room = 354;

	if (_G(game).previous_room == KERNEL_RESTORING_GAME && _G(flags)[V001])
		_G(game).previous_room = 354;

	hotspot_set_active(_G(flags)[V001] ? "RUG" : "RUG ", false);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		if (_G(flags)[V001])
			midi_play("thinker2", 255, 1, -1, 949);

		if (_G(flags)[V084] == 2 || player_been_here(201))
			_sword = series_show_sprite("one frame sword", 0, 0xa00);
		else
			hotspot_set_active("SAMURAI SWORD", false);
		break;

	case 303:
		player_set_commands_allowed(false);
		ws_demand_location(458, 263, 8);

		if (_G(flags)[V084] == 2 || player_been_here(201)) {
			_sword = series_show_sprite("one frame sword", 0, 0xa00);
			kernel_timing_trigger(1, 51);
		} else {
			hotspot_set_active("SAMURAI SWORD", false);
		}
		break;

	default:
		AddSystemHotkey(KEY_ESCAPE, Hotkeys::escape_key_pressed);
		AddSystemHotkey(KEY_F2, Hotkeys::saveGame);
		AddSystemHotkey(KEY_F3, Hotkeys::loadGame);

		if (_G(flags)[V001]) {
			mouse_show();
			_G(flags)[V001] = 1;
			digi_preload("304r52");
			digi_preload("304_s01");
			digi_preload("304_s04");
			player_set_commands_allowed(false);

			_val2 = _val3 = _val4 = 0;
			ws_demand_location(452, 285, 9);
			kernel_timing_trigger(1, 49);
			_trunk = series_show_sprite("one frame trunk", 0, 0);
			_stick = series_show_sprite("one frame stick", 0, 0xf00);

			LoadWSAssets("304 MC SCRIPT", _G(master_palette));

			_mei1 = series_load("MEI STANDS AND LOOKS");
			_G(globals)[GLB_TEMP_1] = _mei1 << 24;
			_G(globals)[GLB_TEMP_2] = 0xf050000;

			_mei2 = series_load("MEI TURNS INTO WALKER");
			_G(globals)[GLB_TEMP_3] = _mei2 << 24;
			_G(globals)[GLB_TEMP_4] = 0xf050000;

			_mei3 = series_load("MEI CHEN NY WALKER POS4");
			_G(globals)[GLB_TEMP_5] = _mei3 << 24;

			_machine1 = TriggerMachineByHash(32, nullptr, -1, -1, intrMsgNull, false, "MACHINE mc");

			LoadWSAssets("304 FL SN SCRIPT", _G(master_palette));
			AddWSAssetCELS("test3", 2, nullptr);
			AddWSAssetCELS("ripsh3", 12, nullptr);
			_smoke = series_load("LF AND COBRA FIRE SMOKE", -1, _G(master_palette));
			_G(globals)[GLB_TEMP_1] = _smoke << 24;
			_G(globals)[GLB_TEMP_2] = 0x10000;

			_machine2 = TriggerMachineByHash(45, nullptr, -1, -1, intrMsg, 0, "MACHINE fl snake rock");
		}

		_sword = series_show_sprite("one frame sword", 0, 0xa00);
		break;
	}

	if (!_G(flags)[V001]) {
		hotspot_set_active("MEI CHEN", false);
		hotspot_set_active("FENG LI", false);

		if (_G(flags)[V084] != 1 || player_been_here(301)) {
			hotspot_set_active("HANDLING STICK", false);
		} else {
			_stick = series_show_sprite("one frame stick", 0, 0xf00);
		}
	}
}

void Room304::daemon() {
	switch (_G(kernel).trigger) {
	case 49:
		ws_walk(387, 292, nullptr, 50, 9);
		digi_play("304m01", 1, 255, 60);
		break;

	case 50:
		player_set_commands_allowed(true);
		break;

	case 51:
		ws_walk(431, 287, nullptr, 50, 8);
		break;

	case 60:
		midi_play("thinker2", 255, 1, -1, 949);
		break;

	default:
		break;
	}
}

void Room304::pre_parser() {
	bool takeFlag = player_said("take");
	bool gearFlag = player_said("gear");

	if (_roomVal1) {
		terminateMachineAndNull(_machine3);
		_roomVal1 = 0;
		intr_cancel_sentence();
		hotspot_restore_all();
		interface_show();
	}

	if (_G(flags)[V001]) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;

		if ((takeFlag || gearFlag) && player_said("samurai sword")) {
			player_set_commands_allowed(false);
			_val2 = 1;
			digi_preload("304_s10");
			digi_preload("304_s05");
			_cutSnake = series_load("CUT SNAKE");
			sendWSMessage(0xa0000, 0, _machine2, 0, nullptr, 1);
		}

		if ((takeFlag || gearFlag) && player_said("handling stick")) {
			digi_preload("304_s03");
			digi_preload("304_s08");
			digi_preload("304_s02");
			digi_preload("304_s11");
		}
	}
}

void Room304::parser() {
	// TODO: parser
}

void Room304::intrMsg(frac16 myMessage, struct machine *sender) {
	Room304 *r = static_cast<Room304 *>(g_engine->_activeRoom);
	auto oldMode = _G(kernel).trigger_mode;

	if ((myMessage >> 16) == 57) {
		if ((_G(globals)[GLB_TEMP_1] >> 16) == 1) {
			player_set_commands_allowed(false);
			terminateMachineAndNull(r->_machine2);
			series_unload(r->_smoke);
			r->_cobra = series_load("COBRA KILLS RIP AND LF");
			digi_preload("304_s07");

			_G(kernel).trigger_mode = KT_PARSE;
			r->_machine2 = series_ranged_play("LF AND COBRA FIRE SMOKE", 1, 0, 25, 49, 100, 0x200, 5, 49);
			_G(kernel).trigger_mode = oldMode;

			digi_play("304_s06", 1);
			r->_val4 = 1;
			sendWSMessage(0x200000, 0, r->_machine1, 0, nullptr, 1);
			return;
		}

		if (r->_val3) {
			ws_demand_location(382, 295);
			player_set_commands_allowed(false);
			terminateMachineAndNull(r->_machine2);
			terminateMachineAndNull(r->_stick);
			digi_stop(1);
			terminateMachineAndNull(r->_trunk);

			r->_handle = series_stream("SNAKE HANDLE", 5, 0, 17);
			series_stream_break_on_frame(r->_handle, 10, 13);
			sendWSMessage(0x200000, 0, r->_machine1, 0, nullptr, 1);
			_G(flags)[V084] = 2;
			return;
		}
	}

	if ((myMessage >> 16) == 58 && r->_val2) {
		ws_demand_location(382, 295);
		ws_hide_walker();
		player_set_commands_allowed(false);
		terminateMachineAndNull(r->_machine2);
		series_unload(r->_smoke);
		digi_stop(1);

		_G(kernel).trigger_mode = KT_PARSE;
		series_ranged_play("CUT SNAKE", 1, 0, 0, 10, 100, 0x200, 4, 11);
		_G(kernel).trigger_mode = oldMode;

		sendWSMessage(0x200000, 0, r->_machine1, 0, nullptr, 1);
		_G(flags)[V084] = 1;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
