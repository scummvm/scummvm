/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GRAPHICS_SCALER_H
#define GRAPHICS_SCALER_H

#include "common/scummsys.h"
#include "graphics/managed_surface.h"
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
 * @return		false if a error occurred
 */
extern bool createThumbnailFromScreen(Graphics::Surface *surf);

/**
 * Creates a thumbnail from a buffer.
 *
 * @param surf      destination surface (will always have 16 bpp after this for now)
 * @param pixels    raw pixel data
 * @param w         width
 * @param h         height
 * @param palette   palette in RGB format
 */
extern bool createThumbnail(Graphics::Surface *surf, const uint8 *pixels, int w, int h, const uint8 *palette);

/**
 * Creates a thumbnail from a ManagedSurface.
 *
 * @param surf	destination surface (will always have 16 bpp after this for now)
 * @param in	source surface to create thumbnail from
 */
extern bool createThumbnail(Graphics::Surface *surf, Graphics::ManagedSurface *in);

#endif
