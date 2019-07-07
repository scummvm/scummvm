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

#ifndef GRAPHICS_MANAGED_SURFACE_H
#define GRAPHICS_MANAGED_SURFACE_H

#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "common/rect.h"
#include "common/types.h"

namespace Graphics {

class Font;

/**
 * A derived graphics surface, which handles automatically managing the allocated
 * surface data block, as well as introducing several new blitting methods
 */
class ManagedSurface {
	friend class Font;
private:
	/**
	 * The Graphics::Surface that the managed surface encapsulates
	 */
	Surface _innerSurface;

	/**
	 * If set, the inner surface will be freed when the surface is recreated,
	 * as well as when the surface is destroyed
	 */
	DisposeAfterUse::Flag _disposeAfterUse;

	/**
	 * Stores the owning surface if this If this managed surface represents
	 * a sub-section of another
	 */
	ManagedSurface *_owner;

	/**
	 * For sub-section areas of an owning parent managed surface, this represents
	 * the offset from the parent's top-left corner this sub-surface starts at
	 */
	Common::Point _offsetFromOwner;
protected:
	/**
	 * Clips the given source bounds so the passed destBounds will be entirely on-screen
	 */
	bool clip(Common::Rect &srcBounds, Common::Rect &destBounds);

	/**
	 * Base method that descendent classes can override for recording affected
	 * dirty areas of the surface
	 */
	virtual void addDirtyRect(const Common::Rect &r);
public:
	uint16 &w;
	uint16 &h;
	uint16 &pitch;
	PixelFormat &format;
public:
	/**
	 * Create the managed surface
	 */
	ManagedSurface();

	/**
	 * Create a managed surface from another one.
	 * If the source surface is maintaining it's own surface data, then
	 * this surface will create it's own surface of the same size and copy
	 * the contents from the source surface
	 */
	ManagedSurface(ManagedSurface &surf);

	/**
	 * Create the managed surface
	 */
	ManagedSurface(int width, int height);

	/**
	 * Create the managed surface
	 */
	ManagedSurface(int width, int height, const Graphics::PixelFormat &pixelFormat);

	/**
	 * Create the managed surface
	 */
	ManagedSurface(ManagedSurface &surf, const Common::Rect &bounds);

	/**
	 * Destroy the managed surface
	 */
	virtual ~ManagedSurface();

	/**
	 * Implements automatic conversion to a Graphics::Surface by
	 * simply returning the inner surface. This must be const,
	 * because we don't want changes being done directly to it,
	 * since it would bypass dirty rect handling
	 */
	operator const Surface &() const { return _innerSurface; }
	const Surface &rawSurface() const { return _innerSurface; }

	/**
	 * Reassign one managed surface to another one
	 * Note that if the source has a managed surface, it will be duplicated
	 */
	ManagedSurface &operator=(ManagedSurface &surf);

	/**
	 * Returns true if the surface has not yet been allocated
	 */
	bool empty() const { return w == 0 || h == 0 || _innerSurface.getPixels() == nullptr; }

	/**
	 * Returns true if the surface is managing its own pixels
	 */
	DisposeAfterUse::Flag disposeAfterUse() const { return _disposeAfterUse; }

	/**
	 * Return a pointer to the pixel at the specified point.
	 *
	 * @param x The x coordinate of the pixel.
	 * @param y The y coordinate of the pixel.
	 * @return Pointer to the pixel.
	 */
	inline const void *getBasePtr(int x, int y) const {
		return _innerSurface.getBasePtr(x, y);
	}

	/**
	 * Return a pointer to the pixel at the specified point.
	 *
	 * @param x The x coordinate of the pixel.
	 * @param y The y coordinate of the pixel.
	 * @return Pointer to the pixel.
	 */
	inline void *getBasePtr(int x, int y) {
		return _innerSurface.getBasePtr(x, y);
	}

	/**
	 * Get a reference to the pixel data
	 */
	inline void *getPixels() { return _innerSurface.getPixels(); }
	inline const void *getPixels() const { return _innerSurface.getPixels(); }

