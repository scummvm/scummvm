/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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
 */

#include "common/file.h"
#include "common/macresman.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/pict.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"

#include "pegasus/pegasus.h"
#include "pegasus/surface.h"

namespace Pegasus {

Surface::Surface() {
	_ownsSurface = false;
	_surface = 0;
}

Surface::~Surface() {
	deallocateSurface();
}

void Surface::deallocateSurface() {
	if (_surface) {
		if (_ownsSurface) {
			_surface->free();
			delete _surface;
		}

		_surface = 0;
		_bounds = Common::Rect();
		_ownsSurface = false;
	}
}

void Surface::shareSurface(Surface *surface) {
	deallocateSurface();

	if (surface) {
		_ownsSurface = false;
		_surface = surface->getSurface();
		surface->getSurfaceBounds(_bounds);
	}
}

void Surface::allocateSurface(const Common::Rect &bounds) {
	deallocateSurface();

	if (bounds.isEmpty())
		return;

	_bounds = bounds;
	_surface = new Graphics::Surface();
	_surface->create(bounds.width(), bounds.height(), g_system->getScreenFormat());
	_ownsSurface = true;
}

void Surface::getImageFromPICTFile(const Common::String &fileName) {
	Common::File pict;
	if (!pict.open(fileName))
		error("Could not open picture '%s'", fileName.c_str());

	getImageFromPICTStream(&pict);
}

void Surface::getImageFromPICTResource(Common::MacResManager *resFork, uint16 id) {
	Common::SeekableReadStream *res = resFork->getResource(MKTAG('P', 'I', 'C', 'T'), id);
	if (!res)
		error("Could not open PICT resource %d from '%s'", id, resFork->getBaseFileName().c_str());
	
	getImageFromPICTStream(res);
	delete res;
}

void Surface::getImageFromPICTStream(Common::SeekableReadStream *stream) {
	Graphics::PictDecoder pict(g_system->getScreenFormat());
	byte pal[256 * 3];

	Graphics::Surface *surface = pict.decodeImage(stream, pal);

	// Create the surface if not present
	if (!_surface)
		_surface = new Graphics::Surface();

	// Update
	if (surface->format.bytesPerPixel == 1) {
		// Convert to true color
		_surface->create(surface->w, surface->h, g_system->getScreenFormat());
		
		for (int y = 0; y < surface->h; y++) {
			for (int x = 0; x < surface->w; x++) {
				byte index = *((byte *)surface->getBasePtr(x, y));
				uint32 color = _surface->format.RGBToColor(pal[index * 3], pal[index * 3 + 1], pal[index * 3 + 2]);
				if (_surface->format.bytesPerPixel == 2)
					*((uint16 *)_surface->getBasePtr(x, y)) = color;
				else
					*((uint32 *)_surface->getBasePtr(x, y)) = color;
			}
		}
	} else {
		// Just a copy
		_surface->copyFrom(*surface);
	}

	_ownsSurface = true;
	_bounds = Common::Rect(0, 0, _surface->w, _surface->h);
}

void Surface::copyToCurrentPort() const {
	copyToCurrentPort(_bounds);
}

void Surface::copyToCurrentPortTransparent() const {
	copyToCurrentPortTransparent(_bounds);
}

void Surface::copyToCurrentPort(const Common::Rect &rect) const {
	copyToCurrentPort(rect, rect);
}

void Surface::copyToCurrentPortTransparent(const Common::Rect &rect) const {
	copyToCurrentPortTransparent(rect, rect);
}

void Surface::copyToCurrentPort(const Common::Rect &srcRect, const Common::Rect &dstRect) const {
	Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getWorkArea();
	byte *src = (byte *)_surface->getBasePtr(srcRect.left, srcRect.top);
	byte *dst = (byte *)screen->getBasePtr(dstRect.left, dstRect.top);

	int lineSize = srcRect.width() * _surface->format.bytesPerPixel;

	for (int y = 0; y < srcRect.height(); y++) {
		memcpy(dst, src, lineSize);
		src += _surface->pitch;
		dst += screen->pitch;
	}
}

void Surface::copyToCurrentPortTransparent(const Common::Rect &srcRect, const Common::Rect &dstRect) const {
	// HACK: Seems we're truncating some color data somewhere...
	uint32 transColor1 = g_system->getScreenFormat().RGBToColor(0xff, 0xff, 0xff);
	uint32 transColor2 = g_system->getScreenFormat().RGBToColor(0xf8, 0xf8, 0xf8);

	Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getWorkArea();
	byte *src = (byte *)_surface->getBasePtr(srcRect.left, srcRect.top);
	byte *dst = (byte *)screen->getBasePtr(dstRect.left, dstRect.top);

	int lineSize = srcRect.width() * _surface->format.bytesPerPixel;

	for (int y = 0; y < srcRect.height(); y++) {
		for (int x = 0; x < srcRect.width(); x++) {
			if (g_system->getScreenFormat().bytesPerPixel == 2) {
				uint16 color = READ_UINT16(src);
				if (color != transColor1 && color != transColor2)
					memcpy(dst, src, 2);
			} else if (g_system->getScreenFormat().bytesPerPixel == 4) {
				uint32 color = READ_UINT32(src);
				if (color != transColor1 && color != transColor2)
					memcpy(dst, src, 4);
			}

			src += g_system->getScreenFormat().bytesPerPixel;
			dst += g_system->getScreenFormat().bytesPerPixel;
		}

		src += _surface->pitch - lineSize;
		dst += screen->pitch - lineSize;
	}
}

PixelImage::PixelImage() {
	_transparent = false;
}

void PixelImage::drawImage(const Common::Rect &sourceBounds, const Common::Rect &destBounds) {
	if (!isSurfaceValid())
		return;

	// Draw from sourceBounds to destBounds based on _transparent
	if (_transparent)
		copyToCurrentPortTransparent(sourceBounds, destBounds);
	else
		copyToCurrentPort(sourceBounds, destBounds);
}

void Frame::initFromPICTFile(const Common::String &fileName, bool transparent) {
	getImageFromPICTFile(fileName);
	_transparent = transparent;
}

void Frame::initFromPICTResource(Common::MacResManager *resFork, uint16 id, bool transparent) {
	getImageFromPICTResource(resFork, id);
	_transparent = transparent;
}

void Picture::draw(const Common::Rect &r) {
	Common::Rect surfaceBounds;
	getSurfaceBounds(surfaceBounds);
	Common::Rect r1 = r;

	Common::Rect bounds;
	getBounds(bounds);
	surfaceBounds.moveTo(bounds.left, bounds.top);
	r1 = r1.findIntersectingRect(surfaceBounds);
	getSurfaceBounds(surfaceBounds);

	Common::Rect r2 = r1;
	r2.translate(surfaceBounds.left - bounds.left, surfaceBounds.top - bounds.top);
	drawImage(r2, r1);
}

void Picture::initFromPICTFile(const Common::String &fileName, bool transparent) {
	Frame::initFromPICTFile(fileName, transparent);

	Common::Rect surfaceBounds;
	getSurfaceBounds(surfaceBounds);
	sizeElement(surfaceBounds.width(), surfaceBounds.height());
}

void Picture::initFromPICTResource(Common::MacResManager *resFork, uint16 id, bool transparent) {
	Frame::initFromPICTResource(resFork, id, transparent);

	Common::Rect surfaceBounds;
	getSurfaceBounds(surfaceBounds);
	sizeElement(surfaceBounds.width(), surfaceBounds.height());
}

} // End of namespace Pegasus
