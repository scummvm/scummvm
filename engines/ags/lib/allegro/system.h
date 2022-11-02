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

#ifndef AGS_LIB_ALLEGRO_SYSTEM_H
#define AGS_LIB_ALLEGRO_SYSTEM_H

#include "ags/lib/allegro/base.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/gfx.h"
#include "ags/shared/core/types.h"

namespace AGS3 {

#ifndef AL_METHOD
#define AL_METHOD(type, name, args)             type (*name) args
#endif

#define SYSTEM_AUTODETECT  0
#define SYSTEM_SCUMMVM     AL_ID('S','C','V','M')
#define SYSTEM_NONE        AL_ID('N','O','N','E')

#define GFX_SCUMMVM             AL_ID('S', 'C', 'V', 'M')

#define SWITCH_NONE           0
#define SWITCH_PAUSE          1
#define SWITCH_AMNESIA        2
#define SWITCH_BACKGROUND     3
#define SWITCH_BACKAMNESIA    4

#define SWITCH_IN             0
#define SWITCH_OUT            1

struct GFX_MODE {
int width, height, bpp;
};

struct GFX_MODE_LIST {
	int num_modes;          /* number of gfx modes */
	GFX_MODE *mode;         /* pointer to the actual mode list array */
};

struct SYSTEM_DRIVER {
	int  id;
	const char *name;
	const char *desc;
	const char *ascii_name;
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
};

/* creates and manages the screen bitmap */
struct GFX_DRIVER {
	int  id;
	AL_CONST char *name;
	AL_CONST char *desc;
	AL_CONST char *ascii_name;
	AL_METHOD(BITMAP *, init, (int w, int h, int v_w, int v_h, int color_depth));
	AL_METHOD(void, exit, (BITMAP *b));
	AL_METHOD(int, scroll, (int x, int y));
	AL_METHOD(void, vsync, (void));
	AL_METHOD(void, set_palette, (AL_CONST RGB *p, int from, int to, int retracesync));
	AL_METHOD(int, request_scroll, (int x, int y));
	AL_METHOD(int, poll_scroll, (void));
	AL_METHOD(void, enable_triple_buffer, (void));
	AL_METHOD(BITMAP *, create_video_bitmap, (int width, int height));
	AL_METHOD(void, destroy_video_bitmap, (BITMAP *bitmap));
	AL_METHOD(int, show_video_bitmap, (BITMAP *bitmap));
	AL_METHOD(int, request_video_bitmap, (BITMAP *bitmap));
	AL_METHOD(BITMAP *, create_system_bitmap, (int width, int height));
	AL_METHOD(void, destroy_system_bitmap, (BITMAP *bitmap));
	AL_METHOD(int, set_mouse_sprite, (BITMAP *sprite, int xfocus, int yfocus));
	AL_METHOD(int, show_mouse, (BITMAP *bmp, int x, int y));
	AL_METHOD(void, hide_mouse, (void));
	AL_METHOD(void, move_mouse, (int x, int y));
	AL_METHOD(void, save_video_state, (void));
	AL_METHOD(void, restore_video_state, (void));
	AL_METHOD(void, set_blender_mode, (int mode, int r, int g, int b, int a));
	AL_METHOD(GFX_MODE_LIST *, fetch_mode_list, (void));
	int w, h;                     /* physical (not virtual!) screen size */
	int linear;                   /* true if video memory is linear */
	long bank_size;               /* bank size, in bytes */
	long bank_gran;               /* bank granularity, in bytes */
	long vid_mem;                 /* video memory size, in bytes */
	long vid_phys_base;           /* physical address of video memory */
	int windowed;                 /* true if driver runs windowed */
};

extern void set_color_depth(int depth);
extern int get_color_depth();
extern int get_desktop_resolution(int32_t *width, int32_t *height);
extern void request_refresh_rate(int rate);
extern void set_close_button_callback(void(*proc)());

extern GFX_MODE_LIST *get_gfx_mode_list(int card);
extern void destroy_gfx_mode_list(GFX_MODE_LIST *list);

inline void vsync() {}
inline int set_display_switch_callback(int dir, AL_METHOD(void, cb, (void))) {
	return 0;
}
inline int set_display_switch_mode(int v) {
	return -1;
}

} // namespace AGS3

#endif
