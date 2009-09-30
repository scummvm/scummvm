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

#include <time.h>	// for time() to seed rand() via srand()
#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"
#include "sci/sci.h"	// for kDebugLevelSci0Pic

namespace Sci {

#define GFXR_PIC0_PALETTE_SIZE 40
#define GFXR_PIC0_NUM_PALETTES 4

#define INTERCOL(a, b) ((int) sqrt((((3.3 * (a))*(a)) + ((1.7 * (b))*(b))) / 5.0))
// Macro for color interpolation

#define SCI0_MAX_PALETTE 2

int sci0_palette = 0;

// Copied from include/kernel.h
#define SCI0_PRIORITY_BAND_FIRST_14_ZONES(nr) ((((nr) == 0)? 0 :  \
	((first) + (((nr)-1) * (last - first)) / 14)))

// Default color maps
PaletteEntry gfx_sci0_image_colors[SCI0_MAX_PALETTE+1][GFX_SCI0_IMAGE_COLORS_NR] = {
	{PaletteEntry(0x00, 0x00, 0x00), PaletteEntry(0x00, 0x00, 0xaa),
		PaletteEntry(0x00, 0xaa, 0x00), PaletteEntry(0x00, 0xaa, 0xaa),
		PaletteEntry(0xaa, 0x00, 0x00), PaletteEntry(0xaa, 0x00, 0xaa),
		PaletteEntry(0xaa, 0x55, 0x00), PaletteEntry(0xaa, 0xaa, 0xaa),
		PaletteEntry(0x55, 0x55, 0x55), PaletteEntry(0x55, 0x55, 0xff),
		PaletteEntry(0x55, 0xff, 0x55), PaletteEntry(0x55, 0xff, 0xff),
		PaletteEntry(0xff, 0x55, 0x55), PaletteEntry(0xff, 0x55, 0xff),
		PaletteEntry(0xff, 0xff, 0x55), PaletteEntry(0xff, 0xff, 0xff)}, // "Normal" EGA


	{PaletteEntry(0x00, 0x00, 0x00), PaletteEntry(0x00, 0x00, 0xff),
		PaletteEntry(0x00, 0xaa, 0x00), PaletteEntry(0x00, 0xaa, 0xaa),
		PaletteEntry(0xce, 0x00, 0x00), PaletteEntry(0xbe, 0x71, 0xde),
		PaletteEntry(0x8d, 0x50, 0x00), PaletteEntry(0xbe, 0xbe, 0xbe),
		PaletteEntry(0x55, 0x55, 0x55), PaletteEntry(0x00, 0xbe, 0xff),
		PaletteEntry(0x00, 0xce, 0x55), PaletteEntry(0x55, 0xff, 0xff),
		PaletteEntry(0xff, 0x9d, 0x8d), PaletteEntry(0xff, 0x55, 0xff),
		PaletteEntry(0xff, 0xff, 0x00), PaletteEntry(0xff, 0xff, 0xff)}, // AGI Amiga-ish

// RGB and I intensities (former taken from the GIMP)
#define GR 30
#define GG 59
#define GB 11
#define GI 15

#define FULL (GR+GG+GB+GI)

#define CC(x) (((x)*255)/FULL),(((x)*255)/FULL),(((x)*255)/FULL)         // Combines color intensities

	{PaletteEntry(CC(0)           ), PaletteEntry(CC(GB)          ),
		PaletteEntry(CC(GG)          ), PaletteEntry(CC(GB + GG)       ),
		PaletteEntry(CC(GR)          ), PaletteEntry(CC(GB + GR)       ),
		PaletteEntry(CC(GG + GR)       ), PaletteEntry(CC(GB + GG + GR)    ),
		PaletteEntry(CC(GI)          ), PaletteEntry(CC(GB + GI)       ),
		PaletteEntry(CC(GG + GI)       ), PaletteEntry(CC(GB + GG + GI)    ),
		PaletteEntry(CC(GR + GI)       ), PaletteEntry(CC(GB + GR + GI)    ),
		PaletteEntry(CC(GG + GR + GI)    ), PaletteEntry(CC(GB + GG + GR + GI) )}
}; // Grayscale

#undef GR
#undef GG
#undef GB
#undef GI

#undef FULL

#undef C2
#undef C3
#undef C4

Palette* gfx_sci0_pic_colors = 0; // Initialized during initialization
Palette* gfx_sci0_image_pal[SCI0_MAX_PALETTE+1];
Palette* embedded_view_pal = 0;

#define SCI1_PALETTE_SIZE 1284

void gfxr_init_static_palette() {
	int i;

	if (!gfx_sci0_pic_colors) {
		gfx_sci0_pic_colors = new Palette(256);
		gfx_sci0_pic_colors->name = "gfx_sci0_pic_colors";

		// This 256 colour palette is used for the kDitherNone and kDither256Colors
		// modes. We set index 0xXY to a blend of EGA colors X^Y and Y. We permute them
		// in this somewhat strange way to ensure the regular EGA colours are at
		// indices 0-15.
		for (i = 0; i < 256; i++) {
			byte r = INTERCOL(gfx_sci0_image_colors[sci0_palette][i & 0xf].r,
			                  gfx_sci0_image_colors[sci0_palette][(i >> 4) ^ (i & 0xf)].r);
			byte g = INTERCOL(gfx_sci0_image_colors[sci0_palette][i & 0xf].g,
			                  gfx_sci0_image_colors[sci0_palette][(i >> 4) ^ (i & 0xf)].g);
			byte b = INTERCOL(gfx_sci0_image_colors[sci0_palette][i & 0xf].b,
			                  gfx_sci0_image_colors[sci0_palette][(i >> 4) ^ (i & 0xf)].b);
			gfx_sci0_pic_colors->setColor(i,r,g,b);
		}
		//warning("Uncomment me after fixing sci0_palette changes to reset me");
		//_gfxr_pic0_colors_initialized = 1;

		for (i = 0; i <= SCI0_MAX_PALETTE; ++i) {
			gfx_sci0_image_pal[i] = new Palette(gfx_sci0_image_colors[i], GFX_SCI0_IMAGE_COLORS_NR);
			gfx_sci0_image_pal[i]->name = "gfx_sci0_image_pal[i]";
		}

		embedded_view_pal = new Palette(16);
		for (i = 0; i < 16; i++)
			embedded_view_pal->setColor(i, 0, 0, i * 0x11);
		embedded_view_pal->name = "embedded_view_pal";
	}
}


gfxr_pic_t *gfxr_init_pic(gfx_mode_t *mode, int ID, bool sci1) {
	gfxr_pic_t *pic = (gfxr_pic_t*)malloc(sizeof(gfxr_pic_t));

	pic->mode = mode;

	pic->control_map = gfx_pixmap_alloc_index_data(gfx_new_pixmap(320, 200, ID, 2, 0));

	pic->priority_map = gfx_pixmap_alloc_index_data(gfx_new_pixmap(mode->scaleFactor * 320, mode->scaleFactor * 200,
	                    ID, 1, 0));


	pic->visual_map = gfx_pixmap_alloc_index_data(gfx_new_pixmap(320 * mode->scaleFactor,
	                  200 * mode->scaleFactor, ID, 0, 0));

	// Initialize colors
	if (!sci1) {
		pic->ID = ID;
		gfxr_init_static_palette();
	}

	pic->visual_map->palette = gfx_sci0_pic_colors->getref();
	pic->visual_map->color_key = GFX_PIXMAP_COLOR_KEY_NONE;

	pic->visual_map->flags = 0;
	pic->priority_map->flags = 0;
	pic->control_map->flags = 0;
	if (mode->scaleFactor > 1) {
		pic->visual_map->flags |= GFX_PIXMAP_FLAG_SCALED_INDEX;
		pic->priority_map->flags |= GFX_PIXMAP_FLAG_SCALED_INDEX;
	}

	pic->priority_map->palette = gfx_sci0_image_pal[sci0_palette]->getref();
	pic->control_map->palette = gfx_sci0_image_pal[sci0_palette]->getref();

	pic->undithered_buffer_size = pic->visual_map->index_width * pic->visual_map->index_height;
	pic->undithered_buffer = NULL;
	pic->priorityTable = NULL;

	return pic;
}

// Pic rendering operations

