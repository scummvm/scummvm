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
#include "common/system.h"

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


BlendBlit::Args::Args(byte *dst, const byte *src,
	const uint dstPitch, const uint srcPitch,
	const int posX, const int posY,
	const uint width, const uint height,
	const int scaleX, const int scaleY,
	const uint32 colorMod, const uint flipping) :
		xp(0), yp(0), dstPitch(dstPitch),
		width(width), height(height), color(colorMod),
		scaleX(scaleX), scaleY(scaleY), flipping(flipping) {
	bool doScale = scaleX != SCALE_THRESHOLD || scaleY != SCALE_THRESHOLD;
	
	rgbmod   = ((colorMod & kRGBModMask) != kRGBModMask);
	alphamod = ((colorMod & kAModMask)   != kAModMask);
	inStep = 4;
	inoStep = srcPitch;
	if (flipping & FLIP_H) {
		inStep = -inStep;
		xp = width - 1;
		if (doScale) xp = xp * scaleX / SCALE_THRESHOLD;
	}

	if (flipping & FLIP_V) {
		inoStep = -inoStep;
		yp = height - 1;
		if (doScale) yp = yp * scaleY / SCALE_THRESHOLD;
	}

	ino = src + yp * srcPitch + xp * 4;
	outo = dst + posY * dstPitch + posX * 4;
}

/**
 * Optimized version of doBlit to be used with multiply blended blitting
 */
template<bool doscale>
void BlendBlit::doBlitMultiplyBlendLogicGeneric(Args &args) {
	const byte *in;
	byte *out;

	int scaleXCtr, scaleYCtr = 0;
	const byte *inBase;

	byte ca = args.alphamod ? ((args.color >> kAModShift) & 0xFF) : 255;
	byte cr = args.rgbmod   ? ((args.color >> kRModShift) & 0xFF) : 255;
	byte cg = args.rgbmod   ? ((args.color >> kGModShift) & 0xFF) : 255;
	byte cb = args.rgbmod   ? ((args.color >> kBModShift) & 0xFF) : 255;

	for (uint32 i = 0; i < args.height; i++) {
		if (doscale) {
			inBase = args.ino + scaleYCtr / SCALE_THRESHOLD * args.inoStep;
			scaleXCtr = 0;
		} else {
			in = args.ino;
		}
		out = args.outo;
		for (uint32 j = 0; j < args.width; j++) {
			if (doscale) {
				in = inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep;
			}

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

			if (doscale)
				scaleXCtr += args.scaleX;
			else
				in += args.inStep;
			out += 4;
		}
		if (doscale)
			scaleYCtr += args.scaleY;
		else
			args.ino += args.inoStep;
		args.outo += args.dstPitch;
	}

}

template<bool doscale>
void BlendBlit::doBlitAlphaBlendLogicGeneric(Args &args) {
	const byte *in;
	byte *out;

	int scaleXCtr, scaleYCtr = 0;
	const byte *inBase;

	byte ca = args.alphamod ? ((args.color >> kAModShift) & 0xFF) : 255;
	byte cr = args.rgbmod   ? ((args.color >> kRModShift) & 0xFF) : 255;
	byte cg = args.rgbmod   ? ((args.color >> kGModShift) & 0xFF) : 255;
	byte cb = args.rgbmod   ? ((args.color >> kBModShift) & 0xFF) : 255;

	for (uint32 i = 0; i < args.height; i++) {
		if (doscale) {
			inBase = args.ino + scaleYCtr / SCALE_THRESHOLD * args.inoStep;
			scaleXCtr = 0;
		} else {
			in = args.ino;
		}
		out = args.outo;
		for (uint32 j = 0; j < args.width; j++) {
			if (doscale) {
				in = inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep;
			}

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

			if (doscale)
				scaleXCtr += args.scaleX;
			else
				in += args.inStep;
			out += 4;
		}

		if (doscale)
			scaleYCtr += args.scaleY;
		else
			args.ino += args.inoStep;
		args.outo += args.dstPitch;
	}
}

/**
 * Optimized version of doBlit to be used with subtractive blended blitting
 */
