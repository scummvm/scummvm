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

#include "sci/gfx/gfx_resource.h"

/* Generic pic filling code, to be included by sci_pic_0.c
 *
 *
 * To use, define the following:
 *    AUXBUF_FILL: Name of the exported floodfill function
 *    AUXBUF_FILL_HELPER: Name of the helper function
 *    FILL_FUNCTION: Name of the exported floodfill function
 *    FILL_FUNCTION_RECURSIVE: Name of the helper function
 *
 * Define DRAW_SCALED to support scaled drawing, or leave it out for faster
 * processing.
 *
 */

namespace Sci {

#define CLIPMASK_HARD_BOUND 0x80 /* ensures that we don't re-fill filled stuff */

static void AUXBUF_FILL_HELPER(gfxr_pic_t *pic, int old_xl, int old_xr, int y, int dy,
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
		if (!(pic->aux_map[ytotal + xl] & clipmask)) { // go left
			while (xl && !(pic->aux_map[ytotal + xl - 1] & clipmask))
				--xl;
		} else // go right and look for the first valid spot
			while ((xl <= old_xr) && (pic->aux_map[ytotal + xl] & clipmask))
				++xl;

		if (xl > old_xr) // No fillable strip above the last one
			return;

		assert((ytotal + xl) >= 0);

		xr = xl;
		while (xr < 320 && !(pic->aux_map[ytotal + xr] & clipmask)) {
			pic->aux_map[ytotal + xr] |= fillmask;
			++xr;
		}

		assert((ytotal + xr) <= 64000);

		--xr;

		if (xr < xl)
			return;

		// Check whether we need to recurse on branches in the same direction
		if ((y > sci_titlebar_size && dy < 0) || (y < 199 && dy > 0)) {
			state = 0;
			xcont = xr + 1;
			while (xcont <= old_xr) {
				if (pic->aux_map[ytotal + xcont] & clipmask)
					state = 0;
				else if (!state) { // recurse
					state = 1;
					AUXBUF_FILL_HELPER(pic, xcont, old_xr, y - dy, dy, clipmask, control, sci_titlebar_size);
				}
				++xcont;
			}
		}

		// Check whether we need to recurse on backward branches:
		// left
		if (xl < old_xl - 1) {
			state = 0;
			for (xcont = old_xl - 1; xcont >= xl; xcont--) {
				if (pic->aux_map[oldytotal + xcont] & clipmask)
					state = xcont;
				else if (state) { // recurse
					AUXBUF_FILL_HELPER(pic, xcont, state, y, -dy, clipmask, control, sci_titlebar_size);
					state = 0;
				}
			}
		}

		// right
		if (xr > old_xr + 1) {
			state = 0;
			for (xcont = old_xr + 1; xcont <= xr; xcont++) {
				if (pic->aux_map[oldytotal + xcont] & clipmask)
					state = xcont;
				else if (state) { // recurse
					AUXBUF_FILL_HELPER(pic, state, xcont, y, -dy, clipmask, control, sci_titlebar_size);
					state = 0;
				}
			}
		}

		assert((ytotal + xl) >= 0);
		assert((ytotal + xr + 1) <= 64000);

		if (control)
			memset(pic->control_map->index_data + ytotal + xl, control, xr - xl + 1);

		oldytotal = ytotal;
		old_xr = xr;
		old_xl = xl;

	} while (1);
}


static void AUXBUF_FILL(gfxr_pic_t *pic, int x, int y, int clipmask, int control, int sci_titlebar_size) {
	// Fills the aux buffer and the control map (if control != 0)
	int xl, xr;
	int ytotal = y * 320;
#ifdef DRAW_SCALED
	unsigned const char fillmask = 0x78;
#else
	unsigned const char fillmask = 0x4;
#endif

#ifndef DRAW_SCALED
	if (!control || !(clipmask & 4))
		return; // Without pic scaling, we only do this to fill the control map
#endif

	if (clipmask & 1)
		clipmask = 1; // vis
	else if (clipmask & 2)
		clipmask = 2; // pri
	else if (clipmask & 4)
		clipmask = 4; // ctl
	else return;

#ifdef DRAW_SCALED
	clipmask |= fillmask; // Bits 3-5
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

	clipmask |= CLIPMASK_HARD_BOUND; // Guarantee clipping

	if (control) // Draw the same strip on the control map
		memset(pic->control_map->index_data + ytotal + xl, control, xr - xl + 1);

	if (y > sci_titlebar_size)
		AUXBUF_FILL_HELPER(pic, xl, xr, y, -1, clipmask, control, sci_titlebar_size);

	if (y < 199)
		AUXBUF_FILL_HELPER(pic, xl, xr, y, + 1, clipmask, control, sci_titlebar_size);
}