void gfxr_clear_pic0(gfxr_pic_t *pic, int titlebar_size) {
	memset(pic->visual_map->index_data, 0x00, (320 * pic->mode->scaleFactor * titlebar_size * pic->mode->scaleFactor));
	memset(pic->visual_map->index_data + (320 * pic->mode->scaleFactor * titlebar_size * pic->mode->scaleFactor),
	       0xff, pic->mode->scaleFactor * 320 * pic->mode->scaleFactor * (200 - titlebar_size)); // white
	memset(pic->priority_map->index_data + (320 * pic->mode->scaleFactor * titlebar_size * pic->mode->scaleFactor),
	       0x0, pic->mode->scaleFactor * 320 * pic->mode->scaleFactor * (200 - titlebar_size));
	memset(pic->priority_map->index_data, 0x0a, titlebar_size * (pic->mode->scaleFactor * 320 * pic->mode->scaleFactor));
	memset(pic->control_map->index_data, 0, GFXR_AUX_MAP_SIZE);
	memset(pic->aux_map, 0, GFXR_AUX_MAP_SIZE);
}


//** Basic operations on the auxiliary buffer **

#define FRESH_PAINT 0x40
// freshly filled or near to something that is

#define LINEMACRO(startx, starty, deltalinear, deltanonlinear, linearvar, nonlinearvar, \
                  linearend, nonlinearstart, linearmod, nonlinearmod, operation) \
	x = (startx); y = (starty); \
	incrNE = ((deltalinear) > 0)? (deltalinear) : -(deltalinear); \
	incrNE <<= 1; \
	deltanonlinear <<= 1; \
	incrE = ((deltanonlinear) > 0) ? -(deltanonlinear) : (deltanonlinear);  \
	d = nonlinearstart-1;  \
	while (linearvar != (linearend)) { \
		buffer[linewidth * y + x] operation color; \
/* color ^= color2; color2 ^= color; color ^= color2; */ /* Swap colors */ \
		linearvar += linearmod; \
		if ((d+=incrE) < 0) { \
			d += incrNE; \
			nonlinearvar += nonlinearmod; \
		}; \
	}; \
	buffer[linewidth * y + x] operation color;

static void _gfxr_auxbuf_line_draw(gfxr_pic_t *pic, rect_t line, int color, int color2, int titlebar_size) {
	int dx, dy, incrE, incrNE, d, finalx, finaly;
	int x = line.x;
	int y = line.y + titlebar_size;
	unsigned char *buffer = pic->aux_map;
	int linewidth = 320;

	dx = line.width;
	dy = line.height;
	finalx = x + dx;
	finaly = y + dy;

	dx = abs(dx);
	dy = abs(dy);

	if (dx > dy) {
		if (finalx < x) {
			if (finaly < y) { // llu == left-left-up
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, -1, -1, |=);
			} else {         // lld
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, -1, 1, |=);
			}
		} else { // x1 >= x
			if (finaly < y) { // rru
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, 1, -1, |=);
			} else {         // rrd
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, 1, 1, |=);
			}
		}
	} else { // dx <= dy
		if (finaly < y) {
			if (finalx < x) { // luu
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, -1, -1, |=);
			} else {         // ruu
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, -1, 1, |=);
			}
		} else { // y1 >= y
			if (finalx < x) { // ldd
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, 1, -1, |=);
			} else {         // rdd
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, 1, 1, |=);
			}
		}
	}
}

static void _gfxr_auxbuf_line_clear(gfxr_pic_t *pic, rect_t line, int color, int titlebar_size) {
	int dx, dy, incrE, incrNE, d, finalx, finaly;
	int x = line.x;
	int y = line.y + titlebar_size;
	unsigned char *buffer = pic->aux_map;
	int linewidth = 320;

	dx = line.width;
	dy = line.height;
	finalx = x + dx;
	finaly = y + dy;

	dx = abs(dx);
	dy = abs(dy);

	if (dx > dy) {
		if (finalx < x) {
			if (finaly < y) { // llu == left-left-up
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, -1, -1, &=);
			} else {         // lld
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, -1, 1, &=);
			}
		} else { // x1 >= x
			if (finaly < y) { // rru
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, 1, -1, &=);
			} else {         // rrd
				LINEMACRO(x, y, dx, dy, x, y, finalx, dx, 1, 1, &=);
			}
		}
	} else { // dx <= dy
		if (finaly < y) {
			if (finalx < x) { // luu
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, -1, -1, &=);
			} else {         // ruu
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, -1, 1, &=);
			}
		} else { // y1 >= y
			if (finalx < x) { // ldd
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, 1, -1, &=);
			} else {         // rdd
				LINEMACRO(x, y, dy, dx, y, x, finaly, dy, 1, 1, &=);
			}
		}
	}
}

#undef LINEMACRO

#ifdef WITH_PIC_SCALING
static void _gfxr_auxbuf_propagate_changes(gfxr_pic_t *pic, int bitmask) {
	// Propagates all filled bits into the planes described by bitmask
	unsigned long *data = (unsigned long *)pic->aux_map;
	unsigned long clearmask = 0x07070707;
	unsigned long andmask = (bitmask << 3) | (bitmask << (3 + 8)) | (bitmask << (3 + 16)) | (bitmask << (3 + 24));

	if (sizeof(unsigned long) == 8) { // UltraSparc, Alpha, newer MIPSens, etc
		andmask |= (andmask << 32);
		clearmask |= (clearmask << 32);
	}

	for (int i = 0; i < GFXR_AUX_MAP_SIZE / sizeof(unsigned long); i++) {
		unsigned long temp = *data & andmask;
		temp >>= 3;
		*data = (temp | *data) & clearmask;
		++data;
	}
}
#endif


/*** Regular drawing operations ***/

#define PATTERN_FLAG_RECTANGLE 0x10
#define PATTERN_FLAG_USE_PATTERN 0x20

#define PIC_OP_FIRST 0xf0

enum {
	PIC_OP_SET_COLOR = 0xf0,
	PIC_OP_DISABLE_VISUAL = 0xf1,
	PIC_OP_SET_PRIORITY = 0xf2,
	PIC_OP_DISABLE_PRIORITY = 0xf3,
	PIC_OP_SHORT_PATTERNS = 0xf4,
	PIC_OP_MEDIUM_LINES = 0xf5,
	PIC_OP_LONG_LINES = 0xf6,
	PIC_OP_SHORT_LINES = 0xf7,
	PIC_OP_FILL = 0xf8,
	PIC_OP_SET_PATTERN = 0xf9,
	PIC_OP_ABSOLUTE_PATTERN = 0xfa,
	PIC_OP_SET_CONTROL = 0xfb,
	PIC_OP_DISABLE_CONTROL = 0xfc,
	PIC_OP_MEDIUM_PATTERNS = 0xfd,
	PIC_OP_OPX = 0xfe,
	PIC_OP_TERMINATE = 0xff
};

enum {
	PIC_SCI0_OPX_SET_PALETTE_ENTRIES = 0,
	PIC_SCI0_OPX_SET_PALETTE = 1,
	PIC_SCI0_OPX_MONO0 = 2,
	PIC_SCI0_OPX_MONO1 = 3,
	PIC_SCI0_OPX_MONO2 = 4,
	PIC_SCI0_OPX_MONO3 = 5,
	PIC_SCI0_OPX_MONO4 = 6,
	PIC_SCI0_OPX_EMBEDDED_VIEW,
	PIC_SCI0_OPX_SET_PRIORITY_TABLE
};

// We use this so we can keep OPX handling in one switch.
// We simply add this constant to the op number if we're running an SCI1 game,
// and offset the OPX constants below correspondingly.
#define SCI1_OP_OFFSET 42

enum {
	PIC_SCI1_OPX_SET_PALETTE_ENTRIES = 0 + SCI1_OP_OFFSET,
	PIC_SCI1_OPX_EMBEDDED_VIEW = 1 + SCI1_OP_OFFSET,
	PIC_SCI1_OPX_SET_PALETTE = 2 + SCI1_OP_OFFSET,
	PIC_SCI1_OPX_PRIORITY_TABLE_EQDIST = 3 + SCI1_OP_OFFSET,
	PIC_SCI1_OPX_PRIORITY_TABLE_EXPLICIT = 4 + SCI1_OP_OFFSET
};


enum {
	ELLIPSE_SOLID, // Normal filled ellipse
	ELLIPSE_OR     // color ORred to the buffer
};

static void _gfxr_fill_ellipse(gfxr_pic_t *pic, byte *buffer, int linewidth, int x, int y,
		int rad_x, int rad_y, int color, int fillstyle) {
	int xx = 0, yy = rad_y;
	int i, x_i, y_i;
	int xr = 2 * rad_x * rad_x;
	int yr = 2 * rad_y * rad_y;

	x_i = 1;
	y_i = xr * rad_y - 1;
	i = y_i >> 1;

	while (yy >= 0) {
		int oldxx = xx;
		int oldyy = yy;

		if (i >= 0) {
			x_i += yr;
			i -= x_i + 1;
			++xx;
		}

		if (i < 0) {
			y_i -= xr;
			i += y_i - 1;
			--yy;
		}

		if (oldyy != yy) {
			int j;
			int offset0 = (y - oldyy) * linewidth;
			int offset1 = (y + oldyy) * linewidth;

			offset0 += x - oldxx;
			offset1 += x - oldxx;

			if (oldyy == 0)
				offset1 = 0; // We never have to draw ellipses in the menu bar

			oldyy = yy;

			switch (fillstyle) {

			case ELLIPSE_SOLID:
				memset(buffer + offset0, color, (oldxx << 1) + 1);
				if (offset1)
					memset(buffer + offset1, color, (oldxx << 1) + 1);
				break;

			case ELLIPSE_OR:
				for (j = 0; j < (oldxx << 1) + 1; j++) {
					buffer[offset0 + j] |= color;
					if (offset1)
						buffer[offset1 + j] |= color;
				}
				break;

			default:
				warning(" to %s,%d", __FILE__, __LINE__);
				return;

			}
		}
	}
}

