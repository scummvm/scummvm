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

#ifndef _SCI_GFX_SYSTEM_
#define _SCI_GFX_SYSTEM_

#include "common/scummsys.h"
#include "common/rect.h"
#include "sci/include/sci_memory.h"
#include "sci/tools.h"

namespace Sci {

#define GFX_DEBUG

/* General output macros */
#  define GFXERROR gfxprintf("GFX Error: %s, L%d:", __FILE__, __LINE__); sciprintf
#  define GFXWARN gfxprintf("GFX Warning: %s, L%d:", __FILE__, __LINE__); sciprintf
#  ifdef GFX_DEBUG
#    define GFXDEBUG gfxprintf("GFX-debug: %s, L%d:", __FILE__, __LINE__); sciprintf
#  else /* !GFX_DEBUG */
#    define GFXDEBUG if (0) printf
#  endif /* !GFX_DEBUG */

/***********************/
/*** Data structures ***/
/***********************/

#define GFX_COLOR_SYSTEM -1


/** Palette color description */
struct gfx_palette_color_t {

	int lockers; /* Number of pixmaps holding a lock on that color.
		     ** 0 means that the color is unused, -1 means that it is
		     ** "system allocated" and may not be freed.  */
	byte r, g, b; /* Red, green, blue; intensity varies from 0 (min) to 255 (max) */

};

/** Palette description for color index modes */
struct gfx_palette_t{

	int max_colors_nr; /* Maximum number of allocated colors */
	gfx_palette_color_t *colors; /* Actual colors, malloc()d as a block */
};



#define GFX_MODE_IS_UNSCALED(mode) (((mode)->xfact == 1) && ((mode)->yfact == 1))

/* Reverse-endian: Target display has non-native endianness
** (BE if local is LE or the other way 'round  */
#define GFX_MODE_FLAG_REVERSE_ENDIAN (1<<0)
/* Reverse Alpha: Alpha values 0 mean "transparent" if this is
** enabled  */
#define GFX_MODE_FLAG_REVERSE_ALPHA  (1<<1)

/** Graphics mode description */
struct gfx_mode_t {

	int xfact, yfact; /* Horizontal and vertical scaling factors */
	int bytespp; /* Bytes per pixel */

	unsigned int flags; /* GFX_MODE_FLAG_* Flags- see above */


	gfx_palette_t *palette; /* Palette or NULL to indicate non-palette mode.
				** Palette (color-index) mode is only supported
				** for bytespp=1.  */

	/* Color masks */
	unsigned int red_mask, green_mask, blue_mask, alpha_mask;
	short red_shift, green_shift, blue_shift, alpha_shift;

