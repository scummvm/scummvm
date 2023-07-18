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

#ifndef GRAPHICS_BLIT_H
#define GRAPHICS_BLIT_H

#include "graphics/pixelformat.h"
#include "graphics/transform_struct.h"

namespace Common {
struct Point;
}

namespace Graphics {

/**
 * @defgroup graphics_blit Blitting
 * @ingroup graphics
 *
 * @brief Graphics blitting operations.
 *
 * @{
 */

struct TransformStruct;

/** Converting a palette for use with crossBlitMap(). */
inline static void convertPaletteToMap(uint32 *dst, const byte *src, uint colors, const Graphics::PixelFormat &format) {
	while (colors-- > 0) {
		*dst++ = format.RGBToColor(src[0], src[1], src[2]);
		src += 3;
	}
}

/**
 * Blits a rectangle.
 *
 * @param dst			the buffer which will receive the converted graphics data
 * @param src			the buffer containing the original graphics data
 * @param dstPitch		width in bytes of one full line of the dest buffer
 * @param srcPitch		width in bytes of one full line of the source buffer
 * @param w				the width of the graphics data
 * @param h				the height of the graphics data
 * @param bytesPerPixel	the number of bytes per pixel
 */
void copyBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel);

/**
 * Blits a rectangle with a transparent color key.
 *
 * @param dst			the buffer which will receive the converted graphics data
 * @param src			the buffer containing the original graphics data
 * @param dstPitch		width in bytes of one full line of the dest buffer
 * @param srcPitch		width in bytes of one full line of the source buffer
 * @param w				the width of the graphics data
 * @param h				the height of the graphics data
 * @param bytesPerPixel	the number of bytes per pixel
 * @param key			the transparent color key
 */
bool keyBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel, const uint32 key);

/**
 * Blits a rectangle from one graphical format to another.
 *
 * @param dst		the buffer which will receive the converted graphics data
 * @param src		the buffer containing the original graphics data
 * @param dstPitch	width in bytes of one full line of the dest buffer
 * @param srcPitch	width in bytes of one full line of the source buffer
 * @param w			the width of the graphics data
 * @param h			the height of the graphics data
 * @param dstFmt	the desired pixel format
 * @param srcFmt	the original pixel format
 * @return			true if conversion completes successfully,
 *					false if there is an error.
 *
 * @note Blitting to a 3Bpp destination is not supported
 * @note This can convert a surface in place, regardless of the
 *       source and destination format, as long as there is enough
 *       space for the destination. The dstPitch / srcPitch ratio
 *       must at least equal the dstBpp / srcBpp ratio for
 *       dstPitch >= srcPitch and at most dstBpp / srcBpp for
 *       dstPitch < srcPitch though.
 */
bool crossBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const Graphics::PixelFormat &dstFmt, const Graphics::PixelFormat &srcFmt);

/**
 * Blits a rectangle from one graphical format to another with a transparent color key.
 *
 * @param dst		the buffer which will receive the converted graphics data
 * @param src		the buffer containing the original graphics data
 * @param dstPitch	width in bytes of one full line of the dest buffer
 * @param srcPitch	width in bytes of one full line of the source buffer
 * @param w			the width of the graphics data
 * @param h			the height of the graphics data
 * @param dstFmt	the desired pixel format
 * @param srcFmt	the original pixel format
 * @param key			the transparent color key in the original pixel format
 * @return			true if conversion completes successfully,
 *					false if there is an error.
 *
 * @note Blitting to a 3Bpp destination is not supported
 * @note This can convert a surface in place, regardless of the
 *       source and destination format, as long as there is enough
 *       space for the destination. The dstPitch / srcPitch ratio
 *       must at least equal the dstBpp / srcBpp ratio for
 *       dstPitch >= srcPitch and at most dstBpp / srcBpp for
 *       dstPitch < srcPitch though.
 */
bool crossKeyBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const Graphics::PixelFormat &dstFmt, const Graphics::PixelFormat &srcFmt, const uint32 key);

