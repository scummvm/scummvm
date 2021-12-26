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

#ifndef GRAPHICS_CONVERSION_H
#define GRAPHICS_CONVERSION_H

#include "common/util.h"

#include "graphics/pixelformat.h"

namespace Common {
struct Point;
}

namespace Graphics {

/**
 * @defgroup graphics_conversion Conversions
 * @ingroup graphics
 *
 * @brief Graphics conversion operations.
 *
 * @{
 */

struct TransformStruct;

/** Converting a color from YUV to RGB colorspace. */
inline static void YUV2RGB(byte y, byte u, byte v, byte &r, byte &g, byte &b) {
	r = CLIP<int>(y + ((1357 * (v - 128)) >> 10), 0, 255);
	g = CLIP<int>(y - (( 691 * (v - 128)) >> 10) - ((333 * (u - 128)) >> 10), 0, 255);
	b = CLIP<int>(y + ((1715 * (u - 128)) >> 10), 0, 255);
}

/** Converting a color from RGB to YUV colorspace. */
inline static void RGB2YUV(byte r, byte g, byte b, byte &y, byte &u, byte &v) {
	y = CLIP<int>( ((r * 306) >> 10) + ((g * 601) >> 10) + ((b * 117) >> 10)      , 0, 255);
	u = CLIP<int>(-((r * 172) >> 10) - ((g * 340) >> 10) + ((b * 512) >> 10) + 128, 0, 255);
	v = CLIP<int>( ((r * 512) >> 10) - ((g * 429) >> 10) - ((b *  83) >> 10) + 128, 0, 255);
}

/** Converting a palette for use with crossBlitMap(). */
inline static void convertPaletteToMap(uint32 *dst, const byte *src, uint colors, const Graphics::PixelFormat &format) {
	while (colors-- > 0) {
		*dst++ = format.RGBToColor(src[0], src[1], src[2]);
		src += 3;
	}
}

// TODO: generic YUV to RGB blit

/**
 * Blits a rectangle.
 *
 * @param dst			the buffer which will recieve the converted graphics data
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
 * @param dst			the buffer which will recieve the converted graphics data
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
 * @param dst		the buffer which will recieve the converted graphics data
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

bool crossBlitMap(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel, const uint32 *map);

bool scaleBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint dstW, const uint dstH,
			   const uint srcW, const uint srcH,
			   const Graphics::PixelFormat &fmt);

bool scaleBlitBilinear(byte *dst, const byte *src,
					   const uint dstPitch, const uint srcPitch,
					   const uint dstW, const uint dstH,
					   const uint srcW, const uint srcH,
					   const Graphics::PixelFormat &fmt);

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
/** @} */
} // End of namespace Graphics

#endif // GRAPHICS_CONVERSION_H
