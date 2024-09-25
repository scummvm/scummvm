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

template<template <bool RGBMOD, bool ALPHAMOD> class PixelFunc, bool doscale, bool rgbmod, bool alphamod>
static inline void blitInnerLoop(BlendBlit::Args &args) {
	const byte *in;
	byte *out;

	const PixelFunc<rgbmod, alphamod> pixelFunc(args.color);

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

			pixelFunc.normal(in, out);

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

void BlendBlit::blitGeneric(Args &args, const TSpriteBlendMode &blendMode, const AlphaType &alphaType) {
	blitT<BlendBlitImpl_Default>(args, blendMode, alphaType);
}

} // End of namespace Graphics
