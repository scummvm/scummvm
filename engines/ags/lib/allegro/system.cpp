/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/lib/allegro/system.h"
#include "common/system.h"

namespace AGS3 {

int color_depth;

SYSTEM_DRIVER system_none;
SYSTEM_DRIVER *system_driver;

_DRIVER_INFO _system_driver_list[] = {
//	{ SYSTEM_IOS, &system_none, true  },
	{ SYSTEM_NONE, &system_none, false },
	{ 0, nullptr , 0     }
};


void set_color_depth(int depth) {
	color_depth = depth;
}

int get_color_depth() {
	return color_depth;
}

int get_desktop_resolution(int *width, int *height) {
	if (*width)
		*width = g_system->getWidth();
	if (*height)
		*height = g_system->getHeight();

	return 0;
}

void request_refresh_rate(int rate) {
	// No implementation
}

void set_close_button_callback(void(*proc)()) {
	// No implementation
}

} // namespace AGS3
