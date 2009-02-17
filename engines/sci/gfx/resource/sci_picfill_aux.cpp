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

/* Generic pic auxbuf filling code, to be included by sci_pic_0.c
 *
 *
 * To use, define the following:
 *    AUXBUF_FILL: Name of the exported floodfill function
 *    AUXBUF_FILL_HELPER: Name of the helper function
 *
 * Define DRAW_SCALED to support scaled drawing, or leave it out for faster
 * processing.
 *
 */

#define CLIPMASK_HARD_BOUND 0x80 /* ensures that we don't re-fill filled stuff */

static void
AUXBUF_FILL_HELPER(gfxr_pic_t *pic, int old_xl, int old_xr, int y, int dy,
                   int clipmask, int control, int sci_titlebar_size) {
	int xl, xr;
	int oldytotal = y * 320;
#ifdef DRAW_SCALED
	unsigned const char fillmask = CLIPMASK_HARD_BOUND | 0x78;
#else
	unsigned const char fillmask = CLIPMASK_HARD_BOUND | 0x84;
#endif

	do {
		int ytotal = oldytotal + (320 * dy);
		int xcont;
		int state;

		y += dy;

		if (y < sci_titlebar_size || y > 199)
			return;

		xl = old_xl;
		if (!(pic->aux_map[ytotal + xl] & clipmask)) { /* go left */
			while (xl && !(pic->aux_map[ytotal + xl - 1] & clipmask))
				--xl;
		} else /* go right and look for the first valid spot */
			while ((xl <= old_xr) && (pic->aux_map[ytotal + xl] & clipmask))
				++xl;

		if (xl > old_xr) /* No fillable strip above the last one */
			return;

		if ((ytotal + xl) < 0) { fprintf(stderr, "AARGH-%d\n", __LINE__); BREAKPOINT(); }

		xr = xl;
		while (xr < 320 && !(pic->aux_map[ytotal + xr] & clipmask)) {
			pic->aux_map[ytotal + xr] |= fillmask;
			++xr;
		}

		if ((ytotal + xr) > 64000) {
			fprintf(stderr, "AARGH-%d\n", __LINE__);
			BREAKPOINT();
		}

		--xr;

		if (xr < xl)
			return;

		/* Check whether we need to recurse on branches in the same direction */
		if ((y > sci_titlebar_size && dy < 0)
		        || (y < 199 && dy > 0)) {

			state = 0;
			xcont = xr + 1;
			while (xcont <= old_xr) {
				if (pic->aux_map[ytotal + xcont] & clipmask)
					state = 0;
				else if (!state) { /* recurse */
					state = 1;
					AUXBUF_FILL_HELPER(pic, xcont, old_xr,
					                   y - dy, dy, clipmask, control, sci_titlebar_size);
				}
				++xcont;
			}
		}

		/* Check whether we need to recurse on backward branches: */
		/* left */
		if (xl < old_xl - 1) {
			state = 0;
			for (xcont = old_xl - 1; xcont >= xl; xcont--) {
				if (pic->aux_map[oldytotal + xcont] & clipmask)
					state = xcont;
				else if (state) { /* recurse */
					AUXBUF_FILL_HELPER(pic, xcont, state,
					                   y, -dy, clipmask, control, sci_titlebar_size);
					state = 0;
				}
			}
		}

		/* right */
		if (xr > old_xr + 1) {
			state = 0;
			for (xcont = old_xr + 1; xcont <= xr; xcont++) {
				if (pic->aux_map[oldytotal + xcont] & clipmask)
					state = xcont;
				else if (state) { /* recurse */
					AUXBUF_FILL_HELPER(pic, state, xcont,
					                   y, -dy, clipmask, control, sci_titlebar_size);
					state = 0;
				}
			}
		}

		if ((ytotal + xl) < 0) { fprintf(stderr, "AARGH-%d\n", __LINE__); BREAKPOINT() }
		if ((ytotal + xr + 1) > 64000) { fprintf(stderr, "AARGH-%d\n", __LINE__); BREAKPOINT(); }

		if (control)
			memset(pic->control_map->index_data + ytotal + xl, control, xr - xl + 1);

		oldytotal = ytotal;
		old_xr = xr;
		old_xl = xl;

	} while (1);
}


static void
AUXBUF_FILL(gfxr_pic_t *pic, int x, int y, int clipmask, int control, int sci_titlebar_size) {
	/* Fills the aux buffer and the control map (if control != 0) */
	int xl, xr;
	int ytotal = y * 320;
#ifdef DRAW_SCALED
	unsigned const char fillmask = 0x78;
#else
	unsigned const char fillmask = 0x4;
#endif

#ifndef DRAW_SCALED
	if (!control || !(clipmask & 4))
		return; /* Without pic scaling, we only do this to fill the control map */
#endif

	if (clipmask & 1)
		clipmask = 1; /* vis */
	else if (clipmask & 2)
		clipmask = 2; /* pri */
	else if (clipmask & 4)
		clipmask = 4; /* ctl */
	else return;

#ifdef DRAW_SCALED
	clipmask |= fillmask; /* Bits 3-5 */
#endif

	if (pic->aux_map[ytotal + x] & clipmask)
		return;

	pic->aux_map[ytotal + x] |= fillmask;

	xl = x;
	while (xl && !(pic->aux_map[ytotal + xl - 1] & clipmask)) {
		--xl;
		pic->aux_map[ytotal + xl] |= fillmask;
	}

	xr = x;
	while ((xr < 319) && !(pic->aux_map[ytotal + xr + 1] & clipmask)) {
		++xr;
		pic->aux_map[ytotal + xr] |= fillmask;
	}

	clipmask |= CLIPMASK_HARD_BOUND; /* Guarantee clipping */

	if (control) /* Draw the same strip on the control map */
		memset(pic->control_map->index_data + ytotal + xl, control, xr - xl + 1);

	if (y > sci_titlebar_size)
		AUXBUF_FILL_HELPER(pic, xl, xr, y, -1, clipmask, control, sci_titlebar_size);

	if (y < 199)
		AUXBUF_FILL_HELPER(pic, xl, xr, y, + 1, clipmask, control, sci_titlebar_size);
}


#undef CLIPMASK_HARD_BOUND