	/**
	 * Sets the pixel data.
	 */
	virtual void setPixels(void *newPixels);

	/**
	 * Allocate memory for the pixel data of the surface.
	 */
	virtual void create(uint16 width, uint16 height);

	/**
	 * Allocate memory for the pixel data of the surface.
	 */
	virtual void create(uint16 width, uint16 height, const PixelFormat &pixelFormat);

	/**
	 * Sets up the surface as a sub-section of another passed parent surface. This surface
	 * will not own the pixels, and any dirty rect notifications will automatically be
	 * passed to the original parent surface.
	 * @remarks		Note that this differs from Graphics::Surface::getSubArea, in that that
	 * method only adds a single initial dirty rect for the whole area, and then none further
	 */
	virtual void create(ManagedSurface &surf, const Common::Rect &bounds);

	/**
	 * Release the memory used by the pixels memory of this surface. This is the
	 * counterpart to create().
	 */
	virtual void free();

	/**
	 * Clears any pending dirty rects that have been generated for the surface
	 */
	virtual void clearDirtyRects() {}

	/**
	 * When the managed surface is a sub-section of a parent surface, returns the
	 * the offset in the parent surface that the surface starts at
	 */
	const Common::Point getOffsetFromOwner() const { return _offsetFromOwner; }

	/**
	 * Return a rect giving the bounds of the surface
	 */
	const Common::Rect getBounds() const {
		return Common::Rect(0, 0, this->w, this->h);
	}

	/**
	 * Copies another surface into this one
	 */
	void blitFrom(const Surface &src);

	/**
	 * Copies another surface into this one at a given destination position
	 */
	void blitFrom(const Surface &src, const Common::Point &destPos);

	/**
	 * Copies another surface into this one at a given destination position
	 */
	void blitFrom(const Surface &src, const Common::Rect &srcRect,
		const Common::Point &destPos);

	/**
	 * Copies another surface into this one ignoring pixels of a designated transparent color
	 * @param src			Source surface
	 * @param transColor	Transparency color to ignore copying
	 * @param flipped		Specifies whether to horizontally flip the image
	 * @param overrideColor	Optional color to use instead of non-transparent pixels from
	 *						the source surface
	 * @param srcAlpha		Optional additional transparency applied to src
	 */
	void transBlitFrom(const Surface &src, uint transColor = 0, bool flipped = false, uint overrideColor = 0, uint srcAlpha = 0xff);

	/**
	 * Copies another surface into this one ignoring pixels of a designated transparent color
	 * @param src			Source surface
	 * @param destPos		Destination position to draw the surface
	 * @param transColor	Transparency color to ignore copying
	 * @param flipped		Specifies whether to horizontally flip the image
	 * @param overrideColor	Optional color to use instead of non-transparent pixels from
	 *						the source surface
	 * @param srcAlpha		Optional additional transparency applied to src
	 */
	void transBlitFrom(const Surface &src, const Common::Point &destPos,
		uint transColor = 0, bool flipped = false, uint overrideColor = 0, uint srcAlpha = 0xff);

	/**
	 * Copies another surface into this one ignoring pixels of a designated transparent color
	 * @param src			Source surface
	 * @param srcRect		Sub-section of source surface to draw
	 * @param destPos		Destination position to draw the surface
	 * @param transColor	Transparency color to ignore copying
	 * @param flipped		Specifies whether to horizontally flip the image
	 * @param overrideColor	Optional color to use instead of non-transparent pixels from
	 *						the source surface
	 * @param srcAlpha		Optional additional transparency applied to src
	 */
	void transBlitFrom(const Surface &src, const Common::Rect &srcRect, const Common::Point &destPos,
		uint transColor = 0, bool flipped = false, uint overrideColor = 0, uint srcAlpha = 0xff);

