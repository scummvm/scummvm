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

#include "m4/riddle/rooms/room.h"
#include "m4/riddle/vars.h"
#include "m4/riddle/riddle.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room::restoreAutosave() {
	if (g_engine->autosaveExists()) {
		_G(kernel).restore_slot = 0;
		auto oldMode = _G(kernel).trigger_mode;
		_G(kernel).trigger_mode = KT_DAEMON;
		disable_player_commands_and_fade_init(255);

		_G(kernel).trigger_mode = oldMode;
		player_set_commands_allowed(false);
	}
}

void Room::triggerMachineByHashCallback(frac16 myMessage, machine *) {
	int32 hi = myMessage >> 16;

	if (hi >= 0)
		kernel_trigger_dispatch_now(hi);
}

void Room::triggerMachineByHashCallbackNegative(frac16 myMessage, machine *) {
	int32 hi = myMessage >> 16;

	if (hi < 0)
		kernel_trigger_dispatchx(hi);
}

int Room::checkFlags(bool flag) {
	int count = 0;

	if (_G(flags)[V364] == 1) {
		++count;
		switch (_G(flags)[V005]) {
		case 1:
			_G(flags)[V351] = 1;
			break;
		case 2:
			_G(flags)[V352] = 1;
			break;
		case 3:
			_G(flags)[V353] = 1;
			break;
		case 4:
			_G(flags)[V354] = 1;
			break;
		default:
			break;
		}
	}

	if (_G(flags)[V365] == 1) {
		_G(flags)[V355] = 1;
		++count;
	}
	if (_G(flags)[V366] == 1) {
		_G(flags)[V356] = 1;
		++count;
	}

	if (flag) {
		if (_G(flags)[V367] == 1) {
			_G(flags)[V357] = 1;
			++count;
		}
		if (_G(flags)[V368] == 1) {
			_G(flags)[V358] = 1;
			++count;
		}
		if (_G(flags)[V369] == 1) {
			_G(flags)[V359] = 1;
			++count;
		}
	}

	if (_G(game).room_id == 501 && _G(flags)[V370] == 1) {
		_G(flags)[V360] = 1;
		++count;
	}

	if (_G(game).room_id != 401 && _G(flags)[V373] == 1) {
		_G(flags)[V363] = 1;
		++count;
	}

	if (_G(game).room_id != 501 && _G(flags)[V371] == 1) {
		_G(flags)[V361] = 1;
		++count;
	}

	if (_G(flags)[V372] == 1) {
		_G(flags)[V362] = 1;
		++count;
	}

	return count;
}

void Room::setFlag45() {
	_G(flags)[V045] = 1;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
