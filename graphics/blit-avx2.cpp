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

#ifdef SCUMMVM_AVX2
#include <immintrin.h>

#include "graphics/blit.h"
#include "graphics/pixelformat.h"

namespace Graphics {

template<bool doscale, bool rgbmod, bool alphamod>
struct AlphaBlendAVX2 {
	static inline __m256i simd(__m256i src, __m256i dst, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
	    __m256i ina;
	    if (alphamod)
			ina = _mm256_srli_epi32(_mm256_mullo_epi16(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask)), _mm256_set1_epi32(ca)), 8);
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
			srcR = _mm256_add_epi32(dstR, _mm256_slli_epi32(_mm256_mullo_epi16(_mm256_srli_epi32(_mm256_mullo_epi16(srcR, ina), 8), _mm256_set1_epi32(cr)), BlendBlit::kRModShift - 8));
			srcG = _mm256_add_epi32(dstG, _mm256_slli_epi32(_mm256_mullo_epi16(_mm256_srli_epi32(_mm256_mullo_epi16(srcG, ina), 8), _mm256_set1_epi32(cg)), BlendBlit::kGModShift - 8));
			srcB = _mm256_add_epi32(dstB, _mm256_mullo_epi16(_mm256_srli_epi32(_mm256_mullo_epi16(srcB, ina), 8), _mm256_set1_epi32(cb)));
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

	static inline void normal(const byte *in, byte *out, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
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

template<bool doscale, bool rgbmod, bool alphamod>
struct MultiplyBlendAVX2 {
	static inline __m256i simd(__m256i src, __m256i dst, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
    	__m256i ina;
	    if (alphamod)
			ina = _mm256_srli_epi32(_mm256_mullo_epi16(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask)), _mm256_set1_epi32(ca)), 8);
	    else
			ina = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
	    __m256i alphaMask = _mm256_cmpeq_epi32(ina, _mm256_setzero_si256());

    	if (rgbmod) {
			__m256i srcb = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m256i srcg = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m256i srcr = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
			__m256i dstb = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m256i dstg = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			__m256i dstr = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);

			srcb = _mm256_and_si256(_mm256_srli_epi32(_mm256_mullo_epi32(dstb, _mm256_srli_epi32(_mm256_mullo_epi32(_mm256_mullo_epi16(srcb, _mm256_set1_epi32(cb)), ina), 16)), BlendBlit::kBModShift - 8), _mm256_set1_epi32(BlendBlit::kBModMask));
			srcg = _mm256_and_si256(_mm256_srli_epi32(_mm256_mullo_epi32(dstg, _mm256_srli_epi32(_mm256_mullo_epi32(_mm256_mullo_epi16(srcg, _mm256_set1_epi32(cg)), ina), 16)), BlendBlit::kGModShift - 8), _mm256_set1_epi32(BlendBlit::kGModMask));
			srcr = _mm256_and_si256(_mm256_srli_epi32(_mm256_mullo_epi32(dstr, _mm256_srli_epi32(_mm256_mullo_epi32(_mm256_mullo_epi16(srcr, _mm256_set1_epi32(cr)), ina), 16)), BlendBlit::kRModShift - 8), _mm256_set1_epi32(BlendBlit::kRModMask));

			src = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
			src = _mm256_or_si256(src, _mm256_or_si256(srcb, _mm256_or_si256(srcg, srcr)));
    	} else {
			__m256i srcg = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kGModMask));
			__m256i srcrb = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
			__m256i dstg = _mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kGModMask));
			__m256i dstrb = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
    	    srcg = _mm256_and_si256(_mm256_srli_epi32(_mm256_mullo_epi32(dstg, _mm256_srli_epi32(_mm256_mullo_epi32(srcg, ina), 8)), 8), _mm256_set1_epi32(BlendBlit::kGModMask));
    	    srcrb = _mm256_and_si256(_mm256_mullo_epi32(dstrb, _mm256_srli_epi32(_mm256_mullo_epi32(srcrb, ina), 8)), _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask));
    	    src = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
    	    src = _mm256_or_si256(src, _mm256_or_si256(srcrb, srcg));
    	}

    	dst = _mm256_and_si256(alphaMask, dst);
    	src = _mm256_andnot_si256(alphaMask, src);
    	return _mm256_or_si256(dst, src);
	}

	static inline void normal(const byte *in, byte *out, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
		uint32 ina = in[BlendBlit::kAIndex] * ca >> 8;

		if (ina != 0) {
			out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] * ((in[BlendBlit::kBIndex] * cb * ina) >> 16) >> 8;
			out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] * ((in[BlendBlit::kGIndex] * cg * ina) >> 16) >> 8;
			out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] * ((in[BlendBlit::kRIndex] * cr * ina) >> 16) >> 8;
		}
	}
};

template<bool doscale, bool rgbmod, bool alphamod>
struct OpaqueBlendAVX2 {
	static inline __m256i simd(__m256i src, __m256i dst, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
		return _mm256_or_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
	}

