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
#include "ags/globals.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/flood.h"
#include "ags/lib/allegro/gfx.h"
#include "common/textconsole.h"
#include "graphics/screen.h"

#include <emmintrin.h>

#if !defined(__x86_64__)

#if defined(__clang__)
#pragma clang attribute push (__attribute__((target("sse2"))), apply_to=function)
#elif defined(__GNUC__)
#pragma GCC push_options
#pragma GCC target("sse2")
#endif

#endif // !defined(__x86_64__)

namespace AGS3 {

class DrawInnerImpl_SSE2 {

static inline __m128i simd2BppTo4Bpp(__m128i pixels) {
	__m128i x = _mm_unpacklo_epi16(pixels, _mm_setzero_si128());

	// c is the extracted 5/6 bit color from the image
	__m128i c = _mm_srli_epi32(x, 11);

	// We convert it back to normal by shifting it thrice over, naturally, and then using the 2 most
	// sinificant bits in the original color for the least significant bits in the new one
	__m128i r = _mm_slli_epi32(_mm_or_si128(_mm_slli_epi32(c, 3), _mm_srli_epi32(c, 2)), 16);
	c = _mm_srli_epi32(_mm_and_si128(x, _mm_set1_epi32(0x07e0)), 5);
	__m128i g = _mm_slli_epi32(_mm_or_si128(_mm_slli_epi32(c, 2), _mm_srli_epi32(c, 4)), 8);
	c = _mm_and_si128(x, _mm_set1_epi32(0x001f));
	__m128i b = _mm_or_si128(_mm_slli_epi32(c, 3), _mm_srli_epi32(c, 2));

	// By default 2bpp to 4bpp makes the alpha channel 255
	return _mm_or_si128(_mm_or_si128(_mm_or_si128(r, g), b), _mm_set1_epi32(0xff000000));
}

static inline __m128i simd4BppTo2Bpp(__m128i pixels) {
	// x is the final 16 bit rgb pixel
	__m128i x = _mm_srli_epi32(_mm_and_si128(pixels, _mm_set1_epi32(0x000000ff)), 3);
	x = _mm_or_si128(x, _mm_slli_epi32(_mm_srli_epi32(_mm_and_si128(pixels, _mm_set1_epi32(0x0000ff00)), 8+2), 5));
	x = _mm_or_si128(x, _mm_slli_epi32(_mm_srli_epi32(_mm_and_si128(pixels, _mm_set1_epi32(0x00ff0000)), 16+3), 11));
	x = _mm_slli_epi32(x, 16);
	x = _mm_srai_epi32(x, 16);
	return _mm_packs_epi32(x, _mm_setzero_si128());
}

static inline __m128i rgbBlendSIMD2Bpp(__m128i srcCols, __m128i destCols, __m128i alphas) {
	// Here we add 1 to alphas if its 0. This is what the original blender function did
	alphas = _mm_add_epi16(alphas, _mm_and_si128(_mm_cmpgt_epi16(alphas, _mm_setzero_si128()), _mm_set1_epi16(1)));

	// Split the components into rgb
	__m128i srcComps[] = {
		_mm_and_si128(srcCols, _mm_set1_epi16(0x1f)),		    		 // B
		_mm_and_si128(_mm_srli_epi16(srcCols, 5), _mm_set1_epi16(0x3f)), // G
		_mm_srli_epi16(srcCols, 11),									 // R
	}, destComps[] = {
		_mm_and_si128(destCols, _mm_set1_epi16(0x1f)),		    		  // B
		_mm_and_si128(_mm_srli_epi16(destCols, 5), _mm_set1_epi16(0x3f)), // G
		_mm_srli_epi16(destCols, 11),									  // R
	};

	// Calculate the differences between the colors
	__m128i diffs[] = {
		_mm_sub_epi16(srcComps[0], destComps[0]), // B
		_mm_sub_epi16(srcComps[1], destComps[1]), // G
		_mm_sub_epi16(srcComps[2], destComps[2]), // R
	};

	// Multiply by alpha and shift depth bits to the right
	// pretty much the same as (int)(((float)component / 255.0f) * ((float)alpha / 255.0f) * 255.0f)
	alphas = _mm_srli_epi16(alphas, 2);
	diffs[1] = _mm_srli_epi16(_mm_mullo_epi16(diffs[1], alphas), 6);
	alphas = _mm_srli_epi16(alphas, 1);
	diffs[0] = _mm_srli_epi16(_mm_mullo_epi16(diffs[0], alphas), 5);
	diffs[2] = _mm_srli_epi16(_mm_mullo_epi16(diffs[2], alphas), 5);

	// Here we add the difference between the 2 colors times alpha onto the destination
	diffs[0] = _mm_and_si128(_mm_add_epi16(diffs[0], destComps[0]), _mm_set1_epi16(0x1f));
	diffs[1] = _mm_and_si128(_mm_add_epi16(diffs[1], destComps[1]), _mm_set1_epi16(0x3f));
	diffs[2] = _mm_and_si128(_mm_add_epi16(diffs[2], destComps[2]), _mm_set1_epi16(0x1f));

	// We compile all the colors into diffs[0] as a 16 bit rgb pixel
	diffs[0] = _mm_or_si128(diffs[0], _mm_slli_epi16(diffs[1], 5));
	return _mm_or_si128(diffs[0], _mm_slli_epi16(diffs[2], 11));
}

static inline __m128i mul32_as32(__m128i a, __m128i b) {
	__m128i tmp1 = _mm_mul_epu32(a,b);
	__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(a,4), _mm_srli_si128(b,4));
	return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE (0,0,2,0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE (0,0,2,0))); /* shuffle results to [63..0] and pack */
}