	/* Each of the mask/shift pairs describe where the corresponding color
	** values are stored for the described mode. Internally, color
	** calculations are done by using 32 bit values for r, g, b, a. After
	** the internal values have been calculated, they are shifted RIGHT
	** by the xxx_shift amount described above, then ANDed with the
	** corresponding color mask; finally, all three results are ORred to-
	** gether. The alpha values are used as appropriate; if alpha_mask is
	** zero, then images use a special alpha map.  */

};



#define GFX_COLOR_INDEX_UNMAPPED -1

/** Pixmap-specific color entries */
struct  gfx_pixmap_color_t{
	int global_index; /* Global index color or GFX_COLOR_INDEX_UNMAPPED. */
	uint8 r, g, b; /* Real color */
};

/** Full color */
struct gfx_color_t {
	gfx_pixmap_color_t visual;
	uint8 alpha; /* transparency = (1-opacity) */
	signed char priority, control;
	byte mask; /* see mask values below */
};


/** Rectangle description */
struct rect_t{
	int x, y;
	int xl, yl; /* width, height: (x,y,xl,yl)=(5,5,1,1) occupies 1 pixel */
};


/* Generates a rect_t from index data
** Parameters: (int x int) x,y: Upper left point of the rectangle
**             (int x int) xl, yl: Horizontal and vertical extension of the rectangle
** Returns   : (rect_t) A rectangle matching the supplied parameters
*/
static inline rect_t gfx_rect(int x, int y, int xl, int yl) {
	rect_t rect;

	rect.x = x;
	rect.y = y;
	rect.xl = xl;
	rect.yl = yl;

	return rect;
}

#define GFX_PRINT_RECT(rect) (rect).x, (rect).y, (rect).xl, (rect).yl

#define OVERLAP(a, b, z, zl) (a.z >= b.z && a.z < (b.z + b.zl))

/* Determines whether two rects overlap
** Parameters: (rect_t x rect_t) a,b: The two rect_ts to check for overlap
** Returns   : (int) 1 if they overlap, 0 otherwise
*/
static inline int gfx_rects_overlap(rect_t a, rect_t b) {
	return (OVERLAP(a, b, x, xl) || OVERLAP(b, a, x, xl)) && (OVERLAP(a, b, y, yl) || OVERLAP(b, a, y, yl));
}

#undef OVERLAP

#define MERGE_PARTIAL(z, zl) \
if (a.z < b.z) SUBMERGE_PARTIAL(a, b, z, zl) \
else SUBMERGE_PARTIAL(b, a, z, zl)

#define SUBMERGE_PARTIAL(a, b, z, zl) \
{ \
	retval.z = a.z; \
	retval.zl = a.zl; \
	if (b.z + b.zl > a.z + a.zl) \
		retval.zl = (b.z + b.zl - a.z); \
}



#define RECT(a) a.x, a.y, a.xl, a.yl

/* Merges two rects
** Parameters: (rect_t x rect_t) a,b: The two rects to merge
** Returns   : (rect_t) The smallest rect containing both a and b
*/
static inline rect_t gfx_rects_merge(rect_t a, rect_t b) {
	rect_t retval;
	MERGE_PARTIAL(x, xl);
	MERGE_PARTIAL(y, yl);
	return retval;
}
#undef MERGE_PARTIAL
#undef SUBMERGE_PARTIAL


/* Subset predicate for rectangles
** Parameters: (rect_t) a, b: The two rects to compare
** Returns   : non-zero iff for each pixel p in a the following holds: p is in b.
*/
static inline int gfx_rect_subset(rect_t a, rect_t b) {
	return ((a.x >= b.x) && (a.y >= b.y) && ((a.x + a.xl) <= (b.x + b.xl)) && ((a.y + a.yl) <= (b.y + b.yl)));
}


/* Equality predicate for rects
** Parameters: (rect_t) a, b
** Returns   : (int) gfx_rect_subset(a,b) AND gfx_rect_subset(b,a)
*/
static inline int gfx_rect_equals(rect_t a, rect_t b) {
	return (a.x == b.x && a.xl == b.xl && a.y == b.y && a.yl == b.yl);
}


/* gfx_rect_fullscreen is declared in gfx/gfx_tools.c */
extern rect_t gfx_rect_fullscreen;

/* Translation operation for rects
** Parameters: (rect_t) rect: The rect to translate
**             (Common::Point) offset: The offset to translate it by
** Returns   : (rect_t) The translated rect
*/
static inline rect_t gfx_rect_translate(rect_t rect, Common::Point offset) {
	rect.x += offset.x;
	rect.y += offset.y;

	return rect;
}

#define GFX_RESID_NONE -1

#define GFX_PIC_COLORS 256

#define GFX_PIXMAP_FLAG_SCALED_INDEX      (1<<0) /* Index data is scaled already */
#define GFX_PIXMAP_FLAG_EXTERNAL_PALETTE  (1<<1) /* The colors pointer points to an external palette */
#define GFX_PIXMAP_FLAG_INSTALLED         (1<<2) /* Pixmap has been registered */
#define GFX_PIXMAP_FLAG_PALETTE_ALLOCATED (1<<3) /* Palette has been allocated */
#define GFX_PIXMAP_FLAG_PALETTE_SET       (1<<4) /* Palette has been propagated to the driver */
#define GFX_PIXMAP_FLAG_DONT_UNALLOCATE_PALETTE (1<<5) /* Used by text, which uses preallocated colors */
#define GFX_PIXMAP_FLAG_PALETTIZED	  (1<<6) /* Indicates a palettized view */

#define GFX_PIXMAP_COLOR_KEY_NONE -1 /* No transpacency colour key */

struct gfx_pixmap_t { /* gfx_pixmap_t: Pixel map */

	/*** Meta information ***/
	int ID; /* Resource ID, or GFX_RESID_NONE for anonymous graphical data */
	short loop, cel; /* loop and cel number for views */


	/*** Color map ***/
	int colors_nr;
	gfx_pixmap_color_t *colors; /* colors_nr color entries, or NULL if the
				    ** default palette is to be used.
				    ** A maximum of 255 colors is allowed; color
				    ** index 0xff is reserved for transparency.
				    ** As a special exception, 256 colors are
				    ** allowed for background pictures (which do
				    ** not use transparency)
				    */
	unsigned int flags;

	/*** Hot spot ***/
	int xoffset, yoffset; /* x and y coordinates of the 'hot spot' (unscaled) */

