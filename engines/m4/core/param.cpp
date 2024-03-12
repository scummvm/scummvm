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

#include "common/config-manager.h"
#include "m4/core/param.h"
#include "m4/vars.h"
#include "m4/m4.h"

namespace M4 {

void param_init() {
}

void param_shutdown() {
}

void parse_all_flags() {
	if (ConfMan.hasKey("LOG"))
		_G(kernel).use_log_file = ConfMan.getBool("LOG");

	if (ConfMan.hasKey("USECACHE"))
		_G(kernel).suppress_cache = true;
	else if (ConfMan.hasKey("NOCACHE"))
		_G(kernel).suppress_cache = false;

	_G(kernel).track_open_close = ConfMan.hasKey("FILECOUNT");
	_G(kernel).start_up_with_dbg_ws = ConfMan.hasKey("W");
	_G(kernel).use_debug_monitor = ConfMan.hasKey("H");

	if (ConfMan.hasKey("M"))
		_G(mem_to_alloc) = ConfMan.getInt("M");

	if (ConfMan.hasKey("R")) {
		int val = ConfMan.getInt("R");

		if (!val) {
			_G(kernel).last_save = -1;
			_G(kernel).restore_game = true;
		} else {
			_G(kernel).last_save = val;
			_G(kernel).restore_game = true;
		}
	}

	if (ConfMan.hasKey("V")) {
		g_engine->showEngineInfo();
		_G(system_shutting_down) = true;
		return;
	}

	if (ConfMan.hasKey("T")) {
		int room = ConfMan.getInt("T");

		if (room) {
			_G(kernel).teleported_in = true;
			_G(game).new_room = room;
			_G(game).new_section = (room / 100);
		}
	}
}

} // namespace M4
