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

/* Graphical operations, called from the widget state manager */

#ifndef SCI_GFX_GFX_OPERATIONS_H
#define SCI_GFX_GFX_OPERATIONS_H

#include "sci/sci.h"	// for INCLUDE_OLDGFX
#ifdef INCLUDE_OLDGFX
#include "sci/gfx/gfx_resmgr.h"
#include "sci/gfx/gfx_tools.h"
#include "sci/gfx/gfx_system.h"
#endif
#include "sci/uinput.h"

#include "common/list.h"

namespace Sci {

#ifdef INCLUDE_OLDGFX

struct TextFragment;

/* Threshold in color index mode to differentiate between visible and non-visible stuff.
** GFXOP_ALPHA_THRESHOLD itself should be treated as non-visible.
*/
#define GFXOP_ALPHA_THRESHOLD 0xff

struct TextHandle {
	Common::String _text; /**< Copy of the actual text */

	int line_height;
	Common::Array<TextFragment> lines; /**< Text offsets */
	gfx_bitmap_font_t *font;
	Common::Array<gfx_pixmap_t *> text_pixmaps;

	int width, height;

	int priority, control;
	gfx_alignment_t halign, valign;

	TextHandle();
	~TextHandle();
};

/* Unless individually stated otherwise, the following applies:
** All operations herein apply to the standard 320x200 coordinate system.
** All operations perform clipping relative to state->clip_zone.
*/

enum gfx_box_shade_t {
	GFX_BOX_SHADE_FLAT,
	GFX_BOX_SHADE_RIGHT,
	GFX_BOX_SHADE_LEFT,
	GFX_BOX_SHADE_DOWN,
	GFX_BOX_SHADE_UP
#if 0
	/* possible with alphaing, but there is no way to check for
	** alpha capability of gfx_driver->draw_filled_rect() yet
	*/
	, GFX_BOX_SHADE_RIGHT_DOWN,
	GFX_BOX_SHADE_LEFT_DOWN,
	GFX_BOX_SHADE_RIGHT_UP,
	GFX_BOX_SHADE_LEFT_UP
#endif
};


typedef Common::List<rect_t> DirtyRectList;

#endif	// INCLUDE_OLDGFX

struct GfxState {
#ifdef INCLUDE_OLDGFX
	rect_t clip_zone_unscaled; /**< The current UNSCALED clipping zone */
	rect_t clip_zone; /**< The current SCALED clipping zone; a cached scaled version of clip_zone_unscaled */

	GfxDriver *driver;

	int visible_map;

	GfxResManager *gfxResMan;

	gfx_pixmap_t *priority_map; /**< back buffer priority map (unscaled) */
	gfx_pixmap_t *static_priority_map; /**< static buffer priority map (unscaled) */
	gfx_pixmap_t *control_map; /**< back buffer control map (only exists unscaled in the first place) */


	int tag_mode; /**< Set to 1 after a new pic is drawn and the resource manager has tagged all resources. Reset after the next front buffer update is done, when all resources that are still tagged are flushed.  */

	int disable_dirty; /**< Set to 1 to disable dirty rect accounting */

	int pic_nr; /**< Number of the current pic */
	int palette_nr; /**< Palette number of the current pic */

	gfxr_pic_t *pic, *pic_unscaled; /**< The background picture and its unscaled equivalent */
	rect_t pic_port_bounds;  /**< Picture port bounds */

	DirtyRectList _dirtyRects; /**< Dirty rectangles */
#endif

