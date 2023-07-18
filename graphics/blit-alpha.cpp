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

static const int kBModShift = 8;
static const int kGModShift = 16;
static const int kRModShift = 24;
static const int kAModShift = 0;

static const uint32 kBModMask = 0x0000ff00;
static const uint32 kGModMask = 0x00ff0000;
static const uint32 kRModMask = 0xff000000;
static const uint32 kAModMask = 0x000000ff;
static const uint32 kRGBModMask = (kRModMask | kGModMask | kBModMask);

#ifdef SCUMM_LITTLE_ENDIAN
static const int kAIndex = 0;
static const int kBIndex = 1;
static const int kGIndex = 2;
static const int kRIndex = 3;
#else
static const int kAIndex = 3;
static const int kBIndex = 2;
static const int kGIndex = 1;
static const int kRIndex = 0;
#endif

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

/**
 * Optimized version of doBlit to be used with multiply blended blitting
 */
template<bool rgbmod, bool alphamod>
static void doBlitMultiplyBlendLogic(byte *ino, byte *outo,
									 uint32 width, uint32 height,
									 uint32 outPitch, int32 inStep,
									 int32 inoStep, uint32 color) {

	byte *in;
	byte *out;

	byte ca = alphamod ? ((color >> kAModShift) & 0xFF) : 255;
	byte cr = rgbmod   ? ((color >> kRModShift) & 0xFF) : 255;
	byte cg = rgbmod   ? ((color >> kGModShift) & 0xFF) : 255;
	byte cb = rgbmod   ? ((color >> kBModShift) & 0xFF) : 255;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (uint32 j = 0; j < width; j++) {

			uint32 ina = in[kAIndex] * ca >> 8;

			if (ina != 0) {
				if (cb != 255) {
					out[kBIndex] = MIN<uint>(out[kBIndex] * ((in[kBIndex] * cb * ina) >> 16) >> 8, 255u);
				} else {
					out[kBIndex] = MIN<uint>(out[kBIndex] * (in[kBIndex] * ina >> 8) >> 8, 255u);
				}

				if (cg != 255) {
					out[kGIndex] = MIN<uint>(out[kGIndex] * ((in[kGIndex] * cg * ina) >> 16) >> 8, 255u);
				} else {
					out[kGIndex] = MIN<uint>(out[kGIndex] * (in[kGIndex] * ina >> 8) >> 8, 255u);
				}

				if (cr != 255) {
					out[kRIndex] = MIN<uint>(out[kRIndex] * ((in[kRIndex] * cr * ina) >> 16) >> 8, 255u);
				} else {
					out[kRIndex] = MIN<uint>(out[kRIndex] * (in[kRIndex] * ina >> 8) >> 8, 255u);
				}
			}

			in += inStep;
			out += 4;
		}
		outo += outPitch;
		ino += inoStep;
	}

}

// Only blits to and from 32bpp images
void multiplyBlendBlit(byte *dst, byte *src,
					   const uint dstPitch, const uint srcPitch,
					   const int posX, const int posY,
					   const uint width, const uint height,
					   const uint32 colorMod, const uint flipping) {
	bool rgbmod   = ((colorMod & kRGBModMask) != kRGBModMask);
	bool alphamod = ((colorMod & kAModMask)   != kAModMask);

	int xp = 0, yp = 0;

	int inStep = 4;
	int inoStep = srcPitch;
	if (flipping & FLIP_H) {
		inStep = -inStep;
		xp = width - 1;
	}

	if (flipping & FLIP_V) {
		inoStep = -inoStep;
		yp = height - 1;
	}

	byte *ino = src + yp * srcPitch + xp * 4;
	byte *outo = dst + posY * dstPitch + posX * 4;

	if (rgbmod) {
		if (alphamod) {
			doBlitMultiplyBlendLogic<true, true>(ino, outo, width, height, dstPitch, inStep, inoStep, colorMod);
		} else {
			doBlitMultiplyBlendLogic<true, false>(ino, outo, width, height, dstPitch, inStep, inoStep, colorMod);
		}
	} else {
		if (alphamod) {
			doBlitMultiplyBlendLogic<false, true>(ino, outo, width, height, dstPitch, inStep, inoStep, colorMod);
		} else {
			doBlitMultiplyBlendLogic<false, false>(ino, outo, width, height, dstPitch, inStep, inoStep, colorMod);
		}
	}
}