bool crossBlitMap(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel, const uint32 *map);

bool crossKeyBlitMap(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel, const uint32 *map, const uint32 key);

bool scaleBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint dstW, const uint dstH,
			   const uint srcW, const uint srcH,
			   const Graphics::PixelFormat &fmt,
			   const byte flip = 0);

bool scaleBlitBilinear(byte *dst, const byte *src,
					   const uint dstPitch, const uint srcPitch,
					   const uint dstW, const uint dstH,
					   const uint srcW, const uint srcH,
					   const Graphics::PixelFormat &fmt,
					   const byte flip = 0);

bool rotoscaleBlit(byte *dst, const byte *src,
				   const uint dstPitch, const uint srcPitch,
				   const uint dstW, const uint dstH,
				   const uint srcW, const uint srcH,
				   const Graphics::PixelFormat &fmt,
				   const TransformStruct &transform,
				   const Common::Point &newHotspot);

bool rotoscaleBlitBilinear(byte *dst, const byte *src,
						   const uint dstPitch, const uint srcPitch,
						   const uint dstW, const uint dstH,
						   const uint srcW, const uint srcH,
						   const Graphics::PixelFormat &fmt,
						   const TransformStruct &transform,
						   const Common::Point &newHotspot);

bool applyColorKey(byte *dst, const byte *src,
                   const uint dstPitch, const uint srcPitch,
                   const uint w, const uint h,
                   const Graphics::PixelFormat &format, const bool overwriteAlpha,
                   const uint8 rKey, const uint8 gKey, const uint8 bKey,
                   const uint8 rNew, const uint8 gNew, const uint8 bNew);

bool setAlpha(byte *dst, const byte *src,
              const uint dstPitch, const uint srcPitch,
              const uint w, const uint h,
              const Graphics::PixelFormat &format,
              const bool skipTransparent, const uint8 alpha);

/**
 * Returns the pixel format all operations of TransparentSurface support.
 *
 * Unlike Surface TransparentSurface only works with a fixed pixel format.
 * This format can be queried using this static function.
 *
 * @return Supported pixel format.
 */
inline PixelFormat getSupportedBlenderPixelFormat() {
	return PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
}

void opaqueBlendBlit(byte *dst, byte *src,
					 const uint dstPitch, const uint srcPitch,
					 const int posX, const int posY,
					 const uint width, const uint height,
					 const uint32 colorMod = 0, const uint flipping = FLIP_NONE);

void binaryBlendBlit(byte *dst, byte *src,
					 const uint dstPitch, const uint srcPitch,
					 const int posX, const int posY,
					 const uint width, const uint height,
					 const uint32 colorMod = 0, const uint flipping = FLIP_NONE);

// Only blits to and from 32bpp images
void multiplyBlendBlit(byte *dst, byte *src,
					   const uint dstPitch, const uint srcPitch,
					   const int posX, const int posY,
					   const uint width, const uint height,
					   const uint32 colorMod = 0, const uint flipping = FLIP_NONE);

// Only blits to and from 32bpp images
void subtractiveBlendBlit(byte *dst, byte *src,
						  const uint dstPitch, const uint srcPitch,
						  const int posX, const int posY,
						  const uint width, const uint height,
						  const uint32 colorMod = 0, const uint flipping = FLIP_NONE);

// Only blits to and from 32bpp images
void additiveBlendBlit(byte *dst, byte *src,
					   const uint dstPitch, const uint srcPitch,
					   const int posX, const int posY,
					   const uint width, const uint height,
					   const uint32 colorMod = 0, const uint flipping = FLIP_NONE);

// Only blits to and from 32bpp images
void alphaBlendBlit(byte *dst, byte *src,
					const uint dstPitch, const uint srcPitch,
					const int posX, const int posY,
					const uint width, const uint height,
					const uint32 colorMod = 0, const uint flipping = FLIP_NONE);

/** @} */
} // End of namespace Graphics

#endif // GRAPHICS_BLIT_H
