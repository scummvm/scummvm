/***************************************************************************
 antialias.c Copyright (C) 2001 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

/** Antialiasing code **/

#include <gfx_system.h>
#include <gfx_tools.h>

static void
antialiase_simple(gfx_pixmap_t *pixmap, int mask[], int shift_const, gfx_mode_t *mode)
{
	int x, y, c;
	int bytespp = mode->bytespp;
	int line_size = bytespp * pixmap->xl;
	char *lastline[2];
	char *lastline_p = NULL;
	char *data_p = (char *) pixmap->data;

	lastline[0] = (char*)sci_malloc(line_size);
	lastline[1] = (char*)sci_malloc(line_size);

	for (y = 0; y < pixmap->yl; y++) {
		int visimode = (y > 0 && y+1 < pixmap->yl)? 1 : 0;
		unsigned long last_pixel;

		memcpy(lastline[y & 1], data_p, line_size);
		lastline_p = lastline[(y & 1)^1];

		for (x = 0; x < pixmap->xl; x++) {
			unsigned long result = 0;

			if (x == 1)
				visimode++;
			else if (x+1 == pixmap->xl)
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
					    || (y_mode == 1 && y+1 < pixmap->yl)) {

						char *src = (y_mode)? data_p + line_size : lastline_p;

						if (x > 0) {
							memcpy(&reader, src - bytespp, bytespp);
							accum += ((reader >> shift_const) & mask[c]) << 0;
						}

						memcpy(&reader, src, bytespp);
						accum += ((reader >> shift_const) & mask[c]) << 1;

						if (x+1 < pixmap->xl) {
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

				if (x+1 < pixmap->xl) {
					memcpy(&reader, data_p + bytespp, bytespp);
					accum += ((reader >> shift_const) & mask[c]) << 1;
				}

				switch (visimode) {

				case 0: accum /= 9; /* Only happens twelve times */
					break;

				case 1: accum = (accum >> 6) + (accum >> 4); /* 15/16 intensity */
					break;

				case 2: accum >>= 4;
					break;

				default: accum = (c == 0)? 0xffffffff : 0; /* Error: mark as red */
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

void
gfxr_antialiase(gfx_pixmap_t *pixmap, gfx_mode_t *mode, gfxr_antialiasing_t type)
{
	int masks[3];
	int shift_const = 0;

#ifdef WORDS_BIGENDIAN
	shift_const = (sizeof(unsigned long) - mode->bytespp) << 3;
#endif /* WORDS_BIGENDIAN */

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

	return;
}
