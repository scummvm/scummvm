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
#include "graphics/pixelformat.h"

namespace Sci {

#define GFX_DEBUG

/***********************/
/*** Data structures ***/
/***********************/

#define GFX_COLOR_SYSTEM -1

#define GFX_MODE_IS_UNSCALED(mode) (((mode)->xfact == 1) && ((mode)->yfact == 1))

/** Graphics mode description
 *
 * Color masks:
 * Each of the mask/shift pairs describe where the corresponding color
 * values are stored for the described mode. Internally, color
 * calculations are done by using 32 bit values for r, g, b, a. After
 * the internal values have been calculated, they are shifted RIGHT
 * by the xxx_shift amount described above, then ANDed with the
 * corresponding color mask; finally, all three results are ORred to-
 * gether. The alpha values are used as appropriate; if alpha_mask is
 * zero, then images use a special alpha map.
 */

struct gfx_mode_t {

	int xfact, yfact; /**< Horizontal and vertical scaling factors */
	int xsize, ysize; /**< Horizontal and vertical size */

	/**
	 * Palette or NULL to indicate non-palette mode.
	 * Palette mode is only supported for bytespp = 1
	 */
	Palette *palette;

	Graphics::PixelFormat format;
};



#define GFX_COLOR_INDEX_UNMAPPED -1

/** Pixmap-specific color entries */
struct  gfx_pixmap_color_t{
	int global_index; /**< Global index color or GFX_COLOR_INDEX_UNMAPPED. */
	uint8 r, g, b; /**< Real color */
};

/** Full color */
struct gfx_color_t {
	PaletteEntry visual;
	uint8 alpha; /**< transparency = (1-opacity) */
	int8 priority, control;
	byte mask; /**< see mask values below */
};


// TODO: Replace rect_t by Common::Rect
/** Rectangle description */
struct rect_t {
	int x, y;
	int width, height; /* width, height: (x,y,width,height)=(5,5,1,1) occupies 1 pixel */
};

/**
 * Generates a rect_t from index data
 *
 * @param[in] x			Left side of the rectangle
 * @param[in] y 		Top side of the rectangle
 * @param[in] width		Horizontal extent of the rectangle
 * @param[in] height	Verical extent of the rectangle
 * @return				A rectangle matching the supplied parameters
 */
static inline rect_t gfx_rect(int x, int y, int width, int height) {
	rect_t rect;

	rect.x = x;
	rect.y = y;
	rect.width = width;
	rect.height = height;

	return rect;
}

/**
 * Temporary helper function to ease the transition from rect_t to Common::Rect
 */
static inline rect_t toSCIRect(Common::Rect in) {
	return gfx_rect(in.left, in.top, in.width(), in.height());
}

/**
 * Temporary helper function to ease the transition from rect_t to Common::Rect
 */
static inline Common::Rect toCommonRect(rect_t in) {
	return Common::Rect(in.x, in.y, in.x + in.width, in.y + in.height);
}

#define GFX_PRINT_RECT(rect) (rect).x, (rect).y, (rect).width, (rect).height

#define OVERLAP(a, b, z, zl) (a.z >= b.z && a.z < (b.z + b.zl))

/**
 * Determines whether two rects overlap
 *
 * @param[in] a	First rect to check for overlap
 * @param[in] b	Second rect to check for overlap
 * @return		1 if they overlap, 0 otherwise
 */
static inline int gfx_rects_overlap(rect_t a, rect_t b) {
	return (OVERLAP(a, b, x, width) || OVERLAP(b, a, x, width)) && (OVERLAP(a, b, y, height) || OVERLAP(b, a, y, height));
}

#undef OVERLAP

/* gfx_rect_fullscreen is declared in gfx/gfx_tools.c */
extern rect_t gfx_rect_fullscreen;

#define GFX_RESID_NONE -1

#define GFX_PIC_COLORS 256

#define GFX_PIXMAP_FLAG_SCALED_INDEX	(1<<0) /* Index data is scaled already */
#define GFX_PIXMAP_FLAG_INSTALLED		(1<<2) /* Pixmap has been registered */
#define GFX_PIXMAP_FLAG_PALETTIZED		(1<<6) /* Indicates a palettized view */

#define GFX_PIXMAP_COLOR_KEY_NONE -1 /* No transpacency colour key */
#define GFX_CURSOR_TRANSPARENT 255 // Cursor colour key

/** Pixel map */
struct gfx_pixmap_t {

