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

#include "m4/riddle/rooms/section4/room406.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room406::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	LoadWSAssets("OTHER SCRIPT");
}

void Room406::init() {
	player_set_commands_allowed(false);

	switch (_G(flags)[V312]) {
	case 0:
		hotspot_set_active("STAIRS", false);
		hotspot_set_active("BILLIARD TABLE ", false);
		break;

	case 1:
		_billiardTable = series_place_sprite("406 BILLIARD TABLE UNLATCHED",
			0, 0, 0, 100, 0x200);
		hotspot_set_active("STAIRS", false);
		break;

	case 2:
		_billiardTable = series_place_sprite("406 BILLIARD TABLE UP",
			0, 0, 0, 100, 0x200);
		hotspot_set_active("BILLIARD TABLE", false);
		hotspot_set_active("BILLIARD TABLE ", false);
		break;

	default:
		break;
	}

	if (_G(flags)[kPaintingOpen]) {
		_painting = series_place_sprite("406 PAINTING OPEN", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("PAINTING", false);
		hotspot_set_active("SMOKING HUTCH", true);
	} else {
		_painting = series_place_sprite("406 PAINTING CLOSED", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("PAINTING", true);
		hotspot_set_active("SMOKING HUTCH", false);
	}

	if (_G(flags)[V316]) {
		hotspot_set_active("MIRROR", false);
		_mirror = series_place_sprite("CRACKED MIRROR IN BILLIARD RM",
			0, 0, 0, 100, 0xf00);
	} else {
		hotspot_set_active("BROKEN MIRROR", false);
	}

	if (_G(flags)[V310]) {
		_cardDoor = series_place_sprite("406 CARD DOOR OPEN BY PICT",
			0, 0, 0, 100, 0xf00);
		hotspot_set_active("ACE OF SPADES", false);
		hotspot_set_active("ACE OF SPADES ", true);
		hotspot_set_active("SWITCH", true);
	} else {
		hotspot_set_active("ACE OF SPADES", true);
		hotspot_set_active("ACE OF SPADES ", false);
		hotspot_set_active("SWITCH", false);
	}

	if (_G(flags)[V306]) {
		hotspot_set_active("GAMES CABINET", false);
		_gamesCabinet = series_place_sprite("406 GAMES CABINET OPEN",
			0, 0, 0, 100, 0xf00);
	} else {
		hotspot_set_active("GAMES CABINET ", false);
		hotspot_set_active("GAMES CABINET DRAWER", false);
		_gamesCabinet = series_place_sprite("406 GAMES CABINET CLOSED",
			0, 0, 0, 100, 0xf00);
	}

	if (_G(flags)[V309]) {
		hotspot_set_active("WRITING DESK", false);
		_desk = series_place_sprite("406 DESK OPEN", 0, 0, 0, 100, 0x600);
	} else {
		hotspot_set_active("WRITING DESK ", false);
		hotspot_set_active("WRITING DESK DRAWER", false);
		_desk = series_place_sprite("406 DESK CLOSE", 0, 0, 0, 100, 0x600);
	}

	if (_G(flags)[V321]) {
		_poolBall = series_place_sprite("406 POOL BALL IN CABINET",
			0, 0, 0, 100, 0xf00);
		hotspot_set_active("BILLIARD BALL", false);
	} else if (!inv_player_has("BILLIARD BALL")) {
		_poolBall = series_place_sprite("BILLIARD BALL", 0, 0, 0, 100, 0x200);
	} else {
		hotspot_set_active("BILLIARD BALL", false);
	}

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 1001;
		_val2 = 1001;
		hotspot_set_active("CABINET DRAWER OPEN", false);
		hotspot_set_active("DESK DRAWER OPEN", false);
		hotspot_set_active("MESSAGES", false);
		hotspot_set_active("ENVELOPE", false);
		hotspot_set_active("KEYS", false);
	} else if (_val2 == 1000) {
		ws_demand_facing(1);
		_rptmhr = series_load("RPTMHR11");
		setGlobals1(_rptmhr, 1, 5, 5, 5, 0, 5, 1, 1, 1);
		sendWSMessage_110000(-1);
		_emptyDrawer = series_place_sprite("406 DESK DRAWER EMPTY",
			0, 0, 0, 100, 0x200);
		disableHotspots();

		hotspot_set_active(" ", true);
		hotspot_set_active("DESK DRAWER OPEN", true);
		hotspot_set_active("MESSAGES", true);

		if (!inv_player_has("ENVELOPE")) {
			_envelope = series_place_sprite("406 DESK DRAWER ENVELOPE",
				0, 0, 0, 100, 0x200);
			hotspot_set_active("ENVELOPE", true);
		}

		if (!inv_player_has("KEYS")) {
			_keys = series_place_sprite("406 DESK DRAWER WITH KEYS",
				0, 0, 0, 100, 0x200);
			hotspot_set_active("KEYS", true);
		}
	} else if (_val1 == 1000) {
		ws_demand_facing(11);
		_ripReachHand = series_load("RIP TREK MED REACH HAND POS1");
		setGlobals1(_ripReachHand, 1, 10, 10, 10, 0, 10, 1, 1, 1);
		sendWSMessage_110000(-1);
		_cards = series_place_sprite("406 GAMES DRAWER WITH CARDS",
			0, 0, 0, 100, 0x100);
		disableHotspots();

		hotspot_set_active(" ", true);
		hotspot_set_active("CABINET DRAWER OPEN", true);
	} else {
		disableHotspots();
		setHotspots();
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);

		if (_G(flags)[V322]) {
			digi_preload("456_s03a");
			digi_play_loop("456_s03a", 3, 255, -1, 456);
		}
		break;

	case 456:
		if (_G(flags)[V322])
			digi_play_loop("456_s03a", 3, 255, -1, 456);

		ws_demand_location(400, 340, 1);
		_ripHiHand = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand, 1, 5, 5, 5, 0, 5, 1, 1, 1);
		sendWSMessage_110000(310);
		break;

	default:
		if (_G(flags)[V322]) {
			digi_preload("456_s03a");
			digi_play_loop("456_s03a", 3, 255, -1, 456);
		}

		ws_demand_location(603, 327, 9);
		ws_walk(530, 332, nullptr, 300, 9);
		break;
	}
}