	static inline void normal(const byte *in, byte *out, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
		*(uint32 *)out = *(const uint32 *)in | BlendBlit::kAModMask;
	}
};

template<bool doscale, bool rgbmod, bool alphamod>
struct BinaryBlendAVX2 {
	static inline __m256i simd(__m256i src, __m256i dst, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
		__m256i alphaMask = _mm256_cmpeq_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask)), _mm256_setzero_si256());
		dst = _mm256_and_si256(dst, alphaMask);
		src = _mm256_andnot_si256(alphaMask, _mm256_or_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask)));
		return _mm256_or_si256(src, dst);
	}

	static inline void normal(const byte *in, byte *out, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
		uint32 pix = *(const uint32 *)in;
		int a = in[BlendBlit::kAIndex];

		if (a != 0) {   // Full opacity (Any value not exactly 0 is Opaque here)
			*(uint32 *)out = pix;
			out[BlendBlit::kAIndex] = 0xFF;
		}
	}
};

template<bool doscale, bool rgbmod, bool alphamod>
struct AdditiveBlendAVX2 {
	static inline __m256i simd(__m256i src, __m256i dst, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
    	__m256i ina;
    	if (alphamod)
    	    ina = _mm256_srli_epi32(_mm256_mullo_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask)), _mm256_set1_epi32(ca)), 8);
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

			srcb = _mm256_and_si256(_mm256_add_epi32(dstb, _mm256_srli_epi32(_mm256_mullo_epi32(srcb, _mm256_mullo_epi32(_mm256_set1_epi32(cb), ina)), 16)), _mm256_set1_epi32(BlendBlit::kBModMask));
			srcg = _mm256_and_si256(_mm256_add_epi32(dstg, _mm256_mullo_epi32(srcg, _mm256_mullo_epi32(_mm256_set1_epi32(cg), ina))), _mm256_set1_epi32(BlendBlit::kGModMask));
			srcr = _mm256_and_si256(_mm256_add_epi32(dstr, _mm256_srli_epi32(_mm256_mullo_epi32(srcr, _mm256_mullo_epi32(_mm256_set1_epi32(cr), ina)), BlendBlit::kRModShift - 16)), _mm256_set1_epi32(BlendBlit::kRModMask));

    	    src = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
    	    src = _mm256_or_si256(src, _mm256_or_si256(srcb, _mm256_or_si256(srcg, srcb)));
    	} else if (alphamod) {
    	    __m256i srcg = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kGModMask));
    	    __m256i srcrb = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
    	    __m256i dstg = _mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kGModMask));
    	    __m256i dstrb = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);

			srcg = _mm256_and_si256(_mm256_add_epi32(dstg, _mm256_srli_epi32(_mm256_mullo_epi32(srcg, ina), 8)), _mm256_set1_epi32(BlendBlit::kGModMask));
			srcrb = _mm256_and_si256(_mm256_add_epi32(dstrb, _mm256_mullo_epi32(srcrb, ina)), _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask));

    	    src = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
    	    src = _mm256_or_si256(src, _mm256_or_si256(srcrb, srcg));
    	} else {
    	    __m256i srcg = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kGModMask));
    	    __m256i srcrb = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
    	    __m256i dstg = _mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kGModMask));
    	    __m256i dstrb = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);

			srcg = _mm256_and_si256(_mm256_add_epi32(dstg, srcg), _mm256_set1_epi32(BlendBlit::kGModMask));
			srcrb = _mm256_and_si256(_mm256_slli_epi32(_mm256_add_epi32(dstrb, srcrb), 8), _mm256_set1_epi32(BlendBlit::kRModMask | BlendBlit::kBModMask));

    	    src = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
    	    src = _mm256_or_si256(src, _mm256_or_si256(srcrb, srcg));
		}

    	dst = _mm256_and_si256(alphaMask, dst);
    	src = _mm256_andnot_si256(alphaMask, src);
    	return _mm256_or_si256(dst, src);
	}

	static inline void normal(const byte *in, byte *out, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
		uint32 ina = in[BlendBlit::kAIndex] * ca >> 8;

		if (ina != 0) {
			out[BlendBlit::kBIndex] = out[BlendBlit::kBIndex] + ((in[BlendBlit::kBIndex] * cb * ina) >> 16);
			out[BlendBlit::kGIndex] = out[BlendBlit::kGIndex] + ((in[BlendBlit::kGIndex] * cg * ina) >> 16);
			out[BlendBlit::kRIndex] = out[BlendBlit::kRIndex] + ((in[BlendBlit::kRIndex] * cr * ina) >> 16);
		}
	}
};

