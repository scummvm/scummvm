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

#ifndef SCI_GFX_GFX_TOOLS_H
#define SCI_GFX_GFX_TOOLS_H

#include "graphics/pixelformat.h"

#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_driver.h"

namespace Sci {

/** @name SCI graphics subsystem helper functions */
/** @{ */

/**
 * Allocates a new gfx_mode_t structure with the specified parameters
 *
 * @param[in] xfact		Horizontal scaling factors
 * @param[in] yfact		Vertical scaling factors
 * @param[in] palette	Number of palette colors, 0 if we're not in palette mode
 * @return				A newly allocated gfx_mode_t structure
 */
gfx_mode_t *gfx_new_mode(int scaleFactor, Palette *palette);

/**
 * Clips a rect_t
 *
 * @param[in] box	Pointer to the box to clip
 * @param[in] maxx	Maximum allowed width
 * @param[in] maxy	Maximum allowed height
 */
void gfx_clip_box_basic(rect_t *box, int maxx, int maxy);

/**
 * Frees all memory allocated by a mode structure
 * @param[in] mode	The mode to free
 */
void gfx_free_mode(gfx_mode_t *mode);

/**
 * Creates a new pixmap structure
 *
 * The following fiels are initialized:
 * ID, loop, cel, index_width, index_height, xl, yl, data <- NULL,
 * alpha_map <- NULL, internal.handle <- 0, internal.info <- NULL,
 * colors <- NULL, index_scaled <- 0
 *
 * @param[in] xl	Width (in SCI coordinates) of the pixmap
 * @param[in] yl	Height (in SCI coordinates) of the pixmap
 * @param[in] resid	The pixmap's resource ID, or GFX_RESID_NONE
 * @param[in] loop	For views: The pixmap's loop number
 * @param[in] cel	For cels: The pixmap's cel number
 * @return			The newly allocated pixmap
 */
gfx_pixmap_t *gfx_new_pixmap(int xl, int yl, int resid, int loop, int cel);

/**
 * Clones a pixmap, minus its index data, palette and driver-specific
 * handles
 *
 * @param[in] pixmap	The pixmap to clone
 * @param[in] mode		The mode to be applied to the pixmap
 * @return				The clone
 */
gfx_pixmap_t *gfx_clone_pixmap(gfx_pixmap_t *pixmap, gfx_mode_t *mode);

/**
 * Allocates the index_data field of a pixmap
 *
 * @param[in] pixmap	The pixmap to allocate for
 * @return				The pixmap
 */
gfx_pixmap_t *gfx_pixmap_alloc_index_data(gfx_pixmap_t *pixmap);

/**
 * Frees the index_data field of a pixmap
 *
 * @param[in] pixmap	The pixmap to modify
 * @return				The pixmap
 */
gfx_pixmap_t *gfx_pixmap_free_index_data(gfx_pixmap_t *pixmap);

/**
 * Allocates the data field of a pixmap
 *
 * @param[in] pixmap	The pixmap to allocate for
 * @param[in] mode		The mode the memory is to be allocated for
 * @return				The pixmap
 */
gfx_pixmap_t *gfx_pixmap_alloc_data(gfx_pixmap_t *pixmap, gfx_mode_t *mode);

/**
 * Frees the memory allocated for a pixmap's data field
 *
 * @param[in] pixmap	The pixmap to modify
 * @return				The pixmap
 */
gfx_pixmap_t *gfx_pixmap_free_data(gfx_pixmap_t *pixmap);

/**
 * Frees all memory associated with a pixmap
 *
 * @param[in] pxm	The pixmap to free
 */
void gfx_free_pixmap(gfx_pixmap_t *pxm);

/**
 * Draws a line to a pixmap's index data buffer
 *
 * Remember, this only draws to the /index/ buffer, not to the drawable buffer.
 * The line is not clipped. Invalid x, y, x1, y1 values will result in memory
 * corruption.
 *
 * @param[in] pxm		The pixmap to draw to
 * @param[in] start		Starting point of the line to draw
 * @param[in] end		End point of the line to draw
 * @param[in] color		The byte value to write
 */
void gfx_draw_line_pixmap_i(gfx_pixmap_t *pxm, Common::Point start,
	Common::Point end, int color);

void gfx_draw_line_buffer(byte *buffer, int linewidth, int pixelwidth, 
	Common::Point start, Common::Point end, unsigned int color);

/**
 * Draws a filled rectangular area to a pixmap's index buffer
 *
 * This function only draws to the index buffer.
 *
 * @param[in] pxm	The pixmap to draw to
 * @param[in] box	The box to fill
 * @param[in] color	The color to use for drawing
 */
void gfx_draw_box_pixmap_i(gfx_pixmap_t *pxm, rect_t box, int color);

void gfx_draw_box_buffer(byte *buffer, int linewidth, rect_t zone, int color);

/**
 * Copies part of a pixmap to another pixmap, with clipping
 *
 * @param[in] dest	The destination pixmap
 * @param[in] src	The source pixmap
 * @param[in] box	The area to copy
 */
void gfx_copy_pixmap_box_i(gfx_pixmap_t *dest, gfx_pixmap_t *src, rect_t box);

/**
 * Translates a pixmap's index data to drawable graphics data
 *
 * @param[in] pxm		The pixmap to translate
 * @param[in] mode		The mode according which to scale
 */
void gfx_xlate_pixmap(gfx_pixmap_t *pxm, gfx_mode_t *mode);

#define GFX_CROSSBLIT_FLAG_DATA_IS_HOMED (1<<0) /**< Means that the first byte in the visual data refers to the point corresponding to (dest.x, dest.y) */

/**
 * Transfers the non-transparent part of a pixmap to a linear pixel
 * buffer.
 *
 * A 'linear buffer' in this context means a data buffer containing an entire
 * screen (visual or priority), with fixed offsets between each data row, and
 * linear access.
 *
 * @param[in] mode					The graphics mode of the target buffer
 * @param[in] pxm					The pixmap to transfer
 * @param[in] priority				The pixmap's priority
 * @param[in] src_coords			The source coordinates within the pixmap
 * @param[in] dest_coords			The destination coordinates (no scaling)
 * @param[in] dest					Memory position of the upper left pixel of
 * 									the linear pixel buffer
 * @param[in] dest_line_width		Byte offset of the very first pixel in the
 * 									second line of the linear pixel buffer,
 * 									relative to dest.
 * @param[in] priority_dest			Destination buffer for the pixmap's priority
 * 									values
 * @param[in] priority_line_width	Byte offset of the first pixel in the second
 * 									line of the priority buffer
 * @param[in] priority_skip			Amount of bytes allocated by each priority
 * 									value
 * @param[in] flags					Any crossblit flags
 */
void gfx_crossblit_pixmap(gfx_mode_t *mode, gfx_pixmap_t *pxm, int priority,
	rect_t src_coords, rect_t dest_coords, byte *dest, int dest_line_width,
	byte *priority_dest, int priority_line_width, int priority_skip, int flags);


/**
 * Scales the index data associated with a pixmap
 *
 * @param[in] pixmap	The pixmap whose index data should be scaled
 * @param[in] mode		The mode to scale it to
 * @return				The pixmap
 */
gfx_pixmap_t *gfx_pixmap_scale_index_data(gfx_pixmap_t *pixmap, gfx_mode_t *mode);

/** @} */
} // End of namespace Sci

#endif // SCI_GFX_GFX_TOOLS_H
