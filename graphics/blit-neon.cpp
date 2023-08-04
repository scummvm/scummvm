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

#if defined(__ARM_NEON__) || defined(__ARM_NEON)
#include <arm_neon.h>

#include "graphics/blit.h"
#include "graphics/pixelformat.h"

namespace Graphics {

template<bool doscale, bool rgbmod, bool alphamod>
struct AlphaBlend {
	static inline uint32x4_t simd(uint32x4_t src, uint32x4_t dst, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
	    uint32x4_t ina;
	    if (alphamod)
	        ina = vshrq_n_u32(vmulq_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask)), vdupq_n_u32(ca)), 8);
	    else
	        ina = vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
	    uint32x4_t alphaMask = vceqq_u32(ina, vmovq_n_u32(0));
	
	    if (rgbmod) {
	        uint32x4_t dstR = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kRModMask)), 16);
	        uint32x4_t srcR = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kRModMask)), 16);
	        uint32x4_t dstG = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kGModMask)), 8);
	        uint32x4_t srcG = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kGModMask)), 8);
	        uint32x4_t dstB = vandq_u32(dst, vmovq_n_u32(BlendBlit::kBModMask));
	        uint32x4_t srcB = vandq_u32(src, vmovq_n_u32(BlendBlit::kBModMask));
	
	        dstR = vshrq_n_u32(vmulq_u32(dstR, vsubq_u32(vmovq_n_u32(255), ina)), 8);
	        dstG = vshrq_n_u32(vmulq_u32(dstG, vsubq_u32(vmovq_n_u32(255), ina)), 8);
	        dstB = vshrq_n_u32(vmulq_u32(dstB, vsubq_u32(vmovq_n_u32(255), ina)), 8);
	        srcR = vaddq_u32(dstR, vshrq_n_u32(vmulq_u32(vmulq_u32(srcR, ina), vmovq_n_u32(cr)), 16));
	        srcG = vaddq_u32(dstG, vshrq_n_u32(vmulq_u32(vmulq_u32(srcG, ina), vmovq_n_u32(cg)), 16));
	        srcB = vaddq_u32(dstB, vshrq_n_u32(vmulq_u32(vmulq_u32(srcB, ina), vmovq_n_u32(cb)), 16));
	        src = vorrq_u32(vandq_u32(srcB, vmovq_n_u32(BlendBlit::kBModMask)), vmovq_n_u32(BlendBlit::kAModMask));
	        src = vorrq_u32(vandq_u32(vshlq_n_u32(srcG, 8), vmovq_n_u32(BlendBlit::kGModMask)), src);
	        src = vorrq_u32(vandq_u32(vshlq_n_u32(srcR, 16), vmovq_n_u32(BlendBlit::kRModMask)), src);
	    } else {
	        uint32x4_t dstRB = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask)), 8);
	        uint32x4_t srcRB = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask)), 8);
	        uint32x4_t dstG = vandq_u32(dst, vmovq_n_u32(BlendBlit::kGModMask));
	        uint32x4_t srcG = vandq_u32(src, vmovq_n_u32(BlendBlit::kGModMask));
	
	        dstRB = vmulq_u32(dstRB, vsubq_u32(vmovq_n_u32(255), ina));
	        dstG = vshrq_n_u32(vmulq_u32(dstG, vsubq_u32(vmovq_n_u32(255), ina)), 8);
	        srcRB = vaddq_u32(dstRB, vmulq_u32(srcRB, ina));
	        srcG = vaddq_u32(dstG, vshrq_n_u32(vmulq_u32(srcG, ina), 8));
	        src = vorrq_u32(vandq_u32(srcG, vmovq_n_u32(BlendBlit::kGModMask)), vmovq_n_u32(BlendBlit::kAModMask));
	        src = vorrq_u32(vandq_u32(srcRB, vmovq_n_u32(BlendBlit::kBModMask | BlendBlit::kRModMask)), src);
	    }
	
	    dst = vandq_u32(alphaMask, dst);
	    src = vandq_u32(vmvnq_u32(alphaMask), src);
	    return vorrq_u32(dst, src);
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
struct MultiplyBlend {
	static inline uint32x4_t simd(uint32x4_t src, uint32x4_t dst, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
    	uint32x4_t ina;
    	if (alphamod)
    	    ina = vshrq_n_u32(vmulq_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask)), vdupq_n_u32(ca)), 8);
    	else
    	    ina = vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
    	uint32x4_t alphaMask = vceqq_u32(ina, vmovq_n_u32(0));

    	if (rgbmod) {
    	    uint32x4_t srcb = vandq_u32(src, vmovq_n_u32(BlendBlit::kBModMask));
    	    uint32x4_t srcg = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
    	    uint32x4_t srcr = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
    	    uint32x4_t dstb = vandq_u32(dst, vmovq_n_u32(BlendBlit::kBModMask));
    	    uint32x4_t dstg = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
    	    uint32x4_t dstr = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kRModMask)), BlendBlit::kRModShift);

    	    srcb = vandq_u32(vshrq_n_u32(vmulq_u32(dstb, vshrq_n_u32(vmulq_u32(vmulq_u32(srcb, vmovq_n_u32(cb)), ina), 16)), 8), vmovq_n_u32(BlendBlit::kBModMask));
    	    srcg = vandq_u32(vshlq_n_u32(vmulq_u32(dstg, vshrq_n_u32(vmulq_u32(vmulq_u32(srcg, vmovq_n_u32(cg)), ina), 16)), BlendBlit::kGModShift - 8), vmovq_n_u32(BlendBlit::kGModMask));
    	    srcr = vandq_u32(vshlq_n_u32(vmulq_u32(dstr, vshrq_n_u32(vmulq_u32(vmulq_u32(srcr, vmovq_n_u32(cr)), ina), 16)), BlendBlit::kRModShift - 8), vmovq_n_u32(BlendBlit::kRModMask));

    	    src = vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
    	    src = vorrq_u32(src, vorrq_u32(srcb, vorrq_u32(srcg, srcb)));
    	} else {
    	    uint32x4_t srcg = vandq_u32(src, vmovq_n_u32(BlendBlit::kGModMask));
    	    uint32x4_t srcrb = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
    	    uint32x4_t dstg = vandq_u32(dst, vmovq_n_u32(BlendBlit::kGModMask));
    	    uint32x4_t dstrb = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
    	    srcg = vandq_u32(vshrq_n_u32(vmulq_u32(dstg, vshrq_n_u32(vmulq_u32(srcg, ina), 8)), 8), vmovq_n_u32(BlendBlit::kGModMask));
    	    srcrb = vandq_u32(vmulq_u32(dstrb, vshrq_n_u32(vmulq_u32(srcrb, ina), 8)), vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask));
    	    src = vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
    	    src = vorrq_u32(src, vorrq_u32(srcrb, srcg));
    	}

    	dst = vandq_u32(alphaMask, dst);
    	src = vandq_u32(vmvnq_u32(alphaMask), src);
    	return vorrq_u32(dst, src);
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
struct OpaqueBlend {
	static inline uint32x4_t simd(uint32x4_t src, uint32x4_t dst, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
		return vorrq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
	}

