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

#ifndef GRAPHICS_MANAGED_SURFACE_H
#define GRAPHICS_MANAGED_SURFACE_H

#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "graphics/transform_struct.h"
#include "common/types.h"
#include "graphics/blit.h"

#define MS_RGB(R,G,B)       (uint32)(((R) << 24) | ((G) << 16) | ((B) << 8) | 0xff)
#define MS_ARGB(A,R,G,B)    (uint32)(((R) << 24) | ((G) << 16) | ((B) << 8) | (A))

namespace Graphics {

struct Palette;

/**
 * @defgroup graphics_managed_surface Managed surface
 * @ingroup graphics
 *
 * @brief The ManagedSurface class that adds extra functionality on top of the Surface class.
 *
 * @{
 */

/**
 * A derived graphics surface, which supports automatically managing the allocated
 * surface data block and introduces several new blitting methods.
 */
class ManagedSurface {
private:
	/**
	 * The Graphics::Surface that the managed surface encapsulates.
	 */
	Surface _innerSurface;

	/**
	 * If set, the inner surface will be freed when the surface is recreated,
	 * as well as when the surface is destroyed.
	 */
	DisposeAfterUse::Flag _disposeAfterUse;

	/**
	 * If this managed surface represents a subsection of another managed surface,
	 * store the owning surface.
	 */
	ManagedSurface *_owner;

	/**
	 * For a managed surface that has a parent, this represents the offset from
	 * the parent's top-left corner where this sub-surface starts at.
	 */
	Common::Point _offsetFromOwner;

	/**
	 * Automatic transparency color. When set, it does not require transparency to be
	 * explicitly passed, and blit methods pass on to transBlit.
	 */
	uint _transparentColor;
	bool _transparentColorSet;

	/**
	 * Local palette for 8-bit images.
	 */
	Palette *_palette;

protected:
	/**
	 * Inner method for blitting.
	 */
	void blitFromInner(const Surface &src, const Common::Rect &srcRect,
		const Common::Rect &destRect, const byte *srcPalette);

	/**
	 * Inner method for copying another surface into this one at a given destination position.
	 */
	void transBlitFromInner(const Surface &src, const Common::Rect &srcRect,
		const Common::Rect &destRect, uint32 transColor, bool flipped, uint32 overrideColor,
		uint32 srcAlpha, const byte *srcPalette, const byte *dstPalette, const Surface *mask, bool maskOnly);
public:
	/**
	 * Clip the given source bounds so the passed destBounds will be entirely on-screen.
	 */
	bool clip(Common::Rect& srcBounds, Common::Rect& destBounds) const {
		return _innerSurface.clip(srcBounds, destBounds);
	}

public:
	int16 &w;           /*!< Width of the surface rectangle. */
	int16 &h;           /*!< Height of the surface rectangle. */
	int32 &pitch;       /*!< Pitch of the surface rectangle. See @ref Surface::pitch. */
	PixelFormat &format; /*!< Pixel format of the surface. See @ref PixelFormat. */
public:
	/**
	 * Create the managed surface.
	 */
	ManagedSurface();

	/**
	 * Create a managed surface from another one.
	 *
	 * If the source surface is maintaining its own surface data, then
	 * this surface will create its own surface of the same size and copy
	 * the contents from the source surface.
	 */
	ManagedSurface(const ManagedSurface &surf);

	/**
	 * Create the managed surface.
	 */
	ManagedSurface(int width, int height);

	/**
	 * Create the managed surface.
	 */
	ManagedSurface(int width, int height, const Graphics::PixelFormat &pixelFormat);

	/**
	 * Create the managed surface.
	 */
	ManagedSurface(ManagedSurface &surf, const Common::Rect &bounds);

