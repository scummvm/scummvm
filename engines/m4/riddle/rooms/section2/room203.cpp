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

#include "m4/riddle/rooms/section2/room203.h"
#include "m4/riddle/rooms/section2/section2.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

const int16 OFFICIAL_NORMAL_DIRS[] = { 200, 201, 202, -1 };
const char *OFFICIAL_NORMAL_NAMES[] = {
	"official walk pos11", "official walk pos9", "official walk pos7"
};
const int16 OFFICIAL_SHADOW_DIRS[] = { 210, 211, 212, -1 };
const char *OFFICIAL_SHADOW_NAMES[] = {
	"203of09s", "203of04s", "203of12s"
};


void Room203::init() {
	if (_G(game).previous_room != KERNEL_RESTORING_GAME)
		_val1 = 0;

	hotspot_set_active("SOLDIER'S HELMET", false);
	setupHelmetHotspot();

	_val2 = _val3 = _val4 = _val5 = 0;
	_val6 = _val7 = _val8 = _val9 = 0;
	_val10 = 0;
	_trigger1 = _trigger2 = _trigger3 = -1;
	_trigger4 = _trigger5 = -1;

	digi_preload("203_s02");
	_shadow3 = series_load("safari shadow 3");

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		setupGk();
		setupOldLady();
		setupPigeons();
		setupOfficial();
		kernel_timing_trigger(1, 130);

		if (_val1) {
			digi_preload("203_s09");
			series_load("one frame helmet");
			series_place_sprite("one frame helmet", 0, 960, -53, 100, 0x700);
			_peasantSeries = series_load("pesky peasant reach for helmet");
			_peasantSeriesShadow = series_load("shadow pp reach for helmet");

			_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1240, 332, 75, 0x800, 0,
				triggerMachineByHashCallback, "pesky peasant");
			_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1240, 332, 75, 0x800, 0,
				triggerMachineByHashCallback, "pesky peasant SHADOW");
			sendWSMessage_10000(1, _peasant, _peasantSeries, 23, 36, 121,
				_peasantSeries, 36, 23, 2);
			sendWSMessage_10000(1, _peasantShadow, _peasantSeriesShadow, 23, 36, -1,
				_peasantSeriesShadow, 36, 23, 2);

			hotspot_set_active("SOLDIER'S HELMET", false);
			hotspot_set_active("SOLDIER'S HELMET ", true);

		} else if (_G(flags)[V061]) {
			hotspot_set_active("PEASANT", false);
			hotspot_set_active("SOLDIER'S HELMET", false);

		} else {
			switch (_peasantMode) {
			case 4050:
			case 4051:
				_peasantX = 745;
				_peasantY = 378;
				_peasantLayer = 0x800;
				_peasantScale = 75;
				break;
			case 4052:
				_peasantX = 418;
				_peasantY = 385;
				_peasantLayer = 0x800;
				_peasantScale = 75;
				break;
			case 4053:
				_peasantX = 950;
				_peasantY = 378;
				_peasantLayer = 0x800;
				_peasantScale = 75;
				break;
			case 4054:
				_peasantX = 1100;
				_peasantY = 375;
				_peasantLayer = 0x800;
				_peasantScale = 75;
				break;
			default:
				break;
			}

			setupPeasant();
		}
		break;

	case 202:
		player_set_commands_allowed(false);
		MoveScreenDelta(_G(game_buff_ptr), -300, 0);
		ws_demand_location(547, 316, 1);

		setupGk();
		setupOldLady();
		setupPigeons();
		setupOfficial();

		if (_G(flags)[V061]) {
			hotspot_set_active("PEASANT", false);
			hotspot_set_active("SOLDIER'S HELMET", false);

		} else {
			if (_G(flags)[V072] == 4050) {
				_peasantMode = 4050;
				_peasantShould = 4091;
			} else {
				_peasantMode = 4051;
				_peasantShould = 4110;
			}

			_peasantX = 745;
			_peasantY = 378;
			_peasantLayer = 0x800;
			_peasantScale = 75;

			setupPeasant();
		}

		kernel_timing_trigger(1, 130);
		player_set_commands_allowed(true);
		break;

	case 204:
		player_set_commands_allowed(false);
		setupOfficial();
		setupGk();
		setupOldLady();
		setupPigeons();

		hotspot_set_active("PEASANT", false);
		hotspot_set_active("SOLDIER'S HELMET", false);
		kernel_timing_trigger(1, 130);
		ws_demand_location(348, 273, 1);
		ws_walk(231, 331, nullptr, 303, 4);

	case 207:
		MoveScreenDelta(_G(game_buff_ptr), -780, 0);
		player_set_commands_allowed(false);
		setupGk();
		setupOldLady();
		setupPigeons();

		if (_G(flags)[V067]) {
			ws_demand_location(1100, 290, 9);
			_G(player).disable_hyperwalk = true;

			_peskyYellsThief = series_load("pesky yells thief");
			_peasantRocksShadow = series_load("shadow pesky rock loop");
			_peasantX = 1100;
			_peasantY = 375;
			_peasantLayer = 0x800;
			_peasantScale = 75;

			_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1100, 322, 75, 0x800, 0,
				triggerMachineByHashCallback, "pp");
			_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
				_peasantX, _peasantY - 53, _peasantScale, _peasantLayer, 0,
				triggerMachineByHashCallback, "pp");

			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 21, 17, 354,
				_peskyYellsThief, 17, 17, 0);
			sendWSMessage_10000(1, _peasantShadow, _peasantRocksShadow, 1, 1, -1,
				_peasantRocksShadow, 1, 1, 0);

			digi_preload("203p02");
			_flag1 = true;
			_officialMode = 2001;
			_officialShould = 2010;

			_officialStander = series_load("official stander");
			_official = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1436, 362, 85, 0xf00, 0,
				triggerMachineByHashCallback, "of");
			sendWSMessage_10000(1, _official, _officialStander, 1, 1, 356,
				_officialStander, 1, 1, 0);

			_G(camera_reacts_to_player) = false;
			ws_walk_load_shadow_series(OFFICIAL_SHADOW_DIRS, OFFICIAL_SHADOW_NAMES);
			ws_walk_load_walker_series(OFFICIAL_NORMAL_DIRS, OFFICIAL_NORMAL_NAMES);
			_officialTurn9_11 = series_load("official turn 9-11");
			_officialTurn11_3 = series_load("official turn 11-3");
			_officialTurn3_7 = series_load("official turn 3-7");

			kernel_timing_trigger(10, 352);
			kernel_timing_trigger(1, 350);

		} else {
			ws_demand_location(1055, 295, 7);
			ws_walk(1045, 345, nullptr, 303, 7);
			kernel_timing_trigger(1, 300);
		}
		break;

	default:
		// TODO
		break;
	}

	digi_play_loop("203_s02", 3, 40);
}

