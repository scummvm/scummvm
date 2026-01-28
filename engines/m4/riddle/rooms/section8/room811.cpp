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

#include "m4/riddle/rooms/section8/room811.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room811::preload() {
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));

	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	_G(player).walker_in_this_scene = false;
}

void Room811::init() {
	interface_hide();
	player_set_commands_allowed(false);
	_811MusicFadingVol = 80;
	digi_stop(1);
	digi_preload("950_s45", 950);
	digi_play_loop("950_s45", 3, 80, -1, 950);

	digi_preload("811_s01", -1);
	digi_preload("811_s01a", -1);
	digi_preload("811_s01b", -1);
	digi_preload("950_s55", -1);
	digi_preload("811R02", -1);
	digi_preload("811B03", -1);
	digi_preload("811R03", -1);
	digi_preload("811B04", -1);
	digi_preload("811B06", -1);
	digi_preload("811R05", -1);

	_811RP01Series = series_load("811RP01", -1, nullptr);
	_811MC01Series = series_load("811MC01", -1, nullptr);
	_811BA01Series = series_load("811BA01", -1, nullptr);
	_811BA02Series = series_load("811BA02", -1, nullptr);
	_811BA03Series = series_load("811BA03", -1, nullptr);
	_811BA04Series = series_load("811BA04", -1, nullptr);
	_811BA05Series = series_load("811BA05", -1, nullptr);

	_811RipleyMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 1024, false, triggerMachineByHashCallback, "811 ripley");
	sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 1, 0);
	_811McMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 1280, false, triggerMachineByHashCallback, "811 mc");
	sendWSMessage_10000(1, _811McMach, _811MC01Series, 1, 1, -1, _811MC01Series, 1, 1, 0);
	_811BaronMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "811 baron");
	sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, 1, _811BA01Series, 12, 17, 4);

	sendWSMessage_110000(_811BaronMach, 11);
}

void Room811::pre_parser() {
	// Nothing
}

void Room811::parser() {
	// Nothing
}

