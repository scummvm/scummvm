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

#ifndef AGS_LIB_ALLEGRO_SYSTEM_H
#define AGS_LIB_ALLEGRO_SYSTEM_H

#include "ags/lib/allegro/base.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/gfx.h"

namespace AGS3 {

#ifndef AL_METHOD
#define AL_METHOD(type, name, args)             type (*name) args
#endif

#define SYSTEM_AUTODETECT  0
#define SYSTEM_NONE        AL_ID('N','O','N','E')


struct GFX_MODE {
	int width, height, bpp;
};

struct GFX_MODE_LIST {
	int num_modes;                /* number of gfx modes */
	GFX_MODE *mode;               /* pointer to the actual mode list array */
};

struct SYSTEM_DRIVER {
	int  id;
	char *name;
	char *desc;
	char *ascii_name;
	AL_METHOD(int, init, (void));
	AL_METHOD(void, exit, (void));
	AL_METHOD(void, get_executable_name, (char *output, int size));
	AL_METHOD(int, find_resource, (char *dest, const char *resource, int size));
	AL_METHOD(void, set_window_title, (const char *name));
	AL_METHOD(int, set_close_button_callback, (AL_METHOD(void, proc, (void))));
	AL_METHOD(void, message, (const char *msg));
	AL_METHOD(void, assert, (const char *msg));
	AL_METHOD(void, save_console_state, (void));
	AL_METHOD(void, restore_console_state, (void));
	AL_METHOD(BITMAP *, create_bitmap, (int color_depth, int width, int height));
	AL_METHOD(void, created_bitmap, (BITMAP *bmp));
	AL_METHOD(BITMAP *, create_sub_bitmap, (BITMAP *parent, int x, int y, int width, int height));
	AL_METHOD(void, created_sub_bitmap, (BITMAP *bmp, BITMAP *parent));
	AL_METHOD(int, destroy_bitmap, (BITMAP *bitmap));
	AL_METHOD(void, read_hardware_palette, (void));
	AL_METHOD(void, set_palette_range, (const RGB *p, int from, int to, int retracesync));
	AL_METHOD(struct GFX_VTABLE *, get_vtable, (int color_depth));
	AL_METHOD(int, set_display_switch_mode, (int mode));
	AL_METHOD(void, display_switch_lock, (int lock, int foreground));
	AL_METHOD(int, desktop_color_depth, (void));
	AL_METHOD(int, get_desktop_resolution, (int *width, int *height));
	AL_METHOD(void, get_gfx_safe_mode, (int *driver, struct GFX_MODE *mode));
	AL_METHOD(void, yield_timeslice, (void));
	AL_METHOD(void *, create_mutex, (void));
	AL_METHOD(void, destroy_mutex, (void *handle));
	AL_METHOD(void, lock_mutex, (void *handle));
	AL_METHOD(void, unlock_mutex, (void *handle));
	AL_METHOD(_DRIVER_INFO *, gfx_drivers, (void));
	AL_METHOD(_DRIVER_INFO *, digi_drivers, (void));
	AL_METHOD(_DRIVER_INFO *, midi_drivers, (void));
	AL_METHOD(_DRIVER_INFO *, keyboard_drivers, (void));
	AL_METHOD(_DRIVER_INFO *, mouse_drivers, (void));
	AL_METHOD(_DRIVER_INFO *, joystick_drivers, (void));
	AL_METHOD(_DRIVER_INFO *, timer_drivers, (void));
};

extern SYSTEM_DRIVER system_none;
extern SYSTEM_DRIVER *system_driver;
extern _DRIVER_INFO _system_driver_list[];

extern void set_color_depth(int depth);
extern int get_color_depth();
extern int get_desktop_resolution(int *width, int *height);
extern void request_refresh_rate(int rate);
extern void set_close_button_callback(void(*proc)());

} // namespace AGS3

#endif
