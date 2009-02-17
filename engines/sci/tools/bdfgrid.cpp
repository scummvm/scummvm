/*
 * Copyright 2001 Computing Research Labs, New Mexico State University
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COMPUTING RESEARCH LAB OR NEW MEXICO STATE UNIVERSITY BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
 * OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef lint
#ifdef __GNUC__
static char rcsid[] __attribute__((unused)) = "$Id$";
#else
static char rcsid[] = "$Id$";
#endif
#endif

#include "bdfP.h"

#ifndef MYABS
#define MYABS(n) ((n) < 0 ? -(n) : (n))
#endif

#undef MAX
#define MAX(h, i) ((h) > (i) ? (h) : (i))

#undef MIN
#define MIN(l, o) ((l) < (o) ? (l) : (o))

double _bdf_cos_tbl[360] = {
	0.000000, 0.999848, 0.999391, 0.998630, 0.997564, 0.996195,
	0.994522, 0.992546, 0.990268, 0.987688, 0.984808, 0.981627,
	0.978148, 0.974370, 0.970296, 0.965926, 0.961262, 0.956305,
	0.951057, 0.945519, 0.939693, 0.933580, 0.927184, 0.920505,
	0.913545, 0.906308, 0.898794, 0.891007, 0.882948, 0.874620,
	0.866025, 0.857167, 0.848048, 0.838671, 0.829038, 0.819152,
	0.809017, 0.798636, 0.788011, 0.777146, 0.766044, 0.754710,
	0.743145, 0.731354, 0.719340, 0.707107, 0.694658, 0.681998,
	0.669131, 0.656059, 0.642788, 0.629320, 0.615661, 0.601815,
	0.587785, 0.573576, 0.559193, 0.544639, 0.529919, 0.515038,
	0.500000, 0.484810, 0.469472, 0.453990, 0.438371, 0.422618,
	0.406737, 0.390731, 0.374607, 0.358368, 0.342020, 0.325568,
	0.309017, 0.292372, 0.275637, 0.258819, 0.241922, 0.224951,
	0.207912, 0.190809, 0.173648, 0.156434, 0.139173, 0.121869,
	0.104528, 0.087156, 0.069756, 0.052336, 0.034899, 0.017452,
	0.000000, -0.017452, -0.034899, -0.052336, -0.069756, -0.087156,
	-0.104528, -0.121869, -0.139173, -0.156434, -0.173648, -0.190809,
	-0.207912, -0.224951, -0.241922, -0.258819, -0.275637, -0.292372,
	-0.309017, -0.325568, -0.342020, -0.358368, -0.374607, -0.390731,
	-0.406737, -0.422618, -0.438371, -0.453990, -0.469472, -0.484810,
	-0.500000, -0.515038, -0.529919, -0.544639, -0.559193, -0.573576,
	-0.587785, -0.601815, -0.615661, -0.629320, -0.642788, -0.656059,
	-0.669131, -0.681998, -0.694658, -0.707107, -0.719340, -0.731354,
	-0.743145, -0.754710, -0.766044, -0.777146, -0.788011, -0.798636,
	-0.809017, -0.819152, -0.829038, -0.838671, -0.848048, -0.857167,
	-0.866025, -0.874620, -0.882948, -0.891007, -0.898794, -0.906308,
	-0.913545, -0.920505, -0.927184, -0.933580, -0.939693, -0.945519,
	-0.951057, -0.956305, -0.961262, -0.965926, -0.970296, -0.974370,
	-0.978148, -0.981627, -0.984808, -0.987688, -0.990268, -0.992546,
	-0.994522, -0.996195, -0.997564, -0.998630, -0.999391, -0.999848,
	-1.000000, -0.999848, -0.999391, -0.998630, -0.997564, -0.996195,
	-0.994522, -0.992546, -0.990268, -0.987688, -0.984808, -0.981627,
	-0.978148, -0.974370, -0.970296, -0.965926, -0.961262, -0.956305,
	-0.951057, -0.945519, -0.939693, -0.933580, -0.927184, -0.920505,
	-0.913545, -0.906308, -0.898794, -0.891007, -0.882948, -0.874620,
	-0.866025, -0.857167, -0.848048, -0.838671, -0.829038, -0.819152,
	-0.809017, -0.798636, -0.788011, -0.777146, -0.766044, -0.754710,
	-0.743145, -0.731354, -0.719340, -0.707107, -0.694658, -0.681998,
	-0.669131, -0.656059, -0.642788, -0.629320, -0.615661, -0.601815,
	-0.587785, -0.573576, -0.559193, -0.544639, -0.529919, -0.515038,
	-0.500000, -0.484810, -0.469472, -0.453990, -0.438371, -0.422618,
	-0.406737, -0.390731, -0.374607, -0.358368, -0.342020, -0.325568,
	-0.309017, -0.292372, -0.275637, -0.258819, -0.241922, -0.224951,
	-0.207912, -0.190809, -0.173648, -0.156434, -0.139173, -0.121869,
	-0.104528, -0.087156, -0.069756, -0.052336, -0.034899, -0.017452,
	-0.000000, 0.017452, 0.034899, 0.052336, 0.069756, 0.087156,
	0.104528, 0.121869, 0.139173, 0.156434, 0.173648, 0.190809,
	0.207912, 0.224951, 0.241922, 0.258819, 0.275637, 0.292372,
	0.309017, 0.325568, 0.342020, 0.358368, 0.374607, 0.390731,
	0.406737, 0.422618, 0.438371, 0.453990, 0.469472, 0.484810,
	0.500000, 0.515038, 0.529919, 0.544639, 0.559193, 0.573576,
	0.587785, 0.601815, 0.615661, 0.629320, 0.642788, 0.656059,
	0.669131, 0.681998, 0.694658, 0.707107, 0.719340, 0.731354,
	0.743145, 0.754710, 0.766044, 0.777146, 0.788011, 0.798636,
	0.809017, 0.819152, 0.829038, 0.838671, 0.848048, 0.857167,
	0.866025, 0.874620, 0.882948, 0.891007, 0.898794, 0.906308,
	0.913545, 0.920505, 0.927184, 0.933580, 0.939693, 0.945519,
	0.951057, 0.956305, 0.961262, 0.965926, 0.970296, 0.974370,
	0.978148, 0.981627, 0.984808, 0.987688, 0.990268, 0.992546,
	0.994522, 0.996195, 0.997564, 0.998630, 0.999391, 0.999848,
};

double _bdf_sin_tbl[360] = {
	0.000000, 0.017452, 0.034899, 0.052336, 0.069756, 0.087156,
	0.104528, 0.121869, 0.139173, 0.156434, 0.173648, 0.190809,
	0.207912, 0.224951, 0.241922, 0.258819, 0.275637, 0.292372,
	0.309017, 0.325568, 0.342020, 0.358368, 0.374607, 0.390731,
	0.406737, 0.422618, 0.438371, 0.453990, 0.469472, 0.484810,
	0.500000, 0.515038, 0.529919, 0.544639, 0.559193, 0.573576,
	0.587785, 0.601815, 0.615661, 0.629320, 0.642788, 0.656059,
	0.669131, 0.681998, 0.694658, 0.707107, 0.719340, 0.731354,
	0.743145, 0.754710, 0.766044, 0.777146, 0.788011, 0.798636,
	0.809017, 0.819152, 0.829038, 0.838671, 0.848048, 0.857167,
	0.866025, 0.874620, 0.882948, 0.891007, 0.898794, 0.906308,
	0.913545, 0.920505, 0.927184, 0.933580, 0.939693, 0.945519,
	0.951057, 0.956305, 0.961262, 0.965926, 0.970296, 0.974370,
	0.978148, 0.981627, 0.984808, 0.987688, 0.990268, 0.992546,
	0.994522, 0.996195, 0.997564, 0.998630, 0.999391, 0.999848,
	1.000000, 0.999848, 0.999391, 0.998630, 0.997564, 0.996195,
	0.994522, 0.992546, 0.990268, 0.987688, 0.984808, 0.981627,
	0.978148, 0.974370, 0.970296, 0.965926, 0.961262, 0.956305,
	0.951057, 0.945519, 0.939693, 0.933580, 0.927184, 0.920505,
	0.913545, 0.906308, 0.898794, 0.891007, 0.882948, 0.874620,
	0.866025, 0.857167, 0.848048, 0.838671, 0.829038, 0.819152,
	0.809017, 0.798636, 0.788011, 0.777146, 0.766044, 0.754710,
	0.743145, 0.731354, 0.719340, 0.707107, 0.694658, 0.681998,
	0.669131, 0.656059, 0.642788, 0.629320, 0.615661, 0.601815,
	0.587785, 0.573576, 0.559193, 0.544639, 0.529919, 0.515038,
	0.500000, 0.484810, 0.469472, 0.453990, 0.438371, 0.422618,
	0.406737, 0.390731, 0.374607, 0.358368, 0.342020, 0.325568,
	0.309017, 0.292372, 0.275637, 0.258819, 0.241922, 0.224951,
	0.207912, 0.190809, 0.173648, 0.156434, 0.139173, 0.121869,
	0.104528, 0.087156, 0.069756, 0.052336, 0.034899, 0.017452,
	0.000000, -0.017452, -0.034899, -0.052336, -0.069756, -0.087156,
	-0.104528, -0.121869, -0.139173, -0.156434, -0.173648, -0.190809,
	-0.207912, -0.224951, -0.241922, -0.258819, -0.275637, -0.292372,
	-0.309017, -0.325568, -0.342020, -0.358368, -0.374607, -0.390731,
	-0.406737, -0.422618, -0.438371, -0.453990, -0.469472, -0.484810,
	-0.500000, -0.515038, -0.529919, -0.544639, -0.559193, -0.573576,
	-0.587785, -0.601815, -0.615661, -0.629320, -0.642788, -0.656059,
	-0.669131, -0.681998, -0.694658, -0.707107, -0.719340, -0.731354,
	-0.743145, -0.754710, -0.766044, -0.777146, -0.788011, -0.798636,
	-0.809017, -0.819152, -0.829038, -0.838671, -0.848048, -0.857167,
	-0.866025, -0.874620, -0.882948, -0.891007, -0.898794, -0.906308,
	-0.913545, -0.920505, -0.927184, -0.933580, -0.939693, -0.945519,
	-0.951057, -0.956305, -0.961262, -0.965926, -0.970296, -0.974370,
	-0.978148, -0.981627, -0.984808, -0.987688, -0.990268, -0.992546,
	-0.994522, -0.996195, -0.997564, -0.998630, -0.999391, -0.999848,
	-1.000000, -0.999848, -0.999391, -0.998630, -0.997564, -0.996195,
	-0.994522, -0.992546, -0.990268, -0.987688, -0.984808, -0.981627,
	-0.978148, -0.974370, -0.970296, -0.965926, -0.961262, -0.956305,
	-0.951057, -0.945519, -0.939693, -0.933580, -0.927184, -0.920505,
	-0.913545, -0.906308, -0.898794, -0.891007, -0.882948, -0.874620,
	-0.866025, -0.857167, -0.848048, -0.838671, -0.829038, -0.819152,
	-0.809017, -0.798636, -0.788011, -0.777146, -0.766044, -0.754710,
	-0.743145, -0.731354, -0.719340, -0.707107, -0.694658, -0.681998,
	-0.669131, -0.656059, -0.642788, -0.629320, -0.615661, -0.601815,
	-0.587785, -0.573576, -0.559193, -0.544639, -0.529919, -0.515038,
	-0.500000, -0.484810, -0.469472, -0.453990, -0.438371, -0.422618,
	-0.406737, -0.390731, -0.374607, -0.358368, -0.342020, -0.325568,
	-0.309017, -0.292372, -0.275637, -0.258819, -0.241922, -0.224951,
	-0.207912, -0.190809, -0.173648, -0.156434, -0.139173, -0.121869,
	-0.104528, -0.087156, -0.069756, -0.052336, -0.034899, -0.017452,
};

double _bdf_tan_tbl[90] = {
	0.000000, 0.017455, 0.034921, 0.052408, 0.069927, 0.087489,
	0.105104, 0.122785, 0.140541, 0.158384, 0.176327, 0.194380,
	0.212557, 0.230868, 0.249328, 0.267949, 0.286745, 0.305731,
	0.324920, 0.344328, 0.363970, 0.383864, 0.404026, 0.424475,
	0.445229, 0.466308, 0.487733, 0.509525, 0.531709, 0.554309,
	0.577350, 0.600861, 0.624869, 0.649408, 0.674509, 0.700208,
	0.726543, 0.753554, 0.781286, 0.809784, 0.839100, 0.869287,
	0.900404, 0.932515, 0.965689, 1.000000, 1.035530, 1.072369,
	1.110613, 1.150368, 1.191754, 1.234897, 1.279942, 1.327045,
	1.376382, 1.428148, 1.482561, 1.539865, 1.600335, 1.664279,
	1.732051, 1.804048, 1.880726, 1.962611, 2.050304, 2.144507,
	2.246037, 2.355852, 2.475087, 2.605089, 2.747477, 2.904211,
	3.077684, 3.270853, 3.487414, 3.732051, 4.010781, 4.331476,
	4.704630, 5.144554, 5.671282, 6.313752, 7.115370, 8.144346,
	9.514364, 11.430052, 14.300666, 19.081137, 28.636253, 57.289962,
};

/*
 * Determine the actual ink bounds.
 */
static int
#ifdef __STDC__
_bdf_grid_ink_bounds(bdf_glyph_grid_t *grid, short *x, short *y,
                     short *width, short *height)
