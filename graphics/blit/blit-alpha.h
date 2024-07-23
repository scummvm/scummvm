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

template<bool rgbmod, bool alphamod>
struct BaseBlend {
public:
	constexpr BaseBlend(const uint32 color) :
		ca(alphamod ? ((color >> BlendBlit::kAModShift) & 0xFF) : 255),
		cr(rgbmod   ? ((color >> BlendBlit::kRModShift) & 0xFF) : 255),
		cg(rgbmod   ? ((color >> BlendBlit::kGModShift) & 0xFF) : 255),
		cb(rgbmod   ? ((color >> BlendBlit::kBModShift) & 0xFF) : 255) {}

protected:
	const byte ca, cr, cg, cb;
};

template<bool rgbmod, bool alphamod>
struct AlphaBlend : public BaseBlend<rgbmod, alphamod> {
public:
	constexpr AlphaBlend(const uint32 color) : BaseBlend<rgbmod, alphamod>(color) {}

	inline void normal(const byte *in, byte *out) const {
		uint32 ina;

		if (alphamod) {
			ina = in[BlendBlit::kAIndex] * this->ca >> 8;
		} else {
			ina = in[BlendBlit::kAIndex];
		}

		if (ina == 255) {
			if (rgbmod) {
				out[BlendBlit::kAIndex] = 255;
				out[BlendBlit::kBIndex] = (in[BlendBlit::kBIndex] * this->cb >> 8);
				out[BlendBlit::kGIndex] = (in[BlendBlit::kGIndex] * this->cg >> 8);
				out[BlendBlit::kRIndex] = (in[BlendBlit::kRIndex] * this->cr >> 8);
			} else {
				out[BlendBlit::kAIndex] = 255;
				out[BlendBlit::kBIndex] = in[BlendBlit::kBIndex];
				out[BlendBlit::kGIndex] = in[BlendBlit::kGIndex];
				out[BlendBlit::kRIndex] = in[BlendBlit::kRIndex];
			}
		} else if (ina != 0) {
			if (rgbmod) {
				const uint outb = (out[BlendBlit::kBIndex] * (255 - ina) >> 8);
				const uint outg = (out[BlendBlit::kGIndex] * (255 - ina) >> 8);
				const uint outr = (out[BlendBlit::kRIndex] * (255 - ina) >> 8);

				out[BlendBlit::kAIndex] = 255;
				out[BlendBlit::kBIndex] = outb + (in[BlendBlit::kBIndex] * ina * this->cb >> 16);
				out[BlendBlit::kGIndex] = outg + (in[BlendBlit::kGIndex] * ina * this->cg >> 16);
				out[BlendBlit::kRIndex] = outr + (in[BlendBlit::kRIndex] * ina * this->cr >> 16);
			} else {
				out[BlendBlit::kAIndex] = 255;
				out[BlendBlit::kBIndex] = (out[BlendBlit::kBIndex] * (255 - ina) + in[BlendBlit::kBIndex] * ina) >> 8;
				out[BlendBlit::kGIndex] = (out[BlendBlit::kGIndex] * (255 - ina) + in[BlendBlit::kGIndex] * ina) >> 8;
				out[BlendBlit::kRIndex] = (out[BlendBlit::kRIndex] * (255 - ina) + in[BlendBlit::kRIndex] * ina) >> 8;
			}
		}

	}
};

template<bool rgbmod, bool alphamod>
struct MultiplyBlend : public BaseBlend<rgbmod, alphamod> {
public:
	constexpr MultiplyBlend(const uint32 color) : BaseBlend<rgbmod, alphamod>(color) {}

