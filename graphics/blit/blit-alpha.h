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

namespace Graphics {

class BlendBlitImpl_Base {
	friend class BlendBlit;
protected:

struct AlphaBlend {
	static inline void normal(const byte *in, byte *out, const byte ca, const byte cr, const byte cg, const byte cb) {
		uint32 ina = in[BlendBlit::kAIndex] * ca >> 8;

		if (ina != 0) {
			uint outb = (out[BlendBlit::kBIndex] * (255 - ina) >> 8);
			uint outg = (out[BlendBlit::kGIndex] * (255 - ina) >> 8);
			uint outr = (out[BlendBlit::kRIndex] * (255 - ina) >> 8);

			out[BlendBlit::kAIndex] = 255;
			out[BlendBlit::kBIndex] = outb + (in[BlendBlit::kBIndex] * ina * cb >> 16);
			out[BlendBlit::kGIndex] = outg + (in[BlendBlit::kGIndex] * ina * cg >> 16);
			out[BlendBlit::kRIndex] = outr + (in[BlendBlit::kRIndex] * ina * cr >> 16);
		}
	}
};

struct MultiplyBlend {
	static inline void normal(const byte *in, byte *out, const byte ca, const byte cr, const byte cg, const byte cb) {
		uint32 ina = in[BlendBlit::kAIndex] * ca >> 8;

		if (ina != 0) {
			out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] * ((in[BlendBlit::kBIndex] * cb * ina) >> 16) >> 8;
			out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] * ((in[BlendBlit::kGIndex] * cg * ina) >> 16) >> 8;
			out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] * ((in[BlendBlit::kRIndex] * cr * ina) >> 16) >> 8;
		}
	}
};

struct OpaqueBlend {
	static inline void normal(const byte *in, byte *out, const byte ca, const byte cr, const byte cg, const byte cb) {
		*(uint32 *)out = *(const uint32 *)in | BlendBlit::kAModMask;
	}
};

struct BinaryBlend {
	static inline void normal(const byte *in, byte *out, const byte ca, const byte cr, const byte cg, const byte cb) {
		uint32 pix = *(const uint32 *)in;
		int a = in[BlendBlit::kAIndex];

		if (a != 0) {   // Full opacity (Any value not exactly 0 is Opaque here)
			*(uint32 *)out = pix;
			out[BlendBlit::kAIndex] = 0xFF;
		}
	}
};

struct AdditiveBlend {
	static inline void normal(const byte *in, byte *out, const byte ca, const byte cr, const byte cg, const byte cb) {
		uint32 ina = in[BlendBlit::kAIndex] * ca >> 8;

		if (ina != 0) {
			out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] + ((in[BlendBlit::kBIndex] * cb * ina) >> 16);
			out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] + ((in[BlendBlit::kGIndex] * cg * ina) >> 16);
			out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] + ((in[BlendBlit::kRIndex] * cr * ina) >> 16);
		}
	}
};

