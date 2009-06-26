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

#ifndef SCI_GFX_GFX_DRIVER_H
#define SCI_GFX_GFX_DRIVER_H

#include "sci/gfx/gfx_system.h"
#include "sci/uinput.h"

#include "graphics/pixelformat.h"

namespace Sci {

enum gfx_buffer_t {
	GFX_BUFFER_FRONT = 0,
	GFX_BUFFER_BACK = 1,
	GFX_BUFFER_STATIC = 2
};


/* Principial graphics driver architecture
** ---------------------------------------
**
** All graphics drivers must provide
** - One visual front buffer (the actually visible thing)
** - Two dynamic back buffers:
**   + visual
**   + priority
** - Two static buffers  (containing the background image and picviews):
**   + visual
**   + priority
**
** The control buffer is handled outside the graphics driver architecture.
** Graphics are drawn by first setting the static buffers, then updating
** the back buffers (from the static buffers), adding all picviews and other
** widgets, and finally updating the front buffer.
**
** All coordinates refer to the scaled coordinate system.
** Invalid parameters should produce an error message.
** Support for some valid parameter values is optional (like different line
** modes). If an unsupported but valid parameter is specified, the function
** must use a reasonable default value.
*/

class GfxDriver {
public:
	/*** Initialization ***/

	GfxDriver(int xfact, int yfact, Graphics::PixelFormat mode);
	/* Attempts to initialize a specific graphics mode
	** Parameters: (int x int) xres, yres: Horizontal and vertical scaling
	**                         factors
	**             (PixelFormat) mode: The mode to use
	** Returns   : (int) GFX_OK on success, GFX_ERROR if the mode could not be
	**                   set, or GFX_FATAL if the graphics target is unuseable.
	** The scaling factors apply to the standard SCI resolution of 320x200 pixels
	** and is used for internal representation of graphical data. The physical
	** resolution set by the graphics driver may be different for practical
	** reasons.
	** Must also set _mode, preferably with the gfx_new_mode() function
	** specified in gfx_tools.h.
	*/

	~GfxDriver();
	/* Uninitializes the current graphics mode
	** This function frees all memory allocated by the graphics driver,
	** including mode and palette information, uninstalls all console commands
	** introduced by preceeding init() or init_specific() commands, and does any
	** clean-up work (like closing visuals or returning to text mode) required by
	** the graphics infrastructure used.
	*/


	/*** Drawing operations ***/

	int drawLine(Common::Point start, Common::Point end, gfx_color_t color,
	             gfx_line_mode_t line_mode, gfx_line_style_t line_style);
	/* Draws a single line to the back buffer.
	** Parameters: (Common::Point) start: Starting point of the line to draw
	**             (Common::Point) end: End point of the line to draw
	**             (gfx_color_t *) color: The color to draw with
	**             (int) line_mode: Any of the line modes
	**             (int) line_style: Any of the line styles
	** Returns   : (int) GFX_OK or GFX_FATAL
	** Note that color.priority is relevant and must be drawn if
	** (color.mask & GFX_MASK_PRIORITY).
	** Support for line modes other than GFX_LINE_MODE_FAST is optional.
	** For non-fine lines, the coordinates provided describe the upper left
	** corner of the pixels of the line to draw.
	** line_style support is optional, if GFX_CAPABILITY_STIPPLED_LINES is not
	** set.
	*/

	int drawFilledRect(rect_t rect, gfx_color_t color1, gfx_color_t color2,
	                   gfx_rectangle_fill_t shade_mode);
	/* Draws a single filled and possibly shaded rectangle to the back buffer.
	** Parameters: (rect_t *) rect: The rectangle to draw
	**             (gfx_color_t *) color1, color2: The colors to draw with
	**             (int) shade_mode: Any of GFX_SHADE_*.
	** Returns   : (int) GFX_OK or GFX_FATAL
	** Note that color.priority is relevant and must be drawn if
	** (color.mask & GFX_MASK_PRIORITY).
	** color2 is relevant only if shade_mode is not GFX_SHADE_FLAT.
	** Support for shade modes other than GFX_SHADE_FLAT is optional.
	*/

	/*** Pixmap operations ***/

