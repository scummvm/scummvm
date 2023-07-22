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

#include "m4/graphics/gr_line.h"
#include "m4/vars.h"

namespace M4 {

void gr_vline_xor(Buffer *buf, int32 x, int32 y1, int32 y2) {
	byte *start;
	int32 i;

	if (y1 > y2) {
		i = y1; y1 = y2; y2 = i;
	}

	if ((x > buf->w) || (y1 > buf->h))
		return;

	if (y2 > buf->h)
		y2 = buf->h;	// Don't draw past bottom

	start = buf->data + x;

	for (i = y1; i < y2; i++, start += buf->stride)
		*start ^= 0xff;
}

void gr_hline_xor(Buffer *buf, int32 x1, int32 x2, int32 y) {
	byte *start;
	int32 i;

	if (x1 > x2) {
		i = x1; x1 = x2; x2 = i;
	}

	if ((y > buf->h) || (x1 > buf->w))
		return;

	start = gr_buffer_pointer(buf, x1, y);

	for (i = x1; i < x2; i++, start++)
		*start ^= 0xff;
}

void gr_vline(Buffer *buf, int32 x, int32 y1, int32 y2) {
	byte *start;
	int32 i;

	if (y1 > y2) {
		i = y1; y1 = y2; y2 = i;
	}

	if ((x > buf->w) || (y1 > buf->h))
		return;

	y2++;
	if (y2 > buf->h)
		y2 = buf->h;	// don't draw past bottom

	start = gr_buffer_pointer(buf, x, y1);

	for (i = y1; i < y2; i++, start += buf->stride)
		*start = _G(color);
}

void gr_hline(Buffer *buf, int32 x1, int32 x2, int32 y) {
	byte *start;
	int32 i;

	if (x1 > x2) {
		i = x1; x1 = x2; x2 = i;
	}

	if ((y > buf->h) || (x1 > buf->w))
		return;

	start = gr_buffer_pointer(buf, x1, y);

	x2++;
	if (x2 > buf->w)
		x2 = buf->w;

	for (i = x1; i < x2; i++, start++)
		*start = _G(color);
}

void gr_line(int32 x1, int32 y1, int32 x2, int32 y2, int32 color, Buffer *screen) {
	byte *myData = (byte *)screen->data;
	int32 y_unit, x_unit; 							// Variables for amount of change in x and y

	int32 offset = y1 * screen->stride + x1;		// Calculate offset into video RAM

	int32 ydiff = y2 - y1; 							// Calculate difference between y coordinates
	if (ydiff < 0) { 								// If the line moves in the negative direction
		ydiff = -ydiff; 	 						// ...get absolute value of difference
		y_unit = -screen->stride;					// ...and set negative unit in y dimension
	} else y_unit = screen->stride;					// Else set positive unit in y dimension

	int32 xdiff = x2 - x1;							// Calculate difference between x coordinates
	if (xdiff < 0) {								// If the line moves in the negative direction
		xdiff = -xdiff;								// ...get absolute value of difference
		x_unit = -1;								// ...and set negative unit in x dimension
	} else x_unit = 1;				 				// Else set positive unit in y dimension

	int32 error_term = 0;							// Initialize error term
	if (xdiff > ydiff) {								// If difference is bigger in x dimension
		int32 length = xdiff + 1;					// ...prepare to count off in x direction
		int32 i;
		for (i = 0; i < length; i++) {				// Loop through points in x direction
			myData[offset] = (char)color;			// Set the next pixel in the line to COLOR
			offset += x_unit;						// Move offset to next pixel in x direction
			error_term += ydiff;					// Check to see if move required in y direction
			if (error_term > xdiff) {				// If so...
				error_term -= xdiff;				// ...reset error term
				offset += y_unit;					// ...and move offset to next pixel in y dir.
			}
		}
	} else {										// If difference is bigger in y dimension
		int32 length = ydiff + 1;					// ...prepare to count off in y direction
		for (int32 i = 0; i < length; i++) {		// Loop through points in y direction
			myData[offset] = (char)color;			// Set the next pixel in the line to COLOR
			offset += y_unit;						// Move offset to next pixel in y direction
			error_term += xdiff; 		 			// Check to see if move required in x direction
			if (error_term > 0) {					// If so...
				error_term -= ydiff;				// ...reset error term
				offset += x_unit;					// ...and move offset to next pixel in x dir.
			}
		}
	}
}

} // namespace M4
