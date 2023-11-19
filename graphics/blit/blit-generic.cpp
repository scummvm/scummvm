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

#include "graphics/blit/blit-alpha.h"
#include "graphics/pixelformat.h"

namespace Graphics {

class BlendBlitImpl_Default : public BlendBlitImpl_Base {
	friend class BlendBlit;
public:

template<template <bool DOSCALE, bool RGBMOD, bool ALPHAMOD> class PixelFunc, bool doscale, bool rgbmod, bool alphamod, bool coloradd1, bool loaddst>
static inline void blitInnerLoop(BlendBlit::Args &args) {
	const byte *in;
	byte *out;

	const byte rawcr = (args.color >> BlendBlit::kRModShift) & 0xFF;
	const byte rawcg = (args.color >> BlendBlit::kGModShift) & 0xFF;
	const byte rawcb = (args.color >> BlendBlit::kBModShift) & 0xFF;
	const byte ca = alphamod ? ((args.color >> BlendBlit::kAModShift) & 0xFF) : 255;
	const uint32 cr = coloradd1 ? (rgbmod   ? (rawcr == 255 ? 256 : rawcr) : 256) : (rgbmod   ? rawcr : 255);
	const uint32 cg = coloradd1 ? (rgbmod   ? (rawcg == 255 ? 256 : rawcg) : 256) : (rgbmod   ? rawcg : 255);
	const uint32 cb = coloradd1 ? (rgbmod   ? (rawcb == 255 ? 256 : rawcb) : 256) : (rgbmod   ? rawcb : 255);

	int scaleXCtr, scaleYCtr = args.scaleYoff;
	const byte *inBase;

	for (uint32 i = 0; i < args.height; i++) {
		if (doscale) {
			inBase = args.ino + scaleYCtr / BlendBlit::SCALE_THRESHOLD * args.inoStep;
			scaleXCtr = args.scaleXoff;
		} else {
			in = args.ino;
		}
		out = args.outo;

		for (uint32 j = 0; j < args.width; j++) {
			if (doscale) {
				in = inBase + scaleXCtr / BlendBlit::SCALE_THRESHOLD * args.inStep;
			}

			PixelFunc<doscale, rgbmod, alphamod>::normal(in, out, ca, cr, cg, cb);

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
static void doBlitOpaqueBlendLogicGeneric(BlendBlit::Args &args) {
	const byte *in;
	byte *out;

	int scaleXCtr, scaleYCtr = args.scaleYoff;
	const byte *inBase;

	for (uint32 i = 0; i < args.height; i++) {
		if (doscale) {
			inBase = args.ino + (scaleYCtr + 1) / BlendBlit::SCALE_THRESHOLD * args.inoStep;
			scaleXCtr = args.scaleXoff;
		} else {
			in = args.ino;
		}
		out = args.outo;

		if (doscale) {
			for (uint32 j = 0; j < args.width; j++) {
				in = inBase + scaleXCtr / BlendBlit::SCALE_THRESHOLD * args.inStep;
				*(uint32 *)out = *(const uint32 *)in | BlendBlit::kAModMask;
				scaleXCtr += args.scaleX;
				out += 4;
			}
		} else {
			for (uint32 j = 0; j < args.width; j++) {
				*(uint32 *)out = *(const uint32 *)in | BlendBlit::kAModMask;
				in += args.inStep;
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
static void doBlitBinaryBlendLogicGeneric(BlendBlit::Args &args) {
	const byte *in;
	byte *out;

	int scaleXCtr, scaleYCtr = args.scaleYoff;
	const byte *inBase;

	for (uint32 i = 0; i < args.height; i++) {
		if (doscale) {
			inBase = args.ino + scaleYCtr / BlendBlit::SCALE_THRESHOLD * args.inoStep;
			scaleXCtr = args.scaleXoff;
		} else {
			in = args.ino;
		}
		out = args.outo;
		for (uint32 j = 0; j < args.width; j++) {
			if (doscale) {
				in = inBase + scaleXCtr / BlendBlit::SCALE_THRESHOLD * args.inStep;
			}

			uint32 pix = *(const uint32 *)in, pixout = *(const uint32 *)out;
			uint32 mask = (pix & BlendBlit::kAModMask) ? 0xffffffff : 0;
			pixout &= ~mask;
			pix = (pix | BlendBlit::kAModMask) & mask;
			*(uint32 *)out = pixout | pix;
			
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

}; // end of class BlendBlitImpl_Default

template<>
inline void BlendBlitImpl_Default::blitInnerLoop<BlendBlitImpl_Default::OpaqueBlend, true, false, false, false, true>(BlendBlit::Args &args) {
	doBlitOpaqueBlendLogicGeneric<true>(args);
}

template<>
inline void BlendBlitImpl_Default::blitInnerLoop<BlendBlitImpl_Default::OpaqueBlend, false, false, false, false, true>(BlendBlit::Args &args) {
	doBlitOpaqueBlendLogicGeneric<false>(args);
}

template<>
inline void BlendBlitImpl_Default::blitInnerLoop<BlendBlitImpl_Default::BinaryBlend, true, false, false, false, true>(BlendBlit::Args &args) {
	doBlitBinaryBlendLogicGeneric<true>(args);
}

template<>
inline void BlendBlitImpl_Default::blitInnerLoop<BlendBlitImpl_Default::BinaryBlend, false, false, false, false, true>(BlendBlit::Args &args) {
	doBlitBinaryBlendLogicGeneric<false>(args);
}

void BlendBlit::blitGeneric(Args &args, const TSpriteBlendMode &blendMode, const AlphaType &alphaType) {
	blitT<BlendBlitImpl_Default>(args, blendMode, alphaType);
}

} // End of namespace Graphics
