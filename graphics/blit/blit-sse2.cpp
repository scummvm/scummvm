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

#include "common/scummsys.h"

#include "graphics/blit/blit-alpha.h"
#include "graphics/pixelformat.h"

#include <emmintrin.h>

#if !defined(__x86_64__)

#if defined(__clang__)
#pragma clang attribute push (__attribute__((target("sse2"))), apply_to=function)
#elif defined(__GNUC__)
#pragma GCC push_options
#pragma GCC target("sse2")
#endif

#endif // !defined(__x86_64__)

namespace Graphics {

static FORCEINLINE __m128i sse2_mul32(__m128i a, __m128i b) {
	__m128i even = _mm_shuffle_epi32(_mm_mul_epu32(a, b), _MM_SHUFFLE(0, 0, 2, 0));
	__m128i odd = _mm_shuffle_epi32(_mm_mul_epu32(_mm_bsrli_si128(a, 4), _mm_bsrli_si128(b, 4)), _MM_SHUFFLE(0, 0, 2, 0));
	return _mm_unpacklo_epi32(even, odd);
}

class BlendBlitImpl_SSE2 : public BlendBlitImpl_Base {
	friend class BlendBlit;

template<bool rgbmod, bool alphamod>
struct AlphaBlend : public BlendBlitImpl_Base::AlphaBlend<rgbmod, alphamod> {
public:
	constexpr AlphaBlend(const uint32 color) : BlendBlitImpl_Base::AlphaBlend<rgbmod, alphamod>(color) {}

	inline __m128i simd(__m128i src, __m128i dst) const {
		__m128i ina;
		if (alphamod)
			ina = _mm_srli_epi32(_mm_mullo_epi16(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kAModMask)), _mm_set1_epi32(this->ca)), 8);
		else
			ina = _mm_and_si128(src, _mm_set1_epi32(BlendBlit::kAModMask));
		__m128i alphaMask = _mm_cmpeq_epi32(ina, _mm_setzero_si128());
	
		if (rgbmod) {
			__m128i dstR = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
			__m128i dstG = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m128i dstB = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m128i srcR = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
			__m128i srcG = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m128i srcB = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);

			dstR = _mm_slli_epi32(_mm_mullo_epi16(dstR, _mm_sub_epi32(_mm_set1_epi32(255), ina)), BlendBlit::kRModShift - 8);
			dstG = _mm_slli_epi32(_mm_mullo_epi16(dstG, _mm_sub_epi32(_mm_set1_epi32(255), ina)), BlendBlit::kGModShift - 8);
			dstB = _mm_mullo_epi16(dstB, _mm_sub_epi32(_mm_set1_epi32(255), ina));
			srcR = _mm_add_epi32(dstR, _mm_slli_epi32(_mm_mullo_epi16(_mm_srli_epi32(_mm_mullo_epi16(srcR, ina), 8), _mm_set1_epi32(this->cr)), BlendBlit::kRModShift - 8));
			srcG = _mm_add_epi32(dstG, _mm_slli_epi32(_mm_mullo_epi16(_mm_srli_epi32(_mm_mullo_epi16(srcG, ina), 8), _mm_set1_epi32(this->cg)), BlendBlit::kGModShift - 8));
			srcB = _mm_add_epi32(dstB, _mm_mullo_epi16(_mm_srli_epi32(_mm_mullo_epi16(srcB, ina), 8), _mm_set1_epi32(this->cb)));
			src = _mm_or_si128(_mm_and_si128(srcB, _mm_set1_epi32(BlendBlit::kBModMask)), _mm_set1_epi32(BlendBlit::kAModMask));
			src = _mm_or_si128(_mm_and_si128(srcG, _mm_set1_epi32(BlendBlit::kGModMask)), src);
			src = _mm_or_si128(_mm_and_si128(srcR, _mm_set1_epi32(BlendBlit::kRModMask)), src);
		} else {
			__m128i dstRB = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m128i srcRB = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m128i dstG = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m128i srcG = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);

			dstRB = _mm_srli_epi32(sse2_mul32(dstRB, _mm_sub_epi32(_mm_set1_epi32(255), ina)), 8);
			dstG = _mm_srli_epi32(_mm_mullo_epi16(dstG, _mm_sub_epi32(_mm_set1_epi32(255), ina)), 8);
			srcRB = _mm_slli_epi32(_mm_add_epi32(dstRB, _mm_srli_epi32(sse2_mul32(srcRB, ina), 8)), BlendBlit::kBModShift);
			srcG = _mm_slli_epi32(_mm_add_epi32(dstG, _mm_srli_epi32(_mm_mullo_epi16(srcG, ina), 8)), BlendBlit::kGModShift);
			src = _mm_or_si128(_mm_and_si128(srcG, _mm_set1_epi32(BlendBlit::kGModMask)), _mm_set1_epi32(BlendBlit::kAModMask));
			src = _mm_or_si128(_mm_and_si128(srcRB, _mm_set1_epi32(BlendBlit::kBModMask | BlendBlit::kRModMask)), src);
		}

		dst = _mm_and_si128(alphaMask, dst);
		src = _mm_andnot_si128(alphaMask, src);
		return _mm_or_si128(dst, src);
	}
};

