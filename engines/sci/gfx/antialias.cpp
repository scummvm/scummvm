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

/** Antialiasing code **/

#include "sci/include/gfx_system.h"
#include "sci/include/gfx_tools.h"

namespace Sci {

static void antialiase_simple(gfx_pixmap_t *pixmap, int mask[], int shift_const, gfx_mode_t *mode) {
	int x, y, c;
	int bytespp = mode->bytespp;
	int line_size = bytespp * pixmap->xl;
	char *lastline[2];
	char *lastline_p = NULL;
	char *data_p = (char *)pixmap->data;

	lastline[0] = (char *)sci_malloc(line_size);
	lastline[1] = (char *)sci_malloc(line_size);

	for (y = 0; y < pixmap->yl; y++) {
		int visimode = (y > 0 && y + 1 < pixmap->yl) ? 1 : 0;
		unsigned long last_pixel = 0;

		memcpy(lastline[y & 1], data_p, line_size);
		lastline_p = lastline[(y & 1)^1];

		for (x = 0; x < pixmap->xl; x++) {
			unsigned long result = 0;

			if (x == 1)
				visimode++;
			else if (x + 1 == pixmap->xl)
				visimode--;

			for (c = 0; c < 3; c++) {
				unsigned long accum = 0;
				unsigned long reader = 0;
				int y_mode;

				/* Yes, bad compilers will read three times as often as neccessary.
				** This optimization is straightforward to detect (common subexpression
				** elemination), so I prefer to write the stuff semi-legibly...
				*/
				for (y_mode = 0; y_mode < 2; y_mode++)
					if ((y_mode == 0 && y > 0)
					        || (y_mode == 1 && y + 1 < pixmap->yl)) {

						char *src = (y_mode) ? data_p + line_size : lastline_p;

						if (x > 0) {
							memcpy(&reader, src - bytespp, bytespp);
							accum += ((reader >> shift_const) & mask[c]) << 0;
						}

						memcpy(&reader, src, bytespp);
						accum += ((reader >> shift_const) & mask[c]) << 1;

						if (x + 1 < pixmap->xl) {
							memcpy(&reader, src + bytespp, bytespp);
							accum += ((reader >> shift_const) & mask[c]) << 0;
						}
					}

				if (x > 0)
					accum += ((last_pixel >> shift_const) & mask[c]) << 1;

				memcpy(&reader, data_p, bytespp);
				if (c == 2)
					last_pixel = reader;
				accum += ((reader >> shift_const) & mask[c]) << 2;

				if (x + 1 < pixmap->xl) {
					memcpy(&reader, data_p + bytespp, bytespp);
					accum += ((reader >> shift_const) & mask[c]) << 1;
				}

				switch (visimode) {

				case 0:
					accum /= 9; // Only happens twelve times
					break;

				case 1:
					accum = (accum >> 6) + (accum >> 4); // 15/16 intensity
					break;

				case 2:
					accum >>= 4;
					break;

				default:
					accum = (c == 0) ? 0xffffffff : 0; // Error: mark as red
				}

				result |= (accum & mask[c]);
			}

			result <<= shift_const;
			memcpy(data_p, &result, bytespp);

			data_p += bytespp;
			lastline_p += bytespp;
		}
	}

	free(lastline[0]);
	free(lastline[1]);
}

void gfxr_antialiase(gfx_pixmap_t *pixmap, gfx_mode_t *mode, gfxr_antialiasing_t type) {
	int masks[3];
	int shift_const = 0;

#ifdef WORDS_BIGENDIAN
	shift_const = (sizeof(unsigned long) - mode->bytespp) << 3;
#endif

	masks[0] = mode->red_mask;
	masks[1] = mode->green_mask;
	masks[2] = mode->blue_mask;

	if (mode->palette)
		return;

	switch (type) {

	case GFXR_ANTIALIASING_NONE:
		break;

	case GFXR_ANTIALIASING_SIMPLE:
		antialiase_simple(pixmap, masks, shift_const, mode);
		break;

	default:
		GFXERROR("Invalid antialiasing mode %d (internal error)\n", type);
	}
}

} // End of namespace Sci
