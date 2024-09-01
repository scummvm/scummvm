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

#include "ags/lib/allegro/gfx.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/flood.h"
#include "ags/ags.h"
#include "ags/globals.h"
#include "common/textconsole.h"
#include "graphics/screen.h"

#include <immintrin.h>

#if defined(__clang__)
#pragma clang attribute push (__attribute__((target("avx2"))), apply_to=function)
#elif defined(__GNUC__)
#pragma GCC push_options
#pragma GCC target("avx2")
#endif

namespace AGS3 {

class DrawInnerImpl_AVX2 {

static inline __m256i simd2BppTo4Bpp(__m256i pixels) {
	__m128i x128 = _mm256_castsi256_si128(pixels);
	__m256i x = _mm256_cvtepu16_epi32(x128);

	// c is the extracted 5/6 bit color from the image
	__m256i c = _mm256_srli_epi32(x, 11);

	// We convert it back to normal by shifting it thrice over, naturally, and then using the 2 most
	// sinificant bits in the original color for the least significant bits in the new one
	__m256i r = _mm256_slli_epi32(_mm256_or_si256(_mm256_slli_epi32(c, 3), _mm256_srli_epi32(c, 2)), 16);
	c = _mm256_srli_epi32(_mm256_and_si256(x, _mm256_set1_epi32(0x07e0)), 5);
	__m256i g = _mm256_slli_epi32(_mm256_or_si256(_mm256_slli_epi32(c, 2), _mm256_srli_epi32(c, 4)), 8);
	c = _mm256_and_si256(x, _mm256_set1_epi32(0x001f));
	__m256i b = _mm256_or_si256(_mm256_slli_epi32(c, 3), _mm256_srli_epi32(c, 2));

	// By default 2bpp to 4bpp makes the alpha channel 255
	return _mm256_or_si256(_mm256_or_si256(_mm256_or_si256(r, g), b), _mm256_set1_epi32(0xff000000));
}

static inline __m256i simd4BppTo2Bpp(__m256i pixels) {
	// x is the final 16 bit rgb pixel
	__m256i x = _mm256_srli_epi32(_mm256_and_si256(pixels, _mm256_set1_epi32(0x000000ff)), 3);
	x = _mm256_or_si256(x, _mm256_slli_epi32(_mm256_srli_epi32(_mm256_and_si256(pixels, _mm256_set1_epi32(0x0000ff00)), 8+2), 5));
	x = _mm256_or_si256(x, _mm256_slli_epi32(_mm256_srli_epi32(_mm256_and_si256(pixels, _mm256_set1_epi32(0x00ff0000)), 16+3), 11));
	x = _mm256_slli_epi32(x, 16);
	x = _mm256_srai_epi32(x, 16);
	x = _mm256_packs_epi32(x, _mm256_setzero_si256());
	return _mm256_permute4x64_epi64(x, _MM_SHUFFLE(3, 1, 2, 0));
}

static inline __m256i rgbBlendSIMD2Bpp(__m256i srcCols, __m256i destCols, __m256i alphas) {
	// Here we add 1 to alphas if its 0. This is what the original blender function did
	alphas = _mm256_add_epi16(alphas, _mm256_and_si256(_mm256_cmpgt_epi16(alphas, _mm256_setzero_si256()), _mm256_set1_epi16(1)));

	// Split the components into rgb
	__m256i srcComps[] = {
		_mm256_and_si256(srcCols, _mm256_set1_epi16(0x1f)),		    		 // B
		_mm256_and_si256(_mm256_srli_epi16(srcCols, 5), _mm256_set1_epi16(0x3f)), // G
		_mm256_srli_epi16(srcCols, 11),									 // R
	}, destComps[] = {
		_mm256_and_si256(destCols, _mm256_set1_epi16(0x1f)),		    		  // B
		_mm256_and_si256(_mm256_srli_epi16(destCols, 5), _mm256_set1_epi16(0x3f)), // G
		_mm256_srli_epi16(destCols, 11),									  // R
	};

	// Calculate the differences between the colors
	__m256i diffs[] = {
		_mm256_sub_epi16(srcComps[0], destComps[0]), // B
		_mm256_sub_epi16(srcComps[1], destComps[1]), // G
		_mm256_sub_epi16(srcComps[2], destComps[2]), // R
	};

	// Multiply by alpha and shift depth bits to the right
	// pretty much the same as (int)(((float)component / 255.0f) * ((float)alpha / 255.0f) * 255.0f)
	alphas = _mm256_srli_epi16(alphas, 2);
	diffs[1] = _mm256_srli_epi16(_mm256_mullo_epi16(diffs[1], alphas), 6);
	alphas = _mm256_srli_epi16(alphas, 1);
	diffs[0] = _mm256_srli_epi16(_mm256_mullo_epi16(diffs[0], alphas), 5);
	diffs[2] = _mm256_srli_epi16(_mm256_mullo_epi16(diffs[2], alphas), 5);

	// Here we add the difference between the 2 colors times alpha onto the destination
	diffs[0] = _mm256_and_si256(_mm256_add_epi16(diffs[0], destComps[0]), _mm256_set1_epi16(0x1f));
	diffs[1] = _mm256_and_si256(_mm256_add_epi16(diffs[1], destComps[1]), _mm256_set1_epi16(0x3f));
	diffs[2] = _mm256_and_si256(_mm256_add_epi16(diffs[2], destComps[2]), _mm256_set1_epi16(0x1f));

	// We compile all the colors into diffs[0] as a 16 bit rgb pixel
	diffs[0] = _mm256_or_si256(diffs[0], _mm256_slli_epi16(diffs[1], 5));
	return _mm256_or_si256(diffs[0], _mm256_slli_epi16(diffs[2], 11));
}

// preserveAlpha:
//		false => set destCols's alpha to 0
// 		true => keep destCols's alpha
static inline __m256i rgbBlendSIMD(__m256i srcCols, __m256i destCols, __m256i alphas, bool preserveAlpha) {
	// Here we add 1 to alphas if its 0. This is what the original blender function did.
	alphas = _mm256_add_epi32(alphas, _mm256_and_si256(_mm256_cmpgt_epi32(alphas, _mm256_setzero_si256()), _mm256_set1_epi32(1)));

	// Get the alpha from the destination
	__m256i alpha = _mm256_and_si256(destCols, _mm256_set1_epi32(0xff000000));

	// Get red and blue components
	__m256i srcColsCopy = srcCols;
	srcColsCopy = _mm256_and_si256(srcColsCopy, _mm256_set1_epi32(0xff00ff));
	__m256i destColsCopy = destCols;
	destColsCopy = _mm256_and_si256(destColsCopy, _mm256_set1_epi32(0xff00ff));

	// Compute the difference, then multiply by alpha and divide by 256
	srcColsCopy = _mm256_sub_epi32(srcColsCopy, destColsCopy);
	srcColsCopy = _mm256_mullo_epi32(srcColsCopy, alphas);
	//srcColsCopy = _mm256_mul_epi32(srcColsCopy, alphas);
	srcColsCopy = _mm256_srli_epi32(srcColsCopy, 8);
	srcColsCopy = _mm256_add_epi32(srcColsCopy, destCols); // Add the new red/blue to the old red/blue

	// Do the same for the green component
	srcCols = _mm256_and_si256(srcCols, _mm256_set1_epi32(0xff00));
	destCols = _mm256_and_si256(destCols, _mm256_set1_epi32(0xff00));
	srcCols = _mm256_sub_epi32(srcCols, destCols);
	srcCols = _mm256_mullo_epi32(srcCols, alphas);
	//srcCols = _mm256_mul_epi32(srcCols, alphas);
	srcCols = _mm256_srli_epi32(srcCols, 8);
	srcCols = _mm256_add_epi32(srcCols, destCols); // Add the new green to the old green

	// Keep values in 8bit range and glue red/blue and green together
	srcColsCopy = _mm256_and_si256(srcColsCopy, _mm256_set1_epi32(0xff00ff));
	srcCols = _mm256_and_si256(srcCols, _mm256_set1_epi32(0xff00));
	srcCols = _mm256_or_si256(srcCols, srcColsCopy);

	// Remember that alpha is not alphas, but rather the alpha of destcols
	if (preserveAlpha) {
		srcCols = _mm256_and_si256(srcCols, _mm256_set1_epi32(0x00ffffff));
		srcCols = _mm256_or_si256(srcCols, alpha);
	}
	return srcCols;
}

static inline __m256i argbBlendSIMD(__m256i srcCols, __m256i destCols) {
	__m256 srcA = _mm256_cvtepi32_ps(_mm256_srli_epi32(srcCols, 24));
	srcA = _mm256_mul_ps(srcA, _mm256_set1_ps(1.0f / 255.0f));
	__m256 srcR = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(srcCols, 16), _mm256_set1_epi32(0xff)));
	__m256 srcG = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(srcCols, 8), _mm256_set1_epi32(0xff)));
	__m256 srcB = _mm256_cvtepi32_ps(_mm256_and_si256(srcCols, _mm256_set1_epi32(0xff)));

	__m256 destA = _mm256_cvtepi32_ps(_mm256_srli_epi32(destCols, 24));
	destA = _mm256_mul_ps(destA, _mm256_set1_ps(1.0f / 255.0f));
	__m256 destR = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(destCols, 16), _mm256_set1_epi32(0xff)));
	__m256 destG = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(destCols, 8), _mm256_set1_epi32(0xff)));
	__m256 destB = _mm256_cvtepi32_ps(_mm256_and_si256(destCols, _mm256_set1_epi32(0xff)));

	// the destination alpha gets multiplied by 255 - source alpha
	destA = _mm256_mul_ps(destA, _mm256_sub_ps(_mm256_set1_ps(1.0f), srcA));

	// ((src * sAlpha) + (dest * dAlpha)) / (sAlpha + dAlpha)
	__m256 combA = _mm256_add_ps(srcA, destA);
	__m256 combArcp = _mm256_rcp_ps(combA);
	destR = _mm256_mul_ps(_mm256_add_ps(_mm256_mul_ps(srcR, srcA), _mm256_mul_ps(destR, destA)), combArcp);
	destG = _mm256_mul_ps(_mm256_add_ps(_mm256_mul_ps(srcG, srcA), _mm256_mul_ps(destG, destA)), combArcp);
	destB = _mm256_mul_ps(_mm256_add_ps(_mm256_mul_ps(srcB, srcA), _mm256_mul_ps(destB, destA)), combArcp);
	combA = _mm256_mul_ps(combA, _mm256_set1_ps(255.0));

	// Now put it back together
	return _mm256_or_si256(_mm256_slli_epi32(_mm256_cvtps_epi32(combA), 24),
		_mm256_or_si256(_mm256_slli_epi32(_mm256_cvtps_epi32(destR), 16),
		_mm256_or_si256(_mm256_slli_epi32(_mm256_cvtps_epi32(destG), 8),
			_mm256_cvtps_epi32(destB))));
}