	static inline void normal(const byte *in, byte *out, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
		*(uint32 *)out = *(const uint32 *)in | BlendBlit::kAModMask;
	}
};

template<bool doscale, bool rgbmod, bool alphamod>
struct BinaryBlend {
	static inline uint32x4_t simd(uint32x4_t src, uint32x4_t dst, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
            uint32x4_t alphaMask = vceqq_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask)), vmovq_n_u32(0));
            dst = vandq_u32(dst, alphaMask);
            src = vandq_u32(vorrq_u32(src, vmovq_n_u32(BlendBlit::kAModMask)), vmvnq_u32(alphaMask));
            return vorrq_u32(dst, src);
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
struct AdditiveBlend {
	static inline uint32x4_t simd(uint32x4_t src, uint32x4_t dst, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
    	uint32x4_t ina;
    	if (alphamod)
    	    ina = vshrq_n_u32(vmulq_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask)), vdupq_n_u32(ca)), 8);
    	else
    	    ina = vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
    	uint32x4_t alphaMask = vceqq_u32(ina, vmovq_n_u32(0));

    	if (rgbmod) {
    	    uint32x4_t srcb = vandq_u32(src, vmovq_n_u32(BlendBlit::kBModMask));
    	    uint32x4_t srcg = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
    	    uint32x4_t srcr = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
    	    uint32x4_t dstb = vandq_u32(dst, vmovq_n_u32(BlendBlit::kBModMask));
    	    uint32x4_t dstg = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
    	    uint32x4_t dstr = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kRModMask)), BlendBlit::kRModShift);

			srcb = vandq_u32(vaddq_u32(dstb, vshrq_n_u32(vmulq_u32(srcb, vmulq_u32(vmovq_n_u32(cb), ina)), 16)), vmovq_n_u32(BlendBlit::kBModMask));
			srcg = vandq_u32(vaddq_u32(dstg, vmulq_u32(srcg, vmulq_u32(vmovq_n_u32(cg), ina))), vmovq_n_u32(BlendBlit::kGModMask));
			srcr = vandq_u32(vaddq_u32(dstr, vshrq_n_u32(vmulq_u32(srcr, vmulq_u32(vmovq_n_u32(cr), ina)), BlendBlit::kRModShift - 16)), vmovq_n_u32(BlendBlit::kRModMask));

    	    src = vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
    	    src = vorrq_u32(src, vorrq_u32(srcb, vorrq_u32(srcg, srcb)));
    	} else if (alphamod) {
    	    uint32x4_t srcg = vandq_u32(src, vmovq_n_u32(BlendBlit::kGModMask));
    	    uint32x4_t srcrb = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
    	    uint32x4_t dstg = vandq_u32(dst, vmovq_n_u32(BlendBlit::kGModMask));
    	    uint32x4_t dstrb = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);

			srcg = vandq_u32(vaddq_u32(dstg, vshrq_n_u32(vmulq_u32(srcg, ina), 8)), vmovq_n_u32(BlendBlit::kGModMask));
			srcrb = vandq_u32(vaddq_u32(dstrb, vmulq_u32(srcrb, ina)), vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask));

    	    src = vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
    	    src = vorrq_u32(src, vorrq_u32(srcrb, srcg));
    	} else {
    	    uint32x4_t srcg = vandq_u32(src, vmovq_n_u32(BlendBlit::kGModMask));
    	    uint32x4_t srcrb = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
    	    uint32x4_t dstg = vandq_u32(dst, vmovq_n_u32(BlendBlit::kGModMask));
    	    uint32x4_t dstrb = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);

			srcg = vandq_u32(vaddq_u32(dstg, srcg), vmovq_n_u32(BlendBlit::kGModMask));
			srcrb = vandq_u32(vshlq_n_u32(vaddq_u32(dstrb, srcrb), 8), vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask));

    	    src = vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
    	    src = vorrq_u32(src, vorrq_u32(srcrb, srcg));
		}

    	dst = vandq_u32(alphaMask, dst);
    	src = vandq_u32(vmvnq_u32(alphaMask), src);
    	return vorrq_u32(dst, src);
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
struct SubtractiveBlend {
	static inline uint32x4_t simd(uint32x4_t src, uint32x4_t dst, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
    	uint32x4_t ina = vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
    	uint32x4_t srcb = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
    	uint32x4_t srcg = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
    	uint32x4_t srcr = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
    	uint32x4_t dstb = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
    	uint32x4_t dstg = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
    	uint32x4_t dstr = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kRModMask)), BlendBlit::kRModShift);

		srcb = vandq_u32(vshlq_n_u32(vreinterpretq_u32_s32(vmaxq_s32(vsubq_s32(vreinterpretq_s32_u32(dstb), vreinterpretq_s32_u32(vshrq_n_u32(vmulq_u32(vmulq_u32(srcb, vmovq_n_u32(cb)), vmulq_u32(dstb, ina)), 24))), vmovq_n_s32(0))), BlendBlit::kBModShift), vmovq_n_u32(BlendBlit::kBModMask));
		srcg = vandq_u32(vshlq_n_u32(vreinterpretq_u32_s32(vmaxq_s32(vsubq_s32(vreinterpretq_s32_u32(dstg), vreinterpretq_s32_u32(vshrq_n_u32(vmulq_u32(vmulq_u32(srcg, vmovq_n_u32(cg)), vmulq_u32(dstg, ina)), 24))), vmovq_n_s32(0))), BlendBlit::kGModShift), vmovq_n_u32(BlendBlit::kGModMask));
		srcr = vandq_u32(vshlq_n_u32(vreinterpretq_u32_s32(vmaxq_s32(vsubq_s32(vreinterpretq_s32_u32(dstr), vreinterpretq_s32_u32(vshrq_n_u32(vmulq_u32(vmulq_u32(srcr, vmovq_n_u32(cr)), vmulq_u32(dstr, ina)), 24))), vmovq_n_s32(0))), BlendBlit::kRModShift), vmovq_n_u32(BlendBlit::kRModMask));

    	return vorrq_u32(vmovq_n_u32(BlendBlit::kAModMask), vorrq_u32(srcb, vorrq_u32(srcg, srcr)));
	}

	static inline void normal(const byte *in, byte *out, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
			out[BlendBlit::kAIndex] = 255;
			out[BlendBlit::kBIndex] = MAX<int32>(out[BlendBlit::kBIndex] - ((in[BlendBlit::kBIndex] * cb  * (out[BlendBlit::kBIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);
			out[BlendBlit::kGIndex] = MAX<int32>(out[BlendBlit::kGIndex] - ((in[BlendBlit::kGIndex] * cg  * (out[BlendBlit::kGIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);
			out[BlendBlit::kRIndex] = MAX<int32>(out[BlendBlit::kRIndex] - ((in[BlendBlit::kRIndex] * cr *  (out[BlendBlit::kRIndex]) * in[BlendBlit::kAIndex]) >> 24), 0);
	}
};

class BlendBlitImpl {

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

	int scaleXCtr, scaleYCtr = 0;
	const byte *inBase;

    if (!doscale && (args.flipping & FLIP_H)) args.ino -= 4 * 3;

	for (uint32 i = 0; i < args.height; i++) {
		if (doscale) {
			inBase = args.ino + scaleYCtr / BlendBlit::SCALE_THRESHOLD * args.inoStep;
			scaleXCtr = 0;
		} else {
			in = args.ino;
		}
		out = args.outo;
        uint32 j = 0;
		for (; j + 4 <= args.width; j += 4) {
            uint32x4_t dstPixels;
            if (loaddst) dstPixels = vld1q_u32((const uint32 *)out);
            uint32x4_t srcPixels;
            if (!doscale) {
                srcPixels = vld1q_u32((const uint32 *)in);
            } else {
				srcPixels = vsetq_lane_u32(*(const uint32 *)(inBase + scaleXCtr / BlendBlit::SCALE_THRESHOLD * args.inStep), vmovq_n_u32(0), 0);
                scaleXCtr += args.scaleX;
				srcPixels = vsetq_lane_u32(*(const uint32 *)(inBase + scaleXCtr / BlendBlit::SCALE_THRESHOLD * args.inStep), srcPixels, 1);
                scaleXCtr += args.scaleX;
				srcPixels = vsetq_lane_u32(*(const uint32 *)(inBase + scaleXCtr / BlendBlit::SCALE_THRESHOLD * args.inStep), srcPixels, 2);
                scaleXCtr += args.scaleX;
				srcPixels = vsetq_lane_u32(*(const uint32 *)(inBase + scaleXCtr / BlendBlit::SCALE_THRESHOLD * args.inStep), srcPixels, 3);
                scaleXCtr += args.scaleX;
            }
            if (!doscale && (args.flipping & FLIP_H)) {
                srcPixels = vrev64q_u32(srcPixels);
	            srcPixels = vcombine_u32(vget_high_u32(srcPixels), vget_low_u32(srcPixels));
            }
			{
				const uint32x4_t res = PixelFunc<doscale, rgbmod, alphamod>::simd(srcPixels, dstPixels, args.flipping & FLIP_H, ca, cr, cg, cb);
            	vst1q_u32((uint32 *)out, res);
			}
			if (!doscale) in += args.inStep * 4;
			out += 4 * 4;
		}
		if (!doscale && (args.flipping & FLIP_H)) in += 4 * 3;
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

};

template<bool doscale, bool rgbmod, bool alphamod>
void BlendBlit::doBlitAlphaBlendLogicNEON(Args &args) {
	BlendBlitImpl::blitInnerLoop<AlphaBlend, doscale, rgbmod, alphamod, false, true>(args);
}
template<bool doscale, bool rgbmod>
void BlendBlit::doBlitSubtractiveBlendLogicNEON(Args &args) {
	BlendBlitImpl::blitInnerLoop<SubtractiveBlend, doscale, rgbmod, false, false, true>(args);
}
template<bool doscale, bool rgbmod, bool alphamod>
void BlendBlit::doBlitAdditiveBlendLogicNEON(Args &args) {
	BlendBlitImpl::blitInnerLoop<AdditiveBlend, doscale, rgbmod, alphamod, false, true>(args);
}
template<bool doscale>
void BlendBlit::doBlitOpaqueBlendLogicNEON(Args &args) {
	BlendBlitImpl::blitInnerLoop<OpaqueBlend, doscale, false, false, false, true>(args);
}
template<bool doscale>
void BlendBlit::doBlitBinaryBlendLogicNEON(Args &args) {
	BlendBlitImpl::blitInnerLoop<BinaryBlend, doscale, false, false, false, true>(args);
}
template<bool doscale, bool rgbmod, bool alphamod>
void BlendBlit::doBlitMultiplyBlendLogicNEON(Args &args) {
	BlendBlitImpl::blitInnerLoop<MultiplyBlend, doscale, rgbmod, alphamod, false, true>(args);
}

} // end of namespace Graphics

#endif // __ARM_NEON__
