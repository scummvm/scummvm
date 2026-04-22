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

#include "mads/madsv2/core/cursor.h"
#include "mads/madsv2/core/screen.h"

namespace MADS {
namespace MADSV2 {

int cursor_mode;             /* global cursor mode (insert/overwrite) */
int cursor_follow = false;
int text_x = 0, text_y = 0;

void cursor_set_size(short start, short finish) {
	// No implementation
}

void cursor_set_pos(short x, short y) {
	// No implementation
}

void cursor_get_pos(int *x, int *y) {
	*x = *y = 0;
}

void cursor_set_mode(int my_type) {
	// No implementation
}

void cursor_toggle_insert(void) {
	if (cursor_mode == CURSOR_OVERWRITE) {
		cursor_set_mode(CURSOR_INSERT);
	} else {
		cursor_set_mode(CURSOR_OVERWRITE);
	}
}

void cursor_set_follow(int follow) {
	cursor_follow = follow;
}

} // namespace MADSV2
} // namespace MADS
