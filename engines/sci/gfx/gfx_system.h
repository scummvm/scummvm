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

#ifndef SCI_GFX_GFX_SYSTEM
#define SCI_GFX_GFX_SYSTEM

#include "common/scummsys.h"
#include "common/rect.h"
#include "sci/tools.h"
#include "sci/gfx/palette.h"

namespace Sci {

#define GFX_DEBUG

/* General output macros */
#  define GFXERROR sciprintf("GFX Error: %s, L%d:", __FILE__, __LINE__); error

/***********************/
/*** Data structures ***/
/***********************/

#define GFX_COLOR_SYSTEM -1

#define GFX_MODE_IS_UNSCALED(mode) (((mode)->xfact == 1) && ((mode)->yfact == 1))

/* Reverse Alpha: Alpha values 0 mean "transparent" if this is
** enabled  */
#define GFX_MODE_FLAG_REVERSE_ALPHA  (1<<1)

/** Graphics mode description */
struct gfx_mode_t {

	int xfact, yfact; /* Horizontal and vertical scaling factors */
	int xsize, ysize; /* Horizontal and vertical size */
	int bytespp; /* Bytes per pixel */

	uint32 flags; /* GFX_MODE_FLAG_* Flags- see above */


	Palette *palette; // Palette or NULL to indicate non-palette mode.
	                  // Palette mode is only supported for bytespp = 1

	/* Color masks */
	uint32 red_mask, green_mask, blue_mask, alpha_mask;
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
	PaletteEntry visual;
	uint8 alpha; /* transparency = (1-opacity) */
	int8 priority, control;
	byte mask; /* see mask values below */
};


// TODO: Replace rect_t by Common::Rect
/** Rectangle description */
struct rect_t {
	int x, y;
	int width, height; /* width, height: (x,y,width,height)=(5,5,1,1) occupies 1 pixel */
};

/* Generates a rect_t from index data
** Parameters: (int x int) x,y: Upper left point of the rectangle
**             (int x int) width, height: Horizontal and vertical extension of the rectangle
** Returns   : (rect_t) A rectangle matching the supplied parameters
*/
static inline rect_t gfx_rect(int x, int y, int width, int height) {
	rect_t rect;

	rect.x = x;
	rect.y = y;
	rect.width = width;
	rect.height = height;

	return rect;
}

// Temporary helper functions to ease the transition from rect_t to Common::Rect
static inline rect_t toSCIRect(Common::Rect in) {
	return gfx_rect(in.left, in.top, in.width(), in.height());
}

static inline Common::Rect toCommonRect(rect_t in) {
	return Common::Rect(in.x, in.y, in.x + in.width, in.y + in.height);
}

#define GFX_PRINT_RECT(rect) (rect).x, (rect).y, (rect).width, (rect).height

#define OVERLAP(a, b, z, zl) (a.z >= b.z && a.z < (b.z + b.zl))

/* Determines whether two rects overlap
** Parameters: (rect_t x rect_t) a,b: The two rect_ts to check for overlap
** Returns   : (int) 1 if they overlap, 0 otherwise
*/
static inline int gfx_rects_overlap(rect_t a, rect_t b) {
	return (OVERLAP(a, b, x, width) || OVERLAP(b, a, x, width)) && (OVERLAP(a, b, y, height) || OVERLAP(b, a, y, height));
}

#undef OVERLAP

/* gfx_rect_fullscreen is declared in gfx/gfx_tools.c */
extern rect_t gfx_rect_fullscreen;

#define GFX_RESID_NONE -1

#define GFX_PIC_COLORS 256

#define GFX_PIXMAP_FLAG_SCALED_INDEX      (1<<0) /* Index data is scaled already */
#define GFX_PIXMAP_FLAG_INSTALLED         (1<<2) /* Pixmap has been registered */
#define GFX_PIXMAP_FLAG_PALETTIZED	  (1<<6) /* Indicates a palettized view */

#define GFX_PIXMAP_COLOR_KEY_NONE -1 /* No transpacency colour key */
#define GFX_CURSOR_TRANSPARENT 255 // Cursor colour key

struct gfx_pixmap_t { /* gfx_pixmap_t: Pixel map */

