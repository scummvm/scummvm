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

#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/pixelformat.h"

namespace Graphics {

namespace {

template<typename Color, int Size>
static inline uint32 READ_PIXEL(const byte *src) {
	if (Size == sizeof(Color)) {
		return *(const Color *)src;
	} else {
		uint32 color;
		uint8 *col = (uint8 *)&color;
#ifdef SCUMM_BIG_ENDIAN
		if (Size == 3)
			col++;
#endif
		memcpy(col, src, Size);
		return color;
	}
}

template<typename Color, int Size>
static inline void WRITE_PIXEL(byte *dst, const uint32 color) {
	if (Size == sizeof(Color)) {
		*(Color *)dst = color;
	} else {
		const uint8 *col = (const uint8 *)&color;
#ifdef SCUMM_BIG_ENDIAN
		if (Size == 3)
			col++;
#endif
		memcpy(dst, col, Size);
	}
}

template<typename SrcColor, int SrcSize, typename DstColor, int DstSize, bool hasKey, bool hasMask, bool hasMap>
static inline void alphaBlitLogic(byte *dst, const byte *src, const byte *mask, const uint w, const uint h,
						const PixelFormat &srcFmt, const PixelFormat &dstFmt, const uint32 *map,
						const int srcDelta, const int dstDelta, const int maskDelta,
						const int srcInc, const int dstInc, const int maskInc,
						const uint32 key, const byte flip, const byte aMod) {
	const uint32 alphaMask = srcFmt.ARGBToColor(255, 0, 0, 0);
	const bool convert = hasMap ? false : ((SrcSize != DstSize) ? true : srcFmt == dstFmt);

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			const uint32 srcColor = hasMap ? map[*src]
				: READ_PIXEL<SrcColor, SrcSize>(src);

			const bool isOpaque = hasMask ? (*mask == 0xff)
				: (hasKey ? (READ_PIXEL<SrcColor, SrcSize>(src) != key)
				: !alphaMask || ((srcColor & alphaMask) == alphaMask));
			const bool isTransparent = hasMask ? (*mask == 0x00)
				: (hasKey ? (READ_PIXEL<SrcColor, SrcSize>(src) == key)
				: alphaMask && ((srcColor & alphaMask) == 0));

			if (isOpaque && aMod == 0xff) {
				if (convert) {
					byte sR, sG, sB;
					srcFmt.colorToRGB(srcColor, sR, sG, sB);
					WRITE_PIXEL<DstColor, DstSize>(dst, dstFmt.RGBToColor(sR, sG, sB));
				} else {
					WRITE_PIXEL<DstColor, DstSize>(dst, srcColor);
				}
			} else if (!isTransparent) {
				// TODO: Optimise for matching formats?
				const uint32 dstColor = READ_PIXEL<DstColor, DstSize>(dst);

				byte sA, sR, sG, sB;
				srcFmt.colorToARGB(srcColor, sA, sR, sG, sB);

				byte dR, dG, dB;
				dstFmt.colorToRGB(dstColor, dR, dG, dB);

				if (hasKey)
					sA = aMod;
				else if (hasMask)
					sA = ((*mask * aMod) >> 8);
				else
					sA = ((sA * aMod) >> 8);

				dR = (dR * (255-sA) + sR * sA) >> 8;
				dG = (dG * (255-sA) + sG * sA) >> 8;
				dB = (dB * (255-sA) + sB * sA) >> 8;

				const uint32 outColor = dstFmt.RGBToColor(dR, dG, dB);
				WRITE_PIXEL<DstColor, DstSize>(dst, outColor);
			}

			src += srcInc;
			dst += dstInc;
			if (hasMask)
				mask += maskInc;
		}

		src += srcDelta;
		dst += dstDelta;
		if (hasMask)
			mask += maskDelta;
	}
}

