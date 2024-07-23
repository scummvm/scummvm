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

#include <immintrin.h>

#if defined(__clang__)
#pragma clang attribute push (__attribute__((target("avx2"))), apply_to=function)
#elif defined(__GNUC__)
#pragma GCC push_options
#pragma GCC target("avx2")
#endif

namespace Graphics {

class BlendBlitImpl_AVX2 : public BlendBlitImpl_Base {
	friend class BlendBlit;

template<bool rgbmod, bool alphamod>
struct AlphaBlend : public BlendBlitImpl_Base::AlphaBlend<rgbmod, alphamod> {
public:
	constexpr AlphaBlend(const uint32 color) : BlendBlitImpl_Base::AlphaBlend<rgbmod, alphamod>(color) {}

	inline __m256i simd(__m256i src, __m256i dst) const {
		__m256i ina;
		if (alphamod)
			ina = _mm256_srli_epi32(_mm256_mullo_epi16(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask)), _mm256_set1_epi32(this->ca)), 8);
		else
			ina = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
		__m256i alphaMask = _mm256_cmpeq_epi32(ina, _mm256_setzero_si256());
	
		if (rgbmod) {
			__m256i dstR = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
			__m256i dstG = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m256i dstB = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m256i srcR = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
			__m256i srcG = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m256i srcB = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);

			dstR = _mm256_slli_epi32(_mm256_mullo_epi16(dstR, _mm256_sub_epi32(_mm256_set1_epi32(255), ina)), BlendBlit::kRModShift - 8);
			dstG = _mm256_slli_epi32(_mm256_mullo_epi16(dstG, _mm256_sub_epi32(_mm256_set1_epi32(255), ina)), BlendBlit::kGModShift - 8);
			dstB = _mm256_mullo_epi16(dstB, _mm256_sub_epi32(_mm256_set1_epi32(255), ina));
			srcR = _mm256_add_epi32(dstR, _mm256_slli_epi32(_mm256_mullo_epi16(_mm256_srli_epi32(_mm256_mullo_epi16(srcR, ina), 8), _mm256_set1_epi32(this->cr)), BlendBlit::kRModShift - 8));
			srcG = _mm256_add_epi32(dstG, _mm256_slli_epi32(_mm256_mullo_epi16(_mm256_srli_epi32(_mm256_mullo_epi16(srcG, ina), 8), _mm256_set1_epi32(this->cg)), BlendBlit::kGModShift - 8));
			srcB = _mm256_add_epi32(dstB, _mm256_mullo_epi16(_mm256_srli_epi32(_mm256_mullo_epi16(srcB, ina), 8), _mm256_set1_epi32(this->cb)));
			src = _mm256_or_si256(_mm256_and_si256(srcB, _mm256_set1_epi32(BlendBlit::kBModMask)), _mm256_set1_epi32(BlendBlit::kAModMask));
			src = _mm256_or_si256(_mm256_and_si256(srcG, _mm256_set1_epi32(BlendBlit::kGModMask)), src);
			src = _mm256_or_si256(_mm256_and_si256(srcR, _mm256_set1_epi32(BlendBlit::kRModMask)), src);
		} else {
			__m256i dstRB = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m256i srcRB = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m256i dstG = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m256i srcG = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);

			dstRB = _mm256_srli_epi32(_mm256_mullo_epi32(dstRB, _mm256_sub_epi32(_mm256_set1_epi32(255), ina)), 8);
			dstG = _mm256_srli_epi32(_mm256_mullo_epi16(dstG, _mm256_sub_epi32(_mm256_set1_epi32(255), ina)), 8);
			srcRB = _mm256_slli_epi32(_mm256_add_epi32(dstRB, _mm256_srli_epi32(_mm256_mullo_epi32(srcRB, ina), 8)), BlendBlit::kBModShift);
			srcG = _mm256_slli_epi32(_mm256_add_epi32(dstG, _mm256_srli_epi32(_mm256_mullo_epi16(srcG, ina), 8)), BlendBlit::kGModShift);
			src = _mm256_or_si256(_mm256_and_si256(srcG, _mm256_set1_epi32(BlendBlit::kGModMask)), _mm256_set1_epi32(BlendBlit::kAModMask));
			src = _mm256_or_si256(_mm256_and_si256(srcRB, _mm256_set1_epi32(BlendBlit::kBModMask | BlendBlit::kRModMask)), src);
		}

		dst = _mm256_and_si256(alphaMask, dst);
		src = _mm256_andnot_si256(alphaMask, src);
		return _mm256_or_si256(dst, src);
	}
};

