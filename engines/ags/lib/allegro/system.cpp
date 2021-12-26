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

#include "ags/lib/allegro/system.h"
#include "ags/lib/allegro/aintern.h"
#include "ags/globals.h"
#include "common/system.h"

namespace AGS3 {

GFX_MODE_LIST *get_gfx_mode_list(int card) {
	GFX_MODE_LIST *list = new GFX_MODE_LIST();
	list->num_modes = 1;
	list->mode = new GFX_MODE[1];

	GFX_MODE &gm = list->mode[0];
	gm.width = 320;
	gm.height = 200;
	gm.bpp = 32;

	return list;
}

void destroy_gfx_mode_list(GFX_MODE_LIST *list) {
	delete[] list->mode;
	delete list;
}

void set_color_depth(int depth) {
	_G(_color_depth) = depth;
}

int get_color_depth() {
	return _G(_color_depth);
}

int get_desktop_resolution(int32_t *width, int32_t *height) {
	// TODO: ScummVM has a hardcoded dummy desktop resolution. See if there's any
	// need to change the values, given we're hardcoded for pretend full-screen
	if (width)
		*width = 640;
	if (height)
		*height = 480;

	return 0;
}

void request_refresh_rate(int rate) {
	// No implementation
}

void set_close_button_callback(void(*proc)()) {
	// No implementation
}

} // namespace AGS3
