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

#include "m4/burger/rooms/section6/section6.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const seriesPlayBreak PLAY1[] = {
	{  0, 10, nullptr,   1,   0, -1, 2048, 0, 0, 0 },
	{ 11, 30, "600_011", 2, 255, -1,    0, 0, 0, 0 },
	{ 31, 33, "600_001", 2, 255, -1,    0, 0, 0, 0 },
	{ 34, -1, "600_002", 1, 255, -1,    0, 0, 0, 0 },
	PLAY_BREAK_END
};

int Section6::_state1;
int Section6::_state2;
int Section6::_gerbilState;
int Section6::_state4;
int Section6::_savedX;
int Section6::_series603;

Section6::Section6() : Rooms::Section() {
	add(601, &_room601);
	add(602, &_room602);
	add(603, &_room603);
	add(604, &_room604);
	add(605, &_room605);
	add(606, &_room606);
	add(608, &_room608);
	add(609, &_room609);
	add(610, &_room610);
	add(612, &_room602);

	_state1 = 6000;
	_state2 = 3;
	_gerbilState = 0;
	_state4 = 0;
	_savedX = 0;
	_series603 = 0;
}

void Section6::daemon() {
	switch (_G(kernel).trigger) {
	case 6003:
	case 6004:
	case 6005:
	case 6006:
		_G(game).new_room = _G(kernel).trigger - 6000 + 600;
		break;

	case 6008:
		_G(game).new_room = 609;
		break;

	case 6009:
		_G(game).new_room = 610;
		break;

	case 6010:
		_G(game).new_room = 612;
		break;

	case 6011:
		if (_gerbilState == 6005)
			kernel_trigger_dispatch_now(6006);
		break;

	case 6013:
		switch (_state1) {
		case 6001:
			if (_G(flags)[kHampsterState] == 6000) {
				if (_G(game).room_id == 602)
					_state4 = 2;
				if (_G(game).room_id == 603)
					_state4 = 1;
				if (_G(game).room_id == 604)
					_state4 = 0;
			}

			_G(flags)[V250] = 1;
			inv_move_object("BLOCK OF ICE", NOWHERE);
			_state1 = 6002;
			ws_unhide_walker();

			if (_G(game).room_id == 602)
				_G(flags)[V245] = 10028;
			if (_G(game).room_id == 603)
				_G(flags)[V245] = 10029;
			if (_G(game).room_id == 604)
				_G(flags)[V245] = 10030;

			if (_G(flags)[kHampsterState] == 6000) {
				if (_G(game).room_id == 602)
					_state4 = 2;
				if (_G(game).room_id == 603)
					_state4 = 1;
				if (_G(game).room_id == 604)
					_state4 = 0;

				kernel_trigger_dispatch_now(6014);
			}

			kernel_trigger_dispatch_now(6013);
			_G(wilbur_should) = 10001;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 6002:
			term_message("Ga-boingy boingy boingy boing!");
			_series1 = series_play("602hop_1", 0xa80, 0, -1, 0, -1);
			_series2 = series_play("602hop_2", 0x780, 0, -1, 0, -1);
			_series3 = series_play("602hop_3", 0x400, 0, -1, 0, -1);
			break;

		default:
			break;
		}
		break;

	case 6014:
		if (_val1)
			freeDigi(_val1);

		switch (_state4) {
		case 0:
			if (_G(flags)[V245] == 10030) {
				term_message("** Ambience: Gerbils snoring with grasshoppers!  100 **");
				_val1 = 6001;
				digi_preload("604_003", 604);
				digi_play_loop("604_003", 3, 125, -1, 604);
			} else {
				term_message("** Ambience: Gerbils snoring!  100 **");
				_val1 = 6000;
				digi_preload("600_003", 600);
				digi_play_loop("600_003", 3, 155, -1, 600);
			}
			break;

		case 1: 
			if (_G(flags)[V245] == 10029) {
				term_message("** Ambience: Gerbils snoring with grasshoppers!  60 **");
				_val1 = 6002;
				digi_preload("600_003", 603);
				digi_play_loop("600_003", 3, 125, -1, 603);
			} else {
				term_message("** Ambience: Gerbils snoring!  60 **");
				_val1 = 6000;
				digi_preload("600_003", 600);
				digi_play_loop("600_003", 3, 65, -1, 600);
			}
			break;

		case 2:
			if (_G(flags)[V245] == 10028) {
				term_message("** Ambience: Gerbils snoring with grasshoppers!  30 **");
				_val1 = 6003;
				digi_preload("602_003", 602);
				digi_play_loop("602_003", 3, 125, -1, 602);
			} else {
				term_message("** Ambience: Gerbils snoring!  30 **");
				_val1 = 6000;
				digi_preload("600_003", 600);
				digi_play_loop("600_003", 3, 20, -1, 600);
			}
			break;

		case 3:
			if (_G(flags)[V245] == 10030) {
				term_message("** Ambience: Gerbils waking up with grasshoppers!  100 **");
				_val1 = 6005;
				digi_preload("604_005", 604);
				digi_play_loop("604_005", 3, 125, -1, 604);
			} else {
				term_message("** Ambience: Gerbils waking up!  100 **");
				_val1 = 6004;
				digi_preload("604_004", 604);
				digi_play_loop("604_004", 3, 155, -1, 604);
			}
			break;

		case 4:
			if ((_G(flags)[V245] == 10030 && _G(game).room_id == 604) ||
					(_G(flags)[V245] == 10029 && _G(game).room_id == 603) ||
					(_G(flags)[V245] == 10028 && _G(game).room_id == 602)) {
				term_message("** Ambience: Gerbils arming weapons with grasshoppers!  100 **'");
				_val1 = 6007;
				digi_preload("600_005", 600);
				digi_play_loop("600_005", 3, 155, -1, 600);
			} else {
				term_message("** Ambience: Gerbils arming weapons!  100 **");
				_val1 = 6006;
				digi_preload("600_004", 600);
				digi_play_loop("600_004", 3, 155, -1, 600);
			}
			break;

		case 5:
			if ((_G(flags)[V245] == 10030 && _G(game).room_id == 604) ||
					(_G(flags)[V245] == 10029 && _G(game).room_id == 603) ||
					(_G(flags)[V245] == 10028 && _G(game).room_id == 602)) {
				term_message("** Ambience: Gerbils entering with grasshoppers!  100 **");
				_val1 = 6009;
				digi_preload("600_007", 600);
				digi_play_loop("600_007", 3, 155, -1, 600);
			} else {
				term_message("** Ambience: Gerbils entering!  100 **");
				_val1 = 6008;
				digi_preload("600_006", 600);
				digi_play_loop("600_006", 3, 155, -1, 600);
			}
			break;

		case 6:
			term_message("** Ambience: Gerbils are firing at grasshoppers!  100 **");
			_val1 = 0;
			break;

		case 7:
			term_message("** Ambience: Gerbils are fried!  25 **");
			_val1 = 0;
			break;

		case 8:
			term_message("** Ambience: Gerbils are fried!  50 **");
			_val1 = 0;
			break;

		case 9:
			if (_G(flags)[V277] == 6003 && _G(flags)[V278] == 1) {
				term_message("** Ambience: Gerbils are fried and wheel running!  100 **");
				_val1 = 0;
			} else {
				term_message("** Ambience: Gerbils are fried!  100 **");
				_val1 = 0;
			}
			break;

		default:
			break;
		}
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 6000:
			player_set_commands_allowed(false);
			series_load("602hop_1");
			series_load("602hop_2");
			series_load("602hop_3");

			if (_G(flags)[kHampsterState] == 6006)
				digi_preload("600_007");

			if (_G(flags)[kHampsterState] == 6000) {
				if (_G(game).room_id == 602)
					digi_preload("602_003");
				if (_G(game).room_id == 603)
					digi_preload("603_003");
				if (_G(game).room_id == 604)
					digi_preload("604_003");
			}

			_G(wilbur_should) = 6001;
			ws_walk(352, 276, nullptr, kCHANGE_WILBUR_ANIMATION, 9);
			break;

		case 6001:
			switch (_G(game).room_id) {
			case 602:
				ws_hide_walker();
				_state1 = 6001;
				_G(wilbur_should) = 6002;
				_G(flags)[V244] = 10028;
				series_play_with_breaks(PLAY1, "602melt", 0x999, kCHANGE_WILBUR_ANIMATION,
					WITH_SHADOW | PRELOAD_SOUNDS);
				break;

			case 603:
				ws_hide_walker();
				_state1 = 6001;
				_G(wilbur_should) = 6002;
				_G(flags)[V244] = 10029;
				series_play_with_breaks(PLAY1, "603melt", 0x999, kCHANGE_WILBUR_ANIMATION,
					_G(executing) == WHOLE_GAME ? WITH_SHADOW | PRELOAD_SOUNDS : PRELOAD_SOUNDS);
				break;

			case 604:
				if (_G(flags)[kHampsterState] == 6006) {
					_G(wilbur_should) = 10002;
					player_set_commands_allowed(true);
				} else {
					_G(flags)[V244] = 10030;
					ws_hide_walker();
					_state1 = 6001;
					_G(wilbur_should) = 6002;
					series_play_with_breaks(PLAY1, "604melt", 0x999, kCHANGE_WILBUR_ANIMATION,
						_G(executing) == WHOLE_GAME ? WITH_SHADOW | PRELOAD_SOUNDS : PRELOAD_SOUNDS);
				}
				break;

			default:
				break;
			}
			break;

		case 6002:
			kernel_trigger_dispatch_now(6013);
			ws_unhide_walker();
			player_set_commands_allowed(true);
			_G(wilbur_should) = 10002;

			if (_G(flags)[V251] == 0) {
				_G(flags)[V251] = 1;
				wilbur_speech("600w008");
			}
			break;

		case 6003:
			_G(flags)[V266] = 0;
			kernel_trigger_dispatch_now(6006);
			break;

		case 10015:
			if (_G(executing) == INTERACTIVE_DEMO) {
				// After having clicked the teleporter/"failed normally" button we end up here
				// In the DEMO this click leads to the main menu (it does not restart the test sequence)
				_G(game).new_section = 9;
				_G(game).new_room = 901;
			} else {
				_G(game).new_room = 608;
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

void Section6::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;
	bool kibble = player_said("KIBBLE");

	if (player_said("RAY GUN", "BLOCK OF ICE")) {
		_G(flags)[V247] = 1;
		_G(wilbur_should) = 6000;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		
	} else if (player_said("RAY GUN", "GERBILS") && _G(flags)[kHampsterState] == 6006) {
		_G(flags)[V247] = 1;
		term_message("Taking gun out to shoot gerbils...");
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("RAY GUN", "KIBBLE ")) {
		if (inv_player_has("KIBBLE")) {
			_G(wilbur_should) = kSERIES_PLAY_BREAK_2;
			ws_turn_to_face(9, kCHANGE_WILBUR_ANIMATION);

		} else {
			wilbur_speech("602w011");
		}
	} else if (kibble && player_said("LOOK AT")) {
		wilbur_speech("602w009");
	} else if (kibble && (player_said("KIBBLE TRAY") || player_said("KIBBLE "))) {
		wilbur_speech("602w048");
	} else if (kibble && (player_said("MOTOR") || player_said("MOTOR "))) {
		wilbur_speech("600w003");
	} else if (kibble && player_said("TAKE") && inv_player_has("KIBBLE")) {
		wilbur_speech("602w010");

	} else if (player_said("RAY GUN")) {
		if (player_said("LOOK AT")) {
			wilbur_speech(_G(flags)[V247] ? "600w002" : "600w001");
		} else if (player_said("GERBIL HEAD") || player_said("GERBIL HEAD ") ||
				player_said("WILBUR")) {
			wilbur_speech("600w005");
		} else if (player_said("GERBIL PIECES")) {
			wilbur_speech("600w004");
		} else if (player_said("BARS")) {
			wilbur_speech("999w018");
		} else if (player_said("FLOOR") || player_said("FLOOR ") || player_said("ROOF")) {
			wilbur_speech("600w007");
		} else if (player_said("TUBE") || player_said("TUBE ") ||
				player_said("TUBE  ") || player_said("TUBE   ")) {
			wilbur_speech("600w003");
		} else if (player_said("TAKE")) {
			wilbur_speech("999w021");
		} else {
			wilbur_speech("600w003");
		}
	} else if (player_said("GERBILS")) {
		if (player_said("BLOCK OF ICE")) {
			wilbur_speech("600w013");
		} else if (player_said("BOTTLE")) {
			wilbur_speech("600w014");
		} else if (player_said("KIBBLE")) {
			wilbur_speech("600w009");
		} else {
			return;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Section6::freeDigi(int state) {
	switch (state) {
	case 6000:
		digi_unload("600_003");
		break;
	case 6001:
		digi_unload("604_003");
		break;
	case 6002:
		digi_unload("603_003");
		break;
	case 6003:
		digi_unload("602_003");
		break;
	case 6004:
		digi_unload("604_004");
		break;
	case 6005:
		digi_unload("604_005");
		break;
	case 6006:
		digi_unload("600_004");
		break;
	case 6007:
		digi_unload("600_005");
		break;
	case 6008:
		digi_unload("600_006");
		break;
	case 6009:
		digi_unload("600_007");
		break;
	case 6010:
		digi_unload("600xxxxx");
		break;
	case 6011:
		digi_unload("600_010");
		break;
	case 6012:
		digi_unload("600xxxxx");
		break;
	default:
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
