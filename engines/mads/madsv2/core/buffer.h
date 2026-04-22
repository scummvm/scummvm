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

#ifndef MADS_CORE_BUFFER_H
#define MADS_CORE_BUFFER_H

#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/font.h"

namespace MADS {
namespace MADSV2 {

#define MAX_PEEL_VALUE                  80

#define BUFFER_CREATED_PAGE_HANDLE      16384   /* Mask if EMS page handle created on the fly */

#define BUFFER_PRESERVE                 -1      /* Normal preserve attempt (EMS or disk) */
#define BUFFER_PRESERVE_RAM             -2      /* Disallows disk preservation           */
#define BUFFER_ATTEMPT_CONVENTIONAL     -8      /* Attempt conventional preserve         */

#define BUFFER_PRESERVED_CONVENTIONAL   -1      /* Preserved in conventional memory */
#define BUFFER_NOT_PRESERVED            -3      /* All attempts to preserve failed  */
#define BUFFER_PRESERVED_DISK           -10     /* Preserved on disk                */


extern int buffer_restore_keep_flag;
extern word pattern_control_value;
extern int  auto_pattern;


/**
 * Allocates space for a buffer, and fills in embedded data.
 * Returns TRUE if successful.
 */
extern bool buffer_init(Buffer *buf, word x, word y);

/**
 * Allocates space for a buffer, and fills in embedded data.
 * Returns TRUE if successful.
 */
extern bool buffer_init_name(Buffer *buf, word x, word y, const char *block_name);

/**
 * De-allocates the data associated with the buffer
 * @param buf 
 * @return	Returns true if successful.
 */
extern bool buffer_free(Buffer *buf);

/**
 * Fills an entire buffer with a single byte value.
 * @return	Returns true if successful.
 */
extern bool buffer_fill(Buffer target, byte value);

/**
 * Copies a "rectangular" buffer area from "from" to "unto".  Size
 * of copied rectangle is determined by "size_x, size_y".  Upper left
 * corner in BOTH buffers is indicated by "ul_x, ul_y".  (To copy
 * using separate corner coordinates in each buffer, use
 * buf_rect_copy_2 ().
 *
 *      * Handles "null" copies (e.g. one or both sizes are 0) correctly.
 *      * Optimizes for word copies.
 * @return	Returns TRUE if successful.
 */
extern bool buffer_rect_copy(Buffer from, Buffer unto,
	int ul_x, int ul_y, int size_x, int size_y);

/**
 * Fills a "rectangular" buffer area with the specified byte value.
 * Upper left corner is determined by "ul_x, ul_y", and sizes are
 * determined by "size_x, size_y."  No range checking is performed,
 * but "null" copies (x or y size 0) are handled correctly.
 * @return	Returns TRUE if successful; false if buffer invalid.
 */
extern bool buffer_rect_fill(Buffer target,
	int ul_x, int ul_y, int size_x, int size_y, byte value);

/**
 * Copies a "rectangular" buffer area from "from" to "unto".  Size
 * of copied rectangle is determined by "size_x, size_y".  Upper left
 * corner in source buffer is indicated by "from_x, from_y", and
 * "unto_x, unto_y" determines upper left corner in destination
 * buffer (if upper left corner coordinates are the same in both
 * buffers, buf_rect_copy() can be used instead).
 *
 * @return	Returns TRUE if successful.
 */
extern bool buffer_rect_copy_2(Buffer from, Buffer unto,
	int    from_x, int    from_y,
	int    unto_x, int    unto_y,
	int    size_x, int    size_y);

/**
 * Given X and Y, sets pixel to color C
 */
void buffer_put_pixel(Buffer buf, word x, word y, byte c);

/**
 * Given X and Y, gets the pixel at that position.
 */
byte buffer_get_pixel(Buffer buf, word x, word y);

/**
 * Given starting and ending points on the X axis, and the constant
 * Y value, draws a line in the given color on the live MCGA screen.
 */
void buffer_hline(Buffer buf, word x1, word x2, word y, byte color);

/**
 * Given starting and ending points on the Y axis, and the constant
 * X value, draws a line in the given color on the live MCGA screen.
 */
void buffer_vline(Buffer buf, word x, word y1, word y2, byte color);

/**
 * Draws outside edge of retangle given home and size along both axis.
 */
void buffer_draw_box(Buffer buf, word x1, word y1, word x2, word y2, byte color);

/**
 * Draws a horizontal line by xoring/inverting the existing pixels
 */
void buffer_hline_xor(Buffer buf, int x1, int x2, int y);

/**
 * Draws a vertical line by xoring/inverting the existing pixels
 */
void buffer_vline_xor(Buffer buf, int x, int y1, int y2);

/**
 * Draws a cross-hairs at the specified x, y
 */
void buffer_draw_crosshair(Buffer buf, int x, int y);

/**
 * Draws a box using pixel xor inversion
 */
void buffer_draw_box_xor(Buffer buf, int x1, int y1, int x2, int y2);

/**
 * Scans the two buffers.  Locations that are not the same on both buffers
 * are changed (in buffer 1) to color <newcol>.  The outside bounds are returned
 * in the words pointed to by *XL,*XH,*YL,*YH.
 * @return	Returns -1 if an error occurs.
 */
extern bool buffer_get_delta_bounds(Buffer buf1, Buffer buf2,
	byte newcol, word *xl, word *xh, word *yl, word *yh);

/**
 * Returns a pointer to the pixels starting at a given X, Y position
 * @param buf	Buffer
 * @param x		x
 * @param y		y
 * @return		Pointer
 */
extern byte *buffer_pointer(Buffer *buf, int x, int y);

extern bool buffer_conform(Buffer *buffer, int *x, int *y,
	int *xs, int *ys);

/**
 * Copies a "rectangular" buffer area from "from" to "unto".  Size
 * of copied rectangle is determined by "size_x, size_y".  Upper left
 * corner in source buffer is indicated by "from_x, from_y", and
 * "unto_x, unto_y" determines upper left corner in destination
 * buffer (if upper left corner coordinates are the same in both
 * buffers, buf_rect_copy() can be used instead).
 *
 * Only copies on top of values 8-11 (the interface background colors)
 *
 * @return	Returns TRUE if successful.
 */
extern int buffer_inter_merge_2(Buffer from, Buffer unto,
	int from_x, int from_y, int unto_x, int unto_y, int size_x, int size_y);

/**
 * This is a column-major Bresenham line drawing algorithm - it iterates along X
 * as the outer loop and uses the accumulator to decide when to step in Y,
 * which suits lines that are wider than they are tall.
 */
extern void buffer_line(Buffer target, int x1, int y1, int x2, int y2,
	int color);

/**
 * This is a column-major Bresenham line drawing algorithm - it iterates along X
 * as the outer loop and uses the accumulator to decide when to step in Y,
 * which suits lines that are wider than they are tall. The only difference between
 * it and buffer_line, is that this version uses an xor rather than storing a color.
 */
extern void buffer_line_xor(Buffer target, int x1, int y1, int x2, int y2);

/**
 * Traces a Bresenham line from (x1,y1) to (x2,y2) across a packed-bit walk
 * buffer (8 walk-code bits per byte) and determines whether the path is
 * legally walkable.  Each time the line enters a new contiguous run of blocked
 * pixels the legality counter is decremented by ILLEGAL; if it reaches zero
 * the function returns immediately with that value.  Paths that stay entirely
 * clear, or whose legality counter never bottoms out, return LEGAL.  Basic
 * bounds checking is performed first: a NULL buffer or any coordinate outside
 * the buffer dimensions short-circuits to LEGAL without scanning.
 */
extern int buffer_legal(const Buffer &walk, int orig_wrap, int x1, int y1, int x2, int y2);

word buffer_rect_fill_pattern(Buffer target, int ul_x, int ul_y, int size_x, int size_y,
	int base_x, int base_y, int base_xs, byte value1, byte value2,
	word start_accum, word note_line);

/**
 * Fills a "rectangular" buffer area with the specified byte value.
 * Upper left corner is determined by "ul_x, ul_y", and sizes are
 * determined by "size_x, size_y."  No range checking is performed,
 * but "null" copies (x or y size 0) are handled correctly.
 * @return	Returns TRUE if successful; false if buffer invalid.
 */
extern bool buffer_rect_fill_swap(Buffer target, int ul_x, int ul_y,
	int size_x, int size_y, byte value1, byte value2);

void buffer_peel_horiz(Buffer *target, int peel);

void buffer_peel_vert(Buffer *target, int peel,
	byte *work_memory, long work_size);

int  buffer_to_disk(Buffer *source, int x, int y,
	int xs, int ys);
void buffer_from_disk(Buffer *source, int buffer_id,
	int keep_flag, int x, int y, int xs, int ys);

extern bool buffer_to_ems(Buffer *source, int page_handle,
	int source_ems_handle, int x, int y, int xs, int ys);
extern bool buffer_from_ems(Buffer *source, int page_handle,
	int target_ems_handle, int x, int y, int xs, int ys);

int  buffer_preserve(Buffer *source, int flags,
	int source_ems_handle, int x, int y, int xs, int ys);
void buffer_restore(Buffer *source, int preserve_handle,
	int target_ems_handle, int x, int y, int xs, int ys);

extern bool buffer_rect_translate(Buffer from, Buffer unto,
	int from_x, int from_y, int unto_x, int unto_y,
	int size_x, int size_y, byte *table);

extern bool buffer_scan(Buffer *buffer, int magic,
	int base_x, int base_y, int size_x, int size_y);

extern bool buffer_compare(Buffer *buffer0, Buffer *buffer1,
	int base_x, int base_y, int base_x2, int base_y2, int size_x, int size_y);

} // namespace MADSV2
} // namespace MADS

#endif