// preserveAlpha:
//		false => set destCols's alpha to 0
// 		true => keep destCols's alpha
static inline __m128i rgbBlendSIMD(__m128i srcCols, __m128i destCols, __m128i alphas, bool preserveAlpha) {
	// Here we add 1 to alphas if its 0. This is what the original blender function did.
	alphas = _mm_add_epi32(alphas, _mm_and_si128(_mm_cmpgt_epi32(alphas, _mm_setzero_si128()), _mm_set1_epi32(1)));

	// Get the alpha from the destination
	__m128i alpha = _mm_and_si128(destCols, _mm_set1_epi32(0xff000000));

	// Get red and blue components
	__m128i srcColsCopy = srcCols;
	srcColsCopy = _mm_and_si128(srcColsCopy, _mm_set1_epi32(0xff00ff));
	__m128i destColsCopy = destCols;
	destColsCopy = _mm_and_si128(destColsCopy, _mm_set1_epi32(0xff00ff));

	// Compute the difference, then multiply by alpha and divide by 256
	srcColsCopy = _mm_sub_epi32(srcColsCopy, destColsCopy);
	srcColsCopy = mul32_as32(srcColsCopy, alphas);
	//srcColsCopy = _mm_mul_epi32(srcColsCopy, alphas);
	srcColsCopy = _mm_srli_epi32(srcColsCopy, 8);
	srcColsCopy = _mm_add_epi32(srcColsCopy, destCols); // Add the new red/blue to the old red/blue

	// Do the same for the green component
	srcCols = _mm_and_si128(srcCols, _mm_set1_epi32(0xff00));
	destCols = _mm_and_si128(destCols, _mm_set1_epi32(0xff00));
	srcCols = _mm_sub_epi32(srcCols, destCols);
	srcCols = mul32_as32(srcCols, alphas);
	//srcCols = _mm_mul_epi32(srcCols, alphas);
	srcCols = _mm_srli_epi32(srcCols, 8);
	srcCols = _mm_add_epi32(srcCols, destCols); // Add the new green to the old green

	// Keep values in 8bit range and glue red/blue and green together
	srcColsCopy = _mm_and_si128(srcColsCopy, _mm_set1_epi32(0xff00ff));
	srcCols = _mm_and_si128(srcCols, _mm_set1_epi32(0xff00));
	srcCols = _mm_or_si128(srcCols, srcColsCopy);

	// Remember that alpha is not alphas, but rather the alpha of destcols
	if (preserveAlpha) {
		srcCols = _mm_and_si128(srcCols, _mm_set1_epi32(0x00ffffff));
		srcCols = _mm_or_si128(srcCols, alpha);
	}
	return srcCols;
}

static inline __m128i argbBlendSIMD(__m128i srcCols, __m128i destCols) {
	__m128 srcA = _mm_cvtepi32_ps(_mm_srli_epi32(srcCols, 24));
	srcA = _mm_mul_ps(srcA, _mm_set1_ps(1.0f / 255.0f));
	__m128 srcR = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(srcCols, 16), _mm_set1_epi32(0xff)));
	__m128 srcG = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(srcCols, 8), _mm_set1_epi32(0xff)));
	__m128 srcB = _mm_cvtepi32_ps(_mm_and_si128(srcCols, _mm_set1_epi32(0xff)));

	__m128 destA = _mm_cvtepi32_ps(_mm_srli_epi32(destCols, 24));
	destA = _mm_mul_ps(destA, _mm_set1_ps(1.0f / 255.0f));
	__m128 destR = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(destCols, 16), _mm_set1_epi32(0xff)));
	__m128 destG = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(destCols, 8), _mm_set1_epi32(0xff)));
	__m128 destB = _mm_cvtepi32_ps(_mm_and_si128(destCols, _mm_set1_epi32(0xff)));

	// the destination alpha gets multiplied by 255 - source alpha
	destA = _mm_mul_ps(destA, _mm_sub_ps(_mm_set1_ps(1.0f), srcA));

	// ((src * sAlpha) + (dest * dAlpha)) / (sAlpha + dAlpha)
	__m128 combA = _mm_add_ps(srcA, destA);
	__m128 combArcp = _mm_rcp_ps(combA);
	destR = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(srcR, srcA), _mm_mul_ps(destR, destA)), combArcp);
	destG = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(srcG, srcA), _mm_mul_ps(destG, destA)), combArcp);
	destB = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(srcB, srcA), _mm_mul_ps(destB, destA)), combArcp);
	combA = _mm_mul_ps(combA, _mm_set1_ps(255.0));

	// Now put it back together
	return _mm_or_si128(_mm_slli_epi32(_mm_cvtps_epi32(combA), 24),
		_mm_or_si128(_mm_slli_epi32(_mm_cvtps_epi32(destR), 16),
		_mm_or_si128(_mm_slli_epi32(_mm_cvtps_epi32(destG), 8),
			_mm_cvtps_epi32(destB))));
}