template<bool hasKey, bool hasMask>
static inline bool alphaBlitHelper(byte *dst, const byte *src, const byte *mask, const uint w, const uint h,
                         const PixelFormat &srcFmt, const PixelFormat &dstFmt,
                         const uint srcPitch, const uint dstPitch, const uint maskPitch,
                         const uint32 key, const byte flip, const byte aMod) {
	const bool hasMap = false;
	const bool flipx = flip & FLIP_H;
	const bool flipy = flip & FLIP_V;

	// Faster, but larger, to provide optimized handling for each case.
	      int dstDelta = (dstPitch - w * dstFmt.bytesPerPixel);
	const int srcDelta = (srcPitch - w * srcFmt.bytesPerPixel);
	const int maskDelta = hasMask ? (maskPitch - w) : 0;

	const int dstInc = flipx ? -dstFmt.bytesPerPixel : dstFmt.bytesPerPixel;
	const int srcInc = srcFmt.bytesPerPixel;
	const int maskInc = 1;

	if (flipx)
		dst += (w - 1) * dstFmt.bytesPerPixel;

	if (flipy)
		dst += (h - 1) * dstPitch;

	if (flipy && flipx)
		dstDelta = -dstDelta;
	else if (flipy)
		dstDelta = -((dstPitch * 2) - dstDelta);
	else if (flipx)
		dstDelta =   (dstPitch * 2) - dstDelta;

	if (aMod == 0)
		return true;

	// TODO: optimized cases for dstDelta of 0
	if (dstFmt.bytesPerPixel == 2) {
		if (srcFmt.bytesPerPixel == 2) {
			alphaBlitLogic<uint16, 2, uint16, 2, hasKey, hasMask, hasMap>(dst, src, mask, w, h, srcFmt, dstFmt, nullptr, srcDelta, dstDelta, maskDelta, srcInc, dstInc, maskInc, key, flip, aMod);
		} else if (srcFmt.bytesPerPixel == 3) {
			alphaBlitLogic<uint8,  3, uint16, 2, hasKey, hasMask, hasMap>(dst, src, mask, w, h, srcFmt, dstFmt, nullptr, srcDelta, dstDelta, maskDelta, srcInc, dstInc, maskInc, key, flip, aMod);
		} else {
			alphaBlitLogic<uint32, 4, uint16, 2, hasKey, hasMask, hasMap>(dst, src, mask, w, h, srcFmt, dstFmt, nullptr, srcDelta, dstDelta, maskDelta, srcInc, dstInc, maskInc, key, flip, aMod);
		}
	} else if (dstFmt.bytesPerPixel == 4) {
		if (srcFmt.bytesPerPixel == 2) {
			alphaBlitLogic<uint16, 2, uint32, 4, hasKey, hasMask, hasMap>(dst, src, mask, w, h, srcFmt, dstFmt, nullptr, srcDelta, dstDelta, maskDelta, srcInc, dstInc, maskInc, key, flip, aMod);
		} else if (srcFmt.bytesPerPixel == 3) {
			alphaBlitLogic<uint8,  3, uint32, 4, hasKey, hasMask, hasMap>(dst, src, mask, w, h, srcFmt, dstFmt, nullptr, srcDelta, dstDelta, maskDelta, srcInc, dstInc, maskInc, key, flip, aMod);
		} else {
			alphaBlitLogic<uint32, 4, uint32, 4, hasKey, hasMask, hasMap>(dst, src, mask, w, h, srcFmt, dstFmt, nullptr, srcDelta, dstDelta, maskDelta, srcInc, dstInc, maskInc, key, flip, aMod);
		}
	} else {
		return false;
	}
	return true;
}

template<bool hasKey, bool hasMask>
static inline bool alphaBlitMapHelper(byte *dst, const byte *src, const byte *mask, const uint w, const uint h,
                         const PixelFormat &dstFmt, const uint32 *map,
                         const uint srcPitch, const uint dstPitch, const uint maskPitch,
                         const uint32 key, const byte flip, const byte aMod) {
	const Graphics::PixelFormat &srcFmt = dstFmt;
	const bool hasMap = true;
	const bool flipx = flip & FLIP_H;
	const bool flipy = flip & FLIP_V;

	// Faster, but larger, to provide optimized handling for each case.
	      int dstDelta = (dstPitch - w * dstFmt.bytesPerPixel);
	const int srcDelta = (srcPitch - w);
	const int maskDelta = hasMask ? (maskPitch - w) : 0;

	const int dstInc = flipx ? -dstFmt.bytesPerPixel : dstFmt.bytesPerPixel;
	const int srcInc = 1;
	const int maskInc = 1;

	if (flipx)
		dst += (w - 1) * dstFmt.bytesPerPixel;

	if (flipy)
		dst += (h - 1) * dstPitch;

	if (flipy && flipx)
		dstDelta = -dstDelta;
	else if (flipy)
		dstDelta = -((dstPitch * 2) - dstDelta);
	else if (flipx)
		dstDelta =   (dstPitch * 2) - dstDelta;

	// TODO: optimized cases for dstDelta of 0
	if (dstFmt.bytesPerPixel == 2) {
		alphaBlitLogic<uint8,  1, uint16, 2, hasKey, hasMask, hasMap>(dst, src, mask, w, h, srcFmt, dstFmt, map, srcDelta, dstDelta, maskDelta, srcInc, dstInc, maskInc, key, flip, aMod);
	} else if (dstFmt.bytesPerPixel == 4) {
		alphaBlitLogic<uint8,  1, uint32, 4, hasKey, hasMask, hasMap>(dst, src, mask, w, h, srcFmt, dstFmt, map, srcDelta, dstDelta, maskDelta, srcInc, dstInc, maskInc, key, flip, aMod);
	} else {
		return false;
	}
	return true;
}

} // End of anonymous namespace