static void _gfxr_auxplot_brush(gfxr_pic_t *pic, byte *buffer, int yoffset, int offset, int plot,
	int color, gfx_brush_mode_t brush_mode, int randseed) {
	// yoffset 63680, offset 320, plot 1, color 34, brush_mode 0, randseed 432)*/
	// Auxplot: Used by plot_aux_pattern to plot to visual and priority
	int xc, yc;
	int line_width = 320 * pic->mode->scaleFactor;
	int full_offset = (yoffset * pic->mode->scaleFactor + offset) * pic->mode->scaleFactor;

	if (yoffset + offset >= 64000) {
		error("_gfxr_auxplot_brush() failed. Breakpoint in %s, line %d", __FILE__, __LINE__);
	}

	switch (brush_mode) {
	case GFX_BRUSH_MODE_SCALED:
		if (plot)
			for (yc = 0; yc < pic->mode->scaleFactor; yc++) {
				memset(buffer + full_offset, color, pic->mode->scaleFactor);
				full_offset += line_width;
			}
		break;

	case GFX_BRUSH_MODE_ELLIPSES:
		if (plot) {
			int x = offset * pic->mode->scaleFactor + ((pic->mode->scaleFactor - 1) >> 1);
			int y = (yoffset / 320) * pic->mode->scaleFactor + ((pic->mode->scaleFactor - 1) >> 1);

			_gfxr_fill_ellipse(pic, buffer, line_width, x, y, pic->mode->scaleFactor >> 1, pic->mode->scaleFactor >> 1, color, ELLIPSE_SOLID);
		}
		break;

	case GFX_BRUSH_MODE_RANDOM_ELLIPSES:
		if (plot) {
			int x = offset * pic->mode->scaleFactor + ((pic->mode->scaleFactor - 1) >> 1);
			int y = (yoffset / 320) * pic->mode->scaleFactor + ((pic->mode->scaleFactor - 1) >> 1);
			int sizex = pic->mode->scaleFactor >> 1;
			int sizey = pic->mode->scaleFactor >> 1;

			srand(randseed);

			x -= (int)((sizex * rand() * 1.0) / (RAND_MAX + 1.0));
			x += (int)((sizex * rand() * 1.0) / (RAND_MAX + 1.0));
			y -= (int)((sizey * rand() * 1.0) / (RAND_MAX + 1.0));
			y += (int)((sizey * rand() * 1.0) / (RAND_MAX + 1.0));
			sizex = (int)((sizex * rand() * 1.0) / (RAND_MAX + 1.0));
			sizey = (int)((sizey * rand() * 1.0) / (RAND_MAX + 1.0));

			_gfxr_fill_ellipse(pic, buffer, line_width, x, y, pic->mode->scaleFactor >> 1, pic->mode->scaleFactor >> 1,
			                   color, ELLIPSE_SOLID);
			srand(time(NULL)); // Make sure we don't accidently forget to re-init the random number generator
		}
		break;

	case GFX_BRUSH_MODE_MORERANDOM: {
		int mask = plot ? 7 : 1;
		srand(randseed);
		for (yc = 0; yc < pic->mode->scaleFactor; yc++) {
			for (xc = 0; xc < pic->mode->scaleFactor; xc++)
				if ((rand() & 7) < mask)
					buffer[full_offset + xc] = color;
			full_offset += line_width;
		}
		srand(time(NULL)); // Make sure we don't accidently forget to re-init the random number generator
	}
	break;
	}
}

#define PLOT_AUX_PATTERN_NO_RANDOM -1

static void _gfxr_plot_aux_pattern(gfxr_pic_t *pic, int x, int y, int size, int circle, int random,
	int mask, int color, int priority, int control, gfx_brush_mode_t brush_mode, int map_nr) {
	// Plots an appropriate pattern to the aux buffer and the control buffer,
	// if mask & GFX_MASK_CONTROL
	// random should be set to the random index, or -1 to disable

	// These circle offsets uniquely identify the circles used by Sierra:
	const int circle_data[][8] = {
		{0},
		{1, 0},
		{2, 2, 1},
		{3, 3, 2, 1},
		{4, 4, 4, 3, 1},
		{5, 5, 4, 4, 3, 1},
		{6, 6, 6, 5, 5, 4, 2},
		{7, 7, 7, 6, 6, 5, 4, 2}
	};

	// 'Random' fill patterns, provided by Carl Muckenhoupt:
	const byte random_data[32] = {
		0x20, 0x94, 0x02, 0x24, 0x90, 0x82, 0xa4, 0xa2, 0x82, 0x09, 0x0a, 0x22,
		0x12, 0x10, 0x42, 0x14, 0x91, 0x4a, 0x91, 0x11, 0x08, 0x12, 0x25, 0x10,
		0x22, 0xa8, 0x14, 0x24, 0x00, 0x50, 0x24, 0x04
	};

	// 'Random' fill offsets, provided by Carl Muckenhoupt:
	const byte random_offset[128] = {
		0x00, 0x18, 0x30, 0xc4, 0xdc, 0x65, 0xeb, 0x48,
		0x60, 0xbd, 0x89, 0x05, 0x0a, 0xf4, 0x7d, 0x7d,
		0x85, 0xb0, 0x8e, 0x95, 0x1f, 0x22, 0x0d, 0xdf,
		0x2a, 0x78, 0xd5, 0x73, 0x1c, 0xb4, 0x40, 0xa1,
		0xb9, 0x3c, 0xca, 0x58, 0x92, 0x34, 0xcc, 0xce,
		0xd7, 0x42, 0x90, 0x0f, 0x8b, 0x7f, 0x32, 0xed,
		0x5c, 0x9d, 0xc8, 0x99, 0xad, 0x4e, 0x56, 0xa6,
		0xf7, 0x68, 0xb7, 0x25, 0x82, 0x37, 0x3a, 0x51,
		0x69, 0x26, 0x38, 0x52, 0x9e, 0x9a, 0x4f, 0xa7,
		0x43, 0x10, 0x80, 0xee, 0x3d, 0x59, 0x35, 0xcf,
		0x79, 0x74, 0xb5, 0xa2, 0xb1, 0x96, 0x23, 0xe0,
		0xbe, 0x05, 0xf5, 0x6e, 0x19, 0xc5, 0x66, 0x49,
		0xf0, 0xd1, 0x54, 0xa9, 0x70, 0x4b, 0xa4, 0xe2,
		0xe6, 0xe5, 0xab, 0xe4, 0xd2, 0xaa, 0x4c, 0xe3,
		0x06, 0x6f, 0xc6, 0x4a, 0xa4, 0x75, 0x97, 0xe1
	};

	int offset = 0, width = 0;
	int yoffset = (y - size) * 320;
	int i;
	int random_index = 0;
	gfx_pixmap_t *map = NULL;

	switch (map_nr) {
	case GFX_MASK_VISUAL:
		map = pic->visual_map;
		break;
	case GFX_MASK_PRIORITY:
		map = pic->priority_map;
		break;
	default:
		map = pic->control_map;
		break;
	}

	if (random >= 0)
		random_index = random_offset[random];

	if (!circle) {
		offset = -size;
		width = (size << 1) + 2;
	}

	for (i = -size; i <= size; i++) {
		int j;
		int height;

		if (circle) {
			offset = circle_data[size][abs(i)];
			height = width = (offset << 1) + 1;
			offset = -offset;
		} else
			height = width - 1;

		if (random == PLOT_AUX_PATTERN_NO_RANDOM) {

			if (mask & map_nr)
				memset(map->index_data + yoffset + offset + x, control, width);

			if (map_nr == GFX_MASK_CONTROL)
				for (j = x; j < x + width; j++)
					pic->aux_map[yoffset + offset + j] |= mask;

		} else { // Semi-Random!
			for (j = 0; j < height; j++) {
				if (random_data[random_index >> 3] & (0x80 >> (random_index & 7))) {
					// The 'seemingly' random decision
					if (mask & GFX_MASK_CONTROL)
						pic->control_map->index_data[yoffset + x + offset + j] = control;

					pic->aux_map[yoffset + x + offset + j] |= mask;

					if (mask & GFX_MASK_VISUAL)
						_gfxr_auxplot_brush(pic, pic->visual_map->index_data, yoffset, x + offset + j,
						                    1, color, brush_mode, random_index + x);

					if (mask & GFX_MASK_PRIORITY)
						_gfxr_auxplot_brush(pic, pic->priority_map->index_data, yoffset, x + offset + j,
						                    1, priority, brush_mode, random_index + x);

				} else {
					if (mask & GFX_MASK_VISUAL)
						_gfxr_auxplot_brush(pic, pic->visual_map->index_data, yoffset, x + offset + j,
						                    0, color, brush_mode, random_index + x);

					if (mask & GFX_MASK_PRIORITY)
						_gfxr_auxplot_brush(pic, pic->priority_map->index_data, yoffset, x + offset + j,
						                    0, priority, brush_mode, random_index + x);
				}
				random_index = (random_index + 1) & 0xff;
			}
		}
		yoffset += 320;
	}
}