template<bool rgbmod, bool alphamod>
struct MultiplyBlend : public BlendBlitImpl_Base::MultiplyBlend<rgbmod, alphamod> {
public:
	constexpr MultiplyBlend(const uint32 color) : BlendBlitImpl_Base::MultiplyBlend<rgbmod, alphamod>(color) {}

	inline __m256i simd(__m256i src, __m256i dst) const {
		__m256i ina, alphaMask;
		if (alphamod) {
			ina = _mm256_srli_epi32(_mm256_mullo_epi16(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask)), _mm256_set1_epi32(this->ca)), 8);
			alphaMask = _mm256_cmpeq_epi32(ina, _mm256_setzero_si256());
		} else {
			ina = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
			alphaMask = _mm256_set1_epi32(BlendBlit::kAModMask);
		}

		if (rgbmod) {
			__m256i srcB = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m256i srcG = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m256i srcR = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
			__m256i dstB = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m256i dstG = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m256i dstR = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);

			srcB = _mm256_and_si256(_mm256_slli_epi32(_mm256_mullo_epi32(dstB, _mm256_srli_epi32(_mm256_mullo_epi32(_mm256_mullo_epi16(srcB, _mm256_set1_epi32(this->cb)), ina), 16)), BlendBlit::kBModShift - 8), _mm256_set1_epi32(BlendBlit::kBModMask));
			srcG = _mm256_and_si256(_mm256_slli_epi32(_mm256_mullo_epi32(dstG, _mm256_srli_epi32(_mm256_mullo_epi32(_mm256_mullo_epi16(srcG, _mm256_set1_epi32(this->cg)), ina), 16)), BlendBlit::kGModShift - 8), _mm256_set1_epi32(BlendBlit::kGModMask));
			srcR = _mm256_and_si256(_mm256_slli_epi32(_mm256_mullo_epi32(dstR, _mm256_srli_epi32(_mm256_mullo_epi32(_mm256_mullo_epi16(srcR, _mm256_set1_epi32(this->cr)), ina), 16)), BlendBlit::kRModShift - 8), _mm256_set1_epi32(BlendBlit::kRModMask));

			src = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
			src = _mm256_or_si256(src, _mm256_or_si256(srcB, _mm256_or_si256(srcG, srcR)));
		} else {
			constexpr uint32 rbMask = BlendBlit::kRModMask | BlendBlit::kBModMask;
			__m256i dstRB = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m256i srcRB = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m256i dstG = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m256i srcG = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);

			srcG = _mm256_and_si256(_mm256_slli_epi32(_mm256_mullo_epi16(dstG, _mm256_srli_epi32(_mm256_mullo_epi16(srcG, ina), 8)), 8), _mm256_set1_epi32(BlendBlit::kGModMask));
			srcRB = _mm256_and_si256(_mm256_mullo_epi16(dstRB, _mm256_srli_epi32(_mm256_and_si256(_mm256_mullo_epi32(srcRB, ina), _mm256_set1_epi32(rbMask)), 8)), _mm256_set1_epi32(rbMask));
			
			src = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
			src = _mm256_or_si256(src, _mm256_or_si256(srcRB, srcG));
		}

		dst = _mm256_and_si256(alphaMask, dst);
		src = _mm256_andnot_si256(alphaMask, src);
		return _mm256_or_si256(dst, src);
	}
};

template<bool rgbmod, bool alphamod>
struct OpaqueBlend : public BlendBlitImpl_Base::OpaqueBlend<rgbmod, alphamod> {
public:
	constexpr OpaqueBlend(const uint32 color) : BlendBlitImpl_Base::OpaqueBlend<rgbmod, alphamod>(color) {}

	inline __m256i simd(__m256i src, __m256i dst) const {
		return _mm256_or_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
	}
};

template<bool rgbmod, bool alphamod>
struct BinaryBlend : public BlendBlitImpl_Base::BinaryBlend<rgbmod, alphamod> {
public:
	constexpr BinaryBlend(const uint32 color) : BlendBlitImpl_Base::BinaryBlend<rgbmod, alphamod>(color) {}

	inline __m256i simd(__m256i src, __m256i dst) const {
		__m256i alphaMask = _mm256_cmpeq_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask)), _mm256_setzero_si256());
		dst = _mm256_and_si256(dst, alphaMask);
		src = _mm256_andnot_si256(alphaMask, _mm256_or_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask)));
		return _mm256_or_si256(src, dst);
	}
};

template<bool rgbmod, bool alphamod>
struct AdditiveBlend : public BlendBlitImpl_Base::AdditiveBlend<rgbmod, alphamod> {
public:
	constexpr AdditiveBlend(const uint32 color) : BlendBlitImpl_Base::AdditiveBlend<rgbmod, alphamod>(color) {}

