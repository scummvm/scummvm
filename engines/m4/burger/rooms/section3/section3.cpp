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

#include "m4/burger/rooms/section3/section3.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const seriesPlayBreak PLAY1[] = {
	{  0, 15, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 16, 46, "300w039", 1, 255, -1,    0, 0, nullptr, 0 },
	{ 47, 60, "300_006", 2, 255, -1,    0, 0, nullptr, 0 },
	{ 60, 60, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 60, -1, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


Section3::Section3() : Rooms::Section() {
	add(301, &_room301);
	add(302, &_room302);
	add(303, &_room303);
	add(304, &_room304);
	add(305, &_room305);
	add(306, &_room306);
	add(307, &_room307);
	add(310, &_room310);
	add(311, &_mine);
	add(312, &_mine);
	add(313, &_mine);
	add(314, &_mine);
	add(315, &_mine);
	add(316, &_mine);
	add(317, &_mine);
	add(318, &_mine);
	add(319, &_mine);
}

void Section3::daemon() {
	switch (_G(kernel).trigger) {
	case 3001:
		_G(game).new_room = 302;
		break;

	case 3002:
		_G(game).new_room = 303;
		break;

	case 3003:
		_G(game).new_room = 304;
		break;

	case 3004:
		_G(game).new_room = 305;
		break;

	case 3005:
		digi_unload("303_003");
		_G(game).new_room = 306;
		break;

	case 3006:
		_G(game).new_room = 307;
		break;

	case 3007:
		player_set_commands_allowed(false);
		gr_pal_clear(_G(master_palette));
		release_trigger_on_digi_state(3005, 1, 0);
		break;

	case 3008:
		_random1 = imath_ranged_rand(0, 5);
		digi_play(Common::String::format("300t001%c", 'a' + _random1).c_str(), 2, 55, -1, 300);
		break;

	case 10008:
		if (!_G(flags)[kTrufflesInMine])
			kernel_timing_trigger(15, 3008);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 3001:
			player_update_info();
			_G(flags)[kDrunkCarrotJuice] = 1;
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 3002;
			series_play_with_breaks(PLAY1, "300wdcj", _G(player_info).depth,
				kCHANGE_WILBUR_ANIMATION, 3, 6, _G(player_info).scale,
				_G(player_info).x, _G(player_info).y);
			break;

		case 3002:
			inv_move_object("DISTILLED CARROT JUICE", NOWHERE);
			inv_give_to_player("JUG");
			ws_unhide_walker();
			_G(wilbur_should) = 10001;

			wilbur_speech(_G(flags)[V101] ? "300w041" : "300w040");
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 10013:
			ws_unhide_walker();
			player_set_commands_allowed(true);
			_G(wilbur_should) = 10002;

			if (_G(flags)[kTrufflesInMine]) {
				_G(flags)[V002] = 1;
				wilbur_speech("300w060");
			}
			break;

		case 10015:
			_G(game).new_room = 306;
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

void Section3::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("DISTILLED CARROT JUICE")) {
		if (player_said("LOOK AT")) {
			wilbur_speech("300w030");
		} else if (player_said("TRUFFLES") || player_said("TROUGH")) {
			wilbur_speech("300w031");
		} else if (player_said_any("GEAR", "WILBUR")) {
			_G(wilbur_should) = 3001;
			ws_turn_to_face(3, kCHANGE_WILBUR_ANIMATION);
		} else if (player_said("BOTTLE")) {
			wilbur_speech("300w076");
		} else if (player_said("TAKE") && inv_player_has("DISTILLED CARROT JUICE")) {
			wilbur_speech("999w021");
		} else {
			wilbur_speech("300w042");
		}
	} else if (player_said("BURGER MORSEL")) {
		if (player_said("LOOK AT")) {
			wilbur_speech("300w006");
		} else if (player_said("TROUGH")) {
			wilbur_speech("300w004");
		} else if (player_said("WILBUR")) {
			wilbur_speech("300w013");
		} else if (player_said("TRUFFLES") && _G(game).room_id == 310) {
			wilbur_speech("300w012");
		} else if (player_said("STOVE") || player_said("FRYING PAN") || player_said("POT")) {
			wilbur_speech("300w011");
		} else if (player_said("TAKE") && inv_player_has("BURGER MORSEL")) {
			wilbur_speech("999w021");
		} else {
			wilbur_speech("300w014");
		}
	} else if (player_said("MATCHES")) {
		if (player_said("LOOK AT")) {
			wilbur_speech("300w015");
		} else if (player_said("TRUFFLES")) {
			wilbur_speech("300w016");
		} else if (player_said("CABIN")) {
			wilbur_speech("300w017");
		} else if (player_said("WILBUR")) {
			wilbur_speech("300w024");
		} else if (player_said("TAKE") && inv_player_has("MATCHES")) {
			wilbur_speech("999w021");
		} else {
			wilbur_speech("300w025");
		}
	} else if (player_said("JUG")) {
		if (player_said("LOOK AT")) {
			wilbur_speech("303w022");
		} else if (player_said("BOILER") || player_said("WILBUR") || player_said("GEAR")) {
			wilbur_speech("300w027");
		} else if (player_said("CARROT JUICE")) {
			wilbur_speech("300w050");
		} else if (player_said("TAKE") && inv_player_has("JUG")) {
			wilbur_speech("999w021");
		} else {
			wilbur_speech("300w029");
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Section3::mine_check_maze() {
	// This was used in the original to do sanity checks that the
	// mine maze data was all valid
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