#else
_bdf_grid_ink_bounds(grid, x, y, width, height)
bdf_glyph_grid_t *grid;
short *x, *y, *width, *height;
#endif
{
	short bx, by, bwd, bht, minx, maxx, miny, maxy, dx, dy;
	unsigned short bpr, ink, sel, col;
	unsigned char *bmap, *masks;

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	if (grid->sel.width != 0 && grid->sel.height != 0) {
		sel = 1;
		bx = by = 0;
		bwd = grid->sel.width;
		bht = grid->sel.height;
		bmap = grid->sel.bitmap;
	} else {
		sel = 0;
		bx = grid->glyph_x;
		by = grid->glyph_y;
		bwd = grid->glyph_bbx.width;
		bht = grid->glyph_bbx.height;
		bmap = grid->bitmap;
	}
	maxx = maxy = 0;
	minx = bx + bwd;
	miny = by + bht;

	bpr = ((bwd * grid->bpp) + 7) >> 3;
	ink = 0;

	bwd += bx;
	bht += by;
	for (dy = by; dy < bht; dy++) {
		for (col = bx * grid->bpp, dx = bx; dx < bwd; dx++, col += grid->bpp) {
			if (bmap[(dy * bpr) + (col >> 3)] & masks[(col & 7) / grid->bpp]) {
				ink = 1;
				minx = MIN(minx, dx);
				miny = MIN(miny, dy);
				maxx = MAX(maxx, dx);
				maxy = MAX(maxy, dy);
			}
		}
	}

	*x = minx + ((sel) ? grid->sel.x : 0);
	*y = miny + ((sel) ? grid->sel.y : 0);
	if (ink == 0)
		*width = *height = 0;
	else {
		*width = (maxx - minx) + 1;
		*height = (maxy - miny) + 1;
	}
	return ink;
}

/**************************************************************************
 *
 * Glyph grid create and destroy functions.
 *
 **************************************************************************/

/*
 * Make a glyph grid with the glyph bitmap set in the bitmap.
 */
