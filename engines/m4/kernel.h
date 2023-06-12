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

#ifndef M4_KERNEL_H
#define M4_KERNEL_H

namespace M4 {

struct Kernel {
	bool use_log_file = false;
	bool suppress_cache = false;
	bool track_open_close = false;
	bool hag_mode = false;
	bool start_up_with_dbg_ws = false;
	bool cheat_keys_enabled = false;
	bool use_debug_monitor = false;
	bool restore_game = false;
	int last_save = -1;
	bool teleported_in = false;
};

} // namespace M4

#endif
