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

#ifndef GRAPHICS_SURFACE_H
#define GRAPHICS_SURFACE_H

#include "common/scummsys.h"
#include "common/list.h"

namespace Common {
struct Rect;
struct Point;
}

#include "graphics/pixelformat.h"

namespace Graphics {

/**
 * @defgroup graphics_surface Surface
 * @ingroup graphics
 *
 * @brief Surface class for managing an arbitrary graphics surface.
 *
 * @{
 */

/**
 * An arbitrary graphics surface that can be the target (or source) of blit
 * operations, font rendering, etc.
 */
struct Surface {
	/*
	 * IMPORTANT implementation-specific detail:
	 *
	 * ARM code relies on the layout of the first 3 of these fields. Do not
	 * change them.
	 */

	/**
	 * Width of the surface.
	 */
	uint16 w;

	/**
	 * Height of the surface.
	 */
	uint16 h;

	/**
	 * Number of bytes in a pixel line.
	 *
	 * @note This might not equal w * bytesPerPixel.
	 */
	uint16 pitch;

protected:
	/**
	 * Pixel data of the surface.
	 */
	void *pixels;

public:
	/**
	 * Pixel format of the surface.
	 */
	PixelFormat format;

	/**
	 * Construct a simple Surface object.
	 */
	Surface() : w(0), h(0), pitch(0), pixels(0), format() {
	}

	/**
	 * Return a pointer to the pixel data.
	 *
	 * @return Pointer to the pixel data.
	 */
	inline const void *getPixels() const {
		return pixels;
	}

	/**
	 * Return a pointer to the pixel data.
	 *
	 * @return Pointer to the pixel data.
	 */
	inline void *getPixels() {
		return pixels;
	}

	/**
	 * Set the pixel data.
	 *
	 * Note that this is a simply a setter. Be aware of that when using it.
	 *
	 * @param newPixels The new pixel data.
	 */
	void setPixels(void *newPixels) { pixels = newPixels; }

	/**
	 * Return a pointer to the pixel at the specified point.
	 *
	 * @param x  The x coordinate of the pixel.
	 * @param y  The y coordinate of the pixel.
	 *
	 * @return Pointer to the pixel.
	 */
	inline const void *getBasePtr(int x, int y) const {
		return (const byte *)(pixels) + y * pitch + x * format.bytesPerPixel;
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
		return static_cast<byte *>(pixels) + y * pitch + x * format.bytesPerPixel;
	}

	/**
	 * Allocate memory for the pixel data of the surface.
	 *
	 * The client code is responsible for calling @ref free.
	 *
	 * @param width   Width of the surface object.
	 * @param height  Height of the surface object.
	 * @param format  The pixel format to be used by the surface.
	 */
	void create(uint16 width, uint16 height, const PixelFormat &format);

	/**
	 * Release the memory used by the pixel memory of this surface.
	 *
	 * This is the counterpart of @ref create().
	 *
	 * @b Important: Only use this if you created the surface data using
	 * @ref create. Otherwise, this function has undefined behavior.
	 */
	void free();

	/**
	 * Set up a surface with user-specified data.
	 *
	 * This simply sets the 'internal' attributes of the surface. It
	 * does free old data using @ref free or similar methods.
	 *
	 * @param width   Width of the pixel data.
	 * @param height  Height of the pixel data.
	 * @param pitch   Pitch of the pixel data.
	 * @param pixels  Pixel data.
	 * @param format  Pixel format of the pixel data.
	 */
	void init(uint16 width, uint16 height, uint16 pitch, void *pixels, const PixelFormat &format);

	/**
	 * Copy the data from another surface.
	 *
	 * This calls @ref free on the current surface to assure that it is
	 * clean. Make sure that the current data was created using @ref create.
	 * Otherwise, the results are undefined.
	 *
	 * @param surf  The surface to copy from.
	 */
	void copyFrom(const Surface &surf);