static inline __m256i blendTintSpriteSIMD(__m256i srcCols, __m256i destCols, __m256i alphas, bool light) {
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
	__m256 ddr, ddg, ddb;
	ddr = _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(destCols, 16), _mm256_set1_epi32(0xff))), _mm256_set1_ps(1.0f / 255.0f));
	ddg = _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(destCols, 8), _mm256_set1_epi32(0xff))), _mm256_set1_ps(1.0f / 255.0f));
	ddb = _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_and_si256(destCols, _mm256_set1_epi32(0xff))), _mm256_set1_ps(1.0f / 255.0f));
	__m256 ssr, ssg, ssb;
	ssr = _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(srcCols, 16), _mm256_set1_epi32(0xff))), _mm256_set1_ps(1.0f / 255.0f));
	ssg = _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(srcCols, 8), _mm256_set1_epi32(0xff))), _mm256_set1_ps(1.0f / 255.0f));
	ssb = _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_and_si256(srcCols, _mm256_set1_epi32(0xff))), _mm256_set1_ps(1.0f / 255.0f));

	// Get the maxes and mins (needed for HSV->RGB and visa-versa)
	__m256 dmaxes = _mm256_max_ps(ddr, _mm256_max_ps(ddg, ddb));
	__m256 smaxes = _mm256_max_ps(ssr, _mm256_max_ps(ssg, ssb));
	__m256 smins = _mm256_min_ps(ssr, _mm256_min_ps(ssg, ssb));

	// This is here to stop from dividing by 0
	const __m256 eplison0 = _mm256_set1_ps(0.0000001f);

	__m256 chroma = _mm256_max_ps(_mm256_sub_ps(smaxes, smins), eplison0);

	// RGB to HSV is a piecewise function, so we compute each part of the function first...
	__m256 hr, hg, hb, hue;
	hr = _mm256_div_ps(_mm256_sub_ps(ssg, ssb), chroma);
	hr = _mm256_sub_ps(hr, _mm256_mul_ps(_mm256_cvtepi32_ps(_mm256_cvtps_epi32(_mm256_mul_ps(hr, _mm256_set1_ps(1.0f / 6.0f)))), _mm256_set1_ps(6.0f)));
	hr = _mm256_add_ps(hr, _mm256_and_ps(_mm256_cmp_ps(hr, _mm256_setzero_ps(), _CMP_LT_OS), _mm256_set1_ps(6.0f)));
	hg = _mm256_add_ps(_mm256_div_ps(_mm256_sub_ps(ssb, ssr), chroma), _mm256_set1_ps(2.0f));
	hg = _mm256_max_ps(hg, _mm256_setzero_ps());
	hb = _mm256_add_ps(_mm256_div_ps(_mm256_sub_ps(ssr, ssg), chroma), _mm256_set1_ps(4.0f));
	hb = _mm256_max_ps(hb, _mm256_setzero_ps());

	// And then compute which one will be used based on criteria
	__m256 hrfactors = _mm256_and_ps(_mm256_and_ps(_mm256_cmp_ps(ssr, smaxes, _CMP_EQ_OS), _mm256_cmp_ps(ssr, ssb, _CMP_NEQ_OS)), _mm256_set1_ps(1.0f));
	__m256 hgfactors = _mm256_and_ps(_mm256_and_ps(_mm256_cmp_ps(ssg, smaxes, _CMP_EQ_OS), _mm256_cmp_ps(ssg, ssr, _CMP_NEQ_OS)), _mm256_set1_ps(1.0f));
	__m256 hbfactors = _mm256_and_ps(_mm256_and_ps(_mm256_cmp_ps(ssb, smaxes, _CMP_EQ_OS), _mm256_cmp_ps(ssb, ssg, _CMP_NEQ_OS)), _mm256_set1_ps(1.0f));
	hue = _mm256_mul_ps(hr, hrfactors);
	hue = _mm256_add_ps(hue, _mm256_mul_ps(hg, hgfactors));
	hue = _mm256_add_ps(hue, _mm256_mul_ps(hb, hbfactors));

	// Mess with the light like the original function
	__m256 val = dmaxes;
	if (light) {
		val = _mm256_sub_ps(val, _mm256_sub_ps(_mm256_set1_ps(1.0f), _mm256_mul_ps(_mm256_cvtepi32_ps(alphas), _mm256_set1_ps(1.0f / 250.0f))));
		val = _mm256_max_ps(val, _mm256_setzero_ps());
	}

	// then it stiches the HSV back together
	// the hue and saturation come from the source (tint) color, and the value comes from
	// the destinaion (real source) color
	chroma = _mm256_mul_ps(val, _mm256_div_ps(_mm256_sub_ps(smaxes, smins), _mm256_add_ps(smaxes, eplison0)));
	__m256 hprime_mod2 = _mm256_mul_ps(hue, _mm256_set1_ps(1.0f / 2.0f));
	hprime_mod2 = _mm256_mul_ps(_mm256_sub_ps(hprime_mod2, _mm256_cvtepi32_ps(_mm256_cvtps_epi32(_mm256_sub_ps(hprime_mod2, _mm256_set1_ps(0.5))))), _mm256_set1_ps(2.0f));
	__m256 x = _mm256_mul_ps(chroma, _mm256_sub_ps(_mm256_set1_ps(1), _mm256_and_ps(_mm256_castsi256_ps(_mm256_set1_epi32(0x7fffffff)), _mm256_sub_ps(hprime_mod2, _mm256_set1_ps(1)))));
	//float32x4_t x = vmulq_f32(chroma, vsubq_f32(vmovq_n_f32(1.0f), vabsq_f32(vsubq_f32(hprime_mod2, vmovq_n_f32(1.0f)))));
	__m256i hprime_rounded = _mm256_cvtps_epi32(_mm256_sub_ps(hue, _mm256_set1_ps(0.5)));
	__m256i x_int = _mm256_cvtps_epi32(_mm256_mul_ps(x, _mm256_set1_ps(255.0f)));
	__m256i c_int = _mm256_cvtps_epi32(_mm256_mul_ps(chroma, _mm256_set1_ps(255.0f)));

	// Again HSV->RGB is also a piecewise function
	__m256i val0 = _mm256_or_si256(_mm256_slli_epi32(x_int, 8), _mm256_slli_epi32(c_int, 16));
	val0 = _mm256_and_si256(val0, _mm256_or_si256(_mm256_cmpeq_epi32(hprime_rounded, _mm256_set1_epi32(0)), _mm256_cmpeq_epi32(hprime_rounded, _mm256_set1_epi32(6))));
	__m256i val1 = _mm256_or_si256(_mm256_slli_epi32(c_int, 8), _mm256_slli_epi32(x_int, 16));
	val1 = _mm256_and_si256(val1, _mm256_cmpeq_epi32(hprime_rounded, _mm256_set1_epi32(1)));
	__m256i val2 = _mm256_or_si256(_mm256_slli_epi32(c_int, 8), x_int);
	val2 = _mm256_and_si256(val2, _mm256_cmpeq_epi32(hprime_rounded, _mm256_set1_epi32(2)));
	__m256i val3 = _mm256_or_si256(_mm256_slli_epi32(x_int, 8), c_int);
	val3 = _mm256_and_si256(val3, _mm256_cmpeq_epi32(hprime_rounded, _mm256_set1_epi32(3)));
	__m256i val4 = _mm256_or_si256(_mm256_slli_epi32(x_int, 16), c_int);
	val4 = _mm256_and_si256(val4, _mm256_cmpeq_epi32(hprime_rounded, _mm256_set1_epi32(4)));
	__m256i val5 = _mm256_or_si256(_mm256_slli_epi32(c_int, 16), x_int);
	val5 = _mm256_and_si256(val5, _mm256_cmpeq_epi32(hprime_rounded, _mm256_set1_epi32(5)));

	// or the values together
	__m256i final = _mm256_or_si256(val0, _mm256_or_si256(val1, _mm256_or_si256(val2, _mm256_or_si256(val3, _mm256_or_si256(val4, val5)))));

	// add the minimums back in
	__m256i val_add = _mm256_cvtps_epi32(_mm256_mul_ps(_mm256_sub_ps(val, chroma), _mm256_set1_ps(255.0f)));
	val_add = _mm256_or_si256(val_add, _mm256_or_si256(_mm256_slli_epi32(val_add, 8), _mm256_or_si256(_mm256_slli_epi32(val_add, 16), _mm256_and_si256(destCols, _mm256_set1_epi32(0xff000000)))));
	final = _mm256_add_epi32(final, val_add);
	return final;
}