void Room203::daemon() {
}

void Room203::setupHelmetHotspot() {
	for (HotSpotRec *hs = _G(currentSceneDef).hotspots; hs; hs = hs->next) {
		if (!strcmp(hs->vocab, "SOLDIER'S HELMET")) {
			hs->active = _G(flags)[V060] ?
				!inv_player_has("SOLDIER'S HELMET") && hs->lr_x > 1200 :
				hs->lr_x < 1200;
			break;
		}
	}
}

void Room203::setupPeasantHotspot(int mode) {
	for (HotSpotRec *hs = _G(currentSceneDef).hotspots; hs; hs = hs->next) {
		if (!strcmp(hs->vocab, "PEASANT")) {
			switch (mode) {
			case 4050:
			case 4051:
				hs->active = hs->lr_x > 450 && hs->lr_x < 800;
				break;
			case 4052:
				hs->active = hs->lr_x < 450;
				break;
			case 4053:
				hs->active = hs->lr_x > 800 && hs->lr_x < 1040;
				break;
			case 4054:
				hs->active = _peasantShould != 4150 && hs->lr_x > 1040;
				break;
			default:
				break;
			}
			break;
		}
	}
}

void Room203::setupGk() {
	_gkFrame = series_load("gk single frame");
	_gk = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x800, 0,
		triggerMachineByHashCallback, "gk single frame");
	_gkMode = _gkShould = 3000;
	sendWSMessage_10000(1, _gk, _gkFrame, 1, 1, -1, _gkFrame, 1, 1, 0);
	kernel_trigger_dispatchx(kernel_trigger_create(125));
}

