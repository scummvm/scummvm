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
#ifndef AGS_LIB_ALLEGRO_SURFACE_SIMD_SSE_H
#define AGS_LIB_ALLEGRO_SURFACE_SIMD_SSE_H
#if defined(__x86_64__) || defined(__i686__) || defined(_M_X86) || defined(_M_X64)

#ifndef AGS_LIB_ALLEGRO_SURFACE_SIMD_IMPL
#define AGS_LIB_ALLEGRO_SURFACE_SIMD_IMPL
#endif

#include <immintrin.h>
#include "ags/globals.h"
#include "ags/lib/allegro/surface.h"

namespace AGS3 {

inline __m128i simd2BppTo4Bpp(__m128i pixels) {
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

inline __m128i simd4BppTo2Bpp(__m128i pixels) {
	// x is the final 16 bit rgb pixel
	__m128i x = _mm_srli_epi32(_mm_and_si128(pixels, _mm_set1_epi32(0x000000ff)), 3);
	x = _mm_or_si128(x, _mm_slli_epi32(_mm_srli_epi32(_mm_and_si128(pixels, _mm_set1_epi32(0x0000ff00)), 8+2), 5));
	x = _mm_or_si128(x, _mm_slli_epi32(_mm_srli_epi32(_mm_and_si128(pixels, _mm_set1_epi32(0x00ff0000)), 16+3), 11));
	x = _mm_slli_epi32(x, 16);
	x = _mm_srai_epi32(x, 16);
	return _mm_packs_epi32(x, _mm_setzero_si128());
}

inline __m128i rgbBlendSIMD2Bpp(__m128i srcCols, __m128i destCols, __m128i alphas) {
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

inline __m128i mul32_as32(__m128i a, __m128i b)
{
	__m128i tmp1 = _mm_mul_epu32(a,b);
	__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(a,4), _mm_srli_si128(b,4));
	return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE (0,0,2,0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE (0,0,2,0))); /* shuffle results to [63..0] and pack */
}

// preserveAlpha:
//		false => set destCols's alpha to 0
// 		true => keep destCols's alpha
inline __m128i rgbBlendSIMD(__m128i srcCols, __m128i destCols, __m128i alphas, bool preserveAlpha) {
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

inline __m128i argbBlendSIMD(__m128i srcCols, __m128i destCols) {
	__m128 srcA = _mm_cvtepi32_ps(_mm_srli_epi32(srcCols, 24));
	srcA = _mm_mul_ps(srcA, _mm_set1_ps(1.0 / 255.0));
	__m128 srcR = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(srcCols, 16), _mm_set1_epi32(0xff)));
	__m128 srcG = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(srcCols, 8), _mm_set1_epi32(0xff)));
	__m128 srcB = _mm_cvtepi32_ps(_mm_and_si128(srcCols, _mm_set1_epi32(0xff)));

	__m128 destA = _mm_cvtepi32_ps(_mm_srli_epi32(destCols, 24));
	destA = _mm_mul_ps(destA, _mm_set1_ps(1.0 / 255.0));
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

