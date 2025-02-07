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

	_field0 = 0;
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
}

void Room810::daemon() {
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