	/**
	 * Create a surface that represents a sub-area of this Surface object.
	 *
	 * The pixel (0, 0) of the returned Surface will be the same as pixel
	 * (area.x, area.y) of the parent surface. Changes to any of the parent surface
	 * objects will change the shared pixel data.
	 *
	 * The returned surface is only valid as long as this Surface
	 * object still exists, that is, its pixel data is not destroyed or
	 * reallocated.
	 *
	 * @b Important: Never attempt to free the returned surface.
	 *
	 * @param area  The area to be represented. Note that the area
	 *              will get clipped in case it does not fit.
	 */
	Surface getSubArea(const Common::Rect &area);

	/**
	 * Create a surface that represents a sub-area of this Surface object.
	 *
	 * The pixel (0, 0) of the returned surface will be the same as pixel
	 * (area.x, area.y) of the parent surface.
	 *
	 * The returned surface is only valid as long as this Surface
	 * object still exists, that is, its pixel data is not destroyed or
	 * reallocated.
	 *
	 * @b Important: Never attempt to free the returned surface.
	 *
	 * @param area  The area to be represented. Note that the area
	 *              will get clipped in case it does not fit.
	 */
	const Surface getSubArea(const Common::Rect &area) const;

	/**
	 * Copy a bitmap to the internal buffer of the surface.
	 *
	 * The pixel format of the buffer must match the pixel format of the surface.
	 *
	 * @param buffer    Buffer containing the graphics data source.
	 * @param srcPitch  Pitch of the buffer (number of bytes in a scanline).
	 * @param destX     The x coordinate of the destination rectangle.
	 * @param destY     The y coordinate of the destination rectangle.
	 * @param width     Width of the destination rectangle.
	 * @param height    Height of the destination rectangle.
	 */
	void copyRectToSurface(const void *buffer, int srcPitch, int destX, int destY, int width, int height);
	/**
	 * Copy a bitmap to the internal buffer of the surface.
	 *
	 * The pixel format of the buffer must match the pixel format of the surface.
	 *
	 * @param srcSurface  Source of the bitmap data.
	 * @param destX       The x coordinate of the destination rectangle.
	 * @param destY       The y coordinate of the destination rectangle.
	 * @param subRect     The subRect of the surface to be blitted.
	 */
	void copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY, const Common::Rect subRect);

	/**
	 * Convert the data to another pixel format.
	 *
	 * This works in-place. This means it does not create an additional buffer
	 * for the conversion process. The value of 'pixels' might change though
	 * (that means it might realloc the pixel data).
	 *
	 * @b Important: Only use this if you created the surface data using
	 * @ref create. Otherwise, this function has undefined behavior.
	 *
	 * @param dstFormat  The desired format.
	 * @param palette    The palette (in RGB888), if the source format has a bpp of 1.
	 */
	void convertToInPlace(const PixelFormat &dstFormat, const byte *palette = 0);

	/**
	 * Convert the data to another pixel format.
	 *
	 * The client code must call @ref free on the returned surface and then delete
	 * it.
	 *
	 * @param dstFormat  The desired format.
	 * @param palette    The palette (in RGB888), if the source format has a bpp of 1.
	 */
	Graphics::Surface *convertTo(const PixelFormat &dstFormat, const byte *palette = 0) const;

	/**
	 * Draw a line.
	 *
	 * @param x0     The x coordinate of the start point.
	 * @param y0     The y coordinate of the start point.
	 * @param x1     The x coordinate of the end point.
	 * @param y1     The y coordinate of the end point.
	 * @param color  Color of the line.
	 *
	 * @note This is just a wrapper around Graphics::drawLine.
	 */
	void drawLine(int x0, int y0, int x1, int y1, uint32 color);

	/**
	 * Draw a thick line.
	 *
	 * @param x0     The x coordinate of the start point.
	 * @param y0     The y coordinate of the start point.
	 * @param x1     The x coordinate of the end point.
	 * @param y1     The y coordinate of the end point.
	 * @param penX   Width of the pen (thickness in the x direction).
	 * @param penY   Height of the pen (thickness in the y direction).
	 * @param color  Color of the line.
	 *
	 * @note This is just a wrapper around Graphics::drawThickLine.
	 *
	 * @note The x/y coordinates of the start and end points are the upper leftmost part of the pen.
	 */
	void drawThickLine(int x0, int y0, int x1, int y1, int penX, int penY, uint32 color);

