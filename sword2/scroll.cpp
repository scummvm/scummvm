/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "driver/driver96.h"
#include "debug.h"
#include "defs.h"
#include "header.h"
#include "interpreter.h"
#include "layers.h"
#include "scroll.h"

namespace Sword2 {

// max no of pixel allowed to scroll per cycle
#define MAX_SCROLL_DISTANCE 8

// used to be a define, but now it's flexible (see new functions below)
uint8 scroll_fraction = 16;

void Set_scrolling(void) {
	// normally we aim to get George's feet at (320,250) from top left
	// of screen window
	// feet_x = 128 + 320
	// feet_y = 128 + 250

	// set scroll offsets according to the player's coords

	int16 offset_x;
	int16 offset_y;
	int16 dx, dy;
	uint16 scroll_distance_x;	// how much we want to scroll
	uint16 scroll_distance_y;

	// if the scroll offsets are being forced in script
	if (SCROLL_X || SCROLL_Y) {
		// ensure not too far right
		if (this_screen.max_scroll_offset_x > SCROLL_X)
			this_screen.scroll_offset_x = SCROLL_X;
		else
 			this_screen.scroll_offset_x = this_screen.max_scroll_offset_x;

		// ensure not too far down
		if (this_screen.max_scroll_offset_y > SCROLL_Y)
			this_screen.scroll_offset_y = SCROLL_Y;
		else
 			this_screen.scroll_offset_y = this_screen.max_scroll_offset_y;
	} else {
		// George's offset from the centre - the desired position
		// for him

		offset_x = this_screen.player_feet_x - this_screen.feet_x;
		offset_y = this_screen.player_feet_y - this_screen.feet_y;

		// prevent scrolling too far left/right/up/down

		if (offset_x < 0)
			offset_x = 0;
		else if ((uint32) offset_x > this_screen.max_scroll_offset_x)
			offset_x = this_screen.max_scroll_offset_x;

		if (offset_y < 0)
			offset_y = 0;
		else if ((uint32) offset_y > this_screen.max_scroll_offset_y)
			offset_y = this_screen.max_scroll_offset_y;

		// first time on this screen - need absolute scroll
		// immediately!

		if (this_screen.scroll_flag == 2) {
			debug(5, "init scroll");
			this_screen.scroll_offset_x = offset_x;
			this_screen.scroll_offset_y = offset_y;
			this_screen.scroll_flag = 1;
		} else {
			// catch up with required scroll offsets - speed
			// depending on distance to catch up (dx and dy) &
			// 'SCROLL_FRACTION' used, but limit to certain
			// number of pixels per cycle (MAX_SCROLL_DISTANCE)

			dx = this_screen.scroll_offset_x - offset_x;
			dy = this_screen.scroll_offset_y - offset_y;

			// current scroll_offset_x is less than the required
			// value

			// NB. I'm adding 1 to the result of
			// dx / SCROLL_FRACTION, because it would otherwise
			// not scroll at all when dx < SCROLL_FRACTION

			if (dx < 0) {
				// => inc by (fraction of the differnce)
				// NB. dx is -ve, so we subtract
				// dx / SCROLL_FRACTION

				scroll_distance_x = 1 - dx / scroll_fraction;

				if (scroll_distance_x > MAX_SCROLL_DISTANCE)
					scroll_distance_x = MAX_SCROLL_DISTANCE;

				this_screen.scroll_offset_x += scroll_distance_x;			} else if (dx > 0) {
				// current scroll_offset_x is greater than
				// the required value
				// => dec by (fraction of the differnce)

				scroll_distance_x = 1 + dx / scroll_fraction;

				if (scroll_distance_x > MAX_SCROLL_DISTANCE)
					scroll_distance_x = MAX_SCROLL_DISTANCE;

				this_screen.scroll_offset_x -= scroll_distance_x;
			}

			if (dy < 0) {
				scroll_distance_y = 1 - dy / scroll_fraction;

				if (scroll_distance_y > MAX_SCROLL_DISTANCE)
					scroll_distance_y = MAX_SCROLL_DISTANCE;

				this_screen.scroll_offset_y += scroll_distance_y;
			} else if (dy > 0) {
				scroll_distance_y = 1 + dy / scroll_fraction;

				if (scroll_distance_y > MAX_SCROLL_DISTANCE)
					scroll_distance_y = MAX_SCROLL_DISTANCE;

				this_screen.scroll_offset_y -= scroll_distance_y;
			}
		}
	}
}

int32 FN_set_scroll_coordinate(int32 *params) {
	// set the special scroll offset variables

	// call when starting screens and to change the camera within screens

	// call AFTER FN_init_background() to override the defaults

	// called feet_x and feet_y to retain intelectual compatibility with
	// Sword1 !

	// feet_x & feet_y refer to the physical screen coords where the
	// system will try to maintain George's feet

	// params:	0 feet_x value
	// 		1 feet_y value

	this_screen.feet_x = params[0];
	this_screen.feet_y = params[1];
	return IR_CONT;
}

int32 FN_set_scroll_speed_normal(int32 *params) {
	scroll_fraction = 16;
	return IR_CONT;
}

int32 FN_set_scroll_speed_slow(int32 *params) {
	scroll_fraction = 32;
	return IR_CONT;
}

} // End of namespace Sword2