	inline void normal(const byte *in, byte *out) const {
		uint32 ina;

		if (alphamod) {
			ina = in[BlendBlit::kAIndex] * this->ca >> 8;
		} else {
			ina = in[BlendBlit::kAIndex];
		}

		if (ina == 255) {
			if (rgbmod) {
				out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] * ((in[BlendBlit::kBIndex] * this->cb) >> 8) >> 8;
				out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] * ((in[BlendBlit::kGIndex] * this->cg) >> 8) >> 8;
				out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] * ((in[BlendBlit::kRIndex] * this->cr) >> 8) >> 8;
			} else {
				out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] * in[BlendBlit::kBIndex] >> 8;
				out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] * in[BlendBlit::kGIndex] >> 8;
				out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] * in[BlendBlit::kRIndex] >> 8;
			}
		} else if (ina != 0) {
			if (rgbmod) {
				out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] * ((in[BlendBlit::kBIndex] * this->cb * ina) >> 16) >> 8;
				out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] * ((in[BlendBlit::kGIndex] * this->cg * ina) >> 16) >> 8;
				out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] * ((in[BlendBlit::kRIndex] * this->cr * ina) >> 16) >> 8;
			} else {
				out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] * ((in[BlendBlit::kBIndex] * ina) >> 8) >> 8;
				out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] * ((in[BlendBlit::kGIndex] * ina) >> 8) >> 8;
				out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] * ((in[BlendBlit::kRIndex] * ina) >> 8) >> 8;
			}
		}
	}
};

template<bool rgbmod, bool alphamod>
struct OpaqueBlend : public BaseBlend<rgbmod, alphamod> {
public:
	constexpr OpaqueBlend(const uint32 color) : BaseBlend<rgbmod, alphamod>(color) {}

	inline void normal(const byte *in, byte *out) const {
		*(uint32 *)out = *(const uint32 *)in | BlendBlit::kAModMask;
	}
};

template<bool rgbmod, bool alphamod>
struct BinaryBlend : public BaseBlend<rgbmod, alphamod> {
public:
	constexpr BinaryBlend(const uint32 color) : BaseBlend<rgbmod, alphamod>(color) {}

	inline void normal(const byte *in, byte *out) const {
		uint32 pix = *(const uint32 *)in;
		uint32 a = pix & BlendBlit::kAModMask;

		if (a != 0) {   // Full opacity (Any value not exactly 0 is Opaque here)
			*(uint32 *)out = pix | BlendBlit::kAModMask;
		}
	}
};

template<bool rgbmod, bool alphamod>
struct AdditiveBlend : public BaseBlend<rgbmod, alphamod> {
public:
	constexpr AdditiveBlend(const uint32 color) : BaseBlend<rgbmod, alphamod>(color) {}

	inline void normal(const byte *in, byte *out) const {
		uint32 ina;

		if (alphamod) {
			ina = in[BlendBlit::kAIndex] * this->ca >> 8;
		} else {
			ina = in[BlendBlit::kAIndex];
		}

		if (ina == 255) {
			if (rgbmod) {
				out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] + ((in[BlendBlit::kBIndex] * this->cb) >> 8);
				out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] + ((in[BlendBlit::kGIndex] * this->cg) >> 8);
				out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] + ((in[BlendBlit::kRIndex] * this->cr) >> 8);
			} else {
				out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] + in[BlendBlit::kBIndex];
				out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] + in[BlendBlit::kGIndex];
				out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] + in[BlendBlit::kRIndex];
			}
		} else if (ina != 0) {
			if (rgbmod) {
				out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] + ((in[BlendBlit::kBIndex] * this->cb * ina) >> 16);
				out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] + ((in[BlendBlit::kGIndex] * this->cg * ina) >> 16);
				out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] + ((in[BlendBlit::kRIndex] * this->cr * ina) >> 16);
			} else {
				out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] + ((in[BlendBlit::kBIndex] * ina) >> 8);
				out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] + ((in[BlendBlit::kGIndex] * ina) >> 8);
				out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] + ((in[BlendBlit::kRIndex] * ina) >> 8);
			}
		}
	}
};

template<bool rgbmod, bool alphamod>
struct SubtractiveBlend : public BaseBlend<rgbmod, alphamod> {
public:
	constexpr SubtractiveBlend(const uint32 color) : BaseBlend<rgbmod, alphamod>(color) {}

