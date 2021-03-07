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

#ifndef AGS_LIB_ALLEGRO_MOUSE_H
#define AGS_LIB_ALLEGRO_MOUSE_H

#include "common/events.h"
#include "ags/lib/allegro/base.h"
#include "ags/lib/allegro/alconfig.h"
#include "ags/lib/allegro/gfx.h"

namespace AGS3 {

#define MOUSEDRV_AUTODETECT  -1
#define MOUSEDRV_NONE         0

AL_FUNC(int, install_mouse, (void));
AL_FUNC(void, remove_mouse, (void));

AL_FUNC(int, poll_mouse, (void));
AL_FUNC(int, mouse_needs_poll, (void));

AL_FUNC(void, enable_hardware_cursor, (void));
AL_FUNC(void, disable_hardware_cursor, (void));

/* Mouse cursors */
#define MOUSE_CURSOR_NONE        0
#define MOUSE_CURSOR_ALLEGRO     1
#define MOUSE_CURSOR_ARROW       2
#define MOUSE_CURSOR_BUSY        3
#define MOUSE_CURSOR_QUESTION    4
#define MOUSE_CURSOR_EDIT        5
#define AL_NUM_MOUSE_CURSORS        6

#define MOUSE_FLAG_MOVE             1
#define MOUSE_FLAG_LEFT_DOWN        2
#define MOUSE_FLAG_LEFT_UP          4
#define MOUSE_FLAG_RIGHT_DOWN       8
#define MOUSE_FLAG_RIGHT_UP         16
#define MOUSE_FLAG_MIDDLE_DOWN      32
#define MOUSE_FLAG_MIDDLE_UP        64
#define MOUSE_FLAG_MOVE_Z           128
#define MOUSE_FLAG_MOVE_W           256

AL_FUNCPTR(void, mouse_callback, (int flags));

AL_FUNC(void, show_mouse, (BITMAP *bmp));
AL_FUNC(void, scare_mouse, (void));
AL_FUNC(void, scare_mouse_area, (int x, int y, int w, int h));
AL_FUNC(void, unscare_mouse, (void));
AL_FUNC(void, position_mouse, (int x, int y));
AL_FUNC(void, position_mouse_z, (int z));
AL_FUNC(void, set_mouse_range, (int x1, int y_1, int x2, int y2));
AL_FUNC(void, set_mouse_speed, (int xspeed, int yspeed));
AL_FUNC(void, select_mouse_cursor, (int cursor));
AL_FUNC(void, set_mouse_cursor_bitmap, (int cursor, BITMAP *bmp));
AL_FUNC(void, set_mouse_sprite_focus, (int x, int y));
AL_FUNC(void, get_mouse_mickeys, (int *mickeyx, int *mickeyy));
AL_FUNC(void, set_mouse_sprite, (BITMAP *sprite));
AL_FUNC(int, show_os_cursor, (int cursor));
AL_FUNC(int, mouse_on_screen, (void));

} // namespace AGS3

#endif
