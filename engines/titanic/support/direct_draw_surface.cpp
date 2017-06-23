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

#include "titanic/support/direct_draw_surface.h"

namespace Titanic {

DirectDrawSurface::DirectDrawSurface() : _surface(nullptr),
		_disposeAfterUse(DisposeAfterUse::YES) {
}

DirectDrawSurface::~DirectDrawSurface() {
	free();
}

void DirectDrawSurface::create(Graphics::ManagedSurface *surface) {
	free();
	_surface = surface;
	_disposeAfterUse = DisposeAfterUse::NO;
}

void DirectDrawSurface::create(int w, int h, int bpp) {
	assert(bpp == 16 || bpp == 32);
	Graphics::PixelFormat pixelFormat = (bpp == 32) ?
		Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0) :
		Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);

	_surface = new Graphics::ManagedSurface(w, h, pixelFormat);
	_disposeAfterUse = DisposeAfterUse::YES;
}

void DirectDrawSurface::free() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete _surface;
	_surface = nullptr;
	_disposeAfterUse = DisposeAfterUse::NO;
}

Graphics::ManagedSurface *DirectDrawSurface::lock(const Rect *bounds, int flags) {
	assert(!_surface->empty());
	return _surface;
}

void DirectDrawSurface::unlock() {
	assert(_surface->w != 0 && _surface->h != 0);
}

void DirectDrawSurface::fill(const Rect *bounds, uint32 color) {
	Rect tempBounds;

	assert(_surface);
	if (bounds) {
		// Bounds are provided, clip them to the bounds of this surface
		tempBounds = *bounds;
		tempBounds.clip(Rect(0, 0, _surface->w, _surface->h));
	} else {
		// No bounds provided, so use the entire surface
		tempBounds = Rect(0, 0, _surface->w, _surface->h);
	}

	// Fill the area
	_surface->fillRect(tempBounds, color);
}

void DirectDrawSurface::fillRect(Rect *rect, byte r, byte g, byte b) {
	uint color = _surface->format.RGBToColor(r, g, b);
	Rect tempRect = rect ? *rect : Rect(0, 0, getWidth(), getHeight());

	_surface->fillRect(tempRect, color);
}

void DirectDrawSurface::blit(const Rect &destRect, DirectDrawSurface *srcSurface, Rect &srcRect) {
	assert(srcSurface);
	if (!destRect.isEmpty())
		_surface->transBlitFrom(*srcSurface->_surface, srcRect, destRect, (uint)-1);
}

void DirectDrawSurface::blit(const Point &destPos, DirectDrawSurface *srcSurface, Rect *bounds) {
	if (bounds)
		_surface->blitFrom(*srcSurface->_surface, *bounds, destPos);
	else
		_surface->blitFrom(*srcSurface->_surface, destPos);
}

} // End of namespace Titanic