static inline __m128i blendTintSpriteSIMD(__m128i srcCols, __m128i destCols, __m128i alphas, bool light) {
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
	__m128 ddr, ddg, ddb;
	ddr = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(destCols, 16), _mm_set1_epi32(0xff))), _mm_set1_ps(1.0f / 255.0f));
	ddg = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(destCols, 8), _mm_set1_epi32(0xff))), _mm_set1_ps(1.0f / 255.0f));
	ddb = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(destCols, _mm_set1_epi32(0xff))), _mm_set1_ps(1.0f / 255.0f));
	__m128 ssr, ssg, ssb;
	ssr = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(srcCols, 16), _mm_set1_epi32(0xff))), _mm_set1_ps(1.0f / 255.0f));
	ssg = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(srcCols, 8), _mm_set1_epi32(0xff))), _mm_set1_ps(1.0f / 255.0f));
	ssb = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(srcCols, _mm_set1_epi32(0xff))), _mm_set1_ps(1.0f / 255.0f));

	// Get the maxes and mins (needed for HSV->RGB and visa-versa)
	__m128 dmaxes = _mm_max_ps(ddr, _mm_max_ps(ddg, ddb));
	__m128 smaxes = _mm_max_ps(ssr, _mm_max_ps(ssg, ssb));
	__m128 smins = _mm_min_ps(ssr, _mm_min_ps(ssg, ssb));

	// This is here to stop from dividing by 0
	const __m128 eplison0 = _mm_set1_ps(0.0000001f);

	__m128 chroma = _mm_max_ps(_mm_sub_ps(smaxes, smins), eplison0);

	// RGB to HSV is a piecewise function, so we compute each part of the function first...
	__m128 hr, hg, hb, hue;
	hr = _mm_div_ps(_mm_sub_ps(ssg, ssb), chroma);
	hr = _mm_sub_ps(hr, _mm_mul_ps(_mm_cvtepi32_ps(_mm_cvtps_epi32(_mm_mul_ps(hr, _mm_set1_ps(1.0f / 6.0f)))), _mm_set1_ps(6.0f)));
	hr = _mm_add_ps(hr, _mm_and_ps(_mm_cmplt_ps(hr, _mm_setzero_ps()), _mm_set1_ps(6.0f)));
	hg = _mm_add_ps(_mm_div_ps(_mm_sub_ps(ssb, ssr), chroma), _mm_set1_ps(2.0f));
	hg = _mm_max_ps(hg, _mm_setzero_ps());
	hb = _mm_add_ps(_mm_div_ps(_mm_sub_ps(ssr, ssg), chroma), _mm_set1_ps(4.0f));
	hb = _mm_max_ps(hb, _mm_setzero_ps());

	// And then compute which one will be used based on criteria
	__m128 hrfactors = _mm_and_ps(_mm_and_ps(_mm_cmpeq_ps(ssr, smaxes), _mm_cmpneq_ps(ssr, ssb)), _mm_set1_ps(1.0f));
	__m128 hgfactors = _mm_and_ps(_mm_and_ps(_mm_cmpeq_ps(ssg, smaxes), _mm_cmpneq_ps(ssg, ssr)), _mm_set1_ps(1.0f));
	__m128 hbfactors = _mm_and_ps(_mm_and_ps(_mm_cmpeq_ps(ssb, smaxes), _mm_cmpneq_ps(ssb, ssg)), _mm_set1_ps(1.0f));
	hue = _mm_mul_ps(hr, hrfactors);
	hue = _mm_add_ps(hue, _mm_mul_ps(hg, hgfactors));
	hue = _mm_add_ps(hue, _mm_mul_ps(hb, hbfactors));

	// Mess with the light like the original function
	__m128 val = dmaxes;
	if (light) {
		val = _mm_sub_ps(val, _mm_sub_ps(_mm_set1_ps(1.0f), _mm_mul_ps(_mm_cvtepi32_ps(alphas), _mm_set1_ps(1.0f / 250.0f))));
		val = _mm_max_ps(val, _mm_setzero_ps());
	}

	// then it stiches the HSV back together
	// the hue and saturation come from the source (tint) color, and the value comes from
	// the destinaion (real source) color
	chroma = _mm_mul_ps(val, _mm_div_ps(_mm_sub_ps(smaxes, smins), _mm_add_ps(smaxes, eplison0)));
	__m128 hprime_mod2 = _mm_mul_ps(hue, _mm_set1_ps(1.0f / 2.0f));
	hprime_mod2 = _mm_mul_ps(_mm_sub_ps(hprime_mod2, _mm_cvtepi32_ps(_mm_cvtps_epi32(_mm_sub_ps(hprime_mod2, _mm_set1_ps(0.5))))), _mm_set1_ps(2.0f));
	__m128 x = _mm_mul_ps(chroma, _mm_sub_ps(_mm_set1_ps(1), _mm_and_ps(_mm_castsi128_ps(_mm_set1_epi32(0x7fffffff)), _mm_sub_ps(hprime_mod2, _mm_set1_ps(1)))));
	//float32x4_t x = vmulq_f32(chroma, vsubq_f32(vmovq_n_f32(1.0f), vabsq_f32(vsubq_f32(hprime_mod2, vmovq_n_f32(1.0f)))));
	__m128i hprime_rounded = _mm_cvtps_epi32(_mm_sub_ps(hue, _mm_set1_ps(0.5)));
	__m128i x_int = _mm_cvtps_epi32(_mm_mul_ps(x, _mm_set1_ps(255.0f)));
	__m128i c_int = _mm_cvtps_epi32(_mm_mul_ps(chroma, _mm_set1_ps(255.0f)));

	// Again HSV->RGB is also a piecewise function
	__m128i val0 = _mm_or_si128(_mm_slli_epi32(x_int, 8), _mm_slli_epi32(c_int, 16));
	val0 = _mm_and_si128(val0, _mm_or_si128(_mm_cmpeq_epi32(hprime_rounded, _mm_set1_epi32(0)), _mm_cmpeq_epi32(hprime_rounded, _mm_set1_epi32(6))));
	__m128i val1 = _mm_or_si128(_mm_slli_epi32(c_int, 8), _mm_slli_epi32(x_int, 16));
	val1 = _mm_and_si128(val1, _mm_cmpeq_epi32(hprime_rounded, _mm_set1_epi32(1)));
	__m128i val2 = _mm_or_si128(_mm_slli_epi32(c_int, 8), x_int);
	val2 = _mm_and_si128(val2, _mm_cmpeq_epi32(hprime_rounded, _mm_set1_epi32(2)));
	__m128i val3 = _mm_or_si128(_mm_slli_epi32(x_int, 8), c_int);
	val3 = _mm_and_si128(val3, _mm_cmpeq_epi32(hprime_rounded, _mm_set1_epi32(3)));
	__m128i val4 = _mm_or_si128(_mm_slli_epi32(x_int, 16), c_int);
	val4 = _mm_and_si128(val4, _mm_cmpeq_epi32(hprime_rounded, _mm_set1_epi32(4)));
	__m128i val5 = _mm_or_si128(_mm_slli_epi32(c_int, 16), x_int);
	val5 = _mm_and_si128(val5, _mm_cmpeq_epi32(hprime_rounded, _mm_set1_epi32(5)));

	// or the values together
	__m128i final = _mm_or_si128(val0, _mm_or_si128(val1, _mm_or_si128(val2, _mm_or_si128(val3, _mm_or_si128(val4, val5)))));

	// add the minimums back in
	__m128i val_add = _mm_cvtps_epi32(_mm_mul_ps(_mm_sub_ps(val, chroma), _mm_set1_ps(255.0f)));
	val_add = _mm_or_si128(val_add, _mm_or_si128(_mm_slli_epi32(val_add, 8), _mm_or_si128(_mm_slli_epi32(val_add, 16), _mm_and_si128(destCols, _mm_set1_epi32(0xff000000)))));
	final = _mm_add_epi32(final, val_add);
	return final;
}

