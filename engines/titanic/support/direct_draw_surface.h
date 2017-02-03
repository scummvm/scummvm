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
 */

#ifndef TITANIC_DIRECT_DRAW_SURFACE_H
#define TITANIC_DIRECT_DRAW_SURFACE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "graphics/managed_surface.h"
#include "titanic/support/rect.h"

namespace Titanic {

class TitanicEngine;

struct DDSurfaceDesc {
	int _w;
	int _h;
	int _bpp;
	int _flags;
	int _caps;

	DDSurfaceDesc(int w, int h, int bpp) : _w(w), _h(h), _bpp(bpp),
		_flags(0x1006), _caps(64) {}
};

class DirectDrawSurface {
private:
	Graphics::ManagedSurface *_surface;
	DisposeAfterUse::Flag _disposeAfterUse;
public:
	DirectDrawSurface();
	~DirectDrawSurface();

	/**
	 * Create a surface
	 */
	void create(int w, int h, int bpp);

	/**
	 * Create a surface based on a passed surface
	 */
	void create(Graphics::ManagedSurface *surface);

	/**
	 * Frees the surface
	 */
	void free();

	/**
	 * Return the size of the surface in ytes
	 */
	int getSize() const { return _surface->pitch * _surface->h; }

	/**
	 * Return the surface width
	 */
	int getWidth() const { return _surface->w; }

	/**
	 * Return the surface width
	 */
	int getHeight() const { return _surface->h; }

	/**
	 * Return the surface pitch
	 */
	int getPitch() const { return _surface->pitch; }

	/**
	 * Return the surface's format
	 */
	const Graphics::PixelFormat &getFormat() { return _surface->format; }

	/**
	 * Lock the surface for access
	 */
	Graphics::ManagedSurface *lock(const Rect *bounds, int flags);

	/**
	 * Unlocks the surface at the end of direct accesses
	 */
	void unlock();

	/**
	 * Fills an area of the surfae with the specified color. If no bounds are passed,
	 * then the entire surface is filled
	 */
	void fill(const Rect *bounds, uint32 color);

	/**
	 * Fill an area with a specific color
	 */
	void fillRect(Rect *rect, byte r, byte g, byte b);

	/**
	 * Copy data from a source surfcae into this one
	 */
	void blit(const Rect &destRect, DirectDrawSurface *srcSurface, Rect &srcRect);

	/**
	 * Copy data from a source surfcae into this one
	 */
	void blit(const Point &destPos, DirectDrawSurface *srcSurface, Rect *bounds);
};

} // End of namespace Titanic

#endif /* TITANIC_DIRECT_DRAW_SURFACE_H */
