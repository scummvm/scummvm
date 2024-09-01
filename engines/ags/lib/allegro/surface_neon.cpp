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

#include "ags/ags.h"

// Without this ifdef the iOS backend breaks, please do not remove
#ifdef SCUMMVM_NEON

#include "ags/globals.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/flood.h"
#include "ags/lib/allegro/gfx.h"
#include "common/textconsole.h"
#include "graphics/screen.h"

#include <arm_neon.h>

#if !defined(__aarch64__)

#if defined(__clang__)
#pragma clang attribute push (__attribute__((target("neon"))), apply_to=function)
#elif defined(__GNUC__)
#pragma GCC push_options
#pragma GCC target("fpu=neon")
#endif

#endif // !defined(__aarch64__)

namespace AGS3 {

class DrawInnerImpl_NEON {

static inline uint32x4_t simd2BppTo4Bpp(uint16x4_t pixels) {
	uint32x4_t x = vmovl_u16(pixels);

	// c is the extracted 5/6 bit color from the image
	uint32x4_t c = vshrq_n_u32(x, 11);

	// We convert it back to normal by shifting it thrice over, naturally, and then using the 2 most
	// sinificant bits in the original color for the least significant bits in the new one
	uint32x4_t r = vshlq_n_u32(vorrq_u32(vshlq_n_u32(c, 3), vshrq_n_u32(c, 2)), 16);
	c = vshrq_n_u32(vandq_u32(x, vmovq_n_u32(0x07e0)), 5);
	uint32x4_t g = vshlq_n_u32(vorrq_u32(vshlq_n_u32(c, 2), vshrq_n_u32(c, 4)), 8);
	c = vandq_u32(x, vmovq_n_u32(0x001f));
	uint32x4_t b = vorrq_u32(vshlq_n_u32(c, 3), vshrq_n_u32(c, 2));

	// By default 2bpp to 4bpp makes the alpha channel 255
	return vorrq_u32(vorrq_u32(vorrq_u32(r, g), b), vmovq_n_u32(0xff000000));
}

static inline uint16x4_t simd4BppTo2Bpp(uint32x4_t pixels) {
	// x is the final 16 bit rgb pixel
	uint32x4_t x = vshrq_n_u32(vandq_u32(pixels, vmovq_n_u32(0x000000ff)), 3);
	x = vorrq_u32(x, vshlq_n_u32(vshrq_n_u32(vandq_u32(pixels, vmovq_n_u32(0x0000ff00)), 8+2), 5));
	x = vorrq_u32(x, vshlq_n_u32(vshrq_n_u32(vandq_u32(pixels, vmovq_n_u32(0x00ff0000)), 16+3), 11));
	return vmovn_u32(x);
}

static inline uint16x8_t rgbBlendSIMD2Bpp(uint16x8_t srcCols, uint16x8_t destCols, uint16x8_t alphas) {
	// Here we add 1 to alphas if its 0. This is what the original blender function did
	alphas = vaddq_u16(alphas, vandq_u16(vceqq_u16(alphas, vmovq_n_u16(0)), vmovq_n_u16(1)));

	// Split the components into rgb
	uint16x8_t srcComps[] = {
		vandq_u16(srcCols, vmovq_n_u16(0x1f)),					// B
		vandq_u16(vshrq_n_u16(srcCols, 5), vmovq_n_u16(0x3f)),	// G
		vshrq_n_u16(srcCols, 11),								// R
	}, destComps[] = {
		vandq_u16(destCols, vmovq_n_u16(0x1f)),					// B
		vandq_u16(vshrq_n_u16(destCols, 5), vmovq_n_u16(0x3f)), // G
		vshrq_n_u16(destCols, 11),								// R
	};

	// At some point I made it so that it would put them into their 8bit depth format
	// to keep the function as 1-1 with the original, but it didn't seem to help much
	//srcComps[0] = vorrq_u16(vshlq_n_u16(srcComps[0], 3), vshrq_n_u16(srcComps[0], 2));
	//srcComps[1] = vorrq_u16(vshlq_n_u16(srcComps[1], 2), vshrq_n_u16(srcComps[1], 4));
	//srcComps[2] = vorrq_u16(vshlq_n_u16(srcComps[2], 3), vshrq_n_u16(srcComps[2], 2));
	//destComps[0] = vorrq_u16(vshlq_n_u16(destComps[0], 3), vshrq_n_u16(destComps[0], 2));
	//destComps[1] = vorrq_u16(vshlq_n_u16(destComps[1], 2), vshrq_n_u16(destComps[1], 4));
	//destComps[2] = vorrq_u16(vshlq_n_u16(destComps[2], 3), vshrq_n_u16(destComps[2], 2));

	// Calculate the differences between the colors
	uint16x8_t diffs[] = {
		vsubq_u16(srcComps[0], destComps[0]), // B
		vsubq_u16(srcComps[1], destComps[1]), // G
		vsubq_u16(srcComps[2], destComps[2]), // R
	};

	// Multiply by alpha and shift depth bits to the right
	// pretty much the same as (int)(((float)component / 255.0f) * ((float)alpha / 255.0f) * 255.0f)
	alphas = vshrq_n_u16(alphas, 2);
	diffs[1] = vshrq_n_u16(vmulq_u16(diffs[1], alphas), 6);
	alphas = vshrq_n_u16(alphas, 1);
	diffs[0] = vshrq_n_u16(vmulq_u16(diffs[0], alphas), 5);
	diffs[2] = vshrq_n_u16(vmulq_u16(diffs[2], alphas), 5);

	// Originally, I converted it back to normal here from the 8bpp form, but don't need to do that anymore
	//diffs[0] = vandq_u16(vshrq_n_u16(vaddq_u16(diffs[0], destComps[0]), 3), vmovq_n_u16(0x1f));
	//diffs[1] = vandq_u16(vshrq_n_u16(vaddq_u16(diffs[1], destComps[1]), 2), vmovq_n_u16(0x3f));
	//diffs[2] = vandq_u16(vshrq_n_u16(vaddq_u16(diffs[2], destComps[2]), 3), vmovq_n_u16(0x1f));

	// Here we add the difference between the 2 colors times alpha onto the destination
	diffs[0] = vandq_u16(vaddq_u16(diffs[0], destComps[0]), vmovq_n_u16(0x1f));
	diffs[1] = vandq_u16(vaddq_u16(diffs[1], destComps[1]), vmovq_n_u16(0x3f));
	diffs[2] = vandq_u16(vaddq_u16(diffs[2], destComps[2]), vmovq_n_u16(0x1f));

	// We compile all the colors into diffs[0] as a 16 bit rgb pixel
	diffs[0] = vorrq_u16(diffs[0], vshlq_n_u16(diffs[1], 5));
	return vorrq_u16(diffs[0], vshlq_n_u16(diffs[2], 11));
}

// preserveAlpha:
//		false => set destCols's alpha to 0
// 		true => keep destCols's alpha
static inline uint32x4_t rgbBlendSIMD(uint32x4_t srcCols, uint32x4_t destCols, uint32x4_t alphas, bool preserveAlpha) {
	// Here we add 1 to alphas if its 0. This is what the original blender function did
	alphas = vaddq_u32(alphas, vandq_u32(vcgtq_u32(alphas, vmovq_n_u32(0)), vmovq_n_u32(1)));

	// Get the alpha from the destination
	uint32x4_t alpha = vandq_u32(destCols, vmovq_n_u32(0xff000000));

	// Get red and blue components
	uint32x4_t srcColsCopy = srcCols;
	srcColsCopy = vandq_u32(srcColsCopy, vmovq_n_u32(0xff00ff));
	uint32x4_t destColsCopy = destCols;
	destColsCopy = vandq_u32(destColsCopy, vmovq_n_u32(0xff00ff));

	// compute the difference, then multiply by alpha and divide by 255
	srcColsCopy = vsubq_u32(srcColsCopy, destColsCopy);
	srcColsCopy = vmulq_u32(srcColsCopy, alphas);
	srcColsCopy = vshrq_n_u32(srcColsCopy, 8);
	srcColsCopy = vaddq_u32(srcColsCopy, destCols); // Add the new red/blue to the old ones

	// do the same for the green component
	srcCols = vandq_u32(srcCols, vmovq_n_u32(0xff00));
	destCols = vandq_u32(destCols, vmovq_n_u32(0xff00));
	srcCols = vsubq_u32(srcCols, destCols);
	srcCols = vmulq_u32(srcCols, alphas);
	srcCols = vshrq_n_u32(srcCols, 8);
	srcCols = vaddq_u32(srcCols, destCols); // Add the new green to the old green

	// keep values in 8bit range and glue red/blue and green together
	srcColsCopy = vandq_u32(srcColsCopy, vmovq_n_u32(0xff00ff));
	srcCols = vandq_u32(srcCols, vmovq_n_u32(0xff00));
	srcCols = vorrq_u32(srcCols, srcColsCopy);

	// Remember that alpha is not alphas, but rather the alpha of destCols
	if (preserveAlpha) {
		srcCols = vandq_u32(srcCols, vmovq_n_u32(0x00ffffff));
		srcCols = vorrq_u32(srcCols, alpha);
	}
	return srcCols;
}

// uses the alpha from srcCols and destCols
static inline uint32x4_t argbBlendSIMD(uint32x4_t srcCols, uint32x4_t destCols) {
	float32x4_t srcA = vcvtq_f32_u32(vshrq_n_u32(srcCols, 24));
	srcA = vmulq_n_f32(srcA, 1.0f / 255.0f);
	float32x4_t srcR = vcvtq_f32_u32(vandq_u32(vshrq_n_u32(srcCols, 16), vmovq_n_u32(0xff)));
	float32x4_t srcG = vcvtq_f32_u32(vandq_u32(vshrq_n_u32(srcCols, 8), vmovq_n_u32(0xff)));
	float32x4_t srcB = vcvtq_f32_u32(vandq_u32(srcCols, vmovq_n_u32(0xff)));

	float32x4_t destA = vcvtq_f32_u32(vshrq_n_u32(destCols, 24));
	destA = vmulq_n_f32(destA, 1.0f / 255.0f);
	float32x4_t destR = vcvtq_f32_u32(vandq_u32(vshrq_n_u32(destCols, 16), vmovq_n_u32(0xff)));
	float32x4_t destG = vcvtq_f32_u32(vandq_u32(vshrq_n_u32(destCols, 8), vmovq_n_u32(0xff)));
	float32x4_t destB = vcvtq_f32_u32(vandq_u32(destCols, vmovq_n_u32(0xff)));

	// the destination alpha gets multiplied by 255 - source alpha
	destA = vmulq_f32(destA, vsubq_f32(vmovq_n_f32(1.0f), srcA));

	// ((src * sAlpha) + (dest * dAlpha)) / (sAlpha + dAlpha)
	float32x4_t combA = vaddq_f32(srcA, destA);
	float32x4_t combArcp = vrecpeq_f32(combA);
	destR = vmulq_f32(vaddq_f32(vmulq_f32(srcR, srcA), vmulq_f32(destR, destA)), combArcp);
	destG = vmulq_f32(vaddq_f32(vmulq_f32(srcG, srcA), vmulq_f32(destG, destA)), combArcp);
	destB = vmulq_f32(vaddq_f32(vmulq_f32(srcB, srcA), vmulq_f32(destB, destA)), combArcp);
	combA = vmulq_n_f32(combA, 255.0);

	// Now put it back together
	return vorrq_u32(vshlq_n_u32(vcvtq_u32_f32(combA), 24),
		vorrq_u32(vshlq_n_u32(vcvtq_u32_f32(destR), 16),
		vorrq_u32(vshlq_n_u32(vcvtq_u32_f32(destG), 8),
			vcvtq_u32_f32(destB))));
}

static inline uint32x4_t blendTintSpriteSIMD(uint32x4_t srcCols, uint32x4_t destCols, uint32x4_t alphas, bool light) {
	// This function is NOT 1 to 1 with the original... It just approximates it
	// It gets the value of the HSV of the dest color
	// Then it gets the HSV of the srcCols

	// how the values are transformed
	// from 1 uint32x4_t srcCols with each lane being ARGB uint32
	// srcCols[0] = A | R | G | B
	// srcCols[1] = A | R | G | B
	// srcCols[2] = A | R | G | B
	// srcCols[3] = A | R | G | B
	//  ->
	// to 4 float32x4_t's each being a separate channel with each lane
	// corresponding to their respective srcCols lane
	// dda = { A[0], A[1], A[2], A[3] }
	// ddr = { R[0], R[1], R[2], R[3] }
	// ddg = { G[0], G[1], G[2], G[3] }
	// ddb = { B[0], B[1], B[2], B[3] }

	// do the transformation (we don't actually need alpha at all)
	float32x4_t ddr, ddg, ddb;
	ddr = vmulq_n_f32(vcvtq_f32_u32(vandq_u32(vshrq_n_u32(destCols, 16), vmovq_n_u32(0xff))), 1.0 / 255.0);
	ddg = vmulq_n_f32(vcvtq_f32_u32(vandq_u32(vshrq_n_u32(destCols, 8), vmovq_n_u32(0xff))), 1.0 / 255.0);
	ddb = vmulq_n_f32(vcvtq_f32_u32(vandq_u32(destCols, vmovq_n_u32(0xff))), 1.0 / 255.0);
	float32x4_t ssr, ssg, ssb;
	ssr = vmulq_n_f32(vcvtq_f32_u32(vandq_u32(vshrq_n_u32(srcCols, 16), vmovq_n_u32(0xff))), 1.0 / 255.0);
	ssg = vmulq_n_f32(vcvtq_f32_u32(vandq_u32(vshrq_n_u32(srcCols, 8), vmovq_n_u32(0xff))), 1.0 / 255.0);
	ssb = vmulq_n_f32(vcvtq_f32_u32(vandq_u32(srcCols, vmovq_n_u32(0xff))), 1.0 / 255.0);

	// Get the maxes and mins (needed for HSV->RGB and vice-versa)
	float32x4_t dmaxes = vmaxq_f32(ddr, vmaxq_f32(ddg, ddb));
	float32x4_t smaxes = vmaxq_f32(ssr, vmaxq_f32(ssg, ssb));
	float32x4_t smins = vminq_f32(ssr, vminq_f32(ssg, ssb));

	// This is here to stop from dividing by 0
	const float32x4_t eplison0 = vmovq_n_f32(0.0000001);

	float32x4_t chroma = vmaxq_f32(vsubq_f32(smaxes, smins), eplison0);

	// RGB to HSV is a piecewise function, so we compute each part of the function first...
	float32x4_t hr, hg, hb, hue, chromaReq;
	chromaReq = vrecpeq_f32(chroma);
	hr = vmulq_f32(vsubq_f32(ssg, ssb), chromaReq);
	float32x4_t hrDiv6 = vmulq_n_f32(hr, 1.0 / 6.0);
	hrDiv6 = vsubq_f32(hrDiv6, vcvtq_f32_u32(vandq_u32(vcltq_f32(hrDiv6, vmovq_n_f32(0.0)), vmovq_n_u32(1))));
	hr = vsubq_f32(hr, vmulq_n_f32(vcvtq_f32_s32(vcvtq_s32_f32(hrDiv6)), 6.0));
	hg = vaddq_f32(vmulq_f32(vsubq_f32(ssb, ssr), chromaReq), vmovq_n_f32(2.0));
	hb = vaddq_f32(vmulq_f32(vsubq_f32(ssr, ssg), chromaReq), vmovq_n_f32(4.0));

	// And then compute which one will be used based on criteria
	float32x4_t hrfactors = vcvtq_f32_u32(vandq_u32(vandq_u32(vceqq_f32(ssr, smaxes), vmvnq_u32(vceqq_f32(ssr, ssb))), vmovq_n_u32(1)));
	float32x4_t hgfactors = vcvtq_f32_u32(vandq_u32(vandq_u32(vceqq_f32(ssg, smaxes), vmvnq_u32(vceqq_f32(ssg, ssr))), vmovq_n_u32(1)));
	float32x4_t hbfactors = vcvtq_f32_u32(vandq_u32(vandq_u32(vceqq_f32(ssb, smaxes), vmvnq_u32(vceqq_f32(ssb, ssg))), vmovq_n_u32(1)));
	hue = vmulq_f32(hr, hrfactors);
	hue = vaddq_f32(hue, vmulq_f32(hg, hgfactors));
	hue = vaddq_f32(hue, vmulq_f32(hb, hbfactors));

	// Mess with the light like the original function
	float32x4_t val = dmaxes;
	if (light) {
		val = vsubq_f32(val, vsubq_f32(vmovq_n_f32(1.0), vmulq_n_f32(vcvtq_f32_u32(alphas), 1.0 / 250.0)));
		val = vmaxq_f32(val, vmovq_n_f32(0.0));
	}

	// then it stitches the HSV back together
	// the hue and saturation come from the source (tint) color, and the value comes from
	// the destination (real source) color
	chroma = vmulq_f32(val, vmulq_f32(vsubq_f32(smaxes, smins), vrecpeq_f32(vaddq_f32(smaxes, eplison0))));
	float32x4_t hprime_mod2 = vmulq_n_f32(hue, 1.0 / 2.0);
	hprime_mod2 = vmulq_n_f32(vsubq_f32(hprime_mod2, vcvtq_f32_s32(vcvtq_s32_f32(hprime_mod2))), 2.0);
	float32x4_t x = vmulq_f32(chroma, vsubq_f32(vmovq_n_f32(1.0), vabsq_f32(vsubq_f32(hprime_mod2, vmovq_n_f32(1.0)))));
	uint32x4_t hprime_rounded = vcvtq_u32_f32(hue);
	uint32x4_t x_int = vcvtq_u32_f32(vmulq_n_f32(x, 255.0));
	uint32x4_t c_int = vcvtq_u32_f32(vmulq_n_f32(chroma, 255.0));

	// Again HSV->RGB is also a piecewise function
	uint32x4_t val0 = vorrq_u32(vshlq_n_u32(x_int, 8), vshlq_n_u32(c_int, 16));
	val0 = vandq_u32(val0, vorrq_u32(vceqq_u32(hprime_rounded, vmovq_n_u32(0)), vceqq_u32(hprime_rounded, vmovq_n_u32(6))));
	uint32x4_t val1 = vorrq_u32(vshlq_n_u32(c_int, 8), vshlq_n_u32(x_int, 16));
	val1 = vandq_u32(val1, vceqq_u32(hprime_rounded, vmovq_n_u32(1)));
	uint32x4_t val2 = vorrq_u32(vshlq_n_u32(c_int, 8), x_int);
	val2 = vandq_u32(val2, vceqq_u32(hprime_rounded, vmovq_n_u32(2)));
	uint32x4_t val3 = vorrq_u32(vshlq_n_u32(x_int, 8), c_int);
	val3 = vandq_u32(val3, vceqq_u32(hprime_rounded, vmovq_n_u32(3)));
	uint32x4_t val4 = vorrq_u32(vshlq_n_u32(x_int, 16), c_int);
	val4 = vandq_u32(val4, vceqq_u32(hprime_rounded, vmovq_n_u32(4)));
	uint32x4_t val5 = vorrq_u32(vshlq_n_u32(c_int, 16), x_int);
	val5 = vandq_u32(val5, vceqq_u32(hprime_rounded, vmovq_n_u32(5)));

	// or the values together
	uint32x4_t final = vorrq_u32(val0, vorrq_u32(val1, vorrq_u32(val2, vorrq_u32(val3, vorrq_u32(val4, val5)))));

	// add the minimums back in
	uint32x4_t val_add = vcvtq_u32_f32(vmulq_n_f32(vsubq_f32(val, chroma), 255.0));
	val_add = vorrq_u32(val_add, vorrq_u32(vshlq_n_u32(val_add, 8), vorrq_u32(vshlq_n_u32(val_add, 16), vandq_u32(destCols, vmovq_n_u32(0xff000000)))));
	final = vaddq_u32(final, val_add);
	return final;
}

static inline uint32x4_t blendPixelSIMD(uint32x4_t srcCols, uint32x4_t destCols, uint32x4_t alphas) {
	uint32x4_t srcAlphas, difAlphas, mask, ch1, ch2;
	auto setupArgbAlphas = [&]() {
		// This acts the same as this in the normal blender functions
		// if (alpha == 0)
		//     alpha = aSrc;
		// else
		//     alpha = aSrc * ((alpha & 0xff) + 1) / 256;
		// where alpha is the alpha byte of the srcCols
		srcAlphas = vshrq_n_u32(srcCols, 24);
		difAlphas = vaddq_u32(vandq_u32(alphas, vmovq_n_u32(0xff)), vmovq_n_u32(1));
		difAlphas = vshrq_n_u32(vmulq_u32(srcAlphas, difAlphas), 8);
		difAlphas = vshlq_n_u32(difAlphas, 24);
		srcAlphas = vshlq_n_u32(srcAlphas, 24);
		mask = vceqq_u32(alphas, vmovq_n_u32(0));
		srcAlphas = vandq_u32(srcAlphas, mask);
		difAlphas = vandq_u32(difAlphas, vmvnq_u32(mask));
		srcCols = vandq_u32(srcCols, vmovq_n_u32(0x00ffffff));
		srcCols = vorrq_u32(srcCols, vorrq_u32(srcAlphas, difAlphas));
	};
	switch (_G(_blender_mode)) {
	case kSourceAlphaBlender: // see BITMAP member function blendSourceAlpha
		alphas = vshrq_n_u32(srcCols, 24);
		return rgbBlendSIMD(srcCols, destCols, alphas, false);
	case kArgbToArgbBlender: // see BITMAP member function blendArgbToArgb
		setupArgbAlphas();
		// only blend if alpha isn't 0, otherwise use destCols
		mask = vcgtq_u32(vshrq_n_u32(srcCols, 24), vmovq_n_u32(0));
		ch1 = vandq_u32(argbBlendSIMD(srcCols, destCols), mask);
		ch2 = vandq_u32(destCols, vmvnq_u32(mask));
		return vorrq_u32(ch1, ch2);
	case kArgbToRgbBlender: // see BITMAP member function blendArgbToRgb
		setupArgbAlphas();
		return rgbBlendSIMD(srcCols, destCols, vshrq_n_u32(srcCols, 24), false);
	case kRgbToArgbBlender: // see BITMAP member function blendRgbToArgb
		// if alpha is NOT 0 or 255
		ch2 = vandq_u32(srcCols, vmovq_n_u32(0x00ffffff));
		ch2 = vorrq_u32(ch2, vshlq_n_u32(alphas, 24));
		ch2 = argbBlendSIMD(ch2, destCols);
		// if alpha is 0 or 255
		ch1 = vorrq_u32(srcCols, vmovq_n_u32(0xff000000));
		// mask and or them together
		mask = vorrq_u32(vceqq_u32(alphas, vmovq_n_u32(0)), vceqq_u32(alphas, vmovq_n_u32(0xff)));
		ch1 = vandq_u32(ch1, mask);
		ch2 = vandq_u32(ch2, vmvnq_u32(mask));
		return vorrq_u32(ch1, ch2);
	case kRgbToRgbBlender: // see BITMAP member function blendRgbToRgb
		return rgbBlendSIMD(srcCols, destCols, alphas, false);
	case kAlphaPreservedBlenderMode: // see BITMAP member function blendPreserveAlpha
		return rgbBlendSIMD(srcCols, destCols, alphas, true);
	case kOpaqueBlenderMode: // see BITMAP member function blendOpaque
		return vorrq_u32(srcCols, vmovq_n_u32(0xff000000));
	case kAdditiveBlenderMode: // see BITMAP member function blendAdditiveAlpha
		srcAlphas = vaddq_u32(vshrq_n_u32(srcCols, 24), vshrq_n_u32(destCols, 24));
		srcAlphas = vminq_u32(srcAlphas, vmovq_n_u32(0xff));
		srcCols = vandq_u32(srcCols, vmovq_n_u32(0x00ffffff));
		return vorrq_u32(srcCols, vshlq_n_u32(srcAlphas, 24));
	case kTintBlenderMode: // see BITMAP member function blendTintSprite
		return blendTintSpriteSIMD(srcCols, destCols, alphas, false);
	case kTintLightBlenderMode: // see BITMAP member function blendTintSprite
		return blendTintSpriteSIMD(srcCols, destCols, alphas, true);
	}
	return srcCols;
}

static inline uint16x8_t blendPixelSIMD2Bpp(uint16x8_t srcCols, uint16x8_t destCols, uint16x8_t alphas) {
	uint16x8_t mask, ch1, ch2;
	switch (_G(_blender_mode)) {
	case kSourceAlphaBlender:
	case kOpaqueBlenderMode:
	case kAdditiveBlenderMode:
		return srcCols;
	case kArgbToArgbBlender:
	case kArgbToRgbBlender:
		ch1 = vandq_u16(vmovq_n_u16(0xff), vceqq_u16(alphas, vmovq_n_u16(0)));
		ch2 = vandq_u16(alphas, vcgtq_u16(alphas, vmovq_n_u16(0)));
		alphas = vorrq_u16(ch1, ch2);
		// fall through
	case kRgbToRgbBlender:
	case kAlphaPreservedBlenderMode:
		return rgbBlendSIMD2Bpp(srcCols, destCols, alphas);
	case kRgbToArgbBlender:
		mask = vorrq_u16(vceqq_u16(alphas, vmovq_n_u16(0)), vceqq_u16(alphas, vmovq_n_u16(255)));
		ch1 = vandq_u16(srcCols, mask);
		ch2 = vandq_u16(rgbBlendSIMD2Bpp(srcCols, destCols, alphas), vmvnq_u16(mask));
		return vorrq_u16(ch1, ch2);
	case kTintBlenderMode:
	case kTintLightBlenderMode:
		uint32x4_t srcColsLo = simd2BppTo4Bpp(vget_low_u16(srcCols));
		uint32x4_t srcColsHi = simd2BppTo4Bpp(vget_high_u16(srcCols));
		uint32x4_t destColsLo = simd2BppTo4Bpp(vget_low_u16(destCols));
		uint32x4_t destColsHi = simd2BppTo4Bpp(vget_high_u16(destCols));
		uint32x4_t alphasLo = vmovl_u16(vget_low_u16(alphas));
		uint32x4_t alphasHi = vmovl_u16(vget_high_u16(alphas));
		uint16x4_t lo = simd4BppTo2Bpp(blendTintSpriteSIMD(srcColsLo, destColsLo, alphasLo, _G(_blender_mode) == kTintLightBlenderMode));
		uint16x4_t hi = simd4BppTo2Bpp(blendTintSpriteSIMD(srcColsHi, destColsHi, alphasHi, _G(_blender_mode) == kTintLightBlenderMode));
		return vcombine_u16(lo, hi);
	}
	return srcCols;
}

template<int DestBytesPerPixel, int SrcBytesPerPixel>
static inline void drawPixelSIMD(byte *destPtr, const byte *srcP2, uint32x4_t tint, uint32x4_t alphas, uint32x4_t maskedAlphas, uint32x4_t transColors, int xDir, int xCtrBpp, int srcAlpha, int skipTrans, bool horizFlip, bool useTint, uint32x4_t skipMask) {
	uint32x4_t srcCols, destCol;

	if (DestBytesPerPixel == 4)
		destCol = vld1q_u32((uint32 *)destPtr);
	else
		destCol = simd2BppTo4Bpp(vld1_u16((uint16 *)destPtr));
	if (SrcBytesPerPixel == 4)
		srcCols = vld1q_u32((const uint32 *)(srcP2 + xDir * xCtrBpp));
	else
		srcCols = simd2BppTo4Bpp(vld1_u16((const uint16 *)(srcP2 + xDir * xCtrBpp)));
	// we do this here because we need to check if we should skip the pixel before we blend it
	uint32x4_t mask1 = skipTrans ? vceqq_u32(vandq_u32(srcCols, maskedAlphas), transColors) : vmovq_n_u32(0);
	mask1 = vorrq_u32(mask1, skipMask);
	if (srcAlpha != -1) {
		// take into account for useTint
		if (useTint) {
			srcCols = blendPixelSIMD(tint, srcCols, alphas);
		} else {
			srcCols = blendPixelSIMD(srcCols, destCol, alphas);
		}
	}
	uint32x4_t destCols2 = vandq_u32(destCol, mask1);
	uint32x4_t srcCols2 = vandq_u32(srcCols, vmvnq_u32(mask1));
	uint32x4_t final = vorrq_u32(destCols2, srcCols2);
	if (horizFlip) {
		final = vrev64q_u32(final);
		final = vcombine_u32(vget_high_u32(final), vget_low_u32(final));
	}
	if (DestBytesPerPixel == 4) {
		vst1q_u32((uint32 *)destPtr, final);
	} else {
		vst1_u16((uint16 *)destPtr, simd4BppTo2Bpp(final));
	}
}

static inline void drawPixelSIMD2Bpp(byte *destPtr, const byte *srcP2, uint16x8_t tint, uint16x8_t alphas, uint16x8_t transColors, int xDir, int xCtrBpp, int srcAlpha, int skipTrans, bool horizFlip, bool useTint, uint16x8_t skipMask) {
	uint16x8_t destCol = vld1q_u16((uint16 *)destPtr);
	uint16x8_t srcCols = vld1q_u16((const uint16 *)(srcP2 + xDir * xCtrBpp));
	uint16x8_t mask1 = skipTrans ? vceqq_u16(srcCols, transColors) : vmovq_n_u16(0);
	mask1 = vorrq_u16(mask1, skipMask);
	if (srcAlpha != -1) {
		// take into account for useTint
		if (useTint) {
			srcCols = blendPixelSIMD2Bpp(tint, srcCols, alphas);
		} else {
			srcCols = blendPixelSIMD2Bpp(srcCols, destCol, alphas);
		}
	}
	uint16x8_t destCols2 = vandq_u16(destCol, mask1);
	uint16x8_t srcCols2 = vandq_u16(srcCols, vmvnq_u16(mask1));
	uint16x8_t final = vorrq_u16(destCols2, srcCols2);
	if (horizFlip) {
		final = vrev64q_u16(final);
		final = vcombine_u16(vget_high_u16(final), vget_low_u16(final));
	}
	vst1q_u16((uint16 *)destPtr, final);
}

public:

// This template handles 2bpp and 4bpp, the other specializations handle 1bpp and format conversion blits
template<int DestBytesPerPixel, int SrcBytesPerPixel, bool Scale>
static void drawInner4BppWithConv(BITMAP::DrawInnerArgs &args) {
	const int xDir = args.horizFlip ? -1 : 1;
	byte rSrc, gSrc, bSrc, aSrc;
	byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;
	uint32x4_t tint = vshlq_n_u32(vdupq_n_u32(args.srcAlpha), 24);
	tint = vorrq_u32(tint, vshlq_n_u32(vdupq_n_u32(args.tintRed), 16));
	tint = vorrq_u32(tint, vshlq_n_u32(vdupq_n_u32(args.tintGreen), 8));
	tint = vorrq_u32(tint, vdupq_n_u32(args.tintBlue));
	uint32x4_t maskedAlphas = vmovq_n_u32(args.alphaMask);
	uint32x4_t transColors = vmovq_n_u32(args.transColor);
	uint32x4_t alphas = vmovq_n_u32(args.srcAlpha);

	// This is so that we can calculate what pixels to crop off in a vectorized way
	const uint32x4_t addIndexesNormal = {0, 1, 2, 3};
	const uint32x4_t addIndexesFlipped = {3, 2, 1, 0};
	uint32x4_t addIndexes = args.horizFlip ? addIndexesFlipped : addIndexesNormal;

	// This is so that we can calculate in parallel the pixel indexes for scaled drawing
	uint32x4_t scaleAdds = {0, (uint32)args.scaleX, (uint32)args.scaleX*2, (uint32)args.scaleX*3};

	// Clip the bounds ahead of time (so we don't waste time checking if we are in bounds when
	// we are in the inner loop)
	int xCtrStart = 0, xCtrBppStart = 0, xCtrWidth = args.dstRect.width();
	if (args.xStart + xCtrWidth > args.destArea.w) {
		xCtrWidth = args.destArea.w - args.xStart;
	}
	if (args.xStart < 0) {
		xCtrStart = -args.xStart;
		xCtrBppStart = xCtrStart * SrcBytesPerPixel;
		args.xStart = 0;
	}
	int destY = args.yStart, srcYCtr = 0, yCtr = 0, scaleYCtr = 0, yCtrHeight = args.dstRect.height();
	if (Scale) yCtrHeight = args.dstRect.height();
	if (args.yStart < 0) {
		yCtr = -args.yStart;
		destY = 0;
		if (Scale) {
			scaleYCtr = yCtr * args.scaleY;
			srcYCtr = scaleYCtr / BITMAP::SCALE_THRESHOLD;
		}
	}
	if (args.yStart + yCtrHeight > args.destArea.h) {
		yCtrHeight = args.destArea.h - args.yStart;
	}
	/*if (!Scale && xCtrWidth % 4 != 0) {
		--yCtrHeight;
	}*/

	const int secondToLast = xCtrWidth - 4;

	byte *destP = (byte *)args.destArea.getBasePtr(0, destY);
	const byte *srcP = (const byte *)args.src.getBasePtr(
	                       args.horizFlip ? args.srcArea.right - 4 : args.srcArea.left,
	                       args.vertFlip ? args.srcArea.bottom - 1 - yCtr : args.srcArea.top + yCtr);
	for (; yCtr < yCtrHeight; ++destY, ++yCtr, scaleYCtr += args.scaleY) {
		uint32x4_t xCtrWidthSIMD = vdupq_n_u32(xCtrWidth); // This is the width of the row

		if (!Scale) {
			int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart;
			for (; xCtr < secondToLast; destX += 4, xCtr += 4, xCtrBpp += SrcBytesPerPixel*4) {
				byte *destPtr = &destP[destX * DestBytesPerPixel];
				drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, srcP, tint, alphas, maskedAlphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, vmovq_n_u32(0));
			}

			byte *destPtr = &destP[destX * DestBytesPerPixel];
			uint32x4_t srcCols = vmovq_n_u32(0);
			uint32x4_t destCols = vmovq_n_u32(0);
			memcpy(&srcCols, srcP + xDir * xCtrBpp, (xCtrWidth - xCtr) * SrcBytesPerPixel);
			memcpy(&destCols, destPtr, (xCtrWidth - xCtr) * DestBytesPerPixel);

			// Skip pixels that are beyond the row
			// uint32x4_t skipMask = vcgeq_u32(vaddq_u32(vdupq_n_u32(xCtr), addIndexes), xCtrWidthSIMD);
			drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>((byte *)&destCols, (byte *)&srcCols, tint, alphas, maskedAlphas, transColors, xDir, 0, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, vmovq_n_u32(0));
			memcpy(destPtr, &destCols, (xCtrWidth - xCtr) * DestBytesPerPixel);

			// Goto next row in source and destination image
			destP += args.destArea.pitch;
			srcP += args.vertFlip ? -args.src.pitch : args.src.pitch;
		} else {
			// Here we are scaling the image
			int newSrcYCtr = scaleYCtr / BITMAP::SCALE_THRESHOLD;
			// Since the source yctr might not update every row of the destination, we have
			// to see if we are on a new row...
			if (srcYCtr != newSrcYCtr) {
				int diffSrcYCtr = newSrcYCtr - srcYCtr; // Have we moved yet
				srcP += args.src.pitch * diffSrcYCtr;
				srcYCtr = newSrcYCtr;
			}

			// Now also since we might skip a pixel or 2 or duplicate one to reach the desired
			// scaling size, we create a small dummy buffer that we copy the pixels into and then
			// call the drawPixelsSIMD function
			byte srcBuffer[4*4];
			for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart, scaleXCtr = xCtrStart * args.scaleX; xCtr < xCtrWidth; destX += 4, xCtr += 4, xCtrBpp += SrcBytesPerPixel*4) {
				if (yCtr + 1 == yCtrHeight && xCtr + 4 > xCtrWidth) break; // Don't go past the last 4 pixels
				uint32x4_t indexes = vdupq_n_u32(scaleXCtr);
				// Calculate in parallel the indexes of the pixels
				indexes = vmulq_n_u32(vshrq_n_u32(vaddq_u32(indexes, scaleAdds), BITMAP::SCALE_THRESHOLD_BITS), SrcBytesPerPixel);
				// Simply memcpy them in. memcpy has no real performance overhead here
				memcpy(&srcBuffer[0*(uintptr_t)SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 0), SrcBytesPerPixel);
				memcpy(&srcBuffer[1*(uintptr_t)SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 1), SrcBytesPerPixel);
				memcpy(&srcBuffer[2*(uintptr_t)SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 2), SrcBytesPerPixel);
				memcpy(&srcBuffer[3*(uintptr_t)SrcBytesPerPixel], srcP + vgetq_lane_u32(indexes, 3), SrcBytesPerPixel);
				scaleXCtr += args.scaleX*4;

				// Now this is pretty much the same as before with non-scaled code, except that we use
				// our dummy source buffer instead of the actual source bitmap
				byte *destPtr = &destP[destX * (uintptr_t)DestBytesPerPixel];
				uint32x4_t skipMask = vcgeq_u32(vaddq_u32(vdupq_n_u32(xCtr), addIndexes), xCtrWidthSIMD);
				drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, (const byte *)srcBuffer, tint, alphas, maskedAlphas, transColors, 1, 0, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, skipMask);
			}
			// We calculate every row here except the last (because then we need to
			// check for if we fall off the edge of the row)
			// The only exception here is scaling drawing this is because:
			// 1) if statements are costly, and the less we do the faster this loop is
			// 2) with this, the only branch in the normal drawing loop is the width check
			// 3) the scaling code will actually draw until the last 4 pixels of the image
			//    and do the extra if checks because the scaling code is already much slower
			//    than the normal drawing loop, and the less duplicate code helps here.
			if (yCtr + 1 != yCtrHeight) destP += args.destArea.pitch;
		}
	}

	// Get the last x values of the last row
	int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart;
	// We have a picture that is a multiple of 4, so no extra pixels to draw
	/*if (xCtrWidth % 4 == 0)*/ return;
	// Drawing the last few not scaled pixels here.
	// Same as the loop above but now we check if we are going to overflow,
	// and thus we don't need to mask out pixels that go over the row.
	if (!Scale) {
		for (; xCtr + 4 < xCtrWidth; destX += 4, xCtr += 4, xCtrBpp += SrcBytesPerPixel*4) {
			byte *destPtr = &destP[(ptrdiff_t)destX * DestBytesPerPixel];
			drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, srcP, tint, alphas, maskedAlphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, vmovq_n_u32(0));
		}
		// Because we move in 4 pixel units, and horizFlip moves in 1, we have to move
		// 1 pixel past the last pixel we did not blit, meaning going forward 3 pixels.
		if (args.horizFlip) srcP += SrcBytesPerPixel * 3;
	} else {
		// So if we are scaling, set up the xCtr to what it was before (AKA the last 4 or so pixels of the image)
		xCtr = xCtrWidth - xCtrWidth % 4;
		xCtrBpp = xCtr * SrcBytesPerPixel;
		destX = args.xStart+xCtr;
	}

	// For the last 4 pixels, we just do them in serial, nothing special
	for (; xCtr < xCtrWidth; ++destX, ++xCtr, xCtrBpp += SrcBytesPerPixel) {
		const byte *srcColPtr = (const byte *)(srcP + xDir * xCtrBpp);
		if (Scale) {
			srcColPtr = (const byte *)(srcP + (xCtr * args.scaleX) / BITMAP::SCALE_THRESHOLD * SrcBytesPerPixel);
		}
		byte *destVal = (byte *)&destP[destX * DestBytesPerPixel];
		uint32 srcCol = args.dstBitmap.getColor(srcColPtr, SrcBytesPerPixel);

		// Check if this is a transparent color we should skip
		if (args.skipTrans && ((srcCol & args.alphaMask) == args.transColor))
			continue;

		args.src.format.colorToARGB(srcCol, aSrc, rSrc, gSrc, bSrc);
		if (args.srcAlpha != -1) {
			if (args.useTint) {
				rDest = rSrc;
				gDest = gSrc;
				bDest = bSrc;
				aDest = aSrc;
				rSrc = args.tintRed;
				gSrc = args.tintGreen;
				bSrc = args.tintBlue;
				aSrc = args.srcAlpha;
			}
			args.dstBitmap.blendPixel(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, args.srcAlpha, args.useTint, destVal);
			srcCol = args.dstBitmap.format.ARGBToColor(aDest, rDest, gDest, bDest);
		} else {
			srcCol = args.dstBitmap.format.ARGBToColor(aSrc, rSrc, gSrc, bSrc);
		}
		if (DestBytesPerPixel == 4)
			*(uint32 *)destVal = srcCol;
		else
			*(uint16 *)destVal = srcCol;
	}
}