static void _gfxr_draw_pattern(gfxr_pic_t *pic, int x, int y, int color, int priority, int control, int drawenable,
	int pattern_code, int pattern_size, int pattern_nr, gfx_brush_mode_t brush_mode, int titlebar_size) {
	int xsize = (pattern_size + 1) * pic->mode->scaleFactor - 1;
	int ysize = (pattern_size + 1) * pic->mode->scaleFactor - 1;
	int scaled_x, scaled_y;
	rect_t boundaries;
	int max_x = (pattern_code & PATTERN_FLAG_RECTANGLE) ? 318 : 319; // Rectangles' width is size+1

	debugC(2, kDebugLevelSci0Pic, "Pattern at (%d,%d) size %d, rand=%d, code=%02x\n", x, y, pattern_size, pattern_nr, pattern_code);

	y += titlebar_size;

	if (x - pattern_size < 0)
		x = pattern_size;

	if (y - pattern_size < titlebar_size)
		y = titlebar_size + pattern_size;

	if (x + pattern_size > max_x)
		x = max_x - pattern_size;

	if (y + pattern_size > 199)
		y = 199 - pattern_size;

	scaled_x = x * pic->mode->scaleFactor + ((pic->mode->scaleFactor - 1) >> 1);
	scaled_y = y * pic->mode->scaleFactor + ((pic->mode->scaleFactor - 1) >> 1);

	if (scaled_x < xsize)
		scaled_x = xsize;

	if (scaled_y < ysize + titlebar_size * pic->mode->scaleFactor)
		scaled_y = ysize + titlebar_size * pic->mode->scaleFactor;

	if (scaled_x > (320 * pic->mode->scaleFactor) - 1 - xsize)
		scaled_x = (320 * pic->mode->scaleFactor) - 1 - xsize;

	if (scaled_y > (200 * pic->mode->scaleFactor) - 1 - ysize)
		scaled_y = (200 * pic->mode->scaleFactor) - 1 - ysize;

	if (pattern_code & PATTERN_FLAG_RECTANGLE) {
		// Rectangle
		boundaries.x = scaled_x - xsize;
		boundaries.y = scaled_y - ysize;
		boundaries.width = ((xsize + 1) << 1) + 1;
		boundaries.height = (ysize << 1) + 1;

		if (pattern_code & PATTERN_FLAG_USE_PATTERN) {
			_gfxr_plot_aux_pattern(pic, x, y, pattern_size, 0, pattern_nr, drawenable, color, priority,
			                       control, brush_mode, GFX_MASK_CONTROL);
		} else {
			_gfxr_plot_aux_pattern(pic, x, y, pattern_size, 0, PLOT_AUX_PATTERN_NO_RANDOM, drawenable, 0, 0, control,
			                       GFX_BRUSH_MODE_SCALED, GFX_MASK_CONTROL);

			if (drawenable & GFX_MASK_VISUAL)
				gfx_draw_box_pixmap_i(pic->visual_map, boundaries, color);

			if (drawenable & GFX_MASK_PRIORITY)
				gfx_draw_box_pixmap_i(pic->priority_map, boundaries, priority);
		}

	} else {
		// Circle

		if (pattern_code & PATTERN_FLAG_USE_PATTERN) {

			_gfxr_plot_aux_pattern(pic, x, y, pattern_size, 1, pattern_nr, drawenable, color, priority,
			                       control, brush_mode, GFX_MASK_CONTROL);
		} else {
			_gfxr_plot_aux_pattern(pic, x, y, pattern_size, 1, PLOT_AUX_PATTERN_NO_RANDOM,
			                       drawenable, 0, 0, control, GFX_BRUSH_MODE_SCALED, GFX_MASK_CONTROL);

			if (pic->mode->scaleFactor == 1 && pic->mode->scaleFactor == 1) {
				if (drawenable & GFX_MASK_VISUAL)
					_gfxr_plot_aux_pattern(pic, x, y, pattern_size, 1, PLOT_AUX_PATTERN_NO_RANDOM,
					                       drawenable, 0, 0, color, GFX_BRUSH_MODE_SCALED, GFX_MASK_VISUAL);

				if (drawenable & GFX_MASK_PRIORITY)
					_gfxr_plot_aux_pattern(pic, x, y, pattern_size, 1, PLOT_AUX_PATTERN_NO_RANDOM,
					                       drawenable, 0, 0, priority, GFX_BRUSH_MODE_SCALED, GFX_MASK_PRIORITY);
			} else {
				if (drawenable & GFX_MASK_VISUAL)
					_gfxr_fill_ellipse(pic, pic->visual_map->index_data, 320 * pic->mode->scaleFactor,
					                   scaled_x, scaled_y, xsize, ysize, color, ELLIPSE_SOLID);

				if (drawenable & GFX_MASK_PRIORITY)
					_gfxr_fill_ellipse(pic, pic->priority_map->index_data, 320 * pic->mode->scaleFactor,
					                   scaled_x, scaled_y, xsize, ysize, priority, ELLIPSE_SOLID);
			}
		}
	}
}

static void _gfxr_draw_subline(gfxr_pic_t *pic, int x, int y, int ex, int ey, int color, int priority, int drawenable) {
	Common::Point start;
	Common::Point end;

	start.x = x;
	start.y = y;
	end.x = ex;
	end.y = ey;

	if (ex >= pic->visual_map->index_width || ey >= pic->visual_map->index_height || x < 0 || y < 0) {
		warning("While drawing pic0: INVALID LINE %d,%d,%d,%d",
		        start.x, start.y, end.x, end.y);
		return;
	}

	if (drawenable & GFX_MASK_VISUAL)
		gfx_draw_line_pixmap_i(pic->visual_map, start, end, color);

	if (drawenable & GFX_MASK_PRIORITY)
		gfx_draw_line_pixmap_i(pic->priority_map, start, end, priority);

}

