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

#include "m4/burger/rooms/section.h"
#include "m4/gui/gui_vmng.h"
#include "m4/burger/vars.h"
#include "m4/m4.h"

namespace M4 {
namespace Burger {
namespace Rooms {

void Section::global_room_init() {
	int roomId = _G(game).room_id;

	if (roomId <= 800)
		_GINT().show();
	else
		_GINT().hide();

	// Disable commands for certain rooms
	if (roomId == 201 || roomId == 301 || roomId == 306 || roomId == 307 ||
		roomId == 401 || roomId == 501 || roomId == 511 || roomId == 512 ||
		roomId == 513 || roomId == 601 || roomId == 605 || roomId == 606 ||
		roomId == 608 || roomId == 609 || roomId == 610 || roomId == 801 ||
		roomId == 902) {
		player_set_commands_allowed(false);
	}

	if (roomId >= 950 || roomId == 902)
		mouse_hide();
	else
		mouse_show();

	// Do stuff that needs to be done each time a scene is started
	init_series_players();

	// Preload digi sounds
	if (_G(player).walker_in_this_scene) {
		switch (roomId) {
		case 102:
		case 103:
		case 105:
		case 134:
		case 135:
		case 143:
		case 174:
		case 175:
		case 176:
		case 304:
		case 505:
		case 506:
		case 507:
		case 509:
		case 602:
		case 603:
		case 604:
		case 612: {
			static const char *NAMES[] = {
				"fs_wood1", "fs_wood2", "fs_wood3", "fs_wood4", "fs_wood5", nullptr
			};
			_G(digi).loadFootstepSounds(NAMES);
			break;
		}

		case 106:
		case 139:
		case 144:
		case 145:
		case 302:
		case 303:
		case 305: {
			static const char *NAMES[] = {
				"fs_dirt1", "fs_dirt2", "fs_dirt3", "fs_dirt4", "fs_dirt5", nullptr
			};
			_G(digi).loadFootstepSounds(NAMES);
			break;
		}

		case 310:
		case 311:
		case 312:
		case 313:
		case 314:
		case 315:
		case 316:
		case 317:
		case 318:
		case 319: {
			static const char *NAMES[] = {
				"fs_mine1", "fs_mine2", "fs_mine3", "fs_mine4", "fs_mine5", nullptr
			};
			_G(digi).loadFootstepSounds(NAMES);
			break;
		}

		default: {
			static const char *NAMES[] = {
				"fs_hard1", "fs_hard2", "fs_hard3", "fs_hard4", "fs_hard5", nullptr
			};
			_G(digi).loadFootstepSounds(NAMES);
			break;
		}
		}
	}

	if (_G(executing) == WHOLE_GAME) {
		kernel_timing_trigger(900, kAdvanceHour, nullptr);
		kernel_timing_trigger(300, kAdvanceTime, nullptr);
	}
}

void Section::init_series_players() {
	_G(seriesPlayers).clear();
}

void Section::tick() {
	int oldTrigger = _G(kernel).trigger;
	_G(kernel).trigger = kCALLED_EACH_LOOP;
	g_engine->game_daemon_code();
	_G(kernel).trigger = oldTrigger;
}

void Section::daemon() {
	_G(kernel).continue_handling_trigger = true;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
