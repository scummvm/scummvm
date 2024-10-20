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

#include "m4/burger/rooms/section3/room310.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_PROBE_ANIMATION = 1,
	kCHANGE_TRUFFLES_ANIMATION = 3
};

const char *Room310::SAID[][4] = {
	{ "TUNNEL",   "310w002", "310w003",  nullptr   },
	{ "PROBE",    "310w004",  nullptr,   nullptr   },
	{ "TRUFFLES", nullptr,    "310w011", "310w012" },
	{ "GROUND",   "310w017",  "310w003", "310w003" },
	{ "WALL",     "310w017",  "310w003", "310w003" },
	{ "CEILING",  "310w017",  "310w003", "310w003" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room310::PLAY1[] = {
	{ 1,  2, "300t004a", 2, 165, -1, 0, 0, &_state1, 1 },
	{ 1,  2, "300t004b", 2, 165, -1, 0, 0, &_state1, 2 },
	{ 1,  2, "300t004c", 2, 165, -1, 0, 0, &_state1, 3 },
	{ 1,  2, "300t004d", 2, 165, -1, 0, 0, &_state1, 4 },
	{ 3,  4, nullptr,    2,   0,  1, 0, 0, nullptr,  0 },
	{ 5, -1, nullptr,    2,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room310::PLAY2[] = {
	{  0,  9, "300t003a", 2, 165,    -1,    0, 0, &_state1, 1 },
	{  0,  9, "300t003b", 2, 165,    -1,    0, 0, &_state1, 2 },
	{  0,  9, "300t003c", 2, 165,    -1,    0, 0, &_state1, 3 },
	{  0,  9, "300t003d", 2, 165,    -1,    0, 0, &_state1, 4 },
	{ 10, 16, nullptr,    2,   0, 10016,    1, 0, nullptr,  0 },
	{ 10, 16, nullptr,    2,   0,    -1,    0, 0, nullptr,  0 },
	{ 17, 21, nullptr,    2,   0,    -1,    0, 0, nullptr,  0 },
	{  0,  0, nullptr,    2,   0,    -1, 2048, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room310::PLAY3[] = {
	{ 15, 29, nullptr, 2, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room310::PLAY4[] = {
	{ 6, -1, nullptr, 1, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room310::PLAY5[] = {
	{ 30, 36, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	{ 37, 51, "300_002", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room310::PLAY6[] = {
	{ 52, 56, "300_001", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

int32 Room310::_state1;


Room310::Room310() : Mine() {
	_state1 = 0;
}

void Room310::init() {
	Mine::init();
	setupDigi();
	set_palette_brightness(48, 127, 30);
	digi_preload("601_007");

	_probeShould = 7;
	kernel_trigger_dispatch_now(kCHANGE_PROBE_ANIMATION);

	if (!_G(flags)[kTrufflesRanAway] && _G(flags)[kTrufflesInMine]) {
		_walk1 = intr_add_no_walk_rect(360, 265, 515, 293, 359, 294);
		_trufflesShould = 12;
		kernel_trigger_dispatch_now(kCHANGE_TRUFFLES_ANIMATION);
	} else {
		hotspot_set_active("TRUFFLES", false);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 319:
		break;

	default:
		_G(flags)[kMineRoomIndex] = 39;
		ws_demand_location(300, 306, 4);
		player_set_commands_allowed(true);
		break;
	}
}

void Room310::daemon() {
	Mine::daemon();
	if (!_G(kernel).continue_handling_trigger)
		return;
	_G(kernel).continue_handling_trigger = false;

	switch (_G(kernel).trigger) {
	case kCHANGE_PROBE_ANIMATION:
		switch (_probeShould) {
		case 7:
			_series2 = series_play("310prob", 0xb00, 0, -1, 6, -1, 100, 0, 0, 0, 17);
			break;

		case 8:
			_probeShould = 7;
			terminateMachineAndNull(_series2);
			series_play_with_breaks(PLAY3, "310prob", 0xb00, kCHANGE_PROBE_ANIMATION, 2);
			break;

		case 9:
			_probeShould = 10;
			terminateMachineAndNull(_series2);
			series_play_with_breaks(PLAY5, "310prob", 0xb00, kCHANGE_PROBE_ANIMATION, 2);
			break;

		case 10:
			_probeShould = 11;
			series_play_with_breaks(PLAY6, "310prob", 0xb00, kCHANGE_PROBE_ANIMATION, 2, 3);
			break;

		case 11:
			series_show("310prob", 0xb00, 0, -1, -1, 60);
			_G(wilbur_should) = 2;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		default:
			break;
		}
		break;

	case 2:
		_series1.terminate();
		kernel_trigger_dispatch_now(kCHANGE_TRUFFLES_ANIMATION);
		break;

	case kCHANGE_TRUFFLES_ANIMATION:
		switch (_trufflesShould) {
		case 12:
			_series1.show("310tr01", 0xb00, 0, -1, -1, 18);
			_trufflesShould = (imath_ranged_rand(1, 4) == 1) ? 13 : 12;
			kernel_timing_trigger(30, 2);
			break;

		case 13:
			_trufflesShould = 12;
			_probeShould = 8;
			series_play_with_breaks(PLAY1, "310tr01", 0xb00, 3, 3);
			_state1 = imath_ranged_rand(1, 4);
			break;

		case 14:
			term_message("Truffles goes to snarl at Wilbur!");
			_G(flags)[V145] = 1;
			_G(wilbur_should) = 4;
			_trufflesShould = 15;
			series_play_with_breaks(PLAY2, "310tr02", 0xb00, 3, 3);
			_state1 = imath_ranged_rand(1, 4);
			break;

		case 15:
			player_set_commands_allowed(true);

			if (!_G(flags)[V142]) {
				_G(flags)[V142] = 1;
				wilbur_speech("310w006");
			} else {
				wilbur_speech("310w007");
			}

			_trufflesShould = 12;
			kernel_trigger_dispatch_now(kCHANGE_TRUFFLES_ANIMATION);
			break;

		case 16:
			digi_play("300t002", 2, 155, -1, 300);
			hotspot_set_active("TRUFFLES", false);
			hotspot_set_active("GROUND ", false);
			intr_remove_no_walk_rect(_walk1);
			series_play("310tr03", 0xb00);
			break;

		default:
			break;
		}
		break;

	case 10008:
		_trufflesShould = 16;
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			ws_hide_walker();
			_general.play("310wi01", 0xb00, 16, -1, 6, 0, 100, 0, 0, 0, 5);
			_probeShould = 9;
			kernel_trigger_dispatch_now(kCHANGE_PROBE_ANIMATION);
			break;

		case 2:
			_general.terminate();
			_G(wilbur_should) = 3;
			series_play_with_breaks(PLAY4, "310wi01", 0xb00, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 3:
			ws_unhide_walker();
			pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 3006);
			break;

		case 4:
			_state1 = imath_ranged_rand(0, 2);

			switch (_state1) {
			case 0:
				wilbur_speech("310w005a");
				term_message("Wilbur: Whoa, easy Truffles!");
				break;

			case 1:
				wilbur_speech("310w005b");
				term_message("Wilbur: Down, piggy!");
				break;

			case 2:
				wilbur_speech("310w005c");
				term_message("Wilbur: Aaaaaaaaaa!");
				break;

			default:
				break;
			}
			break;

		case 5:
			wilbur_speech(_G(flags)[V145] ? "310w010" : "310w009");
			break;

		case 6:
			player_set_commands_allowed(false);
			_G(wilbur_should) = 1;
			wilbur_speech("310w008", kCHANGE_WILBUR_ANIMATION);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room310::pre_parser() {
	Mine::pre_parser();

	if (player_said("GEAR", "PROBE") && (!_G(flags)[kTrufflesInMine] || _G(flags)[kTrufflesRanAway] == 1)) {
		_G(wilbur_should) = 6;
		player_hotspot_walk_override(293, 288, 3, kCHANGE_WILBUR_ANIMATION);
		_G(player).command_ready = false;
	} else if (!_G(flags)[kTrufflesRanAway]) {
		HotSpotRec *hotspot = hotspot_which(_G(player).click_x, _G(player).click_y);
		assert(hotspot);

		if (hotspot->feet_x > 360 && hotspot->feet_x < 515 &&
				hotspot->feet_y > 265 && hotspot->feet_y < 293) {
			player_hotspot_walk_override(hotspot->feet_x, 294, 11);
		}
	}
}

void Room310::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;
	Mine::parser();
	if (!_G(player).command_ready)
		return;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("GEAR", "PROBE") || player_said("TAKE", "PROBE")) {
		term_message("truffles in mine %d    truffles ran away %d",
			_G(flags)[kTrufflesInMine], _G(flags)[kTrufflesRanAway]);

		if (_G(flags)[kTrufflesInMine] == 1 && !_G(flags)[kTrufflesRanAway]) {
			term_message("Wilbur pissed off truffles!");
			player_set_commands_allowed(false);
			_trufflesShould = 14;
		}
	} else if (player_said("LOOK AT", "TRUFFLES")) {
		_G(wilbur_should) = 5;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