static inline __m128i mul32_as16(__m128i a, __m128i b) {
	__m128i a16 = _mm_packs_epi32(a, _mm_setzero_si128());
	__m128i b16 = _mm_packs_epi32(b, _mm_setzero_si128());
	__m128i res = _mm_mullo_epi16(a16, b16);
	return _mm_unpacklo_epi16(res, _mm_setzero_si128());
}

static inline __m128i findmin32_as16(__m128i a, __m128i b) {
	__m128i a16 = _mm_packs_epi32(a, _mm_setzero_si128());
	__m128i b16 = _mm_packs_epi32(b, _mm_setzero_si128());
	__m128i res = _mm_min_epi16(a16, b16);
	return _mm_unpacklo_epi16(res, _mm_setzero_si128());
}

static inline __m128i blendPixelSIMD(__m128i srcCols, __m128i destCols, __m128i alphas) {
	__m128i srcAlphas, difAlphas, mask, ch1, ch2;
	auto setupArgbAlphas = [&]() {
		// This acts the same as this in the normal blender functions
		// if (alpha == 0)
		//     alpha = aSrc;
		// else
		//     alpha = aSrc * ((alpha & 0xff) + 1) / 256;
		// where alpha is the alpha byte of the srcCols
		srcAlphas = _mm_srli_epi32(srcCols, 24);
		difAlphas = _mm_add_epi32(_mm_and_si128(alphas, _mm_set1_epi32(0xff)), _mm_set1_epi32(1));
		difAlphas = _mm_srli_epi32(mul32_as16(srcAlphas, difAlphas), 8);
		difAlphas = _mm_slli_epi32(difAlphas, 24);
		srcAlphas = _mm_slli_epi32(srcAlphas, 24);
		mask = _mm_cmpeq_epi32(alphas, _mm_setzero_si128());
		srcAlphas = _mm_and_si128(srcAlphas, mask);
		difAlphas = _mm_andnot_si128(mask, difAlphas);
		srcCols = _mm_and_si128(srcCols, _mm_set1_epi32(0x00ffffff));
		srcCols = _mm_or_si128(srcCols, _mm_or_si128(srcAlphas, difAlphas));
	};
	switch (_G(_blender_mode)) {
	case kSourceAlphaBlender: // see BITMAP member function blendSourceAlpha
		alphas = _mm_srli_epi32(srcCols, 24);
		return rgbBlendSIMD(srcCols, destCols, alphas, false);
	case kArgbToArgbBlender: // see BITMAP member function blendArgbToArgb
		setupArgbAlphas();
		// only blend if alpha isn't 0, otherwise use destCols
		mask = _mm_cmpgt_epi32(_mm_srli_epi32(srcCols, 24), _mm_setzero_si128());
		ch1 = _mm_and_si128(argbBlendSIMD(srcCols, destCols), mask);
		ch2 = _mm_andnot_si128(mask, destCols);
		return _mm_or_si128(ch1, ch2);
	case kArgbToRgbBlender: // see BITMAP member function blendArgbToRgb
		setupArgbAlphas();
		return rgbBlendSIMD(srcCols, destCols, _mm_srli_epi32(srcCols, 24), false);
	case kRgbToArgbBlender: // see BITMAP member function blendRgbToArgb
		// if alpha is NOT 0 or 255
		ch2 = _mm_and_si128(srcCols, _mm_set1_epi32(0x00ffffff));
		ch2 = _mm_or_si128(ch2, _mm_slli_epi32(alphas, 24));
		ch2 = argbBlendSIMD(ch2, destCols);
		// if alpha is 0 or 255
		ch1 = _mm_or_si128(srcCols, _mm_set1_epi32(0xff000000));
		// mask and or them together
		mask = _mm_or_si128(_mm_cmpeq_epi32(alphas, _mm_setzero_si128()), _mm_cmpeq_epi32(alphas, _mm_set1_epi32(0xff)));
		ch1 = _mm_and_si128(ch1, mask);
		ch2 = _mm_andnot_si128(mask, ch2);
		return _mm_or_si128(ch1, ch2);
	case kRgbToRgbBlender: // see BITMAP member function blendRgbToRgb
		return rgbBlendSIMD(srcCols, destCols, alphas, false);
	case kAlphaPreservedBlenderMode: // see BITMAP member function blendPreserveAlpha
		return rgbBlendSIMD(srcCols, destCols, alphas, true);
	case kOpaqueBlenderMode: // see BITMAP member function blendOpaque
		return _mm_or_si128(srcCols, _mm_set1_epi32(0xff000000));
	case kAdditiveBlenderMode: // see BITMAP member function blendAdditiveAlpha
		srcAlphas = _mm_add_epi32(_mm_srli_epi32(srcCols, 24), _mm_srli_epi32(destCols, 24));
		srcAlphas = findmin32_as16(srcAlphas, _mm_set1_epi32(0xff));
		srcCols = _mm_and_si128(srcCols, _mm_set1_epi32(0x00ffffff));
		return _mm_or_si128(srcCols, _mm_slli_epi32(srcAlphas, 24));
	case kTintBlenderMode: // see BITMAP member function blendTintSprite
		return blendTintSpriteSIMD(srcCols, destCols, alphas, false);
	case kTintLightBlenderMode: // see BITMAP member function blendTintSprite
		return blendTintSpriteSIMD(srcCols, destCols, alphas, true);
	}
	return _mm_setzero_si128();
}