inline __m128i blendTintSpriteSIMD(__m128i srcCols, __m128i destCols, __m128i alphas, bool light) {
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
	// to 4 float32x4_t's each being a seperate channel with each lane
	// corresponding to their respective srcCols lane
	// dda = { A[0], A[1], A[2], A[3] }
	// ddr = { R[0], R[1], R[2], R[3] }
	// ddg = { G[0], G[1], G[2], G[3] }
	// ddb = { B[0], B[1], B[2], B[3] }

	// do the transformation (we don't actually need alpha at all)
	__m128 ddr, ddg, ddb;
	ddr = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(destCols, 16), _mm_set1_epi32(0xff))), _mm_set1_ps(1.0 / 255.0));
	ddg = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(destCols, 8), _mm_set1_epi32(0xff))), _mm_set1_ps(1.0 / 255.0));
	ddb = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(destCols, _mm_set1_epi32(0xff))), _mm_set1_ps(1.0 / 255.0));
	__m128 ssr, ssg, ssb;
	ssr = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(srcCols, 16), _mm_set1_epi32(0xff))), _mm_set1_ps(1.0 / 255.0));
	ssg = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(srcCols, 8), _mm_set1_epi32(0xff))), _mm_set1_ps(1.0 / 255.0));
	ssb = _mm_mul_ps(_mm_cvtepi32_ps(_mm_and_si128(srcCols, _mm_set1_epi32(0xff))), _mm_set1_ps(1.0 / 255.0));

	// Get the maxes and mins (needed for HSV->RGB and visa-versa)
	__m128 dmaxes = _mm_max_ps(ddr, _mm_max_ps(ddg, ddb));
	__m128 smaxes = _mm_max_ps(ssr, _mm_max_ps(ssg, ssb));
	__m128 smins = _mm_min_ps(ssr, _mm_min_ps(ssg, ssb));

	// This is here to stop from dividing by 0
	const __m128 eplison0 = _mm_set1_ps(0.0000001);

	__m128 chroma = _mm_max_ps(_mm_sub_ps(smaxes, smins), eplison0);

	// RGB to HSV is a piecewise function, so we compute each part of the function first...
	__m128 hr, hg, hb, hue;
	hr = _mm_div_ps(_mm_sub_ps(ssg, ssb), chroma);
	hr = _mm_sub_ps(hr, _mm_mul_ps(_mm_cvtepi32_ps(_mm_cvtps_epi32(_mm_mul_ps(hr, _mm_set1_ps(1.0 / 6.0)))), _mm_set1_ps(6.0)));
	hr = _mm_add_ps(hr, _mm_and_ps(_mm_cmplt_ps(hr, _mm_setzero_ps()), _mm_set1_ps(6.0)));
	hg = _mm_add_ps(_mm_div_ps(_mm_sub_ps(ssb, ssr), chroma), _mm_set1_ps(2.0));
	hg = _mm_max_ps(hg, _mm_setzero_ps());
	hb = _mm_add_ps(_mm_div_ps(_mm_sub_ps(ssr, ssg), chroma), _mm_set1_ps(4.0));
	hb = _mm_max_ps(hb, _mm_setzero_ps());

	// And then compute which one will be used based on criteria
	__m128 hrfactors = _mm_and_ps(_mm_and_ps(_mm_cmpeq_ps(ssr, smaxes), _mm_cmpneq_ps(ssr, ssb)), _mm_set1_ps(1.0));
	__m128 hgfactors = _mm_and_ps(_mm_and_ps(_mm_cmpeq_ps(ssg, smaxes), _mm_cmpneq_ps(ssg, ssr)), _mm_set1_ps(1.0));
	__m128 hbfactors = _mm_and_ps(_mm_and_ps(_mm_cmpeq_ps(ssb, smaxes), _mm_cmpneq_ps(ssb, ssg)), _mm_set1_ps(1.0));
	hue = _mm_mul_ps(hr, hrfactors);
	hue = _mm_add_ps(hue, _mm_mul_ps(hg, hgfactors));
	hue = _mm_add_ps(hue, _mm_mul_ps(hb, hbfactors));

	// Mess with the light like the original function
	__m128 val = dmaxes;
	if (light) {
		val = _mm_sub_ps(val, _mm_sub_ps(_mm_set1_ps(1.0), _mm_mul_ps(_mm_cvtepi32_ps(alphas), _mm_set1_ps(1.0 / 250.0))));
		val = _mm_max_ps(val, _mm_setzero_ps());
	}
		
	// then it stiches the HSV back together
	// the hue and saturation come from the source (tint) color, and the value comes from
	// the destinaion (real source) color
	chroma = _mm_mul_ps(val, _mm_div_ps(_mm_sub_ps(smaxes, smins), _mm_add_ps(smaxes, eplison0)));
	__m128 hprime_mod2 = _mm_mul_ps(hue, _mm_set1_ps(1.0 / 2.0));
	hprime_mod2 = _mm_mul_ps(_mm_sub_ps(hprime_mod2, _mm_cvtepi32_ps(_mm_cvtps_epi32(_mm_sub_ps(hprime_mod2, _mm_set1_ps(0.5))))), _mm_set1_ps(2.0));
	__m128 x = _mm_mul_ps(chroma, _mm_sub_ps(_mm_set1_ps(1), _mm_and_ps(_mm_castsi128_ps(_mm_set1_epi32(0x7fffffff)), _mm_sub_ps(hprime_mod2, _mm_set1_ps(1)))));
	//float32x4_t x = vmulq_f32(chroma, vsubq_f32(vmovq_n_f32(1.0), vabsq_f32(vsubq_f32(hprime_mod2, vmovq_n_f32(1.0)))));
	__m128i hprime_rounded = _mm_cvtps_epi32(_mm_sub_ps(hue, _mm_set1_ps(0.5)));
	__m128i x_int = _mm_cvtps_epi32(_mm_mul_ps(x, _mm_set1_ps(255.0)));
	__m128i c_int = _mm_cvtps_epi32(_mm_mul_ps(chroma, _mm_set1_ps(255.0)));

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
	__m128i val_add = _mm_cvtps_epi32(_mm_mul_ps(_mm_sub_ps(val, chroma), _mm_set1_ps(255.0)));
	val_add = _mm_or_si128(val_add, _mm_or_si128(_mm_slli_epi32(val_add, 8), _mm_or_si128(_mm_slli_epi32(val_add, 16), _mm_and_si128(destCols, _mm_set1_epi32(0xff000000)))));
	final = _mm_add_epi32(final, val_add);
	return final;
}

inline __m128i mul32_as16(__m128i a, __m128i b) {	
	__m128i a16 = _mm_packs_epi32(a, _mm_setzero_si128());
	__m128i b16 = _mm_packs_epi32(b, _mm_setzero_si128());
	__m128i res = _mm_mullo_epi16(a16, b16);
	return _mm_unpacklo_epi16(res, _mm_setzero_si128());
}

inline __m128i findmin32_as16(__m128i a, __m128i b) {
	__m128i a16 = _mm_packs_epi32(a, _mm_setzero_si128());
	__m128i b16 = _mm_packs_epi32(b, _mm_setzero_si128());
	__m128i res = _mm_min_epi16(a16, b16);
	return _mm_unpacklo_epi16(res, _mm_setzero_si128());
}

inline __m128i blendPixelSIMD(__m128i srcCols, __m128i destCols, __m128i alphas) {
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

#include "common/debug.h"
inline __m128i blendPixelSIMD2Bpp(__m128i srcCols, __m128i destCols, __m128i alphas) {
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
inline void drawPixelSIMD(byte *destPtr, const byte *srcP2, __m128i tint, __m128i alphas, __m128i maskedAlphas, __m128i transColors, int xDir, int xCtrBpp, int srcAlpha, int skipTrans, bool horizFlip, bool useTint, __m128i skipMask) {
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

inline void drawPixelSIMD2Bpp(byte *destPtr, const byte *srcP2, __m128i tint, __m128i alphas, __m128i transColors, int xDir, int xCtrBpp, int srcAlpha, int skipTrans, bool horizFlip, bool useTint, __m128i skipMask) {
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

} // namespace AGS3

#endif /* __x86_64__ __i686__ */
#endif /* AGS_LIB_ALLEGRO_SURFACE_SIMD_SSE */