template<bool rgbmod, bool alphamod>
struct MultiplyBlend : public BlendBlitImpl_Base::MultiplyBlend<rgbmod, alphamod> {
public:
	constexpr MultiplyBlend(const uint32 color) : BlendBlitImpl_Base::MultiplyBlend<rgbmod, alphamod>(color) {}

	inline __m128i simd(__m128i src, __m128i dst) const {
		__m128i ina, alphaMask;
		if (alphamod) {
			ina = _mm_srli_epi32(_mm_mullo_epi16(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kAModMask)), _mm_set1_epi32(this->ca)), 8);
			alphaMask = _mm_cmpeq_epi32(ina, _mm_setzero_si128());
		} else {
			ina = _mm_and_si128(src, _mm_set1_epi32(BlendBlit::kAModMask));
			alphaMask = _mm_set1_epi32(BlendBlit::kAModMask);
		}

		if (rgbmod) {
			__m128i srcB = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m128i srcG = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m128i srcR = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
			__m128i dstB = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m128i dstG = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m128i dstR = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);

			srcB = _mm_and_si128(_mm_slli_epi32(_mm_mullo_epi16(dstB, _mm_srli_epi32(sse2_mul32(_mm_mullo_epi16(srcB, _mm_set1_epi32(this->cb)), ina), 16)), BlendBlit::kBModShift - 8), _mm_set1_epi32(BlendBlit::kBModMask));
			srcG = _mm_and_si128(_mm_slli_epi32(_mm_mullo_epi16(dstG, _mm_srli_epi32(sse2_mul32(_mm_mullo_epi16(srcG, _mm_set1_epi32(this->cg)), ina), 16)), BlendBlit::kGModShift - 8), _mm_set1_epi32(BlendBlit::kGModMask));
			srcR = _mm_and_si128(_mm_slli_epi32(_mm_mullo_epi16(dstR, _mm_srli_epi32(sse2_mul32(_mm_mullo_epi16(srcR, _mm_set1_epi32(this->cr)), ina), 16)), BlendBlit::kRModShift - 8), _mm_set1_epi32(BlendBlit::kRModMask));

			src = _mm_and_si128(src, _mm_set1_epi32(BlendBlit::kAModMask));
			src = _mm_or_si128(src, _mm_or_si128(srcB, _mm_or_si128(srcG, srcR)));
		} else {
			constexpr uint32 rbMask = BlendBlit::kRModMask | BlendBlit::kBModMask;
			__m128i srcG = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m128i srcRB = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(rbMask)), BlendBlit::kBModShift);
			__m128i dstG = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m128i dstRB = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(rbMask)), BlendBlit::kBModShift);

			srcG = _mm_and_si128(_mm_slli_epi32(_mm_mullo_epi16(dstG, _mm_srli_epi32(_mm_mullo_epi16(srcG, ina), 8)), 8), _mm_set1_epi32(BlendBlit::kGModMask));
			srcRB = _mm_and_si128(_mm_mullo_epi16(dstRB, _mm_srli_epi32(_mm_and_si128(sse2_mul32(srcRB, ina), _mm_set1_epi32(rbMask)), 8)), _mm_set1_epi32(rbMask));
			
			src = _mm_and_si128(src, _mm_set1_epi32(BlendBlit::kAModMask));
			src = _mm_or_si128(src, _mm_or_si128(srcRB, srcG));
		}

		dst = _mm_and_si128(alphaMask, dst);
		src = _mm_andnot_si128(alphaMask, src);
		return _mm_or_si128(dst, src);
	}
};

template<bool rgbmod, bool alphamod>
struct OpaqueBlend : public BlendBlitImpl_Base::OpaqueBlend<rgbmod, alphamod> {
public:
	constexpr OpaqueBlend(const uint32 color) : BlendBlitImpl_Base::OpaqueBlend<rgbmod, alphamod>(color) {}

	inline __m128i simd(__m128i src, __m128i dst) const {
		return _mm_or_si128(src, _mm_set1_epi32(BlendBlit::kAModMask));
	}
};

template<bool rgbmod, bool alphamod>
struct BinaryBlend : public BlendBlitImpl_Base::BinaryBlend<rgbmod, alphamod> {
public:
	constexpr BinaryBlend(const uint32 color) : BlendBlitImpl_Base::BinaryBlend<rgbmod, alphamod>(color) {}