static inline __m128i blendPixelSIMD2Bpp(__m128i srcCols, __m128i destCols, __m128i alphas) {
	__m128i mask, ch1, ch2;
	switch (_G(_blender_mode)) {
	case kSourceAlphaBlender:
	case kOpaqueBlenderMode:
	case kAdditiveBlenderMode:
		return srcCols;
	case kArgbToArgbBlender:
	case kArgbToRgbBlender:
		ch1 = _mm_and_si128(_mm_set1_epi16(0xff), _mm_cmpeq_epi16(alphas, _mm_setzero_si128()));
		ch2 = _mm_and_si128(alphas, _mm_cmpgt_epi16(alphas, _mm_setzero_si128()));
		alphas = _mm_or_si128(ch1, ch2);
		// fall through
	case kRgbToRgbBlender:
	case kAlphaPreservedBlenderMode:
		return rgbBlendSIMD2Bpp(srcCols, destCols, alphas);
	case kRgbToArgbBlender:
		mask = _mm_or_si128(_mm_cmpeq_epi16(alphas, _mm_set1_epi16(0)), _mm_cmpeq_epi16(alphas, _mm_set1_epi16(255)));
		ch1 = _mm_and_si128(srcCols, mask);
		ch2 = _mm_andnot_si128(mask, rgbBlendSIMD2Bpp(srcCols, destCols, alphas));
		return _mm_or_si128(ch1, ch2);
	case kTintBlenderMode:
	case kTintLightBlenderMode:
		__m128i srcColsLo = simd2BppTo4Bpp(_mm_and_si128(srcCols, _mm_set_epi32(0, 0, -1, -1)));
		__m128i srcColsHi = simd2BppTo4Bpp(_mm_srli_si128(srcCols, 8));
		__m128i destColsLo = simd2BppTo4Bpp(_mm_and_si128(destCols, _mm_set_epi32(0, 0, -1, -1)));
		__m128i destColsHi = simd2BppTo4Bpp(_mm_srli_si128(destCols, 8));
		__m128i alphasLo = _mm_unpacklo_epi16(_mm_and_si128(alphas, _mm_set_epi32(0, 0, -1, -1)), _mm_setzero_si128());
		__m128i alphasHi = _mm_unpacklo_epi16(_mm_srli_si128(alphas, 8), _mm_setzero_si128());
		__m128i lo = simd4BppTo2Bpp(blendTintSpriteSIMD(srcColsLo, destColsLo, alphasLo, _G(_blender_mode) == kTintLightBlenderMode));
		__m128i hi = simd4BppTo2Bpp(blendTintSpriteSIMD(srcColsHi, destColsHi, alphasHi, _G(_blender_mode) == kTintLightBlenderMode));
		return _mm_or_si128(lo, _mm_slli_si128(hi, 8));
	}
	return _mm_setzero_si128();
}

template<int DestBytesPerPixel, int SrcBytesPerPixel>
static inline void drawPixelSIMD(byte *destPtr, const byte *srcP2, __m128i tint, __m128i alphas, __m128i maskedAlphas, __m128i transColors, int xDir, int xCtrBpp, int srcAlpha, int skipTrans, bool horizFlip, bool useTint, __m128i skipMask) {
	__m128i srcCols, destCol;

	if (DestBytesPerPixel == 4)
		destCol = _mm_loadu_si128((const __m128i *)destPtr);
	else
		destCol = simd2BppTo4Bpp(_mm_loadl_epi64((const __m128i *)destPtr));
	if (SrcBytesPerPixel == 4)
		srcCols = _mm_loadu_si128((const __m128i *)(srcP2 + xDir * xCtrBpp));
	else
		srcCols = simd2BppTo4Bpp(_mm_loadl_epi64((const __m128i *)(srcP2 + xDir * xCtrBpp)));

	// we do this here because we need to check if we should skip the pixel before we blend it
	__m128i mask1 = skipTrans ? _mm_cmpeq_epi32(_mm_and_si128(srcCols, maskedAlphas), transColors) : _mm_setzero_si128();
	mask1 = _mm_or_si128(mask1, skipMask);
	if (srcAlpha != -1) {
		// take into account for useTint
		if (useTint) {
			srcCols = blendPixelSIMD(tint, srcCols, alphas);
		} else {
			srcCols = blendPixelSIMD(srcCols, destCol, alphas);
		}
	}
	__m128i destCols2 = _mm_and_si128(destCol, mask1);
	__m128i srcCols2 = _mm_andnot_si128(mask1, srcCols);
	__m128i final = _mm_or_si128(destCols2, srcCols2);
	if (horizFlip) {
		final = _mm_shuffle_epi32(final, _MM_SHUFFLE(0, 1, 2, 3));
	}
	if (DestBytesPerPixel == 4) {
		_mm_storeu_si128((__m128i *)destPtr, final);
	} else {
		_mm_storel_epi64((__m128i *)destPtr, simd4BppTo2Bpp(final));
	}
}