	int drawPixmap(gfx_pixmap_t *pxm, int priority,
	               rect_t src, rect_t dest, gfx_buffer_t buffer);
	/* Draws part of a pixmap to the static or back buffer
	** Parameters: (gfx_pixmap_t *) pxm: The pixmap to draw
	**             (int) priority: The priority to draw with, or GFX_NO_PRIORITY
	**                   to draw on top of everything without setting the
	**                   priority back buffer
	**             (rect_t) src: The pixmap-relative source rectangle
	**             (rect_t) dest: The destination rectangle
	**             (int) buffer: One of GFX_BUFFER_STATIC and GFX_BUFFER_BACK
	** Returns   : (int) GFX_OK or GFX_FATAL, or GFX_ERROR if pxm was not
	**                   (but should have been) registered.
	*/

	int grabPixmap(rect_t src, gfx_pixmap_t *pxm, gfx_map_mask_t map);
	/* Grabs an image from the visual or priority back buffer
	** Parameters: (rect_t) src: The rectangle to grab
	**             (gfx_pixmap_t *) pxm: The pixmap structure the data is to
	**                              be written to
	**             (int) map: GFX_MASK_VISUAL or GFX_MASK_PRIORITY
	** Returns   : (int) GFX_OK, GFX_FATAL, or GFX_ERROR for invalid map values
	** pxm may be assumed to be empty and pre-allocated with an appropriate
	** memory size.
	** This function is now mandatory.
	*/


	/*** Buffer operations ***/

	int update(rect_t src, Common::Point dest, gfx_buffer_t buffer);
	/* Updates the front buffer or the back buffers
	** Parameters: (rect_t) src: Source rectangle
	**             (Common::Point) dest: Destination point
	**             (int) buffer: One of GFX_BUFFER_FRONT or GFX_BUFFER_BACK
	** Returns   : (int) GFX_OK, GFX_ERROR or GFX_FATAL
	** This function updates either the visual front buffer, or the two back
	** buffers, by copying the specified source region to the destination
	** region.
	** For heuristical reasons, it may be assumed that the x and y fields of
	** src and dest will be identical in /most/ cases.
	** If they aren't, the priority map will not be required to be copied.
	*/

	int setStaticBuffer(gfx_pixmap_t *pic, gfx_pixmap_t *priority);
	/* Sets the contents of the static visual and priority buffers
	** Parameters: (gfx_pixmap_t *) pic: The image defining the new content
	**                              of the visual back buffer
	**             (gfx_pixmap_t *) priority: The priority map containing
	**                              the new content of the priority back buffer
	**                              in the index buffer
	** Returns   : (int) GFX_OK or GFX_FATAL
	** pic and priority may be modified or written to freely. They may also be
	** used as the actual static buffers, since they are not freed and re-
	** allocated between calls to set_static_buffer() and update(), unless
	** exit() was called in between.
	** Note that later version of the driver interface may disallow modifying
	** pic and priority.
	** pic and priority are always scaled to the appropriate resolution
	*/


	/*** Mouse pointer operations ***/

	int setPointer(gfx_pixmap_t *pointer, Common::Point *hotspot);
	/* Sets a new mouse pointer.
	** Parameters: (gfx_pixmap_t *) pointer: The pointer to set, or NULL to set
	**                              no pointer
	**             (Common::Point *) hotspot: The coordinates of the hotspot,
	**                               or NULL to set no pointer
	** Returns   : (int) GFX_OK or GFX_FATAL
	** If pointer is not NULL, it will have been scaled to the appropriate
	** size and registered as a pixmap (if neccessary) beforehand.
	** If this function is called for a target that supports only two-color
	** pointers, the image is a color index image, where only color index values
	** 0, 1, and GFX_COLOR_INDEX_TRANSPARENT are used.
	*/

	gfx_mode_t *getMode() { return _mode; }
	byte *getVisual0() { return _visual[0]; }

private:
	byte *createCursor(gfx_pixmap_t *pointer);

	gfx_pixmap_t *_priority[2];
	byte *_visual[2];
	gfx_mode_t *_mode; /* Currently active mode, NULL if no mode is active */
};

} // End of namespace Sci

#endif // SCI_GFX_GFX_DRIVER_H