bool alphaBlit(byte *dst, const byte *src,
				const uint dstPitch, const uint srcPitch,
				const uint w, const uint h,
				const Graphics::PixelFormat &dstFmt, const Graphics::PixelFormat &srcFmt,
				const byte flip, const byte aMod) {
	// Error out if conversion is impossible
	if ((srcFmt.bytesPerPixel == 1) || (dstFmt.bytesPerPixel == 1)
			|| (!srcFmt.bytesPerPixel) || (!dstFmt.bytesPerPixel))
		return false;

	return alphaBlitHelper<false, false>(dst, src, nullptr, w, h, srcFmt, dstFmt, srcPitch, dstPitch, 0, 0, flip, aMod);
}

bool alphaKeyBlit(byte *dst, const byte *src,
				const uint dstPitch, const uint srcPitch,
				const uint w, const uint h,
				const Graphics::PixelFormat &dstFmt, const Graphics::PixelFormat &srcFmt,
				const uint32 key, const byte flip, const byte aMod) {
	// Error out if conversion is impossible
	if ((srcFmt.bytesPerPixel == 1) || (dstFmt.bytesPerPixel == 1)
			|| (!srcFmt.bytesPerPixel) || (!dstFmt.bytesPerPixel))
		return false;

	return alphaBlitHelper<true, false>(dst, src, nullptr, w, h, srcFmt, dstFmt, srcPitch, dstPitch, 0, key, flip, aMod);
}

bool alphaMaskBlit(byte *dst, const byte *src, const byte *mask,
				const uint dstPitch, const uint srcPitch, const uint maskPitch,
				const uint w, const uint h,
				const Graphics::PixelFormat &dstFmt, const Graphics::PixelFormat &srcFmt,
				const byte flip, const byte aMod) {
	// Error out if conversion is impossible
	if ((srcFmt.bytesPerPixel == 1) || (dstFmt.bytesPerPixel == 1)
			|| (!srcFmt.bytesPerPixel) || (!dstFmt.bytesPerPixel))
		return false;

	return alphaBlitHelper<false, true>(dst, src, mask, w, h, srcFmt, dstFmt, srcPitch, dstPitch, maskPitch, 0, flip, aMod);
}

bool alphaBlitMap(byte *dst, const byte *src,
			        const uint dstPitch, const uint srcPitch,
			        const uint w, const uint h,
			        const Graphics::PixelFormat &dstFmt, const uint32 *map,
			        const byte flip, const byte aMod) {
	// Error out if conversion is impossible
	if ((dstFmt.bytesPerPixel == 1) || (!dstFmt.bytesPerPixel))
		return false;

	return alphaBlitMapHelper<false, false>(dst, src, nullptr, w, h, dstFmt, map, srcPitch, dstPitch, 0, 0, flip, aMod);
}

bool alphaKeyBlitMap(byte *dst, const byte *src,
			        const uint dstPitch, const uint srcPitch,
			        const uint w, const uint h,
			        const Graphics::PixelFormat &dstFmt, const uint32 *map,
			        const uint32 key, const byte flip, const byte aMod) {
	// Error out if conversion is impossible
	if ((dstFmt.bytesPerPixel == 1) || (!dstFmt.bytesPerPixel))
		return false;

	return alphaBlitMapHelper<true, false>(dst, src, nullptr, w, h, dstFmt, map, srcPitch, dstPitch, 0, key, flip, aMod);
}

bool alphaMaskBlitMap(byte *dst, const byte *src, const byte *mask,
			        const uint dstPitch, const uint srcPitch, const uint maskPitch,
			        const uint w, const uint h,
			        const Graphics::PixelFormat &dstFmt, const uint32 *map,
			        const byte flip, const byte aMod) {
	// Error out if conversion is impossible
	if ((dstFmt.bytesPerPixel == 1) || (!dstFmt.bytesPerPixel))
		return false;

	return alphaBlitMapHelper<false, true>(dst, src, mask, w, h, dstFmt, map, srcPitch, dstPitch, maskPitch, 0, flip, aMod);
}

namespace {

template<typename Size, bool overwriteAlpha>
inline bool applyColorKeyLogic(byte *dst, const byte *src, const uint w, const uint h,
							   const uint srcDelta, const uint dstDelta,
							   const Graphics::PixelFormat &format,
							   const uint8 rKey, const uint8 gKey, const uint8 bKey,
							   const uint8 rNew, const uint8 gNew, const uint8 bNew) {

	const uint32 keyPix    = format.ARGBToColor(0,   rKey, gKey, bKey);
	const uint32 newPix    = format.ARGBToColor(0,   rNew, gNew, bNew);
	const uint32 rgbMask   = format.ARGBToColor(0,   255,  255,  255);
	const uint32 alphaMask = format.ARGBToColor(255, 0,    0,    0);
	bool applied = false;

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			uint32 pix = *(const Size *)src;

			if ((pix & rgbMask) == keyPix) {
				*(Size *)dst = newPix;
				applied = true;
			} else if (overwriteAlpha) {
				*(Size *)dst = pix | alphaMask;
			}

			src += sizeof(Size);
			dst += sizeof(Size);
		}

