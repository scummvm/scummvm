/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#ifndef GRAPHICS_SCALER_H
#define GRAPHICS_SCALER_H

#include "common/sys.h"
#include "graphics/surface.h"


// creates a 160x100 thumbnail for 320x200 games
// and 160x120 thumbnail for 320x240 and 640x480 games
// only 565 mode
enum {
	kThumbnailWidth = 160,
	kThumbnailHeight1 = 100,
	kThumbnailHeight2 = 120
};

/**
 * Creates a thumbnail from the current screen (without overlay).
 *
 * @param surf	a surface (will always have 16 bpp after this for now)
 * @return		false if a error occured
 */
//extern bool createThumbnailFromScreen(Graphics::Surface *surf);

/**
 * Creates a thumbnail from a buffer.
 *
 * @param surf      destination surface (will always have 16 bpp after this for now)
 * @param pixels    raw pixel data
 * @param w         width
 * @param h         height
 * @param palette   palette in RGB format
 */
//extern bool createThumbnail(Graphics::Surface *surf, const uint8 *pixels, int w, int h, const uint8 *palette);

#endif