	/**
	 * Draw a horizontal line.
	 *
	 * @param x      The start x coordinate of the line.
	 * @param y      The y coordinate of the line.
	 * @param x2     The end x coordinate of the line.
	 *               In case x > x2, the coordinates are swapped.
	 * @param color  Color of the line.
	 */
	void hLine(int x, int y, int x2, uint32 color);

	/**
	 * Draw a vertical line.
	 *
	 * @param x      The x coordinate of the line.
	 * @param y      The start y coordinate of the line.
	 * @param y2     The end y coordinate of the line.
	 *               In case y > y2, the coordinates are swapped.
	 * @param color  Color of the line.
	 */
	void vLine(int x, int y, int y2, uint32 color);

	/**
	 * Fill a rect with a given color.
	 *
	 * @param r      The rectangle to fill.
	 * @param color  The color to fill the rect with.
	 */
	void fillRect(Common::Rect r, uint32 color);

	/**
	 * Draw a frame around a specified rect.
	 *
	 * @param r      The rectangle to frame.
	 * @param color  The color of the frame.
	 */
	void frameRect(const Common::Rect &r, uint32 color);

	/**
	 * Move the content of the surface horizontally or vertically
	 * by the given number of pixels.
	 */
	void move(int dx, int dy, int height);

	/**
	 * Flip the specified rect vertically.
	 *
	 * @param r  The rectangle to flip.
	 */
	void flipVertical(const Common::Rect &r);

	/**
	 * Scale the data to the given size.
	 *
	 * The client code must call @ref free on the returned surface and then delete
	 * it.
	 *
	 * @param newWidth   The resulting width.
	 * @param newHeight  The resulting height.
	 * @param filtering  Whether or not to use bilinear filtering.
	 */
	Graphics::Surface *scale(uint16 newWidth, uint16 newHeight, bool filtering = false) const;

};

/**
 * A deleter for Surface objects that can be used with SharedPtr.
 *
 * This deleter assures Surface::free is called on deletion.
 */
struct SurfaceDeleter {
	void operator()(Surface *ptr) {
		if (ptr) {
			ptr->free();
		}
		delete ptr;
	}
};

/**
 * Stack-based flood fill algorithm for arbitrary surfaces.
 *
 * This can be used in two ways. One is to fill the pixels of oldColor
 * with fillColor. Second is when the surface stays intact but another
 * surface with mask is created, where filled colors are marked with 255.
 *
 * Before running fill() or fillMask(), the initial pixels must be addSeed
 * with the addSeed() method.
 */
class FloodFill {
public:
	/**
	 * Construct a simple Surface object.
	 *
	 * @param surface    Input surface.
	 * @param oldColor   The color on the surface to change.
	 * @param fillColor  The color to fill with.
	 */
	FloodFill(Surface *surface, uint32 oldColor, uint32 fillColor, bool maskMode = false);
	~FloodFill();

	/**
	 * Add pixels to the fill queue.
	 *
	 * @param x  The x coordinate of the pixel.
	 * @param y  The x coordinate of the pixel.
	 */
	void addSeed(int x, int y);

	/**
	 * Fill the surface as requested.
	 *
	 * This uses pixels that were added with the @ref addSeed() method.
	 */
	void fill();

	/**
	 * Fill the mask.
	 *
	 * The mask is a CLUT8 surface with pixels 0 and 255.
	 * 255 means that the pixel has been filled.
	 *
	 * This uses pixels that were added with the @ref addSeed() method.
	 */
	void fillMask();

	/**
	 * Get the resulting mask.
	 *
	 * @see fillMask
	 */
	Surface *getMask() { return _mask; }

private:
	Common::List<Common::Point *> _queue;
	Surface *_surface;
	Surface *_mask;
	uint32 _oldColor, _fillColor;
	byte *_visited;
	int _w, _h;

	bool _maskMode;
};
/** @} */
} // End of namespace Graphics


#endif
