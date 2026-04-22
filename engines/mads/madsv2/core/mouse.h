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

#ifndef MADS_CORE_MOUSE_H
#define MADS_CORE_MOUSE_H

#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/sprite.h"

namespace MADS {
namespace MADSV2 {

#define MOUSE_TIMING_ONE        7       /* Mouse 1st repeat delay (ticks) */
#define MOUSE_TIMING_TWO        2       /* Mouse 2nd repeat delay (ticks) */

#define MOUSE_DOUBLE_TIMING     5       /* Double click threshold (ticks) */
#define MOUSE_BALLISTIC_TIMING  36      /* Ballistic threshold    (ticks) */

extern word mouse_driver;       /* True if mouse driver is currently active  */
extern word mouse_known_mode;   /* True if mouse in a 320x200 graphics mode  */
extern int  mouse_video_mode;   /* Rom BIOS video mode # of current cursor   */
extern byte mouse_showing;      /* Mouse cursor showing status (0 = show)    */

extern int mouse_button;        /* Last button pressed (0 = left, 1 = right) */
extern int mouse_status;        /* Button status flags                       */
extern int mouse_x, mouse_y;    /* Most recent cursor position               */
extern int mouse_buttons;		/* Most recent button state                  */
extern bool mouse_start_stroke;  /* True if new button press this round       */
extern bool mouse_stroke_going;  /* True if any button currently down         */
extern bool mouse_changed;       /* True if position or any button changed    */
extern bool mouse_latched;       /* Internal use (same as mouse_stroke_going) */
extern bool mouse_stop_stroke;   /* True if button released this round        */
extern bool mouse_any_stroke;    /* True if button down or just now released  */
extern int mouse_old_x;         /* Cursor X position on previous round       */
extern int mouse_old_y;         /* Cursor Y position on previous round       */
extern long mouse_clock;        /* Timing clock to insure at least 1 tick    */


extern int mouse_init(int driver_flag, int mouse_video_mode);
extern int mouse_set_hotspot(int spot_x, int spot_y);
extern void mouse_change_cursor_begin();
extern void mouse_change_cursor_end();
extern void mouse_screen_swap(int mouse_video_mode);
extern int mouse_get_video_mode();
extern void mouse_begin_double(int first_video_mode, int second_video_mode,
	int mono_to_right, int auto_freedom);
extern void mouse_check_double();
extern void mouse_end_double();
extern void mouse_double_freedom(int freedom_flag);
extern void mouse_show();
extern void mouse_hide();
extern int mouse_get_status(int *x, int *y);
extern void mouse_timing();
extern void mouse_freeze();
extern void mouse_thaw();
extern void mouse_horiz_bound(int min_x, int max_x);
extern void mouse_vert_bound(int min_y, int max_y);
extern void mouse_force(int x, int y);
extern void mouse_set_work_buffer(byte *work_buffer, int wrap_value);
extern void mouse_set_view_port_loc(int x1, int y1, int x2, int y2);
extern void mouse_set_view_port(int dx, int dy);
extern int mouse_refresh_view_port();
extern void mouse_refresh_done();
extern void mouse_disable_scale();
extern void mouse_hard_cursor_mode(int mode, Palette *mypal);
extern const byte *mouse_get_stack();

/**
 * Returns true if the mouse is in the specified box
 */
extern int  mouse_in_box(int ul_x, int ul_y, int lr_x, int lr_y);

/**
 * Call at beginning of routine which will use double screen
 * cursor interaction; initializes global variables.
 */
extern void mouse_init_cycle();

/**
 * Call once at beginning of each input loop.  Reads mouse cursor
 * information, and checks double cursor status.
 */
extern void mouse_begin_cycle(int double_flag);

/**
 * Call once each loop at end of loop.  Clears cursor freedom
 * semaphore, and performs any timing that might be necessary.
 */
extern void mouse_end_cycle(int double_flag, int timing_flag);

extern void mouse_cursor_sprite(SeriesPtr series, int id);
extern void mouse_video_init();
extern void mouse_video_update(int from_x, int from_y,
	int unto_x, int unto_y, int size_x, int size_y);

} // namespace MADSV2
} // namespace MADS

#endif