void Room811::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		digi_play("811B01", 2, 255, 2, -1);
		break;

	case 2:
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 12, 0);
		digi_play("811m01", 2, 255, 3, -1);
		sendWSMessage_10000(1, _811McMach, _811MC01Series, 1, 1, -1, _811MC01Series, 1, 6, 4);
		sendWSMessage_1a0000(_811McMach, 11);

		break;

	case 3:
		sendWSMessage_10000(1, _811McMach, _811MC01Series, 1, 1, -1, _811MC01Series, 1, 1, 0);
		digi_play("811r01", 2, 255, 5, -1);
		sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 7, 4);
		sendWSMessage_1a0000(_811RipleyMach, 11);

		break;

	case 5:
		sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 1, 0);
		digi_play("811b02", 2, 255, 7, -1);
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 17, 4);
		sendWSMessage_1a0000(_811BaronMach, 11);

		break;

	case 7:
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 12, 0);
		_811Pu01Mach = series_stream("811pu01", 10, 256, -1);
		digi_play("950_s55", 1, 60, -1, 950);
		series_stream_break_on_frame(_811Pu01Mach, 26, 1050);
		digi_play("811r02", 2, 255, 11, -1);
		sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 7, 4);
		sendWSMessage_1a0000(_811RipleyMach, 11);

		break;

	case 11:
		digi_unload("811R02");
		sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 1, 0);
		digi_play("811b03", 2, 255, 13, -1);
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 17, 4);
		sendWSMessage_1a0000(_811BaronMach, 11);

		break;

	case 13:
		digi_unload("811B03");
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 12, 0);
		_811Pu01Mach = series_stream("811pu02", 12, 256, -1);
		series_stream_break_on_frame(_811Pu01Mach, 5, 76);
		digi_play("811r03", 2, 255, 15, -1);
		sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 7, 4);
		sendWSMessage_1a0000(_811RipleyMach, 11);

		break;

	case 15:
		digi_unload("811r03");
		sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 1, 0);
		digi_play("811B04", 2, 255, 17, -1);
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 17, 4);
		sendWSMessage_1a0000(_811BaronMach, 11);

		break;

	case 17:
		digi_unload("811B04");
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 12, 0);
		digi_play("811m02", 2, 255, 19, -1);
		sendWSMessage_10000(1, _811McMach, _811MC01Series, 1, 1, -1, _811MC01Series, 1, 6, 4);
		sendWSMessage_1a0000(_811McMach, 11);

		break;

	case 19:
		sendWSMessage_10000(1, _811McMach, _811MC01Series, 1, 1, -1, _811MC01Series, 1, 1, 0);
		digi_play("811r04", 2, 255, 20, -1);
		sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 7, 4);
		sendWSMessage_1a0000(_811RipleyMach, 11);

		break;

	case 20:
		sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 1, 0);
		digi_play("811B05", 2, 255, 21, -1);
		kernel_timing_trigger(45, 1100, nullptr);
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 17, 4);
		sendWSMessage_1a0000(_811BaronMach, 11);

		break;

	case 21:
		digi_play("811B05A", 2, 255, 23, -1);
		sendWSMessage_10000(1, _811BaronMach, _811BA02Series, 1, 12, 22, _811BA02Series, 12, 12, 0);

		break;

	case 22:
		sendWSMessage_10000(1, _811BaronMach, _811BA02Series, 12, 1, -1, _811BA01Series, 12, 17, 0);
		sendWSMessage_1a0000(_811BaronMach, 11);

		break;

	case 23:
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 12, 0);
		_811Pu01Mach = series_stream("811pu03", 15, 256, -1);
		series_stream_break_on_frame(_811Pu01Mach, 5, 77);
		digi_play("811B06", 2, 255, 25, -1);
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 17, 4);
		sendWSMessage_1a0000(_811BaronMach, 11);

		break;

	case 25:
		digi_unload("811B06");
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 12, 0);
		digi_play("811r05", 2, 255, 26, -1);
		sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 7, 4);
		sendWSMessage_1a0000(_811RipleyMach, 11);

		break;

	case 26:
		digi_unload("811R05");
		sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 1, 0);
		digi_play("811B07", 2, 255, 27, -1);
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 17, 4);
		sendWSMessage_1a0000(_811BaronMach, 11);

		break;

	case 27:
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 12, 0);
		digi_play("811r06", 2, 255, 28, -1);
		sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 7, 4);
		sendWSMessage_1a0000(_811RipleyMach, 11);

		break;

	case 28:
		sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 1, 0);
		kernel_timing_trigger(20, 29, nullptr);

		break;

	case 29:
		digi_play("811B08", 2, 255, 30, -1);
		kernel_timing_trigger(95, 1110, nullptr);
		sendWSMessage_10000(1, _811BaronMach, _811BA03Series, 1, 24, -1, _811BA01Series, 12, 17, 4);
		sendWSMessage_1a0000(_811BaronMach, 11);

		break;

	case 30:
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 12, 0);
		digi_play("811R07", 2, 255, 31, -1);
		sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 7, 4);
		sendWSMessage_1a0000(_811RipleyMach, 11);

		break;

	case 31:
		sendWSMessage_10000(1, _811RipleyMach, _811RP01Series, 1, 1, -1, _811RP01Series, 1, 1, 0);
		digi_play("811B09", 2, 255, 82, -1);
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 17, 4);
		sendWSMessage_1a0000(_811BaronMach, 11);

		break;

	case 32:
		digi_play("811B09B", 2, 255, 34, -1);
		sendWSMessage_10000(1, _811BaronMach, _811BA05Series, 1, 4, 33, _811BA05Series, 24, 24, 0);

		break;

	case 33:
		sendWSMessage_10000(1, _811BaronMach, _811BA05Series, 24, 1, -1, _811BA01Series, 12, 17, 0);
		sendWSMessage_1a0000(_811BaronMach, 11);

		break;

	case 34:
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 12, 0);
		series_unload(_811BA02Series);
		series_unload(_811BA03Series);
		series_unload(_811BA04Series);
		series_unload(_811BA05Series);

		digi_stop(1);
		digi_stop(2);

		digi_unload("811_s01");
		digi_unload("811_s01a");
		digi_unload("811_s01b");
		digi_unload("950_s55");
		digi_unload("811R02");
		digi_unload("811B03");
		digi_unload("811R03");
		digi_unload("811B04");
		digi_unload("811B06");
		digi_unload("811R05");

		_811EndSeries = series_load("811END", -1, nullptr);

		terminateMachine(_811RipleyMach);
		terminateMachine(_811McMach);
		terminateMachine(_811BaronMach);

		series_unload(_811RP01Series);
		series_unload(_811MC01Series);
		series_unload(_811BA01Series);

		_811Pu01Mach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 256, false, triggerMachineByHashCallback, "811 end series");
		sendWSMessage_10000(1, _811Pu01Mach, _811EndSeries, 1, 57, 64, _811EndSeries, 57, 57, 0);

		break;

	case 35:
		digi_play("811b10", 2, 255, 36, -1);
		sendWSMessage_10000(1, _811Pu01Mach, _811EndSeries, 61, 61, -1, _811EndSeries, 61, 63, 4);
		sendWSMessage_1a0000(_811Pu01Mach, 11);

		break;

	case 36:
		sendWSMessage_10000(1, _811Pu01Mach, _811EndSeries, 64, 73, 65, _811EndSeries, 73, 73, 0);
		break;

	case 37:
		digi_play("811r08", 2, 255, 91, -1);
		break;

	case 40:
		digi_play("811_s06", 1, 255, 37, -1);
		break;

	case 64:
		sendWSMessage_10000(1, _811Pu01Mach, _811EndSeries, 58, 60, 35, _811EndSeries, 60, 60, 0);
		digi_play("811_s03", 1, 255, -1, -1);

		break;

	case 65:
		sendWSMessage_10000(1, _811Pu01Mach, _811EndSeries, 74, 87, -1, _811EndSeries, 87, 87, 0);
		digi_play("811_s04", 1, 255, -1, -1);
		kernel_timing_trigger(55, 40, nullptr);

		break;

	case 76:
		digi_play("811_s01", 1, 220, -1, -1);
		break;

	case 77:
		digi_play("811_s01", 1, 255, -1, -1);
		break;

	case 82:
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 12, 0);
		kernel_timing_trigger(15, 83, nullptr);

		break;

	case 83:
		sendWSMessage_10000(1, _811BaronMach, _811BA04Series, 1, 17, -1, _811BA01Series, 12, 17, 4);
		sendWSMessage_1a0000(_811BaronMach, 11);
		digi_play("811B09A", 2, 255, 84, -1);

		break;

	case 84:
		sendWSMessage_10000(1, _811BaronMach, _811BA01Series, 12, 12, -1, _811BA01Series, 12, 12, 0);
		kernel_timing_trigger(20, 32, nullptr);

		break;

	case 91:
		player_set_commands_allowed(false);
		adv_kill_digi_between_rooms(false);
		disable_player_commands_and_fade_init(92);
		digi_stop(1);
		digi_stop(2);

		break;

	case 92:
		digi_play_loop("950_s45", 3, 128, -1, 950);
		_G(game).setRoom(861);
		break;

	case 1050:
		_811MusicFadingVol -= 5;
		if (_811MusicFadingVol <= 0) {
			digi_stop(1);
		} else {
			digi_change_panning(1, _811MusicFadingVol);
			kernel_timing_trigger(2, 1050, nullptr);
		}

		break;

	case 1100:
		digi_play("811_s01a", 1, 255, -1, -1);
		break;

	case 1110:
		digi_play("811_s01b", 1, 255, -1, -1);
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