bdf_glyph_grid_t *
#ifdef __STDC__
bdf_make_glyph_grid(bdf_font_t *font, long code, int unencoded)
#else
bdf_make_glyph_grid(font, code, unencoded)
bdf_font_t *font;
long code;
int unencoded;
#endif
{
	unsigned short si, di, col, colx, byte;
	short ht, as, ds, gsize, bpr, x, y, nx, ny;
	long l, r, m;
	bdf_glyph_grid_t *gr;
	bdf_glyph_t *gl, *glp;
	bdf_property_t *p;
	unsigned char *masks;
	char name[24];

	if (font == 0)
		return 0;

	/*
	 * Allocate the grid and initialize it.
	 */
	gr = (bdf_glyph_grid_t *) malloc(sizeof(bdf_glyph_grid_t));
	(void) memset((char *) gr, 0, sizeof(bdf_glyph_grid_t));

	/*
	 * Set the encoding and the unencoded flag.
	 */
	gr->bpp = font->bpp;
	gr->encoding = code;
	gr->unencoded = unencoded;

	/*
	 * Set the glyph grid spacing.
	 */
	gr->spacing = font->spacing;

	/*
	 * Set the point size and resolutions.
	 */
	gr->point_size = font->point_size;
	gr->resolution_x = font->resolution_x;
	gr->resolution_y = font->resolution_y;

	/*
	 * Set the CAP_HEIGHT and X_HEIGHT if they exist in the font.
	 */
	if ((p = bdf_get_font_property(font, "CAP_HEIGHT")) != 0)
		gr->cap_height = (short) p->value.int32;
	if ((p = bdf_get_font_property(font, "X_HEIGHT")) != 0)
		gr->x_height = (short) p->value.int32;

	masks = 0;
	switch (gr->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	/*
	 * Copy the font bounding box into the grid.
	 */
	(void) memcpy((char *) &gr->font_bbx, (char *) &font->bbx,
	              sizeof(bdf_bbx_t));

	if (unencoded) {
		gl = font->unencoded;
		r = font->unencoded_used;
	} else {
		gl = font->glyphs;
		r = font->glyphs_used;
	}

	/*
	 * Locate the specified glyph using a simple binary search.
	 */
	glp = 0;
	if (r > 0) {
		for (l = 0; r >= l;) {
			m = (l + r) >> 1;
			glp = gl + m;
			if (glp->encoding == code)
				break;
			if (glp->encoding > code)
				r = m - 1;
			else if (glp->encoding < code)
				l = m + 1;
			glp = 0;
		}
	}

	ht = gr->font_bbx.height;
	as = gr->font_bbx.ascent;
	ds = gr->font_bbx.descent;

	/*
	 * 1. Determine width and height needed from the largest of the
	 *    width or height.
	 */
	gr->grid_width = gr->grid_height =
	                     MAX(gr->font_bbx.width, gr->font_bbx.height);

	/*
	 * 2. Make sure the grid is at least a square of the largest of the width
	 *    or height of the glyph itself to allow room for transformations.
	 */
	if (glp != 0) {
		/*
		 * Set the glyph name and other metrics.
		 */
		if (glp->name) {
			gr->name = (char *) malloc(strlen(glp->name) + 1);
			(void) memcpy(gr->name, glp->name, strlen(glp->name) + 1);
		} else {
			sprintf(name, "char%ld", code);
			gr->name = (char *) malloc(strlen(name) + 1);
			(void) memcpy(gr->name, name, strlen(name) + 1);
		}
		gr->dwidth = glp->dwidth;

		/*
		 * Copy the glyph bounding box into the grid.
		 */
		(void) memcpy((char *) &gr->glyph_bbx, (char *) &glp->bbx,
		              sizeof(bdf_bbx_t));

		if (glp->bbx.height < glp->bbx.ascent + glp->bbx.descent)
			gsize = glp->bbx.ascent + glp->bbx.descent;
		else
			gsize = glp->bbx.height;

		/*
		 * Figure the maximum of the glyph width and height.
		 */
		gsize = MAX(gr->glyph_bbx.width, gsize);

		/*
		 * If either the grid width or grid height is less than the
		 * grid size just determined, then adjust them to the new grid size.
		 */
		gr->grid_width = MAX(gr->grid_width, gsize);
		gr->grid_height = MAX(gr->grid_height, gsize);
	} else {
		/*
		 * The glyph doesn't exist, so make up a name for it.
		 */
		if (unencoded)
			sprintf(name, "unencoded%ld", code);
		else
			sprintf(name, "char%ld", code);
		gr->name = (char *) malloc(strlen(name) + 1);
		(void) memcpy(gr->name, name, strlen(name) + 1);
	}

	/*
	 * If the font has character-cell or mono spacing, make sure the grid
	 * device width is set to the width stored in the font.
	 */
	if (gr->spacing != BDF_PROPORTIONAL)
		gr->dwidth = font->monowidth;

	/*
	 * Determine the vertical origin based on the font bounding box.
	 */
	if (ht >= as + ds)
		gr->base_y = (((gr->grid_height >> 1) - (ht >> 1)) + ht) - ds;
	else
		gr->base_y = ((gr->grid_height >> 1) - ((as + ds) >> 1)) + as;

	/*
	 * The final adjust is to check to see if the glyph positioned relative to
	 * the baseline would cause the grid to change size.  This sometimes
	 * happens in fonts that have incorrect metrics.
	 */
	if (gr->base_y + gr->glyph_bbx.descent > gr->grid_height) {
		gsize = gr->base_y + gr->glyph_bbx.descent;
		gr->grid_width = MAX(gsize, gr->grid_width);
		gr->grid_height = MAX(gsize, gr->grid_height);
	}

	/*
	 * Determine the horizontal origin based on the font bounding box and
	 * centered within the grid.
	 */
	gr->base_x = (gr->grid_width >> 1) - (gr->font_bbx.width >> 1);
	if (gr->font_bbx.x_offset < 0)
		gr->base_x += MYABS(gr->font_bbx.x_offset);

	/*
	 * Allocate double the storage needed for the grid bitmap.  The extra
	 * storage will be used for transformations.
	 */
	gr->bytes = ((((gr->grid_width * gr->bpp) + 7) >> 3) *
	             gr->grid_height) << 1;
	gr->bitmap = (unsigned char *) malloc(gr->bytes);
	(void) memset((char *) gr->bitmap, 0, gr->bytes);

	/*
	 * Initialize the top-left coordinates of the glyph to the baseline
	 * coordinates.
	 */
	gr->glyph_x = gr->base_x;
	gr->glyph_y = gr->base_y;

	/*
	 * If the glyph was not found, simply return the empty grid.
	 */
	if (glp == 0)
		return gr;

	/*
	 * Determine the top-left coordinates of the glyph with respect to the
	 * baseline coordinates.
	 */
	gr->glyph_x = nx = gr->base_x + gr->glyph_bbx.x_offset;
	gr->glyph_y = ny = gr->base_y - gr->glyph_bbx.ascent;

	/*
	 * Now copy the glyph bitmap to the appropriate location in the
	 * grid.
	 */
	bpr = ((gr->glyph_bbx.width * gr->bpp) + 7) >> 3;
	gsize = ((gr->grid_width * gr->bpp) + 7) >> 3;
	for (y = 0; y < gr->glyph_bbx.height; y++, ny++) {
		for (colx = nx * gr->bpp, col = x = 0; x < gr->glyph_bbx.width;
		        x++, col += gr->bpp, colx += gr->bpp) {
			si = (col & 7) / gr->bpp;
			byte = glp->bitmap[(y * bpr) + (col >> 3)] & masks[si];
			if (byte) {
				di = (colx & 7) / gr->bpp;
				if (di < si)
					byte <<= (si - di) * gr->bpp;
				else if (di > si)
					byte >>= (di - si) * gr->bpp;
				gr->bitmap[(ny * gsize) + (colx >> 3)] |= byte;
			}
		}
	}

	/*
	 * Always crop the glyph to the ink bounds before editing.
	 */
	bdf_grid_crop(gr, 0);

	/*
	 * Return the grid.
	 */
	return gr;
}

void
#ifdef __STDC__
bdf_free_glyph_grid(bdf_glyph_grid_t *grid)
#else
bdf_free_glyph_grid(grid)
bdf_glyph_grid_t *grid;
#endif
{
	if (grid == 0)
		return;

	if (grid->name != 0)
		free(grid->name);
	if (grid->bytes > 0)
		free((char *) grid->bitmap);
	if (grid->sel.bytes > 0)
		free((char *) grid->sel.bitmap);
	free((char *) grid);
}

/**************************************************************************
 *
 * Glyph grid resize functions.
 *
 **************************************************************************/

/*
 * Enlarge the grid without affecting the font or glyph metrics.
 */
int
#ifdef __STDC__
bdf_grid_enlarge(bdf_glyph_grid_t *grid, unsigned short width,
                 unsigned short height)
#else
bdf_grid_enlarge(grid, width, height)
bdf_glyph_grid_t *grid;
unsigned short width, height;
#endif
{
	unsigned short si, di, col, colx, byte;
	short ht, wd, as, ds, x, y, nx, ny;
	unsigned short gwd, ght, bytes, obpr, nbpr, gsize;
	unsigned char *bitmap, *masks;

	if (grid == 0 || (width < grid->grid_width && height < grid->grid_height))
		return 0;

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	ht = height;
	as = grid->font_bbx.ascent;
	ds = grid->font_bbx.descent;

	gwd = MAX(width, grid->grid_width);
	ght = MAX(height, grid->grid_height);
	gsize = MAX(gwd, ght);

	nbpr = ((gsize * grid->bpp) + 7) >> 3;
	bytes = (nbpr * ght) << 1;
	bitmap = (unsigned char *) malloc(bytes);
	(void) memset((char *) bitmap, 0, bytes);

	/*
	 * Determine the new baseline.
	 */
	if (ht >= as + ds)
		grid->base_y = (((ght >> 1) - (ht >> 1)) + ht) - ds;
	else
		grid->base_y = ((ght >> 1) - ((as + ds) >> 1)) + as;

	grid->base_x = (gwd >> 1) - (grid->font_bbx.width >> 1);
	if (grid->font_bbx.x_offset < 0)
		grid->base_x += MYABS(grid->font_bbx.x_offset);

	nx = grid->base_x + grid->glyph_bbx.x_offset;
	ny = grid->base_y - grid->glyph_bbx.ascent;

	/*
	 * Now copy the bitmap into the new storage base on the new metrics
	 * values.
	 */
	obpr = ((grid->grid_width * grid->bpp) + 7) >> 3;
	wd = grid->glyph_x + grid->glyph_bbx.width;
	ht = grid->glyph_y + grid->glyph_bbx.height;
	for (y = grid->glyph_y; y < ht; y++, ny++) {
		col = grid->glyph_x * grid->bpp;
		colx = nx * grid->bpp;
		for (x = grid->glyph_x; x < wd;
		        x++, col += grid->bpp, colx += grid->bpp) {
			si = (col & 7) / grid->bpp;
			byte = grid->bitmap[(y * obpr) + (col >> 3)] & masks[si];
			if (byte) {
				di = (colx & 7) / grid->bpp;
				if (di < si)
					byte <<= (si - di) * grid->bpp;
				else if (di > si)
					byte >>= (di - si) * grid->bpp;
				bitmap[(ny * nbpr) + (colx >> 3)] |= byte;
			}
		}
	}

	/*
	 * Adjust the glyph coordinates.
	 */
	grid->glyph_x = grid->base_x + grid->glyph_bbx.x_offset;
	grid->glyph_y = grid->base_y - grid->glyph_bbx.ascent;

	/*
	 * Get rid of the old grid bitmap and replace it with the new one.
	 */
	free((char *) grid->bitmap);
	grid->bytes = bytes;
	grid->bitmap = bitmap;

	/*
	 * Update the new grid width and height.
	 */
	grid->grid_width = grid->grid_height = gsize;

	/*
	 * Always mark the grid as being modified on a resize.
	 */
	grid->modified = 1;

	return 1;
}

/*
 * Change the font bounding box values and resize the grid bitmap if
 * necessary.
 */
int
#ifdef __STDC__
bdf_grid_resize(bdf_glyph_grid_t *grid, bdf_metrics_t *metrics)
#else
bdf_grid_resize(grid, metrics)
bdf_glyph_grid_t *grid;
bdf_metrics_t *metrics;
#endif
{
	int changed;
	unsigned short si, di, col, colx, byte;
	short ht, wd, as, ds, x, y, nx, ny;
	unsigned short gwd, ght, bytes, obpr, nbpr, gsize;
	unsigned char *bitmap, *masks;

	changed = 0;

	if (grid == 0 || metrics == 0)
		return changed;

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	/*
	 * Create new grid bitmaps in preparation for the various metrics changing.
	 */
	if (metrics->width > grid->grid_width ||
	        metrics->height > grid->grid_height) {
		changed = 1;

		ht = metrics->height;
		as = metrics->ascent;
		ds = metrics->descent;

		gwd = MAX(metrics->width, grid->grid_width);
		ght = MAX(metrics->height, grid->grid_height);

		/*
		 * Get the larger of the two dimensions.
		 */
		gsize = MAX(gwd, ght);

		nbpr = ((gsize * grid->bpp) + 7) >> 3;
		bytes = (nbpr * gsize) << 1;
		bitmap = (unsigned char *) malloc(bytes);
		(void) memset((char *) bitmap, 0, bytes);

		/*
		 * Determine the new baseline.
		 */
		if (ht >= as + ds)
			grid->base_y = (((ght >> 1) - (ht >> 1)) + ht) - ds;
		else
			grid->base_y = ((ght >> 1) - ((as + ds) >> 1)) + as;

		grid->base_x = (gwd >> 1) - (metrics->width >> 1);
		if (metrics->x_offset < 0)
			grid->base_x += MYABS(metrics->x_offset);

		nx = grid->base_x + grid->glyph_bbx.x_offset;
		ny = grid->base_y - grid->glyph_bbx.ascent;

		/*
		 * Now copy the bitmap into the new storage base on the new metrics
		 * values.
		 */
		obpr = ((grid->grid_width * grid->bpp) + 7) >> 3;
		wd = grid->glyph_x + grid->glyph_bbx.width;
		ht = grid->glyph_y + grid->glyph_bbx.height;
		for (y = grid->glyph_y; y < ht; y++, ny++) {
			col = grid->glyph_x * grid->bpp;
			colx = nx * grid->bpp;
			for (x = grid->glyph_x; x < wd;
			        x++, col += grid->bpp, colx += grid->bpp) {
				si = (col & 7) / grid->bpp;
				byte = grid->bitmap[(y * obpr) + (col >> 3)] & masks[si];
				if (byte) {
					di = (colx & 7) / grid->bpp;
					if (di < si)
						byte <<= (si - di) * grid->bpp;
					else if (di > si)
						byte >>= (di - si) * grid->bpp;
					bitmap[(ny * nbpr) + (colx >> 3)] |= byte;
				}
			}
		}

		/*
		 * Adjust the glyph coordinates.
		 */
		grid->glyph_x = grid->base_x + grid->glyph_bbx.x_offset;
		grid->glyph_y = grid->base_y - grid->glyph_bbx.ascent;

		/*
		 * Get rid of the old grid bitmap and replace it with the new one.
		 */
		free((char *) grid->bitmap);
		grid->bytes = bytes;
		grid->bitmap = bitmap;

		/*
		 * Update the new grid width and height.
		 */
		grid->grid_width = grid->grid_height = gsize;

		/*
		 * Copy the metrics info into the font bounding box.
		 */
		grid->font_bbx.width = metrics->width;
		grid->font_bbx.x_offset = metrics->x_offset;
		grid->font_bbx.height = metrics->height;
		grid->font_bbx.ascent = metrics->ascent;
		grid->font_bbx.descent = metrics->descent;
		grid->font_bbx.y_offset = metrics->y_offset;
	} else {
		/*
		 * The grid does not need to resized, but the baseline must
		 * be recalculated and the bitmap copied again.
		 */
		bytes = grid->bytes >> 1;
		bitmap = grid->bitmap + bytes;
		(void) memset((char *) bitmap, 0, bytes);

		ht = metrics->height;
		as = metrics->ascent;
		ds = metrics->descent;

		gwd = grid->grid_width;
		ght = grid->grid_height;

		/*
		 * Determine the new baseline.
		 */
		if (ht >= as + ds)
			grid->base_y = (((ght >> 1) - (ht >> 1)) + ht) - ds;
		else
			grid->base_y = ((ght >> 1) - ((as + ds) >> 1)) + as;

		grid->base_x = (gwd >> 1) - (metrics->width >> 1);
		if (metrics->x_offset < 0)
			grid->base_x += MYABS(metrics->x_offset);

		nx = grid->base_x + grid->glyph_bbx.x_offset;
		ny = grid->base_y - grid->glyph_bbx.ascent;

		wd = grid->glyph_x + grid->glyph_bbx.width;
		ht = grid->glyph_y + grid->glyph_bbx.height;

		obpr = nbpr = ((grid->grid_width * grid->bpp) + 7) >> 3;
		for (y = grid->glyph_y; y < ht; y++, ny++) {
			col = grid->glyph_x * grid->bpp;
			colx = nx * grid->bpp;
			for (x = grid->glyph_x; x < wd;
			        x++, col += grid->bpp, colx += grid->bpp) {
				si = (col & 7) / grid->bpp;
				byte = grid->bitmap[(y * obpr) + (col >> 3)] & masks[si];
				if (byte) {
					di = (colx & 7) / grid->bpp;
					if (di < si)
						byte <<= (si - di) * grid->bpp;
					else if (di > si)
						byte >>= (di - si) * grid->bpp;
					bitmap[(ny * nbpr) + (colx >> 3)] |= byte;
				}
			}
		}

		/*
		 * Copy the adjusted bitmap back into the main area.
		 */
		(void) memcpy((char *) grid->bitmap, (char *) bitmap, bytes);

		/*
		 * Adjust the glyph coordinates.
		 */
		grid->glyph_x = grid->base_x + grid->glyph_bbx.x_offset;
		grid->glyph_y = grid->base_y - grid->glyph_bbx.ascent;

		/*
		 * Copy the metrics info into the font bounding box.
		 */
		grid->font_bbx.width = metrics->width;
		grid->font_bbx.x_offset = metrics->x_offset;
		grid->font_bbx.height = metrics->height;
		grid->font_bbx.ascent = metrics->ascent;
		grid->font_bbx.descent = metrics->descent;
		grid->font_bbx.y_offset = metrics->y_offset;
	}

	/*
	 * If the font is not proportional, make sure the device width is adjusted
	 * to meet the new font bounding box.
	 */
	if (changed && grid->spacing != BDF_PROPORTIONAL)
		grid->dwidth = grid->font_bbx.width;

	/*
	 * Always mark the grid as being modified on a resize.
	 */
	grid->modified = 1;

	return changed;
}

int
#ifdef __STDC__
bdf_grid_crop(bdf_glyph_grid_t *grid, int grid_modified)
#else
bdf_grid_crop(grid, grid_modified)
bdf_glyph_grid_t *grid;
int grid_modified;
#endif
{
	int cropped;
	short x, y, delta, maxx, minx, maxy, miny, col;
	unsigned short bpr;
	unsigned char *masks;

	cropped = 0;
	if (grid == 0)
		return cropped;

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;

	maxx = maxy = -1;
	minx = miny = grid->grid_width;
	for (y = 0; y < grid->grid_height; y++) {
		for (col = x = 0; x < grid->grid_width; x++, col += grid->bpp) {
			if (grid->bitmap[(y * bpr) + (col >> 3)] &
			        masks[(col & 7) / grid->bpp]) {
				minx = MIN(minx, x);
				maxx = MAX(maxx, x);
				miny = MIN(miny, y);
				maxy = MAX(maxy, y);
			}
		}
	}

	/*
	 * Handle an empty bitmap as a special case.
	 */
	if (maxx == -1) {
		/*
		 * If the glyph bounding box indicated something was there originally,
		 * then indicate that it was cropped.
		 */
		if (grid->glyph_bbx.width != 0 || grid->glyph_bbx.height != 0)
			cropped = 1;
		(void) memset((char *) &grid->glyph_bbx, 0, sizeof(bdf_bbx_t));
		grid->glyph_x = grid->base_x;
		grid->glyph_y = grid->base_y;
		if (cropped)
			grid->modified = 1;
		return cropped;
	}

	/*
	 * Increment the max points so width and height calculations won't go
	 * wrong.
	 */
	maxx++;
	maxy++;

	if (minx != grid->glyph_x) {
		cropped = 1;
		delta = minx - grid->glyph_x;
		grid->glyph_x += delta;
		grid->glyph_bbx.x_offset += delta;
	}
	if (maxx - minx != grid->glyph_bbx.width) {
		cropped = 1;
		delta = (maxx - minx) - grid->glyph_bbx.width;
		grid->glyph_bbx.width += delta;
		if (grid->spacing == BDF_PROPORTIONAL)
			grid->dwidth += delta;
	}

	if (miny != grid->glyph_y) {
		cropped = 1;
		delta = miny - grid->glyph_y;
		grid->glyph_y += delta;
		grid->glyph_bbx.y_offset =
		    grid->base_y - (grid->glyph_y + (maxy - miny));
	}
	if (maxy - miny != grid->glyph_bbx.height) {
		cropped = 1;
		delta = (maxy - miny) - grid->glyph_bbx.height;
		grid->glyph_bbx.height += delta;
		grid->glyph_bbx.y_offset =
		    grid->base_y - (grid->glyph_y + (maxy - miny));
		grid->glyph_bbx.ascent =
		    grid->glyph_bbx.height + grid->glyph_bbx.y_offset;
		grid->glyph_bbx.descent = -grid->glyph_bbx.y_offset;
	}

	/*
	 * Indicate that the grid was modified if the glyph had to be cropped.
	 */
	if (cropped && grid_modified)
		grid->modified = 1;

	return cropped;
}

/**************************************************************************
 *
 * Glyph grid pixel functions.
 *
 **************************************************************************/

int
#ifdef __STDC__
bdf_grid_set_pixel(bdf_glyph_grid_t *grid, short x, short y, int val)
#else
bdf_grid_set_pixel(grid, x, y, val)
bdf_glyph_grid_t *grid;
short x, y;
int val;
#endif
{
	unsigned short si, di, dx;
	int set, bpr, delta;
	unsigned char *masks;

	set = 0;

	if (grid == 0 || x < 0 || x >= grid->grid_width ||
	        y < 0 || y >= grid->grid_height)
		return set;

	si = 0;
	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		si = 7;
		break;
	case 2:
		masks = twobpp;
		si = 3;
		break;
	case 4:
		masks = fourbpp;
		si = 1;
		break;
	}

	/*
	 * Remove any unused bits from the value.
	 */
	val &= masks[si];

	dx = x * grid->bpp;
	di = (dx & 7) / grid->bpp;

	/*
	 * Shift up the value to the appropriate place if necessary.
	 */
	if (di < si)
		val <<= (si - di) * grid->bpp;

	/*
	 * Determine the bytes-per-row.
	 */
	bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;

	/*
	 * If the bit is already set, simply return with an indication that
	 * nothing changed.
	 */
	if ((grid->bitmap[(y * bpr) + (dx >> 3)] & masks[di]) == val)
		return set;

	/*
	 * Set the bit.
	 */
	set = 1;

	/*
	 * Clear the bits that will take the new value.
	 */
	grid->bitmap[(y * bpr) + (dx >> 3)] &= ~masks[di];
	grid->bitmap[(y * bpr) + (dx >> 3)] |= val;

	/*
	 * Adjust the glyph bounding box.
	 */
	if (x < grid->glyph_x) {
		delta = grid->glyph_x - x;
		grid->glyph_bbx.width += delta;
		grid->glyph_bbx.x_offset -= delta;
		if (grid->spacing == BDF_PROPORTIONAL)
			grid->dwidth = grid->glyph_bbx.width + grid->glyph_bbx.x_offset;
		grid->glyph_x -= delta;
	} else if (x >= grid->glyph_x + grid->glyph_bbx.width) {
		delta = x - (grid->glyph_x + grid->glyph_bbx.width) + 1;
		grid->glyph_bbx.width += delta;
		if (grid->spacing == BDF_PROPORTIONAL)
			grid->dwidth = grid->glyph_bbx.width + grid->glyph_bbx.x_offset;
	}
	if (y < grid->glyph_y) {
		delta = grid->glyph_y - y;
		grid->glyph_bbx.ascent += delta;
		grid->glyph_bbx.height += delta;
		grid->glyph_y -= delta;
	} else if (y >= grid->glyph_y + grid->glyph_bbx.height) {
		delta = y - (grid->glyph_y + grid->glyph_bbx.height) + 1;
		grid->glyph_bbx.descent += delta;
		grid->glyph_bbx.height += delta;
		grid->glyph_bbx.y_offset = -grid->glyph_bbx.descent;
	}

	/*
	 * Indicate that the glyph was modified.
	 */
	grid->modified = 1;

	return set;
}

