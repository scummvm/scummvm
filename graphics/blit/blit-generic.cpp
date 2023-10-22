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

class BlendBlitImpl_Default {
public:
/**
 * Optimized version of doBlit to be used with multiply blended blitting
 */
template<bool doscale, bool rgbmod, bool alphamod>
static void doBlitMultiplyBlendLogicGeneric(BlendBlit::Args &args) {
	const byte *in;
	byte *out;

	int scaleXCtr, scaleYCtr = args.scaleYoff;
	const byte *inBase;

	const byte rawcr = (args.color >> BlendBlit::kRModShift) & 0xFF;
	const byte rawcg = (args.color >> BlendBlit::kGModShift) & 0xFF;
	const byte rawcb = (args.color >> BlendBlit::kBModShift) & 0xFF;
	const byte ca = alphamod ? ((args.color >> BlendBlit::kAModShift) & 0xFF) : 255;
	const uint32 cr = rgbmod   ? (rawcr == 255 ? 256 : rawcr) : 256;
	const uint32 cg = rgbmod   ? (rawcg == 255 ? 256 : rawcg) : 256;
	const uint32 cb = rgbmod   ? (rawcb == 255 ? 256 : rawcb) : 256;

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

			uint32 ina = in[BlendBlit::kAIndex] * ca >> 8;

			if (ina != 0) {
				out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] * ((in[BlendBlit::kBIndex] * cb * ina) >> 16) >> 8;
				out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] * ((in[BlendBlit::kGIndex] * cg * ina) >> 16) >> 8;
				out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] * ((in[BlendBlit::kRIndex] * cr * ina) >> 16) >> 8;
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