		src += srcDelta;
		dst += dstDelta;
	}

	return applied;
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
			return applyColorKeyLogic<uint8, true>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
		} else if (format.bytesPerPixel == 2) {
			return applyColorKeyLogic<uint16, true>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
		} else if (format.bytesPerPixel == 4) {
			return applyColorKeyLogic<uint32, true>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
		} else {
			return false;
		}
	} else {
		if (format.bytesPerPixel == 1) {
			return applyColorKeyLogic<uint8, false>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
		} else if (format.bytesPerPixel == 2) {
			return applyColorKeyLogic<uint16, false>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
		} else if (format.bytesPerPixel == 4) {
			return applyColorKeyLogic<uint32, false>(dst, src, w, h, srcDelta, dstDelta, format, rKey, gKey, bKey, rNew, gNew, bNew);
		} else {
			return false;
		}
	}
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
	const uint _dstPitch, const uint _srcPitch,
	const int posX, const int posY,
	const uint _width, const uint _height,
	const int _scaleX, const int _scaleY,
	const int scaleXsrcOff, const int scaleYsrcOff,
	const uint32 colorMod, const uint _flipping) :
		xp(0), yp(0), dstPitch(_dstPitch),
		width(_width), height(_height), color(colorMod),
		scaleX(_scaleX), scaleY(_scaleY), flipping(_flipping),
		scaleXoff(scaleXsrcOff), scaleYoff(scaleYsrcOff) {
	bool doScale = scaleX != SCALE_THRESHOLD || scaleY != SCALE_THRESHOLD;
	
	rgbmod   = ((colorMod & kRGBModMask) != kRGBModMask);
	alphamod = ((colorMod & kAModMask)   != kAModMask);
	inStep = 4;
	inoStep = _srcPitch;
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

	ino = src + yp * _srcPitch + xp * 4;
	outo = dst + posY * _dstPitch + posX * 4;
}

// Initialize these to nullptr at the start
BlendBlit::BlitFunc BlendBlit::blitFunc = nullptr;
BlendBlit::FillFunc BlendBlit::fillFunc = nullptr;

// Only blits to and from 32bpp images
// So this function is just here to jump to whatever function is in
// BlendBlit::blitFunc. This way, we can detect at runtime whether or not
// the cpu has certain SIMD feature enabled or not.
void BlendBlit::blit(byte *dst, const byte *src,
					 const uint dstPitch, const uint srcPitch,
					 const int posX, const int posY,
					 const uint width, const uint height,
					 const int scaleX, const int scaleY,
					 const int scaleXsrcOff, const int scaleYsrcOff,
					 const uint32 colorMod, const uint flipping,
					 const TSpriteBlendMode blendMode,
					 const AlphaType alphaType) {
	if (width == 0 || height == 0) return;

	// If no function has been selected yet, detect and select
	if (!blitFunc) {
		// Get the correct blit function
		blitFunc = blitGeneric;
#ifdef SCUMMVM_NEON
		if (g_system->hasFeature(OSystem::kFeatureCpuNEON)) blitFunc = blitNEON;
#endif
#ifdef SCUMMVM_SSE2
		if (g_system->hasFeature(OSystem::kFeatureCpuSSE2)) blitFunc = blitSSE2;
#endif
#ifdef SCUMMVM_AVX2
		if (g_system->hasFeature(OSystem::kFeatureCpuAVX2)) blitFunc = blitAVX2;
#endif
	}
	
	Args args(dst, src, dstPitch, srcPitch, posX, posY, width, height, scaleX, scaleY, scaleXsrcOff, scaleYsrcOff, colorMod, flipping);
	blitFunc(args, blendMode, alphaType);
}

// Only fills 32bpp images
// So this function is just here to jump to whatever function is in
// BlendBlit::fillFunc. This way, we can detect at runtime whether or not
// the cpu has certain SIMD feature enabled or not.
void BlendBlit::fill(byte *dst, const uint dstPitch,
					 const uint width, const uint height,
					 const uint32 colorMod,
					 const TSpriteBlendMode blendMode) {
	if (width == 0 || height == 0) return;

	// If no function has been selected yet, detect and select
	if (!fillFunc) {
		// Get the correct blit function
		// TODO: Add SIMD variants
		fillFunc = fillGeneric;
	}

	Args args(dst, nullptr, dstPitch, 0, 0, 0, width, height, 0, 0, 0, 0, colorMod, 0);
	fillFunc(args, blendMode);
}

} // End of namespace Graphics