int
#ifdef __STDC__
bdf_grid_clear_pixel(bdf_glyph_grid_t *grid, short x, short y)
#else
bdf_grid_clear_pixel(grid, x, y)
bdf_glyph_grid_t *grid;
short x, y;
#endif
{
	int cleared, bpr;
	short delta, maxx, minx, maxy, miny, wd, ht;
	unsigned short di, dx;
	unsigned char *masks;

	cleared = 0;

	if (grid == 0 || x < 0 || x >= grid->grid_width ||
	        y < 0 || y >= grid->grid_height)
		return cleared;

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	/*
	 * Determine the bytes-per-row.
	 */
	bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;

	dx = x * grid->bpp;
	di = (dx & 7) / grid->bpp;

	/*
	 * If the bit is already clear, simply return with an indication that
	 * nothing changed.
	 */
	if (!(grid->bitmap[(y * bpr) + (dx >> 3)] & masks[di]))
		return cleared;

	/*
	 * Clear the bit.
	 */
	cleared = 1;
	grid->bitmap[(y * bpr) + (dx >> 3)] &= ~masks[di];

	/*
	 * Determine the new min and max values.
	 */
	maxx = maxy = 0;
	minx = miny = 32767;

	wd = grid->glyph_x + grid->glyph_bbx.width;
	ht = grid->glyph_y + grid->glyph_bbx.height;

	for (y = grid->glyph_y; y < ht; y++) {
		dx = grid->glyph_x * grid->bpp;
		for (x = grid->glyph_x; x < wd; x++, dx += grid->bpp) {
			di = (dx & 7) / grid->bpp;
			if (grid->bitmap[(y * bpr) + (dx >> 3)] & masks[di]) {
				minx = MIN(minx, x);
				maxx = MAX(maxx, x);
				miny = MIN(miny, y);
				maxy = MAX(maxy, y);
			}
		}
	}

	/*
	 * If this call clears the last bit in the image, set the glyph origin
	 * to the base and return.
	 */
	if (maxx == 0) {
		grid->glyph_x = grid->base_x;
		grid->glyph_y = grid->base_y;
		if (grid->spacing == BDF_PROPORTIONAL)
			grid->dwidth = 0;
		(void) memset((char *) &grid->glyph_bbx, 0, sizeof(grid->glyph_bbx));
		grid->modified = 1;
		return cleared;
	}

	/*
	 * Figure out the left and right bearing changes.
	 */
	if (minx > grid->glyph_x) {
		delta = minx - grid->glyph_x;
		grid->glyph_bbx.width -= delta;
		grid->glyph_bbx.x_offset += delta;
		if (grid->spacing == BDF_PROPORTIONAL)
			grid->dwidth = grid->glyph_bbx.width + grid->glyph_bbx.x_offset;
		grid->glyph_x += delta;
	} else if (maxx < wd - 1) {
		delta = (wd - 1) - maxx;
		grid->glyph_bbx.width -= delta;
		if (grid->spacing == BDF_PROPORTIONAL)
			grid->dwidth = grid->glyph_bbx.width + grid->glyph_bbx.x_offset;
	}

	if (miny > grid->glyph_y) {
		delta = miny - grid->glyph_y;
		grid->glyph_bbx.ascent -= delta;
		grid->glyph_bbx.height -= delta;
		grid->glyph_y += delta;
	} else if (maxy < ht - 1) {
		delta = (ht - 1) - maxy;
		grid->glyph_bbx.descent -= delta;
		grid->glyph_bbx.height -= delta;
		grid->glyph_bbx.y_offset = -grid->glyph_bbx.descent;
	}

	/*
	 * Indicate that the glyph was modified.
	 */
	grid->modified = 1;

	return cleared;
}

int
#ifdef __STDC__
bdf_grid_invert_pixel(bdf_glyph_grid_t *grid, short x, short y, int val)
#else
bdf_grid_invert_pixel(grid, x, y, val)
bdf_glyph_grid_t *grid;
short x, y;
int val;
#endif
{
	short bpr, di;
	unsigned char *masks;

	if (grid == 0 || x < 0 || x >= grid->grid_width ||
	        y < 0 || y >= grid->grid_height)
		return 0;

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	/*
	 * Determine the bytes-per-row and mask index.
	 */
	bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;
	di = ((x * grid->bpp) & 7) / grid->bpp;

	/*
	 * If the bit is set, then clear it, otherwise, set it.
	 */
	if (grid->bitmap[(y * bpr) + ((x * grid->bpp) >> 3)] & masks[di])
		return bdf_grid_clear_pixel(grid, x, y);
	else
		return bdf_grid_set_pixel(grid, x, y, val);
}

/**************************************************************************
 *
 * Glyph grid bitmap transformation functions.
 *
 **************************************************************************/

short
#ifdef __STDC__
_bdf_ceiling(double v)
#else
_bdf_ceiling(v)
double v;
#endif
{
	short val, neg;

	val = neg = 0;
	if (v < 0) {
		neg = 1;
		while (v < -1.0) {
			val++;
			v += 1.0;
		}
	} else if (v > 0) {
		while (v > 1.0) {
			val++;
			v -= 1.0;
		}
		if (v > 0.0)
			val++;
	}
	return (!neg) ? val : -val;
}

static int
#ifdef __STDC__
_bdf_rotate_selection(bdf_glyph_grid_t *grid, int mul90, short degrees)
#else
_bdf_rotate_selection(grid, mul90, degrees)
bdf_glyph_grid_t *grid;
int mul90;
short degrees;
#endif
{
	int rotated, byte;
	short wd, ht, nx, ny, cx, cy, x, y, col;
	short ox, oy, shiftx, shifty, si, di;
	double dx, dy;
	unsigned short bytes, bpr;
	unsigned char *scratch, *masks;

	rotated = 0;

	/*
	 * Check to see if the number of rotations would have no affect by
	 * checking if the count is a multiple of 4 (mod 4 == 0).
	 */
	if (grid == 0 || degrees == 0)
		return rotated;

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	bytes = grid->sel.bytes >> 1;
	scratch = grid->sel.bitmap + bytes;
	(void) memset((char *) scratch, 0, bytes);

	cx = grid->sel.width >> 1;
	cy = grid->sel.height >> 1;

	wd = ht = MAX(grid->sel.width, grid->sel.height);
	cx = cy = wd >> 1;

	bpr = ((wd * grid->bpp) + 7) >> 3;

	for (shiftx = shifty = y = 0; y < ht; y++) {
		for (col = x = 0; x < wd; x++, col += grid->bpp) {
			dx = (double)(x - cx);
			dy = (double)(y - cy);
			if (mul90) {
				nx = cx + (short)((dx * _bdf_cos_tbl[degrees]) -
				                  (dy * _bdf_sin_tbl[degrees]));
				ny = cy + (short)((dx * _bdf_sin_tbl[degrees]) +
				                  (dy * _bdf_cos_tbl[degrees]));
			} else {
				nx = cx + _bdf_ceiling((dx * _bdf_cos_tbl[degrees]) -
				                       (dy * _bdf_sin_tbl[degrees]));
				ny = cy + _bdf_ceiling((dx * _bdf_sin_tbl[degrees]) +
				                       (dy * _bdf_cos_tbl[degrees]));
			}

			/*
			 * Wrap the coordinates around the edges if necessary.
			 */
			if (nx < 0) {
				shiftx = MIN(shiftx, nx);
				nx += wd;
			} else if (nx >= wd) {
				ox = (nx - wd) + 1;
				shiftx = MAX(shiftx, ox);
				nx -= wd;
			}
			if (ny < 0) {
				shifty = MIN(shifty, ny);
				ny += ht;
			} else if (ny >= ht) {
				oy = (ny - ht) + 1;
				shifty = MAX(shifty, oy);
				ny -= ht;
			}

			si = (col & 7) / grid->bpp;
			byte = grid->sel.bitmap[(y * bpr) + (col >> 3)] & masks[si];
			if (byte) {
				rotated = 1;
				nx *= grid->bpp;
				di = (nx & 7) / grid->bpp;
				if (di < si)
					byte <<= (si - di) * grid->bpp;
				else if (di > si)
					byte >>= (di - si) * grid->bpp;
				scratch[(ny * bpr) + (nx >> 3)] |= byte;
			}
		}
	}

	if (rotated) {
		/*
		 * If a shift is required, then shift the scratch area back into
		 * the main bitmap.
		 */
		if (shiftx || shifty) {
			(void) memset((char *) grid->sel.bitmap, 0, bytes);
			for (y = 0; y < ht; y++) {
				for (col = x = 0; x < wd; x++, col += grid->bpp) {
					si = (col & 7) / grid->bpp;
					byte = scratch[(y * bpr) + (col >> 3)] & masks[si];
					if (byte) {
						nx = x - shiftx;
						ny = y - shifty;

						if (nx < 0)
							nx += wd;
						else if (nx >= wd)
							nx -= wd;
						if (ny < 0)
							ny += ht;
						else if (ny >= ht)
							ny -= ht;

						nx *= grid->bpp;
						di = (nx & 7) / grid->bpp;
						if (di < si)
							byte <<= (si - di) * grid->bpp;
						else if (di > si)
							byte >>= (di - si) * grid->bpp;
						grid->sel.bitmap[(ny * bpr) + (nx >> 3)] |= byte;
					}
				}
			}
		} else
			/*
			 * Copy the scratch buffer back to the main buffer.
			 */
			(void) memcpy((char *) grid->sel.bitmap, (char *) scratch, bytes);

		/*
		 * Determine the new selection width and height.
		 */
		ox = oy = 0;
		nx = ny = 16384;
		for (y = 0; y < ht; y++) {
			for (col = x = 0; x < wd; x++, col += grid->bpp) {
				si = (col & 7) / grid->bpp;
				if (grid->sel.bitmap[(y * bpr) + (col >> 3)] & masks[si]) {
					ox = MAX(ox, x);
					nx = MIN(nx, x);
					oy = MAX(oy, y);
					ny = MIN(ny, y);
				}
			}
		}

		/*
		 * Recalculate the center corrdinates so the selection will be
		 * positioned nicely once it is shifted to the upper left corner.
		 */
		cx = grid->sel.width >> 1;
		cy = grid->sel.height >> 1;

		/*
		 * Set the new width and height.
		 */
		grid->sel.width = (ox - nx) + 1;
		grid->sel.height = (oy - ny) + 1;

		/*
		 * Shift again to force the selection to the upper left corner.
		 */
		if (nx || ny) {
			(void) memset((char *) scratch, 0, bytes);
			for (y = 0; y < ht; y++) {
				for (col = x = 0; x < wd; x++, col += grid->bpp) {
					si = (col & 7) / grid->bpp;
					byte = grid->sel.bitmap[(y * bpr) + (col >> 3)] &
					       masks[si];
					if (byte) {
						oy = y - ny;
						ox = (x - nx) * grid->bpp;
						di = (ox & 7) / grid->bpp;
						if (di < si)
							byte <<= (si - di) * grid->bpp;
						else if (di > si)
							byte >>= (di - si) * grid->bpp;
						scratch[(oy * bpr) + (ox >> 3)] |= byte;
					}
				}
			}
			(void) memcpy((char *) grid->sel.bitmap, (char *) scratch, bytes);
		}

		/*
		 * Determine the new top left coordinates from the center coordinates.
		 */
		grid->sel.x = (grid->sel.x + cx) - (grid->sel.width >> 1);
		grid->sel.y = (grid->sel.y + cy) - (grid->sel.height >> 1);

		/*
		 * If the rotation caused the selection rectangle to overlap the edges
		 * of the grid, shift it so it is completely visible again.
		 */
		if (grid->sel.x + grid->sel.width > grid->grid_width)
			grid->sel.x -= (grid->sel.x + grid->sel.width) - grid->grid_width;
		if (grid->sel.y + grid->sel.height > grid->grid_height)
			grid->sel.y -= (grid->sel.y + grid->sel.height) - grid->grid_height;

		/*
		 * Mark the grid as being modified.
		 */
		grid->modified = 1;
	}

	return rotated;
}

static void
#ifdef __STDC__
_bdf_rotate_resize(bdf_glyph_grid_t *grid, int mul90, short degrees,
                   int *resize)
