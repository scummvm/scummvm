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
}

void Section6::daemon() {
	// TODO
}

void Section6::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;
	bool kibble = player_said("KIBBLE");

	if (player_said("RAY GUN", "BLOCK OF ICE")) {
		_G(flags)[V247] = 1;
		_G(wilbur_should) = 6000;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		
	} else if (player_said("RAY GUN", "GERBILS") && _G(flags)[V243] == 6006) {
		_G(flags)[V247] = 1;
		term_message("Taking gun out to shoot gerbils...");
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("RAY GUN", "KIBBLE ")) {
		if (inv_player_has("KIBBLE")) {
			_G(wilbur_should) = gSERIES_PLAY_BREAK_2;
			ws_turn_to_face(9, gCHANGE_WILBUR_ANIMATION);

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

} // namespace Rooms
} // namespace Burger
} // namespace M4