struct SubtractiveBlend {
	static inline void normal(const byte *in, byte *out, const byte ca, const byte cr, const byte cg, const byte cb) {
		out[BlendBlit::kAIndex] = 255;
		out[BlendBlit::kBIndex] = MAX<int32>(out[BlendBlit::kBIndex] - ((in[BlendBlit::kBIndex] * cb  * (out[BlendBlit::kBIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);
		out[BlendBlit::kGIndex] = MAX<int32>(out[BlendBlit::kGIndex] - ((in[BlendBlit::kGIndex] * cg  * (out[BlendBlit::kGIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);
		out[BlendBlit::kRIndex] = MAX<int32>(out[BlendBlit::kRIndex] - ((in[BlendBlit::kRIndex] * cr *  (out[BlendBlit::kRIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);
	}
};

}; // End of class BlendBlitImpl_Base

template<class T>
void BlendBlit::blitT(Args &args, const TSpriteBlendMode &blendMode, const AlphaType &alphaType) {
	bool rgbmod   = ((args.color & kRGBModMask) != kRGBModMask);
	bool alphamod = ((args.color & kAModMask)   != kAModMask);
	if (args.scaleX == SCALE_THRESHOLD && args.scaleY == SCALE_THRESHOLD) {
		if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_OPAQUE) {
			T::template blitInnerLoop<T::template OpaqueBlend, false, false, false, false, true>(args);
		} else if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_BINARY) {
			T::template blitInnerLoop<T::template BinaryBlend, false, false, false, false, true>(args);
		} else {
			if (blendMode == BLEND_ADDITIVE) {
				if (rgbmod) {
					if (alphamod) {
						T::template blitInnerLoop<T::template AdditiveBlend, false, true, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template AdditiveBlend, false, true, false, false, true>(args);
					}
				} else {
					if (alphamod) {
						T::template blitInnerLoop<T::template AdditiveBlend, false, false, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template AdditiveBlend, false, false, false, false, true>(args);
					}
				}
			} else if (blendMode == BLEND_SUBTRACTIVE) {
				if (rgbmod) {
					T::template blitInnerLoop<T::template SubtractiveBlend, false, true, false, false, true>(args);
				} else {
					T::template blitInnerLoop<T::template SubtractiveBlend, false, false, false, false, true>(args);
				}
			} else if (blendMode == BLEND_MULTIPLY) {
				if (rgbmod) {
					if (alphamod) {
						T::template blitInnerLoop<T::template MultiplyBlend, false, true, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template MultiplyBlend, false, true, false, false, true>(args);
					}
				} else {
					if (alphamod) {
						T::template blitInnerLoop<T::template MultiplyBlend, false, false, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template MultiplyBlend, false, false, false, false, true>(args);
					}
				}
			} else {
				assert(blendMode == BLEND_NORMAL);
				if (rgbmod) {
					if (alphamod) {
						T::template blitInnerLoop<T::template AlphaBlend, false, true, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template AlphaBlend, false, true, false, false, true>(args);
					}
				} else {
					if (alphamod) {
						T::template blitInnerLoop<T::template AlphaBlend, false, false, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template AlphaBlend, false, false, false, false, true>(args);
					}
				}
			}
		}
	} else {
		if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_OPAQUE) {
			T::template blitInnerLoop<T::template OpaqueBlend, true, false, false, false, true>(args);
		} else if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_BINARY) {
			T::template blitInnerLoop<T::template BinaryBlend, true, false, false, false, true>(args);
		} else {
			if (blendMode == BLEND_ADDITIVE) {
				if (rgbmod) {
					if (alphamod) {
						T::template blitInnerLoop<T::template AdditiveBlend, true, true, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template AdditiveBlend, true, true, false, false, true>(args);
					}
				} else {
					if (alphamod) {
						T::template blitInnerLoop<T::template AdditiveBlend, true, false, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template AdditiveBlend, true, false, false, false, true>(args);
					}
				}
			} else if (blendMode == BLEND_SUBTRACTIVE) {
				if (rgbmod) {
					T::template blitInnerLoop<T::template SubtractiveBlend, true, true, false, false, true>(args);
				} else {
					T::template blitInnerLoop<T::template SubtractiveBlend, true, false, false, false, true>(args);
				}
			} else if (blendMode == BLEND_MULTIPLY) {
				if (rgbmod) {
					if (alphamod) {
						T::template blitInnerLoop<T::template MultiplyBlend, true, true, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template MultiplyBlend, true, true, false, false, true>(args);
					}
				} else {
					if (alphamod) {
						T::template blitInnerLoop<T::template MultiplyBlend, true, false, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template MultiplyBlend, true, false, false, false, true>(args);
					}
				}
			} else {
				assert(blendMode == BLEND_NORMAL);
				if (rgbmod) {
					if (alphamod) {
						T::template blitInnerLoop<T::template AlphaBlend, true, true, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template AlphaBlend, true, true, false, false, true>(args);
					}
				} else {
					if (alphamod) {
						T::template blitInnerLoop<T::template AlphaBlend, true, false, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template AlphaBlend, true, false, false, false, true>(args);
					}
				}
			}
		}
	}
}

} // End of namespace Graphics