/**
 * Optimized version of doBlit to be used with alpha blended blitting
 * @param ino a pointer to the input surface
 * @param outo a pointer to the output surface
 * @param width width of the input surface
 * @param height height of the input surface
 * @param pitch pitch of the output surface - that is, width in bytes of every row, usually bpp * width of the TARGET surface (the area we are blitting to might be smaller, do the math)
 * @inStep size in bytes to skip to address each pixel, usually bpp of the source surface
 * @inoStep width in bytes of every row on the *input* surface / kind of like pitch
 * @color colormod in 0xAARRGGBB format - 0xFFFFFFFF for no colormod
 */
template<bool rgbmod, bool alphamod>
static void doBlitAlphaBlendLogic(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color) {

	byte *in;
	byte *out;

	byte ca = alphamod ? ((color >> kAModShift) & 0xFF) : 255;
	byte cr = rgbmod   ? ((color >> kRModShift) & 0xFF) : 255;
	byte cg = rgbmod   ? ((color >> kGModShift) & 0xFF) : 255;
	byte cb = rgbmod   ? ((color >> kBModShift) & 0xFF) : 255;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (uint32 j = 0; j < width; j++) {

			uint32 ina = in[kAIndex] * ca >> 8;

			if (ina != 0) {
				uint outb = (out[kBIndex] * (255 - ina) >> 8);
				uint outg = (out[kGIndex] * (255 - ina) >> 8);
				uint outr = (out[kRIndex] * (255 - ina) >> 8);

				out[kAIndex] = 255;
				out[kBIndex] = outb + (in[kBIndex] * ina * cb >> 16);
				out[kGIndex] = outg + (in[kGIndex] * ina * cg >> 16);
				out[kRIndex] = outr + (in[kRIndex] * ina * cr >> 16);
			}

			in += inStep;
			out += 4;
		}
		outo += pitch;
		ino += inoStep;
	}
}

// Only blits to and from 32bpp images
void alphaBlendBlit(byte *dst, byte *src,
					const uint dstPitch, const uint srcPitch,
					const int posX, const int posY,
					const uint width, const uint height,
					const uint32 colorMod, const uint flipping) {
	bool rgbmod   = ((colorMod & kRGBModMask) != kRGBModMask);
	bool alphamod = ((colorMod & kAModMask)   != kAModMask);

	int xp = 0, yp = 0;

	int inStep = 4;
	int inoStep = srcPitch;
	if (flipping & FLIP_H) {
		inStep = -inStep;
		xp = width - 1;
	}

	if (flipping & FLIP_V) {
		inoStep = -inoStep;
		yp = height - 1;
	}

	byte *ino = src + yp * srcPitch + xp * 4;
	byte *outo = dst + posY * dstPitch + posX * 4;

	if (rgbmod) {
		if (alphamod) {
			doBlitAlphaBlendLogic<true, true>(ino, outo, width, height, dstPitch, inStep, inoStep, colorMod);
		} else {
			doBlitAlphaBlendLogic<true, false>(ino, outo, width, height, dstPitch, inStep, inoStep, colorMod);
		}
	} else {
		if (alphamod) {
			doBlitAlphaBlendLogic<false, true>(ino, outo, width, height, dstPitch, inStep, inoStep, colorMod);
		} else {
			doBlitAlphaBlendLogic<false, false>(ino, outo, width, height, dstPitch, inStep, inoStep, colorMod);
		}
	}
}

/**
 * Optimized version of doBlit to be used with subtractive blended blitting
 */
