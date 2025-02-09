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
	_field0 = 80;
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
	sendWSMessage_10000(1, _811RipleyMach, 1, _811RP01Series, 1, -1, _811RP01Series, 1, 1, 0);
	_811McMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 1280, false, triggerMachineByHashCallback, "811 mc");
	sendWSMessage_10000(1, _811McMach, 1, _811MC01Series, 1, -1, _811MC01Series, 1, 1, 0);
	_811BaronMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "811 baron");
	sendWSMessage_10000(1, _811BaronMach, 12, _811BA01Series, 12, 1, _811BA01Series, 12, 17, 4);

	sendWSMessage_110000(_811BaronMach, 11);
}

void Room811::pre_parser() {
	// Nothing
}

void Room811::parser() {
}

void Room811::daemon() {
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