void Room406::daemon() {
}

void Room406::pre_parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (useFlag && player_said_any("BILLIARD TABLE", "BILLIARD TABLE ")) {
		_G(player).resetWalk();
		_G(kernel).trigger_mode = KT_PARSE;
		kernel_timing_trigger(1, 69);
		_G(kernel).trigger_mode = KT_PREPARSE;
	}

	if (player_said("BILLIARD BALL", "BILLIARD TABLE") &&
			_G(kernel).trigger == -1) {
		_G(player).resetWalk();
		_G(kernel).trigger_mode = KT_PARSE;
		kernel_timing_trigger(1, 69);
		_G(kernel).trigger_mode = KT_PREPARSE;
	}

	if (_val1 == 1000) {
		_G(player).resetWalk();

		if (!player_said(" ") &&
			!(lookFlag && player_said("MESSAGE LOG")) &&
			!player_said("journal")) {
			if (!useFlag || !player_said("CABINET DRAWER OPEN"))
				return;
		}

		intr_cancel_sentence();
		_val1 = 1001;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 10);
		_G(kernel).trigger_mode = KT_PARSE;

	} else if (_val2 == 1000) {
		_G(player).resetWalk();

		if (!player_said(" ") &&
			!(lookFlag && player_said("MESSAGE LOG")) &&
			!player_said("journal")) {
			if (!useFlag || !player_said("DESK DRAWER OPEN"))
				return;
		}

		intr_cancel_sentence();
		_val1 = 1001;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 20);
		_G(kernel).trigger_mode = KT_PARSE;

	} else if (player_said("journal") && !takeFlag && !lookFlag &&
			_G(kernel).trigger == -1) {
		_G(player).resetWalk();
	}
}

void Room406::parser() {

}

void Room406::disableHotspots() {
	for (auto *hs = _G(currentSceneDef).hotspots; hs; hs = hs->next)
		hs->active = false;
}

void Room406::setHotspots() {
	for (auto *hs = _G(currentSceneDef).hotspots; hs; hs = hs->next)
		hs->active = true;

	hotspot_set_active(_G(flags)[V316] ? "MIRROR" : "BROKEN MIRROR", false);

	if (_G(flags)[V306]) {
		hotspot_set_active("GAMES CABINET", false);
	} else {
		hotspot_set_active("GAMES CABINET ", false);
		hotspot_set_active("GAMES CABINET DRAWER", false);
	}

	if (_G(flags)[V309]) {
		hotspot_set_active("WRITING DESK", false);
	} else {
		hotspot_set_active("WRITING DESK ", false);
		hotspot_set_active("WRITING DESK DRAWER", false);
	}

	if (inv_player_has("BILLIARD BALL") || _G(flags)[V321])
		hotspot_set_active("BILLIARD BALL", false);

	hotspot_set_active("CABINET DRAWER OPEN", false);
	hotspot_set_active("DESK DRAWER OPEN", false);
	hotspot_set_active("MESSAGES", false);
	hotspot_set_active("ENVELOPE", false);
	hotspot_set_active("KEYS", false);

	if (_G(flags)[V310]) {
		hotspot_set_active("ACE OF SPADES", false);
	} else {
		hotspot_set_active("ACE OF SPADES ", false);
		hotspot_set_active("SWITCH", false);
	}

	switch (_G(flags)[V312]) {
	case 0:
		hotspot_set_active("STAIRS", false);
		hotspot_set_active("BILLIARD TABLE ", false);
		break;

	case 1:
		hotspot_set_active("STAIRS", false);
		break;

	case 2:
		hotspot_set_active("BILLIARD TABLE", false);
		hotspot_set_active("BILLIARD TABLE ", false);
		break;

	default:
		break;
	}

	hotspot_set_active(_G(flags)[kPaintingOpen] ? "PAINTING" : "SMOKING HUTCH", false);
	hotspot_set_active(_G(flags)[V316] ? "MIRROR" : "BROKEN MIRROR", false);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