#else
_bdf_rotate_resize(grid, mul90, degrees, resize)
bdf_glyph_grid_t *grid;
int mul90;
short degrees;
int *resize;
#endif
{
	unsigned short wd, ht;
	short cx, cy, x1, y1, x2, y2;
	double dx1, dy1, dx2, dy2;
	bdf_metrics_t metrics;

	*resize = 0;
	(void) memset((char *) &metrics, 0, sizeof(bdf_metrics_t));

	metrics.x_offset = grid->font_bbx.x_offset;
	metrics.width = grid->font_bbx.width;
	metrics.ascent = grid->font_bbx.ascent;
	metrics.descent = grid->font_bbx.descent;
	metrics.height = grid->font_bbx.height;
	metrics.y_offset = grid->font_bbx.y_offset;

	cx = grid->glyph_x + (grid->glyph_bbx.width >> 1);
	cy = grid->glyph_y + (grid->glyph_bbx.height >> 1);

	/*
	 * Rotate the lower left and upper right corners and check for a potential
	 * resize.
	 */
	x1 = grid->glyph_x;
	y1 = grid->glyph_y + grid->glyph_bbx.height;
	x2 = grid->glyph_x + grid->glyph_bbx.width;
	y2 = grid->glyph_y;

	dx1 = (double)(x1 - cx);
	dy1 = (double)(y1 - cy);
	dx2 = (double)(x2 - cx);
	dy2 = (double)(y2 - cx);

	if (mul90) {
		x1 = cx + (short)((dx1 * _bdf_cos_tbl[degrees]) -
		                  (dy1 * _bdf_sin_tbl[degrees]));
		y1 = cy + (short)((dx1 * _bdf_sin_tbl[degrees]) +
		                  (dy1 * _bdf_cos_tbl[degrees]));
		x2 = cx + (short)((dx2 * _bdf_cos_tbl[degrees]) -
		                  (dy2 * _bdf_sin_tbl[degrees]));
		y2 = cy + (short)((dx2 * _bdf_sin_tbl[degrees]) +
		                  (dy2 * _bdf_cos_tbl[degrees]));
	} else {
		x1 = cx + _bdf_ceiling((dx1 * _bdf_cos_tbl[degrees]) -
		                       (dy1 * _bdf_sin_tbl[degrees]));
		y1 = cy + _bdf_ceiling((dx1 * _bdf_sin_tbl[degrees]) +
		                       (dy1 * _bdf_cos_tbl[degrees]));
		x2 = cx + _bdf_ceiling((dx2 * _bdf_cos_tbl[degrees]) -
		                       (dy2 * _bdf_sin_tbl[degrees]));
		y2 = cy + _bdf_ceiling((dx2 * _bdf_sin_tbl[degrees]) +
		                       (dy2 * _bdf_cos_tbl[degrees]));
	}

	wd = MYABS(x2 - x1);
	ht = MYABS(y2 - y1);
	if (wd > metrics.width) {
		metrics.width += wd - grid->font_bbx.width;
		*resize = 1;
	}
	if (ht > metrics.height) {
		metrics.ascent += ht - grid->font_bbx.height;
		metrics.height += ht - grid->font_bbx.height;
		*resize = 1;
	}

	/*
	 * Rotate the upper left and lower right corners and check for a potential
	 * resize.
	 */
	x1 = grid->glyph_x;
	y1 = grid->glyph_y;
	x2 = grid->glyph_x + grid->glyph_bbx.width;
	y2 = grid->glyph_y + grid->glyph_bbx.height;

	dx1 = (double)(x1 - cx);
	dy1 = (double)(y1 - cy);
	dx2 = (double)(x2 - cx);
	dy2 = (double)(y2 - cx);

	if (mul90) {
		x1 = cx + (short)((dx1 * _bdf_cos_tbl[degrees]) -
		                  (dy1 * _bdf_sin_tbl[degrees]));
		y1 = cy + (short)((dx1 * _bdf_sin_tbl[degrees]) +
		                  (dy1 * _bdf_cos_tbl[degrees]));
		x2 = cx + (short)((dx2 * _bdf_cos_tbl[degrees]) -
		                  (dy2 * _bdf_sin_tbl[degrees]));
		y2 = cy + (short)((dx2 * _bdf_sin_tbl[degrees]) +
		                  (dy2 * _bdf_cos_tbl[degrees]));
	} else {
		x1 = cx + _bdf_ceiling((dx1 * _bdf_cos_tbl[degrees]) -
		                       (dy1 * _bdf_sin_tbl[degrees]));
		y1 = cy + _bdf_ceiling((dx1 * _bdf_sin_tbl[degrees]) +
		                       (dy1 * _bdf_cos_tbl[degrees]));
		x2 = cx + _bdf_ceiling((dx2 * _bdf_cos_tbl[degrees]) -
		                       (dy2 * _bdf_sin_tbl[degrees]));
		y2 = cy + _bdf_ceiling((dx2 * _bdf_sin_tbl[degrees]) +
		                       (dy2 * _bdf_cos_tbl[degrees]));
	}

	wd = MYABS(x2 - x1);
	ht = MYABS(y2 - y1);
	if (wd > metrics.width) {
		metrics.width += wd - grid->font_bbx.width;
		*resize = 1;
	}
	if (ht > metrics.height) {
		metrics.ascent += ht - grid->font_bbx.height;
		metrics.height += ht - grid->font_bbx.height;
		*resize = 1;
	}

	if (*resize)
		(void) bdf_grid_resize(grid, &metrics);
}

static void
#ifdef __STDC__
_bdf_shear_resize(bdf_glyph_grid_t *grid, short degrees, int neg, int *resize)
#else
_bdf_shear_resize(grid, degrees, neg, resize)
bdf_glyph_grid_t *grid;
short degrees;
int neg, *resize;
#endif
{
	unsigned short wd;
	short x1, y1, x2, y2;
	bdf_metrics_t metrics;

	*resize = 0;
	(void) memset((char *) &metrics, 0, sizeof(bdf_metrics_t));

	metrics.x_offset = grid->font_bbx.x_offset;
	metrics.width = grid->font_bbx.width;
	metrics.ascent = grid->font_bbx.ascent;
	metrics.descent = grid->font_bbx.descent;
	metrics.height = grid->font_bbx.height;
	metrics.y_offset = grid->font_bbx.y_offset;

	/*
	 * Shear the lower left and upper right corners and check for a potential
	 * resize.
	 */
	x1 = 0;
	y1 = grid->glyph_bbx.height;
	x2 = grid->glyph_bbx.width;
	y2 = 0;

	if (neg) {
		x1 += (short)((double) y1 * _bdf_tan_tbl[degrees]);
		x2 += (short)((double) y2 * _bdf_tan_tbl[degrees]);
	} else {
		x1 += (short)((double)(grid->glyph_bbx.height - y1) *
		              _bdf_tan_tbl[degrees]);
		x2 += (short)((double)(grid->glyph_bbx.height - y2) *
		              _bdf_tan_tbl[degrees]);
	}

	wd = MYABS(x2 - x1);
	if (wd > metrics.width) {
		metrics.width += wd - grid->font_bbx.width;
		*resize = 1;
	}

	/*
	 * Shear the upper left and lower right corners and check for a potential
	 * resize.
	 */
	x1 = 0;
	y1 = 0;
	x2 = grid->glyph_bbx.width;
	y2 = grid->glyph_bbx.height;

	if (neg) {
		x1 += (short)((double) y1 * _bdf_tan_tbl[degrees]);
		x2 += (short)((double) y2 * _bdf_tan_tbl[degrees]);
	} else {
		x1 += (short)((double)(grid->glyph_bbx.height - y1) *
		              _bdf_tan_tbl[degrees]);
		x2 += (short)((double)(grid->glyph_bbx.height - y2) *
		              _bdf_tan_tbl[degrees]);
	}

	wd = MYABS(x2 - x1);
	if (wd > metrics.width) {
		metrics.width += wd - grid->font_bbx.width;
		*resize = 1;
	}

	if (*resize)
		(void) bdf_grid_resize(grid, &metrics);
}

/*
 * Rotate the bitmap in the grid by some number of degrees.
 */
int
#ifdef __STDC__
bdf_grid_rotate(bdf_glyph_grid_t *grid, short degrees, int *resize)
#else
bdf_grid_rotate(grid, degrees, resize)
bdf_glyph_grid_t *grid;
short degrees;
int *resize;
#endif
{
	int rotated, mul90;
	short nx, ny, cx, cy, x, y, wd, ht;
	short ox, oy, gx, gy, shiftx, shifty;
	unsigned short si, di, col, byte;
	double dx, dy;
	unsigned short bytes, bpr;
	unsigned char *scratch, *masks;

	rotated = 0;

	/*
	 * Make sure the number of degrees is between 0 and 359 and adjusted to a
	 * positive number of degrees if necessary.
	 */
	while (degrees < 0)
		degrees += 360;
	while (degrees >= 360)
		degrees -= 360;

	if (grid == 0 || degrees == 0 ||
	        (grid->glyph_bbx.width == 0 && grid->glyph_bbx.height == 0))
		return rotated;

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	mul90 = ((degrees % 90) == 0) ? 1 : 0;

	/*
	 * Force the grid to resize if the rotation requires it.
	 */
	_bdf_rotate_resize(grid, mul90, degrees, resize);

	if (grid->sel.width != 0 && grid->sel.height != 0)
		return _bdf_rotate_selection(grid, mul90, degrees);

	/*
	 * Halve the byte count in the grid for later use.
	 */
	bytes = grid->bytes >> 1;

	/*
	 * Point at the scratch buffer area and initialize it.
	 */
	scratch = grid->bitmap + bytes;
	(void) memset((char *) scratch, 0, bytes);

	/*
	 * Determine the bytes per row.
	 */
	bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;

	/*
	 * Determine the center coordinates of the glyph bitmap rectangle.
	 */
	cx = grid->glyph_x + (grid->glyph_bbx.width >> 1);
	cy = grid->glyph_y + (grid->glyph_bbx.height >> 1);

	/*
	 * Only run over the rectangle containing the glyph itself.
	 */
	gx = grid->glyph_x;
	gy = grid->glyph_y;

	wd = gx + grid->glyph_bbx.width;
	ht = gy + grid->glyph_bbx.height;

	/*
	 * Initialize the adjustment counts used if the bitmap
	 * wraps around the edge.
	 */
	shiftx = shifty = 0;

	for (y = gy; y < ht; y++) {
		col = gx * grid->bpp;
		for (x = gx; x < wd; x++, col += grid->bpp) {

			/*
			 * Rotate the point.
			 */
			dx = (double)(x - cx);
			dy = (double)(y - cy);
			if (mul90) {
				nx = cx + (short)((dx * _bdf_cos_tbl[degrees]) -
				                  (dy * _bdf_sin_tbl[degrees]));
				ny = cy + (short)((dx * _bdf_sin_tbl[degrees]) +
				                  (dy * _bdf_cos_tbl[degrees]));
			} else {
				nx = cx + _bdf_ceiling((dx * _bdf_cos_tbl[degrees]) -
				                       (dy * _bdf_sin_tbl[degrees]));
				ny = cy + _bdf_ceiling((dx * _bdf_sin_tbl[degrees]) +
				                       (dy * _bdf_cos_tbl[degrees]));
			}

			/*
			 * Wrap the coordinates around the edges if necessary.
			 */
			if (nx < 0) {
				shiftx = MIN(shiftx, nx);
				nx += grid->grid_width;
			} else if (nx >= grid->grid_width) {
				ox = (nx - grid->grid_width) + 1;
				shiftx = MAX(shiftx, ox);
				nx -= grid->grid_width;
			}
			if (ny < 0) {
				shifty = MIN(shifty, ny);
				ny += grid->grid_height;
			} else if (ny >= grid->grid_height) {
				oy = (ny - grid->grid_height) + 1;
				shifty = MAX(shifty, oy);
				ny -= grid->grid_height;
			}

			si = (col & 7) / grid->bpp;
			byte = grid->bitmap[(y * bpr) + (col >> 3)] & masks[si];
			if (byte) {
				rotated = 1;
				nx *= grid->bpp;
				di = (nx & 7) / grid->bpp;
				if (di < si)
					byte <<= (si - di) * grid->bpp;
				else if (di > si)
					byte >>= (di - si) * grid->bpp;
				scratch[(ny * bpr) + (nx >> 3)] |= byte;
			}
		}
	}

	if (rotated) {
		/*
		 * If a shift is required, then shift the scratch area back into
		 * the main bitmap.
		 */
		if (shiftx || shifty) {
			(void) memset((char *) grid->bitmap, 0, bytes);
			for (y = 0; y < grid->grid_height; y++) {
				for (col = x = 0; x < grid->grid_width;
				        x++, col += grid->bpp) {
					si = (col & 7) / grid->bpp;
					byte = scratch[(y * bpr) + (col >> 3)] & masks[si];
					if (byte) {
						nx = x - shiftx;
						ny = y - shifty;

						if (nx < 0)
							nx += grid->grid_width;
						else if (nx >= grid->grid_width)
							nx -= grid->grid_width;
						if (ny < 0)
							ny += grid->grid_height;
						else if (ny >= grid->grid_height)
							ny -= grid->grid_height;

						nx *= grid->bpp;
						di = (nx & 7) / grid->bpp;
						if (di < si)
							byte <<= (si - di) * grid->bpp;
						else if (di > si)
							byte >>= (di - si) * grid->bpp;
						grid->bitmap[(ny * bpr) + (nx >> 3)] |= byte;
					}
				}
			}
		} else
			/*
			 * Copy the scratch buffer back to the main buffer.
			 */
			(void) memcpy((char *) grid->bitmap, (char *) scratch, bytes);

		/*
		 * Determine the new glyph bounding box and the top left coordinates.
		 */
		ox = oy = 0;
		nx = ny = 16384;
		for (y = 0; y < grid->grid_height; y++) {
			for (col = x = 0; x < grid->grid_width; x++, col += grid->bpp) {
				si = (col & 7) / grid->bpp;
				if (grid->bitmap[(y * bpr) + (col >> 3)] & masks[si]) {
					nx = MIN(nx, x);
					ox = MAX(ox, x);
					ny = MIN(ny, y);
					oy = MAX(oy, y);
				}
			}
		}

		/*
		 * Set the new top left corrdinates.
		 */
		grid->glyph_x = nx;
		grid->glyph_y = ny;

		/*
		 * Set the new glyph bounding box.
		 */
		grid->glyph_bbx.width = (ox - nx) + 1;
		grid->glyph_bbx.x_offset = nx - grid->base_x;
		grid->glyph_bbx.height = (oy - ny) + 1;
		grid->glyph_bbx.ascent = grid->base_y - ny;
		grid->glyph_bbx.descent = grid->glyph_bbx.height -
		                          grid->glyph_bbx.ascent;
		grid->glyph_bbx.y_offset = -grid->glyph_bbx.descent;

		/*
		 * Mark the grid as being modified.
		 */
		grid->modified = 1;
	}

	return rotated;
}