	Common::List<sci_event_t> _events;
};


#ifdef INCLUDE_OLDGFX


/** @name Fundamental operations */
/** @{ */

/**
 * Initializes a graphics mode.
 *
 * @param[in] state			The state to initialize
 * @param[in] scaleFactor	Scale factor
 * @param[in] mode			Graphics mode to use
 * @param[in] resMan		Resource manager to use
 */
void gfxop_init(GfxState *state, ResourceManager *resMan,
		SciGuiScreen *screen, SciGuiPalette *palette, int scaleFactor = 1);

/**
 * Deinitializes a currently active driver.
 *
 * @param[in] state	The state encapsulating the driver in question
 */
void gfxop_exit(GfxState *state);

/**
 * Calculates a bit mask calculated from some pixels on the specified
 * map.
 *
 * @param[in] state	The state containing the pixels to scan
 * @param[in] area	The area to check
 * @param[in] map	The GFX_MASKed map(s) to test
 * @return			An integer value where, for each 0 <= i <= 15, bit i is set
 * 					iff	there exists a map for which the corresponding bit was
 * 					set in the 'map' parameter and for which there exists a
 * 					pixel within the specified area so that the pixel's lower 4
 * 					bits, interpreted as an integer value, equal i. (Short
 * 					version: This is an implementation of "on_control()").
 */
int gfxop_scan_bitmask(GfxState *state, rect_t area, gfx_map_mask_t map);

/**
 * Sets a new clipping zone.
 *
 * @param[in] state	The affected state
 * @param[in] zone	The new clipping zone
 */
void gfxop_set_clip_zone(GfxState *state, rect_t zone);
/** @} */


/** @name Generic drawing operations */
/** @{ */

/**
 * Renders a clipped line to the back buffer.
 *
 * @param[in] state			The state affected
 * @param[in] start			Starting point of the line
 * @param[in] end			End point of the line
 * @param[in] color			The color to use for drawing
 * @param[in] line_mode		Any valid line mode to use
 * @param[in] line_style	The line style to use
 */
void gfxop_draw_line(GfxState *state,
	Common::Point start, Common::Point end, gfx_color_t color,
	gfx_line_mode_t line_mode, gfx_line_style_t line_style);

/**
 * Draws a non-filled rectangular box to the back buffer.
 *
 * Boxes drawn in thin lines will surround the minimal area described by rect.
 *
 * @param[in] state			The affected state
 * @param[in] rect			The rectangular area the box is drawn to
 * @param[in] color			The color the box is to be drawn in
 * @param[in] line_mode		The line mode to use
 * @param[in] line_style	The line style to use for the box
 */
void gfxop_draw_rectangle(GfxState *state, rect_t rect, gfx_color_t color,
	gfx_line_mode_t line_mode, gfx_line_style_t line_style);

/**
 * Draws a filled box to the back buffer.
 *
 * The draw mask, control, and priority values are derived from color1.
 *
 * @param[in] state			The affected state
 * @param[in] box			The area to draw to
 * @param[in] color1		The primary color to use for drawing
 * @param[in] color2		The secondary color to draw in
 * @param[in] shade_type	The shading system to use (e.g. GFX_BOX_SHADE_FLAT)
 */
void gfxop_draw_box(GfxState *state, rect_t box, gfx_color_t color1,
	gfx_color_t color2, gfx_box_shade_t shade_type);

/**
 * Fills a box in the back buffer with a specific color.
 *
 * This is a simple wrapper function for gfxop_draw_box
 *
 * @param[in] state	The state to draw to
 * @param[in] box	The box to fill
 * @param[in] color	The color to use for filling
 */
void gfxop_fill_box(GfxState *state, rect_t box, gfx_color_t color);

/**
 * Copies a box from the static buffer to the back buffer.
 *
 * @param[in] state	The affected state
 * @param[in] box	The box to propagate from the static buffer
 */
void gfxop_clear_box(GfxState *state, rect_t box);


/**
 * Updates all dirty rectangles.
 *
 * In order to track dirty rectangles, they must be enabled in the options. This
 * function instructs the resource manager to free all tagged data on certain
 * occasions (see gfxop_new_pic).
 *
 * @param[in] state	The relevant state
 */
void gfxop_update(GfxState *state);


/**
 * Propagates a box from the back buffer to the front (visible) buffer.
 *
 * This function instructs the resource manager to free all tagged data on
 * certain occasions (see gfxop_new_pic). When called with dirty rectangle
 * management enabled, it will automatically propagate all dirty rectangles as
 * well, UNLESS dirty frame accounting has been disabled explicitly.
 *
 * @param[in] state	The affected state
 * @param[in] box	The box to propagate to the front buffer
 */
void gfxop_update_box(GfxState *state, rect_t box);

/**
 * Enables dirty frame accounting.
 *
 * Dirty frame accounting is enabled by default.
 *
 * @param[in] state	The state dirty frame accounting is to be enabled in
 */
void gfxop_enable_dirty_frames(GfxState *state);

/**
 * Disables dirty frame accounting.
 *
 * @param[in] state	The state dirty frame accounting is to be disabled in
 */
void gfxop_disable_dirty_frames(GfxState *state);
/** @} */


/** @name Color operations */
/** @{ */

/**
 * Maps an r/g/b value to a color and sets a gfx_color_t structure.
 *
 * In palette mode, this may allocate a new color. If any of the r/g/b values
 * are less than zero, the resulting color will not affect the visual map when
 * used for drawing
 *
 * @param[in] state		The current state
 * @param[in] color		Pointer to the structure to write to
 * @param[in] r			The red color intensity values of the result color
 * @param[in] g			The green color intensity values of the result color
 * @param[in] b			The blue color intensity values of the result color
 * @param[in] a			The alpha (transparency) value, with 0x00 meaning
 * 						absolutely opaque and 0xff meaning fully transparent.
 * 						Alpha blending support is optional for drivers, so these
 * 						are the only two values that are guaranteed to work as
 * 						intended. Any value in between them must guarantee the
 * 						following opaqueness: opaqueness(x-1) >= opaqueness(x)
 * 						>= opaqueness (x+1) (i.e. ([0,255],
 * 						less-transparent-than) must define a partial order)
 * @param[in] priority	The priority to use for drawing, or -1 for none
 * @param[in] control	The control to use for drawing, or -1 to disable drawing
 * 						to the control map
 */
void gfxop_set_color(GfxState *state, gfx_color_t *color, int r, int g, int b,
	int a, int priority, int control);

/** @} */

/** @name View operations */
/** @{ */

/**
 * Retrieves the width and height of a cel.
 *
 * @param[in] state		The state to use
 * @param[in] nr		Number of the view
 * @param[in] loop		Loop number to examine
 * @param[in] cel		The cel (inside the loop) to look up
 * @param[in] width		The variable the width will be stored in
 * @param[in] height	The variable the height will be stored in
 * @param[in] offset	The variable the cel's x/y offset will be stored in
 */
void gfxop_get_cel_parameters(GfxState *state, int nr, int loop, int cel,
	int *width, int *height, Common::Point *offset);

/**
 * Draws (part of) a cel to the back buffer.
 *
 * @param[in] state		The state encapsulating the driver to draw with
 * @param[in] nr		Number of the view to draw
 * @param[in] loop		Loop of the cel to draw
 * @param[in] cel		The cel number of the cel to draw
 * @param[in] pos		The positino the cel is to be drawn to
 * @param[in] color		The priority and control values to use for drawing
 * @param[in] palette	The palette to use
 */
void gfxop_draw_cel(GfxState *state, int nr, int loop, int cel,
	Common::Point pos, gfx_color_t color, int palette);


/**
 * Draws a cel to the static buffer; no clipping is performed.
 *
 * No clipping (except for the display borders) is performed.
 *
 * @param[in] state		The state encapsulating the driver to draw with
 * @param[in] nr		Number of the view to draw
 * @param[in] loop		Loop of the cel to draw
 * @param[in] cel		The cel number of the cel to draw
 * @param[in] pos		The positino the cel is to be drawn to
 * @param[in] color		The priority and control values to use for drawing
 * @param[in] palette	The palette to use
 */
void gfxop_draw_cel_static(GfxState *state, int nr, int loop, int cel,
	Common::Point pos, gfx_color_t color, int palette);


/**
 * Draws (part of) a clipped cel to the static buffer.
 *
 * This function does clip.
 *
 * @param[in] state		The state encapsulating the driver to draw with
 * @param[in] nr		Number of the view to draw
 * @param[in] loop		Loop of the cel to draw
 * @param[in] cel		The cel number of the cel to draw
 * @param[in] pos		The positino the cel is to be drawn to
 * @param[in] color		The priority and control values to use for drawing
 * @param[in] palette	The palette to use
 */
void gfxop_draw_cel_static_clipped(GfxState *state, int nr, int loop, int cel,
	Common::Point pos, gfx_color_t color, int palette);
/** @} */


/** @name Pic operations
 * These operations are exempt from clipping */
/** @{ */

/**
 * Draws a pic and writes it over the static buffer.
 *
 * This function instructs the resource manager to tag all data as "unused".
 * See the resource manager tag functions for a full description.
 *
 * @param[in] state				The state affected
 * @param[in] nr				Number of the pic to draw
 * @param[in] flags				Interpreter-dependant flags to use for drawing
 * @param[in] default_palette	The default palette for drawing
 */
void gfxop_new_pic(GfxState *state, int nr, int flags, int default_palette);

/**
 * Adds a pic to the static buffer.
 *
 * @param[in] state				The state affected
 * @param[in] nr				Number of the pic to add
 * @param[in] flags				Interpreter-dependant flags to use for drawing
 * @param[in] default_palette	The default palette for drawing
 */
void gfxop_add_to_pic(GfxState *state, int nr, int flags, int default_palette);
/** @} */


/** @name Text operations */

/**
 * Calculates the width and height of a specified text in a specified
 * font.
 *
 * @param[in]  state			The state to use
 * @param[in]  font_nr			Font number to use for the calculation
 * @param[in]  text				The text to examine
 * @param[in]  flags			ORred GFXR_FONT_FLAGs
 * @param[in]  maxwidth			The maximum pixel width to allow for the text
 * @param[out] width			The resulting width
 * @param[out] height			The resulting height
 * @param[out] lines_nr			Number of lines used in the text
 * @param[out] lineheight		Pixel height (SCI scale) of each text line
 * @param[out] lastline_width	Pixel offset (SCI scale) of the space after
 * 								the last character in the last line
 */
void gfxop_get_text_params(GfxState *state, int font_nr, const char *text,
	int maxwidth, int *width, int *height, int flags,
	int *lines_nr, int *lineheight, int *lastline_width);

/**
 * Generates a new text handle that can be used to draw any text.
 *
 * The control and priority values for the text will be extracted from color1.
 * Note that the colors must have been allocated properly, or the text may
 * display in incorrect colors.
 *
 * @param[in] state		The state to use
 * @param[in] font_nr	Font number to use for the calculation
 * @param[in] text		The text to examine
 * @param[in] maxwidth: The maximum pixel width to allow for the text
 * @param[in] halign	The horizontal text alignment
 * @param[in] valign	The vertical text alignment
 * @param[in] color1	The text's foreground colors (the function will dither
 * 						between color1 and 2)
 * @param[in] color2	The text's foreground colors (the function will dither
 * 						between color1 and 2)
 * @param[in] bg_color	The background color
 * @param[in] flags		ORred GFXR_FONT_FLAGs
 * @return				A newly allocated TextHandle, or NULL if font_nr was
 * 						invalid
 */
TextHandle *gfxop_new_text(GfxState *state, int font_nr,
	const Common::String &text, int maxwidth, gfx_alignment_t halign,
	gfx_alignment_t valign, gfx_color_t color1,	gfx_color_t color2,
	gfx_color_t bg_color, int flags);

/**
 * Draws text stored in a text handle.
 *
 * @param[in] state		The target state
 * @param[in] handle	The text handle to use for drawing
 * @param[in] zone		The rectangular box to draw to. In combination with
 * 						halign and valign, this defines where the text is drawn
 * 						to.
 */
void gfxop_draw_text(GfxState *state, TextHandle *handle, rect_t zone);
/** @} */


/** @name Manual pixmap operations */
/** @{ */

/**
 * Grabs a screen section from the back buffer and stores it in a pixmap.
 *
 * Obviously, this only affects the visual map
 *
 * @param[in] state	The affected state
 * @param[in] area	The area to grab
 * Returns			A result pixmap, or NULL on error
 */
gfx_pixmap_t *gfxop_grab_pixmap(GfxState *state, rect_t area);

/**
 * Draws part of a pixmap to the screen.
 *
 * @param[in] state	The affected state
 * @param[in] pxm	The pixmap to draw
 * @param[in] zone	The segment of the pixmap to draw
 * @param[in] pos	The position the pixmap should be drawn to
 */
void gfxop_draw_pixmap(GfxState *state, gfx_pixmap_t *pxm, rect_t zone,
	Common::Point pos);

/** @} */


/** @name Dirty rectangle operations */
/** @{ */

/**
 * Adds a dirty rectangle to 'base' according to a strategy.
 *
 * @param[in] list		the list to add to
 * @param[in] box		the dirty frame to addable
 */
void gfxdr_add_dirty(DirtyRectList &list, rect_t box);

/**
 * Clips a rectangle against another one.
 *
 * @param[in] rect		The rectangle to clip
 * @param[in] clipzone	The outer bounds rect must be in
 * @return				1 if rect is empty now, 0 otherwise
 */
int _gfxop_clip(rect_t *rect, rect_t clipzone);
/** @} */

#endif // INCLUDE_OLDGFX

/** @name Pointer and IO ops */
/** @{ */

/**
 * Suspends program execution for the specified amount of milliseconds.
 *
 * The mouse pointer will be redrawn continually, if applicable
 *
 * @param[in] state	The state affected
 * @param[in] msecs	The amount of milliseconds to wait
 */
void gfxop_sleep(GfxState *state, uint32 msecs);

/**
 * Retrieves the next input event from the driver.
 *
 * @param[in] state	The affected state
 * @param[in] mask	The event mask to poll from (see uinput.h)
 * @return			The next event in the driver's event queue, or a NONE event
 * 					if no event matching the mask was found.
 */
sci_event_t gfxop_get_event(GfxState *state, unsigned int mask);
/** @} */


} // End of namespace Sci

#endif // SCI_GFX_GFX_OPERATIONS_H