	inline __m256i simd(__m256i src, __m256i dst) const {
		__m256i ina;
		if (alphamod)
			ina = _mm256_srli_epi32(_mm256_mullo_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask)), _mm256_set1_epi32(this->ca)), 8);
		else
			ina = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
		__m256i alphaMask = _mm256_cmpeq_epi32(ina, _mm256_set1_epi32(0));

		if (rgbmod) {
			__m256i srcb = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kBModMask));
			__m256i srcg = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m256i srcr = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
			__m256i dstb = _mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kBModMask));
			__m256i dstg = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m256i dstr = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);

			srcb = _mm256_and_si256(_mm256_add_epi32(dstb, _mm256_srli_epi32(_mm256_mullo_epi32(srcb, _mm256_mullo_epi32(_mm256_set1_epi32(this->cb), ina)), 16)), _mm256_set1_epi32(BlendBlit::kBModMask));
			srcg = _mm256_and_si256(_mm256_add_epi32(dstg, _mm256_mullo_epi32(srcg, _mm256_mullo_epi32(_mm256_set1_epi32(this->cg), ina))), _mm256_set1_epi32(BlendBlit::kGModMask));
			srcr = _mm256_and_si256(_mm256_add_epi32(dstr, _mm256_srli_epi32(_mm256_mullo_epi32(srcr, _mm256_mullo_epi32(_mm256_set1_epi32(this->cr), ina)), BlendBlit::kRModShift - 16)), _mm256_set1_epi32(BlendBlit::kRModMask));

			src = _mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kAModMask));
			src = _mm256_or_si256(src, _mm256_or_si256(srcb, _mm256_or_si256(srcg, srcb)));
		} else if (alphamod) {
			__m256i srcg = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kGModMask));
			__m256i srcrb = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m256i dstg = _mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kGModMask));
			__m256i dstrb = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);

			srcg = _mm256_and_si256(_mm256_add_epi32(dstg, _mm256_srli_epi32(_mm256_mullo_epi32(srcg, ina), 8)), _mm256_set1_epi32(BlendBlit::kGModMask));
			srcrb = _mm256_and_si256(_mm256_add_epi32(dstrb, _mm256_mullo_epi32(srcrb, ina)), _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask));

			src = _mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kAModMask));
			src = _mm256_or_si256(src, _mm256_or_si256(srcrb, srcg));
		} else {
			__m256i srcg = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kGModMask));
			__m256i srcrb = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m256i dstg = _mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kGModMask));
			__m256i dstrb = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);

			srcg = _mm256_and_si256(_mm256_add_epi32(dstg, srcg), _mm256_set1_epi32(BlendBlit::kGModMask));
			srcrb = _mm256_and_si256(_mm256_slli_epi32(_mm256_add_epi32(dstrb, srcrb), 8), _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask));

			src = _mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kAModMask));
			src = _mm256_or_si256(src, _mm256_or_si256(srcrb, srcg));
		}

		dst = _mm256_and_si256(alphaMask, dst);
		src = _mm256_andnot_si256(alphaMask, src);
		return _mm256_or_si256(dst, src);
	}
};

template<bool rgbmod, bool alphamod>
struct SubtractiveBlend : public BlendBlitImpl_Base::SubtractiveBlend<rgbmod, alphamod> {
public:
	constexpr SubtractiveBlend(const uint32 color) : BlendBlitImpl_Base::SubtractiveBlend<rgbmod, alphamod>(color) {}

	inline __m256i simd(__m256i src, __m256i dst) const {
		__m256i ina = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
		__m256i srcb = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
		__m256i srcg = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
		__m256i srcr = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
		__m256i dstb = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
		__m256i dstg = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
		__m256i dstr = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);

		srcb = _mm256_and_si256(_mm256_slli_epi32(_mm256_max_epi16(_mm256_sub_epi32(dstb, _mm256_srli_epi32(_mm256_mullo_epi32(_mm256_mullo_epi32(srcb, _mm256_set1_epi32(this->cb)), _mm256_mullo_epi32(dstb, ina)), 24)), _mm256_set1_epi32(0)), BlendBlit::kBModShift), _mm256_set1_epi32(BlendBlit::kBModMask));
		srcg = _mm256_and_si256(_mm256_slli_epi32(_mm256_max_epi16(_mm256_sub_epi32(dstg, _mm256_srli_epi32(_mm256_mullo_epi32(_mm256_mullo_epi32(srcg, _mm256_set1_epi32(this->cg)), _mm256_mullo_epi32(dstg, ina)), 24)), _mm256_set1_epi32(0)), BlendBlit::kGModShift), _mm256_set1_epi32(BlendBlit::kGModMask));
		srcr = _mm256_and_si256(_mm256_slli_epi32(_mm256_max_epi16(_mm256_sub_epi32(dstr, _mm256_srli_epi32(_mm256_mullo_epi32(_mm256_mullo_epi32(srcr, _mm256_set1_epi32(this->cr)), _mm256_mullo_epi32(dstr, ina)), 24)), _mm256_set1_epi32(0)), BlendBlit::kRModShift), _mm256_set1_epi32(BlendBlit::kRModMask));