template<bool doscale>
void BlendBlit::doBlitSubtractiveBlendLogicGeneric(Args &args) {
	const byte *in;
	byte *out;

	int scaleXCtr, scaleYCtr = 0;
	const byte *inBase;

	byte cr = args.rgbmod   ? ((args.color >> kRModShift) & 0xFF) : 255;
	byte cg = args.rgbmod   ? ((args.color >> kGModShift) & 0xFF) : 255;
	byte cb = args.rgbmod   ? ((args.color >> kBModShift) & 0xFF) : 255;

	for (uint32 i = 0; i < args.height; i++) {
		if (doscale) {
			inBase = args.ino + scaleYCtr / SCALE_THRESHOLD * args.inoStep;
			scaleXCtr = 0;
		} else {
			in = args.ino;
		}
		out = args.outo;
		for (uint32 j = 0; j < args.width; j++) {
			if (doscale) {
				in = inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep;
			}

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

			if (doscale)
				scaleXCtr += args.scaleX;
			else
				in += args.inStep;
			out += 4;
		}
		if (doscale)
			scaleYCtr += args.scaleY;
		else
			args.ino += args.inoStep;
		args.outo += args.dstPitch;
	}
}

/**
 * Optimized version of doBlit to be used with additive blended blitting
 */
template<bool doscale>
void BlendBlit::doBlitAdditiveBlendLogicGeneric(Args &args) {
	const byte *in;
	byte *out;

	int scaleXCtr, scaleYCtr = 0;
	const byte *inBase;

	byte ca = args.alphamod ? ((args.color >> kAModShift) & 0xFF) : 255;
	byte cr = args.rgbmod   ? ((args.color >> kRModShift) & 0xFF) : 255;
	byte cg = args.rgbmod   ? ((args.color >> kGModShift) & 0xFF) : 255;
	byte cb = args.rgbmod   ? ((args.color >> kBModShift) & 0xFF) : 255;

	for (uint32 i = 0; i < args.height; i++) {
		if (doscale) {
			inBase = args.ino + scaleYCtr / SCALE_THRESHOLD * args.inoStep;
			scaleXCtr = 0;
		} else {
			in = args.ino;
		}
		out = args.outo;
		for (uint32 j = 0; j < args.width; j++) {
			if (doscale) {
				in = inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep;
			}

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

			if (doscale)
				scaleXCtr += args.scaleX;
			else
				in += args.inStep;
			out += 4;
		}

		if (doscale)
			scaleYCtr += args.scaleY;
		else
			args.ino += args.inoStep;
		args.outo += args.dstPitch;
	}
}

template<bool doscale>
void BlendBlit::doBlitOpaqueBlendLogicGeneric(Args &args) {
	const byte *in;
	byte *out;

	int scaleXCtr, scaleYCtr = 0;
	const byte *inBase;

	for (uint32 i = 0; i < args.height; i++) {
		if (doscale) {
			inBase = args.ino + (scaleYCtr + 1) / SCALE_THRESHOLD * args.inoStep;
			scaleXCtr = 0;
		} else {
			in = args.ino;
		}
		out = args.outo;

		if (doscale) {
			for (uint32 j = 0; j < args.width; j++) {
				in = inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep;

				memcpy(out, in, 4);
				out[kAIndex] = 0xFF;

				scaleXCtr += args.scaleX;
				out += 4;
			}
		} else if (args.flipping & FLIP_H) {
			for (uint32 j = 0; j < args.width; j++) {
				memcpy(out, in, 4);
				out[kAIndex] = 0xFF;
				out += 4;
				in += args.inStep;
			}
		} else {
			memcpy(out, in, args.width * 4);
			for (uint32 j = 0; j < args.width; j++) {
				out[kAIndex] = 0xFF;
				out += 4;
			}
		}

		if (doscale)
			scaleYCtr += args.scaleY;
		else
			args.ino += args.inoStep;
		args.outo += args.dstPitch;
	}
}

