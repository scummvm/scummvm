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

#include "glk/zcode/processor.h"

namespace Glk {
namespace ZCode {

static struct {
	Story story_id;
	int pic;
	int pic1;
	int pic2;
} mapper[] = {
	{ ZORK_ZERO,  5, 497, 498 },
	{ ZORK_ZERO,  6, 501, 502 },
	{ ZORK_ZERO,  7, 499, 500 },
	{ ZORK_ZERO,  8, 503, 504 },
	{    ARTHUR, 54, 170, 171 },
	{    SHOGUN, 50,  61,  62 },
	{   UNKNOWN,  0,   0,   0 }
};

void Processor::z_draw_picture() {
	zword pic = zargs[0];
	zword y = zargs[1];
	zword x = zargs[2];
	int i;

	flush_buffer();

	Window &win = _wp[_wp._cwin];
	if (_storyId == ZORK_ZERO && _wp._cwin == 0) {
		// WORKAROUND: Zork Zero has pictures for graphics embedded in the text with specific
		// co-prdinates. We need to reset it to 0,0 to flag it should be drawn at the cursor
		x = y = 0;
	} else {
		assert(x && y);
		x += win[X_POS] - 1;
		y += win[Y_POS] - 1;
	}

	/* The following is necessary to make Amiga and Macintosh story
	 * files work with MCGA graphics files.  Some screen-filling
	 *  pictures of the original Amiga release like the borders of
	 *  Zork Zero were split into several MCGA pictures (left, right
	 *  and top borders).  We pretend this has not happened.
	 */
	for (i = 0; mapper[i].story_id != UNKNOWN; i++) {
		if (_storyId == mapper[i].story_id && pic == mapper[i].pic) {
			uint height1, width1;
			uint height2, width2;

			int delta = 0;

			os_picture_data(pic, &height1, &width1);
			os_picture_data(mapper[i].pic2, &height2, &width2);

			if (_storyId == ARTHUR && pic == 54)
				delta = h_screen_width / 160;

			assert(x && y);
			os_draw_picture(mapper[i].pic1, Point(x + delta, y + height1));
			os_draw_picture(mapper[i].pic2, Point(x + width1 - width2 - delta, y + height1));
		}
	}

	os_draw_picture(pic, Point(x, y));

	if (_storyId == SHOGUN && pic == 3) {
		uint height, width;

		os_picture_data(59, &height, &width);
		os_draw_picture(59, Point(h_screen_width - width + 1, y));
	}
}

void Processor::z_picture_data() {
	zword pic = zargs[0];
	zword table = zargs[1];
	uint height, width;
	int i;

	bool avail = os_picture_data(pic, &height, &width);

	for (i = 0; mapper[i].story_id != UNKNOWN; i++) {
		if (_storyId == mapper[i].story_id) {
			if (pic == mapper[i].pic) {
				uint height2, width2;

				avail &= os_picture_data(mapper[i].pic1, &height2, &width2);
				avail &= os_picture_data(mapper[i].pic2, &height2, &width2);

				height += height2;
			} else if (pic == mapper[i].pic1 || pic == mapper[i].pic2) {
				avail = false;
			}
		}
	}

	storew((zword)(table + 0), (zword)(height));
	storew((zword)(table + 2), (zword)(width));

	branch(avail);
}

void Processor::z_erase_picture() {
#ifdef TODO
	int height, width;

	zword y = zargs[1];
	zword x = zargs[2];

	flush_buffer();

	/* Do nothing if the background is transparent */

	if (hi(cwp->colour) == TRANSPARENT_COLOUR)
		return;

	if (y == 0)		/* use cursor line if y-coordinate is 0 */
		y = cwp->y_cursor;
	if (x == 0)    	/* use cursor column if x-coordinate is 0 */
		x = cwp->x_cursor;

	os_picture_data(zargs[0], &height, &width);

	y += cwp->y_pos - 1;
	x += cwp->x_pos - 1;

	os_erase_area(y, x, y + height - 1, x + width - 1, -1);
#endif
}

void Processor::z_set_margins() {
#ifdef TODO
	zword win = winarg2();

	flush_buffer();

	wp[win].left = zargs[0];
	wp[win].right = zargs[1];

	/* Protect the margins */

	if (wp[win].x_cursor <= zargs[0] || wp[win].x_cursor > wp[win].x_size - zargs[1]) {

		wp[win].x_cursor = zargs[0] + 1;

		if (win == cwin)
			update_cursor();

	}
#endif
}

void Processor::z_move_window(void) {
	flush_buffer();

	zword win = winarg0();
	_wp[win].setPosition(Point(zargs[2], zargs[1]));
}

void Processor::z_window_size() {
	flush_buffer();

	zword win = winarg0();
	_wp[win].setSize(Point(zargs[2], zargs[1]));
}

void Processor::z_window_style() {
#ifdef TODO
	zword win = winarg0();
	zword flags = zargs[1];

	flush_buffer();

	/* Supply default arguments */

	if (zargc < 3)
		zargs[2] = 0;

	/* Set window style */

	switch (zargs[2]) {
	case 0: wp[win].attribute = flags; break;
	case 1: wp[win].attribute |= flags; break;
	case 2: wp[win].attribute &= ~flags; break;
	case 3: wp[win].attribute ^= flags; break;
	}

	if (cwin == win)
		update_attributes();
#endif
}

void Processor::z_get_wind_prop() {
	flush_buffer();

	zword win = winarg0();
	zword prop = zargs[1];

	if (prop <= TRUE_BG_COLOR)
		store(_wp[win][(WindowProperty)prop]);
	else
		runtimeError(ERR_ILL_WIN_PROP);
}

void Processor::z_put_wind_prop() {
	flush_buffer();

	zword win = winarg0();
	WindowProperty prop = (WindowProperty)zargs[1];
	zword val = zargs[2];

	if (prop >= TRUE_FG_COLOR)
		runtimeError(ERR_ILL_WIN_PROP);

	_wp[win][prop] = val;
}

void Processor::z_scroll_window() {
#ifdef TODO
	zword win = winarg0();
	zword y, x;

	flush_buffer();

	/* Use the correct set of colours when scrolling the window */

	if (win != cwin && !amiga_screen_model())
		os_set_colour(lo(wp[win].colour), hi(wp[win].colour));

	y = wp[win].y_pos;
	x = wp[win].x_pos;

	os_scroll_area(y,
		x,
		y + wp[win].y_size - 1,
		x + wp[win].x_size - 1,
		(short)zargs[1]);

	if (win != cwin && !amiga_screen_model())
		os_set_colour(lo(cwp->colour), hi(cwp->colour));
#endif
}

void Processor::z_mouse_window() {
	// No implementation - since ScummVM can run as a window, it's better
	// not to constrain the area the mouse can move
}

void Processor::z_picture_table() {
	/* This opcode is used by Shogun and Zork Zero when the player
	 * encounters built-in games such as Peggleboz. Nowadays it is
	 * not very helpful to hold the picture data in memory because
	 * even a small disk cache avoids re-loading of data.
	 */
}

zword Processor::winarg0() {
	if (h_version == V6 && (short)zargs[0] == -3)
		return _wp._cwin;

	if (zargs[0] >= ((h_version == V6) ? 8 : 2))
		runtimeError(ERR_ILL_WIN);

	return zargs[0];
}

zword Processor::winarg2() {
	if (zargc < 3 || (short)zargs[2] == -3)
		return _wp._cwin;

	if (zargs[2] >= 8)
		runtimeError(ERR_ILL_WIN);

	return zargs[2];
}

} // End of namespace ZCode
} // End of namespace Glk
