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

template<bool doscale>
void BlendBlit::doBlitBinaryBlendLogicNEON(Args &args) {
    (void)args;
}
template<bool doscale>
void BlendBlit::doBlitOpaqueBlendLogicNEON(Args &args) {
    (void)args;
}
template<bool doscale, bool rgbmod, bool alphamod>
void BlendBlit::doBlitMultiplyBlendLogicNEON(Args &args) {
    (void)args;
}
template<bool doscale, bool rgbmod>
void BlendBlit::doBlitSubtractiveBlendLogicNEON(Args &args) {
    (void)args;
}
template<bool doscale, bool rgbmod, bool alphamod>
void BlendBlit::doBlitAdditiveBlendLogicNEON(Args &args) {
    (void)args;
}

template<bool rgbmod, bool alphamod>
static inline uint32x4_t drawPixelAlphaBlend(uint32x4_t src, uint32x4_t dst, const bool flip, const byte ca, const byte cr, const byte cg, const byte cb) {
    if (flip) {
        src = vrev64q_u32(src);
	    src = vcombine_u32(vget_high_u32(src), vget_low_u32(src));
    }
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

        dstRB = vshrq_n_u32(vmulq_u32(dstRB, vsubq_u32(vmovq_n_u32(255), ina)), 8);
        dstG = vshrq_n_u32(vmulq_u32(dstG, vsubq_u32(vmovq_n_u32(255), ina)), 8);
        srcRB = vaddq_u32(dstRB, vshrq_n_u32(vmulq_u32(srcRB, ina), 8));
        srcG = vaddq_u32(dstG, vshrq_n_u32(vmulq_u32(srcG, ina), 8));
        src = vorrq_u32(vandq_u32(srcG, vmovq_n_u32(BlendBlit::kGModMask)), vmovq_n_u32(BlendBlit::kAModMask));
        src = vorrq_u32(vandq_u32(vshlq_n_u32(srcRB, 8), vmovq_n_u32(BlendBlit::kBModMask | BlendBlit::kRModMask)), src);
    }

    dst = vandq_u32(alphaMask, dst);
    src = vandq_u32(vmvnq_u32(alphaMask), src);
    return vorrq_u32(dst, src);
}
template<bool doscale, bool rgbmod, bool alphamod>
void BlendBlit::doBlitAlphaBlendLogicNEON(Args &args) {
	const byte *in;
	byte *out;

	int scaleXCtr, scaleYCtr = 0;
	const byte *inBase;

	const byte ca = alphamod ? ((args.color >> kAModShift) & 0xFF) : 255;
	const byte cr = rgbmod   ? ((args.color >> kRModShift) & 0xFF) : 255;
	const byte cg = rgbmod   ? ((args.color >> kGModShift) & 0xFF) : 255;
	const byte cb = rgbmod   ? ((args.color >> kBModShift) & 0xFF) : 255;

	for (uint32 i = 0; i < args.height; i++) {
		if (doscale) {
			inBase = args.ino + scaleYCtr / SCALE_THRESHOLD * args.inoStep;
			scaleXCtr = 0;
		} else {
			in = args.ino;
		}
		out = args.outo;

        uint32 j;
		for (j = 0; j + 4 < args.width; j += 4) {
            uint32x4_t dstPixels = vld1q_u32((const uint32 *)out);
            uint32x4_t srcPixels;
            if (!doscale) {
                srcPixels = vld1q_u32((const uint32 *)in);
            } else {
				srcPixels = vsetq_lane_u32(*(const uint32 *)(inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep), vmovq_n_u32(0), 0);
                scaleXCtr += args.scaleX;
				srcPixels = vsetq_lane_u32(*(const uint32 *)(inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep), srcPixels, 0);
                scaleXCtr += args.scaleX;
				srcPixels = vsetq_lane_u32(*(const uint32 *)(inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep), srcPixels, 0);
                scaleXCtr += args.scaleX;
				srcPixels = vsetq_lane_u32(*(const uint32 *)(inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep), srcPixels, 0);
                scaleXCtr += args.scaleX;
            }
            uint32x4_t res = drawPixelAlphaBlend<rgbmod, alphamod>(srcPixels, dstPixels, args.flipping & FLIP_H, ca, cr, cg, cb);
            vst1q_u32((uint32 *)out, res);
			if (!doscale)
				in += args.inStep * 4;
			out += 4 * 4;
		}
		for (; j < args.width; j++) {
			if (doscale) {
				in = inBase + scaleXCtr / SCALE_THRESHOLD * args.inStep;
			}
			uint32 ina = in[kAIndex] * ca >> 8;

			if (ina != 0) {
				uint outb = (out[kBIndex] * (255 - ina) >> 8);
				uint outg = (out[kGIndex] * (255 - ina) >> 8);
				uint outr = (out[kRIndex] * (255 - ina) >> 8);

				out[kAIndex] = 255;
				out[kBIndex] = outb + (in[kBIndex] * ina * cb >> 16);
				out[kGIndex] = outg + (in[kGIndex] * ina * cg >> 16);
				out[kRIndex] = outr + (in[kRIndex] * ina * cr >> 16);
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

} // end of namespace Graphics

#endif // __ARM_NEON__
