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
#include "ags/lib/allegro/aintern.h"
#include "common/system.h"

namespace AGS3 {

SYSTEM_DRIVER system_none;

SYSTEM_DRIVER system_scummvm = {
	SYSTEM_SCUMMVM,
	nullptr,
	nullptr,
	"ScummVM Device",
	nullptr, // TODO: ios_sys_init,
	nullptr, // TODO: ios_sys_exit,
	nullptr,  /* AL_METHOD(void, get_executable_name, (char *output, int size)); */
	nullptr,  /* AL_METHOD(int, find_resource, (char *dest, AL_CONST char *resource, int size)); */
	nullptr,  /* AL_METHOD(void, set_window_title, (AL_CONST char *name)); */
	nullptr,  /* AL_METHOD(int, set_close_button_callback, (AL_METHOD(void, proc, (void)))); */
	nullptr,  /* AL_METHOD(void, message, (AL_CONST char *msg)); */
	nullptr,  /* AL_METHOD(void, assert, (AL_CONST char *msg)); */
	nullptr,  /* AL_METHOD(void, save_console_state, (void)); */
	nullptr,  /* AL_METHOD(void, restore_console_state, (void)); */
	nullptr,  /* AL_METHOD(struct BITMAP *, create_bitmap, (int color_depth, int width, int height)); */
	nullptr,  /* AL_METHOD(void, created_bitmap, (struct BITMAP *bmp)); */
	nullptr,  /* AL_METHOD(struct BITMAP *, create_sub_bitmap, (struct BITMAP *parent, int x, int y, int width, int height)); */
	nullptr,  /* AL_METHOD(void, created_sub_bitmap, (struct BITMAP *bmp, struct BITMAP *parent)); */
	nullptr,  /* AL_METHOD(int, destroy_bitmap, (struct BITMAP *bitmap)); */
	nullptr,  /* AL_METHOD(void, read_hardware_palette, (void)); */
	nullptr,  /* AL_METHOD(void, set_palette_range, (AL_CONST struct RGB *p, int from, int to, int retracesync)); */
	nullptr,  /* AL_METHOD(struct GFX_VTABLE *, get_vtable, (int color_depth)); */
	nullptr,  /* AL_METHOD(int, set_display_switch_mode, (int mode)); */
	nullptr,  /* AL_METHOD(void, display_switch_lock, (int lock, int foreground)); */
	nullptr,  /* AL_METHOD(int, desktop_color_depth, (void)); */
	nullptr,  /* AL_METHOD(int, get_desktop_resolution, (int *width, int *height)); */
	nullptr, // TODO: ios_get_gfx_safe_mode,  /*AL_METHOD(void, get_gfx_safe_mode, (int *driver, struct GFX_MODE *mode));*/
	nullptr,  /* AL_METHOD(void, yield_timeslice, (void)); */
	nullptr, // TODO: _ios_create_mutex,  /* AL_METHOD(void *, create_mutex, (void)); */
	nullptr, // TODO: _ios_destroy_mutex,  /* AL_METHOD(void, destroy_mutex, (void *handle)); */
	nullptr, // TODO: _ios_lock_mutex,  /* AL_METHOD(void, lock_mutex, (void *handle)); */
	nullptr, // TODO: _ios_unlock_mutex,  /* AL_METHOD(void, unlock_mutex, (void *handle)); */
	nullptr,  /* AL_METHOD(_DRIVER_INFO *, gfx_drivers, (void)); */
	nullptr,  /* AL_METHOD(_DRIVER_INFO *, digi_drivers, (void)); */
	nullptr,  /* AL_METHOD(_DRIVER_INFO *, midi_drivers, (void)); */
	nullptr,  /* AL_METHOD(_DRIVER_INFO *, keyboard_drivers, (void)); */
	nullptr,  /* AL_METHOD(_DRIVER_INFO *, mouse_drivers, (void)); */
	nullptr,  /* AL_METHOD(_DRIVER_INFO *, joystick_drivers, (void)); */
	nullptr   /* AL_METHOD(_DRIVER_INFO *, timer_drivers, (void)); */
};

_DRIVER_INFO _system_driver_list[] = {
	{ SYSTEM_SCUMMVM, &system_scummvm, true },
	{ SYSTEM_NONE, &system_none, false },
	{ 0, nullptr , 0     }
};

SYSTEM_DRIVER *system_driver = &system_scummvm;


GFX_MODE_LIST *get_gfx_mode_list(int card) {
	assert(card == 0);

	GFX_MODE_LIST *list = new GFX_MODE_LIST();
	list->num_modes = 1;
	list->mode = new GFX_MODE[1];

	GFX_MODE &gm = list->mode[0];
	gm.width = 320;
	gm.height = 200;
	gm.bpp = 16;

	return list;
}

void destroy_gfx_mode_list(GFX_MODE_LIST *list) {
	delete[] list->mode;
	delete list;
}

void set_color_depth(int depth) {
	_color_depth = depth;
}

int get_color_depth() {
	return _color_depth;
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