static inline __m256i mul32_as16(__m256i a, __m256i b) {
	__m256i a16 = _mm256_packs_epi32(a, _mm256_setzero_si256());
	__m256i b16 = _mm256_packs_epi32(b, _mm256_setzero_si256());
	__m256i res = _mm256_mullo_epi16(a16, b16);
	return _mm256_unpacklo_epi16(res, _mm256_setzero_si256());
}

static inline __m256i findmin32_as16(__m256i a, __m256i b) {
	__m256i a16 = _mm256_packs_epi32(a, _mm256_setzero_si256());
	__m256i b16 = _mm256_packs_epi32(b, _mm256_setzero_si256());
	__m256i res = _mm256_min_epi16(a16, b16);
	return _mm256_unpacklo_epi16(res, _mm256_setzero_si256());
}

static inline __m256i blendPixelSIMD(__m256i srcCols, __m256i destCols, __m256i alphas) {
	__m256i srcAlphas, difAlphas, mask, ch1, ch2;
	auto setupArgbAlphas = [&]() {
		// This acts the same as this in the normal blender functions
		// if (alpha == 0)
		//     alpha = aSrc;
		// else
		//     alpha = aSrc * ((alpha & 0xff) + 1) / 256;
		// where alpha is the alpha byte of the srcCols
		srcAlphas = _mm256_srli_epi32(srcCols, 24);
		difAlphas = _mm256_add_epi32(_mm256_and_si256(alphas, _mm256_set1_epi32(0xff)), _mm256_set1_epi32(1));
		difAlphas = _mm256_srli_epi32(mul32_as16(srcAlphas, difAlphas), 8);
		difAlphas = _mm256_slli_epi32(difAlphas, 24);
		srcAlphas = _mm256_slli_epi32(srcAlphas, 24);
		mask = _mm256_cmpeq_epi32(alphas, _mm256_setzero_si256());
		srcAlphas = _mm256_and_si256(srcAlphas, mask);
		difAlphas = _mm256_andnot_si256(mask, difAlphas);
		srcCols = _mm256_and_si256(srcCols, _mm256_set1_epi32(0x00ffffff));
		srcCols = _mm256_or_si256(srcCols, _mm256_or_si256(srcAlphas, difAlphas));
	};
	switch (_G(_blender_mode)) {
	case kSourceAlphaBlender: // see BITMAP member function blendSourceAlpha
		alphas = _mm256_srli_epi32(srcCols, 24);
		return rgbBlendSIMD(srcCols, destCols, alphas, false);
	case kArgbToArgbBlender: // see BITMAP member function blendArgbToArgb
		setupArgbAlphas();
		// only blend if alpha isn't 0, otherwise use destCols
		mask = _mm256_cmpgt_epi32(_mm256_srli_epi32(srcCols, 24), _mm256_setzero_si256());
		ch1 = _mm256_and_si256(argbBlendSIMD(srcCols, destCols), mask);
		ch2 = _mm256_andnot_si256(mask, destCols);
		return _mm256_or_si256(ch1, ch2);
	case kArgbToRgbBlender: // see BITMAP member function blendArgbToRgb
		setupArgbAlphas();
		return rgbBlendSIMD(srcCols, destCols, _mm256_srli_epi32(srcCols, 24), false);
	case kRgbToArgbBlender: // see BITMAP member function blendRgbToArgb
		// if alpha is NOT 0 or 255
		ch2 = _mm256_and_si256(srcCols, _mm256_set1_epi32(0x00ffffff));
		ch2 = _mm256_or_si256(ch2, _mm256_slli_epi32(alphas, 24));
		ch2 = argbBlendSIMD(ch2, destCols);
		// if alpha is 0 or 255
		ch1 = _mm256_or_si256(srcCols, _mm256_set1_epi32(0xff000000));
		// mask and or them together
		mask = _mm256_or_si256(_mm256_cmpeq_epi32(alphas, _mm256_setzero_si256()), _mm256_cmpeq_epi32(alphas, _mm256_set1_epi32(0xff)));
		ch1 = _mm256_and_si256(ch1, mask);
		ch2 = _mm256_andnot_si256(mask, ch2);
		return _mm256_or_si256(ch1, ch2);
	case kRgbToRgbBlender: // see BITMAP member function blendRgbToRgb
		return rgbBlendSIMD(srcCols, destCols, alphas, false);
	case kAlphaPreservedBlenderMode: // see BITMAP member function blendPreserveAlpha
		return rgbBlendSIMD(srcCols, destCols, alphas, true);
	case kOpaqueBlenderMode: // see BITMAP member function blendOpaque
		return _mm256_or_si256(srcCols, _mm256_set1_epi32(0xff000000));
	case kAdditiveBlenderMode: // see BITMAP member function blendAdditiveAlpha
		srcAlphas = _mm256_add_epi32(_mm256_srli_epi32(srcCols, 24), _mm256_srli_epi32(destCols, 24));
		srcAlphas = findmin32_as16(srcAlphas, _mm256_set1_epi32(0xff));
		srcCols = _mm256_and_si256(srcCols, _mm256_set1_epi32(0x00ffffff));
		return _mm256_or_si256(srcCols, _mm256_slli_epi32(srcAlphas, 24));
	case kTintBlenderMode: // see BITMAP member function blendTintSprite
		return blendTintSpriteSIMD(srcCols, destCols, alphas, false);
	case kTintLightBlenderMode: // see BITMAP member function blendTintSprite
		return blendTintSpriteSIMD(srcCols, destCols, alphas, true);
	}
	return _mm256_setzero_si256();
}

