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

#ifndef SHERLOCK_GRAPHICS_H
#define SHERLOCK_GRAPHICS_H

#include "common/rect.h"
#include "common/platform.h"
#include "graphics/surface.h"
#include "sherlock/fonts.h"

namespace Sherlock {

#define SCALE_THRESHOLD 0x100
#define TRANSPARENCY 255

struct ImageFrame;

class Surface: public Fonts {
private:
	bool _freePixels;

	/**
	 * Clips the given source bounds so the passed destBounds will be entirely on-screen
	 */
	bool clip(Common::Rect &srcBounds, Common::Rect &destBounds);

	/**
	 * Copy a surface into this one
	 */
	void blitFrom(const Graphics::Surface &src);

	/**
	 * Draws a sub-section of a surface at a given position within this surface
	 */
	void blitFrom(const Graphics::Surface &src, const Common::Point &pt, const Common::Rect &srcBounds);

	/**
	 * Draws a surface at a given position within this surface with transparency
	 */
	void transBlitFromUnscaled(const Graphics::Surface &src, const Common::Point &pt, bool flipped, 
		int overrideColor);

protected:
	Graphics::Surface _surface;

	virtual void addDirtyRect(const Common::Rect &r) {}
public:
	Surface(uint16 width, uint16 height);
	Surface();
	virtual ~Surface();

	/**
	 * Sets up an internal surface with the specified dimensions that will be automatically freed
	 * when the surface object is destroyed
	 */
	void create(uint16 width, uint16 height);

	Graphics::PixelFormat getPixelFormat();

	/**
	 * Copy a surface into this one
	 */
	void blitFrom(const Surface &src);

	/**
	 * Copy an image frame into this surface
	 */
	void blitFrom(const ImageFrame &src);

	/**
	 * Draws a surface at a given position within this surface
	 */
	void blitFrom(const Surface &src, const Common::Point &pt);

	/**
	 * Copy an image frame onto this surface at a given position
	 */
	void blitFrom(const ImageFrame &src, const Common::Point &pt);

	/**
	 * Draws a sub-section of a surface at a given position within this surface
	 */
	void blitFrom(const Surface &src, const Common::Point &pt, const Common::Rect &srcBounds);

	/**
	 * Copy a sub-area of a source image frame into this surface at a given position
	 */
	void blitFrom(const ImageFrame &src, const Common::Point &pt, const Common::Rect &srcBounds);

	/**
	 * Draws a surface at a given position within this surface
	 */
	void blitFrom(const Graphics::Surface &src, const Common::Point &pt);

	/**
	 * Draws an image frame at a given position within this surface with transparency
	 */
	void transBlitFrom(const ImageFrame &src, const Common::Point &pt,
		bool flipped = false, int overrideColor = 0, int scaleVal = 256);
	
	/**
	* Draws a surface at a given position within this surface with transparency
	*/
	void transBlitFrom(const Surface &src, const Common::Point &pt,
		bool flipped = false, int overrideColor = 0, int scaleVal = 256);

	/**
	 * Draws a surface at a given position within this surface with transparency
	 */
	void transBlitFrom(const Graphics::Surface &src, const Common::Point &pt,
		bool flipped = false, int overrideColor = 0, int scaleVal = 256);

	/**
	 * Fill a given area of the surface with a given color
	 */
	void fillRect(int x1, int y1, int x2, int y2, uint color);
	
	/**
	 * Fill a given area of the surface with a given color
	 */
	void fillRect(const Common::Rect &r, uint color);

	void fill(uint color);

	/**
	 * Clear the surface
	 */
	void clear();

	/**
	 * Free the underlying surface
	 */
	void free();

	/**
	 * Returns true if the surface is empty
	 */
	bool empty() const { return _surface.getPixels() == nullptr; }

	/**
	 * Set the pixels for the surface to an existing data block
	 */
	void setPixels(byte *pixels, int width, int height, Graphics::PixelFormat format);

	/**
	 * Draws the given string into the back buffer using the images stored in _font
	 */
	virtual void writeString(const Common::String &str, const Common::Point &pt, uint overrideColor);
	void writeFancyString(const Common::String &str, const Common::Point &pt, uint overrideColor1, uint overrideColor2);

	inline uint16 w() const { return _surface.w; }
	inline uint16 h() const { return _surface.h; }
	inline const byte *getPixels() const { return (const byte *)_surface.getPixels(); }
	inline byte *getPixels() { return (byte *)_surface.getPixels(); }
	inline byte *getBasePtr(int x, int y) { return (byte *)_surface.getBasePtr(x, y); }
	inline const byte *getBasePtr(int x, int y) const { return (const byte *)_surface.getBasePtr(x, y); }
	inline Graphics::Surface &getRawSurface() { return _surface; }
	inline void hLine(int x, int y, int x2, uint color) { _surface.hLine(x, y, x2, color); }
	inline void vLine(int x, int y, int y2, uint color) { _surface.vLine(x, y, y2, color); }
};

} // End of namespace Sherlock

#endif
