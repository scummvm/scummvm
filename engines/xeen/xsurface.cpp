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

#include "common/algorithm.h"
#include "common/util.h"
#include "xeen/xsurface.h"
#include "xeen/resources.h"

namespace Xeen {

XSurface::XSurface() : Graphics::Surface(), _freeFlag(false) {
}

XSurface::XSurface(int w, int h) : Graphics::Surface(), _freeFlag(false) {
	create(w, h);
}

XSurface::~XSurface() {
	if (_freeFlag)
		free();
}

void XSurface::create(uint16 w, uint16 h) {
	Graphics::Surface::create(w, h, Graphics::PixelFormat::createFormatCLUT8());
	_freeFlag = true;
}

void XSurface::create(XSurface *s, const Common::Rect &bounds) {
	pixels = (byte *)s->getBasePtr(bounds.left, bounds.top);
	format = Graphics::PixelFormat::createFormatCLUT8();
	pitch = s->pitch;
	w = bounds.width();
	h = bounds.height();

	_freeFlag = false;
}


void XSurface::transBlitTo(XSurface &dest) const {
	transBlitTo(dest, Common::Point());
}

void XSurface::blitTo(XSurface &dest) const {
	blitTo(dest, Common::Point());
}

void XSurface::transBlitTo(XSurface &dest, const Common::Point &destPos) const {
	if (dest.getPixels() == nullptr)
		dest.create(w, h);

	for (int yp = 0; yp < h; ++yp) {
		const byte *srcP = (const byte *)getBasePtr(0, yp);
		byte *destP = (byte *)dest.getBasePtr(destPos.x, destPos.y + yp);

		for (int xp = 0; xp < w; ++xp, ++srcP, ++destP) {
			if (*srcP != 0)
				*destP = *srcP;
		}
	}

	dest.addDirtyRect(Common::Rect(destPos.x, destPos.y, destPos.x + w, destPos.y));
}

void XSurface::blitTo(XSurface &dest, const Common::Point &destPos) const {
	if (dest.getPixels() == nullptr)
		dest.create(w, h);

	for (int yp = 0; yp < h; ++yp) {
		const byte *srcP = (const byte *)getBasePtr(0, yp);
		byte *destP = (byte *)dest.getBasePtr(destPos.x, destPos.y + yp);

		Common::copy(srcP, srcP + w, destP);
	}

	dest.addDirtyRect(Common::Rect(destPos.x, destPos.y, destPos.x + w, destPos.y + h));
}

} // End of namespace Xeen
