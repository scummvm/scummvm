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

#include "graphics/blit.h"
#include "graphics/pixelformat.h"

namespace Graphics {

namespace {

template<typename Size, bool overwriteAlpha>
inline void applyColorKeyLogic(byte *dst, const byte *src, const uint w, const uint h,
                               const uint srcDelta, const uint dstDelta,
                               const Graphics::PixelFormat &format,
                               const uint8 rKey, const uint8 gKey, const uint8 bKey,
                               const uint8 rNew, const uint8 gNew, const uint8 bNew) {

	const uint32 keyPix    = format.ARGBToColor(0,   rKey, gKey, bKey);
	const uint32 newPix    = format.ARGBToColor(0,   rNew, gNew, bNew);
	const uint32 rgbMask   = format.ARGBToColor(0,   255,  255,  255);
	const uint32 alphaMask = format.ARGBToColor(255, 0,    0,    0);

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			uint32 pix = *(const Size *)src;

			if ((pix & rgbMask) == keyPix) {
				*(Size *)dst = newPix;
			} else if (overwriteAlpha) {
				*(Size *)dst = pix | alphaMask;
			}

			src += sizeof(Size);
			dst += sizeof(Size);
		}

		src += srcDelta;
		dst += dstDelta;
	}
}

template<typename Size, bool skipTransparent>
inline void setAlphaLogic(byte *dst, const byte *src, const uint w, const uint h,
                          const uint srcDelta, const uint dstDelta,
                          const Graphics::PixelFormat &format, const uint8 alpha) {

	const uint32 newAlpha  = format.ARGBToColor(alpha, 0,   0,   0);
	const uint32 rgbMask   = format.ARGBToColor(0,     255, 255, 255);
	const uint32 alphaMask = format.ARGBToColor(255,   0,   0,   0);

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			uint32 pix = *(const Size *)src;

			if (!skipTransparent || (pix & alphaMask))
				*(Size *)dst = (pix & rgbMask) | newAlpha;
			else
				*(Size *)dst = pix;

			src += sizeof(Size);
			dst += sizeof(Size);
		}

		src += srcDelta;
		dst += dstDelta;
	}
}

} // End of anonymous namespace

// Function to merge a transparent color key with the alpha channel
bool applyColorKey(byte *dst, const byte *src,
                   const uint dstPitch, const uint srcPitch,
                   const uint w, const uint h,
                   const Graphics::PixelFormat &format, const bool overwriteAlpha,
                   const uint8 rKey, const uint8 gKey, const uint8 bKey,
                   const uint8 rNew, const uint8 gNew, const uint8 bNew) {

	// Faster, but larger, to provide optimized handling for each case.
	const uint srcDelta = (srcPitch - w * format.bytesPerPixel);
	const uint dstDelta = (dstPitch - w * format.bytesPerPixel);

	if (format.aBits() == 0) {
		return false;
	}

	if (overwriteAlpha) {
		if (format.bytesPerPixel == 1) {
			applyColorKeyLogic<uint8, true>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
		} else if (format.bytesPerPixel == 2) {
			applyColorKeyLogic<uint16, true>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
		} else if (format.bytesPerPixel == 4) {
			applyColorKeyLogic<uint32, true>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
		} else {
			return false;
		}
	} else {
		if (format.bytesPerPixel == 1) {
			applyColorKeyLogic<uint8, false>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
		} else if (format.bytesPerPixel == 2) {
			applyColorKeyLogic<uint16, false>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
		} else if (format.bytesPerPixel == 4) {
			applyColorKeyLogic<uint32, false>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
		} else {
			return false;
		}
	}

	return true;
}

// Function to set the alpha channel for all pixels to the specified value
bool setAlpha(byte *dst, const byte *src,
              const uint dstPitch, const uint srcPitch,
              const uint w, const uint h,
              const Graphics::PixelFormat &format,
              const bool skipTransparent, const uint8 alpha) {

	// Faster, but larger, to provide optimized handling for each case.
	const uint srcDelta = (srcPitch - w * format.bytesPerPixel);
	const uint dstDelta = (dstPitch - w * format.bytesPerPixel);

	if (format.aBits() == 0) {
		return false;
	}

	if (skipTransparent) {
		if (format.bytesPerPixel == 1) {
			setAlphaLogic<uint8, true>(dst, src, w, h, srcDelta, dstDelta, format, alpha);
		} else if (format.bytesPerPixel == 2) {
			setAlphaLogic<uint16, true>(dst, src, w, h, srcDelta, dstDelta, format, alpha);
		} else if (format.bytesPerPixel == 4) {
			setAlphaLogic<uint32, true>(dst, src, w, h, srcDelta, dstDelta, format, alpha);
		} else {
			return false;
		}
	} else {
		if (format.bytesPerPixel == 1) {
			setAlphaLogic<uint8, false>(dst, src, w, h, srcDelta, dstDelta, format, alpha);
		} else if (format.bytesPerPixel == 2) {
			setAlphaLogic<uint16, false>(dst, src, w, h, srcDelta, dstDelta, format, alpha);
		} else if (format.bytesPerPixel == 4) {
			setAlphaLogic<uint32, false>(dst, src, w, h, srcDelta, dstDelta, format, alpha);
		} else {
			return false;
		}
	}

	return true;
}

} // End of namespace Graphics