static void _gfxr_draw_line(gfxr_pic_t *pic, int x, int y, int ex, int ey, int color,
	int priority, int control, int drawenable, int line_mode, int cmd, int titlebar_size) {
	int scale_x = pic->mode->scaleFactor;
	int scale_y = pic->mode->scaleFactor;
	int xc, yc;
	rect_t line;
	int mask;
	int partially_white = (drawenable & GFX_MASK_VISUAL) && (((color & 0xf0) == 0xf0) || ((color & 0x0f) == 0x0f));

	line.x = x;
	line.y = y;
	line.width = ex - x;
	line.height = ey - y;

	if (x > 319 || y > 199 || x < 0 || y < 0 || ex > 319 || ey > 199 || ex < 0 || ey < 0) {
		warning("[GFX] While building pic: Attempt to draw line (%d,%d) to (%d,%d): cmd was %d", x, y, ex, ey, cmd);
		return;
	}

	y += titlebar_size;
	ey += titlebar_size;

	if (drawenable & GFX_MASK_CONTROL) {
		debugC(2, kDebugLevelSci0Pic, " ctl:%x", control);
		gfx_draw_line_pixmap_i(pic->control_map, Common::Point(x, y), Common::Point(x + line.width, y + line.height), control);
	}

	// Calculate everything that is changed to SOLID
	mask = drawenable & (((color != 0xff) ? 1 : 0) | ((priority) ? 2 : 0) | ((control) ? 4 : 0));

	if (mask) {
		int mask2 = mask;
		if (partially_white)
			mask2 = mask &= ~GFX_MASK_VISUAL;
		_gfxr_auxbuf_line_draw(pic, line, mask, mask2, titlebar_size);
	}

	// Calculate everything that is changed to TRANSPARENT
	mask = drawenable & (((color == 0xff) ? 1 : 0) | ((!priority) ? 2 : 0) | ((!control) ? 4 : 0));

	if (mask)
		_gfxr_auxbuf_line_clear(pic, line, ~mask, titlebar_size);

	x *= scale_x;
	y *= scale_y;
	ex *= scale_x;
	ey *= scale_y;

	if (drawenable & GFX_MASK_VISUAL)
		debugC(2, kDebugLevelSci0Pic, " col:%02x", color);

	if (drawenable & GFX_MASK_PRIORITY)
		debugC(2, kDebugLevelSci0Pic, " pri:%x", priority);

	if (line_mode == GFX_LINE_MODE_FINE) {  // Adjust lines to extend over the full visual
		x = (x * ((320 + 1) * scale_x - 1)) / (320 * scale_x);
		y = (y * ((200 + 1) * scale_y - 1)) / (200 * scale_y);
		ex = (ex * ((320 + 1) * scale_x - 1)) / (320 * scale_x);
		ey = (ey * ((200 + 1) * scale_y - 1)) / (200 * scale_y);

		_gfxr_draw_subline(pic, x, y, ex, ey, color, priority, drawenable);
	} else {
		if (x == ex && y == ey) { // Just one single point?
			rect_t drawrect;
			drawrect.x = x;
			drawrect.y = y;
			drawrect.width = scale_x;
			drawrect.height = scale_y;

			if (drawenable & GFX_MASK_VISUAL)
				gfx_draw_box_pixmap_i(pic->visual_map, drawrect, color);

			if (drawenable & GFX_MASK_PRIORITY)
				gfx_draw_box_pixmap_i(pic->priority_map, drawrect, priority);

		} else {
			int width = scale_x;
			int height = scale_y;
			int x_offset = 0;
			int y_offset = 0;

			if (line_mode == GFX_LINE_MODE_FAST) {
				width = (width + 1) >> 1;
				height = (height + 1) >> 1;
				x_offset = (width >> 1);
				y_offset = (height >> 1);
			}

			for (xc = 0; xc < width; xc++)
				_gfxr_draw_subline(pic, x + xc + x_offset, y + y_offset, ex + xc + x_offset, ey + y_offset,
				                   color, priority, drawenable);

			if (height > 0)
				for (xc = 0; xc < width; xc++)
					_gfxr_draw_subline(pic, x + xc + x_offset, y + height - 1 + y_offset,
					                   ex + xc + x_offset, ey + height - 1 + y_offset, color, priority, drawenable);

			if (height > 1) {
				for (yc = 1; yc < height - 1; yc++)
					_gfxr_draw_subline(pic, x + x_offset, y + yc + y_offset, ex + x_offset, ey + yc + y_offset,
					                   color, priority, drawenable);
				if (width > 0)
					for (yc = 1; yc < height - 1; yc++)
						_gfxr_draw_subline(pic, x + width - 1 + x_offset, y + yc + y_offset,
						                   ex + width - 1 + x_offset, ey + yc + y_offset, color, priority, drawenable);
			}
		}
	}

	debugC(2, kDebugLevelSci0Pic, "\n");
}


#define IS_FILL_BOUNDARY(x) (((x) & legalmask) != legalcolor)

#ifdef WITH_PIC_SCALING

#define TEST_POINT(xx, yy) \
	if (pic->aux_map[(yy) * 320 + (xx)] & FRESH_PAINT) { \
		mpos = (((yy) * 320 * pic->mode->scaleFactor) + (xx)) * pic->mode->scaleFactor; \
		for (iy = 0; iy < pic->mode->scaleFactor; iy++) { \
			for (ix = 0; ix < pic->mode->scaleFactor; ix++) { \
				if (!IS_FILL_BOUNDARY(test_map[mpos + ix])) { \
					*x = ix + (xx) * pic->mode->scaleFactor; \
					*y = iy + (yy) * pic->mode->scaleFactor; \
					return 0; \
				} \
				mpos += linewidth; \
			} \
		} \
	}

static int _gfxr_find_fill_point(gfxr_pic_t *pic, int min_x, int min_y, int max_x, int max_y, int x_320,
	int y_200, int color, int drawenable, int *x, int *y) {
	// returns -1 on failure, 0 on success
	int linewidth = pic->mode->scaleFactor * 320;
	int mpos, ix, iy;
	int size_x = (max_x - min_x + 1) >> 1;
	int size_y = (max_y - min_y + 1) >> 1;
	int mid_x = min_x + size_x;
	int mid_y = min_y + size_y;
	int max_size = (size_x > size_y) ? size_x : size_y;
	int size;
	int legalcolor;
	int legalmask;
	byte *test_map;
	*x = x_320 * pic->mode->scaleFactor;
	*y = y_200 * pic->mode->scaleFactor;

	if (size_x < 0 || size_y < 0)
		return 0;

	if (drawenable & GFX_MASK_VISUAL) {
		test_map = pic->visual_map->index_data;

		if ((color & 0xf) == 0xf // When dithering with white, do more
								// conservative checks
		        || (color & 0xf0) == 0xf0)
			legalcolor = 0xff;
		else
			legalcolor = 0xf0; // Only check the second color

		legalmask = legalcolor;
	} else if (drawenable & GFX_MASK_PRIORITY) {
		test_map = pic->priority_map->index_data;
		legalcolor = 0;
		legalmask = 0xf;
	} else return -3;

	TEST_POINT(x_320, y_200); // Most likely candidate
	TEST_POINT(mid_x, mid_y); // Second most likely candidate

	for (size = 1; size <= max_size; size++) {
		int i;

		if (size <= size_y) {
			int limited_size = (size > size_x) ? size_x : size;

			for (i = mid_x - limited_size; i <= mid_x + limited_size; i++) {
				TEST_POINT(i, mid_y - size);
				TEST_POINT(i, mid_y + size);
			}
		}

		if (size <= size_x) {
			int limited_size = (size - 1 > size_y) ? size_y : size - 1;

			for (i = mid_y - limited_size; i <= mid_y + limited_size; i++) {
				TEST_POINT(mid_x - size, i);
				TEST_POINT(mid_x + size, i);
			}
		}
	}

	return -1;
}

#undef TEST_POINT

} // End of namespace Sci

// Now include the actual filling code (with scaling support)
#define FILL_FUNCTION _gfxr_fill_any
#define FILL_FUNCTION_RECURSIVE _gfxr_fill_any_recursive
#define AUXBUF_FILL_HELPER _gfxr_auxbuf_fill_any_recursive
#define AUXBUF_FILL _gfxr_auxbuf_fill_any
#define DRAW_SCALED
# include "picfill.cpp"
#undef DRAW_SCALED
#undef AUXBUF_FILL
#undef AUXBUF_FILL_HELPER
#undef FILL_FUNCTION_RECURSIVE
#undef FILL_FUNCTION

namespace Sci {

#endif // defined(WITH_PIC_SCALING)

} // End of namespace Sci

// Include again, but this time without support for scaling
#define FILL_FUNCTION _gfxr_fill_1
#define FILL_FUNCTION_RECURSIVE _gfxr_fill_1_recursive
#define AUXBUF_FILL_HELPER _gfxr_auxbuf_fill_1_recursive
#define AUXBUF_FILL _gfxr_auxbuf_fill_1
# include "picfill.cpp"
#undef AUXBUF_FILL
#undef AUXBUF_FILL_HELPER
#undef FILL_FUNCTION_RECURSIVE
#undef FILL_FUNCTION