	inline void normal(const byte *in, byte *out) const {
		uint32 ina = in[BlendBlit::kAIndex];
		out[BlendBlit::kAIndex] = 255;

		if (ina == 255) {
			if (rgbmod) {
				out[BlendBlit::kBIndex] = MAX<int32>(out[BlendBlit::kBIndex] - ((in[BlendBlit::kBIndex] * this->cb * (out[BlendBlit::kBIndex])) >> 16), 0);
				out[BlendBlit::kGIndex] = MAX<int32>(out[BlendBlit::kGIndex] - ((in[BlendBlit::kGIndex] * this->cg * (out[BlendBlit::kGIndex])) >> 16), 0);
				out[BlendBlit::kRIndex] = MAX<int32>(out[BlendBlit::kRIndex] - ((in[BlendBlit::kRIndex] * this->cr * (out[BlendBlit::kRIndex])) >> 16), 0);
			} else {
				out[BlendBlit::kBIndex] = MAX<int32>(out[BlendBlit::kBIndex] - ((in[BlendBlit::kBIndex] * (out[BlendBlit::kBIndex])) >> 8), 0);
				out[BlendBlit::kGIndex] = MAX<int32>(out[BlendBlit::kGIndex] - ((in[BlendBlit::kGIndex] * (out[BlendBlit::kGIndex])) >> 8), 0);
				out[BlendBlit::kRIndex] = MAX<int32>(out[BlendBlit::kRIndex] - ((in[BlendBlit::kRIndex] * (out[BlendBlit::kRIndex])) >> 8), 0);
			}
		} else if (ina != 0) {
			if (rgbmod) {
				out[BlendBlit::kBIndex] = MAX<int32>(out[BlendBlit::kBIndex] - ((in[BlendBlit::kBIndex] * this->cb * (out[BlendBlit::kBIndex]) * ina) >> 24), 0);
				out[BlendBlit::kGIndex] = MAX<int32>(out[BlendBlit::kGIndex] - ((in[BlendBlit::kGIndex] * this->cg * (out[BlendBlit::kGIndex]) * ina) >> 24), 0);
				out[BlendBlit::kRIndex] = MAX<int32>(out[BlendBlit::kRIndex] - ((in[BlendBlit::kRIndex] * this->cr * (out[BlendBlit::kRIndex]) * ina) >> 24), 0);
			} else {
				out[BlendBlit::kBIndex] = MAX<int32>(out[BlendBlit::kBIndex] - ((in[BlendBlit::kBIndex] * (out[BlendBlit::kBIndex]) * ina) >> 16), 0);
				out[BlendBlit::kGIndex] = MAX<int32>(out[BlendBlit::kGIndex] - ((in[BlendBlit::kGIndex] * (out[BlendBlit::kGIndex]) * ina) >> 16), 0);
				out[BlendBlit::kRIndex] = MAX<int32>(out[BlendBlit::kRIndex] - ((in[BlendBlit::kRIndex] * (out[BlendBlit::kRIndex]) * ina) >> 16), 0);
			}
		}
	}
};

}; // End of class BlendBlitImpl_Base

