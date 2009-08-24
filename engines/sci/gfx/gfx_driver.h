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


/**
 * Graphics driver.
 *
 * Principial graphics driver architecture:
 *
 * All graphics drivers must provide
 * - One visual front buffer (the actually visible thing)
 * - Two dynamic back buffers:
 *   - visual
 *   - priority
 * - Two static buffers  (containing the background image and picviews):
 *   - visual
 *   - priority
 *
 * The control buffer is handled outside the graphics driver architecture.
 * Graphics are drawn by first setting the static buffers, then updating
 * the back buffers (from the static buffers), adding all picviews and other
 * widgets, and finally updating the front buffer.
 *
 * All coordinates refer to the scaled coordinate system.
 * Invalid parameters should produce an error message.
 * Support for some valid parameter values is optional (like different line
 * modes). If an unsupported but valid parameter is specified, the function
 * must use a reasonable default value.
 */
class GfxDriver {
public:
	/** @name Initialization */
	/** @{ */
	/**
	 * Attempts to initialize a specific graphics mode.
	 *
	 * The scaling factors apply to the standard SCI resolution of 320x200
	 * pixels and is used for internal representation of graphical data.
	 * The physical resolution set by the graphics driver may be different
	 * for practical reasons.
	 * Must also set _mode, preferably with the gfx_new_mode() function
	 * specified in gfx_tools.h.
	 *
	 * @param[in] xfact		Horizontal scaling factor
	 * @param[in] yfact		Vertical scaling factor
	 * @param[in] bytespp	Any of GFX_COLOR_MODE_*. GFX_COLOR_MODE_INDEX
	 * 						implies color index mode.
	 * @return				GFX_OK on success, GFX_ERROR if the mode could
	 * 						not be set, or GFX_FATAL if the graphics target
	 * 						is unuseable.
	 */
	GfxDriver(int xfact, int yfact, Graphics::PixelFormat mode);

	/**
	 * Uninitializes the current graphics mode.
	 *
	 * This function frees all memory allocated by the graphics driver,
	 * including mode and palette information, uninstalls all console
	 * commands introduced by preceeding init() or init_specific()
	 * commands, and does any clean-up work (like closing visuals or
	 * returning to text mode) required by the graphics infrastructure used.
	 */
	~GfxDriver();
	/** @} */

	/** @name Drawing operations */
	/** @{ */

	/**
	 * Draws a single line to the back buffer.
	 *
	 * Note that color.priority is relevant and must be drawn if
	 * (color.mask & GFX_MASK_PRIORITY). Support for line modes other than
	 * GFX_LINE_MODE_FAST is optional. For non-fine lines, the coordinates
	 * provided describe the upper left corner of the pixels of the line
	 * to draw.line_style support is optional, if
	 * GFX_CAPABILITY_STIPPLED_LINES is not set.
	 *
	 * @param[in] start	Starting point of the line to draw
	 * @param[in] end			End point of the line to draw
	 * @param[in] color			The color to draw with
	 * @param[in] line_mode		Any of the line modes
	 * @param[in] line_style	Any of the line styles
	 * @return					GFX_OK or GFX_FATAL
	 */
	int drawLine(Common::Point start, Common::Point end, gfx_color_t color,
	             gfx_line_mode_t line_mode, gfx_line_style_t line_style);

	/**
	 * Draws a single filled and possibly shaded rectangle to the back
	 * buffer.
	 *
	 * Note that color.priority is relevant and must be drawn if
	 * (color.mask & GFX_MASK_PRIORITY). color2 is relevant only if
	 * shade_mode is not GFX_SHADE_FLAT. Support for shade modes other
	 * than GFX_SHADE_FLAT is optional.
	 *
	 * @param[in] rect			The rectangle to draw
	 * @param[in] color1		The first color to draw with
	 * @param[in] color2		The second color to draw with
	 * @param[in] shade_mode	Any of GFX_SHADE_*.
	 * @return					GFX_OK or GFX_FATAL
	 */
	int drawFilledRect(rect_t rect, gfx_color_t color1, gfx_color_t color2,
	                   gfx_rectangle_fill_t shade_mode);
	/** @} */

	/** @name Pixmap operations */
	/** @{ */