namespace Sci {

#define GET_ABS_COORDS(x, y) \
	temp = *(resource + pos++); \
	x = *(resource + pos++); \
	y = *(resource + pos++); \
	x |= (temp & 0xf0) << 4; \
	y |= (temp & 0x0f) << 8; \
	if (flags & DRAWPIC1_FLAG_MIRRORED) \
		x = 319 - x;

#define GET_REL_COORDS(x, y) \
	temp = *(resource + pos++); \
	if (temp & 0x80) \
		x -= ((temp >> 4) & 0x7) * (flags & DRAWPIC1_FLAG_MIRRORED ? -1 : 1); \
	else \
		x += (temp >> 4) * (flags & DRAWPIC1_FLAG_MIRRORED ? -1 : 1); \
	\
	if (temp & 0x08) \
		y -= (temp & 0x7); \
	else \
		y += (temp & 0x7);

#define GET_MEDREL_COORDS(oldx, oldy) \
	temp = *(resource + pos++); \
	if (temp & 0x80) \
		y = oldy - (temp & 0x7f); \
	else \
		y = oldy + temp; \
	x = oldx + *((signed char *) resource + pos++) * (flags & DRAWPIC1_FLAG_MIRRORED ? -1 : 1);


static void check_and_remove_artifact(byte *dest, byte* srcp, int legalcolor, byte l, byte r, byte u, byte d) {
	if (*dest == legalcolor) {
		if (*srcp == legalcolor)
			return;
		if (l) {
			if (srcp[-1] == legalcolor)
				return;
			if (u && srcp[-320 - 1] == legalcolor)
				return;
			if (d && srcp[320 - 1] == legalcolor)
				return;
		}
		if (r) {
			if (srcp[1] == legalcolor)
				return;
			if (u && srcp[-320 + 1] == legalcolor)
				return;
			if (d && srcp[320 + 1] == legalcolor)
				return;
		}

		if (u && srcp[-320] == legalcolor)
			return;

		if (d && srcp[-320] == legalcolor)
			return;

		*dest = *srcp;
	}
}

void gfxr_remove_artifacts_pic0(gfxr_pic_t *dest, gfxr_pic_t *src) {
	int x_320, y_200;
	int scaled_line_size = dest->mode->scaleFactor * 320;
	int read_offset = 0;

	assert(src->mode->scaleFactor == 1);

	if (dest->mode->scaleFactor == 1) {
		warning("[GFX] attempt to remove artifacts from unscaled pic");
		return;
	}

	for (y_200 = 0; y_200 < 200; y_200++) {
		for (x_320 = 0; x_320 < 320; x_320++) {
			int write_offset = (y_200 * dest->mode->scaleFactor * scaled_line_size) + (x_320 * dest->mode->scaleFactor);
			int sub_x, sub_y;
			byte *src_visualp = &(src->visual_map->index_data[read_offset]);
			byte *src_priorityp = &(src->priority_map->index_data[read_offset]);

			for (sub_y = 0; sub_y < dest->mode->scaleFactor; sub_y++) {
				for (sub_x = 0; sub_x < dest->mode->scaleFactor; sub_x++) {
					check_and_remove_artifact(dest->visual_map->index_data + write_offset, src_visualp, (int)0xff,
					                          (byte)x_320, (byte)(x_320 < 319), (byte)(y_200 > 10), (byte)(y_200 < 199));
					check_and_remove_artifact(dest->priority_map->index_data + write_offset, src_priorityp, 0,
					                          (byte)x_320, (byte)(x_320 < 319), (byte)(y_200 > 10), (byte)(y_200 < 199));
					++write_offset;
				}
				write_offset += scaled_line_size - dest->mode->scaleFactor;
			}
			++read_offset;
		}
	}

}

static void view_transparentize(gfx_pixmap_t *view, gfx_pixmap_t *background, int posx, int posy, int width, int height) {
	int i, j;
	byte *pic_index_data = background->index_data;

	// FIXME: this assumes view and background have the same palette...
	// We may want to do a reverse mapping or similar to make it general,
	// but this (hopefully...) suffices for the current uses of this function.

	for (i = 0;i < width;i++)
		for (j = 0;j < height;j++) {
			if (view->index_data[j*width+i] == view->color_key) {
				view->index_data[j*width+i] = pic_index_data[(j+posy)*width+i+posx];
			}
		}
}

extern gfx_pixmap_t *gfxr_draw_cel0(int id, int loop, int cel, byte *resource, int size, gfxr_view_t *view, int mirrored);
extern void _gfx_crossblit_simple(byte *dest, byte *src, int dest_line_width, int src_line_width, int xl, int yl);

void gfxr_draw_pic01(gfxr_pic_t *pic, int flags, int default_palette, int size, byte *resource,
					 gfxr_pic0_params_t *style, int resid, ViewType viewType, Palette *static_pal, Common::Rect portBounds) {
	const int default_palette_table[GFXR_PIC0_PALETTE_SIZE] = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x88,
		0x88, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x88,
		0x88, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
		0x08, 0x91, 0x2a, 0x3b, 0x4c, 0x5d, 0x6e, 0x88
	};