	/**
	 * Create a managed surface from plain Surface.
	 *
	 * If disposeAfterUse flag is set (default), the surface will reuse all structures
	 * from the surface and destroy it, otherwise it will make a copy.
	 */
	ManagedSurface(Surface *surf, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

	/**
	 * Create a managed surface from plain Surface.
	 */
	ManagedSurface(const Surface *surf);

	/**
	 * Destroy the managed surface.
	 */
	virtual ~ManagedSurface();

	/**
	 * Automatically convert to a Graphics::Surface by
	 * simply returning the inner surface.
	 *
	 * This must be const, because changes are not supposed to be done
	 * directly to it, since it would bypass dirty rect handling.
	 */
	operator const Surface &() const { return _innerSurface; }

	/**
	 * Return the underlying Graphics::Surface
	 *
	 * If a caller uses the non-const surfacePtr version and changes
	 * the surface, they'll be responsible for calling addDirtyRect
	 * for any affected area
	 */
	const Surface &rawSurface() const { return _innerSurface; }
	Surface *surfacePtr() { return &_innerSurface; }

	/**
	 * Reassign one managed surface to another one.
	 *
	 * @note If the source has a managed surface, it will be duplicated.
	 */
	ManagedSurface &operator=(const ManagedSurface &surf);

	/**
	 * Return true if the surface has not yet been allocated.
	 */
	bool empty() const { return w == 0 || h == 0 || _innerSurface.getPixels() == nullptr; }

	/**
	 * Return true if the surface manages its own pixels.
	 */
	DisposeAfterUse::Flag disposeAfterUse() const { return _disposeAfterUse; }

	/**
	 * Return the pixel at the specified point.
	 *
	 * @param x  The x coordinate of the pixel.
	 * @param y  The y coordinate of the pixel.
	 *
	 * @return The value of the pixel.
	 */
	inline uint32 getPixel(int x, int y) const {
		return _innerSurface.getPixel(x, y);
	}

	/**
	 * Set the pixel at the specified point.
	 *
	 * @param x     The x coordinate of the pixel.
	 * @param y     The y coordinate of the pixel.
	 * @param pixel The value of the pixel.
	 */
	inline void setPixel(int x, int y, uint32 pixel) {
		return _innerSurface.setPixel(x, y, pixel);
	}

	/**
	 * Return a pointer to the pixel at the specified point.
	 *
	 * @param x  The x coordinate of the pixel.
	 * @param y  The y coordinate of the pixel.
	 *
	 * @return Pointer to the pixel.
	 */
	inline const void *getBasePtr(int x, int y) const {
		return _innerSurface.getBasePtr(x, y);
	}

	/**
	 * Return a pointer to the pixel at the specified point.
	 *
	 * @param x  The x coordinate of the pixel.
	 * @param y  The y coordinate of the pixel.
	 *
	 * @return Pointer to the pixel.
	 */
	inline void *getBasePtr(int x, int y) {
		return _innerSurface.getBasePtr(x, y);
	}

	/**
	 * Get a reference to the pixel data.
	 */
	inline void *getPixels() { return _innerSurface.getPixels(); }
	/** @overload */
	inline const void *getPixels() const { return _innerSurface.getPixels(); }

	/**
	 * Set the pixel data.
	 */
	virtual void setPixels(void *newPixels);

	/**
	 * Allocate memory for the pixel data of the surface.
	 */
	virtual void create(int16 width, int16 height);

	/**
	 * Allocate memory for the pixel data of the surface.
	 */
	virtual void create(int16 width, int16 height, const PixelFormat &pixelFormat);

	/**
	 * Set up the surface as a subsection of another passed parent surface.
	 *
	 * This surface will not own the pixels, and any dirty rect notifications will be
	 * automatically passed to the original parent surface.
	 *
	 * @note This differs from Graphics::Surface::getSubArea, in that this
	 * method only adds a single initial dirty rect for the whole area, and then none more.
	 */
	virtual void create(ManagedSurface &surf, const Common::Rect &bounds);

	/**
	 * Release the memory used by the pixel memory of this surface.
	 *
	 * This is a counterpart of create().
	 */
	virtual void free();

	/**
	 * Clear any pending dirty rectangles that have been generated for the surface.
	 */
	virtual void clearDirtyRects() {}

	/**
	 * Base method that descendant classes can override for recording the affected
	 * dirty areas of the surface.
	 */
	virtual void addDirtyRect(const Common::Rect &r);

	/**
	 * When the managed surface is a subsection of a parent surface, return the
	 * the offset in the parent surface where the managed surface starts at.
	 */
	const Common::Point getOffsetFromOwner() const { return _offsetFromOwner; }

	/**
	 * Return a rect providing the bounds of the surface.
	 */
	const Common::Rect getBounds() const {
		return Common::Rect(0, 0, this->w, this->h);
	}

	/**
	 * Copy another surface into this one.
	 */
	void blitFrom(const Surface &src, const byte *srcPalette = nullptr);

	/**
	 * Copy another surface into this one at a given destination position.
	 */
	void blitFrom(const Surface &src, const Common::Point &destPos, const byte *srcPalette = nullptr);

	/**
	 * Copy another surface into this one at a given destination position.
	 */
	void blitFrom(const Surface &src, const Common::Rect &srcRect,
		const Common::Point &destPos, const byte *srcPalette = nullptr);

	/**
	 * Copy another surface into this one at a given destination area and perform the potential scaling.
	 */
	void blitFrom(const Surface &src, const Common::Rect &srcRect,
		const Common::Rect &destRect, const byte *srcPalette = nullptr);

	/**
	 * Copy another surface into this one at a given destination area and perform the potential scaling.
	 */
	void blitFrom(const ManagedSurface &src, const Common::Rect &srcRect,
		const Common::Rect &destRect);

	/**
	 * Copy another surface into this one.
	 */
	void blitFrom(const ManagedSurface &src);

	/**
	 * Copy another surface into this one at a given destination position.
	 */
	void blitFrom(const ManagedSurface &src, const Common::Point &destPos);

	/**
	 * Copy another surface into this one at a given destination position.
	 */
	void blitFrom(const ManagedSurface &src, const Common::Rect &srcRect,
		const Common::Point &destPos);

	/**
	 * Copy another surface into this one, ignoring pixels of a designated transparent color.
	 *
	 * @param src			Source surface.
	 * @param transColor	Transparency color to ignore copying of.
	 * @param flipped		Whether to horizontally flip the image.
	 * @param overrideColor	Optional color to use instead of non-transparent pixels from
	 *						the source surface.
	 * @param srcAlpha		Optional additional transparency applied to @p src.
	 * @param srcPalette	Optional palette if the @p src surface uses a CLUT8 pixel format.
	 */
	void transBlitFrom(const Surface &src, uint32 transColor = 0, bool flipped = false,
		uint32 overrideColor = 0, uint32 srcAlpha = 0xff, const byte *srcPalette = nullptr);

	/**
	 * Copy another surface into this one, ignoring pixels of a designated transparent color.
	 *
	 * @param src			Source surface.
	 * @param destPos		Destination position to draw the surface.
	 * @param transColor	Transparency color to ignore copying of.
	 * @param flipped		Whether to horizontally flip the image.
	 * @param overrideColor	Optional color to use instead of non-transparent pixels from
	 *						the source surface.
	 * @param srcAlpha		Optional additional transparency applied to @p src.
	 * @param srcPalette	Optional palette if the @p src surface uses a CLUT8 pixel format.
	 */
	void transBlitFrom(const Surface &src, const Common::Point &destPos,
		uint32 transColor = 0, bool flipped = false, uint32 overrideColor = 0, uint32 srcAlpha = 0xff, const byte *srcPalette = nullptr);

	/**
	 * Copy another surface into this one, ignoring pixels of a designated transparent color.
	 *
	 * @param src			Source surface.
	 * @param destPos		Destination position to draw the surface.
	 * @param mask			Mask definition.
	 * @param srcPalette	Optional palette if the @p src surface uses a CLUT8 pixel format.
	 */
	void transBlitFrom(const Surface &src, const Common::Point &destPos,
		const ManagedSurface &mask, const byte *srcPalette = nullptr);

	/**
	 * Copy another surface into this one, ignoring pixels of a designated transparent color.
	 *
	 * @param src			Source surface.
	 * @param destPos		Destination position to draw the surface.
	 * @param mask			Mask definition.
	 * @param srcPalette	Optional palette if the @p src surface uses a CLUT8 pixel format.
	 */
	void transBlitFrom(const Surface &src, const Common::Point &destPos,
		const Surface &mask, const byte *srcPalette = nullptr);

	/**
	 * Copy another surface into this one, ignoring pixels of a designated transparent color.
	 *
	 * @param src			Source surface.
	 * @param srcRect		Subsection of the source surface to draw.
	 * @param destPos		Destination position to draw the surface.
	 * @param transColor	Transparency color to ignore copying of.
	 * @param flipped		Specifies whether to horizontally flip the image.
	 * @param overrideColor	Optional color to use instead of non-transparent pixels from
	 *						the source surface.
	 * @param srcAlpha		Optional additional transparency applied to @p src.
	 * @param srcPalette	Optional palette if the @p src surface uses a CLUT8 pixel format.
	 */
	void transBlitFrom(const Surface &src, const Common::Rect &srcRect, const Common::Point &destPos,
		uint32 transColor = 0, bool flipped = false, uint32 overrideColor = 0, uint32 srcAlpha = 0xff, const byte *srcPalette = nullptr);

	/**
	 * Copy another surface into this one, ignoring pixels of a designated transparent color.
	 *
	 * @param src			Source surface.
	 * @param srcRect		Subsection of the source surface to draw.
	 * @param destRect		Destination area to draw the surface in. This can be sized differently
	 *						then @p srcRect, allowing for arbitrary scaling of the image.
	 * @param srcPalette	Palette for the CLUT8  @p src surface.
	 */
	void transBlitFrom(const Surface &src, const Common::Rect &srcRect, const Common::Rect &destRect, const byte *srcPalette);

	/**
	 * Copy another surface into this one, ignoring pixels of a designated transparent color.
	 *
	 * @param src			Source surface.
	 * @param srcRect		Subsection of the source surface to draw.
	 * @param destRect		Destination area to draw the surface in. This can be sized differently
	 *						then @p srcRect, allowing for arbitrary scaling of the image.
	 * @param transColor	Transparency color to ignore copying of.
	 * @param flipped		Whether to horizontally flip the image.
	 * @param overrideColor	Optional color to use instead of non-transparent pixels from
	 *						the source surface.
	 * @param srcAlpha		Optional additional transparency applied to @p src.
	 * @param mask			Optional parameter with mask definition.
	 * @param maskOnly		Optional parameter for using mask over @p transColor.
	 * @param srcPalette	Optional palette if the @p src surface uses a CLUT8 pixel format.
	 */
	void transBlitFrom(const Surface &src, const Common::Rect &srcRect, const Common::Rect &destRect,
		uint32 transColor = 0, bool flipped = false, uint32 overrideColor = 0, uint32 srcAlpha = 0xff,
		const Surface *mask = nullptr, bool maskOnly = false, const byte *srcPalette = nullptr);

	/**
	 * Copy another surface into this one, ignoring pixels of a designated transparent color.
	 *
	 * @param src			Source surface.
	 * @param transColor	Transparency color to ignore copying of.
	 * @param flipped		Whether to horizontally flip the image.
	 * @param overrideColor	Optional color to use instead of non-transparent pixels from
	 *						the source surface.
	 * @param srcAlpha		Optional additional transparency applied to @p src.
	 */
	void transBlitFrom(const ManagedSurface &src, uint32 transColor = 0, bool flipped = false,
		uint32 overrideColor = 0, uint32 srcAlpha = 0xff);

	/**
	 * Copy another surface into this one, ignoring pixels of a designated transparent color.
	 *
	 * @param src			Source surface.
	 * @param destPos		Destination position to draw the surface.
	 * @param transColor	Transparency color to ignore copying of.
	 * @param flipped		Whether to horizontally flip the image.
	 * @param overrideColor	Optional color to use instead of non-transparent pixels from
	 *						the source surface.
	 * @param srcAlpha		Optional additional transparency applied to @p src.
	 */
	void transBlitFrom(const ManagedSurface &src, const Common::Point &destPos,
		uint32 transColor = 0, bool flipped = false, uint32 overrideColor = 0, uint32 srcAlpha = 0xff);

	/**
	 * Copy another surface into this one, ignoring pixels of a designated transparent color.
	 *
	 * @param src			Source surface.
	 * @param destPos		Destination position to draw the surface.
	 * @param mask			Mask definition.
	 */
	void transBlitFrom(const ManagedSurface &src, const Common::Point &destPos,
		const ManagedSurface &mask);

	/**
	 * Copy another surface into this one, ignoring pixels of a designated transparent color.
	 *
	 * @param src			Source surface.
	 * @param srcRect		Subsection of the source surface to draw.
	 * @param destPos		Destination position to draw the surface.
	 * @param transColor	Transparency color to ignore copying of.
	 * @param flipped		Whether to horizontally flip the image.
	 * @param overrideColor	Optional color to use instead of non-transparent pixels from
	 *						the source surface.
	 * @param srcAlpha		Optional additional transparency applied to @p src.
	 */
	void transBlitFrom(const ManagedSurface &src, const Common::Rect &srcRect, const Common::Point &destPos,
		uint32 transColor = 0, bool flipped = false, uint32 overrideColor = 0, uint32 srcAlpha = 0xff);

	/**
	 * Copy another surface into this one, ignoring pixels of a designated transparent color.
	 *
	 * @param src			Source surface.
	 * @param srcRect		Subsection of the source surface to draw.
	 * @param destRect		Destination area to draw the surface in. This can be sized differently
	 *						then @p srcRect, allowing for arbitrary scaling of the image.
	 * @param transColor	Transparency color to ignore copying of.
	 * @param flipped		Whether to horizontally flip the image.
	 * @param overrideColor	Optional color to use instead of non-transparent pixels from
	 *						the source surface.
	 * @param srcAlpha		Optional additional transparency applied to @p src.
	 * @param mask			Optional parameter with mask definition.
	 * @param maskOnly		Optional parameter for using mask over @p transColor.
	 */
	void transBlitFrom(const ManagedSurface &src, const Common::Rect &srcRect, const Common::Rect &destRect,
		uint32 transColor = 0, bool flipped = false, uint32 overrideColor = 0, uint32 srcAlpha = 0xff,
		const Surface *mask = nullptr, bool maskOnly = false);

	/**
	 * Does a blitFrom ignoring any transparency settings
	 */
	void rawBlitFrom(const ManagedSurface &src, const Common::Rect &srcRect,
					 const Common::Point &destPos);
	
	/**
	 * ManagedSurface::blendBlitTo is meant to be a highly optimized
	 * blending/blitting function, so it can only accept certain format combinations.
	 * @return true if the formats can be used by blendBlitTo.
	 */
	static inline bool isBlendBlitPixelFormatSupported(const PixelFormat &src, const PixelFormat &dst) {
		return BlendBlit::getSupportedPixelFormat() == src && BlendBlit::getSupportedPixelFormat() == dst;
	}

	/**
	 * @brief Renders this surface onto target
	 * @param target renders this surface onto this one
	 * @param src source surface
	 * @param posX, posY are the position of the src onto this surface
	 * @param flipping flipping flags (use Graphics::FLIP_FLAGS)
	 * @param srcRect source clipping
	 * @param width width of destination
	 * @param height height of destination
	 * @param colorMod what color to multiply by (0xffffffff does nothing)
	 * @param blend the blending mode to use.
	 * @param alphaType what alpha mode to use. FULL is default
	 * @return returns the size of the rendered rectangle
	 */
	Common::Rect blendBlitTo(ManagedSurface &target,
							 const int posX = 0, const int posY = 0,
							 const int flipping = FLIP_NONE,
							 const Common::Rect *srcRect = nullptr,
							 const uint colorMod = MS_ARGB(255, 255, 255, 255),
							 const int width = -1, const int height = -1,
							 const TSpriteBlendMode blend = BLEND_NORMAL,
							 const AlphaType alphaType = ALPHA_FULL);
	Common::Rect blendBlitTo(Surface &target,
							 const int posX = 0, const int posY = 0,
							 const int flipping = FLIP_NONE,
							 const Common::Rect *srcRect = nullptr,
							 const uint colorMod = MS_ARGB(255, 255, 255, 255),
							 const int width = -1, const int height = -1,
							 const TSpriteBlendMode blend = BLEND_NORMAL,
							 const AlphaType alphaType = ALPHA_FULL);

	/**
	 * Clear the entire surface.
	 */
	void clear(uint32 color = 0);

	/**
	 * Mark the entire surface as dirty.
	 */
	void markAllDirty();

	/**
	 * Copy a bitmap to the internal buffer of the surface.
	 *
	 * The pixel format of the buffer must match the pixel format of the surface.
	 */
	void copyRectToSurface(const void *buffer, int srcPitch, int destX, int destY, int width, int height) {
		_innerSurface.copyRectToSurface(buffer, srcPitch, destX, destY, width, height);
	}

	/**
	 * Copy a bitmap to the internal buffer of the surface.
	 *
	 * The pixel format of the buffer must match the pixel format of the surface.
	 */
	void copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY, const Common::Rect subRect) {
		_innerSurface.copyRectToSurface(srcSurface, destX, destY, subRect);
	}