template<bool Scale>
static void drawInner2Bpp(BITMAP::DrawInnerArgs &args) {
	const int xDir = args.horizFlip ? -1 : 1;
	byte rSrc, gSrc, bSrc, aSrc;
	byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;
	uint16x8_t tint = vdupq_n_u16(args.src.format.ARGBToColor(args.srcAlpha, args.tintRed, args.tintGreen, args.tintBlue));
	uint16x8_t transColors = vdupq_n_u16(args.transColor);
	uint16x8_t alphas = vdupq_n_u16(args.srcAlpha);

	// This is so that we can calculate what pixels to crop off in a vectorized way
	uint16x8_t addIndexesNormal = {0, 1, 2, 3, 4, 5, 6, 7};
	uint16x8_t addIndexesFlipped = {7, 6, 5, 4, 3, 2, 1, 0};
	uint16x8_t addIndexes = args.horizFlip ? addIndexesFlipped : addIndexesNormal;

	// This is so that we can calculate in parallel the pixel indices for scaled drawing
	uint32x4_t scaleAdds = {0, (uint32)args.scaleX, (uint32)args.scaleX*2, (uint32)args.scaleX*3};
	uint32x4_t scaleAdds2 = {(uint32)args.scaleX*4, (uint32)args.scaleX*5, (uint32)args.scaleX*6, (uint32)args.scaleX*7};

	// Clip the bounds ahead of time (so we don't waste time checking if we are in bounds when
	// we are in the inner loop)
	int xCtrStart = 0, xCtrBppStart = 0, xCtrWidth = args.dstRect.width();
	if (args.xStart + xCtrWidth > args.destArea.w) {
		xCtrWidth = args.destArea.w - args.xStart;
	}
	if (args.xStart < 0) {
		xCtrStart = -args.xStart;
		xCtrBppStart = xCtrStart * 2;
		args.xStart = 0;
	}
	int destY = args.yStart, yCtr = 0, srcYCtr = 0, scaleYCtr = 0, yCtrHeight = args.dstRect.height();
	if (Scale) yCtrHeight = args.dstRect.height();
	if (args.yStart < 0) {
		yCtr = -args.yStart;
		destY = 0;
		if (Scale) {
			scaleYCtr = yCtr * args.scaleY;
			srcYCtr = scaleYCtr / BITMAP::SCALE_THRESHOLD;
		}
	}
	if (args.yStart + yCtrHeight > args.destArea.h) {
		yCtrHeight = args.destArea.h - args.yStart;
	}
	/*if (!Scale && xCtrWidth % 8 != 0) {
		--yCtrHeight;
	}*/

	const int secondToLast = xCtrWidth - 8;

	byte *destP = (byte *)args.destArea.getBasePtr(0, destY);
	const byte *srcP = (const byte *)args.src.getBasePtr(
	                       args.horizFlip ? args.srcArea.right - 8 : args.srcArea.left,
	                       args.vertFlip ? args.srcArea.bottom - 1 - yCtr : args.srcArea.top + yCtr);
	for (; yCtr < yCtrHeight; ++destY, ++yCtr, scaleYCtr += args.scaleY) {
		uint16x8_t xCtrWidthSIMD = vmovq_n_u16(xCtrWidth); // This is the width of the row
		if (!Scale) {
			// If we are not scaling the image
			int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart;
			for (; xCtr < secondToLast; destX += 8, xCtr += 8, xCtrBpp += 16) {
				byte *destPtr = &destP[destX * 2];
				drawPixelSIMD2Bpp(destPtr, srcP, tint, alphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, vmovq_n_u16(0));
			}

			byte *destPtr = &destP[destX * 2];
			uint16x8_t srcCols = vmovq_n_u16(0);
			uint16x8_t destCols = vmovq_n_u16(0);
			const int copySize = (xCtrWidth - xCtr) * 2;
			memcpy(&srcCols, srcP + xDir * xCtrBpp, copySize);
			memcpy(&destCols, destPtr, copySize);

			// Skip pixels that are beyond the row
			// uint16x8_t skipMask = vcgeq_u16(vaddq_u16(vdupq_n_u16(xCtr), addIndexes), xCtrWidthSIMD);
			drawPixelSIMD2Bpp((byte *)&destCols, (byte *)&srcCols, tint, alphas, transColors, xDir, 0, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, vmovq_n_u16(0));
			memcpy(destPtr, &destCols, copySize);

			// Goto next row in source and destination image
			destP += args.destArea.pitch;
			srcP += args.vertFlip ? -args.src.pitch : args.src.pitch;
		} else {
			// Here we are scaling the image
			int newSrcYCtr = scaleYCtr / BITMAP::SCALE_THRESHOLD;
			// Since the source yctr might not update every row of the destination, we have
			// to see if we are on a new row...
			if (srcYCtr != newSrcYCtr) {
				int diffSrcYCtr = newSrcYCtr - srcYCtr;
				srcP += args.src.pitch * diffSrcYCtr;
				srcYCtr = newSrcYCtr;
			}

			// Now also since we might skip a pixel or 2 or duplicate one to reach the desired
			// scaling size, we create a small dummy buffer that we copy the pixels into and then
			// call the drawPixelsSIMD function
			uint16 srcBuffer[8];
			for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart, scaleXCtr = xCtrStart * args.scaleX; xCtr < xCtrWidth; destX += 8, xCtr += 8, xCtrBpp += 16) {
				if (yCtr + 1 == yCtrHeight && xCtr + 8 > xCtrWidth) break;
				uint32x4_t indexes = vdupq_n_u32(scaleXCtr), indexes2 = vdupq_n_u32(scaleXCtr);
				// Calculate in parallel the indices of the pixels
				indexes = vmulq_n_u32(vshrq_n_u32(vaddq_u32(indexes, scaleAdds), BITMAP::SCALE_THRESHOLD_BITS), 2);
				indexes2 = vmulq_n_u32(vshrq_n_u32(vaddq_u32(indexes2, scaleAdds2), BITMAP::SCALE_THRESHOLD_BITS), 2);
				// Simply memcpy them in. memcpy has no real performance overhead here
				srcBuffer[0] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes, 0));
				srcBuffer[1] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes, 1));
				srcBuffer[2] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes, 2));
				srcBuffer[3] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes, 3));
				srcBuffer[4] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes2, 0));
				srcBuffer[5] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes2, 1));
				srcBuffer[6] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes2, 2));
				srcBuffer[7] = *(const uint16 *)(srcP + vgetq_lane_u32(indexes2, 3));
				scaleXCtr += args.scaleX*8;

				// Now this is pretty much the same as before with non-scaled code, except that we use
				// our dummy source buffer instead of the actual source bitmap
				byte *destPtr = &destP[destX * 2];
				uint16x8_t skipMask = vcgeq_u16(vaddq_u16(vdupq_n_u16(xCtr), addIndexes), xCtrWidthSIMD);
				drawPixelSIMD2Bpp(destPtr, (const byte *)srcBuffer, tint, alphas, transColors, 1, 0, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, skipMask);
			}
			// We calculate every row here except the last (because then we need to
			// check for if we fall off the edge of the row)
			// The only exception here is scaling drawing this is because:
			// 1) if statements are costly, and the less we do the faster this loop is
			// 2) with this, the only branch in the normal drawing loop is the width check
			// 3) the scaling code will actually draw until the last 4 pixels of the image
			//    and do the extra if checks because the scaling code is already much slower
			//    than the normal drawing loop, and the less duplicate code helps here.
			if (yCtr + 1 != yCtrHeight) destP += args.destArea.pitch;
		}
	}

	// We have a picture that is a multiple of 8, so no extra pixels to draw
	/*if (xCtrWidth % 8 == 0)*/ return;
	// Get the last x values of the last row
	int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart;
	// Drawing the last few not scaled pixels here.
	// Same as the loop above but now we check if we are going to overflow,
	// and thus we don't need to mask out pixels that go over the row.
	if (!Scale) {
		for (; xCtr + 8 < xCtrWidth; destX += 8, xCtr += 8, xCtrBpp += 16) {
			byte *destPtr = &destP[destX * 2];
			drawPixelSIMD2Bpp(destPtr, srcP, tint, alphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, vmovq_n_u16(0));
		}
		// Because we move in 8 pixel units, and horizFlip moves in 1, we have to move
		// 1 pixel past the last pixel we did not blit, meaning going forward 7 pixels.
		if (args.horizFlip) srcP += 2 * 7;
	} else {
		// So if we are scaling, set up the xCtr to what it was before (AKA the last 8 or so pixels of the image)
		xCtr = xCtrWidth - xCtrWidth % 8;
		xCtrBpp = xCtr * 2;
		destX = args.xStart+xCtr;
	}

	// For the last 4 pixels, we just do them in serial, nothing special
	for (; xCtr < xCtrWidth; ++destX, ++xCtr, xCtrBpp += 2) {
		const byte *srcColPtr = (const byte *)(srcP + xDir * xCtrBpp);
		if (Scale) {
			srcColPtr = (const byte *)(srcP + (xCtr * args.scaleX) / BITMAP::SCALE_THRESHOLD * 2);
		}
		byte *destVal = (byte *)&destP[destX * 2];
		uint32 srcCol = (uint32)(*(const uint16 *)srcColPtr);

		// Check if this is a transparent color we should skip
		if (args.skipTrans && srcCol == args.transColor)
			continue;

		args.src.format.colorToARGB(srcCol, aSrc, rSrc, gSrc, bSrc);
		if (args.srcAlpha != -1) {
			if (args.useTint) {
				rDest = rSrc;
				gDest = gSrc;
				bDest = bSrc;
				aDest = aSrc;
				rSrc = args.tintRed;
				gSrc = args.tintGreen;
				bSrc = args.tintBlue;
				aSrc = args.srcAlpha;
			}/* else {
				format.colorToARGB((uint32)(*(uint16 *)destVal), aDest, rDest, gDest, bDest);
			}*/
			args.dstBitmap.blendPixel(aSrc, rSrc, gSrc, bSrc, aDest, rDest, gDest, bDest, args.srcAlpha, args.useTint, destVal);
			srcCol = args.dstBitmap.format.ARGBToColor(aDest, rDest, gDest, bDest);
		} else {
			srcCol = args.dstBitmap.format.ARGBToColor(aSrc, rSrc, gSrc, bSrc);
		}
		*(uint16 *)destVal = srcCol;
	}
}

