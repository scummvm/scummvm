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

#include "m4/riddle/rooms/section8/room810.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const char *LOOK[][2] = {
	{"sarcophagus", "810r03"},
	{"grate", "810r05"},
	{"large urn", "810r18"},
	{nullptr, nullptr}
};

void Room810::preload() {
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));

	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}

void Room810::init() {
	digi_preload("950_s45", 950);
	digi_preload("950_s29", 950);
	digi_preload("810_s01", -1);
	_ripleyTakesJadeSealFromTombSeries = series_load("RIPLEY TAKES JADE SEAL FROM TOMB", -1, nullptr);
	_ripPos3LookAroundSeries = series_load("RIP POS 3 LOOK AROUND", -1, nullptr);
	_ripTrekHandTalkPos3Series = series_load("RIP TREK HAND TALK POS3", -1, nullptr);
	_810FireFlickerSeries = series_load("810 fire flicker", -1, nullptr);
	_810LitUrnSeries = series_load("810 lit urn", -1, nullptr);
	_810MercSeries = series_load("810merc", -1, nullptr);
	_810BlockSlidesOutSeries = series_load("810 block slides out", -1, nullptr);
	series_load("810seal", -1, nullptr);
	series_load("SAFARI SHADOW 3", -1, nullptr);
	series_play("810 fire flicker", 1024, 0, -1, 12, -1, 100, 0, 0, 0, -1);
	series_play("810 lit urn", 1024, 0, -1, 12, -1, 100, 0, 0, 0, -1);
	_810SealMach = series_show_sprite("810seal", 0, 768);
	digi_play_loop("950_s29", 3, 100, -1, 950);

	if (_G(game).previous_room == KERNEL_RESTORING_GAME)
		return;

	_alreadyPlayedVideo04aFl = false;
	player_set_commands_allowed(false);
	ws_demand_facing(_G(my_walker), 3);
	ws_demand_location(_G(my_walker), 55, 318);
	ws_walk(_G(my_walker), 160, 318, nullptr, 1, 3, true);
}