	inline __m128i simd(__m128i src, __m128i dst) const {
		__m128i alphaMask = _mm_cmpeq_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kAModMask)), _mm_setzero_si128());
		dst = _mm_and_si128(dst, alphaMask);
		src = _mm_andnot_si128(alphaMask, _mm_or_si128(src, _mm_set1_epi32(BlendBlit::kAModMask)));
		return _mm_or_si128(src, dst);
	}
};

template<bool rgbmod, bool alphamod>
struct AdditiveBlend : public BlendBlitImpl_Base::AdditiveBlend<rgbmod, alphamod> {
public:
	constexpr AdditiveBlend(const uint32 color) : BlendBlitImpl_Base::AdditiveBlend<rgbmod, alphamod>(color) {}

	inline __m128i simd(__m128i src, __m128i dst) const {
		__m128i ina;
		if (alphamod)
			ina = _mm_srli_epi32(sse2_mul32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kAModMask)), _mm_set1_epi32(this->ca)), 8);
		else
			ina = _mm_and_si128(src, _mm_set1_epi32(BlendBlit::kAModMask));
		__m128i alphaMask = _mm_cmpeq_epi32(ina, _mm_set1_epi32(0));

		if (rgbmod) {
			__m128i srcb = _mm_and_si128(src, _mm_set1_epi32(BlendBlit::kBModMask));
			__m128i srcg = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m128i srcr = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
			__m128i dstb = _mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kBModMask));
			__m128i dstg = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m128i dstr = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);

			srcb = _mm_and_si128(_mm_add_epi32(dstb, _mm_srli_epi32(sse2_mul32(srcb, sse2_mul32(_mm_set1_epi32(this->cb), ina)), 16)), _mm_set1_epi32(BlendBlit::kBModMask));
			srcg = _mm_and_si128(_mm_add_epi32(dstg, sse2_mul32(srcg, sse2_mul32(_mm_set1_epi32(this->cg), ina))), _mm_set1_epi32(BlendBlit::kGModMask));
			srcr = _mm_and_si128(_mm_add_epi32(dstr, _mm_srli_epi32(sse2_mul32(srcr, sse2_mul32(_mm_set1_epi32(this->cr), ina)), BlendBlit::kRModShift - 16)), _mm_set1_epi32(BlendBlit::kRModMask));

			src = _mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kAModMask));
			src = _mm_or_si128(src, _mm_or_si128(srcb, _mm_or_si128(srcg, srcr)));
		} else if (alphamod) {
			__m128i srcg = _mm_and_si128(src, _mm_set1_epi32(BlendBlit::kGModMask));
			__m128i srcrb = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m128i dstg = _mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kGModMask));
			__m128i dstrb = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);

			srcg = _mm_and_si128(_mm_add_epi32(dstg, _mm_srli_epi32(sse2_mul32(srcg, ina), 8)), _mm_set1_epi32(BlendBlit::kGModMask));
			srcrb = _mm_and_si128(_mm_add_epi32(dstrb, sse2_mul32(srcrb, ina)), _mm_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask));

			src = _mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kAModMask));
			src = _mm_or_si128(src, _mm_or_si128(srcrb, srcg));
		} else {
			__m128i srcg = _mm_and_si128(src, _mm_set1_epi32(BlendBlit::kGModMask));
			__m128i srcrb = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m128i dstg = _mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kGModMask));
			__m128i dstrb = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);

			srcg = _mm_and_si128(_mm_add_epi32(dstg, srcg), _mm_set1_epi32(BlendBlit::kGModMask));
			srcrb = _mm_and_si128(_mm_slli_epi32(_mm_add_epi32(dstrb, srcrb), 8), _mm_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask));

			src = _mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kAModMask));
			src = _mm_or_si128(src, _mm_or_si128(srcrb, srcg));
		}

		dst = _mm_and_si128(alphaMask, dst);
		src = _mm_andnot_si128(alphaMask, src);
		return _mm_or_si128(dst, src);
	}
};

template<bool rgbmod, bool alphamod>
struct SubtractiveBlend : public BlendBlitImpl_Base::SubtractiveBlend<rgbmod, alphamod> {
public:
	constexpr SubtractiveBlend(const uint32 color) : BlendBlitImpl_Base::SubtractiveBlend<rgbmod, alphamod>(color) {}

