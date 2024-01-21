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

void Room::triggerMachineByHashCallback(frac16 myMessage, machine *sender) {
	int32 hi = myMessage >> 16;

	if (hi >= 0)
		kernel_trigger_dispatch_now(hi);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
