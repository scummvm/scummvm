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

#include "m4/burger/rooms/section9/room951.h"
#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/adv_trigger.h"
#include "m4/burger/vars.h"
#include "m4/m4.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum scene_triggers {
	START_ORION_BURGER_POSTER = 1,
	START_PLANET_X_HILLTOP_A,
	START_3,
	START_4,
	START_PLANET_X_LOW_GROUND_SHOT,
	START_6,
	START_7,
	START_PLANET_X_HILLTOP_B,
	START_9,
	START_10,
	START_SPACE_STATION_PANORAMA_A,
	START_12,
	START_13,
	START_CARGO_TRANSFER_AREA_A,
	START_15,
	START_16,
	START_VPS_OFFICE_A,
	START_18,
	START_19,
	START_HOLOGRAM,
	START_21,
	START_22,
	START_VPS_OFFICE_B,
	START_24,
	START_25,
	START_CARGO_TRANSFER_AREA_B,
	START_27,
	START_28,
	START_CARGO_TRANSFER_CONTROLS,
	START_30,
	START_31,
	START_SPACE_STATION_PANORAMA_B,
	START_33,
	START_34,
	START_35,
	START_36,
	START_37,
	START_38,
	START_39,
	START_40,
	START_41,
	START_42,
	START_43,
	START_44,
	START_45,
	START_46,
	START_47,
	START_48,
	START_50,
	START_51,
	START_52,
	START_53,
	START_54,
	START_55,
	SAVEGAME_CHECK,
	LAST_SCENE_TRIGGER = 57
};