	/*** Index data ***/
	int index_xl, index_yl; /* width and height of the indexed original image */
	byte *index_data; /* Color-index data, or NULL if read from an
			  ** external source
			  */

	/*** Drawable data ***/
	int xl, yl; /* width and height of the actual image */
	int data_size; /* Amount of allocated memory */
	byte *data; /* Drawable data, or NULL if not converted.  */

	byte *alpha_map; /* Byte map with alpha values. It is used only if the
			 ** graphics mode's alpha_mask is zero.
			 */

	int color_key;

	/*** Data reserved for gfx_driver use ***/
	struct pixmap_internal { /* Internal state management data for use by drivers */
		int handle; /* initialized to 0 */
		void *info; /* initialized to NULL */
	} internal;

};


#define GFX_FONT_BUILTIN_5x8  -1
#define GFX_FONT_BUILTIN_6x10 -2

struct gfx_bitmap_font_t { /* gfx_bitmap_font_t: Bitmap font information */
	int ID; /* Unique resource ID */

	int chars_nr; /* Numer of available characters */

	int *widths; /* chars_nr character widths, in pixels */

	int row_size; /* Byte size of each pixel row. For unscaled fonts, this is
		      ** always 1, 2, or 4. Otherwise, it's a multiple of 4.
		      */

	int line_height; /* Height of each text line (usually identical to height) */
	int height; /* Height for all characters, in pixel rows */
	int char_size; /* Amount of memory occupied by one character in data */

	byte *data; /* Font data, consisting of 'chars_nr' entries of 'height' rows
		    ** of 'row_size' bytes. For each character ch, its first byte
		    ** (the topmost row) is located at (data + (charsize * ch)), and
		    ** its pixel width is widths[ch], provided that (ch < chars_nr).
		    */

};



/***********************/
/*** Constant values ***/
/***********************/

/* Default palettes */
extern gfx_pixmap_color_t gfx_sci0_image_colors[][16];
extern gfx_pixmap_color_t gfx_sci0_pic_colors[256];

/* Return values */
enum gfx_return_value_t {
	GFX_OK = 0, /* Indicates "operation successful" */
	GFX_ERROR = -1, /* Indicates "operation failed" */
	GFX_FATAL = -2
	/* Fatal error: Used by graphics drivers to indicate that they were unable to
	** do anything useful
	*/
};


typedef enum {/* Map masks */
	GFX_MASK_NONE = 0,
	GFX_MASK_VISUAL = 1,
	GFX_MASK_PRIORITY = 2,
	GFX_MASK_CONTROL = 4
} gfx_map_mask_t;

/* 'no priority' mode */
#define GFX_NO_PRIORITY -1

/* Text alignment values */

typedef enum {
	ALIGN_RIGHT = -1,
	ALIGN_TOP = -1,
	ALIGN_CENTER = 1,
	ALIGN_LEFT = 0,
	ALIGN_BOTTOM = 0
} gfx_alignment_t;


typedef enum {
	GFX_LINE_MODE_CORRECT, /* Scaled separately */
	GFX_LINE_MODE_FAST,    /* Scaled by (xfact+yfact)/2 */
	GFX_LINE_MODE_FINE    /* Always drawn at width 1 */
} gfx_line_mode_t;

typedef enum {
	GFX_BRUSH_MODE_SCALED, /* Just scale the brush pixels */
	GFX_BRUSH_MODE_ELLIPSES, /* Replace pixels with ellipses */
	GFX_BRUSH_MODE_RANDOM_ELLIPSES, /* Replace pixels with ellipses moved and re-scaled randomly */
	GFX_BRUSH_MODE_MORERANDOM /* Distribute randomly */
} gfx_brush_mode_t;


typedef enum {
	GFX_LINE_STYLE_NORMAL,
	GFX_LINE_STYLE_STIPPLED
} gfx_line_style_t;


typedef enum {
	GFX_SHADE_FLAT, /* Don't shade */
	GFX_SHADE_VERTICALLY, /* Shade vertically */
	GFX_SHADE_HORIZONTALLY /* Shade horizontally */
} gfx_rectangle_fill_t;


typedef enum {
	GFX_COLOR_MODE_AUTO = 0, /* Auto-detect- handled by the gfxop library */
	GFX_COLOR_MODE_INDEX = 1, /* Index mode */
	GFX_COLOR_MODE_HIGH = 2, /* High color mode (15bpp or 16 bpp) */
	GFX_COLOR_MODE_TRUE = 4 /* True color mode (24 bpp padded to 32 bpp) */
} gfx_color_mode_t;

} // End of namespace Sci

#endif /* !_SCI_GFX_SYSTEM_ */