		return _mm256_or_si256(_mm256_set1_epi32(BlendBlit::kAModMask), _mm256_or_si256(srcb, _mm256_or_si256(srcg, srcr)));
	}
};

public:
template<template <bool RGBMOD, bool ALPHAMOD> class PixelFunc, bool doscale, bool rgbmod, bool alphamod>
static void blitInnerLoop(BlendBlit::Args &args) {
	const bool loaddst = true; // TODO: Only set this when necessary

	const byte *in;
	byte *out;

	const PixelFunc<rgbmod, alphamod> pixelFunc(args.color);

	int scaleXCtr, scaleYCtr = args.scaleYoff;
	const byte *inBase;

	if (!doscale && (args.flipping & FLIP_H)) args.ino -= 4 * 7;

	for (uint32 i = 0; i < args.height; i++) {
		if (doscale) {
			inBase = args.ino + scaleYCtr / BlendBlit::SCALE_THRESHOLD * args.inoStep;
			scaleXCtr = args.scaleXoff;
		} else {
			in = args.ino;
		}
		out = args.outo;

		uint32 j = 0;
		for (; j + 8 <= args.width; j += 8) {
			__m256i dstPixels, srcPixels;
			if (loaddst) dstPixels = _mm256_loadu_si256((const __m256i *)out);
			if (!doscale) {
				srcPixels = _mm256_loadu_si256((const __m256i *)in);
			} else {
				srcPixels = _mm256_setr_epi32(
					*(const uint32 *)(inBase + (ptrdiff_t)(scaleXCtr + args.scaleX * 0) / (ptrdiff_t)BlendBlit::SCALE_THRESHOLD * args.inStep),
					*(const uint32 *)(inBase + (ptrdiff_t)(scaleXCtr + args.scaleX * 1) / (ptrdiff_t)BlendBlit::SCALE_THRESHOLD * args.inStep),
					*(const uint32 *)(inBase + (ptrdiff_t)(scaleXCtr + args.scaleX * 2) / (ptrdiff_t)BlendBlit::SCALE_THRESHOLD * args.inStep),
					*(const uint32 *)(inBase + (ptrdiff_t)(scaleXCtr + args.scaleX * 3) / (ptrdiff_t)BlendBlit::SCALE_THRESHOLD * args.inStep),
					*(const uint32 *)(inBase + (ptrdiff_t)(scaleXCtr + args.scaleX * 4) / (ptrdiff_t)BlendBlit::SCALE_THRESHOLD * args.inStep),
					*(const uint32 *)(inBase + (ptrdiff_t)(scaleXCtr + args.scaleX * 5) / (ptrdiff_t)BlendBlit::SCALE_THRESHOLD * args.inStep),
					*(const uint32 *)(inBase + (ptrdiff_t)(scaleXCtr + args.scaleX * 6) / (ptrdiff_t)BlendBlit::SCALE_THRESHOLD * args.inStep),
					*(const uint32 *)(inBase + (ptrdiff_t)(scaleXCtr + args.scaleX * 7) / (ptrdiff_t)BlendBlit::SCALE_THRESHOLD * args.inStep)
				);
				scaleXCtr += args.scaleX * 8;
			}
			if (!doscale && (args.flipping & FLIP_H)) {
				srcPixels = _mm256_shuffle_epi32(srcPixels, _MM_SHUFFLE(0, 1, 2, 3));
				srcPixels = _mm256_permute2x128_si256(srcPixels, srcPixels, 0x01);
			}
			{
				const __m256i res = pixelFunc.simd(srcPixels, dstPixels);
				_mm256_storeu_si256((__m256i *)out, res);
			}
			if (!doscale) in += (ptrdiff_t)args.inStep * 8;
			out += 4ULL * 8;
		}
		if (!doscale && (args.flipping & FLIP_H)) in += 4 * 7;
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

}; // end of class BlendBlitImpl_AVX2

void BlendBlit::blitAVX2(Args &args, const TSpriteBlendMode &blendMode, const AlphaType &alphaType) {
	blitT<BlendBlitImpl_AVX2>(args, blendMode, alphaType);
}

} // End of namespace Graphics

#if defined(__clang__)
#pragma clang attribute pop
#elif defined(__GNUC__)
#pragma GCC pop_options
#endif