static inline __m256i blendPixelSIMD2Bpp(__m256i srcCols, __m256i destCols, __m256i alphas) {
	__m256i mask, ch1, ch2;
	switch (_G(_blender_mode)) {
	case kSourceAlphaBlender:
	case kOpaqueBlenderMode:
	case kAdditiveBlenderMode:
		return srcCols;
	case kArgbToArgbBlender:
	case kArgbToRgbBlender:
		ch1 = _mm256_and_si256(_mm256_set1_epi16(0xff), _mm256_cmpeq_epi16(alphas, _mm256_setzero_si256()));
		ch2 = _mm256_and_si256(alphas, _mm256_cmpgt_epi16(alphas, _mm256_setzero_si256()));
		alphas = _mm256_or_si256(ch1, ch2);
		// fall through
	case kRgbToRgbBlender:
	case kAlphaPreservedBlenderMode:
		return rgbBlendSIMD2Bpp(srcCols, destCols, alphas);
	case kRgbToArgbBlender:
		mask = _mm256_or_si256(_mm256_cmpeq_epi16(alphas, _mm256_set1_epi16(0)), _mm256_cmpeq_epi16(alphas, _mm256_set1_epi16(255)));
		ch1 = _mm256_and_si256(srcCols, mask);
		ch2 = _mm256_andnot_si256(mask, rgbBlendSIMD2Bpp(srcCols, destCols, alphas));
		return _mm256_or_si256(ch1, ch2);
	case kTintBlenderMode:
	case kTintLightBlenderMode:
		__m256i srcColsLo = simd2BppTo4Bpp(srcCols);
		__m256i srcColsHi = simd2BppTo4Bpp(_mm256_permute2x128_si256(srcCols, srcCols, _MM_SHUFFLE(2, 0, 0, 1)));
		__m256i destColsLo = simd2BppTo4Bpp(destCols);
		__m256i destColsHi = simd2BppTo4Bpp(_mm256_permute2x128_si256(destCols, destCols, _MM_SHUFFLE(2, 0, 0, 1)));
		__m256i alphasLo = _mm256_unpacklo_epi16(alphas, _mm256_setzero_si256());
		__m256i alphasHi = _mm256_unpacklo_epi16(_mm256_permute2x128_si256(alphas, alphas, _MM_SHUFFLE(2, 3, 0, 1)), _mm256_setzero_si256());
		__m256i lo = simd4BppTo2Bpp(blendTintSpriteSIMD(srcColsLo, destColsLo, alphasLo, _G(_blender_mode) == kTintLightBlenderMode));
		__m256i hi = simd4BppTo2Bpp(blendTintSpriteSIMD(srcColsHi, destColsHi, alphasHi, _G(_blender_mode) == kTintLightBlenderMode));
		return _mm256_or_si256(lo, _mm256_permute2x128_si256(hi, hi, _MM_SHUFFLE(0, 0, 2, 0)));
	}
	return _mm256_setzero_si256();
}

