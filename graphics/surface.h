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

#ifndef GRAPHICS_SURFACE_H
#define GRAPHICS_SURFACE_H

#include "common/sys.h"
#include "common/rect.h"

namespace Graphics {

/**
 * An arbitrary graphics surface, which can be the target (or source) of blit
 * operations, font rendering, etc.
 */
struct Surface {
	/**
	 * ARM code relies on the layout of the first 3 of these fields. Do
	 * not change them.
	 */
	uint16 w;
	uint16 h;
	uint16 pitch;
	void *pixels;
	uint8 bytesPerPixel;
	Surface() : w(0), h(0), pitch(0), pixels(0), bytesPerPixel(0) {}

	inline const void *getBasePtr(int x, int y) const {
		return (const byte *)(pixels) + y * pitch + x * bytesPerPixel;
	}

	inline void *getBasePtr(int x, int y) {
		return static_cast<byte *>(pixels) + y * pitch + x * bytesPerPixel;
	}

	/**
	 * Allocate pixels memory for this surface and for the specified dimension.
	 */
	void create(uint16 width, uint16 height, uint8 bytesPP);

	/**
	 * Release the memory used by the pixels memory of this surface. This is the
	 * counterpart to create().
	 */
	void free();

	/**
	 * Copies data from another Surface, this calls *free* on the current surface, to assure
	 * it being clean.
	 */
	void copyFrom(const Surface &surf);

	void drawLine(int x0, int y0, int x1, int y1, uint32 color);
	void hLine(int x, int y, int x2, uint32 color);
	void vLine(int x, int y, int y2, uint32 color);
	void fillRect(Common::Rect r, uint32 color);
	void frameRect(const Common::Rect &r, uint32 color);
	// See comment in graphics/surface.cpp about it
	void move(int dx, int dy, int height);
};

/**
 * For safe deletion of surface with SharedPtr.
 * The deleter assures Surface::free is called on
 * deletion.
 */
struct SharedPtrSurfaceDeleter {
	void operator()(Surface *ptr) {
		ptr->free();
		delete ptr;
	}
};


} // End of namespace Graphics


#endif