template<bool rgbmod>
static void doBlitSubtractiveBlendLogic(byte *ino, byte *outo,
										uint32 width, uint32 height,
										uint32 pitch, int32 inStep,
										int32 inoStep, uint32 color) {
	byte *in;
	byte *out;

	byte cr = rgbmod   ? ((color >> kRModShift) & 0xFF) : 255;
	byte cg = rgbmod   ? ((color >> kGModShift) & 0xFF) : 255;
	byte cb = rgbmod   ? ((color >> kBModShift) & 0xFF) : 255;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (uint32 j = 0; j < width; j++) {

			out[kAIndex] = 255;
			if (cb != 255) {
				out[kBIndex] = MAX(out[kBIndex] - ((in[kBIndex] * cb  * (out[kBIndex]) * in[kAIndex]) >> 24), 0);
			} else {
				out[kBIndex] = MAX(out[kBIndex] - (in[kBIndex] * (out[kBIndex]) * in[kAIndex] >> 16), 0);
			}

			if (cg != 255) {
				out[kGIndex] = MAX(out[kGIndex] - ((in[kGIndex] * cg  * (out[kGIndex]) * in[kAIndex]) >> 24), 0);
			} else {
				out[kGIndex] = MAX(out[kGIndex] - (in[kGIndex] * (out[kGIndex]) * in[kAIndex] >> 16), 0);
			}

			if (cr != 255) {
				out[kRIndex] = MAX(out[kRIndex] - ((in[kRIndex] * cr * (out[kRIndex]) * in[kAIndex]) >> 24), 0);
			} else {
				out[kRIndex] = MAX(out[kRIndex] - (in[kRIndex] * (out[kRIndex]) * in[kAIndex] >> 16), 0);
			}

			in += inStep;
			out += 4;
		}
		outo += pitch;
		ino += inoStep;
	}
}

// Only blits to and from 32bpp images
void subtractiveBlendBlit(byte *dst, byte *src,
						  const uint dstPitch, const uint srcPitch,
						  const int posX, const int posY,
						  const uint width, const uint height,
						  const uint32 colorMod, const uint flipping) {
	bool rgbmod   = ((colorMod & kRGBModMask) != kRGBModMask);

	int xp = 0, yp = 0;

	int inStep = 4;
	int inoStep = srcPitch;
	if (flipping & FLIP_H) {
		inStep = -inStep;
		xp = width - 1;
	}

	if (flipping & FLIP_V) {
		inoStep = -inoStep;
		yp = height - 1;
	}

	byte *ino = src + yp * srcPitch + xp * 4;
	byte *outo = dst + posY * dstPitch + posX * 4;

	if (rgbmod) {
		doBlitSubtractiveBlendLogic<true>(ino, outo, width, height, dstPitch, inStep, inoStep, colorMod);
	} else {
		doBlitSubtractiveBlendLogic<false>(ino, outo, width, height, dstPitch, inStep, inoStep, colorMod);
	}
}

/**
 * Optimized version of doBlit to be used with additive blended blitting
 */
template<bool rgbmod, bool alphamod>
static void doBlitAdditiveBlendLogic(byte *ino, byte *outo,
									 uint32 width, uint32 height, uint32 pitch,
									 int32 inStep, int32 inoStep, uint32 color) {

	byte *in;
	byte *out;

	byte ca = alphamod ? ((color >> kAModShift) & 0xFF) : 255;
	byte cr = rgbmod   ? ((color >> kRModShift) & 0xFF) : 255;
	byte cg = rgbmod   ? ((color >> kGModShift) & 0xFF) : 255;
	byte cb = rgbmod   ? ((color >> kBModShift) & 0xFF) : 255;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (uint32 j = 0; j < width; j++) {

			uint32 ina = in[kAIndex] * ca >> 8;

			if (ina != 0) {
				if (cb != 255) {
					out[kBIndex] = MIN<uint>(out[kBIndex] + ((in[kBIndex] * cb * ina) >> 16), 255u);
				} else {
					out[kBIndex] = MIN<uint>(out[kBIndex] + (in[kBIndex] * ina >> 8), 255u);
				}

				if (cg != 255) {
					out[kGIndex] = MIN<uint>(out[kGIndex] + ((in[kGIndex] * cg * ina) >> 16), 255u);
				} else {
					out[kGIndex] = MIN<uint>(out[kGIndex] + (in[kGIndex] * ina >> 8), 255u);
				}

				if (cr != 255) {
					out[kRIndex] = MIN<uint>(out[kRIndex] + ((in[kRIndex] * cr * ina) >> 16), 255u);
				} else {
					out[kRIndex] = MIN<uint>(out[kRIndex] + (in[kRIndex] * ina >> 8), 255u);
				}
			}

			in += inStep;
			out += 4;
		}

		outo += pitch;
		ino += inoStep;
	}
}

