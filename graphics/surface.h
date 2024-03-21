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

#ifndef GRAPHICS_SURFACE_H
#define GRAPHICS_SURFACE_H

#include "common/scummsys.h"
#include "common/endian.h"
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

struct TransformStruct;

enum DitherMethod {
	kDitherNaive,
	kDitherFloyd,
	kDitherAtkinson,
	kDitherBurkes,
	kDitherFalseFloyd,
	kDitherSierra,
	kDitherSierraTwoRow,
	kDitherSierraLite,
	kDitherStucki,
	kDitherJarvis,
};

/**
 * An arbitrary graphics surface that can be the target (or source) of blit
 * operations, font rendering, etc.
 */
struct Surface {
	/**
	 * Width of the surface.
	 */
	int16 w;

	/**
	 * Height of the surface.
	 */
	int16 h;

	/**
	 * Number of bytes in a pixel line.
	 *
	 * @note This might not equal w * bytesPerPixel.
	 */
	int32 pitch;

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
	 * Return the pixel at the specified point.
	 *
	 * @param x  The x coordinate of the pixel.
	 * @param y  The y coordinate of the pixel.
	 *
	 * @return The value of the pixel.
	 */
	inline uint32 getPixel(int x, int y) const {
		assert(format.bytesPerPixel > 0 && format.bytesPerPixel <= 4);
		if (format.bytesPerPixel == 1)
			return *((const uint8 *)getBasePtr(x, y));
		else if (format.bytesPerPixel == 2)
			return *((const uint16 *)getBasePtr(x, y));
		else if (format.bytesPerPixel == 3)
			return READ_UINT24(getBasePtr(x, y));
		else if (format.bytesPerPixel == 4)
			return *((const uint32 *)getBasePtr(x, y));
		else
			return 0;
	}

