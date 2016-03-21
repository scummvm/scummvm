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

#include "common/rect.h"
#include "titanic/direct_draw_surface.h"

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

void DirectDrawSurface::create(int w, int h) {
	Graphics::PixelFormat pixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	_surface = new Graphics::ManagedSurface(w, h, pixelFormat);
	_disposeAfterUse = DisposeAfterUse::YES;
}

void DirectDrawSurface::free() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete _surface;
	_surface = nullptr;
	_disposeAfterUse = DisposeAfterUse::NO;
}

Graphics::ManagedSurface *DirectDrawSurface::lock(const Common::Rect *bounds, int flags) {
	assert(!_surface->empty());
	return _surface;
}

void DirectDrawSurface::unlock() {
	assert(_surface->w != 0 && _surface->h != 0);
}

void DirectDrawSurface::fill(const Common::Rect *bounds, uint32 color) {
	Common::Rect tempBounds;

	if (bounds) {
		// Bounds are provided, clip them to the bounds of this surface
		tempBounds = *bounds;
		tempBounds.clip(Common::Rect(0, 0, _surface->w, _surface->h));
	} else {
		// No bounds provided, so use the entire surface
		tempBounds = Common::Rect(0, 0, _surface->w, _surface->h);
	}

	// Fill the area
	_surface->fillRect(tempBounds, color);
}

void DirectDrawSurface::blit(const Common::Rect &destRect, DirectDrawSurface *srcSurface, Common::Rect &srcRect) {
	assert(srcSurface);
	if (!destRect.isEmpty())
		_surface->transBlitFrom(*srcSurface->_surface, srcRect, destRect, (uint)-1);
}

void DirectDrawSurface::blit(const Common::Point &destPos, DirectDrawSurface *srcSurface, Common::Rect *bounds) {
	if (bounds)
		_surface->blitFrom(*srcSurface->_surface, *bounds, destPos);
	else
		_surface->blitFrom(*srcSurface->_surface, destPos);
}

} // End of namespace Titanic