// Only blits to and from 32bpp images
void additiveBlendBlit(byte *dst, byte *src,
					   const uint dstPitch, const uint srcPitch,
					   const int posX, const int posY,
					   const uint width, const uint height,
					   const uint32 colorMod, const uint flipping) {
	bool rgbmod   = ((colorMod & kRGBModMask) != kRGBModMask);
	bool alphamod = ((colorMod & kAModMask)   != kAModMask);

	int xp = 0, yp = 0;

	int inStep = 4;
	int inoStep = srcPitch;
	if (flipping & FLIP_H) {
		inStep = -inStep;
		xp = width - 1;
	}

	if (flipping & FLIP_V) {
		inoStep = -inoStep;
		yp = height - 1;
	}

	byte *ino = src + yp * srcPitch + xp * 4;
	byte *outo = dst + posY * dstPitch + posX * 4;

	if (rgbmod) {
		if (alphamod) {
			doBlitAdditiveBlendLogic<true, true>(ino, outo, width, height, dstPitch, inStep, inoStep, colorMod);
		} else {
			doBlitAdditiveBlendLogic<true, false>(ino, outo, width, height, dstPitch, inStep, inoStep, colorMod);
		}
	} else {
		if (alphamod) {
			doBlitAdditiveBlendLogic<false, true>(ino, outo, width, height, dstPitch, inStep, inoStep, colorMod);
		} else {
			doBlitAdditiveBlendLogic<false, false>(ino, outo, width, height, dstPitch, inStep, inoStep, colorMod);
		}
	}
}

void opaqueBlendBlit(byte *dst, byte *src,
					 const uint dstPitch, const uint srcPitch,
					 const int posX, const int posY,
					 const uint width, const uint height,
					 const uint32 colorMod, const uint flipping) {
	int xp = 0, yp = 0;

	int inStep = 4;
	int inoStep = srcPitch;
	if (flipping & FLIP_H) {
		inStep = -inStep;
		xp = width - 1;
	}

	if (flipping & FLIP_V) {
		inoStep = -inoStep;
		yp = height - 1;
	}

	byte *ino = src + yp * srcPitch + xp * 4;
	byte *outo = dst + posY * dstPitch + posX * 4;
	
	byte *in;
	byte *out;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		memcpy(out, in, width * 4);
		for (uint32 j = 0; j < width; j++) {
			out[kAIndex] = 0xFF;
			out += 4;
		}
		outo += dstPitch;
		ino += inoStep;
	}
}

void binaryBlendBlit(byte *dst, byte *src,
					 const uint dstPitch, const uint srcPitch,
					 const int posX, const int posY,
					 const uint width, const uint height,
					 const uint32 colorMod, const uint flipping) {
	int xp = 0, yp = 0;

	int inStep = 4;
	int inoStep = srcPitch;
	if (flipping & FLIP_H) {
		inStep = -inStep;
		xp = width - 1;
	}

	if (flipping & FLIP_V) {
		inoStep = -inoStep;
		yp = height - 1;
	}

	byte *ino = src + yp * srcPitch + xp * 4;
	byte *outo = dst + posY * dstPitch + posX * 4;

	byte *in;
	byte *out;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (uint32 j = 0; j < width; j++) {
			uint32 pix = *(uint32 *)in;
			int a = in[kAIndex];

			if (a != 0) {   // Full opacity (Any value not exactly 0 is Opaque here)
				*(uint32 *)out = pix;
				out[kAIndex] = 0xFF;
			}
			out += 4;
			in += inStep;
		}
		outo += dstPitch;
		ino += inoStep;
	}
}

} // End of namespace Graphics