#undef CLIPMASK_HARD_BOUND


#ifdef FILL_RECURSIVE_DEBUG
#  define PRINT_DEBUG0(s) if (!fillmagc) fprintf(stderr, s)
#  define PRINT_DEBUG1(s,p1) if (!fillmagc) fprintf(stderr, s, p1)
#  define PRINT_DEBUG4(s,p1,p2,p3,p4) if (!fillmagc) fprintf(stderr, s, p1)
#else
#  define PRINT_DEBUG0(s)
#  define PRINT_DEBUG1(s,p1)
#  define PRINT_DEBUG4(s,p1,p2,p3,p4)
#endif

#ifdef DRAW_SCALED
#  define SCALED_CHECK(x) (x)
#  define IS_BOUNDARY(x, y, index)  (((index) & legalmask) != legalcolor)
#else
#  define SCALED_CHECK(x) 1
#  define IS_BOUNDARY(x, y, index) (								\
	(((x)+(y)) & 1)? /* figure out which part of the mask to use, to simulate dithering */	\
	   ((((index)) & ((legalmask) )) != ((legalcolor) & ((legalmask)))) /* odd coordinate */			\
	 : ((((index)) & ((legalmask) >> 8)) != ((legalcolor) & ((legalmask) >> 8))) /* even coordinate */			\
	)
#endif