	/**
	 * Draws part of a pixmap to the static or back buffer.
	 *
	 * @param[in] pxm		The pixmap to draw
	 * @param[in] priority	The priority to draw with, or GFX_NO_PRIORITY
	 *						to draw on top of everything without setting the
	 * 						priority back buffer.
	 * @param[in] src		The pixmap-relative source rectangle
	 * @param[in] dest		The destination rectangle
	 * @param[in] buffer	One of GFX_BUFFER_STATIC and GFX_BUFFER_BACK
	 * @return				GFX_OK or GFX_FATAL, or GFX_ERROR if pxm was
	 * 						not (but should have been) registered.
	 */
	int drawPixmap(gfx_pixmap_t *pxm, int priority,
	               rect_t src, rect_t dest, gfx_buffer_t buffer);

	/**
	 * Grabs an image from the visual or priority back buffer.
	 *
	 * This function is now mandatory.
	 *
	 * @param[in] src	The rectangle to grab
	 * @param[in] pxm	The pixmap structure the data is to be written to
	 * @param[in] map	GFX_MASK_VISUAL or GFX_MASK_PRIORITY
	 * @return			GFX_OK, GFX_FATAL, or GFX_ERROR for invalid map
	 * 					values pxm may be assumed to be empty and
	 * 					pre-allocated with an appropriate memory size.
	 */
	int grabPixmap(rect_t src, gfx_pixmap_t *pxm, gfx_map_mask_t map);
	/** @} */

	/** @name Buffer operations */
	/** @{ */

	/**
	 * Updates the front buffer or the back buffers.
	 *
	 * This function updates either the visual front buffer, or the two
	 * back buffers, by copying the specified source region to the
	 * destination region.
	 * For heuristical reasons, it may be assumed that the x and y fields
	 * of src and dest will be identical in /most/ cases.If they aren't,
	 * the priority map will not be required to be copied.
	 *
	 * @param[in] src:		Source rectangle
	 * @param[in] dest:		Destination point
	 * @param[in] buffer:	One of GFX_BUFFER_FRONT or GFX_BUFFER_BACK
	 * @return				GFX_OK, GFX_ERROR or GFX_FATAL
	 */
	int update(rect_t src, Common::Point dest, gfx_buffer_t buffer);

	/**
	 * Sets the contents of the static visual and priority buffers.
	 *
	 * pic and priority may be modified or written to freely. They may also
	 * be used as the actual static buffers, since they are not freed and
	 * reallocated between calls to set_static_buffer() and update(),
	 * unless exit() was called in between.
	 * Note that later version of the driver interface may disallow
	 * modifying pic and priority. pic and priority are always scaled to
	 * the appropriate resolution
	 *
	 * @param[in] pic		The image defining the new content of the
	 * 						visual back buffer
	 * @param[in] priority	The priority map containing the new content of
	 * 						the priority back buffer in the index buffer
	 * @return				GFX_OK or GFX_FATAL
	 */
	int setStaticBuffer(gfx_pixmap_t *pic, gfx_pixmap_t *priority);
	/** @} */

	/** @name Mouse pointer operations */
	/** @{ */

	/**
	 * Sets a new mouse pointer.
	 *
	 * If pointer is not NULL, it will have been scaled to the appropriate
	 * size and registered as a pixmap (if neccessary) beforehand. If this
	 * function is called for a target that supports only two-color
	 * pointers, the image is a color index image, where only color index
	 * values 0, 1, and GFX_COLOR_INDEX_TRANSPARENT are used.
	 *
	 * @param[in] pointer	The pointer to set, or NULL to set no pointer.
	 * @param[in] hotspot	The coordinates of the hotspot, or NULL to set
	 * 						no pointer.
	 * @return				GFX_OK or GFX_FATAL
	 */
	int setPointer(gfx_pixmap_t *pointer, Common::Point *hotspot);
	/** @} */

	gfx_mode_t *getMode() { return _mode; }
	byte *getVisual0() { return _visual[0]; }

private:
	byte *createCursor(gfx_pixmap_t *pointer);

	gfx_pixmap_t *_priority[2];
	byte *_visual[2];
	gfx_mode_t *_mode; /**< Currently active mode, NULL if no mode is active */
};

} // End of namespace Sci

#endif // SCI_GFX_GFX_DRIVER_H