	inline __m128i simd(__m128i src, __m128i dst) const {
		__m128i ina = _mm_and_si128(src, _mm_set1_epi32(BlendBlit::kAModMask));
		__m128i srcb = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
		__m128i srcg = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
		__m128i srcr = _mm_srli_epi32(_mm_and_si128(src, _mm_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
		__m128i dstb = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
		__m128i dstg = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
		__m128i dstr = _mm_srli_epi32(_mm_and_si128(dst, _mm_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);

		srcb = _mm_and_si128(_mm_slli_epi32(_mm_max_epi16(_mm_sub_epi32(dstb, _mm_srli_epi32(sse2_mul32(sse2_mul32(srcb, _mm_set1_epi32(this->cb)), sse2_mul32(dstb, ina)), 24)), _mm_set1_epi32(0)), BlendBlit::kBModShift), _mm_set1_epi32(BlendBlit::kBModMask));
		srcg = _mm_and_si128(_mm_slli_epi32(_mm_max_epi16(_mm_sub_epi32(dstg, _mm_srli_epi32(sse2_mul32(sse2_mul32(srcg, _mm_set1_epi32(this->cg)), sse2_mul32(dstg, ina)), 24)), _mm_set1_epi32(0)), BlendBlit::kGModShift), _mm_set1_epi32(BlendBlit::kGModMask));
		srcr = _mm_and_si128(_mm_slli_epi32(_mm_max_epi16(_mm_sub_epi32(dstr, _mm_srli_epi32(sse2_mul32(sse2_mul32(srcr, _mm_set1_epi32(this->cr)), sse2_mul32(dstr, ina)), 24)), _mm_set1_epi32(0)), BlendBlit::kRModShift), _mm_set1_epi32(BlendBlit::kRModMask));

		return _mm_or_si128(_mm_set1_epi32(BlendBlit::kAModMask), _mm_or_si128(srcb, _mm_or_si128(srcg, srcr)));
	}
};

public:
template<template <bool RGBMOD, bool ALPHAMOD> class PixelFunc, bool doscale, bool rgbmod, bool alphamod>
static inline void blitInnerLoop(BlendBlit::Args &args) {
	const bool loaddst = true; // TODO: Only set this when necessary

	const byte *in;
	byte *out;

	PixelFunc<rgbmod, alphamod> pixelFunc(args.color);

	int scaleXCtr, scaleYCtr = args.scaleYoff;
	const byte *inBase;

	if (!doscale && (args.flipping & FLIP_H)) args.ino -= 4 * 3;

	for (uint32 i = 0; i < args.height; i++) {
		if (doscale) {
			inBase = args.ino + scaleYCtr / BlendBlit::SCALE_THRESHOLD * args.inoStep;
			scaleXCtr = args.scaleXoff;
		} else {
			in = args.ino;
		}
		out = args.outo;

		uint32 j = 0;
		for (; j + 4 <= args.width; j += 4) {
			__m128i dstPixels, srcPixels;
			if (loaddst) dstPixels = _mm_loadu_si128((const __m128i *)out);
			if (!doscale) {
				srcPixels = _mm_loadu_si128((const __m128i *)in);
			} else {
				srcPixels = _mm_setr_epi32(
					*(const uint32 *)(inBase + (ptrdiff_t)(scaleXCtr + args.scaleX * 0) / (ptrdiff_t)BlendBlit::SCALE_THRESHOLD * args.inStep),
					*(const uint32 *)(inBase + (ptrdiff_t)(scaleXCtr + args.scaleX * 1) / (ptrdiff_t)BlendBlit::SCALE_THRESHOLD * args.inStep),
					*(const uint32 *)(inBase + (ptrdiff_t)(scaleXCtr + args.scaleX * 2) / (ptrdiff_t)BlendBlit::SCALE_THRESHOLD * args.inStep),
					*(const uint32 *)(inBase + (ptrdiff_t)(scaleXCtr + args.scaleX * 3) / (ptrdiff_t)BlendBlit::SCALE_THRESHOLD * args.inStep)
				);
				scaleXCtr += args.scaleX * 4;
			}
			if (!doscale && (args.flipping & FLIP_H)) {
				srcPixels = _mm_shuffle_epi32(srcPixels, _MM_SHUFFLE(0, 1, 2, 3));
			}
			{
				const __m128i res = pixelFunc.simd(srcPixels, dstPixels);
				_mm_storeu_si128((__m128i *)out, res);
			}
			if (!doscale) in += (ptrdiff_t)args.inStep * 4;
			out += 4ULL * 4;
		}
		if (!doscale && (args.flipping & FLIP_H)) in += 4 * 3;
		for (; j < args.width; j++) {
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

}; // End of class BlendBlitImpl_SSE2

void BlendBlit::blitSSE2(Args &args, const TSpriteBlendMode &blendMode, const AlphaType &alphaType) {
	blitT<BlendBlitImpl_SSE2>(args, blendMode, alphaType);
}

} // End of namespace Graphics

#if !defined(__x86_64__)

#if defined(__clang__)
#pragma clang attribute pop
#elif defined(__GNUC__)
#pragma GCC pop_options
#endif

#endif // !defined(__x86_64__)
