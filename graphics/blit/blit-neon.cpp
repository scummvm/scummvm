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

#ifdef SCUMMVM_NEON

#include "graphics/blit/blit-alpha.h"
#include "graphics/pixelformat.h"

#include <arm_neon.h>

#if !defined(__aarch64__)

#if defined(__clang__)
#pragma clang attribute push (__attribute__((target("neon"))), apply_to=function)
#elif defined(__GNUC__)
#pragma GCC push_options
#pragma GCC target("fpu=neon")
#endif

#endif // !defined(__aarch64__)

namespace Graphics {

class BlendBlitImpl_NEON : public BlendBlitImpl_Base {
	friend class BlendBlit;

template<bool rgbmod, bool alphamod>
struct AlphaBlend : public BlendBlitImpl_Base::AlphaBlend<rgbmod, alphamod> {
public:
	constexpr AlphaBlend(const uint32 color) : BlendBlitImpl_Base::AlphaBlend<rgbmod, alphamod>(color) {}

	inline uint32x4_t simd(uint32x4_t src, uint32x4_t dst) const {
		uint32x4_t ina;
		if (alphamod)
			ina = vshrq_n_u32(vmulq_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask)), vdupq_n_u32(this->ca)), 8);
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
			srcR = vaddq_u32(dstR, vshrq_n_u32(vmulq_u32(vmulq_u32(srcR, ina), vmovq_n_u32(this->cr)), 16));
			srcG = vaddq_u32(dstG, vshrq_n_u32(vmulq_u32(vmulq_u32(srcG, ina), vmovq_n_u32(this->cg)), 16));
			srcB = vaddq_u32(dstB, vshrq_n_u32(vmulq_u32(vmulq_u32(srcB, ina), vmovq_n_u32(this->cb)), 16));
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
};

template<bool rgbmod, bool alphamod>
struct MultiplyBlend : public BlendBlitImpl_Base::MultiplyBlend<rgbmod, alphamod> {
public:
	constexpr MultiplyBlend(const uint32 color) : BlendBlitImpl_Base::MultiplyBlend<rgbmod, alphamod>(color) {}

	inline uint32x4_t simd(uint32x4_t src, uint32x4_t dst) const {
		uint32x4_t ina, alphaMask;
		if (alphamod) {
			ina = vshrq_n_u32(vmulq_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask)), vdupq_n_u32(this->ca)), 8);
			alphaMask = vceqq_u32(ina, vmovq_n_u32(0));
		} else {
			ina = vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
			alphaMask = vdupq_n_u32(BlendBlit::kAModMask);
		}

		if (rgbmod) {
			uint32x4_t srcB = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
			uint32x4_t srcG = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			uint32x4_t srcR = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
			uint32x4_t dstB = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
			uint32x4_t dstG = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			uint32x4_t dstR = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kRModMask)), BlendBlit::kRModShift);

			srcB = vandq_u32(vshlq_n_u32(vmulq_u32(dstB, vshrq_n_u32(vmulq_u32(vmulq_u32(srcB, vmovq_n_u32(this->cb)), ina), 16)), BlendBlit::kBModShift - 8), vmovq_n_u32(BlendBlit::kBModMask));
			srcG = vandq_u32(vshlq_n_u32(vmulq_u32(dstG, vshrq_n_u32(vmulq_u32(vmulq_u32(srcG, vmovq_n_u32(this->cg)), ina), 16)), BlendBlit::kGModShift - 8), vmovq_n_u32(BlendBlit::kGModMask));
			srcR = vandq_u32(vshlq_n_u32(vmulq_u32(dstR, vshrq_n_u32(vmulq_u32(vmulq_u32(srcR, vmovq_n_u32(this->cr)), ina), 16)), BlendBlit::kRModShift - 8), vmovq_n_u32(BlendBlit::kRModMask));

			src = vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
			src = vorrq_u32(src, vorrq_u32(srcB, vorrq_u32(srcG, srcR)));
		} else {
			constexpr uint32 rbMask = BlendBlit::kRModMask | BlendBlit::kBModMask;
			uint32x4_t srcG = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			uint32x4_t srcRB = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(rbMask)), BlendBlit::kBModShift);
			uint32x4_t dstG = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
			uint32x4_t dstRB = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(rbMask)), BlendBlit::kBModShift);

			srcG = vandq_u32(vshlq_n_u32(vmulq_u32(dstG, vshrq_n_u32(vmulq_u32(srcG, ina), 8)), 8), vmovq_n_u32(BlendBlit::kGModMask));
			srcRB = vandq_u32(vreinterpretq_u32_u16(vmulq_u16(vreinterpretq_u16_u32(dstRB), vreinterpretq_u16_u32(vshrq_n_u32(vandq_u32(vmulq_u32(srcRB, ina), vmovq_n_u32(rbMask)), 8)))), vmovq_n_u32(rbMask));

			src = vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
			src = vorrq_u32(src, vorrq_u32(srcRB, srcG));
		}

		dst = vandq_u32(alphaMask, dst);
		src = vandq_u32(vmvnq_u32(alphaMask), src);
		return vorrq_u32(dst, src);
	}
};

