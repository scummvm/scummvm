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

#ifndef MADS_CORE_SCREEN_H
#define MADS_CORE_SCREEN_H

#include "mads/madsv2/core/window.h"

namespace MADS {
namespace MADSV2 {

/* some colors */

#define black                   0x00
#define blue                    0x01
#define green                   0x02
#define cyan                    0x03
#define red                     0x04
#define magenta                 0x05
#define yellow                  0x06
#define white                   0x07
#define hi_black                0x08
#define hi_blue                 0x09
#define hi_green                0x0a
#define hi_cyan                 0x0b
#define hi_red                  0x0c
#define hi_magenta              0x0d
#define hi_yellow               0x0e
#define hi_white                0x0f

#define colorbyte(f,b)  ( ( (b) << 4) + f )

#define colorhash(color)  ((long) ((((long)color.r) << 16) + (((long)color.g) << 8) + (((long)color.b) ) ))

#define screen_char_add(x,y)  ( ( ( (y) * 80) + x) * 2 )
#define screen_color_add(x,y) ( ( ( ( (y) * 80) + x) * 2 ) + 1 )

extern byte *screen;
extern int *screen_bound_x;

extern int screen_video_mode;
extern int screen_max_x;
extern int screen_max_y;

extern int screen_center_x;
extern int screen_center_y;

extern int screen_save_mode;
extern int screen_save_size;

extern int screen_normal_color;
extern int screen_hilite_color;

extern int screen_line_width;

extern Palette pal_orig;
extern Buffer  scr_live;

extern Window     screen_active;

/**
 * Sets # of lines in VGA text mode
 */
void  screen_set_size(short numlines);

/**
 * Sets uniform color & character for specified line
 */
void screen_wipe_line(short ul_x, short ul_y, short len, short wipe_color, byte wipe_char);

short screen_out(const char *outstring, short strcolor, short selcolor, short str_x, short str_y);
short screen_put(const char *outstring, short strcolor, short selcolor, short str_x, short str_y);
void  screen_set_colors(int normal_color, int hilite_color);
void  screen_set_line_width(int line_width);

/* DO NOT USE */
//short screen_show(const char *outstring, short locx, short locy);

short screen_show_line(const char *outstring, short locx, short locy);
short screen_write(const char *outstring);
short screen_write_line(const char *outstring);
void  screen_clear(int clear_color);
void  screen_dominant_mode(int dominant_mode);
void  screen_init(int video_mode);
void  screen_init_dual(int mono_left);
void  screen_shutdown_dual(int clear_flag);
void  screen_init_graphics(int which_mode);
void  screen_shutdown_graphics(int clear_flag);
void  screen_show_spot(const char *message, int wx, int wy, int class_, int num);

int screen_printf(int x, int y, const char *string, ...);
int screen_print(const char *string, ...);

void screen_init_text(int which_mode);
void screen_shutdown_text(int clear_flag);
void screen_save(void);
void screen_restore(void);
short screen_show_wide(const char *outstring, short locx, short locy, short width);

} // namespace MADSV2
} // namespace MADS

#endif
