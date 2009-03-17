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

/* FreeSCI 0.3.1+ graphics subsystem helper functions */


#ifndef SCI_GFX_GFX_TOOLS_H
#define SCI_GFX_GFX_TOOLS_H

#include "graphics/pixelformat.h"

#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_driver.h"

namespace Sci {

enum gfx_xlate_filter_t {
	GFX_XLATE_FILTER_NONE,
	GFX_XLATE_FILTER_LINEAR,
	GFX_XLATE_FILTER_TRILINEAR
};


extern int gfx_crossblit_alpha_threshold; /* Crossblitting functions use this value as threshold
					  ** for distinguishing between transparent and opaque
					  ** wrt alpha values */

gfx_mode_t *gfx_new_mode(int xfact, int yfact, const Graphics::PixelFormat &format, Palette *palette, int flags);
/* Allocates a new gfx_mode_t structure with the specified parameters
** Parameters: (int x int) xfact x yfact: Horizontal and vertical scaling factors
**             (Graphics::PixelFormat) format: pixel format description
**             (int) palette: Number of palette colors, 0 if we're not in palette mode
**             (int) flags: GFX_MODE_FLAG_* values ORred together, or just 0
** Returns   : (gfx_mode_t *) A newly allocated gfx_mode_t structure
*/


void gfx_clip_box_basic(rect_t *box, int maxx, int maxy);
/* Clips a rect_t
** Parameters: (rect_t *) box: Pointer to the box to clip
**             (int x int) maxx, maxy: Maximum allowed width and height
** Returns   : (void)
*/


void gfx_free_mode(gfx_mode_t *mode);
/* Frees all memory allocated by a mode structure
** Parameters: (gfx_mode_t *) mode: The mode to free
** Returns   : (void)
*/


gfx_pixmap_t *gfx_new_pixmap(int xl, int yl, int resid, int loop, int cel);
/* Creates a new pixmap structure
** Parameters: (int x int) xl x yl: The dimensions (in SCI coordinates) of the pixmap
**             (int) resid: The pixmap's resource ID, or GFX_RESID_NONE
**             (int) loop: For views: The pixmap's loop number
**             (int) cel: For cels: The pixmap's cel number
** Returns   : (gfx_pixmap_t *) The newly allocated pixmap
** The following fiels are initialized:
** ID, loop, cel, index_width, index_height, xl, yl, data <- NULL,
** alpha_map <- NULL, internal.handle <- 0, internal.info <- NULL, colors <- NULL,
** index_scaled <- 0
*/

gfx_pixmap_t *gfx_clone_pixmap(gfx_pixmap_t *pixmap, gfx_mode_t *mode);
/* Clones a pixmap, minus its index data, palette and driver-specific handles
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap to clone
**             (gfx_mode_t *) mode: The mode to be applied to the pixmap
** Returns   : (gfx_pixmap_t *) The clone
*/


gfx_pixmap_t *gfx_pixmap_alloc_index_data(gfx_pixmap_t *pixmap);
/* Allocates the index_data field of a pixmap
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap to allocate for
** Returns   : (gfx_pixmap_t *) pixmap
*/

gfx_pixmap_t *gfx_pixmap_free_index_data(gfx_pixmap_t *pixmap);
/* Frees the index_data field of a pixmap
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap to modify
** Returns   : (gfx_pixmap_t *) pixmap
*/

gfx_pixmap_t *gfx_pixmap_alloc_data(gfx_pixmap_t *pixmap, gfx_mode_t *mode);
/* Allocates the data field of a pixmap
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap to allocate for
**             (gfx_mode_t *) mode: The mode the memory is to be allocated for
** Returns   : (gfx_pixmap_t *) pixmap
*/

gfx_pixmap_t *gfx_pixmap_free_data(gfx_pixmap_t *pixmap);
/* Frees the memory allocated for a pixmap's data field
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap to modify
** Returns   : (gfx_pixmap_t *) pixmap
*/

void gfx_free_pixmap(gfx_pixmap_t *pxm);
/* Frees all memory associated with a pixmap
** Parameters: (gfx_pixmap_t *) pxm: The pixmap to free
** Returns   : (void)
*/

void gfx_draw_line_pixmap_i(gfx_pixmap_t *pxm, Common::Point start, Common::Point end, int color);
/* Draws a line to a pixmap's index data buffer
** Parameters: (gfx_pixmap_t *) pxm: The pixmap to draw to
**             (Common::Point) start: Starting point of the line to draw
**             (Common::Point) end: End point of the line to draw
**             (int) color: The byte value to write
** Returns   : (void)
** Remember, this only draws to the /index/ buffer, not to the drawable buffer.
** The line is not clipped. Invalid x, y, x1, y1 values will result in memory corruption.
*/

void gfx_draw_box_pixmap_i(gfx_pixmap_t *pxm, rect_t box, int color);
/* Draws a filled rectangular area to a pixmap's index buffer
** Parameters: (gfx_pixmap_t *) pxm: The pixmap to draw to
**             (rect_t) box: The box to fill
**             (int) color: The color to use for drawing
** Returns   : (void)
** This function only draws to the index buffer.
*/

void gfx_copy_pixmap_box_i(gfx_pixmap_t *dest, gfx_pixmap_t *src, rect_t box);
/* Copies part of a pixmap to another pixmap, with clipping
** Parameters: (gfx_pixmap_t *) dest: The destination pixmap
**             (gfx_pixmap_t *) src: The source pixmap
**             (rect_t) box: The area to copy
** Returns   : (void)
*/

void gfx_xlate_pixmap(gfx_pixmap_t *pxm, gfx_mode_t *mode, gfx_xlate_filter_t filter);
/* Translates a pixmap's index data to drawable graphics data
** Parameters: (gfx_pixmap_t *) pxm: The pixmap to translate
**             (gfx_mode_t *) mode: The mode according which to scale
**             (gfx_xlate_filter_t) filter: How to filter the data
** Returns   : (void)
*/

#define GFX_CROSSBLIT_FLAG_DATA_IS_HOMED (1<<0)
/* Means that the first byte in the visual data refers to the
** point corresponding to (dest.x, dest.y) */

int gfx_crossblit_pixmap(gfx_mode_t *mode, gfx_pixmap_t *pxm, int priority,
	rect_t src_coords, rect_t dest_coords, byte *dest, int dest_line_width,
	byte *priority_dest, int priority_line_width, int priority_skip, int flags);
/* Transfers the non-transparent part of a pixmap to a linear pixel buffer
** Parameters: (gfx_mode_t *) mode: The graphics mode of the target buffer
**             (gfx_pixmap_t *) pxm: The pixmap to transfer
**             (int priority): The pixmap's priority
**             (rect_t) src_coords: The source coordinates within the pixmap
**             (rect_t) dest_coords: The destination coordinates (no scaling)
**             (byte *) dest: Memory position of the upper left pixel of the
**                      linear pixel buffer
**             (int) dest_line_width: Byte offset of the very first pixel in the
**                                    second line of the linear pixel buffer,
**                                    relative to dest.
**             (byte *) priority_dest: Destination buffer for the pixmap's priority
**                                     values
**             (int) priority_line_width: Byte offset of the first pixel in the
**                                        second line of the priority buffer
**             (int) priority_skip: Amount of bytes allocated by each priority value
**             (int) flags: Any crossblit flags
** Returns   : (int) GFX_OK, or GFX_ERROR if the specified mode was invalid or unsupported
** A 'linear buffer' in this context means a data buffer containing an entire
** screen (visual or priority), with fixed offsets between each data row, and
** linear access.
*/

//int gfx_alloc_color(gfx_palette_t *pal, gfx_pixmap_color_t *color);
/* Allocates a color entry for the specified pixmap color
** Parameters: (gfx_palette_t *) pal: The palette structure the color should be allocated in
**             (gfx_pixmap_color_t *) color: The color to allocate
** Returns   : (int) GFX_ERROR if any error occured, GFX_OK if the color could be mapped to an
**                   existing color or a positive value if a new color was allocated in the
**                   palette.
*/

//int gfx_free_color(gfx_palette_t *pal, gfx_pixmap_color_t *color);
/* Frees the color entry allocated for the specified pixmap color
** Parameters: (gfx_palette_t *) pal: The palette structure the color was previously allocated in
**             (gfx_pixmap_color_t *) color: The color to free
** Returns   : (int) GFX_ERROR if any error occured, GFX_OK otherwise
*/

gfx_pixmap_t *gfx_pixmap_scale_index_data(gfx_pixmap_t *pixmap, gfx_mode_t *mode);
/* Scales the index data associated with a pixmap
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap whose index data should be scaled
**             (gfx_mode_t *) mode: The mode to scale it to
** Returns   : (gfx_pixmap_t *) pixmap
*/

} // End of namespace Sci

#endif // SCI_GFX_GFX_TOOLS_H