	/**
	 * Copy a bitmap to the internal buffer of the surface.
	 *
	 * The pixel format of the buffer must match the pixel format of the surface.
	 */
	void copyRectToSurfaceWithKey(const void *buffer, int srcPitch, int destX, int destY, int width, int height, uint32 key) {
		_innerSurface.copyRectToSurfaceWithKey(buffer, srcPitch, destX, destY, width, height, key);
	}

	/**
	 * Copy a bitmap to the internal buffer of the surface.
	 *
	 * The pixel format of the buffer must match the pixel format of the surface.
	 */
	void copyRectToSurfaceWithKey(const Graphics::Surface &srcSurface, int destX, int destY, const Common::Rect subRect, uint32 key) {
		_innerSurface.copyRectToSurfaceWithKey(srcSurface, destX, destY, subRect, key);
	}

	/**
	 * Copy the data from another surface, reinitializing the
	 * surface to match the dimensions of the passed surface.
	 */
	void copyFrom(const ManagedSurface &surf);

	/**
	 * Copy the data from another surface, reinitializing the
	 * surface to match the dimensions of the passed surface.
	 */
	void copyFrom(const Surface &surf);

	/**
	 * Draw a line.
	 */
	void drawLine(int x0, int y0, int x1, int y1, uint32 color) {
		_innerSurface.drawLine(x0, y0, x1, y1, color);
		addDirtyRect(Common::Rect(MIN(x0, x1), MIN(y0, y1), MAX(x0, x1 + 1), MAX(y0, y1 + 1)));
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
	 * Return a sub-area of the screen, but only add a single initial dirty rect
	 * for the retrieved area.
	 */
	Surface getSubArea(const Common::Rect &area) {
		addDirtyRect(area);
		return _innerSurface.getSubArea(area);
	}

	/**
	 * Convert the data to another pixel format.
	 *
	 * This works in-place. This means it does not create an additional buffer
	 * for the conversion process. The value of 'pixels' might change though
	 * (that means it might realloc the pixel data).
	 *
	 * @param dstFormat  The desired format.
	 */
	void convertToInPlace(const PixelFormat &dstFormat) {
		_innerSurface.convertToInPlace(dstFormat);
	}

	/**
	 * Convert the data to another pixel format.
	 *
	 * This works in-place. This means it does not create an additional buffer
	 * for the conversion process. The value of 'pixels' might change though
	 * (that means it might realloc the pixel data).
	 *
	 * @param dstFormat  The desired format.
	 * @param palette    The palette (in RGB888), if the source format has one.
	 * @param paletteStart  The starting index of the palette.
	 * @param paletteCount  The number of colors in the palette.
	 */
	void convertToInPlace(const PixelFormat &dstFormat, const byte *palette, byte paletteStart, uint16 paletteCount) {
		_innerSurface.convertToInPlace(dstFormat, palette, paletteStart, paletteCount);
	}

	/**
	 * Return the current transparent color.
	 */
	uint getTransparentColor() const { return _transparentColor; }

	/**
	 * Set the transparent color.
	 */
	void setTransparentColor(uint32 color) {
		_transparentColor = color;
		_transparentColorSet = true;
	}

	/**
	 * Clear the transparent color setting.
	 */
	void clearTransparentColor() {
		_transparentColorSet = false;
	}

	/**
	 * Return true if a transparent color has been set.
	 */
	bool hasTransparentColor() const {
		return _transparentColorSet;
	}

	/**
	 * Clear any existing palette.
	 */
	void clearPalette();

	/**
	 * Return true if a palette has been set.
	 */
	bool hasPalette() const;

	/**
	 * Grab the palette using RGB tuples.
	 */
	void grabPalette(byte *colors, uint start, uint num) const;
	void grabPalette(Palette &palette, uint start, uint num) const;

	/**
	 * Set the palette using RGB tuples.
	 */
	void setPalette(const byte *colors, uint start, uint num);
	void setPalette(const Palette &palette, uint start, uint num);
};
/** @} */
} // End of namespace Graphics

#endif