	/**
	 * Set the pixel at the specified point.
	 *
	 * @param x     The x coordinate of the pixel.
	 * @param y     The y coordinate of the pixel.
	 * @param pixel The value of the pixel.
	 */
	inline void setPixel(int x, int y, int pixel) {
		assert(format.bytesPerPixel > 0 && format.bytesPerPixel <= 4);
		assert(x >= 0 && x < w && y >= 0 && y < h);
		if (format.bytesPerPixel == 1)
			*((uint8 *)getBasePtr(x, y)) = pixel;
		else if (format.bytesPerPixel == 2)
			*((uint16 *)getBasePtr(x, y)) = pixel;
		else if (format.bytesPerPixel == 3)
			WRITE_UINT24(getBasePtr(x, y), pixel);
		else if (format.bytesPerPixel == 4)
			*((uint32 *)getBasePtr(x, y)) = pixel;
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
	void create(int16 width, int16 height, const PixelFormat &format);

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
	void init(int16 width, int16 height, int16 pitch, void *pixels, const PixelFormat &format);

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
	 * Convert the data from another surface to the specified format.
	 *
	 * This calls @ref free on the current surface to assure that it is
	 * clean. Make sure that the current data was created using @ref create.
	 * Otherwise, the results are undefined.
	 *
	 * @param surf    The surface to convert from.
	 * @param format  The pixel format to convert to.
	 */
	void convertFrom(const Surface &surf, const PixelFormat &format);

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
	 * Clip the given source bounds so the passed destBounds will be entirely on-screen.
	 */
	bool clip(Common::Rect &srcBounds, Common::Rect &destBounds) const;

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
	 * @param key
	 */
	void copyRectToSurfaceWithKey(const void *buffer, int srcPitch, int destX, int destY, int width, int height, uint32 key);

	/**
	 * Copy a bitmap to the internal buffer of the surface.
	 *
	 * The pixel format of the buffer must match the pixel format of the surface.
	 *
	 * @param srcSurface  Source of the bitmap data.
	 * @param destX       The x coordinate of the destination rectangle.
	 * @param destY       The y coordinate of the destination rectangle.
	 * @param subRect     The subRect of the surface to be blitted.
	 * @param key
	 */
	void copyRectToSurfaceWithKey(const Graphics::Surface &srcSurface, int destX, int destY, const Common::Rect subRect, uint32 key);

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
	 */
	inline void convertToInPlace(const PixelFormat &dstFormat) {
		convertToInPlace(dstFormat, nullptr, 0);
	}

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
	 * @param palette    The palette (in RGB888), if the source format has one.
	 * @param paletteCount	The number of colors in the palette.
	 */
	void convertToInPlace(const PixelFormat &dstFormat, const byte *palette, uint16 paletteCount);

	/**
	 * Convert the data to another pixel format.
	 *
	 * The client code must call @ref free on the returned surface and then delete
	 * it.
	 *
	 * @param dstFormat   The desired format.
	 * @param srcPalette  The palette (in RGB888), if the source format has a bpp of 1.
	 * @param srcPaletteCount The color count in the for the srcPalette.
	 * @param dstPalette  The palette (in RGB888), If the destination format has a bpp of 1.
	 * @param dstaletteCount The color count in the for the dstPalette.
	 * @param method      The dithering method if destination format has a bpp of 1. Default is Floyd-Steinberg.
	 */
	Graphics::Surface *convertTo(const PixelFormat &dstFormat, const byte *srcPalette = 0, int srcPaletteCount = 0, const byte *dstPalette = 0, int dstPaletteCount = 0, DitherMethod method = kDitherFloyd) const;

protected:
	void ditherFloyd(const byte *srcPalette, int srcPaletteCount, Surface *dstSurf, const byte *dstPalette, int dstPaletteCount, DitherMethod method, const PixelFormat &dstFormat) const;

public:

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
	 * Flip the specified rect horizontally.
	 *
	 * @param r  The rectangle to flip.
	 */
	void flipHorizontal(const Common::Rect &r);

	/**
	 * Writes a color key to the alpha channel of the surface
	 * @param rKey  the red component of the color key
	 * @param gKey  the green component of the color key
	 * @param bKey  the blue component of the color key
	 * @param overwriteAlpha if true, all other alpha will be set fully opaque
	 */
	bool applyColorKey(uint8 rKey, uint8 gKey, uint8 bKey, bool overwriteAlpha = false);

	/**
	 * Writes a color key to the alpha channel of the surface
	 * @param rKey  the red component of the color key
	 * @param gKey  the green component of the color key
	 * @param bKey  the blue component of the color key
	 * @param overwriteAlpha if true, all other alpha will be set fully opaque
	 * @param rNew  the red component to replace the color key with
	 * @param gNew  the green component to replace the color key with
	 * @param bNew  the blue component to replace the color key with
	 */
	bool applyColorKey(uint8 rKey, uint8 gKey, uint8 bKey, bool overwriteAlpha,
	                   uint8 rNew, uint8 gNew, uint8 bNew);

	/**
	 * Sets alpha channel for all pixels to specified value
	 * @param alpha  value of the alpha channel to set
	 * @param skipTransparent  if set to true, then do not touch pixels with alpha=0
	 */
	bool setAlpha(uint8 alpha, bool skipTransparent = false);

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
	Graphics::Surface *scale(int16 newWidth, int16 newHeight, bool filtering = false) const;

	/**
	 * @brief Rotoscale function; this returns a transformed version of this surface after rotation and
	 * scaling. Please do not use this if angle == 0, use plain old scaling function.
	 *
	 * The client code must call @ref free on the returned surface and then delete
	 * it.
	 *
	 * @param transform a TransformStruct wrapping the required info. @see TransformStruct
	 * @param filtering Whether or not to use bilinear filtering.
	 *
	 */
	Graphics::Surface *rotoscale(const TransformStruct &transform, bool filtering = false) const;

	/**
	 * Print surface content on console in pseudographics
	 *
	 * @param debuglevel debug level to print at, default is 0.
	 * @param width width of the printed area in pixels. Default is 0 which is whole surface.
	 * @param height height of the printed area in pixels. Default is 0 which is whole surface.
	 * @param x horizontal offset to the print area. Default is 0.
	 * @param y vertical offset to the print area. Default is 0.
	 * @param scale number of pixels per single character. Default is -1, fit whole surface to maxwidth
	 * @param maxwidth horizontal size of the print out in characters. Default is 160. Note that 2 characters
	 *                 are taken by the frame
	 * @param palette palette to use for 1bpp pixels. If omitted, we assume grayscale palette
	 *
	 */
	void debugPrint(int debuglevel = 0, int width = 0, int height = 0, int x = 0, int y = 0, int scale = -1, int maxwidth = 160, const byte *palette = NULL) const;
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
