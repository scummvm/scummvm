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

#include "m4/burger/rooms/section1/room145.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/burger.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_DRUMZ_ANIMATION = 2,
	kCHANGE_ROXY_ANIMATION = 4,
	kCHANGE_VIPE_ANIMATION = 5,
	kSPEAK_TO_VIPE = 8,
	kSPEAK_TO_ROXY = 9,
	kSPEAK_TO_DRUMZ = 10
};

const char *Room145::SAID[][4] = {
	{ "TOUR BUS",      "145w001", "145w002", "145w003" },
	{ "INSTRUMENTS",   "145w005", "145w006", "145w006" },
	{ "AMPLIFIER ",    "145w009", nullptr,   "145w012" },
	{ "VIPE",          "145w013", "145w004", "145w004" },
	{ "ROXY",          "145w016", "145w004", "145w004" },
	{ "DRUMZ",         "145w019", "145w004", "145w004" },
	{ "MAP",           "145w022", "145w002", "145w023" },
	{ "VERA'S DINER",  "145w024", "145w004", "145w004" },
	{ "VERA'S DINER ", "145w024", "145w004", nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

const WilburMatch Room145::MATCH[] = {
	{ nullptr, "TOUR BUS", 1, 0, 0, &_state1, 12 },
	{ "TALK", "ROXY", 10016, 0, 0, &Vars::_wilbur_should, 11 },
	{ "AMPLIFIER", "ROXY", 1, 0, 0, &_state1, 15 },
	{ nullptr, "ROXY", 1, 0, 0, &_state1, 17 },
	{ "TALK" , "DRUMZ", kSPEAK_TO_DRUMZ, 0, 0, 0, 0 }, 
	{ "LAXATIVE" , "DRUMZ", 10016, 0, 0, &Vars::_wilbur_should, 3 },
	{ nullptr, "DRUMZ", 1, 0, 0, &_state1, 16 },
	{ "TALK" , "VIPE", 10016, 0, 0, &Vars::_wilbur_should, 10 },
	{ "AMPLIFIER", "VIPE", 1, 0, 0, &_state1, 15 },
	{ nullptr, "VIPE", 1, 0, 0, &_state1, 16 },
	{ "AMPLIFIER" , "INSTRUMENTS", 1, 0, 0, &_state1, 13 },
	{ nullptr, "INSTRUMENTS", 1, 0, 0, &_state1, 14 },
	{ "TAKE", "AMPLIFIER ", 10016, &Flags::_flags[kDrumzFled], 0, &Vars::_wilbur_should, 1 },
	{ "TAKE", "AMPLIFIER ", 10016, &Flags::_flags[kDrumzFled], 1, &Vars::_wilbur_should, 8 },
	{ nullptr, "AMPLIFIER ", 1, 0, 0, &_state1, 14 },
	{ nullptr, "MAP", 1, 0, 0, &_state1, 18 },
	WILBUR_MATCH_END
};

const seriesPlayBreak Room145::PLAY1[] = {
	{ 0, 6, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{ 6, 6, nullptr,   0,   0, -1, 0,  6, nullptr, 0 },
	{ 6, 6, "145w010", 1, 255, -1, 0, -1, nullptr, 0 },
	{ 0, 6, nullptr,   0,   0, -1, 2,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY2[] = {
	{  0, 23, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 24, -1, "145w011", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY3[] = {
	{ 15, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY4[] = {
	{ 0, 3, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{ 4, 4, nullptr,   0,   0, -1, 0,  4, nullptr, 0 },
	{ 5, 9, "145w021", 1, 255, -1, 4, -1, nullptr, 0 },
	{ 4, 4, nullptr,   0,   0, -1, 0,  4, nullptr, 0 },
	{ 0, 3, nullptr,   0,   0, -1, 2,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY5[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY6[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY7[] = {
	{ 0,  1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 2, -1, nullptr, 0, 0, -1, 0, 4, nullptr, 0 },
	{ 0,  1, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY8[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY9[] = {
	{ 0, -1, "145_004", 2, 125, -1, 1, 0, &_state2, 0 },
	{ 0, -1, nullptr,   0,   0, -1, 0, 0, &_state2, 1 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY10[] = {
	{ 0, -1, nullptr, 0, 0, -1, 3, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY11[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY12[] = {
	{ 0,  1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 2,  4, nullptr, 0, 0, -1, 1, 2, nullptr, 0 },
	{ 5, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY13[] = {
	{  0,  5, nullptr,   0,   0,    -1, 0, 0, nullptr, 0 },
	{  6, 16, nullptr,   0,   0, 10016, 0, 0, nullptr, 0 },
	{ 17, 20, "145_002", 1, 255,    -1, 0, 0, nullptr, 0 },
	{ 21, 25, "145_005", 1, 255,    -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY14[] = {
	{ 26, -1, "145_001", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY15[] = {
	{ 0, 3, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY16[] = {
	{ 0, 3, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY17[] = {
	{ 1, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY18[] = {
	{ 0, -1, "145r901", 2, 125, -1, 4, -1, &_state3, 1 },
	{ 0, -1, "145r902", 2, 125, -1, 4, -1, &_state3, 2 },
	{ 0, -1, "145r903", 2, 125, -1, 4, -1, &_state3, 3 },
	{ 0, -1, "145r904", 2, 125, -1, 4, -1, &_state3, 4 },
	{ 0, -1, "145r905", 2, 125, -1, 4, -1, &_state3, 5 },
	{ 0, -1, "145r906", 2, 125, -1, 4, -1, &_state3, 6 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY19[] = {
	{ 1, -1, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY20[] = {
	{ 1, -1, nullptr, 0, 0, -1, 0, 2, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY21[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY22[] = {
	{ 0, -1, nullptr, 0, 0, -1, 1, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY23[] = {
	{ 0, -1, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

int32 Room145::_state1;
int32 Room145::_state2;
int32 Room145::_state3;

Room145::Room145() : Room() {
	_state1 = 0;
	_state2 = 0;
	_state3 = 0;
}

void Room145::init() {
	digi_preload("145_006");
	digi_play_loop("145_006", 3, 255, -1);
	_state2 = 0;

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 142:
		ws_demand_location(680, 340, 9);
		ws_walk(570, 330, 0, -1, -1);
		break;

	default:
		ws_demand_location(570, 330, 9);
		break;
	}

	kernel_timing_trigger(600, 7);

	if (_G(flags)[kDrumzFled]) {
		series_show("145drum", 0xa01);
		hotspot_set_active("DRUMZ", false);
	} else {
		loadDrum();
		_walk1 = intr_add_no_walk_rect(140, 295, 240, 325, 139, 326);
		_drumzState = 19;
		kernel_trigger_dispatch_now(kCHANGE_DRUMZ_ANIMATION);
	}

	loadRx();

	_walk2 = intr_add_no_walk_rect(423, 308, 540, 340, 541, 341);
	_roxyTalkTo = 101;
	_roxyState = 33;
	kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);

	_walk3 = intr_add_no_walk_rect(341, 326, 480, 365, 481, 366);
	_vipeState = 43;
	kernel_trigger_dispatch_now(kCHANGE_VIPE_ANIMATION);

	if (inv_player_has("AMPLIFIER")) {
		hotspot_set_active("AMPLIFIER ", false);
	} else {
		_amplifier = series_show("145amp", 0xa01);
	}
}

void Room145::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		switch (_state1) {
		case 12:
			wilbur_speech("145w004");
			break;

		case 13:
			wilbur_speech("145w007");
			break;

		case 14:
			wilbur_speech("145w008");
			break;

		case 15:
			wilbur_speech("145w014");
			break;

		case 16:
			wilbur_speech("145w015");
			break;

		case 17:
			wilbur_speech("145w018");
			break;

		case 18:
			wilbur_speech("145w003");
			break;

		default:
			break;
		}
		break;

	case kCHANGE_DRUMZ_ANIMATION:
		switch (_drumzState) {
		case 19:
			if (!digi_play_state(2))
				digi_play_loop("145_003", 2, 125, -1);

			_drumzState = getRandomDrumzState();
			series_play_with_breaks(PLAY5, "145dz01", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 20:
			_drumzState = 23;
			series_play_with_breaks(PLAY6, "145dz02", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 21:
			_drumzState = 23;
			series_play_with_breaks(PLAY7, "145dz03", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 22:
			if (!_state2)
				digi_stop(2);

			_drumzState = 23;
			series_play_with_breaks(PLAY8, "145dz04", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 23:
			kernel_trigger_dispatch_now(10001);
			_drumzState = getRandomDrumzState();
			kernel_trigger_dispatch_now(kCHANGE_DRUMZ_ANIMATION);
			break;

		case 24:
			_drumzState = getRandomDrumzState();
			series_play_with_breaks(PLAY9, "145dz05", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 25:
			if (!_state2)
				digi_stop(2);

			_drumzState = getRandomDrumzState();
			series_play_with_breaks(PLAY10, "145dz06", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 26:
			_drumzState = 27;
			series_play_with_breaks(PLAY11, "145dz05", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			_G(wilbur_should) = 2;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 27:
			_drumzState = getRandomDrumzState();
			series_play_with_breaks(PLAY12, "145dz06", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 28:
			if (!_state2)
				digi_stop(2);

			_G(wilbur_should) = 5;
			_drumzState = 29;
			series_play_with_breaks(PLAY13, "145dz08", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 29:
			hotspot_set_active("DRUMZ", false);
			intr_remove_no_walk_rect(_walk1);

			_G(flags)[kDrumzFled] = 1;
			series_play_with_breaks(PLAY14, "145dz08", 0, -1, 3);
			series_show("145drum", 0xa01);
			_G(wilbur_should) = 6;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		default:
			break;
		}
		break;

	case kCHANGE_ROXY_ANIMATION:
		_val5 = _roxyState;

		switch (_roxyState) {
		case 30:
			_roxy.terminate();
			_roxyState = 31;
			series_play_with_breaks(PLAY15, "145rx01", 0x900, kCHANGE_ROXY_ANIMATION, 3, 6, 100, 0, 0);
			break;

		case 31:
			_duration = imath_ranged_rand(180, 360);
			_roxyState = 32;
			_roxy.show("145rx01", 0x900, 0, 4, _duration);
			break;

		case 32:
			_roxy.terminate();
			_roxyState = 33;
			series_play_with_breaks(PLAY16, "145rx01", 0x900, kCHANGE_ROXY_ANIMATION, 3);
			break;

		case 33:
			if (_roxyTalkTo == 101) {
				_duration = imath_ranged_rand(180, 360);
				_roxyState = 30;
				_roxy.show("145rx01", 0x900, 0, kCHANGE_ROXY_ANIMATION, _duration, 3);
			} else if (_roxyTalkTo == 17) {
				playRandomDigi2();
			} else {
				_roxyState = 35;
				kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);
			}
			break;

		case 34:
			digi_unload(_digiName2);
			_state2 = 0;
			_roxyTalkTo = 101;
			_roxyState = 33;
			kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);
			break;

		case 35:
			switch (_roxyTalkTo) {
			case 0:
				player_set_commands_allowed(false);
				_roxyState = 36;
				break;

			case 15:
				_roxyState = 37;
				break;

			case 100:
				_roxyState = 40;
				break;

			default:
				term_message("ERROR!!! Who is roxy talking to???");
				break;
			}

			series_play_with_breaks(PLAY17, "145rx02", 0x900, kCHANGE_ROXY_ANIMATION, 3);
			break;

		case 36:
			kernel_trigger_dispatch_now(kSPEAK_TO_ROXY);
			_roxyState = 40;
			kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);
			break;

		case 37:
			_state2 = 1;
			_roxyState = 38;
			series_play_with_breaks(PLAY18, "145rx04", 0x900, kCHANGE_ROXY_ANIMATION, 3);
			break;

		case 38:
			_roxy.show("145rx04", 0x900);

			switch (_state3) {
			case 1:
				_digiName1 = "145v901";
				break;
			case 2:
				_digiName1 = "145v902";
				break;
			case 3:
				_digiName1 = "145v903";
				break;
			case 4:
				_digiName1 = "145v904";
				break;
			case 5:
				_digiName1 = "145v905";
				break;
			case 6:
				_digiName1 = "145v906";
				break;
			default:
				break;
			}

			digi_preload(_digiName1);
			digi_play(_digiName1, 2, 125, -1);
			_roxyState = 42;
			kernel_timing_trigger(120, kCHANGE_ROXY_ANIMATION);
			break;

		case 39:
			_roxy.terminate();
			_roxyState = 40;
			kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);
			kernel_trigger_dispatch_now(10001);
			break;

		case 40:
			_roxy.show("145rx04", 0x900);
			break;

		case 41:
			_roxyTalkTo = 101;
			_roxy.terminate();
			_roxyState = 33;
			series_play_with_breaks(PLAY19, "145rx02", 0x900, kCHANGE_ROXY_ANIMATION, 3);
			break;

		case 42:
			digi_unload(_digiName1);
			_state2 = 0;
			_roxy.terminate();
			_roxyState = 41;
			series_play_with_breaks(PLAY20, "145rx06", 0x900, kCHANGE_ROXY_ANIMATION, 3);
			break;

		default:
			break;
		}
		break;

	case kCHANGE_VIPE_ANIMATION:
		switch (_vipeState) {
		case 39:
			_vipe.terminate();
			_vipeState = 40;
			kernel_trigger_dispatch_now(kCHANGE_VIPE_ANIMATION);
			kernel_trigger_dispatch_now(10001);
			break;

		case 40:
			_vipe.show("145vp04", 0x300);
			break;

		case 43:
			player_set_commands_allowed(true);
			_vipe.show("145vp02", 0x300);
			break;

		case 44:
			player_set_commands_allowed(true);
			_flag1 = true;

			if (_roxyTalkTo == 101) {
				_roxyTalkTo = 100;
				resetRoxy();
				_vipe.terminate();
				_vipeState = 45;
				series_play_with_breaks(PLAY21, "145vp02", 0x300, kCHANGE_VIPE_ANIMATION, 3);
			} else {
				kernel_timing_trigger(15, kCHANGE_VIPE_ANIMATION);
			}
			break;

		case 45:
			_vipeState = 46;
			series_play_with_breaks(PLAY22, "145vp06", 0x300, kCHANGE_VIPE_ANIMATION, 3);
			break;

		case 46:
			kernel_trigger_dispatch_now(kSPEAK_TO_VIPE);
			_vipeState = 40;
			kernel_trigger_dispatch_now(kCHANGE_VIPE_ANIMATION);
			break;

		case 47:
			_vipe.terminate();
			_vipeState = 43;
			series_play_with_breaks(PLAY23, "145vp02", 0x300, kCHANGE_VIPE_ANIMATION, 3);
			break;

		default:
			break;
		}
		break;

	case 6:
		digi_unload(_digiName1);
		_state2 = 0;
		break;

	case 7:
		if (!digi_play_state(1) && !_flag1 && _roxyTalkTo == 101) {
			switch (imath_ranged_rand(1, 3)) {
			case 1:
				_state3 = imath_ranged_rand(1, 6);
				_roxyTalkTo = 15;
				resetRoxy();
				break;

			case 2:
				playRandomDigi1();
				break;

			default:
				_roxyTalkTo = 17;
				resetRoxy();
				break;
			}
		}

		kernel_timing_trigger(imath_ranged_rand(600, 900), 7);
		break;

	case kSPEAK_TO_VIPE:
		conv_load_and_prepare("conv21", 11);
		conv_export_pointer_curr(&_G(flags)[V068], 0);
		conv_export_pointer_curr(&_G(flags)[V069], 1);
		conv_export_pointer_curr(&_G(flags)[V070], 2);
		conv_export_pointer_curr(&_G(flags)[V071], 3);
		conv_export_pointer_curr(&_G(flags)[V072], 4);
		conv_play_curr();
		break;

	case kSPEAK_TO_ROXY:
		conv_load_and_prepare("conv22", 12);
		conv_export_pointer_curr(&_G(flags)[V073], 0);
		conv_play_curr();
		break;

	case kSPEAK_TO_DRUMZ:
		player_set_commands_allowed(false);
		conv_load_and_prepare("conv23", 13);
		conv_export_value_curr(_G(flags)[V068], 0);
		conv_export_value_curr(_G(flags)[V069], 1);
		conv_export_pointer_curr(&_G(flags)[V074], 2);
		conv_export_pointer_curr(&_G(flags)[V075], 3);
		conv_export_pointer_curr(&_G(flags)[V076], 4);
		conv_play_curr();
		break;

	case 11:
		_vipeState = 47;
		kernel_trigger_dispatch_now(kCHANGE_VIPE_ANIMATION);
		_roxyState = 41;
		kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);
		break;

	case 12:
		_G(flags)[V073] = 1;
		player_set_commands_allowed(true);
		_roxyState = 41;
		kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);
		break;

	case 13:
		_G(flags)[V070] = 1;
		player_set_commands_allowed(true);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			player_set_commands_allowed(false);
			_drumzState = 26;
			break;

		case 2:
			disable_player();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY1, "145wi04", 0xa00, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 3:
			player_set_commands_allowed(false);
			loadSeries1();
			ws_hide_walker();
			_G(wilbur_should) = 4;
			_general.play("145wi02", 0xa00, 16, kCHANGE_WILBUR_ANIMATION, 6, 0, 100, 0, 0, 0, 14);
			digi_play("145w020", 1, 255, -1);
			break;

		case 4:
			_drumzState = 28;
			break;

		case 5:
			_general.terminate();
			_G(wilbur_should) = 7;
			series_play_with_breaks(PLAY3, "145wi02", 0xa00, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 6:
			disable_player();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY4, "145wi03", 0xa01, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 7:
			ws_unhide_walker();
			break;

		case 8:
			player_set_commands_allowed(false);
			ws_hide_walker();
			terminateMachineAndNull(_amplifier);
			_G(wilbur_should) = 9;
			series_play_with_breaks(PLAY2, "145wi04", 0xa01, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 9:
			enable_player();
			hotspot_set_active("AMPLIFIER ", false);
			inv_give_to_player("AMPLIFIER");
			break;

		case 10:
			player_set_commands_allowed(false);

			if (_state2) {
				kernel_timing_trigger(15, kCHANGE_WILBUR_ANIMATION);
			} else {
				_vipeState = 44;
				kernel_trigger_dispatch_now(kCHANGE_VIPE_ANIMATION);
			}
			break;

		case 11:
			player_set_commands_allowed(false);

			if (_roxyTalkTo == 101) {
				_roxyTalkTo = 0;
				resetRoxy();
			} else {
				kernel_timing_trigger(15, kCHANGE_WILBUR_ANIMATION);
			}
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

void Room145::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (!_G(flags)[kDrumzFled] && _G(player).walk_x >= 140 && _G(player).walk_x <= 240 &&
			_G(player).walk_y >= 295 && _G(player).walk_y <= 325)
		player_walk_to(139, 326);

	if (_G(player).walk_x >= 423 && _G(player).walk_x <= 540 &&
		_G(player).walk_y >= 308 && _G(player).walk_y <= 340)
		player_walk_to(541, 341);

	if (_G(player).walk_x >= 341 && _G(player).walk_x <= 480 &&
		_G(player).walk_y >= 326 && _G(player).walk_y <= 365)
		player_walk_to(481, 366);

	if (player_said("GEAR") && player_said("VERA'S DINER "))
		player_set_facing_at(680, 340);
}

void Room145::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("conv21")) {
		conv21();
	} else if (player_said("conv22")) {
		conv22();
	} else if (player_said("conv23")) {
		conv23();
	} else if (player_said("GEAR", "VERA'S DINER ")) {
		disable_player_commands_and_fade_init(1014);
	} else if (!_G(walker).wilbur_match(MATCH)) {
		return;
	}

	_G(player).command_ready = false;
}

void Room145::loadDrum() {
	static const char *NAMES[12] = {
		"145dz01", "145dz02", "145dz03", "145dz04", "145dz05",
		"145dz06", "145dz01s", "145dz02s", "145dz03s", "145dz04s",
		"145dz05s", "145dz06s"
	};
	for (int i = 0; i < 12; ++i)
		series_load(NAMES[i]);

	digi_preload("145_003");
	digi_preload("145_004");
}

void Room145::loadRx() {
	static const char *NAMES[8] = {
		"145rx01", "145rx02", "145rx04", "145rx06",
		 "145rx01s", "145rx02s", "145rx04s", "145rx06s"
	};
	for (int i = 0; i < 8; ++i)
		series_load(NAMES[i]);
}

void Room145::conv21() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (sound) {
		if (who == 1) {
			wilbur_speech(sound, 10001);
		} else {
			switch (node) {
			case 2:
			case 7:
			case 12:
				vipeSpeaking();
				break;

			case 3:
			case 4:
			case 5:
			case 6:
			case 8:
			case 11:
				roxySpeaking();
				break;

			case 9:
				if (entry <= 0) {
					roxySpeaking();
				} else if (entry <= 2) {
					vipeSpeaking();
				}
				break;

			case 10:
				if (entry <= 0) {
					vipeSpeaking();
				} else if (entry == 1) {
					roxySpeaking();
				}
				break;

			default:
				break;
			}
		}
	}
}

void Room145::conv22() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();

	if (sound) {
		if (who == 1)
			wilbur_speech(sound, 10001);
		else
			roxySpeaking();
	}
}

void Room145::conv23() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (sound) {
		if (who == 1) {
			wilbur_speech(sound, 10001);
		} else {
			switch (node) {
			case 2:
				switch (entry) {
				case 0:
					_drumzState = 20;
					break;
				case 1:
					_drumzState = 21;
					break;
				case 2:
				case 3:
					_drumzState = 22;
					break;
				case 4:
					_drumzState = 21;
					break;
				case 5:
					_drumzState = 20;
					break;
				case 6:
					_drumzState = 20;
					break;
				case 7:
					conv_resume_curr();
					break;
				default:
					break;
				}
				break;

			case 5:
				_drumzState = 20;
				break;

			default:
				break;
			}
		}
	}
}

void Room145::vipeSpeaking() {
	_vipe.terminate();
	_vipeState = 39;
	digi_play(conv_sound_to_play(), 1, 255, 5);
	_vipe.play("145vp04", 0x300, 4, -1., 6, -1);
}

void Room145::roxySpeaking() {
	_roxy.terminate();
	_roxyState = 39;
	digi_play(conv_sound_to_play(), 1, 255, 4);
	_roxy.play("145rx04", 0x900, 4, -1, 6, -1);
}

void Room145::loadSeries1() {
	series_load("145wi02");
	series_load("145wi02s");
	series_load("145wi03");
	series_load("145wi03s");
	series_load("145dz08");
	series_load("145dz08s");
}

int Room145::getRandomDrumzState() const {
	switch (imath_ranged_rand(1, 10)) {
	case 1:
		return 24;
	case 2:
		return 25;
	default:
		return 19;
	}
}

void Room145::resetRoxy() {
	switch (_val5) {
	case 31:
		_roxy.terminate();
		_roxyState = 32;
		kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);
		break;

	case 33:
		_roxy.terminate();
		_roxyState = 33;
		kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);
		break;

	default:
		term_message("Can't move roxy to home state, just have to wait!");
		break;
	}
}

void Room145::playRandomDigi1() {
	_state2 = 1;
	static const char *NAMES[4] = { "145v907", "145v908", "145v909", "145v910" };

	_digiName1 = NAMES[imath_ranged_rand(0, 3)];
	digi_preload(_digiName1);
	digi_play(_digiName1, 2, 125, 6);
}

void Room145::playRandomDigi2() {
	_state2 = 1;
	static const char *NAMES[13] = {
		"145r907", "145r908", "145r909", "145r910", "145r911",
		"145r912", "145r913", "145r914", "145r915", "145r916",
		"145r917", "145r918", "145r919"
	};

	_digiName2 = NAMES[imath_ranged_rand(0, 12)];
	_roxyState = 34;
	digi_preload(_digiName2);
	npc_say(_digiName2, 4, "145rx01", 0x900, 1, 3, 7, 2);
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
