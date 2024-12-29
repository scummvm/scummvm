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

#ifndef SHERLOCK_SURFACE_H
#define SHERLOCK_SURFACE_H

#include "common/rect.h"
#include "common/platform.h"
#include "graphics/screen.h"
#include "sherlock/fonts.h"
#include "sherlock/image_file.h"

namespace Sherlock {

#define SCALE_THRESHOLD 0x100
#define TRANSPARENCY 255

/**
 * Implements a base surface that combines both a managed surface and the font
 * drawing code. It also introduces a series of drawing method stubs that the 3DO
 * Serrated Scalpel screen overrides to implement sprite doubling
 */
class BaseSurface: public Graphics::Screen, public Fonts {
public:
	/**
	 * Constructor
	 */
	BaseSurface();

	/**
	 * Constructor
	 */
	BaseSurface(int width, int height);
	BaseSurface(int width_, int height_, const Graphics::PixelFormat &pf);

	/**
	 * Draws a surface on this surface
	 */
	virtual void SHblitFrom(const Graphics::Surface &src) {
		Graphics::ManagedSurface::blitFrom(src);
	}

	/**
	 * Draws a surface at a given position within this surface
	 */
	virtual void SHblitFrom(const Graphics::Surface &src, const Common::Point &destPos) {
		Graphics::ManagedSurface::blitFrom(src, destPos);
	}

	/**
	 * Draws a sub-section of a surface at a given position within this surface
	 */
	virtual void SHblitFrom(const Graphics::Surface &src, const Common::Point &destPos, const Common::Rect &srcBounds) {
		Graphics::ManagedSurface::blitFrom(src, srcBounds, destPos);
	}

	virtual void SHbitmapBlitFrom(const byte *src, int width, int height, int pitchSrc, const Common::Point &pt,
				      int overrideColor = 0);

	/**
	 * Draws an image frame at a given position within this surface with transparency
	 */
	virtual void SHtransBlitFrom(const ImageFrame &src, const Common::Point &pt,
		bool flipped = false, int scaleVal = SCALE_THRESHOLD);

	/**
	 * Draws an image frame at a given position within this surface with transparency
	 */
	virtual void SHtransBlitFrom(const Graphics::Surface &src, const Common::Point &pt,
		bool flipped = false, int scaleVal = SCALE_THRESHOLD);

	/**
	 * Draws an image frame at a given position within this surface with transparency
	 */
	virtual void SHoverrideBlitFrom(const ImageFrame &src, const Common::Point &pt,
		int overrideColor);

	/**
	 * Draws an image frame at a given position within this surface with transparency
	 */
	virtual void SHoverrideBlitFrom(const Graphics::Surface &src, const Common::Point &pt,
		int overrideColor);

	/**
	 * Fill a given area of the surface with a given color
	 */
	virtual void SHfillRect(const Common::Rect &r, uint color) {
		Graphics::ManagedSurface::fillRect(r, color);
	}

	/**
	 * Return the width of the surface
	 */
	virtual uint16 width() const { return this->w; }

	/**
	 * Return the height of the surface
	 */
	virtual uint16 height() const { return this->h; }

	/**
	 * Draws the given string into the back buffer using the images stored in _font
	 */
	void writeString(const Common::String &str, const Common::Point &pt, uint overrideColor);

	/**
	 * Draws a fancy version of the given string at the given position
	 */
	void writeFancyString(const Common::String &str, const Common::Point &pt, uint overrideColor1, uint overrideColor2);
};

class Surface : public BaseSurface {
protected:
	/**
	* Override the addDirtyRect from Graphics::Screen, since for standard
	* surfaces we don't need dirty rects to be tracked
	*/
	void addDirtyRect(const Common::Rect &r) override {}
public:
	Surface() : BaseSurface() {}
	Surface(int width_, int height_) : BaseSurface(width_, height_) {}
	Surface(int width_, int height_, const Graphics::PixelFormat &pf) : BaseSurface(width_, height_, pf) {}
};

} // End of namespace Sherlock

#endif
