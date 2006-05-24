/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef AGI_GRAPHICS_H
#define AGI_GRAPHICS_H

#include "common/stdafx.h"

#include "agi/agi.h"

namespace Agi {

#define GFX_WIDTH	320
#define GFX_HEIGHT	200
#define CHAR_COLS	8
#define CHAR_LINES	8

extern uint8 palette[];

/* Transparent layer */
extern uint8 layer1_data[];
extern uint8 layer2_data[];

void gfx_putblock(int x1, int y1, int x2, int y2);

void put_text_character(int, int, int, unsigned int, int, int);
void shake_screen(int);
void shake_start(void);
void shake_end(void);
void save_screen(void);
void restore_screen(void);

int init_video(void);
int deinit_video(void);
void schedule_update(int, int, int, int);
void do_update(void);
void put_screen(void);
void flush_block(int, int, int, int);
void flush_block_a(int, int, int, int);
void put_pixels_a(int, int, int, uint8 *);
void flush_screen(void);
void clear_screen(int);
void clear_console_screen(int);
void draw_box(int, int, int, int, int, int, int);
void draw_button(int, int, const char *, int, int);
int test_button(int, int, const char *);
void draw_rectangle(int, int, int, int, int);
void save_block(int, int, int, int, uint8 *);
void restore_block(int, int, int, int, uint8 *);
void init_palette(uint8 *);

void put_pixel(int, int, int);

void put_pixels_hires(int x, int y, int n, uint8 * p);
int keypress(void);
int get_key(void);
void print_character(int, int, char, int, int);
void poll_timer(void);

} // End of namespace Agi

#endif				/* AGI_GRAPHICS_H */