template<bool rgbmod, bool alphamod>
struct OpaqueBlend : public BlendBlitImpl_Base::OpaqueBlend<rgbmod, alphamod> {
public:
	constexpr OpaqueBlend(const uint32 color) : BlendBlitImpl_Base::OpaqueBlend<rgbmod, alphamod>(color) {}

	inline uint32x4_t simd(uint32x4_t src, uint32x4_t dst) const {
		return vorrq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
	}
};

template<bool rgbmod, bool alphamod>
struct BinaryBlend : public BlendBlitImpl_Base::BinaryBlend<rgbmod, alphamod> {
public:
	constexpr BinaryBlend(const uint32 color) : BlendBlitImpl_Base::BinaryBlend<rgbmod, alphamod>(color) {}

	inline uint32x4_t simd(uint32x4_t src, uint32x4_t dst) const {
		uint32x4_t alphaMask = vceqq_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask)), vmovq_n_u32(0));
		dst = vandq_u32(dst, alphaMask);
		src = vandq_u32(vorrq_u32(src, vmovq_n_u32(BlendBlit::kAModMask)), vmvnq_u32(alphaMask));
		return vorrq_u32(dst, src);
	}
};

template<bool rgbmod, bool alphamod>
struct AdditiveBlend : public BlendBlitImpl_Base::AdditiveBlend<rgbmod, alphamod> {
public:
	constexpr AdditiveBlend(const uint32 color) : BlendBlitImpl_Base::AdditiveBlend<rgbmod, alphamod>(color) {}

	inline uint32x4_t simd(uint32x4_t src, uint32x4_t dst) const {
		uint32x4_t ina;
		if (alphamod)
			ina = vshrq_n_u32(vmulq_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask)), vdupq_n_u32(this->ca)), 8);
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

			srcb = vandq_u32(vaddq_u32(dstb, vshrq_n_u32(vmulq_u32(srcb, vmulq_u32(vmovq_n_u32(this->cb), ina)), 16)), vmovq_n_u32(BlendBlit::kBModMask));
			srcg = vandq_u32(vaddq_u32(dstg, vmulq_u32(srcg, vmulq_u32(vmovq_n_u32(this->cg), ina))), vmovq_n_u32(BlendBlit::kGModMask));
			srcr = vandq_u32(vaddq_u32(dstr, vshrq_n_u32(vmulq_u32(srcr, vmulq_u32(vmovq_n_u32(this->cr), ina)), BlendBlit::kRModShift - 16)), vmovq_n_u32(BlendBlit::kRModMask));

			src = vandq_u32(dst, vmovq_n_u32(BlendBlit::kAModMask));
			src = vorrq_u32(src, vorrq_u32(srcb, vorrq_u32(srcg, srcr)));
		} else if (alphamod) {
			uint32x4_t srcg = vandq_u32(src, vmovq_n_u32(BlendBlit::kGModMask));
			uint32x4_t srcrb = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
			uint32x4_t dstg = vandq_u32(dst, vmovq_n_u32(BlendBlit::kGModMask));
			uint32x4_t dstrb = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);

			srcg = vandq_u32(vaddq_u32(dstg, vshrq_n_u32(vmulq_u32(srcg, ina), 8)), vmovq_n_u32(BlendBlit::kGModMask));
			srcrb = vandq_u32(vaddq_u32(dstrb, vmulq_u32(srcrb, ina)), vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask));

			src = vandq_u32(dst, vmovq_n_u32(BlendBlit::kAModMask));
			src = vorrq_u32(src, vorrq_u32(srcrb, srcg));
		} else {
			uint32x4_t srcg = vandq_u32(src, vmovq_n_u32(BlendBlit::kGModMask));
			uint32x4_t srcrb = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);
			uint32x4_t dstg = vandq_u32(dst, vmovq_n_u32(BlendBlit::kGModMask));
			uint32x4_t dstrb = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask)), BlendBlit::kBModShift);

			srcg = vandq_u32(vaddq_u32(dstg, srcg), vmovq_n_u32(BlendBlit::kGModMask));
			srcrb = vandq_u32(vshlq_n_u32(vaddq_u32(dstrb, srcrb), 8), vmovq_n_u32(BlendBlit::kRModMask | BlendBlit::kBModMask));

			src = vandq_u32(dst, vmovq_n_u32(BlendBlit::kAModMask));
			src = vorrq_u32(src, vorrq_u32(srcrb, srcg));
		}

		dst = vandq_u32(alphaMask, dst);
		src = vandq_u32(vmvnq_u32(alphaMask), src);
		return vorrq_u32(dst, src);
	}
};