template<class T>
void BlendBlit::blitT(Args &args, const TSpriteBlendMode &blendMode, const AlphaType &alphaType) {
	bool rgbmod   = ((args.color & kRGBModMask) != kRGBModMask);
	bool alphamod = ((args.color & kAModMask)   != kAModMask);
	if (args.scaleX == SCALE_THRESHOLD && args.scaleY == SCALE_THRESHOLD) {
		if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_OPAQUE) {
			T::template blitInnerLoop<T::template OpaqueBlend, false, false, false>(args);
		} else if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_BINARY) {
			T::template blitInnerLoop<T::template BinaryBlend, false, false, false>(args);
		} else {
			if (blendMode == BLEND_ADDITIVE) {
				if (rgbmod) {
					if (alphamod) {
						T::template blitInnerLoop<T::template AdditiveBlend, false, true, true>(args);
					} else {
						T::template blitInnerLoop<T::template AdditiveBlend, false, true, false>(args);
					}
				} else {
					if (alphamod) {
						T::template blitInnerLoop<T::template AdditiveBlend, false, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template AdditiveBlend, false, false, false>(args);
					}
				}
			} else if (blendMode == BLEND_SUBTRACTIVE) {
				if (rgbmod) {
					T::template blitInnerLoop<T::template SubtractiveBlend, false, true, false>(args);
				} else {
					T::template blitInnerLoop<T::template SubtractiveBlend, false, false, false>(args);
				}
			} else if (blendMode == BLEND_MULTIPLY) {
				if (rgbmod) {
					if (alphamod) {
						T::template blitInnerLoop<T::template MultiplyBlend, false, true, true>(args);
					} else {
						T::template blitInnerLoop<T::template MultiplyBlend, false, true, false>(args);
					}
				} else {
					if (alphamod) {
						T::template blitInnerLoop<T::template MultiplyBlend, false, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template MultiplyBlend, false, false, false>(args);
					}
				}
			} else {
				assert(blendMode == BLEND_NORMAL);
				if (rgbmod) {
					if (alphamod) {
						T::template blitInnerLoop<T::template AlphaBlend, false, true, true>(args);
					} else {
						T::template blitInnerLoop<T::template AlphaBlend, false, true, false>(args);
					}
				} else {
					if (alphamod) {
						T::template blitInnerLoop<T::template AlphaBlend, false, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template AlphaBlend, false, false, false>(args);
					}
				}
			}
		}
	} else {
		if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_OPAQUE) {
			T::template blitInnerLoop<T::template OpaqueBlend, true, false, false>(args);
		} else if (args.color == 0xffffffff && blendMode == BLEND_NORMAL && alphaType == ALPHA_BINARY) {
			T::template blitInnerLoop<T::template BinaryBlend, true, false, false>(args);
		} else {
			if (blendMode == BLEND_ADDITIVE) {
				if (rgbmod) {
					if (alphamod) {
						T::template blitInnerLoop<T::template AdditiveBlend, true, true, true>(args);
					} else {
						T::template blitInnerLoop<T::template AdditiveBlend, true, true, false>(args);
					}
				} else {
					if (alphamod) {
						T::template blitInnerLoop<T::template AdditiveBlend, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template AdditiveBlend, true, false, false>(args);
					}
				}
			} else if (blendMode == BLEND_SUBTRACTIVE) {
				if (rgbmod) {
					T::template blitInnerLoop<T::template SubtractiveBlend, true, true, false>(args);
				} else {
					T::template blitInnerLoop<T::template SubtractiveBlend, true, false, false>(args);
				}
			} else if (blendMode == BLEND_MULTIPLY) {
				if (rgbmod) {
					if (alphamod) {
						T::template blitInnerLoop<T::template MultiplyBlend, true, true, true>(args);
					} else {
						T::template blitInnerLoop<T::template MultiplyBlend, true, true, false>(args);
					}
				} else {
					if (alphamod) {
						T::template blitInnerLoop<T::template MultiplyBlend, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template MultiplyBlend, true, false, false>(args);
					}
				}
			} else {
				assert(blendMode == BLEND_NORMAL);
				if (rgbmod) {
					if (alphamod) {
						T::template blitInnerLoop<T::template AlphaBlend, true, true, true>(args);
					} else {
						T::template blitInnerLoop<T::template AlphaBlend, true, true, false>(args);
					}
				} else {
					if (alphamod) {
						T::template blitInnerLoop<T::template AlphaBlend, true, false, true>(args);
					} else {
						T::template blitInnerLoop<T::template AlphaBlend, true, false, false>(args);
					}
				}
			}
		}
	}
}

} // End of namespace Graphics