template<bool doscale, bool rgbmod, bool alphamod>
struct SubtractiveBlendAVX2 {
	static inline __m256i simd(__m256i src, __m256i dst, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
    	__m256i ina = _mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kAModMask));
    	__m256i srcb = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
    	__m256i srcg = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
    	__m256i srcr = _mm256_srli_epi32(_mm256_and_si256(src, _mm256_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
    	__m256i dstb = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
    	__m256i dstg = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
    	__m256i dstr = _mm256_srli_epi32(_mm256_and_si256(dst, _mm256_set1_epi32(BlendBlit::kRModMask)), BlendBlit::kRModShift);

		srcb = _mm256_and_si256(_mm256_slli_epi32(_mm256_max_epi16(_mm256_sub_epi32(dstb, _mm256_srli_epi32(_mm256_mullo_epi32(_mm256_mullo_epi32(srcb, _mm256_set1_epi32(cb)), _mm256_mullo_epi32(dstb, ina)), 24)), _mm256_set1_epi32(0)), BlendBlit::kBModShift), _mm256_set1_epi32(BlendBlit::kBModMask));
		srcg = _mm256_and_si256(_mm256_slli_epi32(_mm256_max_epi16(_mm256_sub_epi32(dstg, _mm256_srli_epi32(_mm256_mullo_epi32(_mm256_mullo_epi32(srcg, _mm256_set1_epi32(cg)), _mm256_mullo_epi32(dstg, ina)), 24)), _mm256_set1_epi32(0)), BlendBlit::kGModShift), _mm256_set1_epi32(BlendBlit::kGModMask));
		srcr = _mm256_and_si256(_mm256_slli_epi32(_mm256_max_epi16(_mm256_sub_epi32(dstr, _mm256_srli_epi32(_mm256_mullo_epi32(_mm256_mullo_epi32(srcr, _mm256_set1_epi32(cr)), _mm256_mullo_epi32(dstr, ina)), 24)), _mm256_set1_epi32(0)), BlendBlit::kRModShift), _mm256_set1_epi32(BlendBlit::kRModMask));

    	return _mm256_or_si256(_mm256_set1_epi32(BlendBlit::kAModMask), _mm256_or_si256(srcb, _mm256_or_si256(srcg, srcr)));
	}

	static inline void normal(const byte *in, byte *out, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
		out[BlendBlit::kAIndex] = 255;
		out[BlendBlit::kBIndex] = MAX<int32>(out[BlendBlit::kBIndex] - ((in[BlendBlit::kBIndex] * cb  * (out[BlendBlit::kBIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);
		out[BlendBlit::kGIndex] = MAX<int32>(out[BlendBlit::kGIndex] - ((in[BlendBlit::kGIndex] * cg  * (out[BlendBlit::kGIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);
		out[BlendBlit::kRIndex] = MAX<int32>(out[BlendBlit::kRIndex] - ((in[BlendBlit::kRIndex] * cr *  (out[BlendBlit::kRIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);
	}
};

template<template <bool DOSCALE, bool RGBMOD, bool ALPHAMOD> class PixelFunc, bool doscale, bool rgbmod, bool alphamod, bool coloradd1, bool loaddst>
void BlendBlitImpl::blitInnerLoopAVX2(BlendBlit::Args &args) {
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
				const __m256i res = PixelFunc<doscale, rgbmod, alphamod>::simd(srcPixels, dstPixels, args.flipping & FLIP_H, ca, cr, cg, cb);
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

			PixelFunc<doscale, rgbmod, alphamod>::normal(in, out, args.flipping & FLIP_H, ca, cr, cg, cb);
            
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
void BlendBlit::doBlitAlphaBlendLogicAVX2(Args &args) {
	BlendBlitImpl::blitInnerLoopAVX2<AlphaBlendAVX2, doscale, rgbmod, alphamod, false, true>(args);
}
template<bool doscale, bool rgbmod>
void BlendBlit::doBlitSubtractiveBlendLogicAVX2(Args &args) {
	BlendBlitImpl::blitInnerLoopAVX2<SubtractiveBlendAVX2, doscale, rgbmod, false, false, true>(args);
}
template<bool doscale, bool rgbmod, bool alphamod>
void BlendBlit::doBlitAdditiveBlendLogicAVX2(Args &args) {
	BlendBlitImpl::blitInnerLoopAVX2<AdditiveBlendAVX2, doscale, rgbmod, alphamod, false, true>(args);
}
template<bool doscale>
void BlendBlit::doBlitOpaqueBlendLogicAVX2(Args &args) {
	BlendBlitImpl::blitInnerLoopAVX2<OpaqueBlendAVX2, doscale, false, false, false, true>(args);
}
template<bool doscale>
void BlendBlit::doBlitBinaryBlendLogicAVX2(Args &args) {
	BlendBlitImpl::blitInnerLoopAVX2<BinaryBlendAVX2, doscale, false, false, false, true>(args);
}
template<bool doscale, bool rgbmod, bool alphamod>
void BlendBlit::doBlitMultiplyBlendLogicAVX2(Args &args) {
	BlendBlitImpl::blitInnerLoopAVX2<MultiplyBlendAVX2, doscale, rgbmod, alphamod, false, true>(args);
}

} // End of namespace Graphics

#endif // SCUMMVM_AVX2