static inline void drawPixelSIMD2Bpp(byte *destPtr, const byte *srcP2, __m128i tint, __m128i alphas, __m128i transColors, int xDir, int xCtrBpp, int srcAlpha, int skipTrans, bool horizFlip, bool useTint, __m128i skipMask) {
	__m128i destCol = _mm_loadu_si128((const __m128i *)destPtr);
	__m128i srcCols = _mm_loadu_si128((const __m128i *)(srcP2 + xDir * xCtrBpp));
	__m128i mask1 = skipTrans ? _mm_cmpeq_epi16(srcCols, transColors) : _mm_setzero_si128();
	mask1 = _mm_or_si128(mask1, skipMask);
	if (srcAlpha != -1) {
		// take into account for useTint
		if (useTint) {
			srcCols = blendPixelSIMD2Bpp(tint, srcCols, alphas);
		} else {
			srcCols = blendPixelSIMD2Bpp(srcCols, destCol, alphas);
		}
	}
	__m128i destCols2 = _mm_and_si128(destCol, mask1);
	__m128i srcCols2 = _mm_andnot_si128(mask1, srcCols);
	__m128i final = _mm_or_si128(destCols2, srcCols2);
	if (horizFlip) {
		final = _mm_shufflelo_epi16(final, _MM_SHUFFLE(0, 1, 2, 3));
		final = _mm_shufflehi_epi16(final, _MM_SHUFFLE(0, 1, 2, 3));
		final = _mm_castpd_si128(_mm_shuffle_pd(_mm_castsi128_pd(final), _mm_castsi128_pd(final), _MM_SHUFFLE2(0, 1)));
	}
	_mm_storeu_si128((__m128i *)destPtr, final);
}

static inline uint32 extract32_idx0(__m128i x) {
	return _mm_cvtsi128_si32(x);
}
static inline uint32 extract32_idx1(__m128i x) {
	return _mm_cvtsi128_si32(_mm_shuffle_epi32(x, _MM_SHUFFLE(1, 1, 1, 1)));
}
static inline uint32 extract32_idx2(__m128i x) {
	return _mm_cvtsi128_si32(_mm_shuffle_epi32(x, _MM_SHUFFLE(2, 2, 2, 2)));
}
static inline uint32 extract32_idx3(__m128i x) {
	return _mm_cvtsi128_si32(_mm_shuffle_epi32(x, _MM_SHUFFLE(3, 3, 3, 3)));
}

public:

// This template handles 2bpp and 4bpp, the other specializations handle 1bpp and format conversion blits
template<int DestBytesPerPixel, int SrcBytesPerPixel, bool Scale>
static void drawInner4BppWithConv(BITMAP::DrawInnerArgs &args) {
	const int xDir = args.horizFlip ? -1 : 1;
	byte rSrc, gSrc, bSrc, aSrc;
	byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;
    __m128i tint = _mm_slli_epi32(_mm_set1_epi32(args.srcAlpha), 24);
	tint = _mm_or_si128(tint, _mm_slli_epi32(_mm_set1_epi32(args.tintRed), 16));
	tint = _mm_or_si128(tint, _mm_slli_epi32(_mm_set1_epi32(args.tintGreen), 8));
	tint = _mm_or_si128(tint, _mm_set1_epi32(args.tintBlue));
	__m128i maskedAlphas = _mm_set1_epi32(args.alphaMask);
	__m128i transColors = _mm_set1_epi32(args.transColor);
	__m128i alphas = _mm_set1_epi32(args.srcAlpha);

	// This is so that we can calculate what pixels to crop off in a vectorized way
	__m128i addIndexes = _mm_set_epi32(3, 2, 1, 0);
	if (args.horizFlip) addIndexes = _mm_set_epi32(0, 1, 2, 3);

	// This is so that we can calculate in parralell the pixel indexes for scaled drawing
	__m128i scaleAdds = _mm_set_epi32((uint32)args.scaleX*3, (uint32)args.scaleX*2, (uint32)args.scaleX, 0);

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
		__m128i xCtrWidthSIMD = _mm_set1_epi32(xCtrWidth); // This is the width of the row

		if (!Scale) {
			// If we are not scaling the image
			int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart;
			for (; xCtr < secondToLast; destX += 4, xCtr += 4, xCtrBpp += SrcBytesPerPixel*4) {
				byte *destPtr = &destP[destX * DestBytesPerPixel];
				drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, srcP, tint, alphas, maskedAlphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, _mm_set1_epi32(0));
			}

			byte *destPtr = &destP[destX * DestBytesPerPixel];
			__m128i srcCols = _mm_setzero_si128();
			__m128i destCols = _mm_setzero_si128();
			memcpy(&srcCols, srcP + xDir * xCtrBpp, (xCtrWidth - xCtr) * SrcBytesPerPixel);
			memcpy(&destCols, destPtr, (xCtrWidth - xCtr) * DestBytesPerPixel);

			// Skip pixels that are beyond the row
			// __m128i skipMask = _mm_cmpgt_epi32(_mm_add_epi32(_mm_add_epi32(_mm_set1_epi32(xCtr), addIndexes), _mm_set1_epi32(1)), xCtrWidthSIMD);
			drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>((byte *)&destCols, (byte *)&srcCols, tint, alphas, maskedAlphas, transColors, xDir, 0, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, _mm_set1_epi32(0));
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
			byte srcBuffer[4*4] = {0};
			for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart, scaleXCtr = xCtrStart * args.scaleX; xCtr < xCtrWidth; destX += 4, xCtr += 4, xCtrBpp += SrcBytesPerPixel*4) {
				if (yCtr + 1 == yCtrHeight && xCtr + 4 > xCtrWidth) break; // Don't go past the last 4 pixels
				__m128i indexes = _mm_set1_epi32(scaleXCtr);
				// Calculate in parallel the indexes of the pixels
				if (SrcBytesPerPixel == 4)
					indexes = _mm_slli_epi32(_mm_srli_epi32(_mm_add_epi32(indexes, scaleAdds), BITMAP::SCALE_THRESHOLD_BITS), 2);
				else
					indexes = _mm_slli_epi32(_mm_srli_epi32(_mm_add_epi32(indexes, scaleAdds), BITMAP::SCALE_THRESHOLD_BITS), 1);
				// Simply memcpy them in. memcpy has no real performance overhead here
				memcpy(&srcBuffer[0*(size_t)SrcBytesPerPixel], srcP + extract32_idx0(indexes), SrcBytesPerPixel);
				memcpy(&srcBuffer[1*(size_t)SrcBytesPerPixel], srcP + extract32_idx1(indexes), SrcBytesPerPixel);
				memcpy(&srcBuffer[2*(size_t)SrcBytesPerPixel], srcP + extract32_idx2(indexes), SrcBytesPerPixel);
				memcpy(&srcBuffer[3*(size_t)SrcBytesPerPixel], srcP + extract32_idx3(indexes), SrcBytesPerPixel);
				scaleXCtr += args.scaleX*4;

				// Now this is pretty much the same as before with non-scaled code, except that we use
				// our dummy source buffer instead of the actuall source bitmap
				byte *destPtr = &destP[destX * (intptr_t)DestBytesPerPixel];
				__m128i skipMask = _mm_cmpgt_epi32(_mm_add_epi32(_mm_add_epi32(_mm_set1_epi32(xCtr), addIndexes), _mm_set1_epi32(1)), xCtrWidthSIMD);
				drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, (const byte *)srcBuffer, tint, alphas, maskedAlphas, transColors, 1, 0, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, skipMask);
			}
			// We calculate every row here except the last (because then we need to
			// check for if we fall off the edge of the row)
			// The only exception here is scaling drawing this is because:
			// 1) if statements are costly, and the less we do the faster this loop is
			// 2) with this, the only branch in the normal drawing loop is the width check
			// 3) the scaling code will actually draw the until the last 4 pixels of the image
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
			drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, srcP, tint, alphas, maskedAlphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, _mm_setzero_si128());
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
	__m128i tint = _mm_set1_epi16(args.src.format.ARGBToColor(args.srcAlpha, args.tintRed, args.tintGreen, args.tintBlue));
	__m128i transColors = _mm_set1_epi16(args.transColor);
	__m128i alphas = _mm_set1_epi16(args.srcAlpha);

	// This is so that we can calculate what pixels to crop off in a vectorized way
	__m128i addIndexes = _mm_set_epi16(7, 6, 5, 4, 3, 2, 1, 0);

	// This is so that we can calculate in parralell the pixel indexes for scaled drawing
	if (args.horizFlip) addIndexes = _mm_set_epi16(0, 1, 2, 3, 4, 5, 6, 7);
	__m128i scaleAdds = _mm_set_epi32((uint32)args.scaleX*3, (uint32)args.scaleX*2, (uint32)args.scaleX, 0);
	__m128i scaleAdds2 = _mm_set_epi32((uint32)args.scaleX*7, (uint32)args.scaleX*6, (uint32)args.scaleX*5, (uint32)args.scaleX*4);

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
		__m128i xCtrWidthSIMD = _mm_set1_epi16(xCtrWidth); // This is the width of the row
		if (!Scale) {
			// If we are not scaling the image
			int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart;
			for (; xCtr < secondToLast; destX += 8, xCtr += 8, xCtrBpp += 16) {
				byte *destPtr = &destP[destX * 2];
				drawPixelSIMD2Bpp(destPtr, srcP, tint, alphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, _mm_set1_epi16(0));
			}

			byte *destPtr = &destP[destX * 2];
			__m128i srcCols = _mm_setzero_si128();
			__m128i destCols = _mm_setzero_si128();
			const int copySize = (xCtrWidth - xCtr) * 2;
			memcpy(&srcCols, srcP + xDir * xCtrBpp, copySize);
			memcpy(&destCols, destPtr, copySize);

			// Skip pixels that are beyond the row
			// __m128i skipMask = _mm_cmpgt_epi16(_mm_add_epi16(_mm_add_epi16(_mm_set1_epi16(xCtr), addIndexes), _mm_set1_epi16(1)), xCtrWidthSIMD);
			drawPixelSIMD2Bpp((byte *)&destCols, (byte *)&srcCols, tint, alphas, transColors, xDir, 0, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, _mm_set1_epi16(0));
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
				__m128i indexes = _mm_set1_epi32(scaleXCtr), indexes2 = _mm_set1_epi32(scaleXCtr);
				// Calculate in parallel the indexes of the pixels
				indexes = _mm_slli_epi32(_mm_srli_epi32(_mm_add_epi32(indexes, scaleAdds), BITMAP::SCALE_THRESHOLD_BITS), 1);
				indexes2 = _mm_slli_epi32(_mm_srli_epi32(_mm_add_epi32(indexes2, scaleAdds2), BITMAP::SCALE_THRESHOLD_BITS), 1);
				// Simply memcpy them in. memcpy has no real performance overhead here
				srcBuffer[0] = *(const uint16 *)(srcP + extract32_idx0(indexes));
				srcBuffer[1] = *(const uint16 *)(srcP + extract32_idx1(indexes));
				srcBuffer[2] = *(const uint16 *)(srcP + extract32_idx2(indexes));
				srcBuffer[3] = *(const uint16 *)(srcP + extract32_idx3(indexes));
				srcBuffer[4] = *(const uint16 *)(srcP + extract32_idx0(indexes2));
				srcBuffer[5] = *(const uint16 *)(srcP + extract32_idx1(indexes2));
				srcBuffer[6] = *(const uint16 *)(srcP + extract32_idx2(indexes2));
				srcBuffer[7] = *(const uint16 *)(srcP + extract32_idx3(indexes2));
				scaleXCtr += args.scaleX*8;

				// Now this is pretty much the same as before with non-scaled code, except that we use
				// our dummy source buffer instead of the actuall source bitmap
				byte *destPtr = &destP[destX * 2];
				__m128i skipMask = _mm_cmpgt_epi16(_mm_add_epi16(_mm_add_epi16(_mm_set1_epi16(xCtr), addIndexes), _mm_set1_epi16(1)), xCtrWidthSIMD);
				drawPixelSIMD2Bpp(destPtr, (const byte *)srcBuffer, tint, alphas, transColors, 1, 0, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, skipMask);
			}
			// We calculate every row here except the last (because then we need to
			// check for if we fall off the edge of the row)
			// The only exception here is scaling drawing this is because:
			// 1) if statements are costly, and the less we do the faster this loop is
			// 2) with this, the only branch in the normal drawing loop is the width check
			// 3) the scaling code will actually draw the until the last 4 pixels of the image
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
			drawPixelSIMD2Bpp(destPtr, srcP, tint, alphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, _mm_setzero_si128());
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
	__m128i transColors = _mm_set1_epi16(args.transColor | (args.transColor << 8));

	// This is so that we can calculate in parralell the pixel indexes for scaled drawing
	__m128i scaleAdds1 = _mm_set_epi32((uint32)args.scaleX*3, (uint32)args.scaleX*2, (uint32)args.scaleX, 0);
	__m128i scaleAdds2 = _mm_set_epi32((uint32)args.scaleX*7, (uint32)args.scaleX*6, (uint32)args.scaleX*5, (uint32)args.scaleX*4);
	__m128i scaleAdds3 = _mm_set_epi32((uint32)args.scaleX*11, (uint32)args.scaleX*10, (uint32)args.scaleX*9, (uint32)args.scaleX*8);
	__m128i scaleAdds4 = _mm_set_epi32((uint32)args.scaleX*15, (uint32)args.scaleX*14, (uint32)args.scaleX*13, (uint32)args.scaleX*12);

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

			// Here we dont use the drawPixelSIMD function because 1bpp bitmaps in allegro
			// can't have any blending applied to them
			__m128i destCols = _mm_loadu_si128((const __m128i *)destPtr);
			__m128i srcCols = _mm_loadu_si128((const __m128i *)(srcP + xDir * xCtr));
			if (Scale) {
				// If we are scaling, we have to set each pixel individually
				__m128i indexes1 = _mm_set1_epi32(scaleXCtr), indexes2 = _mm_set1_epi32(scaleXCtr);
				__m128i indexes3 = _mm_set1_epi32(scaleXCtr), indexes4 = _mm_set1_epi32(scaleXCtr);
				// Calculate in parallel the indexes of the pixels
				indexes1 = _mm_srli_epi32(_mm_add_epi32(indexes1, scaleAdds1), BITMAP::SCALE_THRESHOLD_BITS);
				indexes2 = _mm_srli_epi32(_mm_add_epi32(indexes2, scaleAdds2), BITMAP::SCALE_THRESHOLD_BITS);
				indexes3 = _mm_srli_epi32(_mm_add_epi32(indexes3, scaleAdds3), BITMAP::SCALE_THRESHOLD_BITS);
				indexes4 = _mm_srli_epi32(_mm_add_epi32(indexes4, scaleAdds4), BITMAP::SCALE_THRESHOLD_BITS);
				srcCols = _mm_set_epi8(
					srcP[extract32_idx3(indexes4)],
					srcP[extract32_idx2(indexes4)],
					srcP[extract32_idx1(indexes4)],
					srcP[extract32_idx0(indexes4)],
					srcP[extract32_idx3(indexes3)],
					srcP[extract32_idx2(indexes3)],
					srcP[extract32_idx1(indexes3)],
					srcP[extract32_idx0(indexes3)],
					srcP[extract32_idx3(indexes2)],
					srcP[extract32_idx2(indexes2)],
					srcP[extract32_idx1(indexes2)],
					srcP[extract32_idx0(indexes2)],
					srcP[extract32_idx3(indexes1)],
					srcP[extract32_idx2(indexes1)],
					srcP[extract32_idx1(indexes1)],
					srcP[extract32_idx0(indexes1)]);
				scaleXCtr += args.scaleX*16;
			}

			// Mask out transparent pixels
			__m128i mask1 = args.skipTrans ? _mm_cmpeq_epi8(srcCols, transColors) : _mm_setzero_si128();
			__m128i final = _mm_or_si128(_mm_andnot_si128(mask1, srcCols), _mm_and_si128(destCols, mask1));
			if (args.horizFlip) {
				__m128i final_swap16 = _mm_srli_epi16(final, 8);
				final_swap16 = _mm_or_si128(final_swap16, _mm_slli_epi16(_mm_and_si128(final, _mm_set1_epi16(0xff)), 8));
				final_swap16 = _mm_shufflelo_epi16(final_swap16, _MM_SHUFFLE(0, 1, 2, 3));
				final_swap16 = _mm_shufflehi_epi16(final_swap16, _MM_SHUFFLE(0, 1, 2, 3));
				final = _mm_castpd_si128(_mm_shuffle_pd(_mm_castsi128_pd(final_swap16), _mm_castsi128_pd(final_swap16), _MM_SHUFFLE2(0, 1)));
			}
			_mm_storeu_si128((__m128i *)destPtr, final);
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
		destP += args.destArea.pitch; // Goto next row
		// Only advance the src row by 1 every time like this if we don't scale
		if (!Scale) srcP += args.vertFlip ? -args.src.pitch : args.src.pitch;
	}
}

}; // end of class DrawInnerImpl_SSE2