void Room203::setupOldLady() {
	_oldLadyFrame = series_load("old lady - two single frames");

	if (_G(flags)[V060]) {
		hotspot_set_active("SOLDIER'S HELMET", false);
		_oldLadyNoHelmet = series_load("old lady feeding no helmet");
		_oldLadyMode = 5569;
		_oldLadyShould = 5200;
		_oldLady = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "ol");
		sendWSMessage_10000(1, _oldLady, _oldLadyNoHelmet, 1, 1, -1,
			_oldLadyNoHelmet, 1, 1, 0);

	} else {
		_oldLadyFeedingBirds = series_load("old lady feeding birds");
		_oldLady = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "ol");
		_oldLadyMode = 5666;
		_oldLadyShould = 5100;
		sendWSMessage_10000(1, _oldLady, _oldLadyFrame, 1, 1, -1,
			_oldLadyFrame, 1, 1, 0);
	}
}

void Room203::setupPigeons() {
	_pigeonsSeries1 = series_load("pigeon series of flcs 1");
	_pigeonsSeries3 = series_load("pigeon series of flcs 3");
	_pigeons1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1045, 373, 85, 0, 0,
		triggerMachineByHashCallback, "pig1");
	_pigeons3 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1065, 370, 85, 1, 0,
		triggerMachineByHashCallback, "pig3");

	kernel_trigger_dispatchx(kernel_trigger_create(392));
}

void Room203::setupOfficial() {
	_officialMode = 2001;
	_officialShould = 2010;
	_officialStander = series_load("official stander");
	_official = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1436, 362, 85, 0xf00, 0,
		triggerMachineByHashCallback, "of");
	_officialShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1436, 362, 85, 0xf00, 0,
		triggerMachineByHashCallback, "ofSH");
	sendWSMessage_10000(1, _official, _officialStander, 1, 1, -1,
		_officialStander, 1, 1, 0);
	sendWSMessage_10000(1, _officialShadow, _shadow3, 1, 1, -1,
		_shadow3, 1, 1, 0);
}

void Room203::setupPeasant() {
	_peasantRocks = series_load("peasant rocks 5frames");
	_peasantRocksShadow = series_load("shadow pesky rock loop");

	ws_walk_load_walker_series(S2_PEASANT_NORMAL_DIRS, S2_PEASANT_NORMAL_NAMES);
	ws_walk_load_shadow_series(S2_PEASANT_SHADOW_DIRS, S2_PEASANT_SHADOW_NAMES);

	setupPeasantHotspot(_peasantMode);

	_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _peasantX, _peasantY - 53,
		_peasantScale, _peasantLayer, false, triggerMachineByHashCallback, "pp");
	sendWSMessage_10000(1, _peasant, _peasantRocks, 1, 5, 120, _peasantRocks, 5, 5, 0);

	_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _peasantX, _peasantY - 53,
		_peasantScale, _peasantLayer, false, triggerMachineByHashCallback, "pp sh");
	sendWSMessage_10000(1, _peasant, _peasantRocksShadow, 1, 1, -1,
		_peasantRocksShadow, 1, 1, 0);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
