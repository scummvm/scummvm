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

#include "xeen/xsurface.h"

namespace Xeen {

XSurface::XSurface(): Graphics::Surface() {
}

XSurface::XSurface(int w, int h) : Graphics::Surface() {
	create(w, h);
}

XSurface::~XSurface() {
	free();
}

void XSurface::create(uint16 w, uint16 h) {
	Graphics::Surface::create(w, h, Graphics::PixelFormat::createFormatCLUT8());
}

void XSurface::transBlitFrom(const XSurface &src) {
	transBlitFrom(src, Common::Point());
}

void XSurface::blitFrom(const XSurface &src) {
	blitFrom(src, Common::Point());
}

void XSurface::transBlitFrom(const XSurface &src, const Common::Point &destPos) {
	if (getPixels() == nullptr)
		create(w, h);

	for (int yp = 0; yp < src.h; ++yp) {
		const byte *srcP = (const byte *)src.getBasePtr(0, yp);
		byte *destP = (byte *)getBasePtr(destPos.x, destPos.y + yp);

		for (int xp = 0; xp < this->w; ++xp, ++srcP, ++destP) {
			if (*srcP != 0)
				*destP = *srcP;
		}
	}
}

void XSurface::blitFrom(const XSurface &src, const Common::Point &destPos) {
	if (getPixels() == nullptr)
		create(w, h);

	for (int yp = 0; yp < src.h; ++yp) {
		const byte *srcP = (const byte *)src.getBasePtr(0, yp);
		byte *destP = (byte *)getBasePtr(destPos.x, destPos.y + yp);

		for (int xp = 0; xp < this->w; ++xp, ++srcP, ++destP) {
			*destP = *srcP;
		}
	}
}

} // End of namespace Xeen