void Room810::pre_parser() {
	const bool lookFl = player_said_any("look", "look at");

	if (lookFl && player_said("mausoleum")) {
		digi_play("810r02", 1, 255, -1, 810);
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	} else if (lookFl && player_said(" ")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room810::parser() {
	const bool lookFl = player_said_any("look", "look at");
	const bool takeFl = player_said_any("talk", "talk to", "take");
	const bool gearFl = player_said("gear");
	const bool goFl = player_said("go");

	if (lookFl && _G(walker).ripley_said(LOOK)) {
		// Nothing: ripley_said() triggers digi_play() if a match is found - Do not remove!
	} else if (lookFl && player_said("IMPERIAL SEAL")) {
		switch (_G(kernel).trigger) {
		case -1:
			if (!_alreadyPlayedVideo04aFl) {
				player_set_commands_allowed(false);
				digi_play("810r04", 1, 255, 1, 810);
			} else {
				digi_play("810r04", 1, 255, -1, 810);
			}

			break;
		case 1:
			digi_play("810r04a", 1, 255, 2, 810);
			_alreadyPlayedVideo04aFl = true;

			break;

		case 2:
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (lookFl && player_said("mausoleum")) {
		// Nothing -> already covered by ripley_said(LOOK)
	} else if (lookFl && player_said("urn")) {
		digi_play("com061", 1, 255, -1, 997);
	} else if (goFl && player_said("outside")) {
		digi_play("810r06", 1, 255, -1, 810);
	} else if (gearFl && player_said("large urn")) {
		digi_play("com072", 1, 255, -1, -1);
	} else if (gearFl && player_said("urn")) {
		digi_play("com073", 1, 255, -1, -1);
	} else if ((takeFl || gearFl) && player_said("IMPERIAL SEAL")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			interface_hide();
			digi_play("810r07", 1, 255, 18, 810);
			kernel_timing_trigger(180, 10, nullptr);

			break;

		case 10:
			_ripLooksAroundAndNodsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 415, 312, 46, 512, false, triggerMachineByHashCallback, "rip takes seal");
			ws_hide_walker(_G(my_walker));
			sendWSMessage_10000(1, _ripLooksAroundAndNodsMach,  _ripTrekHandTalkPos3Series,1, 6, -1, _ripTrekHandTalkPos3Series, 7, 10, 4);
			_safariShadow3Mach = series_show("SAFARI SHADOW 3", 1280, 16, -1, -1, 0, 46, 415, 312);

			break;

		case 18:
			sendWSMessage_10000(1, _ripLooksAroundAndNodsMach,  _ripTrekHandTalkPos3Series, 6, 1, -1, _ripTrekHandTalkPos3Series, 1, 1, 0);
			kernel_timing_trigger(15, 20, nullptr);

			break;

		case 20:
			digi_play("810r07A", 1, 255, 40, 810);
			kernel_timing_trigger(50, 22, nullptr);

			break;

		case 22:
			sendWSMessage_10000(1, _ripLooksAroundAndNodsMach,  _ripTrekHandTalkPos3Series, 1, 6, -1, _ripTrekHandTalkPos3Series, 7, 10, 4);
			kernel_timing_trigger(540, 25, nullptr);

			break;

		case 24:
			sendWSMessage_10000(1, _ripLooksAroundAndNodsMach,  _ripTrekHandTalkPos3Series, 6, 1, -1, _ripTrekHandTalkPos3Series, 1, 1, 0);
			break;

		case 40:
			terminateMachine(_ripLooksAroundAndNodsMach);
			terminateMachine(_safariShadow3Mach);
			terminateMachine(_810SealMach);
			_ripLooksAroundAndNodsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 512, false, triggerMachineByHashCallback, "rip takes seal");
			sendWSMessage_10000(1, _ripLooksAroundAndNodsMach,  _ripleyTakesJadeSealFromTombSeries, 1, 38, 48, _ripleyTakesJadeSealFromTombSeries, 38, 38, 0);

			break;

		case 48:
			_blockSlidesOutMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "block slides out");
			sendWSMessage_10000(1, _blockSlidesOutMach, _810BlockSlidesOutSeries, 1, 14, -1, _810BlockSlidesOutSeries, 14, 14, 0);
			sendWSMessage_190000(_blockSlidesOutMach, 12);
			digi_play("810_s01", 3, 255, 60, 810);
			kernel_timing_trigger(40, 50, nullptr);

			break;

		case 50:
			sendWSMessage_10000(1, _ripLooksAroundAndNodsMach, _ripleyTakesJadeSealFromTombSeries, 39, 45, 51, _ripleyTakesJadeSealFromTombSeries, 45, 45, 0);
			break;

		case 51:
			kernel_timing_trigger(45, 52, nullptr);
			break;

		case 52:
			digi_play("810r08", 1, 255, 53, 810);
			break;

		case 53:
			sendWSMessage_10000(1, _ripLooksAroundAndNodsMach, _ripleyTakesJadeSealFromTombSeries, 46, 52, 54, _ripleyTakesJadeSealFromTombSeries, 52, 52, 0);
			break;

		case 54:
			sendWSMessage_10000(1, _ripLooksAroundAndNodsMach, _ripleyTakesJadeSealFromTombSeries, 52, 46, 55, _ripleyTakesJadeSealFromTombSeries, 57, 57, 0);
			sendWSMessage_10000(1, _ripLooksAroundAndNodsMach, _ripleyTakesJadeSealFromTombSeries, 46, 52, 56, _ripleyTakesJadeSealFromTombSeries, 52, 52, 0);

			break;

		case 55:
			sendWSMessage_10000(1, _ripLooksAroundAndNodsMach, _ripleyTakesJadeSealFromTombSeries, 46, 52, 56, _ripleyTakesJadeSealFromTombSeries, 52, 52, 0);
			break;

		case 56:
			digi_play("810r09", 1, 255, 58, 810);
			break;

		case 58:
			sendWSMessage_10000(1, _ripLooksAroundAndNodsMach, _ripleyTakesJadeSealFromTombSeries, 52, 84, 110, _ripleyTakesJadeSealFromTombSeries, 84, 84, 0);
			kernel_timing_trigger(25, 70, nullptr);

			break;

		case 60:
			digi_stop(3);
			digi_unload("810_s01");
			digi_play_loop("950_s45", 3, 128, -1, 950);

			break;

		case 70:
			series_play("810merc", 768, 0, 130, 12, 0, 100, 0, 0, 0, 35);
			break;

		case 110:
			terminateMachine(_ripLooksAroundAndNodsMach);
			ws_unhide_walker(_G(my_walker));
			ws_demand_facing(_G(my_walker), 9);
			ws_demand_location(_G(my_walker), 404, 311);
			sendWSMessage_3860000(_G(my_walker), 2);
			ws_walk(_G(my_walker), 55, 318, nullptr, -1, 9, true);

			break;

		case 130:
			_810MercMach = series_play("810merc", 768, 16, -1, 12, 0, 100, 0, 0, 36, 41);
			disable_player_commands_and_fade_init(150);
			digi_stop(2);

			break;

		case 150:
			adv_kill_digi_between_rooms(false);
			digi_unload("950_s29");
			_G(game).setRoom(860);

			break;

		default:
			break;
		}
	} else if (player_said("journal") && !takeFl && !lookFl && !gearFl) {
		digi_play("com042", 1, 255, -1, 950);
	} else if (!player_said("walk", "mausoleum"))
		return;

	_G(player).command_ready = false;
}

void Room810::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		_ripLooksAroundAndNodsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 160, 318, 46, 768, false, triggerMachineByHashCallback, "rip looks around and nods");
		ws_hide_walker(_G(my_walker));
		sendWSMessage_10000(1, _ripLooksAroundAndNodsMach, _ripPos3LookAroundSeries, 1, 16, 51, _ripPos3LookAroundSeries, 17, 20, 4);
		_safariShadow3Mach = series_show("SAFARI SHADOW 3", 1280, 16, -1, -1, 0, 46, 160, 318);

		break;

	case 51:
		sendWSMessage_190000(_ripLooksAroundAndNodsMach, 20);
		digi_play("810r01", 1, 255, 52, 810);

		break;

	case 52:
		sendWSMessage_190000(_ripLooksAroundAndNodsMach, 12);
		sendWSMessage_10000(1, _ripLooksAroundAndNodsMach, _ripPos3LookAroundSeries, 16, 1, 53, _ripPos3LookAroundSeries, 1, 1, 0);

		break;

	case 53:
		terminateMachine(_ripLooksAroundAndNodsMach);
		terminateMachine(_safariShadow3Mach);
		ws_unhide_walker(_G(my_walker));
		player_set_commands_allowed(true);

		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