int
#ifdef __STDC__
bdf_grid_shear(bdf_glyph_grid_t *grid, short degrees, int *resize)
#else
bdf_grid_shear(grid, degrees, resize)
bdf_glyph_grid_t *grid;
short degrees;
int *resize;
#endif
{
	int sheared, neg;
	short cx, cy, wd, ht, gx, gy, x, y;
	short nx, ox, ny, oy, shiftx, shifty;
	unsigned short bytes, bpr, si, di, col, byte;
	unsigned char *scratch, *masks;

	sheared = 0;

	if (degrees == 0 || degrees < -45 || degrees > 45 || grid == 0 ||
	        (grid->glyph_bbx.width == 0 && grid->glyph_bbx.height == 0))
		return sheared;

	if ((neg = (degrees < 0)))
		degrees = -degrees;

	/*
	 * Check to see if the grid needs to be resized to hold the sheared glyph.
	 */
	_bdf_shear_resize(grid, degrees, neg, resize);

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	/*
	 * Halve the byte count in the grid for later use.
	 */
	bytes = grid->bytes >> 1;

	/*
	 * Point at the scratch buffer area and initialize it.
	 */
	scratch = grid->bitmap + bytes;
	(void) memset((char *) scratch, 0, bytes);

	/*
	 * Determine the bytes per row.
	 */
	bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;

	/*
	 * Determine the center coordinates of the glyph bitmap rectangle.
	 */
	gx = grid->glyph_x;
	gy = grid->glyph_y;

	cx = gx + (grid->glyph_bbx.width >> 1);
	cy = gy + (grid->glyph_bbx.height >> 1);

	wd = gx + grid->glyph_bbx.width;
	ht = gy + grid->glyph_bbx.height;

	shiftx = shifty = 0;
	for (y = gy; y < ht; y++) {
		col = gx * grid->bpp;
		for (x = gx; x < wd; x++, col += grid->bpp) {
			ny = y;
			if (neg)
				nx = x + (short)((double) y * _bdf_tan_tbl[degrees]);
			else
				nx = x + (short)((double)(gy + (ht - y)) *
				                 _bdf_tan_tbl[degrees]);

			if (nx < 0) {
				shiftx = MIN(shiftx, nx);
				nx += grid->grid_width;
			} else if (nx >= grid->grid_width) {
				ox = (nx - grid->grid_width) + 1;
				shiftx = MAX(shiftx, ox);
				nx -= grid->grid_width;
			}
			if (ny < 0) {
				shifty = MIN(shifty, ny);
				ny += grid->grid_height;
			} else if (ny >= grid->grid_height) {
				oy = (ny - grid->grid_height) + 1;
				shifty = MAX(shifty, oy);
				ny -= grid->grid_height;
			}

			si = (col & 7) / grid->bpp;
			byte = grid->bitmap[(y * bpr) + (col >> 3)] & masks[si];
			if (byte) {
				sheared = 1;
				nx *= grid->bpp;
				di = (nx & 7) / grid->bpp;
				if (di < si)
					byte <<= (si - di) * grid->bpp;
				else if (di > si)
					byte >>= (di - si) * grid->bpp;
				scratch[(y * bpr) + (nx >> 3)] |= byte;
			}
		}
	}

	if (sheared) {
		/*
		 * If a shift is required, then shift the scratch area back into
		 * the main bitmap.
		 */
		if (shiftx || shifty) {
			(void) memset((char *) grid->bitmap, 0, bytes);
			for (y = 0; y < grid->grid_height; y++) {
				for (col = x = 0; x < grid->grid_width;
				        x++, col += grid->bpp) {
					si = (col & 7) / grid->bpp;
					byte = scratch[(y * bpr) + (col >> 3)] & masks[si];
					if (byte) {
						nx = x - shiftx;
						ny = y - shifty;

						if (nx < 0)
							nx += grid->grid_width;
						else if (nx >= grid->grid_width)
							nx -= grid->grid_width;
						if (ny < 0)
							ny += grid->grid_height;
						else if (ny >= grid->grid_height)
							ny -= grid->grid_height;

						nx *= grid->bpp;
						di = (nx & 7) / grid->bpp;
						if (di < si)
							byte <<= (si - di) * grid->bpp;
						else if (di > si)
							byte >>= (di - si) * grid->bpp;
						grid->bitmap[(ny * bpr) + (nx >> 3)] |= byte;
					}
				}
			}
		} else
			/*
			 * Copy the scratch buffer back to the main buffer.
			 */
			(void) memcpy((char *) grid->bitmap, (char *) scratch, bytes);

		ox = oy = 0;
		nx = ny = 16384;
		for (y = 0; y < grid->grid_height; y++) {
			for (col = x = 0; x < grid->grid_width; x++, col += grid->bpp) {
				si = (col & 7) / grid->bpp;
				if (grid->bitmap[(y * bpr) + (col >> 3)] & masks[si]) {
					ox = MAX(ox, x);
					nx = MIN(nx, x);
					oy = MAX(oy, y);
					ny = MIN(ny, y);
				}
			}
		}

		/*
		 * Set the new top left corrdinates.
		 */
		grid->glyph_x = nx;
		grid->glyph_y = ny;

		/*
		 * Set the new glyph bounding box.
		 */
		grid->glyph_bbx.width = (ox - nx) + 1;
		grid->glyph_bbx.x_offset = nx - grid->base_x;
		grid->glyph_bbx.height = (oy - ny) + 1;
		grid->glyph_bbx.ascent = grid->base_y - ny;
		grid->glyph_bbx.descent = grid->glyph_bbx.height -
		                          grid->glyph_bbx.ascent;
		grid->glyph_bbx.y_offset = -grid->glyph_bbx.descent;

		/*
		 * Mark the grid as being modified.
		 */
		grid->modified = 1;
	}

	return sheared;
}

int
#ifdef __STDC__
bdf_grid_embolden(bdf_glyph_grid_t *grid)
#else
bdf_grid_embolden(grid)
bdf_glyph_grid_t *grid;
#endif
{
	int done;
	short wd, ht, gx, gy, x, y;
	unsigned short b1, b2, bpr, si, di, col;
	unsigned char *masks;

	done = 0;

	if (grid == 0 ||
	        (grid->glyph_bbx.width == 0 && grid->glyph_bbx.height == 0))
		return done;

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	/*
	 * Determine the bytes per row.
	 */
	bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;

	gx = grid->glyph_x;
	gy = grid->glyph_y;

	wd = gx + grid->glyph_bbx.width;
	ht = gy + grid->glyph_bbx.height;

	if (grid->spacing == BDF_PROPORTIONAL ||
	        (grid->spacing == BDF_MONOWIDTH &&
	         grid->glyph_bbx.width < grid->font_bbx.width))
		/*
		 * Only allow horizontal expansion in the cases that make sense.
		 */
		wd++;

	for (y = gy; y < ht; y++) {
		col = (wd - 1) * grid->bpp;
		for (x = wd - 1; x > gx; x--, col -= grid->bpp) {
			si = (col & 7) / grid->bpp;
			di = ((col - grid->bpp) & 7) / grid->bpp;
			b1 = grid->bitmap[(y * bpr) + (col >> 3)] & masks[si];
			b2 = grid->bitmap[(y * bpr) + ((col - grid->bpp) >> 3)] &
			     masks[di];
			if (!b1 && b2) {
				if (di < si)
					b2 >>= (si - di) * grid->bpp;
				else if (di > si)
					b2 <<= (di - si) * grid->bpp;
				grid->bitmap[(y * bpr) + (col >> 3)] |= b2;
				/*
				 * Mark the grid as being modified.
				 */
				done = grid->modified = 1;
			}
		}
	}

	/*
	 * Adjust the glyph width so it will be reflected when the glyph is stored
	 * back in the font.
	 */
	grid->glyph_bbx.width = wd - gx;

	return done;
}

/**************************************************************************
 *
 * Glyph grid selection functions.
 *
 **************************************************************************/

int
#ifdef __STDC__
bdf_has_selection(bdf_glyph_grid_t *grid, short *x, short *y,
                  short *width, short *height)
#else
bdf_has_selection(grid, x, y, width, height)
bdf_glyph_grid_t *grid;
short *x, *y, *width, *height;
#endif
{
	if (grid == 0 || (grid->sel.width == 0 && grid->sel.height == 0))
		return 0;

	if (x != 0)
		*x = grid->sel.x;
	if (y != 0)
		*y = grid->sel.y;
	if (width != 0)
		*width = grid->sel.width;
	if (height != 0)
		*height = grid->sel.height;

	return 1;
}

/*
 * Select a rectangle on the grid.
 */
void
#ifdef __STDC__
bdf_set_selection(bdf_glyph_grid_t *grid, short x, short y,
                  short width, short height)
#else
bdf_set_selection(grid, x, y, width, height)
bdf_glyph_grid_t *grid;
short x, y, width, height;
#endif
{
	short nx, ny, wd, ht, ssize, dx, dy, col;
	unsigned short bytes, bpr, sbpr, si, di, byte;
	unsigned char *masks;

	if (grid == 0)
		return;

	/*
	 * Make sure the specified rectangle is within reasonable bounds.
	 */
	if (x < 0 || x >= grid->grid_width)
		x = 0;
	if (y < 0 || y >= grid->grid_height)
		y = 0;

	if (x + width > grid->grid_width)
		width = (x + width) - grid->grid_width;
	if (y + height > grid->grid_height)
		height = (y + height) - grid->grid_height;

	grid->sel.x = x;
	grid->sel.y = y;
	grid->sel.width = width;
	grid->sel.height = height;

	/*
	 * Allocate enough space to represent a square the size of the largest
	 * of the width and height of the selection.  This allows rotation and
	 * flipping of the selected bitmap.
	 */
	ssize = MAX(width, height);

	bytes = ((((ssize * grid->bpp) + 7) >> 3) * ssize) << 1;

	/*
	 * If the selection is being removed (width and height are 0), then simply
	 * return.
	 */
	if (bytes == 0)
		return;

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	if (bytes > grid->sel.bytes) {
		if (grid->sel.bytes == 0)
			grid->sel.bitmap = (unsigned char *) malloc(bytes);
		else
			grid->sel.bitmap = (unsigned char *)
			                   realloc((char *) grid->sel.bitmap, bytes);
		grid->sel.bytes = bytes;
	} else
		bytes = grid->sel.bytes;

	/*
	 * Initialize the selection bitmap and copy the selected bits to it.
	 */
	(void) memset((char *) grid->sel.bitmap, 0, bytes);

	wd = x + width;
	ht = y + height;

	bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;
	sbpr = ((grid->sel.width * grid->bpp) + 7) >> 3;

	for (ny = 0, dy = y; dy < ht; dy++, ny++) {
		col = x * grid->bpp;
		for (nx = 0, dx = x; dx < wd;
		        dx++, nx += grid->bpp, col += grid->bpp) {
			si = (col & 7) / grid->bpp;
			byte = grid->bitmap[(dy * bpr) + (col >> 3)] & masks[si];
			if (byte) {
				di = (nx & 7) / grid->bpp;
				if (di < si)
					byte <<= (si - di) * grid->bpp;
				else if (di > si)
					byte >>= (di - si) * grid->bpp;
				grid->sel.bitmap[(ny * sbpr) + (nx >> 3)] |= byte;
			}
		}
	}
}

/*
 * Detach a selection in preparation for moving it.  What is does is clear the
 * bits set in the selection from the main grid.  Again, this is only used for
 * move operations.
 */
void
#ifdef __STDC__
bdf_detach_selection(bdf_glyph_grid_t *grid)
#else
bdf_detach_selection(grid)
bdf_glyph_grid_t *grid;
#endif
{
	short sx, sy, x, y, wd, ht, dx;
	unsigned short bpr, sbpr, si, di, byte;
	unsigned char *masks;

	if (grid == 0 || (grid->sel.width == 0 && grid->sel.height == 0))
		return;

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;
	sbpr = ((grid->sel.width * grid->bpp) + 7) >> 3;

	wd = grid->sel.x + grid->sel.width;
	ht = grid->sel.y + grid->sel.height;

	for (sy = 0, y = grid->sel.y; y < ht; y++, sy++) {
		for (sx = 0, x = grid->sel.x; x < wd; x++, sx += grid->bpp) {
			si = (sx & 7) / grid->bpp;
			byte = grid->sel.bitmap[(sy * sbpr) + (sx >> 3)] & masks[si];
			if (byte) {
				dx = x * grid->bpp;
				di = (dx & 7) / grid->bpp;
				grid->bitmap[(y * bpr) + (dx >> 3)] &= ~masks[di];
			}
		}
	}

	/*
	 * Crop the new image to determine the new bounds with the selection.
	 */
	(void) bdf_grid_crop(grid, 1);
}

void
#ifdef __STDC__
bdf_attach_selection(bdf_glyph_grid_t *grid)
#else
bdf_attach_selection(grid)
bdf_glyph_grid_t *grid;
#endif
{
	short sx, sy, x, y, wd, ht;
	unsigned short bpr, sbpr, dx, di, si, byte;
	unsigned char *masks;

	if (grid == 0 || (grid->sel.width == 0 && grid->sel.height == 0))
		return;

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;
	sbpr = ((grid->sel.width * grid->bpp) + 7) >> 3;

	wd = grid->sel.x + grid->sel.width;
	ht = grid->sel.y + grid->sel.height;

	for (sy = 0, y = grid->sel.y; y < ht; y++, sy++) {
		for (sx = 0, x = grid->sel.x; x < wd; x++, sx += grid->bpp) {
			si = (sx & 7) / grid->bpp;
			byte = grid->sel.bitmap[(sy * sbpr) + (sx >> 3)] & masks[si];
			if (byte) {
				dx = x * grid->bpp;
				di = (dx & 7) / grid->bpp;
				if (di < si)
					byte <<= (si - di) * grid->bpp;
				else if (di > si)
					byte >>= (di - si) * grid->bpp;
				grid->bitmap[(y * bpr) + (dx >> 3)] |= byte;
			}
		}
	}

	/*
	 * Crop the new image to determine the new bounds with the selection.
	 */
	(void) bdf_grid_crop(grid, 1);
}

/*
 * Indicate the selection no longer exists by setting the width and height to
 * 0.
 */
void
#ifdef __STDC__
bdf_lose_selection(bdf_glyph_grid_t *grid)
#else
bdf_lose_selection(grid)
bdf_glyph_grid_t *grid;
#endif
{
	if (grid == 0)
		return;
	grid->sel.width = grid->sel.height = 0;
}

/*
 * Delete the selection by first detaching it which will erase the rectangle
 * on the grid and then losing the selection.
 */
void
#ifdef __STDC__
bdf_delete_selection(bdf_glyph_grid_t *grid)
#else
bdf_delete_selection(grid)
bdf_glyph_grid_t *grid;
#endif
{
	bdf_detach_selection(grid);
	bdf_lose_selection(grid);
}

/*
 * Check to see if a coordinate pair is in the selected region.
 */