	/*** Meta information ***/
	int ID; /* Resource ID, or GFX_RESID_NONE for anonymous graphical data */
	short loop, cel; /* loop and cel number for views */


	/*** Color map ***/
	Palette *palette;
	int colors_nr() const { return palette ? palette->size() : 0; }
				    /* color entries, or NULL if the
				    ** default palette is to be used.
				    ** A maximum of 255 colors is allowed; color
				    ** index 0xff is reserved for transparency.
				    ** As a special exception, 256 colors are
				    ** allowed for background pictures (which do
				    ** not use transparency)
				    */
	uint32 flags;

	/*** Hot spot ***/
	int xoffset, yoffset; /* x and y coordinates of the 'hot spot' (unscaled) */

	/*** Index data ***/
	int index_width, index_height; /* width and height of the indexed original image */
	byte *index_data; /* Color-index data, or NULL if read from an
			  ** external source
			  */

	/*** Drawable data ***/
	int width, height; /* width and height of the actual image */
	int data_size; /* Amount of allocated memory */
	byte *data; /* Drawable data, or NULL if not converted.  */

	byte *alpha_map; /* Byte map with alpha values. It is used only if the
			 ** graphics mode's alpha_mask is zero.
			 */

	int color_key;
	int palette_revision; // Revision of palette at the time data was generated
};



/***********************/
/*** Constant values ***/
/***********************/

/* Return values */
enum gfx_return_value_t {
	GFX_OK = 0, /* Indicates "operation successful" */
	GFX_ERROR = -1, /* Indicates "operation failed" */
	GFX_FATAL = -2
	/* Fatal error: Used by graphics drivers to indicate that they were unable to
	** do anything useful
	*/
};


enum gfx_map_mask_t {/* Map masks */
	GFX_MASK_NONE = 0,
	GFX_MASK_VISUAL = 1,
	GFX_MASK_PRIORITY = 2,
	GFX_MASK_CONTROL = 4
};

/* 'no priority' mode */
enum {
	GFX_NO_PRIORITY = -1
};

/* Text alignment values */

enum gfx_alignment_t {
	ALIGN_RIGHT = -1,
	ALIGN_TOP = -1,
	ALIGN_CENTER = 1,
	ALIGN_LEFT = 0,
	ALIGN_BOTTOM = 0
};


enum gfx_line_mode_t {
	GFX_LINE_MODE_CORRECT, /* Scaled separately */
	GFX_LINE_MODE_FAST,    /* Scaled by (xfact+yfact)/2 */
	GFX_LINE_MODE_FINE    /* Always drawn at width 1 */
};

enum gfx_brush_mode_t {
	GFX_BRUSH_MODE_SCALED, /* Just scale the brush pixels */
	GFX_BRUSH_MODE_ELLIPSES, /* Replace pixels with ellipses */
	GFX_BRUSH_MODE_RANDOM_ELLIPSES, /* Replace pixels with ellipses moved and re-scaled randomly */
	GFX_BRUSH_MODE_MORERANDOM /* Distribute randomly */
};


enum gfx_line_style_t {
	GFX_LINE_STYLE_NORMAL,
	GFX_LINE_STYLE_STIPPLED
};


enum gfx_rectangle_fill_t {
	GFX_SHADE_FLAT, /* Don't shade */
	GFX_SHADE_VERTICALLY, /* Shade vertically */
	GFX_SHADE_HORIZONTALLY /* Shade horizontally */
};


enum gfx_color_mode_t {
	GFX_COLOR_MODE_AUTO = 0, /* Auto-detect- handled by the gfxop library */
	GFX_COLOR_MODE_INDEX = 1, /* Index mode */
	GFX_COLOR_MODE_HIGH = 2, /* High color mode (15bpp or 16 bpp) */
	GFX_COLOR_MODE_TRUE = 4 /* True color mode (24 bpp padded to 32 bpp) */
};

} // End of namespace Sci

#endif // SCI_GFX_GFX_SYSTEM