template<bool Scale>
static void drawInner1Bpp(BITMAP::DrawInnerArgs &args) {
	const int xDir = args.horizFlip ? -1 : 1;
	uint8x16_t transColors = vmovq_n_u8(args.transColor);

	// This is so that we can calculate in parallel the pixel indices for scaled drawing
	uint32x4_t scaleAdds1 = {0, (uint32)args.scaleX, (uint32)args.scaleX*2, (uint32)args.scaleX*3};
	uint32x4_t scaleAdds2 = {(uint32)args.scaleX*4, (uint32)args.scaleX*5, (uint32)args.scaleX*6, (uint32)args.scaleX*7};
	uint32x4_t scaleAdds3 = {(uint32)args.scaleX*8, (uint32)args.scaleX*9, (uint32)args.scaleX*10, (uint32)args.scaleX*11};
	uint32x4_t scaleAdds4 = {(uint32)args.scaleX*12, (uint32)args.scaleX*13, (uint32)args.scaleX*14, (uint32)args.scaleX*15};

	// Clip the bounds ahead of time (so we don't waste time checking if we are in bounds when
	// we are in the inner loop)
	int xCtrStart = 0, xCtrWidth = args.dstRect.width();
	if (args.xStart + xCtrWidth > args.destArea.w) {
		xCtrWidth = args.destArea.w - args.xStart;
	}
	if (args.xStart < 0) {
		xCtrStart = -args.xStart;
		args.xStart = 0;
	}
	int destY = args.yStart, yCtr = 0, srcYCtr = 0, scaleYCtr = 0, yCtrHeight = args.dstRect.height();
	if (Scale) yCtrHeight = args.dstRect.height();
	if (args.yStart < 0) {
		yCtr = -args.yStart;
		destY = 0;
		if (Scale) {
			scaleYCtr = yCtr * args.scaleY;
			srcYCtr = scaleYCtr / BITMAP::SCALE_THRESHOLD;
		}
	}
	if (args.yStart + yCtrHeight > args.destArea.h) {
		yCtrHeight = args.destArea.h - args.yStart;
	}

	byte *destP = (byte *)args.destArea.getBasePtr(0, destY);
	const byte *srcP = (const byte *)args.src.getBasePtr(
	                       args.horizFlip ? args.srcArea.right - 16 : args.srcArea.left,
	                       args.vertFlip ? args.srcArea.bottom - 1 - yCtr : args.srcArea.top + yCtr);
	for (; yCtr < yCtrHeight; ++destY, ++yCtr, scaleYCtr += args.scaleY) {
		if (Scale) {
			// So here we update the srcYCtr differently due to this being for
			// scaling
			int newSrcYCtr = scaleYCtr / BITMAP::SCALE_THRESHOLD;
			if (srcYCtr != newSrcYCtr) {
				// Since the source yctr might not update every row of the destination, we have
				// to see if we are on a new row...
				int diffSrcYCtr = newSrcYCtr - srcYCtr;
				srcP += args.src.pitch * diffSrcYCtr;
				srcYCtr = newSrcYCtr;
			}
		}
		int xCtr = xCtrStart, destX = args.xStart, scaleXCtr = xCtrStart * args.scaleX;
		for (; xCtr + 16 < xCtrWidth; destX += 16, xCtr += 16) {
			byte *destPtr = &destP[destX];

			// Here we don't use the drawPixelSIMD function because 1bpp bitmaps in allegro
			// can't have any blending applied to them
			uint8x16_t destCols = vld1q_u8(destPtr);
			uint8x16_t srcCols = vld1q_u8(srcP + xDir * xCtr);
			if (Scale) {
				// If we are scaling, we have to set each pixel individually
				uint32x4_t indexes1 = vdupq_n_u32(scaleXCtr), indexes2 = vdupq_n_u32(scaleXCtr);
				uint32x4_t indexes3 = vdupq_n_u32(scaleXCtr), indexes4 = vdupq_n_u32(scaleXCtr);
				indexes1 = vshrq_n_u32(vaddq_u32(indexes1, scaleAdds1), BITMAP::SCALE_THRESHOLD_BITS);
				indexes2 = vshrq_n_u32(vaddq_u32(indexes2, scaleAdds2), BITMAP::SCALE_THRESHOLD_BITS);
				indexes3 = vshrq_n_u32(vaddq_u32(indexes3, scaleAdds3), BITMAP::SCALE_THRESHOLD_BITS);
				indexes4 = vshrq_n_u32(vaddq_u32(indexes4, scaleAdds4), BITMAP::SCALE_THRESHOLD_BITS);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes1, 0)], srcCols, 0);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes1, 1)], srcCols, 1);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes1, 2)], srcCols, 2);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes1, 3)], srcCols, 3);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes2, 0)], srcCols, 4);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes2, 1)], srcCols, 5);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes2, 2)], srcCols, 6);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes2, 3)], srcCols, 7);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes3, 0)], srcCols, 8);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes3, 1)], srcCols, 9);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes3, 2)], srcCols, 10);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes3, 3)], srcCols, 11);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes4, 0)], srcCols, 12);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes4, 1)], srcCols, 13);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes4, 2)], srcCols, 14);
				srcCols = vsetq_lane_u8(srcP[vgetq_lane_u32(indexes4, 3)], srcCols, 15);
				scaleXCtr += args.scaleX*16;
			}

			// Mask out transparent pixels
			uint8x16_t mask1 = args.skipTrans ? vceqq_u8(srcCols, transColors) : vmovq_n_u8(0);
			uint8x16_t final = vorrq_u8(vandq_u8(srcCols, vmvnq_u8(mask1)), vandq_u8(destCols, mask1));
			if (args.horizFlip) {
				final = vrev64q_u8(final);
				final = vcombine_u8(vget_high_u8(final), vget_low_u8(final));
			}
			vst1q_u8(destPtr, final);
		}
		// Get the last x values

		// Because we move in 16 pixel units, and horizFlip moves in 1, we have to move
		// 1 pixel past the last pixel we did not blit, meaning going forward 15 pixels.
		if (args.horizFlip) srcP += 15;
		for (; xCtr < xCtrWidth; ++destX, ++xCtr, scaleXCtr += args.scaleX) {
			const byte *srcCol = (const byte *)(srcP + xDir * xCtr);
			if (Scale) {
				srcCol = (const byte *)(srcP + scaleXCtr / BITMAP::SCALE_THRESHOLD);
			}
			// Check if this is a transparent color we should skip
			if (args.skipTrans && *srcCol == args.transColor)
				continue;

			byte *destVal = (byte *)&destP[destX];
			*destVal = *srcCol;
		}
		if (args.horizFlip) srcP -= 15; // Undo what we did up there
		destP += args.destArea.pitch; // Go to next row
		// Only advance the src row by 1 every time like this if we don't scale
		if (!Scale) srcP += args.vertFlip ? -args.src.pitch : args.src.pitch;
	}
}

}; // end of class DrawInnerImpl_NEON

