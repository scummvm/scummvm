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

#include "sherlock/surface.h"
#include "sherlock/fonts.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

BaseSurface::BaseSurface() : Graphics::Screen(0, 0), Fonts() {
	free();		// Free the 0x0 surface allocated by Graphics::Screen
}

BaseSurface::BaseSurface(int width_, int height_) : Graphics::Screen(width_, height_),
		Fonts() {
	create(width_, height_);
}

BaseSurface::BaseSurface(int width_, int height_, const Graphics::PixelFormat &pf) :
		Graphics::Screen(width_, height_, pf), Fonts() {
}

void BaseSurface::writeString(const Common::String &str, const Common::Point &pt, uint overrideColor) {
	Fonts::writeString(this, str, pt, overrideColor);
}

void BaseSurface::writeFancyString(const Common::String &str, const Common::Point &pt, uint overrideColor1, uint overrideColor2) {
	writeString(str, Common::Point(pt.x, pt.y), overrideColor1);
	writeString(str, Common::Point(pt.x + 1, pt.y), overrideColor1);
	writeString(str, Common::Point(pt.x + 2, pt.y), overrideColor1);
	writeString(str, Common::Point(pt.x, pt.y + 1), overrideColor1);
	writeString(str, Common::Point(pt.x + 2, pt.y + 1), overrideColor1);
	writeString(str, Common::Point(pt.x, pt.y + 2), overrideColor1);
	writeString(str, Common::Point(pt.x + 1, pt.y + 2), overrideColor1);
	writeString(str, Common::Point(pt.x + 2, pt.y + 2), overrideColor1);
	writeString(str, Common::Point(pt.x + 1, pt.y + 1), overrideColor2);
}

void BaseSurface::SHtransBlitFrom(const ImageFrame &src, const Common::Point &pt,
		bool flipped, int scaleVal) {
	Common::Point drawPt(pt.x + src.sDrawXOffset(scaleVal), pt.y + src.sDrawYOffset(scaleVal));
	SHtransBlitFrom(src._frame, drawPt, flipped, scaleVal);
}

void BaseSurface::SHtransBlitFrom(const Graphics::Surface &src, const Common::Point &pt,
		bool flipped, int scaleVal) {
	Common::Rect srcRect(0, 0, src.w, src.h);
	Common::Rect destRect(pt.x, pt.y, pt.x + src.w * SCALE_THRESHOLD / scaleVal,
		pt.y + src.h * SCALE_THRESHOLD / scaleVal);

	Graphics::Screen::transBlitFrom(src, srcRect, destRect, IS_3DO ? 0 : TRANSPARENCY,
		flipped);
}

void BaseSurface::SHoverrideBlitFrom(const ImageFrame &src, const Common::Point &pt,
		int overrideColor) {
	Common::Point drawPt(pt.x + src.sDrawXOffset(SCALE_THRESHOLD), pt.y + src.sDrawYOffset(SCALE_THRESHOLD));
	SHoverrideBlitFrom(src._frame, drawPt, overrideColor);
}

void BaseSurface::SHoverrideBlitFrom(const Graphics::Surface &src, const Common::Point &pt,
		int overrideColor) {
	Common::Rect srcRect(0, 0, src.w, src.h);
	Common::Rect destRect(pt.x, pt.y, pt.x + src.w, pt.y + src.h);
	clip(srcRect, destRect);

	const uint32 transColor = IS_3DO ? 0 : TRANSPARENCY;

	for (int y = 0; y < destRect.height(); y++) {
		for (int x = 0; x < destRect.width(); x++) {
			const uint8 srcVal = src.getPixel(srcRect.left + x, srcRect.top + y);
			if (srcVal == transColor)
				continue;
			setPixel(destRect.left + x, destRect.top + y, overrideColor);
		}
	}
}

void BaseSurface::SHbitmapBlitFrom(const byte *src, int widthSrc, int heightSrc, int pitchSrc, const Common::Point &pt,
				   int overrideColor) {
	const byte *ptr = src;
	int yin = 0, yout = pt.y;
	int xin = 0, xout = pt.x;
	byte bit = 0x80;
	int ymax = MIN(heightSrc, h - pt.y);
	int xmax = MIN(widthSrc, w - pt.x);
	int pitchskip = pitchSrc - (xmax / 8);
	for (yin = 0; yin < ymax; yin++, yout++) {
		bit = 0x80;
		for (xin = 0, xout = pt.x; xin < xmax; xin++, xout++) {
			if (*ptr & bit)
				setPixel(xout, yout, overrideColor);
			bit >>= 1;
			if (!bit) {
				bit = 0x80;
				ptr++;
			}
		}
		ptr += pitchskip;
	}
}

} // End of namespace Sherlock