template<bool Scale>
void BITMAP::drawSSE2(DrawInnerArgs &args) {
	if (args.sameFormat) {
		switch (format.bytesPerPixel) {
		case 1: DrawInnerImpl_SSE2::drawInner1Bpp<Scale>(args); break;
		case 2: DrawInnerImpl_SSE2::drawInner2Bpp<Scale>(args); break;
		case 4: DrawInnerImpl_SSE2::drawInner4BppWithConv<4, 4, Scale>(args); break;
		}
	} else if (format.bytesPerPixel == 4 && args.src.format.bytesPerPixel == 2) {
		DrawInnerImpl_SSE2::drawInner4BppWithConv<4, 2, Scale>(args);
	} else if (format.bytesPerPixel == 2 && args.src.format.bytesPerPixel == 4) {
		DrawInnerImpl_SSE2::drawInner4BppWithConv<2, 4, Scale>(args);
	}
}

template void BITMAP::drawSSE2<false>(DrawInnerArgs &);
template void BITMAP::drawSSE2<true>(DrawInnerArgs &);

} // namespace AGS3

#if !defined(__x86_64__)

#if defined(__clang__)
#pragma clang attribute pop
#elif defined(__GNUC__)
#pragma GCC pop_options
#endif

#endif // !defined(__x86_64__)