int
#ifdef __STDC__
bdf_in_selection(bdf_glyph_grid_t *grid, short x, short y, short *set)
#else
bdf_in_selection(grid, x, y, set)
bdf_glyph_grid_t *grid;
short x, y, *set;
#endif
{
	short wd, ht;
	unsigned short bpr, si, di, byte;
	unsigned char *masks;

	if (grid == 0 || (grid->sel.width == 0 && grid->sel.height == 0))
		return 0;

	di = 0;
	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		di = 7;
		break;
	case 2:
		masks = twobpp;
		di = 3;
		break;
	case 4:
		masks = fourbpp;
		di = 1;
		break;
	}

	bpr = ((grid->sel.width * grid->bpp) + 7) >> 3;

	wd = grid->sel.x + grid->sel.width;
	ht = grid->sel.y + grid->sel.height;

	if ((x >= grid->sel.x && x < wd) && (y >= grid->sel.y && y < ht)) {
		if (set) {
			/*
			 * Adjust the byte back to an index value.
			 */
			x *= grid->bpp;
			si = (x & 7) / grid->bpp;
			byte = grid->sel.bitmap[(y * bpr) + (x >> 3)] & masks[si];
			if (di > si)
				byte >>= (di - si) * grid->bpp;
			*set = byte;
		}
		return 1;
	}

	return 0;
}

int
#ifdef __STDC__
bdf_grid_shift(bdf_glyph_grid_t *grid, short xcount, short ycount)
#else
bdf_grid_shift(grid, xcount, ycount)
bdf_glyph_grid_t *grid;
short xcount, ycount;
#endif
{
	int sel, delta;
	short xdir, ydir, x, y, wd, ht, dx, dy, nx, ny;
	unsigned short bytes, bpr, si, di, byte, col;
	unsigned char *scratch, *masks;

	if (grid == 0)
		return 0;

	xdir = ydir = 1;
	if (xcount < 0) {
		xdir = -1;
		xcount = -xcount;
	}

	if (ycount < 0) {
		ydir = -1;
		ycount = -ycount;
	}

	/*
	 * Adjust the shift counts if they are larger than they should be.
	 */
	if (xcount > grid->grid_width)
		xcount -= grid->grid_width;
	if (ycount > grid->grid_height)
		ycount -= grid->grid_height;

	/*
	 * Adjust the counts to limit the shift to the boundaries of the grid.
	 */
	if (grid->sel.width != 0 && grid->sel.height != 0) {
		/*
		 * The selection is being shifted.
		 */
		x = grid->sel.x;
		y = grid->sel.y;
		wd = grid->sel.width;
		ht = grid->sel.height;
		sel = 1;
	} else {
		x = grid->glyph_x;
		y = grid->glyph_y;
		wd = grid->glyph_bbx.width;
		ht = grid->glyph_bbx.height;
		sel = 0;
	}

	/*
	 * If the width and height are 0, then simply return, because there
	 * is nothing to shift.
	 */
	if (wd == 0 && ht == 0)
		return 0;

	if (xdir == 1 && x + wd + xcount > grid->grid_width)
		xcount = grid->grid_width - (x + wd);
	else if (xdir == -1 && xcount > x)
		xcount = x;

	if (ydir == 1 && y + ht + ycount > grid->grid_height)
		ycount = grid->grid_height - (y + ht);
	else if (ydir == -1 && ycount > y)
		ycount = y;

	if (xcount == 0 && ycount == 0)
		return 0;

	/*
	 * If the selection is the one being shifted, adjust the X and Y
	 * coordinates and adjust the glyph metrics.
	 */
	if (sel) {
		/*
		 * Determine the actual ink bounds of the selection so the
		 * glyph metrics can be adjusted if necessary.
		 */
		if (_bdf_grid_ink_bounds(grid, &x, &y, &wd, &ht)) {
			/*
			 * Have to adjust the glyph metrics.
			 */
			x += xdir * xcount;
			y += ydir * ycount;
			if (x < grid->glyph_x) {
				delta = grid->glyph_x - x;
				grid->glyph_bbx.width += delta;
				grid->glyph_bbx.x_offset -= delta;
				if (grid->spacing == BDF_PROPORTIONAL)
					grid->dwidth += delta;
				grid->glyph_x -= delta;
			} else if (x >= grid->glyph_x + grid->glyph_bbx.width) {
				delta = x - (grid->glyph_x + grid->glyph_bbx.width);
				grid->glyph_bbx.width += delta;
				if (grid->spacing == BDF_PROPORTIONAL)
					grid->dwidth += delta;
			}

			if (y < grid->glyph_y) {
				delta = grid->glyph_y - y;
				grid->glyph_bbx.height += delta;
				grid->glyph_bbx.ascent += delta;
				grid->glyph_y -= delta;
			} else if (y + ht >= grid->glyph_y + grid->glyph_bbx.height) {
				delta = (y + ht) - (grid->glyph_y + grid->glyph_bbx.height);
				grid->glyph_bbx.height += delta;
				grid->glyph_bbx.y_offset -= delta;
				grid->glyph_bbx.descent += delta;
			}

			grid->modified = 1;
		}

		/*
		 * Adjust the top-left coordinate of the selection rectangle.
		 */
		grid->sel.x += xdir * xcount;
		grid->sel.y += ydir * ycount;

		return 1;
	}

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		di = 7;
		break;
	case 2:
		masks = twobpp;
		di = 3;
		break;
	case 4:
		masks = fourbpp;
		di = 1;
		break;
	}

	/*
	 * The glyph itself is being shifted.
	 */
	bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;
	bytes = grid->bytes >> 1;
	scratch = grid->bitmap + bytes;
	(void) memset((char *) scratch, 0, bytes);

	/*
	 * Shift just the glyph rectangle to keep things fast.
	 */
	wd += x;
	ht += y;
	for (dy = y; dy < ht; dy++) {
		col = x * grid->bpp;
		for (dx = x; dx < wd; dx++, col += grid->bpp) {
			si = (col & 7) / grid->bpp;
			byte = grid->bitmap[(dy * bpr) + (col >> 3)] & masks[si];
			if (byte) {
				nx = dx + (xdir * xcount);
				ny = dy + (ydir * ycount);
				nx *= grid->bpp;
				di = (nx & 7) / grid->bpp;
				if (di < si)
					byte <<= (si - di) * grid->bpp;
				else if (di > si)
					byte >>= (di - si) * grid->bpp;
				scratch[(ny * bpr) + (nx >> 3)] |= byte;
			}
		}
	}

	/*
	 * Copy the scratch buffer back to the main buffer.
	 */
	(void) memcpy((char *) grid->bitmap, (char *) scratch, bytes);

	/*
	 * Adjust the top-left coordinate of the glyph rectangle.
	 */
	grid->glyph_x += xdir * xcount;
	grid->glyph_y += ydir * ycount;

	/*
	 * Adjust the glyph offsets relative to the baseline coordinates.
	 */
	grid->glyph_bbx.x_offset = grid->glyph_x - grid->base_x;
	grid->glyph_bbx.y_offset = grid->base_y -
	                           (grid->glyph_y + grid->glyph_bbx.height);

	/*
	 * Adjust the glyph ascent and descent.
	 */
	grid->glyph_bbx.ascent = grid->base_y - grid->glyph_y;
	grid->glyph_bbx.descent = (grid->glyph_y + grid->glyph_bbx.height) -
	                          grid->base_y;

	/*
	 * Mark the grid as being modified.
	 */
	grid->modified = 1;

	return 1;
}


int
#ifdef __STDC__
bdf_grid_flip(bdf_glyph_grid_t *grid, short dir)
#else
bdf_grid_flip(grid, dir)
bdf_glyph_grid_t *grid;
short dir;
#endif
{
	int flipped, sel, delta;
	short dx, dy, x, y, nx, ny, wd, ht;
	unsigned short bytes, bpr, si, di, col, colx, byte;
	unsigned char *bmap, *scratch, *masks;

	flipped = 0;

	if (grid == 0)
		return flipped;

	if (grid->sel.width != 0 && grid->sel.height != 0) {
		sel = 1;
		x = y = 0;
		wd = grid->sel.width;
		ht = grid->sel.height;
		bpr = ((wd * grid->bpp) + 7) >> 3;
		bytes = grid->sel.bytes >> 1;
		bmap = grid->sel.bitmap;
	} else {
		sel = 0;
		x = grid->glyph_x;
		y = grid->glyph_y;
		wd = grid->glyph_bbx.width;
		ht = grid->glyph_bbx.height;
		bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;
		bytes = grid->bytes >> 1;
		bmap = grid->bitmap;
	}

	/*
	 * If the width or height is 0, don't do anything.
	 */
	if (wd == 0 || ht == 0)
		return flipped;

	nx = 0;
	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		di = 7;
		break;
	case 2:
		masks = twobpp;
		di = 3;
		break;
	case 4:
		masks = fourbpp;
		di = 1;
		break;
	}

	/*
	 * Set and initialize the scratch area.
	 */
	scratch = bmap + bytes;
	(void) memset((char *) scratch, 0, bytes);

	wd += x;
	ht += y;

	if (dir < 0) {
		/*
		 * Flip horizontally.
		 */
		for (dy = y; dy < ht; dy++) {
			col = x * grid->bpp;
			for (nx = wd - 1, dx = x; dx < wd; dx++, nx--, col += grid->bpp) {
				si = (col & 7) / grid->bpp;
				byte = bmap[(dy * bpr) + (col >> 3)] & masks[si];
				if (byte) {
					flipped = 1;
					colx = nx * grid->bpp;
					di = (colx & 7) / grid->bpp;
					if (di < si)
						byte <<= (si - di) * grid->bpp;
					else if (di > si)
						byte >>= (di - si) * grid->bpp;
					scratch[(dy * bpr) + (colx >> 3)] |= byte;
				}
			}
		}
		if (flipped) {
			if (sel)
				grid->sel.x += nx + 1;
			else {
				grid->glyph_x = nx + 1;
				grid->glyph_bbx.x_offset = grid->glyph_x - grid->base_x;
			}
		}
	} else {
		/*
		 * Flip vertically.
		 */
		for (ny = ht - 1, dy = y; dy < ht; dy++, ny--) {
			col = x * grid->bpp;
			for (dx = x; dx < wd; dx++, col += grid->bpp) {
				si = (col & 7) / grid->bpp;
				byte = bmap[(dy * bpr) + (col >> 3)] & masks[si];
				if (byte) {
					flipped = 1;
					scratch[(ny * bpr) + (col >> 3)] |= byte;
				}
			}
		}
		if (flipped) {
			if (sel)
				grid->sel.y += ny + 1;
			else {
				grid->glyph_y = ny + 1;
				grid->glyph_bbx.y_offset = grid->base_y -
				                           (grid->glyph_y + grid->glyph_bbx.height);
				grid->glyph_bbx.ascent = grid->base_y - grid->glyph_y;
				grid->glyph_bbx.descent =
				    (grid->glyph_y + grid->glyph_bbx.height) - grid->base_y;
			}
		}
	}

	if (flipped) {
		/*
		 * Copy the scratch area back to the working area.
		 */
		if (sel)
			(void) memcpy((char *) grid->sel.bitmap, (char *) scratch, bytes);
		else
			(void) memcpy((char *) grid->bitmap, (char *) scratch, bytes);

		if (sel) {
			/*
			 * Check to see if flipping the selection caused the glyph metrics
			 * to change.
			 */
			if (_bdf_grid_ink_bounds(grid, &x, &y, &wd, &ht)) {
				if (x < grid->glyph_x) {
					delta = grid->glyph_x - x;
					grid->glyph_bbx.width += delta;
					grid->glyph_bbx.x_offset -= delta;
					grid->glyph_x -= delta;
					if (grid->spacing == BDF_PROPORTIONAL)
						grid->dwidth += delta;
				} else if (x >= grid->glyph_x + grid->glyph_bbx.width) {
					delta = x - (grid->glyph_x + grid->glyph_bbx.width);
					grid->glyph_bbx.width += delta;
					if (grid->spacing == BDF_PROPORTIONAL)
						grid->dwidth += delta;
				}

				if (y < grid->glyph_y) {
					delta = grid->glyph_y - y;
					grid->glyph_bbx.height += delta;
					grid->glyph_bbx.ascent += delta;
					grid->glyph_y -= delta;
				} else if (y >= grid->glyph_y + grid->glyph_bbx.height) {
					delta = y - (grid->glyph_y + grid->glyph_bbx.height);
					grid->glyph_bbx.height += delta;
					grid->glyph_bbx.y_offset -= delta;
					grid->glyph_bbx.descent += delta;
				}
			}
		}

		/*
		 * Mark the grid as being modified.
		 */
		grid->modified = 1;
	}

	return flipped;
}

void
#ifdef __STDC__
bdf_grid_origin(bdf_glyph_grid_t *grid, short *x, short *y)
#else
bdf_grid_origin(grid, x, y)
bdf_glyph_grid_t *grid;
short *x, *y;
#endif
{
	if (grid == 0)
		return;

	*x = grid->base_x;
	*y = grid->base_y;
}