	const int default_priority_table[GFXR_PIC0_PALETTE_SIZE] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
	};
	int palette[GFXR_PIC0_NUM_PALETTES][GFXR_PIC0_PALETTE_SIZE];
	int priority_table[GFXR_PIC0_PALETTE_SIZE];
	int drawenable = GFX_MASK_VISUAL | GFX_MASK_PRIORITY;
	int priority = 0;
	int color = 0;
	int pattern_nr = 0;
	int pattern_code = 0;
	int pattern_size = 0;
	int control = 0;
	int pos = 0;
	int x, y;
	int oldx, oldy;
	int pal = 0, index = 0;
	int temp;
	int line_mode = style->line_mode;
	int titlebar_size = portBounds.top;
	byte op, opx;

	// Initialize palette
	for (int i = 0; i < GFXR_PIC0_NUM_PALETTES; i++)
		memcpy(palette[i], default_palette_table, sizeof(int) * GFXR_PIC0_PALETTE_SIZE);

	memcpy(priority_table, default_priority_table, sizeof(int) * GFXR_PIC0_PALETTE_SIZE);

	// Main loop
	while (pos < size) {
		op = *(resource + pos++);

		switch (op) {

		case PIC_OP_SET_COLOR:
			debugC(2, kDebugLevelSci0Pic, "Set color @%d\n", pos);

			if (viewType == kViewEga) {
				pal = *(resource + pos++);
				index = pal % GFXR_PIC0_PALETTE_SIZE;
				pal /= GFXR_PIC0_PALETTE_SIZE;

				pal += default_palette;

				if (pal >= GFXR_PIC0_NUM_PALETTES) {
					error("Attempt to access invalid palette %d", pal);
					return;
				}

				color = palette[pal][index];
			} else
				color = *(resource + pos++);
			debugC(2, kDebugLevelSci0Pic, "  color <- %02x [%d/%d]\n", color, pal, index);
			drawenable |= GFX_MASK_VISUAL;
			goto end_op_loop;

		case PIC_OP_DISABLE_VISUAL:
			debugC(2, kDebugLevelSci0Pic, "Disable visual @%d\n", pos);
			drawenable &= ~GFX_MASK_VISUAL;
			goto end_op_loop;

		case PIC_OP_SET_PRIORITY:
			debugC(2, kDebugLevelSci0Pic, "Set priority @%d\n", pos);

			if (viewType == kViewEga) {
				pal = *(resource + pos++);
				index = pal % GFXR_PIC0_PALETTE_SIZE;
				pal /= GFXR_PIC0_PALETTE_SIZE; // Ignore pal

				priority = priority_table[index];
			} else priority = *(resource + pos++);

			debugC(2, kDebugLevelSci0Pic, "  priority <- %d [%d/%d]\n", priority, pal, index);
			drawenable |= GFX_MASK_PRIORITY;
			goto end_op_loop;

		case PIC_OP_DISABLE_PRIORITY:
			debugC(2, kDebugLevelSci0Pic, "Disable priority @%d\n", pos);
			drawenable &= ~GFX_MASK_PRIORITY;
			goto end_op_loop;

		case PIC_OP_SHORT_PATTERNS:
			debugC(2, kDebugLevelSci0Pic, "Short patterns @%d\n", pos);
			if (pattern_code & PATTERN_FLAG_USE_PATTERN) {
				pattern_nr = ((*(resource + pos++)) >> 1) & 0x7f;
				debugC(2, kDebugLevelSci0Pic, "  pattern_nr <- %d\n", pattern_nr);
			}

			GET_ABS_COORDS(x, y);

			_gfxr_draw_pattern(pic, x, y, color, priority, control, drawenable, pattern_code,
			                   pattern_size, pattern_nr, style->brush_mode, titlebar_size);

			while (*(resource + pos) < PIC_OP_FIRST) {
				if (pattern_code & PATTERN_FLAG_USE_PATTERN) {
					pattern_nr = ((*(resource + pos++)) >> 1) & 0x7f;
					debugC(2, kDebugLevelSci0Pic, "  pattern_nr <- %d\n", pattern_nr);
				}

				GET_REL_COORDS(x, y);

				_gfxr_draw_pattern(pic, x, y, color, priority, control, drawenable, pattern_code,
				                   pattern_size, pattern_nr, style->brush_mode, titlebar_size);
			}
			goto end_op_loop;

		case PIC_OP_MEDIUM_LINES:
			debugC(2, kDebugLevelSci0Pic, "Medium lines @%d\n", pos);
			GET_ABS_COORDS(oldx, oldy);
			while (*(resource + pos) < PIC_OP_FIRST) {
#if 0
				fprintf(stderr, "Medium-line: [%04x] from %d,%d, data %02x %02x (dx=%d)", pos, oldx, oldy,
				        0xff & resource[pos], 0xff & resource[pos+1], *((signed char *) resource + pos + 1));
#endif
				GET_MEDREL_COORDS(oldx, oldy);
#if 0
				fprintf(stderr, " to %d,%d\n", x, y);
#endif
				_gfxr_draw_line(pic, oldx, oldy, x, y, color, priority, control, drawenable, line_mode,
				                PIC_OP_MEDIUM_LINES, titlebar_size);
				oldx = x;
				oldy = y;
			}
			goto end_op_loop;

		case PIC_OP_LONG_LINES:
			debugC(2, kDebugLevelSci0Pic, "Long lines @%d\n", pos);
			GET_ABS_COORDS(oldx, oldy);
			while (*(resource + pos) < PIC_OP_FIRST) {
				GET_ABS_COORDS(x, y);
				_gfxr_draw_line(pic, oldx, oldy, x, y, color, priority, control, drawenable, line_mode,
				                PIC_OP_LONG_LINES, titlebar_size);
				oldx = x;
				oldy = y;
			}
			goto end_op_loop;

		case PIC_OP_SHORT_LINES:
			debugC(2, kDebugLevelSci0Pic, "Short lines @%d\n", pos);
			GET_ABS_COORDS(oldx, oldy);
			x = oldx;
			y = oldy;
			while (*(resource + pos) < PIC_OP_FIRST) {
				GET_REL_COORDS(x, y);
				_gfxr_draw_line(pic, oldx, oldy, x, y, color, priority, control, drawenable, line_mode,
				                PIC_OP_SHORT_LINES, titlebar_size);
				oldx = x;
				oldy = y;
			}
			goto end_op_loop;

		case PIC_OP_FILL:
			debugC(2, kDebugLevelSci0Pic, "Fill @%d\n", pos);
			while (*(resource + pos) < PIC_OP_FIRST) {
				//fprintf(stderr,"####################\n");
				GET_ABS_COORDS(x, y);
				debugC(2, kDebugLevelSci0Pic, "Abs coords %d,%d\n", x, y);
				//fprintf(stderr,"C=(%d,%d)\n", x, y + titlebar_size);
#ifdef WITH_PIC_SCALING
				if (pic->mode->scaleFactor > 1)
					_gfxr_fill_any(pic, x, y + titlebar_size, (flags & DRAWPIC01_FLAG_FILL_NORMALLY) ?
					               color : 0, priority, control, drawenable, titlebar_size);

				else
#endif
					_gfxr_fill_1(pic, x, y + titlebar_size, (flags & DRAWPIC01_FLAG_FILL_NORMALLY) ?
					             color : 0, priority, control, drawenable, titlebar_size);
			}
			goto end_op_loop;

		case PIC_OP_SET_PATTERN:
			debugC(2, kDebugLevelSci0Pic, "Set pattern @%d\n", pos);
			pattern_code = (*(resource + pos++));
			pattern_size = pattern_code & 0x07;
			goto end_op_loop;

		case PIC_OP_ABSOLUTE_PATTERN:
			debugC(2, kDebugLevelSci0Pic, "Absolute pattern @%d\n", pos);
			while (*(resource + pos) < PIC_OP_FIRST) {
				if (pattern_code & PATTERN_FLAG_USE_PATTERN) {
					pattern_nr = ((*(resource + pos++)) >> 1) & 0x7f;
					debugC(2, kDebugLevelSci0Pic, "  pattern_nr <- %d\n", pattern_nr);
				}

				GET_ABS_COORDS(x, y);

				_gfxr_draw_pattern(pic, x, y, color, priority, control, drawenable, pattern_code,
				                   pattern_size, pattern_nr, style->brush_mode, titlebar_size);
			}
			goto end_op_loop;

		case PIC_OP_SET_CONTROL:
			debugC(2, kDebugLevelSci0Pic, "Set control @%d\n", pos);
			control = (*(resource + pos++)) & 0xf;
			drawenable |= GFX_MASK_CONTROL;
			goto end_op_loop;


		case PIC_OP_DISABLE_CONTROL:
			debugC(2, kDebugLevelSci0Pic, "Disable control @%d\n", pos);
			drawenable &= ~GFX_MASK_CONTROL;
			goto end_op_loop;


		case PIC_OP_MEDIUM_PATTERNS:
			debugC(2, kDebugLevelSci0Pic, "Medium patterns @%d\n", pos);
			if (pattern_code & PATTERN_FLAG_USE_PATTERN) {
				pattern_nr = ((*(resource + pos++)) >> 1) & 0x7f;
				debugC(2, kDebugLevelSci0Pic, "  pattern_nr <- %d\n", pattern_nr);
			}

			GET_ABS_COORDS(oldx, oldy);

			_gfxr_draw_pattern(pic, oldx, oldy, color, priority, control, drawenable, pattern_code,
			                   pattern_size, pattern_nr, style->brush_mode, titlebar_size);

			x = oldx;
			y = oldy;
			while (*(resource + pos) < PIC_OP_FIRST) {
				if (pattern_code & PATTERN_FLAG_USE_PATTERN) {
					pattern_nr = ((*(resource + pos++)) >> 1) & 0x7f;
					debugC(2, kDebugLevelSci0Pic, "  pattern_nr <- %d\n", pattern_nr);
				}

				GET_MEDREL_COORDS(x, y);

				_gfxr_draw_pattern(pic, x, y, color, priority, control, drawenable, pattern_code,
				                   pattern_size, pattern_nr, style->brush_mode, titlebar_size);
			}
			goto end_op_loop;

		case PIC_OP_OPX:
			opx = *(resource + pos++);
			debugC(2, kDebugLevelSci0Pic, "OPX: ");

			if (viewType != kViewEga)
				opx += SCI1_OP_OFFSET; // See comment at the definition of SCI1_OP_OFFSET.

			switch (opx) {

			case PIC_SCI1_OPX_SET_PALETTE_ENTRIES:
				warning("[GFX] SCI1 Set palette entried not implemented");
				goto end_op_loop;

			case PIC_SCI0_OPX_SET_PALETTE_ENTRIES:
				debugC(2, kDebugLevelSci0Pic, "Set palette entry @%d\n", pos);
				while (*(resource + pos) < PIC_OP_FIRST) {
					index = *(resource + pos++);
					pal = index / GFXR_PIC0_PALETTE_SIZE;
					index %= GFXR_PIC0_PALETTE_SIZE;

					if (pal >= GFXR_PIC0_NUM_PALETTES) {
						error("Attempt to write to invalid palette %d", pal);
						return;
					}
					palette[pal][index] = *(resource + pos++);
				}
				goto end_op_loop;

			case PIC_SCI0_OPX_SET_PALETTE:
				debugC(2, kDebugLevelSci0Pic, "Set palette @%d\n", pos);
				pal = *(resource + pos++);
				if (pal >= GFXR_PIC0_NUM_PALETTES) {
					error("Attempt to write to invalid palette %d", pal);
					return;
				}

				debugC(2, kDebugLevelSci0Pic, "  palette[%d] <- (", pal);
				for (index = 0; index < GFXR_PIC0_PALETTE_SIZE; index++) {
					palette[pal][index] = *(resource + pos++);
					if (index > 0)
						debugC(2, kDebugLevelSci0Pic, ",");
					if (!(index & 0x7))
						debugC(2, kDebugLevelSci0Pic, "[%d]=", index);
					debugC(2, kDebugLevelSci0Pic, "%02x", palette[pal][index]);
				}
				debugC(2, kDebugLevelSci0Pic, ")\n");
				goto end_op_loop;

			case PIC_SCI1_OPX_SET_PALETTE:
				debugC(2, kDebugLevelSci0Pic, "Set palette @%d\n", pos);
				if (pic->visual_map->palette)
					pic->visual_map->palette->free();
				pic->visual_map->palette = gfxr_read_pal1(resid,
				                          resource + pos, SCI1_PALETTE_SIZE);
				pos += SCI1_PALETTE_SIZE;
				goto end_op_loop;

			case PIC_SCI0_OPX_MONO0:
				debugC(2, kDebugLevelSci0Pic, "Monochrome opx 0 @%d\n", pos);
				pos += 41;
				goto end_op_loop;

			case PIC_SCI0_OPX_MONO1:
			case PIC_SCI0_OPX_MONO3:
				++pos;
				debugC(2, kDebugLevelSci0Pic, "Monochrome opx %d @%d\n", opx, pos);
				goto end_op_loop;

			case PIC_SCI0_OPX_MONO2:
			case PIC_SCI0_OPX_MONO4: // Monochrome ops: Ignored by us
				debugC(2, kDebugLevelSci0Pic, "Monochrome opx %d @%d\n", opx, pos);
				goto end_op_loop;

			case PIC_SCI0_OPX_EMBEDDED_VIEW:
			case PIC_SCI1_OPX_EMBEDDED_VIEW: {
				int posx, posy;
				int bytesize;
				//byte *vismap = pic->visual_map->index_data;
				int nodraw = 0;

				gfx_pixmap_t *view;

				debugC(2, kDebugLevelSci0Pic, "Embedded view @%d\n", pos);

				GET_ABS_COORDS(posx, posy);
				bytesize = (*(resource + pos)) + (*(resource + pos + 1) << 8);
				debugC(2, kDebugLevelSci0Pic, "(%d, %d)\n", posx, posy);
				pos += 2;
				if (!nodraw) {
					if (viewType == kViewEga)
						view = gfxr_draw_cel0(-1, -1, -1, resource + pos, bytesize, NULL, flags & DRAWPIC1_FLAG_MIRRORED);
					else
						view = gfxr_draw_cel1(-1, -1, -1, flags & DRAWPIC1_FLAG_MIRRORED, resource + pos, resource + pos,
											  bytesize, NULL, viewType);
				}
				pos += bytesize;
				if (nodraw)
					continue;

				if (flags & DRAWPIC1_FLAG_MIRRORED)
					posx -= view->index_width - 1;

				debugC(2, kDebugLevelSci0Pic, "(%d, %d)-(%d, %d)\n", posx, posy, posx + view->index_width, posy + view->index_height);

				// we can only safely replace the palette if it's static
				// *if it's not for some reason, we should die

				if (view->palette && view->palette->isShared() && (viewType == kViewEga)) {
					warning("gfx_draw_pic0(): can't set a non-static palette for an embedded view");
				}

				// For SCI0, use special color mapping to copy the low
				// nibble of the color index to the high nibble.

				if (viewType != kViewEga) {
					if (view->palette)
						view->palette->free();

					if (viewType == kViewAmiga) {
						pic->visual_map->palette = static_pal->getref();
					} else {
						view->palette = pic->visual_map->palette->copy();
					}
				} else
					view->palette = embedded_view_pal->getref();

				// Clip the view's height to fit within the screen buffer
				// It can go off screen at some cases, e.g. in KQ6's intro
				view->index_height = CLIP<int>(view->index_height, 0, portBounds.height());

				// Set up mode structure for resizing the view
				gfx_mode_t *mode = gfx_new_mode(pic->visual_map->index_width / 320,
				           pic->visual_map->index_height / 200, view->palette);

				gfx_xlate_pixmap(view, mode);
				gfx_free_mode(mode);
				// When the mode is freed, the associated view
				// palette is freed too, so set it to NULL
				view->palette = NULL;

				if (flags & DRAWPIC01_FLAG_OVERLAID_PIC)
					view_transparentize(view, pic->visual_map, posx, titlebar_size + posy,
					                    view->index_width, view->index_height);

				_gfx_crossblit_simple(pic->visual_map->index_data + (titlebar_size * 320) + posy * 320 + posx,
				                      view->index_data, pic->visual_map->index_width, view->index_width,
				                      view->index_width, view->index_height);

				gfx_free_pixmap(view);
				view = NULL;
			}
			goto end_op_loop;

			case PIC_SCI0_OPX_SET_PRIORITY_TABLE:
			case PIC_SCI1_OPX_PRIORITY_TABLE_EXPLICIT: {
				int *pri_table;

				debugC(2, kDebugLevelSci0Pic, "Explicit priority table @%d\n", pos);
				if (!pic->priorityTable) {
					pic->priorityTable = (int*)malloc(16 * sizeof(int));
				} else {
					// This occurs in the title screen of Longbow, perhaps with the animated Robin sprite
					warning("[GFX] pic->priorityTable is not NULL (%p); this only occurs with overlaid pics, otherwise it's a bug", (void *)pic->priorityTable);
				}

				pri_table = pic->priorityTable;

				pri_table[0] = 0;
				pri_table[15] = 190;

				for (int i = 1; i < 15; i++)
					pri_table[i] = resource[pos++];
			}
			goto end_op_loop;

			case PIC_SCI1_OPX_PRIORITY_TABLE_EQDIST: {
				int first = (int16)READ_LE_UINT16(resource + pos);
				int last = (int16)READ_LE_UINT16(resource + pos + 2);
				int nr;
				int *pri_table;

				if (!pic->priorityTable) {
					pic->priorityTable = (int*)malloc(16 * sizeof(int));
				} else {
					error("pic->priorityTable is not NULL (%p); possible memory corruption", (void *)pic->priorityTable);
				}

				pri_table = pic->priorityTable;

				for (nr = 0; nr < 16; nr ++)
					pri_table[nr] = SCI0_PRIORITY_BAND_FIRST_14_ZONES(nr);
				pos += 4;
				goto end_op_loop;
			}

			default:
				warning("gfxr_draw_pic01(): Unknown opx %02x", opx);
				return;
			}
			goto end_op_loop;

		case PIC_OP_TERMINATE:
			debugC(2, kDebugLevelSci0Pic, "Terminator\n");
			//warning( "ARTIFACT REMOVAL CODE is commented out")
			//_gfxr_vismap_remove_artifacts();
			return;

		default:
			warning("[GFX] Unknown op %02x", op);
			return;
		}
end_op_loop: {}
	}

	warning("[GFX] Reached end of pic resource %04x", resid);
}