template<bool rgbmod, bool alphamod>
struct SubtractiveBlend : public BlendBlitImpl_Base::SubtractiveBlend<rgbmod, alphamod> {
public:
	constexpr SubtractiveBlend(const uint32 color) : BlendBlitImpl_Base::SubtractiveBlend<rgbmod, alphamod>(color) {}

	inline uint32x4_t simd(uint32x4_t src, uint32x4_t dst) const {
		uint32x4_t ina = vandq_u32(src, vmovq_n_u32(BlendBlit::kAModMask));
		uint32x4_t srcb = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
		uint32x4_t srcg = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
		uint32x4_t srcr = vshrq_n_u32(vandq_u32(src, vmovq_n_u32(BlendBlit::kRModMask)), BlendBlit::kRModShift);
		uint32x4_t dstb = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kBModMask)), BlendBlit::kBModShift);
		uint32x4_t dstg = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kGModMask)), BlendBlit::kGModShift);
		uint32x4_t dstr = vshrq_n_u32(vandq_u32(dst, vmovq_n_u32(BlendBlit::kRModMask)), BlendBlit::kRModShift);

		srcb = vandq_u32(vshlq_n_u32(vreinterpretq_u32_s32(vmaxq_s32(vsubq_s32(vreinterpretq_s32_u32(dstb), vreinterpretq_s32_u32(vshrq_n_u32(vmulq_u32(vmulq_u32(srcb, vmovq_n_u32(this->cb)), vmulq_u32(dstb, ina)), 24))), vmovq_n_s32(0))), BlendBlit::kBModShift), vmovq_n_u32(BlendBlit::kBModMask));
		srcg = vandq_u32(vshlq_n_u32(vreinterpretq_u32_s32(vmaxq_s32(vsubq_s32(vreinterpretq_s32_u32(dstg), vreinterpretq_s32_u32(vshrq_n_u32(vmulq_u32(vmulq_u32(srcg, vmovq_n_u32(this->cg)), vmulq_u32(dstg, ina)), 24))), vmovq_n_s32(0))), BlendBlit::kGModShift), vmovq_n_u32(BlendBlit::kGModMask));
		srcr = vandq_u32(vshlq_n_u32(vreinterpretq_u32_s32(vmaxq_s32(vsubq_s32(vreinterpretq_s32_u32(dstr), vreinterpretq_s32_u32(vshrq_n_u32(vmulq_u32(vmulq_u32(srcr, vmovq_n_u32(this->cr)), vmulq_u32(dstr, ina)), 24))), vmovq_n_s32(0))), BlendBlit::kRModShift), vmovq_n_u32(BlendBlit::kRModMask));

		return vorrq_u32(vmovq_n_u32(BlendBlit::kAModMask), vorrq_u32(srcb, vorrq_u32(srcg, srcr)));
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
				const uint32x4_t res = pixelFunc.simd(srcPixels, dstPixels);
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

}; // end of class BlendBlitImpl_NEON

void BlendBlit::blitNEON(Args &args, const TSpriteBlendMode &blendMode, const AlphaType &alphaType) {
	blitT<BlendBlitImpl_NEON>(args, blendMode, alphaType);
}

} // end of namespace Graphics

#if !defined(__aarch64__)

#if defined(__clang__)
#pragma clang attribute pop
#elif defined(__GNUC__)
#pragma GCC pop_options
#endif

#endif // !defined(__aarch64__)

#endif // SCUMMVM_NEON