template<bool Scale>
void BITMAP::drawNEON(DrawInnerArgs &args) {
	if (args.sameFormat) {
		switch (format.bytesPerPixel) {
		case 1: DrawInnerImpl_NEON::drawInner1Bpp<Scale>(args); break;
		case 2: DrawInnerImpl_NEON::drawInner2Bpp<Scale>(args); break;
		case 4: DrawInnerImpl_NEON::drawInner4BppWithConv<4, 4, Scale>(args); break;
		}
	} else if (format.bytesPerPixel == 4 && args.src.format.bytesPerPixel == 2) {
		DrawInnerImpl_NEON::drawInner4BppWithConv<4, 2, Scale>(args);
	} else if (format.bytesPerPixel == 2 && args.src.format.bytesPerPixel == 4) {
		DrawInnerImpl_NEON::drawInner4BppWithConv<2, 4, Scale>(args);
	}
}

template void BITMAP::drawNEON<false>(DrawInnerArgs &);
template void BITMAP::drawNEON<true>(DrawInnerArgs &);

} // namespace AGS3

#if !defined(__aarch64__)

#if defined(__clang__)
#pragma clang attribute pop
#elif defined(__GNUC__)
#pragma GCC pop_options
#endif

#endif // !defined(__aarch64__)

#endif // SCUMMVM_NEON
