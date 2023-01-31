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
#include "common/debug.h"
#include "graphics/scaler/downscaler.h"
#include "graphics/scaler/intern.h"

namespace Graphics {

#ifdef USE_ARM_SCALER_ASM
extern "C" {
	void downscaleAllByHalfARM(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height, int mask, int round);
}

void downscaleAllByHalf(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height, int gBitFormat) {
	// Rounding constants and masks used for different pixel formats
	static const int roundingconstants[] = { 0x00200802, 0x00201002 };
	static const int redbluegreenMasks[] = { 0x03E07C1F, 0x07E0F81F };

	const int maskUsed = (gBitFormat == 565);
	downscaleAllByHalfARM(srcPtr, srcPitch, dstPtr, dstPitch, width, height, redbluegreenMasks[maskUsed], roundingconstants[maskUsed]);
}

#else

template<typename ColorMask>
void downscaleAllByHalfTemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	uint8 *work;
	uint16 srcPitch16 = (uint16)(srcPitch / sizeof(uint16));

	while ((height -= 2) >= 0) {
		work = dstPtr;

		for (int i=0; i<width; i+=2) {
			// Another lame filter attempt :)
			uint16 color1 = *(((const uint16 *)srcPtr) + i);
			uint16 color2 = *(((const uint16 *)srcPtr) + (i + 1));
			uint16 color3 = *(((const uint16 *)srcPtr) + (i + srcPitch16));
			uint16 color4 = *(((const uint16 *)srcPtr) + (i + srcPitch16 + 1));
			*(((uint16 *)work) + 0) = interpolate16_1_1_1_1<ColorMask>(color1, color2, color3, color4);

			work += sizeof(uint16);
		}
		srcPtr += 2 * srcPitch;
		dstPtr += dstPitch;
	}
}

void downscaleAllByHalf(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height, int gBitFormat) {
	if (gBitFormat == 565)
		downscaleAllByHalfTemplate<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		downscaleAllByHalfTemplate<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}

#endif

namespace {
uint32 getBrightness(byte col, const byte *pal) {
	return pal[3 * col] * pal[3 * col] + pal[3 * col + 1] * pal[3 * col + 1] + pal[3 * col + 2] * pal[3 * col + 2];
}

void downscaleCLUT8ByHalf(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int maxX, int maxY, const byte *palette) {
	const byte *src1 = srcPtr;
	const byte *src2 = srcPtr + srcPitch;
	byte *dst = dstPtr;
	int32 srcSkip = 2 * srcPitch - maxX * 2;
	int32 dstSkip = dstPitch - maxX;
	for (int y = 0; y < maxY; y++) {
		for (int x = 0; x < maxX; x++) {
			// Choose the brightest pixel. Writing often is bright on
			// a dark background, so this preserves text as much as we can.
			byte colors[4] = {
				*src1++,
				*src1++,
				*src2++,
				*src2++
			};

			byte col = colors[0];
			uint32 bri = getBrightness(col, palette);
			for (uint i = 1; i < 4; i++) {
				uint32 nbri = getBrightness(colors[i], palette);
				if (nbri > bri) {
					bri = nbri;
					col = colors[i];
				}
			}

			*dst++ = col;
		}
		src1 += srcSkip;
		src2 += srcSkip;
		dst += dstSkip;
	}
}
}

void downscaleSurfaceByHalf(Surface *out, const Surface *in, const byte *palette) {
	if (in->format.isCLUT8() && out->format.isCLUT8() && palette != nullptr) {
		downscaleCLUT8ByHalf((const byte *) in->getBasePtr(0, 0), in->pitch, (byte *) out->getBasePtr(0, 0), out->pitch,
				     MIN<int>(out->w, in->w / 2), MIN<int>(out->h, in->h / 2), palette);
		return;
	}

	if ((in->format == PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)
	     && out->format == PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0))
	    || (in->format == PixelFormat(2, 5, 6, 5, 0, 0, 5, 11, 0)
		&& out->format == PixelFormat(2, 5, 6, 5, 0, 0, 5, 11, 0))) {
		downscaleAllByHalf((const byte *) in->getBasePtr(0, 0), in->pitch, (byte *) out->getBasePtr(0, 0), out->pitch,
				   MIN<int>(in->w, out->w * 2), MIN<int>(in->h, out->h * 2), 565);
		return;
	}

	if ((in->format == PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0)
	     && out->format == PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0))
	    || (in->format == PixelFormat(2, 5, 5, 5, 0, 0, 5, 10, 0)
		&& out->format == PixelFormat(2, 5, 5, 5, 0, 0, 5, 10, 0))) {
		downscaleAllByHalf((const byte *) in->getBasePtr(0, 0), in->pitch, (byte *) out->getBasePtr(0, 0), out->pitch,
				   MIN<int>(in->w, out->w * 2), MIN<int>(in->h, out->h * 2), 555);
		return;
	}

	error("downscaleCLUT8ByHalf(): Unsupported downscale format %s->%s", in->format.toString().c_str(), out->format.toString().c_str());
}

}  // end of namespace Graphics
