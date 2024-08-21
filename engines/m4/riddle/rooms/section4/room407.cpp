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

#include "m4/riddle/rooms/section4/room407.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room407::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	LoadWSAssets("OTHER SCRIPT");
}

void Room407::init() {
	player_set_commands_allowed(false);
	digi_preload("407_s31");
	digi_preload("407_s32");
	_val1 = 357;
	_val2 = 293;

	_waterFlowTube = series_load("water flowing out surg tube");
	_waterFlowFaucet = series_load("WATER FLOWING OUT OF FAUCET");
	_waterFlowStem = series_load("water flws out fauct stem surg");

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		if (_val5 == 1010) {
			// TODO
		} else {
			// TODO
		}
	} else {
		static const char *HOTSPOTS[] = {
			"GARDEN HOSE", "RUBBER PLUG", "SURGICAL TUBE",
			"PUMP GRIPS", "PUMP GRIPS ", "JAR/CORK ",
			"JAR/GRIPS ", "JAR/RUBBER PLUG ", "PUMP ROD ",
			"GLASS JAR ", "LEVER KEY  ", "GARDEN HOSE ",
			"SURGICAL TUBE ", "FAUCET PIPE ", "GARDEN HOSE  ",
			"SURGICAL TUBE  ", "FAUCET PIPE  ", "SURGICAL TUBE   ",
			"GARDEN HOSE   ", "GARDEN HOSE    ", "AIR VALVE/HANDLE",
			"LEVER KEY ", "NOZZLES/TUBE", "PERIODIC TABLE/JAR",
			"PERIODIC TABLE ", "JAR/RUBBER PLUG", "JAR/CORK",
			"MICROSCOPE", "BUTTONS", "LETTER", "BUTTON",
			"BUTTON ", "BUTTON  ", "BUTTON   ", "WATER",
			"WATER ", "WATER  ", nullptr
		};
		for (auto *hs = HOTSPOTS; *hs; ++hs)
			hotspot_set_active(*hs, false);

		_val3 = 1010;
		_val4 = 1011;
		_val5 = 1011;
		_val6 = 1011;
		_val7 = 1020;
		_val8 = 1100;
		_val9 = 1100;
		_val10 = 1112;
		_xyzzy1 = 1115;
		_xyzzy2 = 1101;
		_xyzzy3 = 1101;
		_xyzzy4 = 1101;
		_xyzzy5 = 1101;
		_xyzzy6 = 1112;
		_xyzzy7 = 1112;
		_xyzzy8 = 1116;
		_xyzzy9 = 1118;
		_xyzzy10 = 1040;
		_frotz1 = 0;
		_frotz2 = 0;
		_frotz3 = 0;
		_frotz4 = 0;
		_frotz5 = 0;
		_frotz6 = 0;
		_frotz7 = 0;
		_frotz8 = 0;
		_frotz9 = 0;
		_frotz10 = 1031;
		_frotz10 = 1031;
		_int1 = 1031;
		_int2 = 1031;
		_int3 = 1031;
		_int4 = 1031;
		_int5 = 1031;
		_int6 = 1031;
		_int7 = 1031;
		_int10 = 0;
		_int11 = 0;
		_int12 = 0;
		_int13 = 0;
		_int14 = 0;

		_bottle = series_place_sprite("407BOTLE", 0, 0, -53, 100, 0xf00);
		_chart = series_place_sprite("407CHART", 0, 0, -53, 100, 0xf00);
		_cpist = series_place_sprite("407CPIST", 0, 0, -53, 100, 0xf00);
		_drawer = series_place_sprite("407DRAWR", 0, 0, -53, 100, 0xf00);
		_escape = series_place_sprite("407ESCAP", 0, 0, -53, 100, 0xd00);
		_faucet1 = series_place_sprite("407FAUC", 0, 0, -53, 100, 0xe00);
		_faucet2 = series_place_sprite("407FAUC", 0, 0, -53, 100, 0xe00);
		_lever = series_place_sprite("407LEVRW", 0, 0, -53, 100, 0xf00);
		_niche = series_place_sprite("407NICH", 0, 0, -53, 100, 0xf00);
		_tpist = series_place_sprite("407TPIST", 0, 0, -53, 100, 0xf00);
		_pump = series_place_sprite("407pump", 0, 0, -53, 100, 0xf00);
	}
	// TODO
}

void Room407::daemon() {
	// TODO
}

void Room407::pre_parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");
	// TODO
}

void Room407::parser() {
	// TODO
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