template<int DestBytesPerPixel, int SrcBytesPerPixel>
static inline void drawPixelSIMD(byte *destPtr, const byte *srcP2, __m256i tint, __m256i alphas, __m256i maskedAlphas, __m256i transColors, int xDir, int xCtrBpp, int srcAlpha, int skipTrans, bool horizFlip, bool useTint, __m256i skipMask) {
	__m256i srcCols, destCol;

	if (DestBytesPerPixel == 4)
		destCol = _mm256_loadu_si256((const __m256i *)destPtr);
	else
		destCol = simd2BppTo4Bpp(_mm256_castsi128_si256(_mm_loadu_si128((const __m128i *)destPtr)));
	if (SrcBytesPerPixel == 4)
		srcCols = _mm256_loadu_si256((const __m256i *)(srcP2 + xDir * xCtrBpp));
	else
		srcCols = simd2BppTo4Bpp(_mm256_castsi128_si256(_mm_loadu_si128((const __m128i *)(srcP2 + xDir * xCtrBpp))));

	// we do this here because we need to check if we should skip the pixel before we blend it
	__m256i mask1 = skipTrans ? _mm256_cmpeq_epi32(_mm256_and_si256(srcCols, maskedAlphas), transColors) : _mm256_setzero_si256();
	mask1 = _mm256_or_si256(mask1, skipMask);
	if (srcAlpha != -1) {
		// take into account for useTint
		if (useTint) {
			srcCols = blendPixelSIMD(tint, srcCols, alphas);
		} else {
			srcCols = blendPixelSIMD(srcCols, destCol, alphas);
		}
	}
	__m256i destCols2 = _mm256_and_si256(destCol, mask1);
	__m256i srcCols2 = _mm256_andnot_si256(mask1, srcCols);
	__m256i final = _mm256_or_si256(destCols2, srcCols2);
	if (horizFlip) {
        final = _mm256_shuffle_epi32(final, _MM_SHUFFLE(0, 1, 2, 3));
        final = _mm256_permute2x128_si256(final, final, 0x01);
	}
	if (DestBytesPerPixel == 4) {
		_mm256_storeu_si256((__m256i *)destPtr, final);
	} else {
        _mm_storeu_si128((__m128i *)destPtr, _mm256_extracti128_si256(simd4BppTo2Bpp(final), 0));
	}
}

static inline void drawPixelSIMD2Bpp(byte *destPtr, const byte *srcP2, __m256i tint, __m256i alphas, __m256i transColors, int xDir, int xCtrBpp, int srcAlpha, int skipTrans, bool horizFlip, bool useTint, __m256i skipMask) {
	__m256i destCol = _mm256_loadu_si256((const __m256i *)destPtr);
	__m256i srcCols = _mm256_loadu_si256((const __m256i *)(srcP2 + xDir * xCtrBpp));
	__m256i mask1 = skipTrans ? _mm256_cmpeq_epi16(srcCols, transColors) : _mm256_setzero_si256();
	mask1 = _mm256_or_si256(mask1, skipMask);
	if (srcAlpha != -1) {
		// take into account for useTint
		if (useTint) {
			srcCols = blendPixelSIMD2Bpp(tint, srcCols, alphas);
		} else {
			srcCols = blendPixelSIMD2Bpp(srcCols, destCol, alphas);
		}
	}
	__m256i destCols2 = _mm256_and_si256(destCol, mask1);
	__m256i srcCols2 = _mm256_andnot_si256(mask1, srcCols);
	__m256i final = _mm256_or_si256(destCols2, srcCols2);
	if (horizFlip) {
		final = _mm256_shufflelo_epi16(final, _MM_SHUFFLE(0, 1, 2, 3));
		final = _mm256_shufflehi_epi16(final, _MM_SHUFFLE(0, 1, 2, 3));
		final = _mm256_castpd_si256(_mm256_shuffle_pd(_mm256_castsi256_pd(final), _mm256_castsi256_pd(final), _MM_SHUFFLE2(0, 1)));
	}
	_mm256_storeu_si256((__m256i *)destPtr, final);
}

public:

// This template handles 2bpp and 4bpp, the other specializations handle 1bpp and format conversion blits
template<int DestBytesPerPixel, int SrcBytesPerPixel, bool Scale>
static void drawInner4BppWithConv(BITMAP::DrawInnerArgs &args) {
	const int xDir = args.horizFlip ? -1 : 1;
	byte rSrc, gSrc, bSrc, aSrc;
	byte rDest = 0, gDest = 0, bDest = 0, aDest = 0;
    __m256i tint = _mm256_slli_epi32(_mm256_set1_epi32(args.srcAlpha), 24);
	tint = _mm256_or_si256(tint, _mm256_slli_epi32(_mm256_set1_epi32(args.tintRed), 16));
	tint = _mm256_or_si256(tint, _mm256_slli_epi32(_mm256_set1_epi32(args.tintGreen), 8));
	tint = _mm256_or_si256(tint, _mm256_set1_epi32(args.tintBlue));
	__m256i maskedAlphas = _mm256_set1_epi32(args.alphaMask);
	__m256i transColors = _mm256_set1_epi32(args.transColor);
    __m256i alphas = _mm256_set1_epi32(args.srcAlpha);

	// This is so that we can calculate what pixels to crop off in a vectorized way
	__m256i addIndexes = _mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0);
	if (args.horizFlip) addIndexes = _mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7);

	// This is so that we can calculate in parralell the pixel indexes for scaled drawing
	__m256i scaleAdds = _mm256_set_epi32((uint32)args.scaleX*7, (uint32)args.scaleX*6, (uint32)args.scaleX*5, (uint32)args.scaleX*4,
        (uint32)args.scaleX*3, (uint32)args.scaleX*2, (uint32)args.scaleX, 0);

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
	/*if (xCtrWidth % 8 != 0) {
		--yCtrHeight;
	}*/

	const int secondToLast = xCtrWidth - 8;

	byte *destP = (byte *)args.destArea.getBasePtr(0, destY);
	const byte *srcP = (const byte *)args.src.getBasePtr(
	                       args.horizFlip ? args.srcArea.right - 8 : args.srcArea.left,
	                       args.vertFlip ? args.srcArea.bottom - 1 - yCtr : args.srcArea.top + yCtr);
	for (; yCtr < yCtrHeight; ++destY, ++yCtr, scaleYCtr += args.scaleY) {
		__m256i xCtrWidthSIMD = _mm256_set1_epi32(xCtrWidth); // This is the width of the row

		if (!Scale) {
			// If we are not scaling the image
			int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart;
			for (; xCtr < secondToLast; destX += 8, xCtr += 8, xCtrBpp += SrcBytesPerPixel*8) {
				byte *destPtr = &destP[destX * DestBytesPerPixel];
				drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, srcP, tint, alphas, maskedAlphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, _mm256_set1_epi32(0));
			}

			byte *destPtr = &destP[destX * DestBytesPerPixel];
			__m256i srcCols = _mm256_setzero_si256();
			__m256i destCols = _mm256_setzero_si256();
			memcpy(&srcCols, srcP + xDir * xCtrBpp, (xCtrWidth - xCtr) * SrcBytesPerPixel);
			memcpy(&destCols, destPtr, (xCtrWidth - xCtr) * DestBytesPerPixel);

			// Skip pixels that are beyond the row
			// __m256i skipMask = _mm256_cmpgt_epi32(_mm256_add_epi32(_mm256_add_epi32(_mm256_set1_epi32(xCtr), addIndexes), _mm256_set1_epi32(1)), xCtrWidthSIMD);
			drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>((byte *)&destCols, (byte *)&srcCols, tint, alphas, maskedAlphas, transColors, xDir, 0, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, _mm256_set1_epi32(0));
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
			byte srcBuffer[4*8] = {0};
			for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart, scaleXCtr = xCtrStart * args.scaleX; xCtr < xCtrWidth; destX += 8, xCtr += 8, xCtrBpp += SrcBytesPerPixel*4) {
				if (yCtr + 1 == yCtrHeight && xCtr + 4 > xCtrWidth) break; // Don't go past the last 4 pixels
				__m256i indexes = _mm256_set1_epi32(scaleXCtr);
				// Calculate in parallel the indexes of the pixels
				if (SrcBytesPerPixel == 4)
					indexes = _mm256_slli_epi32(_mm256_srli_epi32(_mm256_add_epi32(indexes, scaleAdds), BITMAP::SCALE_THRESHOLD_BITS), 2);
				else
					indexes = _mm256_slli_epi32(_mm256_srli_epi32(_mm256_add_epi32(indexes, scaleAdds), BITMAP::SCALE_THRESHOLD_BITS), 1);
				// Simply memcpy them in. memcpy has no real performance overhead here
				memcpy(&srcBuffer[0*(size_t)SrcBytesPerPixel], srcP + _mm256_extract_epi32(indexes, 0), SrcBytesPerPixel);
				memcpy(&srcBuffer[1*(size_t)SrcBytesPerPixel], srcP + _mm256_extract_epi32(indexes, 1), SrcBytesPerPixel);
				memcpy(&srcBuffer[2*(size_t)SrcBytesPerPixel], srcP + _mm256_extract_epi32(indexes, 2), SrcBytesPerPixel);
				memcpy(&srcBuffer[3*(size_t)SrcBytesPerPixel], srcP + _mm256_extract_epi32(indexes, 3), SrcBytesPerPixel);
				memcpy(&srcBuffer[4*(size_t)SrcBytesPerPixel], srcP + _mm256_extract_epi32(indexes, 4), SrcBytesPerPixel);
				memcpy(&srcBuffer[5*(size_t)SrcBytesPerPixel], srcP + _mm256_extract_epi32(indexes, 5), SrcBytesPerPixel);
				memcpy(&srcBuffer[6*(size_t)SrcBytesPerPixel], srcP + _mm256_extract_epi32(indexes, 6), SrcBytesPerPixel);
				memcpy(&srcBuffer[7*(size_t)SrcBytesPerPixel], srcP + _mm256_extract_epi32(indexes, 7), SrcBytesPerPixel);
				scaleXCtr += args.scaleX*8;

				// Now this is pretty much the same as before with non-scaled code, except that we use
				// our dummy source buffer instead of the actuall source bitmap
				byte *destPtr = &destP[destX * (intptr_t)DestBytesPerPixel];
				__m256i skipMask = _mm256_cmpgt_epi32(_mm256_add_epi32(_mm256_add_epi32(_mm256_set1_epi32(xCtr), addIndexes), _mm256_set1_epi32(1)), xCtrWidthSIMD);
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
	// We have a picture that is a multiple of 8, so no extra pixels to draw
	/*if (xCtrWidth % 8 == 0)*/ return;
	// Drawing the last few not scaled pixels here.
	// Same as the loop above but now we check if we are going to overflow,
	// and thus we don't need to mask out pixels that go over the row.
	if (!Scale) {
		for (; xCtr + 8 < xCtrWidth; destX += 8, xCtr += 8, xCtrBpp += SrcBytesPerPixel*8) {
			byte *destPtr = &destP[(ptrdiff_t)destX * DestBytesPerPixel];
			drawPixelSIMD<DestBytesPerPixel, SrcBytesPerPixel>(destPtr, srcP, tint, alphas, maskedAlphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, _mm256_setzero_si256());
		}
		// Because we move in 8 pixel units, and horizFlip moves in 1, we have to move
		// 1 pixel past the last pixel we did not blit, meaning going forward 7 pixels.
		if (args.horizFlip) srcP += SrcBytesPerPixel * 7;
	} else {
		// So if we are scaling, set up the xCtr to what it was before (AKA the last 8 or so pixels of the image)
		xCtr = xCtrWidth - xCtrWidth % 8;
		xCtrBpp = xCtr * SrcBytesPerPixel;
		destX = args.xStart+xCtr;
	}

	// For the last 8 pixels, we just do them in serial, nothing special
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
	__m256i tint = _mm256_set1_epi16(args.src.format.ARGBToColor(args.srcAlpha, args.tintRed, args.tintGreen, args.tintBlue));
	__m256i transColors = _mm256_set1_epi16(args.transColor);
	__m256i alphas = _mm256_set1_epi16(args.srcAlpha);

	// This is so that we can calculate what pixels to crop off in a vectorized way
	__m256i addIndexes = _mm256_set_epi16(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

	// This is so that we can calculate in parralell the pixel indexes for scaled drawing
	if (args.horizFlip) addIndexes = _mm256_set_epi16(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
	__m256i scaleAdds = _mm256_set_epi32((uint32)args.scaleX*7, (uint32)args.scaleX*6, (uint32)args.scaleX*5, (uint32)args.scaleX*4, (uint32)args.scaleX*3, (uint32)args.scaleX*2, (uint32)args.scaleX, 0);
	__m256i scaleAdds2 = _mm256_set_epi32((uint32)args.scaleX*15, (uint32)args.scaleX*14, (uint32)args.scaleX*13, (uint32)args.scaleX*12, (uint32)args.scaleX*11, (uint32)args.scaleX*10, (uint32)args.scaleX*9, (uint32)args.scaleX*8);

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
	/*if (!Scale && xCtrWidth % 16 != 0) {
		--yCtrHeight;
	}*/

	const int secondToLast = xCtrWidth - 16;

	byte *destP = (byte *)args.destArea.getBasePtr(0, destY);
	const byte *srcP = (const byte *)args.src.getBasePtr(
	                       args.horizFlip ? args.srcArea.right - 16 : args.srcArea.left,
	                       args.vertFlip ? args.srcArea.bottom - 1 - yCtr : args.srcArea.top + yCtr);
	for (; yCtr < yCtrHeight; ++destY, ++yCtr, scaleYCtr += args.scaleY) {
		__m256i xCtrWidthSIMD = _mm256_set1_epi16(xCtrWidth); // This is the width of the row
		if (!Scale) {
			// If we are not scaling the image
			int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart;
			for (; xCtr < secondToLast; destX += 16, xCtr += 16, xCtrBpp += 32) {
				byte *destPtr = &destP[destX * 2];
				drawPixelSIMD2Bpp(destPtr, srcP, tint, alphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, _mm256_set1_epi32(0));
			}

			byte *destPtr = &destP[destX * 2];
			__m256i srcCols = _mm256_setzero_si256();
			__m256i destCols = _mm256_setzero_si256();
			const int copySize = (xCtrWidth - xCtr) * 2;
			memcpy(&srcCols, srcP + xDir * xCtrBpp, copySize);
			memcpy(&destCols, destPtr, copySize);

			// Skip pixels that are beyond the row
			// __m256i skipMask = _mm256_cmpgt_epi16(_mm256_add_epi16(_mm256_add_epi16(_mm256_set1_epi16(xCtr), addIndexes), _mm256_set1_epi16(1)), xCtrWidthSIMD);
			drawPixelSIMD2Bpp((byte *)&destCols, (byte *)&srcCols, tint, alphas, transColors, xDir, 0, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, _mm256_set1_epi32(0));
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
			uint16 srcBuffer[16];
			for (int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart, scaleXCtr = xCtrStart * args.scaleX; xCtr < xCtrWidth; destX += 16, xCtr += 16, xCtrBpp += 32) {
				if (yCtr + 1 == yCtrHeight && xCtr + 8 > xCtrWidth) break;
				__m256i indexes = _mm256_set1_epi32(scaleXCtr), indexes2 = _mm256_set1_epi32(scaleXCtr);
				// Calculate in parallel the indexes of the pixels
				indexes = _mm256_slli_epi32(_mm256_srli_epi32(_mm256_add_epi32(indexes, scaleAdds), BITMAP::SCALE_THRESHOLD_BITS), 1);
				indexes2 = _mm256_slli_epi32(_mm256_srli_epi32(_mm256_add_epi32(indexes2, scaleAdds2), BITMAP::SCALE_THRESHOLD_BITS), 1);
				// Simply memcpy them in. memcpy has no real performance overhead here
				srcBuffer[0] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes, 0));
				srcBuffer[1] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes, 1));
				srcBuffer[2] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes, 2));
				srcBuffer[3] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes, 3));
				srcBuffer[4] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes, 4));
				srcBuffer[5] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes, 5));
				srcBuffer[6] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes, 6));
				srcBuffer[7] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes, 7));
				srcBuffer[8] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes2, 0));
				srcBuffer[9] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes2, 1));
				srcBuffer[10] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes2, 2));
				srcBuffer[11] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes2, 3));
				srcBuffer[12] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes2, 4));
				srcBuffer[13] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes2, 5));
				srcBuffer[14] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes2, 6));
				srcBuffer[15] = *(const uint16 *)(srcP + _mm256_extract_epi32(indexes2, 7));
				scaleXCtr += args.scaleX*16;

				// Now this is pretty much the same as before with non-scaled code, except that we use
				// our dummy source buffer instead of the actuall source bitmap
				byte *destPtr = &destP[destX * 2];
				__m256i skipMask = _mm256_cmpgt_epi16(_mm256_add_epi16(_mm256_add_epi16(_mm256_set1_epi16(xCtr), addIndexes), _mm256_set1_epi16(1)), xCtrWidthSIMD);
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

	// We have a picture that is a multiple of 16, so no extra pixels to draw
	/*if (xCtrWidth % 16 == 0)*/ return;
	// Get the last x values of the last row
	int xCtr = xCtrStart, xCtrBpp = xCtrBppStart, destX = args.xStart;
	// Drawing the last few not scaled pixels here.
	// Same as the loop above but now we check if we are going to overflow,
	// and thus we don't need to mask out pixels that go over the row.
	if (!Scale) {
		for (; xCtr + 16 < xCtrWidth; destX += 16, xCtr += 16, xCtrBpp += 32) {
			byte *destPtr = &destP[destX * 2];
			drawPixelSIMD2Bpp(destPtr, srcP, tint, alphas, transColors, xDir, xCtrBpp, args.srcAlpha, args.skipTrans, args.horizFlip, args.useTint, _mm256_setzero_si256());
		}
		// Because we move in 16 pixel units, and horizFlip moves in 1, we have to move
		// 1 pixel past the last pixel we did not blit, meaning going forward 15 pixels.
		if (args.horizFlip) srcP += 2 * 15;
	} else {
		// So if we are scaling, set up the xCtr to what it was before (AKA the last 16 or so pixels of the image)
		xCtr = xCtrWidth - xCtrWidth % 16;
		xCtrBpp = xCtr * 2;
		destX = args.xStart+xCtr;
	}

	// For the last 16 pixels, we just do them in serial, nothing special
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
	__m256i transColors = _mm256_set1_epi16(args.transColor | (args.transColor << 8));

	// This is so that we can calculate in parralell the pixel indexes for scaled drawing
	__m256i scaleAdds1 = _mm256_set_epi32((uint32)args.scaleX*7, (uint32)args.scaleX*6, (uint32)args.scaleX*5, (uint32)args.scaleX*4, (uint32)args.scaleX*3, (uint32)args.scaleX*2, (uint32)args.scaleX, 0);
	__m256i scaleAdds2 = _mm256_set_epi32((uint32)args.scaleX*15, (uint32)args.scaleX*14, (uint32)args.scaleX*13, (uint32)args.scaleX*12, (uint32)args.scaleX*11, (uint32)args.scaleX*10, (uint32)args.scaleX*9, (uint32)args.scaleX*8);
	__m256i scaleAdds3 = _mm256_set_epi32((uint32)args.scaleX*23, (uint32)args.scaleX*22, (uint32)args.scaleX*21, (uint32)args.scaleX*20, (uint32)args.scaleX*19, (uint32)args.scaleX*18, (uint32)args.scaleX*17, (uint32)args.scaleX*16);
	__m256i scaleAdds4 = _mm256_set_epi32((uint32)args.scaleX*31, (uint32)args.scaleX*30, (uint32)args.scaleX*29, (uint32)args.scaleX*28, (uint32)args.scaleX*27, (uint32)args.scaleX*26, (uint32)args.scaleX*25, (uint32)args.scaleX*24);

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
	                       args.horizFlip ? args.srcArea.right - 32 : args.srcArea.left,
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
		for (; xCtr + 32 < xCtrWidth; destX += 32, xCtr += 32) {
			byte *destPtr = &destP[destX];

			// Here we dont use the drawPixelSIMD function because 1bpp bitmaps in allegro
			// can't have any blending applied to them
			__m256i destCols = _mm256_loadu_si256((const __m256i *)destPtr);
			__m256i srcCols = _mm256_loadu_si256((const __m256i *)(srcP + xDir * xCtr));
			if (Scale) {
				// If we are scaling, we have to set each pixel individually
				__m256i indexes1 = _mm256_set1_epi32(scaleXCtr), indexes2 = _mm256_set1_epi32(scaleXCtr);
				__m256i indexes3 = _mm256_set1_epi32(scaleXCtr), indexes4 = _mm256_set1_epi32(scaleXCtr);
				// Calculate in parallel the indexes of the pixels
				indexes1 = _mm256_srli_epi32(_mm256_add_epi32(indexes1, scaleAdds1), BITMAP::SCALE_THRESHOLD_BITS);
				indexes2 = _mm256_srli_epi32(_mm256_add_epi32(indexes2, scaleAdds2), BITMAP::SCALE_THRESHOLD_BITS);
				indexes3 = _mm256_srli_epi32(_mm256_add_epi32(indexes3, scaleAdds3), BITMAP::SCALE_THRESHOLD_BITS);
				indexes4 = _mm256_srli_epi32(_mm256_add_epi32(indexes4, scaleAdds4), BITMAP::SCALE_THRESHOLD_BITS);
				srcCols = _mm256_set_epi8(
					srcP[_mm256_extract_epi32(indexes4, 7)],
					srcP[_mm256_extract_epi32(indexes4, 6)],
					srcP[_mm256_extract_epi32(indexes4, 5)],
					srcP[_mm256_extract_epi32(indexes4, 4)],
					srcP[_mm256_extract_epi32(indexes4, 3)],
					srcP[_mm256_extract_epi32(indexes4, 2)],
					srcP[_mm256_extract_epi32(indexes4, 1)],
					srcP[_mm256_extract_epi32(indexes4, 0)],
					srcP[_mm256_extract_epi32(indexes3, 7)],
					srcP[_mm256_extract_epi32(indexes3, 6)],
					srcP[_mm256_extract_epi32(indexes3, 5)],
					srcP[_mm256_extract_epi32(indexes3, 4)],
					srcP[_mm256_extract_epi32(indexes3, 3)],
					srcP[_mm256_extract_epi32(indexes3, 2)],
					srcP[_mm256_extract_epi32(indexes3, 1)],
					srcP[_mm256_extract_epi32(indexes3, 0)],
					srcP[_mm256_extract_epi32(indexes2, 7)],
					srcP[_mm256_extract_epi32(indexes2, 6)],
					srcP[_mm256_extract_epi32(indexes2, 5)],
					srcP[_mm256_extract_epi32(indexes2, 4)],
					srcP[_mm256_extract_epi32(indexes2, 3)],
					srcP[_mm256_extract_epi32(indexes2, 2)],
					srcP[_mm256_extract_epi32(indexes2, 1)],
					srcP[_mm256_extract_epi32(indexes2, 0)],
					srcP[_mm256_extract_epi32(indexes1, 7)],
					srcP[_mm256_extract_epi32(indexes1, 6)],
					srcP[_mm256_extract_epi32(indexes1, 5)],
					srcP[_mm256_extract_epi32(indexes1, 4)],
					srcP[_mm256_extract_epi32(indexes1, 3)],
					srcP[_mm256_extract_epi32(indexes1, 2)],
					srcP[_mm256_extract_epi32(indexes1, 1)],
					srcP[_mm256_extract_epi32(indexes1, 0)]);
				scaleXCtr += args.scaleX*16;
			}

			// Mask out transparent pixels
			__m256i mask1 = args.skipTrans ? _mm256_cmpeq_epi8(srcCols, transColors) : _mm256_setzero_si256();
			__m256i final = _mm256_or_si256(_mm256_andnot_si256(mask1, srcCols), _mm256_and_si256(destCols, mask1));
			if (args.horizFlip) {
				__m256i final_swap16 = _mm256_srli_epi16(final, 8);
				final_swap16 = _mm256_or_si256(final_swap16, _mm256_slli_epi16(_mm256_and_si256(final, _mm256_set1_epi16(0xff)), 8));
				final_swap16 = _mm256_shufflelo_epi16(final_swap16, _MM_SHUFFLE(0, 1, 2, 3));
				final_swap16 = _mm256_shufflehi_epi16(final_swap16, _MM_SHUFFLE(0, 1, 2, 3));
				final = _mm256_castpd_si256(_mm256_shuffle_pd(_mm256_castsi256_pd(final_swap16), _mm256_castsi256_pd(final_swap16), _MM_SHUFFLE2(0, 1)));
			}
			_mm256_storeu_si256((__m256i *)destPtr, final);
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

}; // end of class DrawInnerImpl_AVX2

template<bool Scale>
void BITMAP::drawAVX2(DrawInnerArgs &args) {
	if (args.sameFormat) {
		switch (format.bytesPerPixel) {
		case 1: DrawInnerImpl_AVX2::drawInner1Bpp<Scale>(args); break;
		case 2: DrawInnerImpl_AVX2::drawInner2Bpp<Scale>(args); break;
		case 4: DrawInnerImpl_AVX2::drawInner4BppWithConv<4, 4, Scale>(args); break;
		}
	} else if (format.bytesPerPixel == 4 && args.src.format.bytesPerPixel == 2) {
		DrawInnerImpl_AVX2::drawInner4BppWithConv<4, 2, Scale>(args);
	} else if (format.bytesPerPixel == 2 && args.src.format.bytesPerPixel == 4) {
		DrawInnerImpl_AVX2::drawInner4BppWithConv<2, 4, Scale>(args);
	}
}

template void BITMAP::drawAVX2<false>(DrawInnerArgs &);
template void BITMAP::drawAVX2<true>(DrawInnerArgs &);

} // namespace AGS3

#if defined(__clang__)
#pragma clang attribute pop
#elif defined(__GNUC__)
#pragma GCC pop_options
#endif