template<bool doscale>
void BlendBlit::doBlitBinaryBlendLogicGeneric(Args &args) {
	const byte *in;
	byte *out;

	int scaleXCtr, scaleYCtr = 0;
	const byte *inBase;

	for (uint32 i = 0; i < args.height; i++) {
		if (doscale) {
			inBase = args.ino + scaleYCtr / SCALE_THRESHOLD * args.inoStep;
			scaleXCtr = 0;
		} else {
			in = args.ino;
		}
		out = args.outo;
		for (uint32 j = 0; j < args.width; j++) {
			if (doscale) {
				in = inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep;
			}
			uint32 pix = *(const uint32 *)in;
			int a = in[kAIndex];

			if (a != 0) {   // Full opacity (Any value not exactly 0 is Opaque here)
				*(uint32 *)out = pix;
				out[kAIndex] = 0xFF;
			}
			if (doscale)
				scaleXCtr += args.scaleX;
			else
				in += args.inStep;
			out += 4;
		}
		if (doscale)
			scaleYCtr += args.scaleY;
		else
			args.ino += args.inoStep;
		args.outo += args.dstPitch;
	}
}

// Initialize this to nullptr at the start
BlendBlit::BlitFunc BlendBlit::blitFunc = nullptr;

// Only blits to and from 32bpp images
void BlendBlit::blit(byte *dst, const byte *src,
					 const uint dstPitch, const uint srcPitch,
					 const int posX, const int posY,
					 const uint width, const uint height,
					 const int scaleX, const int scaleY,
					 const uint32 colorMod, const uint flipping,
					 const TSpriteBlendMode blendMode,
					 const AlphaType alphaType) {
	if (width == 0 || height == 0) return;
	if (!blitFunc) {
		// Get the correct blit function
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
		if (g_system->hasFeature(OSystem::kFeatureNEON)) blitFunc = blitNEON;
		else blitFunc = blitGeneric;
#else
		blitFunc = blitGeneric;
#endif
	}
	
	Args args(dst, src, dstPitch, srcPitch, posX, posY, width, height, scaleX, scaleY, colorMod, flipping);
	blitFunc(args, blendMode, alphaType);
}

#define BLIT_FUNC(ext) \
	void BlendBlit::blit##ext(Args &args, const TSpriteBlendMode &blendMode, const AlphaType &alphaType) { \
		if (args.scaleX == SCALE_THRESHOLD && args.scaleY == SCALE_THRESHOLD) { \
			if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_OPAQUE) { \
				doBlitOpaqueBlendLogic##ext<false>(args); \
			} else if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_BINARY) { \
				doBlitBinaryBlendLogic##ext<false>(args); \
			} else { \
				if (blendMode == BLEND_ADDITIVE) { \
					doBlitAdditiveBlendLogic##ext<false>(args); \
				} else if (blendMode == BLEND_SUBTRACTIVE) { \
					doBlitSubtractiveBlendLogic##ext<false>(args); \
				} else if (blendMode == BLEND_MULTIPLY) { \
					doBlitMultiplyBlendLogic##ext<false>(args); \
				} else { \
					assert(blendMode == BLEND_NORMAL); \
					doBlitAlphaBlendLogic##ext<false>(args); \
				} \
			} \
		} else { \
			if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_OPAQUE) { \
				doBlitOpaqueBlendLogic##ext<true>(args); \
			} else if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_BINARY) { \
				doBlitBinaryBlendLogic##ext<true>(args); \
			} else { \
				if (blendMode == BLEND_ADDITIVE) { \
					doBlitAdditiveBlendLogic##ext<true>(args); \
				} else if (blendMode == BLEND_SUBTRACTIVE) { \
					doBlitSubtractiveBlendLogic##ext<true>(args); \
				} else if (blendMode == BLEND_MULTIPLY) { \
					doBlitMultiplyBlendLogic##ext<true>(args); \
				} else { \
					assert(blendMode == BLEND_NORMAL); \
					doBlitAlphaBlendLogic##ext<true>(args); \
				} \
			} \
		} \
	}
BLIT_FUNC(Generic)
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
BLIT_FUNC(NEON)
#endif

} // End of namespace Graphics