bdf_glyph_t *
#ifdef __STDC__
bdf_grid_glyph(bdf_glyph_grid_t *grid)
#else
bdf_grid_glyph(grid)
bdf_glyph_grid_t *grid;
#endif
{
	int len;
	short x, y, nx, ny, wd, ht, gx, gy;
	unsigned short bpr, nbpr, si, di, col, byte;
	bdf_glyph_t *glyph;
	unsigned char *masks;
	double ps, dw, rx;

	if (grid == 0)
		return 0;

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		di = 7;
		break;
	case 2:
		masks = twobpp;
		di = 3;
		break;
	case 4:
		masks = fourbpp;
		di = 1;
		break;
	}

	/*
	 * Create the new glyph.
	 */
	glyph = (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t));
	(void) memset((char *) glyph, 0, sizeof(bdf_glyph_t));

	gx = grid->glyph_x;
	gy = grid->glyph_y;

	/*
	 * Copy the bounding box.
	 */
	(void) memcpy((char *) &glyph->bbx, (char *) &grid->glyph_bbx,
	              sizeof(bdf_bbx_t));

	/*
	 * If the font has character-cell spacing, then make sure the bitmap is
	 * cropped to fit within the bounds of the font bbx.
	 */
	if (grid->spacing == BDF_CHARCELL) {
		if (gx < grid->base_x) {
			glyph->bbx.x_offset = 0;
			glyph->bbx.width -= grid->base_x - gx;
			gx += grid->base_x - gx;
		}
		if (glyph->bbx.width > grid->font_bbx.width)
			glyph->bbx.width -= glyph->bbx.width - grid->font_bbx.width;
	}

	/*
	 * Set up its bitmap.
	 */
	nbpr = ((glyph->bbx.width * grid->bpp) + 7) >> 3;
	glyph->bytes = nbpr * glyph->bbx.height;
	glyph->bitmap = (unsigned char *) malloc(glyph->bytes);
	(void) memset((char *) glyph->bitmap, 0, glyph->bytes);

	/*
	 * Set the other values.
	 */
	if (grid->name != 0) {
		len = strlen(grid->name) + 1;
		glyph->name = (char *) malloc(len);
		(void) memcpy(glyph->name, grid->name, len);
	}
	glyph->encoding = grid->encoding;
	glyph->dwidth = grid->dwidth;

	/*
	 * Reset the glyph SWIDTH value.
	 */
	ps = (double) grid->point_size;
	rx = (double) grid->resolution_x;
	dw = (double) grid->dwidth;
	glyph->swidth = (unsigned short)((dw * 72000.0) / (ps * rx));

	bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;
	wd = gx + glyph->bbx.width;
	ht = gy + glyph->bbx.height;

	/*
	 * Copy the bitmap from the grid into the glyph.
	 */
	for (ny = 0, y = gy; y < ht; y++, ny++) {
		col = gx * grid->bpp;
		for (nx = 0, x = gx; x < wd; x++, nx += grid->bpp, col += grid->bpp) {
			si = (col & 7) / grid->bpp;
			byte = grid->bitmap[(y * bpr) + (col >> 3)] & masks[si];
			if (byte) {
				di = (nx & 7) / grid->bpp;
				if (di < si)
					byte <<= (si - di) * grid->bpp;
				else if (di > si)
					byte >>= (di - si) * grid->bpp;
				glyph->bitmap[(ny * nbpr) + (nx >> 3)] |= byte;
			}
		}
	}

	/*
	 * Return the new glyph.
	 */
	return glyph;
}

/*
 * Create a bitmap with the glyph image as well as the selection.
 */
void
#ifdef __STDC__
bdf_grid_image(bdf_glyph_grid_t *grid, bdf_bitmap_t *image)
#else
bdf_grid_image(grid, image)
bdf_glyph_grid_t *grid;
bdf_bitmap_t *image;
#endif
{
	short x, y, ix, iy;
	unsigned short bpr, ibpr, si, di, col, colx, byte;
	unsigned char *masks;

	if (grid == 0 || image == 0)
		return;

	masks = 0;
	switch (grid->bpp) {
	case 1:
		masks = onebpp;
		di = 7;
		break;
	case 2:
		masks = twobpp;
		di = 3;
		break;
	case 4:
		masks = fourbpp;
		di = 1;
		break;
	}

	image->bpp = grid->bpp;
	image->x = image->y = 0;
	image->width = grid->grid_width;
	image->height = grid->grid_height;
	image->bytes = grid->bytes >> 1;
	image->bitmap = (unsigned char *) malloc(image->bytes);
	(void) memcpy((char *) image->bitmap, (char *) grid->bitmap, image->bytes);

	/*
	 * Add the selection to the bitmap if it exists.
	 */
	if (grid->sel.width != 0 && grid->sel.height != 0) {
		ibpr = ((image->width * grid->bpp) + 7) >> 3;
		bpr = ((grid->sel.width * grid->bpp) + 7) >> 3;
		for (iy = grid->sel.y, y = 0; y < grid->sel.height; y++, iy++) {
			for (ix = grid->sel.x, col = x = 0; x < grid->sel.width;
			        x++, ix++, col += grid->bpp) {
				si = (col & 7) / grid->bpp;
				byte = grid->sel.bitmap[(y * bpr) + (col >> 3)] & masks[si];
				if (byte) {
					colx = ix * grid->bpp;
					di = (colx & 7) / grid->bpp;
					if (di < si)
						byte <<= (si - di) * grid->bpp;
					else if (di > si)
						byte >>= (di - si) * grid->bpp;
					image->bitmap[(iy * ibpr) + (colx >> 3)] |= byte;
				}
			}
		}
	}
}

/*
 * Routines for quick and dirty dithering.
 */
static void
#ifdef __STDC__
_bdf_one_to_n(bdf_bitmap_t *bmap, int n)
#else
_bdf_one_to_n(bmap, n)
bdf_bitmap_t *bmap;
int n;
#endif
{
	unsigned short bpr, sbpr, bytes, col, sx, sy;
	unsigned char *nbmap, *masks;

	if (bmap == 0 || bmap->width == 0 || bmap->height == 0)
		return;

	masks = 0;
	switch (n) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	sbpr = (bmap->width + 7) >> 3;
	bpr = ((bmap->width * n) + 7) >> 3;
	bytes = bpr * bmap->height;
	nbmap = (unsigned char *) malloc(bytes);
	(void) memset((char *) nbmap, 0, bytes);

	for (sy = 0; sy < bmap->height; sy++) {
		for (col = sx = 0; sx < bmap->width; sx++, col += n) {
			if (bmap->bitmap[(sy * sbpr) + (sx >> 3)] & (0x80 >> (sx & 7)))
				nbmap[(sy * bpr) + (col >> 3)] |= masks[(col & 7) / n];
		}
	}
	free((char *) bmap->bitmap);
	bmap->bpp = n;
	bmap->bytes = bytes;
	bmap->bitmap = nbmap;
}

static void
#ifdef __STDC__
_bdf_n_to_one(bdf_bitmap_t *bmap)
#else
_bdf_n_to_one(bmap)
bdf_bitmap_t *bmap;
#endif
{
	unsigned short bpr, sbpr, bytes, col, sx, sy;
	unsigned char *nbmap, *masks;

	if (bmap == 0 || bmap->width == 0 || bmap->height == 0)
		return;

	masks = 0;
	switch (bmap->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	sbpr = ((bmap->width * bmap->bpp) + 7) >> 3;
	bpr = (bmap->width + 7) >> 3;
	bytes = bpr * bmap->height;
	nbmap = (unsigned char *) malloc(bytes);
	(void) memset((char *) nbmap, 0, bytes);

	for (sy = 0; sy < bmap->height; sy++) {
		for (col = sx = 0; sx < bmap->width; sx++, col += bmap->bpp) {
			if (bmap->bitmap[(sy * sbpr) + (col >> 3)] &
			        masks[(col & 7) / bmap->bpp])
				nbmap[(sy * bpr) + (sx >> 3)] |= (0x80 >> (sx & 7));
		}
	}
	free((char *) bmap->bitmap);
	bmap->bpp = 1;
	bmap->bytes = bytes;
	bmap->bitmap = nbmap;
}

static void
#ifdef __STDC__
_bdf_two_to_four(bdf_bitmap_t *bmap)
#else
_bdf_two_to_four(bmap)
bdf_bitmap_t *bmap;
#endif
{
	unsigned short bpr, sbpr, bytes, col, si, byte, sx, sy;
	unsigned char *nbmap, *masks;

	if (bmap == 0 || bmap->width == 0 || bmap->height == 0)
		return;

	masks = twobpp;

	sbpr = ((bmap->width << 1) + 7) >> 3;
	bpr = ((bmap->width << 2) + 7) >> 3;
	bytes = bpr * bmap->height;
	nbmap = (unsigned char *) malloc(bytes);
	(void) memset((char *) nbmap, 0, bytes);

	for (sy = 0; sy < bmap->height; sy++) {
		for (col = sx = 0; sx < bmap->width; sx++, col += 2) {
			si = (col & 7) >> 1;
			byte = bmap->bitmap[(sy * sbpr) + (col >> 3)] & masks[si];
			if (byte) {
				/*
				 * Shift the byte down to make an index.
				 */
				if (si < 3)
					byte >>= (3 - si) << 1;

				/*
				 * Scale the index to four bits per pixel and shift it into
				 * place before adding it.
				 */
				byte = (byte << 2) + 3;
				if ((sx & 1) == 0)
					byte <<= 4;
				nbmap[(sy * bpr) + ((sx << 2) >> 3)] |= byte;
			}
		}
	}
	free((char *) bmap->bitmap);
	bmap->bpp = 4;
	bmap->bytes = bytes;
	bmap->bitmap = nbmap;
}

static void
#ifdef __STDC__
_bdf_four_to_two(bdf_bitmap_t *bmap)
#else
_bdf_four_to_two(bmap)
bdf_bitmap_t *bmap;
#endif
{
	unsigned short bpr, sbpr, bytes, col, si, byte, sx, sy;
	unsigned char *nbmap, *masks;

	if (bmap == 0 || bmap->width == 0 || bmap->height == 0)
		return;

	masks = fourbpp;

	sbpr = ((bmap->width << 2) + 7) >> 3;
	bpr = ((bmap->width << 1) + 7) >> 3;
	bytes = bpr * bmap->height;
	nbmap = (unsigned char *) malloc(bytes);
	(void) memset((char *) nbmap, 0, bytes);

	for (sy = 0; sy < bmap->height; sy++) {
		for (col = sx = 0; sx < bmap->width; sx++, col += 4) {
			si = (col & 7) >> 2;
			byte = bmap->bitmap[(sy * sbpr) + (col >> 3)] & masks[si];
			if (byte) {
				/*
				 * Shift the byte down to make an index.
				 */
				if (si == 0)
					byte >>= 4;

				/*
				 * Scale the index to two bits per pixel and shift it into
				 * place if necessary.
				 */
				byte >>= 2;

				si = ((sx << 1) & 7) >> 1;
				if (si < 3)
					byte <<= (3 - si) << 1;

				nbmap[(sy * bpr) + ((sx << 1) >> 3)] |= byte;
			}
		}
	}
	free((char *) bmap->bitmap);
	bmap->bpp = 2;
	bmap->bytes = bytes;
	bmap->bitmap = nbmap;
}

/*
 * Add a bitmap to a grid as a selection.
 */
void
#ifdef __STDC__
bdf_add_selection(bdf_glyph_grid_t *grid, bdf_bitmap_t *sel)
#else
bdf_add_selection(grid, sel)
bdf_glyph_grid_t *grid;
bdf_bitmap_t *sel;
#endif
{
	unsigned short bytes, bpr;

	if (grid == 0 || sel == 0 || sel->width == 0 || sel->height == 0 ||
	        sel->bytes == 0)
		return;

	if (sel->bpp != grid->bpp) {
		/*
		 * Dither the incoming bitmap to match the same bits per pixel as the
		 * grid it is being added to.
		 */
		if (sel->bpp == 1)
			_bdf_one_to_n(sel, grid->bpp);
		else if (grid->bpp == 1)
			_bdf_n_to_one(sel);
		else if (sel->bpp == 2)
			_bdf_two_to_four(sel);
		else
			_bdf_four_to_two(sel);
	}

	/*
	 * If the bitmap is too big then trim the right and/or the bottom to fit
	 * in the grid.
	 */
	if (sel->width > grid->grid_width)
		sel->width = grid->grid_width;
	if (sel->height > grid->grid_height)
		sel->height = grid->grid_height;

	/*
	 * If the positioning puts the selection bitmap off one of the edges,
	 * adjust it so it is completely on the grid.
	 */
	if (sel->x + sel->width > grid->grid_width)
		sel->x -= (sel->x + sel->width) - grid->grid_width;
	if (sel->y + sel->height > grid->grid_height)
		sel->y -= (sel->y + sel->height) - grid->grid_height;

	bpr = ((sel->width * grid->bpp) + 7) >> 3;
	bytes = (bpr * sel->height) << 1;

	/*
	 * Resize the storage for the selection bitmap if necessary.
	 */
	if (bytes > grid->sel.bytes) {
		if (grid->sel.bytes == 0)
			grid->sel.bitmap = (unsigned char *) malloc(bytes);
		else
			grid->sel.bitmap = (unsigned char *)
			                   realloc((char *) grid->sel.bitmap, bytes);
		grid->sel.bytes = bytes;
	}

	/*
	 * Copy the width and height values.
	 */
	grid->sel.x = sel->x;
	grid->sel.y = sel->y;
	grid->sel.width = sel->width;
	grid->sel.height = sel->height;

	/*
	 * Copy the incoming bitmap to the new selection bitmap.
	 */
	(void) memcpy((char *) grid->sel.bitmap, (char *) sel->bitmap,
	              bytes >> 1);

	/*
	 * Crop the image to adjust the glyph bounding box.
	 */
	(void) bdf_grid_crop(grid, 1);
}

int
#ifdef __STDC__
bdf_grid_color_at(bdf_glyph_grid_t *grid, short x, short y)
#else
bdf_grid_color_at(grid, x, y)
bdf_glyph_grid_t *grid;
short x, y;
#endif
{
	unsigned short bpr, si, di, byte;
	unsigned char *masks;

	if (grid->bpp == 1)
		return -1;

	masks = twobpp;
	di = 0;
	switch (grid->bpp) {
	case 2:
		di = 3;
		break;
	case 4:
		di = 1;
		break;
	}

	x *= grid->bpp;

	bpr = ((grid->grid_width * grid->bpp) + 7) >> 3;
	si = (x & 7) / grid->bpp;

	byte = grid->bitmap[(y * bpr) + (x >> 3)] & masks[si];
	if (di > si)
		byte >>= (di - si) * grid->bpp;
	return (int) byte;
}
