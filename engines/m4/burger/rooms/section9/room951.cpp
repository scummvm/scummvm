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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/burger/rooms/section9/room951.h"
#include "m4/adv_r/adv_trigger.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum scene_triggers {
	START_ORION_BURGER_POSTER = 1,

	// Triggers to start streamers
	START_PLANET_X_HILLTOP_A,
	START_PLANET_X_LOW_GROUND_SHOT,
	START_PLANET_X_HILLTOP_B,
	START_SPACE_STATION_PANORAMA_A,
	START_CARGO_TRANSFER_AREA_A,
	START_VPS_OFFICE_A,
	START_HOLOGRAM,
	START_VPS_OFFICE_B,
	START_CARGO_TRANSFER_AREA_B,
	START_CARGO_TRANSFER_CONTROLS,
	START_SPACE_STATION_PANORAMA_B,
	START_CALL_TO_ACTION,

	// Misc triggers for all animations
	FADE_DOWN_FAST,
	FADE_UP_FAST,

	FADE_DOWN_SLOW,
	FADE_UP_SLOW,

	LAST_SCENE_TRIGGER
};


static seriesStreamBreak planet_x_hilltop_a[] = {
	// frame	  sound   chan  vol   trigger 		 flags  variable value
	{   0,  "951_000", 3, 100, NO_TRIGGER, 0, nullptr, 0 },
	{  15,  "951_004", 2, 150, NO_TRIGGER, 0, nullptr, 0 },
	{  36,  "951_009", 1, 150, NO_TRIGGER, 1024, nullptr, 0 },
	{  80, "951_004b", 2, 150, NO_TRIGGER, 1024, nullptr, 0 },
	{ 114,    nullptr, 2, 100, NO_TRIGGER, 0, nullptr, 0 },
	{ 124,    nullptr, 2,  50, NO_TRIGGER, 0, nullptr, 0 },
	{ 134,  "951Z001", 2, 230, NO_TRIGGER, 0, nullptr, 0 },
	{ 158,  "951_006", 1, 255, NO_TRIGGER, 0, nullptr, 0 },
	{ 165,    nullptr, 0,   0, NO_TRIGGER, 0, nullptr, 0 },
	{  -1,    nullptr, 0,   0, NO_TRIGGER, 0, nullptr, 0 },
	STREAM_BREAK_END
};

void Room951::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room951::init() {
	_G(kernel).supress_fadeup = true;
	pal_fade_set_start(_G(master_palette), 0);
	pal_fade_init(_G(master_palette), 0, 255, 100, 40, START_ORION_BURGER_POSTER);
}

