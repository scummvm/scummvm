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

#ifndef AGS_LIB_ALLEGRO_SURFACE_H
#define AGS_LIB_ALLEGRO_SURFACE_H

#include "graphics/managed_surface.h"
#include "ags/lib/allegro/base.h"
#include "common/array.h"

namespace AGS3 {

class BITMAP {
private:
	Graphics::ManagedSurface *_owner;
public:
	uint16 &w, &h, &pitch;
	Graphics::PixelFormat &format;
	bool clip;
	int ct, cb, cl, cr;
	Common::Array<byte *> line;
public:
	BITMAP(Graphics::ManagedSurface *owner);
	virtual ~BITMAP() {
	}

	Graphics::ManagedSurface &operator*() const {
		return *_owner;
	}
	Graphics::ManagedSurface &getSurface() {
		return *_owner;
	}
	const Graphics::ManagedSurface &getSurface() const {
		return *_owner;
	}

	unsigned char *getPixels() const {
		return (unsigned char *)_owner->getPixels();
	}

	unsigned char *getBasePtr(uint16 x, uint16 y) const {
		return (unsigned char *)_owner->getBasePtr(x, y);
	}

	uint getTransparentColor() const {
		return format.RGBToColor(255, 0, 255);
	}

	int getpixel(int x, int y) const;

	void clear() {
		_owner->clear();
	}

	/**
	 * Draws a solid filled in circle
	 */
	void circlefill(int x, int y, int radius, int color);

	/**
	 * Fills an enclosed area starting at a given point
	 */
	void floodfill(int x, int y, int color);

	/**
	 * Draw a horizontal line
	 */
	void hLine(int x, int y, int x2, uint32 color) {
		_owner->hLine(x, y, x2, color);
	}

	/**
	 * Draw a vertical line.
	 */
	void vLine(int x, int y, int y2, uint32 color) {
		_owner->vLine(x, y, y2, color);
	}

	/**
	 * Draws the passed surface onto this one
	 */
	void draw(const BITMAP *srcBitmap, const Common::Rect &srcRect,
		const Common::Rect &destRect, bool horizFlip, bool vertFlip,
		bool skipTrans, int srcAlpha, int tintRed = -1, int tintGreen = -1,
		int tintBlue = -1);
};

/**
 * Derived surface class
 */
class Surface : public Graphics::ManagedSurface, public BITMAP {
public:
	Surface() : Graphics::ManagedSurface(), BITMAP(this) {
	}
	Surface(const Graphics::ManagedSurface &surf) : Graphics::ManagedSurface(surf), BITMAP(this) {
	}
	Surface(int width, int height) : Graphics::ManagedSurface(width, height), BITMAP(this) {
	}
	Surface(int width, int height, const Graphics::PixelFormat &pixelFormat) :
			Graphics::ManagedSurface(width, height, pixelFormat), BITMAP(this) {
		// Allegro uses 255, 0, 255 RGB as the transparent color
		if (pixelFormat.bytesPerPixel == 4)
			setTransparentColor(pixelFormat.RGBToColor(255, 0, 255));
	}
	Surface(Graphics::ManagedSurface &surf, const Common::Rect &bounds) :
			Graphics::ManagedSurface(surf, bounds), BITMAP(this) {
		// Allegro uses 255, 0, 255 RGB as the transparent color
		if (surf.format.bytesPerPixel == 4)
			setTransparentColor(surf.format.RGBToColor(255, 0, 255));
	}
	~Surface() override {
	}
};

BITMAP *create_bitmap(int width, int height);
BITMAP *create_bitmap_ex(int color_depth, int width, int height);
BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height);
BITMAP *create_video_bitmap(int width, int height);
BITMAP *create_system_bitmap(int width, int height);
void destroy_bitmap(BITMAP *bitmap);

} // namespace AGS3

#endif