static void FILL_FUNCTION_RECURSIVE(gfxr_pic_t *pic, int old_xl, int old_xr, int y, int dy, byte *bounds,
	int legalcolor, int legalmask, int color, int priority, int drawenable, int sci_titlebar_size) {
	int linewidth = pic->mode->scaleFactor * 320;
	int miny = pic->mode->scaleFactor * sci_titlebar_size;
	int maxy = pic->mode->scaleFactor * 200;
	int xl, xr;
	int oldytotal = y * linewidth;
#ifdef DRAW_SCALED
	int old_proj_y = -42;
	int proj_y;
	int proj_ytotal;
	int proj_x;
	int proj_xl_bound = 0;
	int proj_xr_bound = 0;
#endif

	do {
		int ytotal = oldytotal + (linewidth * dy);
		int xcont;
		int state;

		y += dy;

#ifdef FILL_RECURSIVE_DEBUG
		if (!fillc)
			return;
		else if (!fillmagc) {
			--fillc;
		}
#endif

		if (y < miny || y >= maxy) {
			PRINT_DEBUG0("ABRT on failed initial assertion!\n");
			return;
		}
#ifdef DRAW_SCALED
		proj_y = y / pic->mode->yfact;

		if (proj_y != old_proj_y) {
			// First, find the projected coordinates, unless known already:
			proj_ytotal = proj_y * 320;
			proj_x = old_xl / pic->mode->xfact;

			proj_xl_bound = proj_x;
			if (SCALED_CHECK(pic->aux_map[proj_ytotal + proj_xl_bound] & FRESH_PAINT)) {
				while (proj_xl_bound && pic->aux_map[proj_ytotal + proj_xl_bound - 1] & FRESH_PAINT)
					--proj_xl_bound;
			} else {
				while (proj_xl_bound < 319 && !(pic->aux_map[proj_ytotal + proj_xl_bound + 1] & FRESH_PAINT))
					++proj_xl_bound;

				if (proj_xl_bound < 319)
					++proj_xl_bound;
			}

			if (proj_xl_bound == 319
			        && !(pic->aux_map[proj_ytotal + proj_xl_bound] & FRESH_PAINT)) {
				PRINT_DEBUG0("ABRT because proj_xl_bound couldn't be found\n");
				return;
			}

			proj_xr_bound = (proj_xl_bound > proj_x) ? proj_xl_bound : proj_x;
			while ((proj_xr_bound < 319)
			        && pic->aux_map[proj_ytotal + proj_xr_bound + 1] & FRESH_PAINT)
				++proj_xr_bound;

#ifdef FILL_RECURSIVE_DEBUG
			if (!fillmagc) {
				fprintf(stderr, "l%d: {%d,%d} | ", proj_y, proj_xl_bound, proj_xr_bound);
				pic->aux_map[proj_y*320 + proj_xl_bound] |= 0x2;
				pic->aux_map[proj_y*320 + proj_xr_bound] |= 0x2;
			}
#endif

			proj_xl_bound *= pic->mode->xfact;
			if (proj_xl_bound)
				proj_xl_bound -= pic->mode->xfact - 1;

			if (proj_xr_bound < 319)
				++proj_xr_bound;
			proj_xr_bound *= pic->mode->xfact;
			proj_xr_bound += pic->mode->xfact - 1;

			old_proj_y = proj_y;
		}
#else
#  define proj_xl_bound 0
#  define proj_xr_bound 319
#endif

		// Now we have the projected limits, get the real ones:

		xl = (old_xl > proj_xl_bound) ? old_xl : proj_xl_bound;
		if (!IS_BOUNDARY(xl, y + 1, bounds[ytotal + xl])) { // go left as far as possible
			while (xl > proj_xl_bound && (!IS_BOUNDARY(xl - 1, y + 1, bounds[ytotal + xl - 1])))
				--xl;
		} else // go right until the fillable area starts
			while (xl < proj_xr_bound && (IS_BOUNDARY(xl, y + 1, bounds[ytotal + xl])))
				++xl;


		PRINT_DEBUG1("<%d,", xl);

		if ((xl > proj_xr_bound)
		        || (xl > old_xr)) {
			PRINT_DEBUG0("ABRT because xl > xr_bound\n");
			return;
		}

		xr = (xl > old_xl) ? xl : old_xl;
		while (xr < proj_xr_bound && (!IS_BOUNDARY(xr + 1, y + 1, bounds[ytotal + xr + 1])))
			++xr;

		PRINT_DEBUG1("%d> -> ", xr);

		if (IS_BOUNDARY(xl, y + 1,  bounds[ytotal + xl])) {
			PRINT_DEBUG0("ABRT because xl illegal\n");
			return;
		}

#ifdef DRAW_SCALED
		PRINT_DEBUG4("[%d[%d,%d]%d]\n", proj_xl_bound, xl, xr, proj_xr_bound);

		if (xl < proj_xl_bound && xr - 3*pic->mode->xfact < proj_xl_bound) {
			PRINT_DEBUG0("ABRT interval left of zone\n");
			return;
		}

		if (xr > proj_xr_bound && xl + 3*pic->mode->xfact > proj_xr_bound) {
			PRINT_DEBUG0("ABRT because interval right of zone\n");
			return;
		}
#endif

		if (drawenable & GFX_MASK_VISUAL)
			memset(pic->visual_map->index_data + ytotal + xl, color, xr - xl + 1);

		if (drawenable & GFX_MASK_PRIORITY)
			memset(pic->priority_map->index_data + ytotal + xl, priority, xr - xl + 1);


		// Check whether we need to recurse on branches in the same direction
		state = 0;
		xcont = xr + 1;
		while (xcont <= old_xr) {
			if (IS_BOUNDARY(xcont, y + 1, bounds[ytotal + xcont]))
				state = xcont;
			else if (state) { // recurse
				PRINT_DEBUG4("[%d[%d,%d],%d]: ", old_xl, xl, xr, old_xr);
				PRINT_DEBUG4("rec BRANCH %d [%d,%d] l%d\n", dy, state, xcont, y - dy);

				FILL_FUNCTION_RECURSIVE(pic, state, xcont, y - dy, dy, bounds, legalcolor,
				                        legalmask, color, priority, drawenable, sci_titlebar_size);
				state = 0;
			}
			++xcont;
		}

		// Check whether we need to recurse on backward branches:
		// left
		if (xl < old_xl - 1) {
			state = 0;
			for (xcont = old_xl - 1; xcont >= xl; xcont--) {
				if (IS_BOUNDARY(xcont, y, bounds[oldytotal + xcont]))
					state = xcont;
				else if (state) { // recurse
					PRINT_DEBUG4("[%d[%d,%d],%d]: ", old_xl, xl, xr, old_xr);
					PRINT_DEBUG4("rec BACK-LEFT %d [%d,%d] l%d\n", -dy, state, xcont, y);

					FILL_FUNCTION_RECURSIVE(pic, xcont, state, y, -dy, bounds,
					                        legalcolor, legalmask, color, priority, drawenable,
					                        sci_titlebar_size);
					state = 0;
				}
			}
		}

		// right
		if (xr > old_xr + 1) {
			state = 0;
			for (xcont = old_xr + 1; xcont <= xr; xcont++) {
				if (IS_BOUNDARY(xcont, y, bounds[oldytotal + xcont]))
					state = xcont;
				else if (state) { // recurse
					PRINT_DEBUG4("[%d[%d,%d],%d]: ", old_xl, xl, xr, old_xr);
					PRINT_DEBUG4("rec BACK-RIGHT %d [%d,%d] l%d\n", -dy, state, xcont, y);

					FILL_FUNCTION_RECURSIVE(pic, state, xcont, y, -dy, bounds,
					                        legalcolor, legalmask, color, priority, drawenable,
					                        sci_titlebar_size);
					state = 0;
				}
			}
		}

		oldytotal = ytotal;
		old_xl = xl;
		old_xr = xr;

	} while (1);
}