	/** @name Meta information
	 * @{*/
	int ID; /**< Resource ID, or GFX_RESID_NONE for anonymous graphical data */
	short loop; /**< loop number for view */
	short cel;	/**< cel number for view */
	/** @}*/

	/** @name Color map
	 * @{*/
	Palette *palette;

	/**
	 * color entries, or NULL if the default palette is to be used. A maximum
	 * of 255 colors is allowed; color index 0xff is reserved for transparency.
	 * As a special exception, 256 colors are allowed for background pictures
	 * (which do not use transparency)
	 */
	int colors_nr() const { return palette ? MIN<int>(palette->size(), 256) : 0; }

	uint32 flags;
	/* @} */

	/** @name Hot spot
	 * x and y coordinates of the 'hot spot' (unscaled)
	 * @{*/
	int xoffset, yoffset;
	/** @} */

	/** @name Index data
	 * @{
	 */
	int index_width; /**< width of the indexed original image */
	int index_height; /**< height of the indexed original image */
	byte *index_data; /**< Color-index data, or NULL if read from an external source */
	/** @} */

	/** @name Drawable data
	 * @{
	 */
	int width; /**< width of the actual image */
	int height; /**< height of the actual image */
	int data_size; /**< Amount of allocated memory */
	byte *data; /**< Drawable data, or NULL if not converted.  */

	byte *alpha_map; /**< Byte map with alpha values. It is used only if the graphics mode's alpha_mask is zero. */

	int color_key; /**< The color to make transparent */
	int palette_revision; /**< Revision of palette at the time data was generated */
	/** @} */
};




/** @name Constant values
 * @{ */

/** Map masks */
enum gfx_map_mask_t {
	GFX_MASK_NONE = 0,
	GFX_MASK_VISUAL = 1,
	GFX_MASK_PRIORITY = 2,
	GFX_MASK_CONTROL = 4
};

/** 'no priority' mode */
enum {
	GFX_NO_PRIORITY = -1
};

/** Text alignment values */
enum gfx_alignment_t {
	ALIGN_RIGHT = -1,
	ALIGN_TOP = -1,
	ALIGN_CENTER = 1,
	ALIGN_LEFT = 0,
	ALIGN_BOTTOM = 0
};


enum gfx_line_mode_t {
	GFX_LINE_MODE_CORRECT,	/**< Scaled separately */
	GFX_LINE_MODE_FAST,		/**< Scaled by (xfact+yfact)/2 */
	GFX_LINE_MODE_FINE		/**< Always drawn at width 1 */
};

enum gfx_brush_mode_t {
	GFX_BRUSH_MODE_SCALED,			/**< Just scale the brush pixels */
	GFX_BRUSH_MODE_ELLIPSES,		/**< Replace pixels with ellipses */
	GFX_BRUSH_MODE_RANDOM_ELLIPSES,	/**< Replace pixels with ellipses moved and re-scaled randomly */
	GFX_BRUSH_MODE_MORERANDOM		/**< Distribute randomly */
};


enum gfx_line_style_t {
	GFX_LINE_STYLE_NORMAL,
	GFX_LINE_STYLE_STIPPLED
};


enum gfx_rectangle_fill_t {
	GFX_SHADE_FLAT,			/**< Don't shade */
	GFX_SHADE_VERTICALLY,	/**< Shade vertically */
	GFX_SHADE_HORIZONTALLY	/**< Shade horizontally */
};

/** @} */

} // End of namespace Sci

#endif // SCI_GFX_GFX_SYSTEM