void Room951::daemon() {
	switch (_G(kernel).trigger) {
	case START_ORION_BURGER_POSTER:
		digi_stop(1);
		digi_stop(2);
		digi_stop(3);

		digi_preload("951music", -1);
		digi_play("951music", 1, 255, START_PLANET_X_HILLTOP_A, -1);
		digi_preload("952music", -1);

		digi_preload_stream_breaks(planet_x_hilltop_a);
#ifdef TODO
		digi_preload_stream_breaks(planet_x_low_shot);
		digi_preload_stream_breaks(planet_x_hilltop_b);
		digi_preload_stream_breaks(panorama_a);
		digi_preload_stream_breaks(cargo_transfer_a);
		digi_preload_stream_breaks(vps_office_a);
		digi_preload_stream_breaks(hologram);
		digi_preload_stream_breaks(vps_office_b);
		break;

	case START_PLANET_X_HILLTOP_A:
		palette_prep_for_stream();
		series_stream_with_breaks(planet_x_hilltop_a, "PLANET X HILLTOP A", 6, 1, START_PLANET_X_LOW_GROUND_SHOT);
		pal_fade_init(&master_palette[0], 0, 255, 100, 30, NO_TRIGGER); // half second fade up
		break;

	case START_PLANET_X_LOW_GROUND_SHOT:
		palette_prep_for_stream();
		series_stream_with_breaks(planet_x_low_shot, "Planet X Low Ground Shot", 6, 1, START_PLANET_X_HILLTOP_B);
		pal_fade_init(&master_palette[0], 0, 255, 100, 30, NO_TRIGGER); // half second fade up
		break;

	case START_PLANET_X_HILLTOP_B:
		palette_prep_for_stream();
		digi_unload_stream_breaks(planet_x_hilltop_a);
		series_stream_with_breaks(planet_x_hilltop_b, "Planet X Hilltop B", 6, 1, START_SPACE_STATION_PANORAMA_A);
		pal_fade_init(&master_palette[0], 0, 255, 100, 30, NO_TRIGGER); // half second fade up
		break;

	case START_SPACE_STATION_PANORAMA_A:
		palette_prep_for_stream();
		digi_unload_stream_breaks(planet_x_low_shot);
		series_stream_with_breaks(panorama_a, "Space Station Panorama A", 6, 1, START_CARGO_TRANSFER_AREA_A);
		pal_fade_init(&master_palette[0], 0, 255, 100, 30, NO_TRIGGER); // half second fade up
		break;

	case START_CARGO_TRANSFER_AREA_A:
		palette_prep_for_stream();
		digi_unload_stream_breaks(planet_x_hilltop_b);
		series_stream_with_breaks(cargo_transfer_a, "Cargo Transfer Area A", 6, 1, START_VPS_OFFICE_A);
		pal_fade_init(&master_palette[0], 0, 255, 100, 30, NO_TRIGGER); // half second fade up
		break;

	case START_VPS_OFFICE_A:
		palette_prep_for_stream();
		digi_unload_stream_breaks(panorama_a);
		series_stream_with_breaks(vps_office_a, "VP's Office A", 6, 1, START_HOLOGRAM);
		pal_fade_init(&master_palette[0], 0, 255, 100, 30, NO_TRIGGER); // half second fade up
		break;

	case START_HOLOGRAM:
		palette_prep_for_stream();
		digi_unload_stream_breaks(cargo_transfer_a);
		series_stream_with_breaks(hologram, "Hologram", 6, 1, START_VPS_OFFICE_B);
		pal_fade_init(&master_palette[0], 0, 255, 100, 30, NO_TRIGGER); // half second fade up
		break;

	case START_VPS_OFFICE_B:
		palette_prep_for_stream();
		digi_unload_stream_breaks(vps_office_a);
		series_stream_with_breaks(vps_office_b, "VP's Office B", 6, 1, START_CARGO_TRANSFER_AREA_B);
		pal_fade_init(&master_palette[0], 0, 255, 100, 30, NO_TRIGGER); // half second fade up
		break;

	case START_CARGO_TRANSFER_AREA_B:
		palette_prep_for_stream();
		digi_unload_stream_breaks(hologram);
		digi_preload_stream_breaks(cargo_transfer_b);
		series_stream_with_breaks(cargo_transfer_b, "Cargo Transfer Area B", 6, 1, START_CARGO_TRANSFER_CONTROLS);
		pal_fade_init(&master_palette[0], 0, 255, 100, 30, NO_TRIGGER); // half second fade up
		break;

	case START_CARGO_TRANSFER_CONTROLS:
		palette_prep_for_stream();
		digi_stop(1);
		digi_stop(2);
		digi_stop(3);
		digi_unload_stream_breaks(vps_office_b);
		digi_preload_stream_breaks(transfer_controls);
		series_stream_with_breaks(transfer_controls, "Cargo Transfer Controls", 6, 1, START_SPACE_STATION_PANORAMA_B);
		pal_fade_init(&master_palette[0], 0, 255, 100, 30, NO_TRIGGER); // half second fade up
		break;

	case START_SPACE_STATION_PANORAMA_B:
		palette_prep_for_stream();
		digi_unload_stream_breaks(transfer_controls);
		digi_unload_stream_breaks(cargo_transfer_b);
		digi_preload_stream_breaks(panorama_b);
		series_stream_with_breaks(panorama_b, "Space Station Panorama B", 6, 1, START_CALL_TO_ACTION);
		pal_fade_init(&master_palette[0], 0, 255, 100, 30, NO_TRIGGER); // half second fade up
		break;

	case START_CALL_TO_ACTION:
		if (executing == INTERACTIVE_DEMO) {
			palette_prep_for_stream();
			digi_unload_stream_breaks(transfer_controls);
			digi_preload_stream_breaks(call_to_action);
			series_stream_with_breaks(call_to_action, "Call To Action", 6, 1, LAST_SCENE_TRIGGER);
			pal_fade_init(&master_palette[0], 0, 255, 100, 30, NO_TRIGGER); // half second fade up
		} else {
			kernel_trigger_dispatch_now(LAST_SCENE_TRIGGER);
		}
		break;

	case FADE_DOWN_FAST:
		pal_fade_init(&master_palette[0], 0, 255, 0, 30, NO_TRIGGER); // half second fade down
		break;

	case FADE_DOWN_SLOW:
		pal_fade_init(&master_palette[0], 0, 255, 0, 60, NO_TRIGGER); // one second fade down
		break;

	case FADE_UP_FAST:
		pal_fade_init(&master_palette[0], 0, 255, 100, 30, NO_TRIGGER); // half second fade up
		break;

	case FADE_UP_SLOW:
		pal_fade_init(&master_palette[0], 0, 255, 100, 60, NO_TRIGGER); // one second fade up
		break;

	case LAST_SCENE_TRIGGER:

		switch (executing) {

		case INTERACTIVE_DEMO:
			game.new_room = 901;  // go back to demo menu
			break;

		case WHOLE_GAME:
			game.new_room = 901;	 // debug for now go back to demo menu until main game menu is available
			break;
		}
		ShowCursor();
		break;
#endif
	case 49:
	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}


} // namespace Rooms
} // namespace Burger
} // namespace M4