	/**
	 * Copies another surface into this one ignoring pixels of a designated transparent color
	 * @param src			Source surface
	 * @param srcRect		Sub-section of source surface to draw
	 * @param destRect		Destination area to draw the surface in. This can be sized differently
	 *						then srcRect, allowing for arbitrary scaling of the image
	 * @param transColor	Transparency color to ignore copying
	 * @param flipped		Specifies whether to horizontally flip the image
	 * @param overrideColor	Optional color to use instead of non-transparent pixels from
	 *						the source surface
	 * @param srcAlpha		Optional additional transparency applied to src
	 */
	void transBlitFrom(const Surface &src, const Common::Rect &srcRect, const Common::Rect &destRect,
		uint transColor = 0, bool flipped = false, uint overrideColor = 0, uint srcAlpha = 0xff);

	/**
	 * Clear the entire surface
	 */
	void clear(uint color = 0);

	/**
	 * Mark the entire surface as dirty
	 */
	void markAllDirty();

	/**
	 * Copies a bitmap to the Surface internal buffer. The pixel format
	 * of buffer must match the pixel format of the Surface.
	 */
	void copyRectToSurface(const void *buffer, int srcPitch, int destX, int destY, int width, int height) {
		_innerSurface.copyRectToSurface(buffer, srcPitch, destX, destY, width, height);
	}

	/**
	 * Copies a bitmap to the Surface internal buffer. The pixel format
	 * of buffer must match the pixel format of the Surface.
	 */
	void copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY, const Common::Rect subRect) {
		_innerSurface.copyRectToSurface(srcSurface, destX, destY, subRect);
	}

	/**
	 * Copy the data from another Surface, reinitializing the
	 * surface to match the dimensions of the passed surface
	 */
	void copyFrom(const ManagedSurface &surf);

	/**
	 * Draw a line.
	 */
	void drawLine(int x0, int y0, int x1, int y1, uint32 color) {
		_innerSurface.drawLine(x0, y0, x1, y1, color);
		addDirtyRect(Common::Rect(MIN(x0, x1), MIN(y0, y1), MAX(x0, x1), MAX(y0, y1)));
	}

	/**
	 * Draw a thick line.
	 */
	void drawThickLine(int x0, int y0, int x1, int y1, int penX, int penY, uint32 color) {
		_innerSurface.drawThickLine(x0, y0, x1, y1, penX, penY, color);
		addDirtyRect(Common::Rect(MIN(x0, x1 + penX), MIN(y0, y1 + penY), MAX(x0, x1 + penX), MAX(y0, y1 + penY)));
	}

	/**
	 * Draw a horizontal line.
	 */
	void hLine(int x, int y, int x2, uint32 color) {
		_innerSurface.hLine(x, y, x2, color);
		addDirtyRect(Common::Rect(x, y, x2 + 1, y + 1));
	}

	/**
	 * Draw a vertical line.
	 */
	void vLine(int x, int y, int y2, uint32 color) {
		_innerSurface.vLine(x, y, y2, color);
		addDirtyRect(Common::Rect(x, y, x + 1, y2 + 1));
	}

	/**
	 * Fill a rect with a given color.
	 */
	void fillRect(Common::Rect r, uint32 color) {
		_innerSurface.fillRect(r, color);
		addDirtyRect(r);
	}

	/**
	 * Draw a frame around a specified rect.
	 */
	void frameRect(const Common::Rect &r, uint32 color) {
		_innerSurface.frameRect(r, color);
		addDirtyRect(r);
	}

	/**
	 * Returns a sub-area of the screen, but only adds a single initial dirty rect
	 * for the retrieved area.
	 */
	Surface getSubArea(const Common::Rect &area) {
		addDirtyRect(area);
		return _innerSurface.getSubArea(area);
	}

	/**
	 * Convert the data to another pixel format.
	 *
	 * This works in-place. This means it will not create an additional buffer
	 * for the conversion process. The value of 'pixels' might change though
	 * (that means it might realloc the pixel data).
	 *
	 * @param dstFormat The desired format
	 * @param palette   The palette (in RGB888), if the source format has a Bpp of 1
	 */
	void convertToInPlace(const PixelFormat &dstFormat, const byte *palette = 0) {
		_innerSurface.convertToInPlace(dstFormat, palette);
	}
};

} // End of namespace Graphics

#endif