void gfxr_draw_pic11(gfxr_pic_t *pic, int flags, int default_palette, int size, byte *resource,
					 gfxr_pic0_params_t *style, int resid, Palette *static_pal, Common::Rect portBounds) {
	int has_bitmap = READ_LE_UINT16(resource + 4);
	int vector_data_ptr = READ_LE_UINT16(resource + 16);
	int palette_data_ptr = READ_LE_UINT16(resource + 28);
	int bitmap_data_ptr = READ_LE_UINT16(resource + 32);
	gfx_pixmap_t *view = NULL;
	int titlebar_size = portBounds.top;

	if (pic->visual_map->palette) pic->visual_map->palette->free();
	pic->visual_map->palette = gfxr_read_pal11(-1, resource + palette_data_ptr, 1284);

	if (has_bitmap)
		view = gfxr_draw_cel1(-1, 0, 0, flags & DRAWPIC1_FLAG_MIRRORED, resource, resource + bitmap_data_ptr, size - bitmap_data_ptr, NULL, kViewVga11);

	if (view) {
		view->palette = pic->visual_map->palette->getref();

		// Set up mode structure for resizing the view
		gfx_mode_t *mode = gfx_new_mode(pic->visual_map->index_width / 320, pic->visual_map->index_height / 200, view->palette);

		gfx_xlate_pixmap(view, mode);
		gfx_free_mode(mode);

		if (flags & DRAWPIC01_FLAG_OVERLAID_PIC)
			view_transparentize(view, pic->visual_map, 0, 0, view->index_width, view->index_height);

		// Clip the view's height to fit within the screen buffer
		// It can go off screen at some cases, e.g. in KQ6's intro
		view->index_height = CLIP<int>(view->index_height, 0, portBounds.height());

		_gfx_crossblit_simple(pic->visual_map->index_data + titlebar_size*view->index_width,
		                      view->index_data,
		                      pic->visual_map->index_width, view->index_width,
		                      view->index_width,
		                      view->index_height);
	} else {
		warning("[GFX] No view was contained in SCI1.1 pic resource");
	}

	gfxr_draw_pic01(pic, flags, default_palette, size - vector_data_ptr, resource + vector_data_ptr, style, resid, kViewVga11, static_pal, portBounds);
}

void gfxr_dither_pic0(gfxr_pic_t *pic, DitherMode dmode) {
	int xl = pic->visual_map->index_width;
	int yl = pic->visual_map->index_height;
	int xfrobc = 0, yfrobc = 0;
	int selection = 0;
	int x, y;
	byte *data = pic->visual_map->index_data;

	if (dmode == kDither16Colors)
		pic->visual_map->palette = gfx_sci0_image_pal[sci0_palette]->getref();

	for (y = 0; y < yl; y++) {
		for (x = 0; x < xl; x++) {

			switch (dmode) {
			case kDither16Colors:
				if (selection)
					*data = (*data & 0xf0) >> 4;
				else
					*data = (*data & 0xf);
				break;

			case kDither256Colors:
				if (selection)
					*data = ((*data & 0xf) << 4) | ((*data & 0xf0) >> 4);
				break;

			case kDitherNone:
				break;

			default:
				error("Invalid dither mode %d", dmode);
				return;
			}

			if (dmode != kDither16Colors) {
				// We permuted the 256 palette indices this way
				// to make sure the first 16 colours in the palette
				// are the regular EGA colours.
				*data ^= *data << 4;
			}

			++data;

			if (++xfrobc == pic->mode->scaleFactor) {
				selection = !selection;
				xfrobc = 0;
			}
		}

		if (++yfrobc == pic->mode->scaleFactor) {
			selection = !selection;
			yfrobc = 0;
		}
	}
}

} // End of namespace Sci