template<bool doscale, bool rgbmod, bool alphamod>
static void doBlitAlphaBlendLogicGeneric(BlendBlit::Args &args) {
	const byte *in;
	byte *out;

	int scaleXCtr, scaleYCtr = args.scaleYoff;
	const byte *inBase;

	const byte ca = alphamod ? ((args.color >> BlendBlit::kAModShift) & 0xFF) : 255;
	const byte cr = rgbmod   ? ((args.color >> BlendBlit::kRModShift) & 0xFF) : 255;
	const byte cg = rgbmod   ? ((args.color >> BlendBlit::kGModShift) & 0xFF) : 255;
	const byte cb = rgbmod   ? ((args.color >> BlendBlit::kBModShift) & 0xFF) : 255;

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

			uint32 ina = in[BlendBlit::kAIndex] * ca >> 8;

			if (ina != 0) {
				if (rgbmod) {
					const uint outb = (out[BlendBlit::kBIndex] * (255 - ina) >> 8);
					const uint outg = (out[BlendBlit::kGIndex] * (255 - ina) >> 8);
					const uint outr = (out[BlendBlit::kRIndex] * (255 - ina) >> 8);

					out[BlendBlit::kAIndex] = 255;
					out[BlendBlit::kBIndex] = outb + (in[BlendBlit::kBIndex] * ina * cb >> 16);
					out[BlendBlit::kGIndex] = outg + (in[BlendBlit::kGIndex] * ina * cg >> 16);
					out[BlendBlit::kRIndex] = outr + (in[BlendBlit::kRIndex] * ina * cr >> 16);
				} else {
					out[BlendBlit::kAIndex] = 255;
					out[BlendBlit::kBIndex] = (out[BlendBlit::kBIndex] * (255 - ina) + in[BlendBlit::kBIndex] * ina) >> 8;
					out[BlendBlit::kGIndex] = (out[BlendBlit::kGIndex] * (255 - ina) + in[BlendBlit::kGIndex] * ina) >> 8;
					out[BlendBlit::kRIndex] = (out[BlendBlit::kRIndex] * (255 - ina) + in[BlendBlit::kRIndex] * ina) >> 8;
					
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

/**
 * Optimized version of doBlit to be used with subtractive blended blitting
 */
template<bool doscale, bool rgbmod>
static void doBlitSubtractiveBlendLogicGeneric(BlendBlit::Args &args) {
	const byte *in;
	byte *out;

	int scaleXCtr, scaleYCtr = args.scaleYoff;
	const byte *inBase;

	const byte rawcr = (args.color >> BlendBlit::kRModShift) & 0xFF;
	const byte rawcg = (args.color >> BlendBlit::kGModShift) & 0xFF;
	const byte rawcb = (args.color >> BlendBlit::kBModShift) & 0xFF;
	const uint32 cr = rgbmod   ? (rawcr == 255 ? 256 : rawcr) : 256;
	const uint32 cg = rgbmod   ? (rawcg == 255 ? 256 : rawcg) : 256;
	const uint32 cb = rgbmod   ? (rawcb == 255 ? 256 : rawcb) : 256;

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

			out[BlendBlit::kAIndex] = 255;
			out[BlendBlit::kBIndex] = MAX<int32>(out[BlendBlit::kBIndex] - ((in[BlendBlit::kBIndex] * cb  * (out[BlendBlit::kBIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);
			out[BlendBlit::kGIndex] = MAX<int32>(out[BlendBlit::kGIndex] - ((in[BlendBlit::kGIndex] * cg  * (out[BlendBlit::kGIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);
			out[BlendBlit::kRIndex] = MAX<int32>(out[BlendBlit::kRIndex] - ((in[BlendBlit::kRIndex] * cr * (out[BlendBlit::kRIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);

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
template<bool doscale, bool rgbmod, bool alphamod>
static void doBlitAdditiveBlendLogicGeneric(BlendBlit::Args &args) {
	const byte *in;
	byte *out;

	int scaleXCtr, scaleYCtr = args.scaleYoff;
	const byte *inBase;

	const byte rawcr = (args.color >> BlendBlit::kRModShift) & 0xFF;
	const byte rawcg = (args.color >> BlendBlit::kGModShift) & 0xFF;
	const byte rawcb = (args.color >> BlendBlit::kBModShift) & 0xFF;
	const byte ca = alphamod ? ((args.color >> BlendBlit::kAModShift) & 0xFF) : 255;
	const uint32 cr = rgbmod   ? (rawcr == 255 ? 256 : rawcr) : 256;
	const uint32 cg = rgbmod   ? (rawcg == 255 ? 256 : rawcg) : 256;
	const uint32 cb = rgbmod   ? (rawcb == 255 ? 256 : rawcb) : 256;

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

			uint32 ina = in[BlendBlit::kAIndex] * ca >> 8;

			if (ina != 0) {
				out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] + ((in[BlendBlit::kBIndex] * cb * ina) >> 16);
				out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] + ((in[BlendBlit::kGIndex] * cg * ina) >> 16);
				out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] + ((in[BlendBlit::kRIndex] * cr * ina) >> 16);
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

void BlendBlit::blitGeneric(Args &args, const TSpriteBlendMode &blendMode, const AlphaType &alphaType) {
	bool rgbmod   = ((args.color & kRGBModMask) != kRGBModMask);
	bool alphamod = ((args.color & kAModMask)   != kAModMask);
	if (args.scaleX == BlendBlit::SCALE_THRESHOLD && args.scaleY == BlendBlit::SCALE_THRESHOLD) {
		if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_OPAQUE) {
			BlendBlitImpl_Default::doBlitOpaqueBlendLogicGeneric<false>(args);
		} else if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_BINARY) {
			BlendBlitImpl_Default::doBlitBinaryBlendLogicGeneric<false>(args);
		} else {
			if (blendMode == BLEND_ADDITIVE) {
				if (rgbmod) {
					if (alphamod) {
						BlendBlitImpl_Default::doBlitAdditiveBlendLogicGeneric<false, true, true>(args);
					} else {
						BlendBlitImpl_Default::doBlitAdditiveBlendLogicGeneric<false, true, false>(args);
					}
				} else {
					if (alphamod) {
						BlendBlitImpl_Default::doBlitAdditiveBlendLogicGeneric<false, false, true>(args);
					} else {
						BlendBlitImpl_Default::doBlitAdditiveBlendLogicGeneric<false, false, false>(args);
					}
				}
			} else if (blendMode == BLEND_SUBTRACTIVE) {
				if (rgbmod) {
					BlendBlitImpl_Default::doBlitSubtractiveBlendLogicGeneric<false, true>(args);
				} else {
					BlendBlitImpl_Default::doBlitSubtractiveBlendLogicGeneric<false, false>(args);
				}
			} else if (blendMode == BLEND_MULTIPLY) {
				if (rgbmod) {
					if (alphamod) {
						BlendBlitImpl_Default::doBlitMultiplyBlendLogicGeneric<false, true, true>(args);
					} else {
						BlendBlitImpl_Default::doBlitMultiplyBlendLogicGeneric<false, true, false>(args);
					}
				} else {
					if (alphamod) {
						BlendBlitImpl_Default::doBlitMultiplyBlendLogicGeneric<false, false, true>(args);
					} else {
						BlendBlitImpl_Default::doBlitMultiplyBlendLogicGeneric<false, false, false>(args);
					}
				}
			} else {
				assert(blendMode == BLEND_NORMAL);
				if (rgbmod) {
					if (alphamod) {
						BlendBlitImpl_Default::doBlitAlphaBlendLogicGeneric<false, true, true>(args);
					} else {
						BlendBlitImpl_Default::doBlitAlphaBlendLogicGeneric<false, true, false>(args);
					}
				} else {
					if (alphamod) {
						BlendBlitImpl_Default::doBlitAlphaBlendLogicGeneric<false, false, true>(args);
					} else {
						BlendBlitImpl_Default::doBlitAlphaBlendLogicGeneric<false, false, false>(args);
					}
				}
			}
		}
	} else {
		if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_OPAQUE) {
			BlendBlitImpl_Default::doBlitOpaqueBlendLogicGeneric<true>(args);
		} else if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_BINARY) {
			BlendBlitImpl_Default::doBlitBinaryBlendLogicGeneric<true>(args);
		} else {
			if (blendMode == BLEND_ADDITIVE) {
				if (rgbmod) {
					if (alphamod) {
						BlendBlitImpl_Default::doBlitAdditiveBlendLogicGeneric<true, true, true>(args);
					} else {
						BlendBlitImpl_Default::doBlitAdditiveBlendLogicGeneric<true, true, false>(args);
					}
				} else {
					if (alphamod) {
						BlendBlitImpl_Default::doBlitAdditiveBlendLogicGeneric<true, false, true>(args);
					} else {
						BlendBlitImpl_Default::doBlitAdditiveBlendLogicGeneric<true, false, false>(args);
					}
				}
			} else if (blendMode == BLEND_SUBTRACTIVE) {
				if (rgbmod) {
					BlendBlitImpl_Default::doBlitSubtractiveBlendLogicGeneric<true, true>(args);
				} else {
					BlendBlitImpl_Default::doBlitSubtractiveBlendLogicGeneric<true, false>(args);
				}
			} else if (blendMode == BLEND_MULTIPLY) {
				if (rgbmod) {
					if (alphamod) {
						BlendBlitImpl_Default::doBlitMultiplyBlendLogicGeneric<true, true, true>(args);
					} else {
						BlendBlitImpl_Default::doBlitMultiplyBlendLogicGeneric<true, true, false>(args);
					}
				} else {
					if (alphamod) {
						BlendBlitImpl_Default::doBlitMultiplyBlendLogicGeneric<true, false, true>(args);
					} else {
						BlendBlitImpl_Default::doBlitMultiplyBlendLogicGeneric<true, false, false>(args);
					}
				}
			} else {
				assert(blendMode == BLEND_NORMAL);
				if (rgbmod) {
					if (alphamod) {
						BlendBlitImpl_Default::doBlitAlphaBlendLogicGeneric<true, true, true>(args);
					} else {
						BlendBlitImpl_Default::doBlitAlphaBlendLogicGeneric<true, true, false>(args);
					}
				} else {
					if (alphamod) {
						BlendBlitImpl_Default::doBlitAlphaBlendLogicGeneric<true, false, true>(args);
					} else {
						BlendBlitImpl_Default::doBlitAlphaBlendLogicGeneric<true, false, false>(args);
					}
				}
			}
		}
	}
}

} // End of namespace Graphics