static const seriesStreamBreak planet_x_hilltop_a[] = {
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

static const seriesStreamBreak planet_x_low_shot[] = {
	{   0,   nullptr, 2,   0, -1, 2048, 0, 0 },
	{   0, "952_003", 1, 200, -1, 0, 0, 0 },
	{  21, "952_001", 3, 120, -1, 0, 0, 0 },
	{  73, "952_002", 2, 160, -1, 0, 0, 0 },
	{  80, "952Z001", 1, 255, -1, 0, 0, 0 },
	{  92, "952_005", 2, 160, -1, 0, 0, 0 },
	{ 111, "952_006", 2, 100, -1, 0, 0, 0 },
	{ 117, "952_007", 2, 100, -1, 0, 0, 0 },
	{ 124, "952_004", 1, 225, -1, 0, 0, 0 },
	{ 136, "952_004", 1, 245, -1, 0, 0, 0 },
	{ 143, "952_010", 1, 255, -1, 0, 0, 0 },
	{ 147, "952_004", 1, 235, -1, 0, 0, 0 },
	{ 153, "952_008", 2, 100, -1, 0, 0, 0 },
	{ 154, "952_009", 3, 200, -1, 0, 0, 0 },
	{ 157, 0, 0, 0, 52, 0, 0, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak planet_x_hilltop_b[] = {
	{  1, "951_020", 3, 100, -1, 0, nullptr, 0 },
	{  4, "951_007", 2, 125, -1, 0, nullptr, 0 },
	{  7, "951z002", 1, 255, -1, 0, nullptr, 0 },
	{ 54,   nullptr, 0,   0, 50, 0, nullptr, 0 },
	{ 56, "951_008", 2, 150, -1, 0, nullptr, 0 },
	{ 75,   nullptr, 0,   0, 52, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak panorama_a[] = {
	{   1, "955_001", 1, 150, -1,    0, nullptr, 0 },
	{   7, "955_007", 2, 220, -1,    0, nullptr, 0 },
	{  24, "955_007", 2, 220, -1,    0, nullptr, 0 },
	{  25,   nullptr, 2,   0, -1, 2048, nullptr, 0 },
	{  26, "955_007", 2, 220, -1,    0, nullptr, 0 },
	{  35, "955_002", 2, 255, -1,    0, nullptr, 0 },
	{  90, "955_007", 1, 220, -1,    0, nullptr, 0 },
	{  92,   nullptr, 1,   0, -1, 2048, nullptr, 0 },
	{  93, "955_007", 1, 220, -1,    0, nullptr, 0 },
	{  99, "955_010", 1, 150, -1,    0, nullptr, 0 },
	{ 104,   nullptr, 3, 240, -1,    0, nullptr, 0 },
	{ 105,   nullptr, 3, 220, -1,    0, nullptr, 0 },
	{ 105, "955_007", 1, 210, -1,    0, nullptr, 0 },
	{ 106,   nullptr, 3, 200, -1,    0, nullptr, 0 },
	{ 107,   nullptr, 3, 190, -1,    0, nullptr, 0 },
	{ 108,   nullptr, 3, 180, -1,    0, nullptr, 0 },
	{ 109,   nullptr, 3, 170, -1,    0, nullptr, 0 },
	{ 110,   nullptr, 3, 160, -1,    0, nullptr, 0 },
	{ 111,   nullptr, 3, 150, -1,    0, nullptr, 0 },
	{ 112,   nullptr, 0,   0, 52,    0, nullptr, 0 },
	{ 113,   nullptr, 3, 140, -1,    0, nullptr, 0 },
	{ 114,   nullptr, 3, 120, -1,    0, nullptr, 0 },
	{ 115,   nullptr, 3, 110, -1,    0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak cargo_transfer_a[] = {
	{   0,   nullptr, 3, 100, -1, 0, nullptr, 0 },
	{   1, "956_001", 2,  80, -1, 0, nullptr, 0 },
	{   4, "956v001", 1, 255, -1, 0, nullptr, 0 },
	{ 136,   nullptr, 0,   0, 52, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak vps_office_a[] = {
	{   1, "957_005", 3,  40, -1, 1024, nullptr, 0 },
	{   1, "957v001", 1, 255, -1,    0, nullptr, 0 },
	{  92, "957v002", 1, 255, -1,    0, nullptr, 0 },
	{ 110, "957v003", 1, 255, -1,    0, nullptr, 0 },
	{ 125, "957_004", 2, 230, -1,    0, nullptr, 0 },
	{ 177, "957_007", 2, 100, -1,    0, nullptr, 0 },
	{ 183, "957_001", 2, 150, -1,    0, nullptr, 0 },
	{ 191, "957v004", 1, 255, -1,    0, nullptr, 0 },
	{ 194, "957_006", 2, 120, -1, 1024, nullptr, 0 },
	{ 267,   nullptr, 0,   0, 52,    0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak hologram[] = {
	{   0, "958v001", 1, 255, -1,    0, nullptr, 0 },
	{   0, "957_005", 3,  20, -1, 1024, nullptr, 0 },
	{   1,   nullptr, 2, 210, -1,    0, nullptr, 0 },
	{  39, "958_003", 1, 150, -1,    0, nullptr, 0 },
	{  42, "958_001", 1, 200, -1,    0, nullptr, 0 },
	{  52, "958z001", 1, 255, -1,    0, nullptr, 0 },
	{ 161, "958_003", 1, 150, -1,    0, nullptr, 0 },
	{ 164, "958_004", 2, 200, -1,    0, nullptr, 0 },
	{ 170, "958v002", 1, 255, -1,    0, nullptr, 0 },
	{ 264, "958z002", 1, 255, -1,    0, nullptr, 0 },
	{ 329, "958v003", 1, 255, -1,    0, nullptr, 0 },
	{ 362, "958_002", 2,  50, -1,    0, nullptr, 0 },
	{ 425, "958v004", 1, 255, -1,    0, nullptr, 0 },
	{ 456,   nullptr, 0,   0, 52,    0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak vps_office_b[] = {
	{   0,  "957_005", 3,  40, -1, 1024, nullptr, 0 },
	{   8, "957Z001y", 1, 255, -1,    0, nullptr, 0 },
	{  43, "957Z001z", 1, 255, -1,    0, nullptr, 0 },
	{ 112,  "957V005", 2, 255, -1,    0, nullptr, 0 },
	{ 139,  "957_002", 1, 255, -1,    0, nullptr, 0 },
	{ 154,  "957_003", 1, 255, -1,    0, nullptr, 0 },
	{ 170,    nullptr, 0,   0, 52,    0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak cargo_transfer_b[] = {
	{   0, "956_020", 1, 130, -1,    0, nullptr, 0 },
	{   0, "956_000", 2,  60, -1, 1024, nullptr, 0 },
	{   0, "956_001", 3,  80, -1, 1024, nullptr, 0 },
	{  29, "956Z002", 1, 255, -1,    0, nullptr, 0 },
	{  67, "956_003", 1,  60, -1,    0, nullptr, 0 },
	{  83, "956Z003", 1, 255, -1,    0, nullptr, 0 },
	{ 121, "956_002", 1, 200, -1,    0, nullptr, 0 },
	{ 132,   nullptr, 0,   0, 52,    0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak transfer_controls[] = {
	{   0,   nullptr, 2,   0, -1, 2048, nullptr, 0 },
	{   0,   nullptr, 3,   0, -1, 2048, nullptr, 0 },
	{   0, "956_001", 3, 180, -1, 1024, nullptr, 0 },
	{   1, "959F001", 1, 255, -1,    0, nullptr, 0 },
	{   5, "959_010", 2, 170, -1,    0, nullptr, 0 },
	{  27, "959_010", 2, 170, -1,    0, nullptr, 0 },
	{  36, "959Z001", 1, 255, -1,    0, nullptr, 0 },
	{  43, "959_013", 2, 100, -1,    0, nullptr, 0 },
	{  48,   nullptr, 2,   0, -1, 2048, nullptr, 0 },
	{  69, "959Z002", 1, 255, -1,    0, nullptr, 0 },
	{ 100, "959F002", 1, 255, -1,    0, nullptr, 0 },
	{ 146, "959Z003", 1, 255, -1,    0, nullptr, 0 },
	{ 207, "959_002", 2, 150, -1,    0, nullptr, 0 },
	{ 226, "959_003", 2, 255, -1,    0, nullptr, 0 },
	{ 235, "959_013", 1, 100, -1,    0, nullptr, 0 },
	{ 245,   nullptr, 1,   0, -1, 2048, nullptr, 0 },
	{ 254, "959_010", 2, 170, -1,    0, nullptr, 0 },
	{ 257, "959_013", 1, 100, -1,    0, nullptr, 0 },
	{ 272,   nullptr, 1,   0, -1, 2048, nullptr, 0 },
	{ 274, "959_004", 2, 255, -1,    0, nullptr, 0 },
	{ 277, "959_005", 2, 255, -1,    0, nullptr, 0 },
	{ 288, "959_012", 1, 255, -1,    0, nullptr, 0 },
	{ 295, "959_011", 2, 255, -1,    0, nullptr, 0 },
	{ 307, "959_005", 1, 200, -1,    0, nullptr, 0 },
	{ 311,   nullptr, 0,   0, 52,    0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak panorama_b[] = {
	{   0, "955_000", 3, 180, -1, 1024, nullptr, 0 },
	{   0, "955_003", 2, 150, -1,    0, nullptr, 0 },
	{   2, "955z001", 1, 255, -1,    0, nullptr, 0 },
	{  74, "955_004", 2, 255, -1,    0, nullptr, 0 },
	{  89, "955z002", 1, 255, -1,    0, nullptr, 0 },
	{ 125, "955_006", 2, 245, -1,    0, nullptr, 0 },
	{ 140,   nullptr, 0,   0, -1,    0, nullptr, 0 },
	{ 148,   nullptr, 0,   0, 52,    0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak break_961a[] = {
	{   0,   nullptr, 0,   0, -1, 0, nullptr, 0 },
	{   1,   nullptr, 0,   0, 47, 0, nullptr, 0 },
	{   1, "961_001", 1, 100, -1, 0, nullptr, 0 },
	{  63, "961_002", 1, 180, -1, 0, nullptr, 0 },
	{ 143,   nullptr, 0,   0, 52, 0, nullptr, 0 },
	{ 147,   nullptr, 0,   0, 48, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak break_961b[] = {
	{   0,   nullptr, 0,   0, -1, 0, 0, 0 },
	{   4, "961_003", 1, 255, -1, 0, 0, 0 },
	{  19, "961_004", 2, 180, -1, 0, 0, 0 },
	{  38, "961_005", 1, 255, -1, 0, 0, 0 },
	{  55,   nullptr, 0,   0, 52, 0, 0, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak break_961c[] = {
	{   0,   nullptr, 0,   0,  -1, 0, 0, 0 },
	{   1, "961_006", 2, 130,  -1, 0, 0, 0 },
	{  29, "961_007", 1, 255,  -1, 0, 0, 0 },
	{  62, "961_008", 1, 255,  -1, 0, 0, 0 },
	{  75, "961_009", 1, 200,  -1, 0, 0, 0 },
	{  95, "961_007", 2, 255,  -1, 0, 0, 0 },
	{ 100,   nullptr, 3, 240,  -1, 0, 0, 0 },
	{ 101,   nullptr, 3, 220,  -1, 0, 0, 0 },
	{ 102,   nullptr, 3, 200,  -1, 0, 0, 0 },
	{ 103,   nullptr, 3, 180,  -1, 0, 0, 0 },
	{ 104,   nullptr, 3, 160,  -1, 0, 0, 0 },
	{ 105,   nullptr, 3, 140,  -1, 0, 0, 0 },
	{ 106,   nullptr, 3, 120,  -1, 0, 0, 0 },
	{ 107,   nullptr, 3, 100,  -1, 0, 0, 0 },
	{ 108,   nullptr, 3,  90,  -1, 0, 0, 0 },
	{ 109,   nullptr, 3,  80,  -1, 0, 0, 0 },
	{ 110,   nullptr, 3,  60,  -1, 0, 0, 0 },
	{ 112,   nullptr, 3,  40,  -1, 0, 0, 0 },
	{ 114,   nullptr, 3,  20,  -1, 0, 0, 0 },
	{ 116,   nullptr, 3,  10,  -1, 0, 0, 0 },
	{ 117,   nullptr, 0,   0,  52, 0, 0, 0 },
	{ 118,   nullptr, 3,   0,  -1, 0, 0, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak break_961d[] = {
	{  -1,   nullptr, 0,   0,  -1, 0, 0, 0 },
	{   0,   nullptr, 0,   0,  -1, 0, 0, 0 },
	{  39,   nullptr, 0,   0,  52, 0, 0, 0 },
	{  42,   nullptr, 0,   0,2048, 0, 0, 0 },
	STREAM_BREAK_END
};

void Room951::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room951::init() {
	_G(kernel).suppress_fadeup = true;
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
		digi_preload_stream_breaks(planet_x_low_shot);
		digi_preload_stream_breaks(planet_x_hilltop_b);
		digi_preload_stream_breaks(panorama_a);
		digi_preload_stream_breaks(cargo_transfer_a);
		break;

	case START_PLANET_X_HILLTOP_A:
		palette_prep_for_stream();
		series_stream_with_breaks(planet_x_hilltop_a, "PLANET X HILLTOP A", 6, 1, START_3);
		pal_fade_init(&_G(master_palette)[0], 0, 255, 100, 30, NO_TRIGGER);		// Half second fade up
		break;

	case START_3:
		palette_prep_for_stream();
		kernel_timing_trigger(TENTH_SECOND, START_4, nullptr);
		break;

	case START_4:
		compact_mem_and_report();
		release_trigger_on_digi_state(START_PLANET_X_LOW_GROUND_SHOT, 1, 0);
		break;

	case START_PLANET_X_LOW_GROUND_SHOT:
		series_stream_with_breaks(planet_x_low_shot, "Planet X Low Ground Shot", 6, 1, START_6);
		pal_fade_init(&_G(master_palette)[0], 0, 255, 100, 30, NO_TRIGGER); // Half second fade up
		break;

	case START_6:
		palette_prep_for_stream();
		kernel_timing_trigger(TENTH_SECOND, START_7, nullptr);
		break;

	case START_7:
		digi_unload_stream_breaks(planet_x_hilltop_a);
		compact_mem_and_report();
		release_trigger_on_digi_state(START_PLANET_X_HILLTOP_B, 1, 0);
		break;

	case START_PLANET_X_HILLTOP_B:
		series_stream_with_breaks(planet_x_hilltop_b, "Planet X Hilltop B", 6, 1, START_9);
		pal_fade_init(&_G(master_palette)[0], 0, 255, 100, 30, NO_TRIGGER); // Half second fade up
		break;

	case START_9:
		palette_prep_for_stream();
		kernel_timing_trigger(TENTH_SECOND, START_10, nullptr);
		break;

	case START_10:
		digi_unload_stream_breaks(planet_x_low_shot);
		compact_mem_and_report();
		digi_preload_stream_breaks(vps_office_a);
		digi_preload_stream_breaks(hologram);
		digi_preload_stream_breaks(vps_office_b);
		release_trigger_on_digi_state(START_SPACE_STATION_PANORAMA_A, 1, 0);
		break;

	case START_SPACE_STATION_PANORAMA_A:
		series_stream_with_breaks(panorama_a, "Space Station Panorama A", 6, 1, START_12);
		pal_fade_init(&_G(master_palette)[0], 0, 255, 100, 30, NO_TRIGGER); // Half second fade up
		break;

	case START_12:
		palette_prep_for_stream();
		kernel_timing_trigger(TENTH_SECOND, START_13, nullptr);
		break;

	case START_13:
		digi_unload_stream_breaks(planet_x_hilltop_b);
		compact_mem_and_report();
		release_trigger_on_digi_state(START_CARGO_TRANSFER_AREA_A, 1, 0);
		break;

	case START_CARGO_TRANSFER_AREA_A:
		series_stream_with_breaks(cargo_transfer_a, "Cargo Transfer Area A", 6, 1, START_15);
		pal_fade_init(&_G(master_palette)[0], 0, 255, 100, 30, NO_TRIGGER); // Half second fade up
		break;

	case START_15:
		palette_prep_for_stream();
		kernel_timing_trigger(TENTH_SECOND, START_16, nullptr);
		break;

	case START_16:
		digi_unload_stream_breaks(panorama_a);
		compact_mem_and_report();
		release_trigger_on_digi_state(START_VPS_OFFICE_A, 1, 0);
		break;

	case START_VPS_OFFICE_A:
		palette_prep_for_stream();
		digi_unload_stream_breaks(panorama_a);
		series_stream_with_breaks(vps_office_a, "VP's Office A", 6, 1, START_18);
		pal_fade_init(&_G(master_palette)[0], 0, 255, 100, 30, NO_TRIGGER); // Half second fade up
		break;

	case START_18:
		palette_prep_for_stream();
		kernel_timing_trigger(TENTH_SECOND, START_19, nullptr);
		break;

	case START_19:
		compact_mem_and_report();
		release_trigger_on_digi_state(START_HOLOGRAM, 1, 0);
		break;

	case START_HOLOGRAM:
		digi_unload_stream_breaks(cargo_transfer_a);
		series_stream_with_breaks(hologram, "Hologram", 6, 1, START_21);
		pal_fade_init(&_G(master_palette)[0], 0, 255, 100, 30, NO_TRIGGER); // Half second fade up
		break;

	case START_21:
		palette_prep_for_stream();
		kernel_timing_trigger(TENTH_SECOND, START_22, nullptr);
		break;


	case START_22:
		compact_mem_and_report();
		release_trigger_on_digi_state(START_VPS_OFFICE_B, 1, 0);
		break;

	case START_VPS_OFFICE_B:
		series_stream_with_breaks(vps_office_b, "VP's Office B", 6, 1, START_24);
		pal_fade_init(&_G(master_palette)[0], 0, 255, 100, 30, NO_TRIGGER); // Half second fade up
		break;

	case START_24:
		palette_prep_for_stream();
		kernel_timing_trigger(TENTH_SECOND, START_25, nullptr);
		break;

	case START_25:
		compact_mem_and_report();
		digi_preload_stream_breaks(cargo_transfer_b);
		release_trigger_on_digi_state(START_CARGO_TRANSFER_AREA_B, 1, 0);
		break;

	case START_CARGO_TRANSFER_AREA_B:
		series_stream_with_breaks(cargo_transfer_b, "Cargo Transfer Area B", 6, 1, START_27);
		pal_fade_init(&_G(master_palette)[0], 0, 255, 100, 30, NO_TRIGGER); // Half second fade up
		break;

	case START_27:
		palette_prep_for_stream();
		kernel_timing_trigger(TENTH_SECOND, START_28, nullptr);
		break;

	case START_28:
		digi_unload_stream_breaks(vps_office_a);
		digi_unload_stream_breaks(hologram);
		digi_unload_stream_breaks(vps_office_b);
		compact_mem_and_report();
		digi_preload_stream_breaks(transfer_controls);
		release_trigger_on_digi_state(START_CARGO_TRANSFER_CONTROLS, 1, 0);
		break;

	case START_CARGO_TRANSFER_CONTROLS:
		series_stream_with_breaks(transfer_controls, "Cargo Transfer Controls", 6, 1, START_30);
		pal_fade_init(&_G(master_palette)[0], 0, 255, 100, 30, NO_TRIGGER); // Half second fade up
		break;

	case START_30:
		palette_prep_for_stream();
		kernel_timing_trigger(TENTH_SECOND, START_31, nullptr);
		break;

	case START_31:
		digi_unload_stream_breaks(cargo_transfer_b);
		compact_mem_and_report();
		digi_preload_stream_breaks(panorama_b);
		release_trigger_on_digi_state(START_SPACE_STATION_PANORAMA_B, 1, 0);
		break;

	case START_SPACE_STATION_PANORAMA_B:
		series_stream_with_breaks(panorama_b, "Space Station Panorama B", 6, 1, START_33);
		pal_fade_init(&_G(master_palette)[0], 0, 255, 100, 30, NO_TRIGGER); // Half second fade up
		break;

	case START_33:
		palette_prep_for_stream();
		kernel_timing_trigger(TENTH_SECOND, START_34, 0);
		break;

	case START_34:
		digi_unload_stream_breaks(transfer_controls);
		kernel_timing_trigger(60, START_35, nullptr);
		break;

	case START_35:
		digi_preload_stream_breaks(break_961a);
		pal_fade_set_start(_G(master_palette), 0);
		series_stream_with_breaks(break_961a, "961a", 6, 1, START_36);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 100, 60, NO_TRIGGER);
		break;

	case START_36:
		pal_fade_set_start(_G(master_palette), 0);
		pal_cycle_stop();
		kernel_timing_trigger(TENTH_SECOND, START_37, 0);
		break;

	case START_37:
		compact_mem_and_report();
		kernel_timing_trigger(1, START_38, 0);
		break;

	case START_38:
		digi_unload_stream_breaks(panorama_b);
		digi_preload_stream_breaks(break_961b);
		pal_fade_set_start(_G(master_palette), 0);
		series_stream_with_breaks(break_961b, "961b", 6, 1, START_39);
		pal_fade_init(_G(kernel).first_fade, 255, 100, 30, NO_TRIGGER);
		break;

	case START_39:
		pal_fade_set_start(_G(master_palette), 0);
		kernel_timing_trigger(TENTH_SECOND, START_40, 0);
		break;

	case START_40:
		compact_mem_and_report();
		release_trigger_on_digi_state(START_41, 1, 0);
		break;

	case START_41:
		digi_unload_stream_breaks(break_961a);
		digi_preload_stream_breaks(break_961c);
		pal_fade_set_start(_G(master_palette), 0);
		series_stream_with_breaks(break_961c, "961c", 6, 1, START_42);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 100, 30, NO_TRIGGER);
		break;

	case START_42:
		pal_fade_set_start(_G(master_palette), 0);
		kernel_timing_trigger(TENTH_SECOND, START_43, 0);
		break;

	case START_43:
		digi_unload_stream_breaks(break_961b);
		compact_mem_and_report();
		kernel_trigger_dispatch_now(SAVEGAME_CHECK);
		break;

	case SAVEGAME_CHECK:
		if (_G(executing) == WHOLE_GAME && !g_engine->autosaveExists()) {
			_G(game).new_room = 903;
			adv_kill_digi_between_rooms(false);
		} else {
			kernel_trigger_dispatch_now(START_44);
		}
		break;

	case START_44:
		pal_fade_set_start(_G(master_palette), 0);
		series_stream_with_breaks(break_961d, "961d", 6, 1, START_45);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 100, 30, -1);
		break;

	case START_45:
		kernel_timing_trigger(TENTH_SECOND, START_46, nullptr);
		break;

	case START_46:
		digi_preload_stream_breaks(break_961c);
		digi_preload_stream_breaks(break_961d);
		compact_mem_and_report();
		release_trigger_on_digi_state(LAST_SCENE_TRIGGER, 1, 0);
		break;

	case START_47:
		pal_cycle_init(224, 254, 4, -1, -1);
		break;

	case START_48:
		pal_cycle_stop();
		break;

	case START_50:
		digi_play("952music", 3, 255, -1, -1);
		break;

	case START_51:
		digi_play_loop("952music", 3, 255, -1, -1);
		break;

	case START_52:
		pal_fade_init(_G(master_palette), 0, 255, 0, 30, -1);
		break;

	case START_53:
		pal_fade_init(_G(master_palette), 0, 255, 100, 30, -1);
		break;

	case START_54:
		pal_fade_init(_G(master_palette), 0, 255, 0, 60, -1);
		break;

	case START_55:
		pal_fade_init(_G(master_palette), 0, 255, 100, 60, -1);
		break;

	case LAST_SCENE_TRIGGER:
		compact_mem_and_report();
		if (_G(executing) == WHOLE_GAME) {
			g_vars->initialize_game();
			_G(game).new_room = 801;
			_G(game).new_section = 8;
		} else if (g_engine->getLanguage() == Common::EN_ANY) {
			_G(game).new_room = 901;
		} else {
			_G(kernel).force_restart = true;
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}


} // namespace Rooms
} // namespace Burger
} // namespace M4