static void FILL_FUNCTION(gfxr_pic_t *pic, int x_320, int y_200, int color, int priority, int control, int drawenable,
	int sci_titlebar_size) {
	int linewidth = pic->mode->scaleFactor * 320;
	int x, y;
	int xl, xr;
	int ytotal;
	int bitmask;
	byte *bounds = NULL;
	int legalcolor, legalmask;
#ifdef DRAW_SCALED
	int min_x, min_y, max_x, max_y;
#endif
	int original_drawenable = drawenable; // Backup, since we need the unmodified value
					      // for filling the aux and control map

	// Restrict drawenable not to restrict itself to zero
	if (pic->control_map->index_data[y_200 * 320 + x_320] != 0)
		drawenable &= ~GFX_MASK_CONTROL;

	if (color == 0xff)
		drawenable &= ~GFX_MASK_VISUAL;

	if (priority == 0) {
		drawenable &= ~GFX_MASK_PRIORITY;
		original_drawenable &= ~GFX_MASK_PRIORITY;
	}

	AUXBUF_FILL(pic, x_320, y_200, original_drawenable, (drawenable & GFX_MASK_CONTROL) ? control : 0,
	            sci_titlebar_size);

#ifdef DRAW_SCALED
	_gfxr_auxbuf_spread(pic, &min_x, &min_y, &max_x, &max_y);

	if (_gfxr_find_fill_point(pic, min_x, min_y, max_x, max_y, x_320, y_200, color, drawenable, &x, &y)) {
		//GFXWARN("Could not find scaled fill point, but unscaled fill point was available!\n");
		drawenable &= GFX_MASK_PRIORITY;
		if (!drawenable)
			_gfxr_auxbuf_propagate_changes(pic, 0);
	}
#else
	x = x_320;
	y = y_200;
#endif

	ytotal = y * linewidth;

	if (!drawenable)
		return;

	if (drawenable & GFX_MASK_VISUAL) {
		bounds = pic->visual_map->index_data;
#if 0
		// Code disabled, as removing it fixes qg1 pic.095 (unscaled). However,
		// it MAY be of relevance to scaled pic drawing...

		if ((color & 0xf) == 0xf // When dithering with white, do more
					 // conservative checks
		        || (color & 0xf0) == 0xf0)
			legalcolor = 0xff;
		else
			legalcolor = 0xf0; // Only check the second color
#endif
#ifdef DRAW_SCALED
		legalcolor = 0xff;
		legalmask = legalcolor;
#else
		legalmask = 0x0ff0;
		legalcolor = 0xff;
#endif
	} else if (drawenable & GFX_MASK_PRIORITY) {
		bounds = pic->priority_map->index_data;
		legalcolor = 0;
		legalmask = 0x0f0f;
	} else {
		legalcolor = 0;
		legalmask = 0x0f0f;
	}

	if (!bounds || IS_BOUNDARY(x, y, bounds[ytotal + x]))
		return;

	if (bounds) {
#ifdef DRAW_SCALED
		int proj_y = y_200;
		int proj_ytotal = proj_y * 320;
		int proj_x = x_320;
		int proj_xl_bound;
		int proj_xr_bound;
		int proj_xl, proj_xr;

		ytotal = y * linewidth;

		proj_xl_bound = proj_x;
		if (SCALED_CHECK(pic->aux_map[proj_ytotal + proj_xl_bound] & FRESH_PAINT)) {
			while (proj_xl_bound && SCALED_CHECK(pic->aux_map[proj_ytotal + proj_xl_bound - 1] & FRESH_PAINT))
				--proj_xl_bound;
		} else
			while (proj_xl_bound < 319 && SCALED_CHECK(!(pic->aux_map[proj_ytotal + proj_xl_bound + 1] & FRESH_PAINT)))
				++proj_xl_bound;

		proj_xr_bound = (proj_xl_bound > proj_x) ? proj_xl_bound : proj_x;
		while ((proj_xr_bound < 319) && SCALED_CHECK(pic->aux_map[proj_ytotal + proj_xr_bound + 1] & FRESH_PAINT))
			++proj_xr_bound;

		proj_xl = proj_xl_bound;
		proj_xr = proj_xr_bound;

		proj_xl_bound *= pic->mode->xfact;
		if (proj_xl_bound)
			proj_xl_bound -= pic->mode->xfact - 1;

		if (proj_xr_bound < 319)
			++proj_xr_bound;
		proj_xr_bound *= pic->mode->xfact;
		proj_xr_bound += pic->mode->xfact - 1;
#endif
		xl = x;
		while (xl > proj_xl_bound && (!IS_BOUNDARY(xl - 1, y, bounds[ytotal + xl -1])))
			--xl;

		while (x < proj_xr_bound && (!IS_BOUNDARY(x + 1, y, bounds[ytotal + x + 1])))
			++x;
		xr = x;

		if (drawenable & GFX_MASK_VISUAL)
			memset(pic->visual_map->index_data + ytotal + xl, color, xr - xl + 1);

		if (drawenable & GFX_MASK_PRIORITY)
			memset(pic->priority_map->index_data + ytotal + xl, priority, xr - xl + 1);

		FILL_FUNCTION_RECURSIVE(pic, xl, xr, y, -1, bounds, legalcolor, legalmask, color, priority, drawenable,
		                        sci_titlebar_size);
		FILL_FUNCTION_RECURSIVE(pic, xl, xr, y, + 1, bounds, legalcolor, legalmask, color, priority, drawenable,
		                        sci_titlebar_size);
	}

	// Now finish the aux buffer
	bitmask = drawenable & (((color != 0xff) ? 1 : 0) | ((priority) ? 2 : 0) | ((control) ? 4 : 0));

#ifdef DRAW_SCALED
#  ifdef FILL_RECURSIVE_DEBUG
	if (fillmagc)
#  endif
		_gfxr_auxbuf_propagate_changes(pic, bitmask);
#endif
}

#undef SCALED_CHECK
#undef IS_BOUNDARY

#ifndef DRAW_SCALED
#  undef proj_xl_bound
#  undef proj_xr_bound
#endif

} // End of namespace Sci
