/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file contains a C and MMX implementation of the Scale2x effect.
 *
 * You can find an high level description of the effect at :
 *
 * http://scale2x.sourceforge.net/
 *
 * Alternatively at the previous license terms, you are allowed to use this
 * code in your program with these conditions:
 * - the program is not used in commercial activities.
 * - the whole source code of the program is released with the binary.
 * - derivative works of the program are allowed.
 */

#include "ultima8/misc/pent_include.h"
#include "Scale2xScaler.h"
#include "Manips.h"

// SDL_cpuinfo.h needs SDL_types.h, but fails to include it
#include <SDL_types.h>
#include <SDL_cpuinfo.h>

namespace Pentagram {

/***************************************************************************/
/* Scale2x C implementation */

template<class uintX, class Manip, class uintS = uintX> class Scale2xScalerInternal {
public:

	/**
	 * Scale by a factor of 2 a row of pixels of 16 bits.
	 * This function operates like scale2x_8_def() but for 16 bits pixels.
	 * \param src0 Pointer at the first pixel of the previous row.
	 * \param src1 Pointer at the first pixel of the current row.
	 * \param src2 Pointer at the first pixel of the next row.
	 * \param count Length in pixels of the src0, src1 and src2 rows.
	 * It must be at least 2.
	 * \param dst0 First destination row, double length in pixels.
	 * \param dst1 Second destination row, double length in pixels.
	 */
	static inline void scale2x_def(uintX *dst0, uintX *dst1, const uintS *src0, const uintS *src1, const uintS *src2, unsigned count) {
		/* first pixel */
		if (src0[0] != src2[0] && src1[0] != src1[1]) {
			dst0[0] = Manip::copy(src1[0] == src0[0] ? src0[0] : src1[0]);
			dst0[1] = Manip::copy(src1[1] == src0[0] ? src0[0] : src1[0]);
			dst1[0] = Manip::copy(src1[0] == src2[0] ? src2[0] : src1[0]);
			dst1[1] = Manip::copy(src1[1] == src2[0] ? src2[0] : src1[0]);
		} else {
			dst0[0] = Manip::copy(src1[0]);
			dst0[1] = Manip::copy(src1[0]);
			dst1[0] = Manip::copy(src1[0]);
			dst1[1] = Manip::copy(src1[0]);
		}
		++src0;
		++src1;
		++src2;
		dst0 += 2;
		dst1 += 2;

		/* central pixels */
		count -= 2;
		while (count) {
			if (src0[0] != src2[0] && src1[-1] != src1[1]) {
				dst0[0] = Manip::copy(src1[-1] == src0[0] ? src0[0] : src1[0]);
				dst0[1] = Manip::copy(src1[1] == src0[0] ? src0[0] : src1[0]);
				dst1[0] = Manip::copy(src1[-1] == src2[0] ? src2[0] : src1[0]);
				dst1[1] = Manip::copy(src1[1] == src2[0] ? src2[0] : src1[0]);
			} else {
				dst0[0] = Manip::copy(src1[0]);
				dst0[1] = Manip::copy(src1[0]);
				dst1[0] = Manip::copy(src1[0]);
				dst1[1] = Manip::copy(src1[0]);
			}

			++src0;
			++src1;
			++src2;
			dst0 += 2;
			dst1 += 2;
			--count;
		}

		/* last pixel */
		if (src0[0] != src2[0] && src1[-1] != src1[0]) {
			dst0[0] = Manip::copy(src1[-1] == src0[0] ? src0[0] : src1[0]);
			dst0[1] = Manip::copy(src1[0] == src0[0] ? src0[0] : src1[0]);
			dst1[0] = Manip::copy(src1[-1] == src2[0] ? src2[0] : src1[0]);
			dst1[1] = Manip::copy(src1[0] == src2[0] ? src2[0] : src1[0]);
		} else {
			dst0[0] = Manip::copy(src1[0]);
			dst0[1] = Manip::copy(src1[0]);
			dst1[0] = Manip::copy(src1[0]);
			dst1[1] = Manip::copy(src1[0]);
		}
	}


	static bool Scale(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
	                  uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
		// Must be at least 3 high
		if (sh < 3 && (clamp_src || tex->height < 3)) return false;

		// Source buffer pointers
		uintS *texel = reinterpret_cast<uintS *>(tex->buffer) + (sy * tex->width + sx);
		int tpitch = tex->width;
//	uintS *tline_end = texel + sw;
		uintS *tex_end = texel + (sh - 1) * tex->width;

		bool clip_y = true;
		if (sh + sy < tex->height && clamp_src == false) {
			clip_y = false;
			tex_end = texel + sh * tex->width;
		}

		if (sy == 0) {
			scale2x_def(reinterpret_cast<uintX *>(pixel),
			            reinterpret_cast<uintX *>(pixel + pitch),
			            texel, texel, texel + tpitch, sw);
			pixel += pitch * 2;
			texel += tpitch;
		}

		// Src Loop Y
		if (texel != tex_end) do {

				scale2x_def(reinterpret_cast<uintX *>(pixel),
				            reinterpret_cast<uintX *>(pixel + pitch),
				            texel - tpitch, texel, texel + tpitch, sw);
				pixel += pitch * 2;
				texel += tpitch;

			} while (texel != tex_end);

		if (clip_y) {
			scale2x_def(reinterpret_cast<uintX *>(pixel),
			            reinterpret_cast<uintX *>(pixel + pitch),
			            texel - tpitch, texel, texel, sw);
		}

		return true;
	}

};

/***************************************************************************/
/* Scale2x MMX implementation */

#if (defined(__GNUC__) && defined(__i386__)) || (defined(_MSC_VER) && defined(_M_IX86))

#ifdef _MSC_VER
#pragma warning(disable:4799)   // No EMMS at end of function
#endif

/*
 * Apply the Scale2x effect at a single row.
 * This function must be called only by the other scale2x functions.
 *
 * Considering the pixel map :
 *
 *      ABC (src0)
 *      DEF (src1)
 *      GHI (src2)
 *
 * this functions compute 2 new pixels in substitution of the source pixel E
 * like this map :
 *
 *      ab (dst)
 *
 * with these variables :
 *
 *      &current -> E
 *      &current_left -> D
 *      &current_right -> F
 *      &current_upper -> B
 *      &current_lower -> H
 *
 *      %0 -> current_upper
 *      %1 -> current
 *      %2 -> current_lower
 *      %3 -> dst
 *      %4 -> counter
 *
 *      %mm0 -> *current_left
 *      %mm1 -> *current_next
 *      %mm2 -> tmp0
 *      %mm3 -> tmp1
 *      %mm4 -> tmp2
 *      %mm5 -> tmp3
 *      %mm6 -> *current_upper
 *      %mm7 -> *current
 */
static inline void scale2x_16_mmx_border(uint16 *dst, const uint16 *src0, const uint16 *src1, const uint16 *src2, unsigned count) {
	/* always do the first and last run */
	count -= 2 * 4;

#if defined(__GNUC__) && defined(__i386__)
	__asm__ __volatile__(
	    /* first run */
	    /* set the current, current_pre, current_next registers */
	    "movq 0(%1), %%mm0\n"
	    "movq 0(%1), %%mm7\n"
	    "movq 8(%1), %%mm1\n"
	    "psllq $48, %%mm0\n"
	    "psllq $48, %%mm1\n"
	    "psrlq $48, %%mm0\n"
	    "movq %%mm7, %%mm2\n"
	    "movq %%mm7, %%mm3\n"
	    "psllq $16, %%mm2\n"
	    "psrlq $16, %%mm3\n"
	    "por %%mm2, %%mm0\n"
	    "por %%mm3, %%mm1\n"

	    /* current_upper */
	    "movq (%0), %%mm6\n"

	    /* compute the upper-left pixel for dst on %%mm2 */
	    /* compute the upper-right pixel for dst on %%mm4 */
	    "movq %%mm0, %%mm2\n"
	    "movq %%mm1, %%mm4\n"
	    "movq %%mm0, %%mm3\n"
	    "movq %%mm1, %%mm5\n"
	    "pcmpeqw %%mm6, %%mm2\n"
	    "pcmpeqw %%mm6, %%mm4\n"
	    "pcmpeqw (%2), %%mm3\n"
	    "pcmpeqw (%2), %%mm5\n"
	    "pandn %%mm2, %%mm3\n"
	    "pandn %%mm4, %%mm5\n"
	    "movq %%mm0, %%mm2\n"
	    "movq %%mm1, %%mm4\n"
	    "pcmpeqw %%mm1, %%mm2\n"
	    "pcmpeqw %%mm0, %%mm4\n"
	    "pandn %%mm3, %%mm2\n"
	    "pandn %%mm5, %%mm4\n"
	    "movq %%mm2, %%mm3\n"
	    "movq %%mm4, %%mm5\n"
	    "pand %%mm6, %%mm2\n"
	    "pand %%mm6, %%mm4\n"
	    "pandn %%mm7, %%mm3\n"
	    "pandn %%mm7, %%mm5\n"
	    "por %%mm3, %%mm2\n"
	    "por %%mm5, %%mm4\n"

	    /* set *dst */
	    "movq %%mm2, %%mm3\n"
	    "punpcklwd %%mm4, %%mm2\n"
	    "punpckhwd %%mm4, %%mm3\n"
	    "movq %%mm2, (%3)\n"
	    "movq %%mm3, 8(%3)\n"

	    /* next */
	    "addl $8, %0\n"
	    "addl $8, %1\n"
	    "addl $8, %2\n"
	    "addl $16, %3\n"

	    /* central runs */
	    "shrl $2, %4\n"
	    "jz 1f\n"

	    "0:\n"

	    /* set the current, current_pre, current_next registers */
	    "movq -8(%1), %%mm0\n"
	    "movq (%1), %%mm7\n"
	    "movq 8(%1), %%mm1\n"
	    "psrlq $48, %%mm0\n"
	    "psllq $48, %%mm1\n"
	    "movq %%mm7, %%mm2\n"
	    "movq %%mm7, %%mm3\n"
	    "psllq $16, %%mm2\n"
	    "psrlq $16, %%mm3\n"
	    "por %%mm2, %%mm0\n"
	    "por %%mm3, %%mm1\n"

	    /* current_upper */
	    "movq (%0), %%mm6\n"

	    /* compute the upper-left pixel for dst on %%mm2 */
	    /* compute the upper-right pixel for dst on %%mm4 */
	    "movq %%mm0, %%mm2\n"
	    "movq %%mm1, %%mm4\n"
	    "movq %%mm0, %%mm3\n"
	    "movq %%mm1, %%mm5\n"
	    "pcmpeqw %%mm6, %%mm2\n"
	    "pcmpeqw %%mm6, %%mm4\n"
	    "pcmpeqw (%2), %%mm3\n"
	    "pcmpeqw (%2), %%mm5\n"
	    "pandn %%mm2, %%mm3\n"
	    "pandn %%mm4, %%mm5\n"
	    "movq %%mm0, %%mm2\n"
	    "movq %%mm1, %%mm4\n"
	    "pcmpeqw %%mm1, %%mm2\n"
	    "pcmpeqw %%mm0, %%mm4\n"
	    "pandn %%mm3, %%mm2\n"
	    "pandn %%mm5, %%mm4\n"
	    "movq %%mm2, %%mm3\n"
	    "movq %%mm4, %%mm5\n"
	    "pand %%mm6, %%mm2\n"
	    "pand %%mm6, %%mm4\n"
	    "pandn %%mm7, %%mm3\n"
	    "pandn %%mm7, %%mm5\n"
	    "por %%mm3, %%mm2\n"
	    "por %%mm5, %%mm4\n"

	    /* set *dst */
	    "movq %%mm2, %%mm3\n"
	    "punpcklwd %%mm4, %%mm2\n"
	    "punpckhwd %%mm4, %%mm3\n"
	    "movq %%mm2, (%3)\n"
	    "movq %%mm3, 8(%3)\n"

	    /* next */
	    "addl $8, %0\n"
	    "addl $8, %1\n"
	    "addl $8, %2\n"
	    "addl $16, %3\n"

	    "decl %4\n"
	    "jnz 0b\n"
	    "1:\n"

	    /* final run */
	    /* set the current, current_pre, current_next registers */
	    "movq (%1), %%mm1\n"
	    "movq (%1), %%mm7\n"
	    "movq -8(%1), %%mm0\n"
	    "psrlq $48, %%mm1\n"
	    "psrlq $48, %%mm0\n"
	    "psllq $48, %%mm1\n"
	    "movq %%mm7, %%mm2\n"
	    "movq %%mm7, %%mm3\n"
	    "psllq $16, %%mm2\n"
	    "psrlq $16, %%mm3\n"
	    "por %%mm2, %%mm0\n"
	    "por %%mm3, %%mm1\n"

	    /* current_upper */
	    "movq (%0), %%mm6\n"

	    /* compute the upper-left pixel for dst on %%mm2 */
	    /* compute the upper-right pixel for dst on %%mm4 */
	    "movq %%mm0, %%mm2\n"
	    "movq %%mm1, %%mm4\n"
	    "movq %%mm0, %%mm3\n"
	    "movq %%mm1, %%mm5\n"
	    "pcmpeqw %%mm6, %%mm2\n"
	    "pcmpeqw %%mm6, %%mm4\n"
	    "pcmpeqw (%2), %%mm3\n"
	    "pcmpeqw (%2), %%mm5\n"
	    "pandn %%mm2, %%mm3\n"
	    "pandn %%mm4, %%mm5\n"
	    "movq %%mm0, %%mm2\n"
	    "movq %%mm1, %%mm4\n"
	    "pcmpeqw %%mm1, %%mm2\n"
	    "pcmpeqw %%mm0, %%mm4\n"
	    "pandn %%mm3, %%mm2\n"
	    "pandn %%mm5, %%mm4\n"
	    "movq %%mm2, %%mm3\n"
	    "movq %%mm4, %%mm5\n"
	    "pand %%mm6, %%mm2\n"
	    "pand %%mm6, %%mm4\n"
	    "pandn %%mm7, %%mm3\n"
	    "pandn %%mm7, %%mm5\n"
	    "por %%mm3, %%mm2\n"
	    "por %%mm5, %%mm4\n"

	    /* set *dst */
	    "movq %%mm2, %%mm3\n"
	    "punpcklwd %%mm4, %%mm2\n"
	    "punpckhwd %%mm4, %%mm3\n"
	    "movq %%mm2, (%3)\n"
	    "movq %%mm3, 8(%3)\n"

	    : "+r"(src0), "+r"(src1), "+r"(src2), "+r"(dst), "+r"(count)
	    :
	    : "cc"
	);
#elif defined(_MSC_VER) && defined(_M_IX86)
	__asm {
		mov eax, src0;
		mov ebx, src1;
		mov ecx, src2;
		mov edx, dst;
		mov esi, count;

		/* first run */
		/* set the current, current_pre, current_next registers */
		movq mm0, [ebx+0];
		movq mm7, [ebx+0];
		movq mm1, [ebx+8];
		psllq mm0, 48;
		psllq mm1, 48;
		psrlq mm0, 48;
		movq mm2, mm7;
		movq mm3, mm7;
		psllq mm2, 16;
		psrlq mm3, 16;
		por mm0, mm2;
		por mm1, mm3;

		/* current_upper */
		movq mm6, [eax];

		/* compute the upper-left pixel for dst on mm2 */
		/* compute the upper-right pixel for dst on mm4 */
		movq mm2, mm0;
		movq mm4, mm1;
		movq mm3, mm0;
		movq mm5, mm1;
		pcmpeqw mm2, mm6;
		pcmpeqw mm4, mm6;
		pcmpeqw mm3, [ecx];
		pcmpeqw mm5, [ecx];
		pandn mm3, mm2;
		pandn mm5, mm4;
		movq mm2, mm0;
		movq mm4, mm1;
		pcmpeqw mm2, mm1;
		pcmpeqw mm4, mm0;
		pandn mm2, mm3;
		pandn mm4, mm5;
		movq mm3, mm2;
		movq mm5, mm4;
		pand mm2, mm6;
		pand mm4, mm6;
		pandn mm3, mm7;
		pandn mm5, mm7;
		por mm2, mm3;
		por mm4, mm5;

		/* set *dst */
		movq mm3, mm2;
		punpcklwd mm2, mm4;
		punpckhwd mm3, mm4;
		movq [edx], mm2;
		movq [edx+8], mm3;

		/* next */
		add eax, 8;
		add ebx, 8;
		add ecx, 8;
		add edx, 16;

		/* central runs */
		shr esi, 2;
		jz label1;
		align 4;
		label0:

		/* set the current, current_pre, current_next registers */
		movq mm0, [ebx-8];
		movq mm7, [ebx];
		movq mm1, [ebx+8];
		psrlq mm0, 48;
		psllq mm1, 48;
		movq mm2, mm7;
		movq mm3, mm7;
		psllq mm2, 16;
		psrlq mm3, 16;
		por mm0, mm2;
		por mm1, mm3;

		/* current_upper */
		movq mm6, [eax];

		/* compute the upper-left pixel for dst on mm2 */
		/* compute the upper-right pixel for dst on mm4 */
		movq mm2, mm0;
		movq mm4, mm1;
		movq mm3, mm0;
		movq mm5, mm1;
		pcmpeqw mm2, mm6;
		pcmpeqw mm4, mm6;
		pcmpeqw mm3, [ecx];
		pcmpeqw mm5, [ecx];
		pandn mm3, mm2;
		pandn mm5, mm4;
		movq mm2, mm0;
		movq mm4, mm1;
		pcmpeqw mm2, mm1;
		pcmpeqw mm4, mm0;
		pandn mm2, mm3;
		pandn mm4, mm5;
		movq mm3, mm2;
		movq mm5, mm4;
		pand mm2, mm6;
		pand mm4, mm6;
		pandn mm3, mm7;
		pandn mm5, mm7;
		por mm2, mm3;
		por mm4, mm5;

		/* set *dst */
		movq mm3, mm2;
		punpcklwd mm2, mm4;
		punpckhwd mm3, mm4;
		movq [edx], mm2;
		movq [edx+8], mm3;

		/* next */
		add eax, 8;
		add ebx, 8;
		add ecx, 8;
		add edx, 16;

		dec esi;
		jnz label0;
		label1:

		/* final run */
		/* set the current, current_pre, current_next registers */
		movq mm1, [ebx];
		movq mm7, [ebx];
		movq mm0, [ebx-8];
		psrlq mm1, 48;
		psrlq mm0, 48;
		psllq mm1, 48;
		movq mm2, mm7;
		movq mm3, mm7;
		psllq mm2, 16;
		psrlq mm3, 16;
		por mm0, mm2;
		por mm1, mm3;

		/* current_upper */
		movq mm6, [eax];

		/* compute the upper-left pixel for dst on mm2 */
		/* compute the upper-right pixel for dst on mm4 */
		movq mm2, mm0;
		movq mm4, mm1;
		movq mm3, mm0;
		movq mm5, mm1;
		pcmpeqw mm2, mm6;
		pcmpeqw mm4, mm6;
		pcmpeqw mm3, [ecx];
		pcmpeqw mm5, [ecx];
		pandn mm3, mm2;
		pandn mm5, mm4;
		movq mm2, mm0;
		movq mm4, mm1;
		pcmpeqw mm2, mm1;
		pcmpeqw mm4, mm0;
		pandn mm2, mm3;
		pandn mm4, mm5;
		movq mm3, mm2;
		movq mm5, mm4;
		pand mm2, mm6;
		pand mm4, mm6;
		pandn mm3, mm7;
		pandn mm5, mm7;
		por mm2, mm3;
		por mm4, mm5;

		/* set *dst */
		movq mm3, mm2;
		punpcklwd mm2, mm4;
		punpckhwd mm3, mm4;
		movq [edx], mm2;
		movq [edx+8], mm3;
	}
#endif
}

static inline void scale2x_32_mmx_border(uint32 *dst, const uint32 *src0, const uint32 *src1, const uint32 *src2, unsigned count) {
	/* always do the first and last run */
	count -= 2 * 2;

#if defined(__GNUC__) && defined(__i386__)
	__asm__ __volatile__(
	    /* first run */
	    /* set the current, current_pre, current_next registers */
	    "movq 0(%1), %%mm0\n"
	    "movq 0(%1), %%mm7\n"
	    "movq 8(%1), %%mm1\n"
	    "psllq $32, %%mm0\n"
	    "psllq $32, %%mm1\n"
	    "psrlq $32, %%mm0\n"
	    "movq %%mm7, %%mm2\n"
	    "movq %%mm7, %%mm3\n"
	    "psllq $32, %%mm2\n"
	    "psrlq $32, %%mm3\n"
	    "por %%mm2, %%mm0\n"
	    "por %%mm3, %%mm1\n"

	    /* current_upper */
	    "movq (%0), %%mm6\n"

	    /* compute the upper-left pixel for dst on %%mm2 */
	    /* compute the upper-right pixel for dst on %%mm4 */
	    "movq %%mm0, %%mm2\n"
	    "movq %%mm1, %%mm4\n"
	    "movq %%mm0, %%mm3\n"
	    "movq %%mm1, %%mm5\n"
	    "pcmpeqd %%mm6, %%mm2\n"
	    "pcmpeqd %%mm6, %%mm4\n"
	    "pcmpeqd (%2), %%mm3\n"
	    "pcmpeqd (%2), %%mm5\n"
	    "pandn %%mm2, %%mm3\n"
	    "pandn %%mm4, %%mm5\n"
	    "movq %%mm0, %%mm2\n"
	    "movq %%mm1, %%mm4\n"
	    "pcmpeqd %%mm1, %%mm2\n"
	    "pcmpeqd %%mm0, %%mm4\n"
	    "pandn %%mm3, %%mm2\n"
	    "pandn %%mm5, %%mm4\n"
	    "movq %%mm2, %%mm3\n"
	    "movq %%mm4, %%mm5\n"
	    "pand %%mm6, %%mm2\n"
	    "pand %%mm6, %%mm4\n"
	    "pandn %%mm7, %%mm3\n"
	    "pandn %%mm7, %%mm5\n"
	    "por %%mm3, %%mm2\n"
	    "por %%mm5, %%mm4\n"

	    /* set *dst */
	    "movq %%mm2, %%mm3\n"
	    "punpckldq %%mm4, %%mm2\n"
	    "punpckhdq %%mm4, %%mm3\n"
	    "movq %%mm2, (%3)\n"
	    "movq %%mm3, 8(%3)\n"

	    /* next */
	    "addl $8, %0\n"
	    "addl $8, %1\n"
	    "addl $8, %2\n"
	    "addl $16, %3\n"

	    /* central runs */
	    "shrl $1, %4\n"
	    "jz 1f\n"

	    "0:\n"

	    /* set the current, current_pre, current_next registers */
	    "movq -8(%1), %%mm0\n"
	    "movq (%1), %%mm7\n"
	    "movq 8(%1), %%mm1\n"
	    "psrlq $32, %%mm0\n"
	    "psllq $32, %%mm1\n"
	    "movq %%mm7, %%mm2\n"
	    "movq %%mm7, %%mm3\n"
	    "psllq $32, %%mm2\n"
	    "psrlq $32, %%mm3\n"
	    "por %%mm2, %%mm0\n"
	    "por %%mm3, %%mm1\n"

	    /* current_upper */
	    "movq (%0), %%mm6\n"

	    /* compute the upper-left pixel for dst on %%mm2 */
	    /* compute the upper-right pixel for dst on %%mm4 */
	    "movq %%mm0, %%mm2\n"
	    "movq %%mm1, %%mm4\n"
	    "movq %%mm0, %%mm3\n"
	    "movq %%mm1, %%mm5\n"
	    "pcmpeqd %%mm6, %%mm2\n"
	    "pcmpeqd %%mm6, %%mm4\n"
	    "pcmpeqd (%2), %%mm3\n"
	    "pcmpeqd (%2), %%mm5\n"
	    "pandn %%mm2, %%mm3\n"
	    "pandn %%mm4, %%mm5\n"
	    "movq %%mm0, %%mm2\n"
	    "movq %%mm1, %%mm4\n"
	    "pcmpeqd %%mm1, %%mm2\n"
	    "pcmpeqd %%mm0, %%mm4\n"
	    "pandn %%mm3, %%mm2\n"
	    "pandn %%mm5, %%mm4\n"
	    "movq %%mm2, %%mm3\n"
	    "movq %%mm4, %%mm5\n"
	    "pand %%mm6, %%mm2\n"
	    "pand %%mm6, %%mm4\n"
	    "pandn %%mm7, %%mm3\n"
	    "pandn %%mm7, %%mm5\n"
	    "por %%mm3, %%mm2\n"
	    "por %%mm5, %%mm4\n"

	    /* set *dst */
	    "movq %%mm2, %%mm3\n"
	    "punpckldq %%mm4, %%mm2\n"
	    "punpckhdq %%mm4, %%mm3\n"
	    "movq %%mm2, (%3)\n"
	    "movq %%mm3, 8(%3)\n"

	    /* next */
	    "addl $8, %0\n"
	    "addl $8, %1\n"
	    "addl $8, %2\n"
	    "addl $16, %3\n"

	    "decl %4\n"
	    "jnz 0b\n"
	    "1:\n"

	    /* final run */
	    /* set the current, current_pre, current_next registers */
	    "movq (%1), %%mm1\n"
	    "movq (%1), %%mm7\n"
	    "movq -8(%1), %%mm0\n"
	    "psrlq $32, %%mm1\n"
	    "psrlq $32, %%mm0\n"
	    "psllq $32, %%mm1\n"
	    "movq %%mm7, %%mm2\n"
	    "movq %%mm7, %%mm3\n"
	    "psllq $32, %%mm2\n"
	    "psrlq $32, %%mm3\n"
	    "por %%mm2, %%mm0\n"
	    "por %%mm3, %%mm1\n"

	    /* current_upper */
	    "movq (%0), %%mm6\n"

	    /* compute the upper-left pixel for dst on %%mm2 */
	    /* compute the upper-right pixel for dst on %%mm4 */
	    "movq %%mm0, %%mm2\n"
	    "movq %%mm1, %%mm4\n"
	    "movq %%mm0, %%mm3\n"
	    "movq %%mm1, %%mm5\n"
	    "pcmpeqd %%mm6, %%mm2\n"
	    "pcmpeqd %%mm6, %%mm4\n"
	    "pcmpeqd (%2), %%mm3\n"
	    "pcmpeqd (%2), %%mm5\n"
	    "pandn %%mm2, %%mm3\n"
	    "pandn %%mm4, %%mm5\n"
	    "movq %%mm0, %%mm2\n"
	    "movq %%mm1, %%mm4\n"
	    "pcmpeqd %%mm1, %%mm2\n"
	    "pcmpeqd %%mm0, %%mm4\n"
	    "pandn %%mm3, %%mm2\n"
	    "pandn %%mm5, %%mm4\n"
	    "movq %%mm2, %%mm3\n"
	    "movq %%mm4, %%mm5\n"
	    "pand %%mm6, %%mm2\n"
	    "pand %%mm6, %%mm4\n"
	    "pandn %%mm7, %%mm3\n"
	    "pandn %%mm7, %%mm5\n"
	    "por %%mm3, %%mm2\n"
	    "por %%mm5, %%mm4\n"

	    /* set *dst */
	    "movq %%mm2, %%mm3\n"
	    "punpckldq %%mm4, %%mm2\n"
	    "punpckhdq %%mm4, %%mm3\n"
	    "movq %%mm2, (%3)\n"
	    "movq %%mm3, 8(%3)\n"

	    : "+r"(src0), "+r"(src1), "+r"(src2), "+r"(dst), "+r"(count)
	    :
	    : "cc"
	);
#elif defined(_MSC_VER) && defined(_M_IX86)
	__asm {
		mov eax, src0;
		mov ebx, src1;
		mov ecx, src2;
		mov edx, dst;
		mov esi, count;

		/* first run */
		/* set the current, current_pre, current_next registers */
		movq mm0, [ebx+0];
		movq mm7, [ebx+0];
		movq mm1, [ebx+8];
		psllq mm0, 32;
		psllq mm1, 32;
		psrlq mm0, 32;
		movq mm2, mm7;
		movq mm3, mm7;
		psllq mm2, 32;
		psrlq mm3, 32;
		por mm0, mm2;
		por mm1, mm3;

		/* current_upper */
		movq mm6, [eax];

		/* compute the upper-left pixel for dst on mm2 */
		/* compute the upper-right pixel for dst on mm4 */
		movq mm2, mm0;
		movq mm4, mm1;
		movq mm3, mm0;
		movq mm5, mm1;
		pcmpeqd mm2, mm6;
		pcmpeqd mm4, mm6;
		pcmpeqd mm3, [ecx];
		pcmpeqd mm5, [ecx];
		pandn mm3, mm2;
		pandn mm5, mm4;
		movq mm2, mm0;
		movq mm4, mm1;
		pcmpeqd mm2, mm1;
		pcmpeqd mm4, mm0;
		pandn mm2, mm3;
		pandn mm4, mm5;
		movq mm3, mm2;
		movq mm5, mm4;
		pand mm2, mm6;
		pand mm4, mm6;
		pandn mm3, mm7;
		pandn mm5, mm7;
		por mm2, mm3;
		por mm4, mm5;

		/* set *dst */
		movq mm3, mm2;
		punpckldq mm2, mm4;
		punpckhdq mm3, mm4;
		movq [edx], mm2;
		movq [edx+8], mm3;

		/* next */
		add eax, 8;
		add ebx, 8;
		add ecx, 8;
		add edx, 16;

		/* central runs */
		shr esi, 1;
		jz label1;

		label0:

		/* set the current, current_pre, current_next registers */
		movq mm0, [ebx-8];
		movq mm7, [ebx];
		movq mm1, [ebx+8];
		psrlq mm0, 32;
		psllq mm1, 32;
		movq mm2, mm7;
		movq mm3, mm7;
		psllq mm2, 32;
		psrlq mm3, 32;
		por mm0, mm2;
		por mm1, mm3;

		/* current_upper */
		movq mm6, [eax];

		/* compute the upper-left pixel for dst on mm2 */
		/* compute the upper-right pixel for dst on mm4 */
		movq mm2, mm0;
		movq mm4, mm1;
		movq mm3, mm0;
		movq mm5, mm1;
		pcmpeqd mm2, mm6;
		pcmpeqd mm4, mm6;
		pcmpeqd mm3, [ecx];
		pcmpeqd mm5, [ecx];
		pandn mm3, mm2;
		pandn mm5, mm4;
		movq mm2, mm0;
		movq mm4, mm1;
		pcmpeqd mm2, mm1;
		pcmpeqd mm4, mm0;
		pandn mm2, mm3;
		pandn mm4, mm5;
		movq mm3, mm2;
		movq mm5, mm4;
		pand mm2, mm6;
		pand mm4, mm6;
		pandn mm3, mm7;
		pandn mm5, mm7;
		por mm2, mm3;
		por mm4, mm5;

		/* set *dst */
		movq mm3, mm2;
		punpckldq mm2, mm4;
		punpckhdq mm3, mm4;
		movq [edx], mm2;
		movq [edx+8], mm3;

		/* next */
		add eax, 8;
		add ebx, 8;
		add ecx, 8;
		add edx, 16;

		dec esi;
		jnz label0
		label1:

		/* final run */
		/* set the current, current_pre, current_next registers */
		movq mm1, [ebx];
		movq mm7, [ebx];
		movq mm0, [ebx-8];
		psrlq mm1, 32;
		psrlq mm0, 32;
		psllq mm1, 32;
		movq mm2, mm7;
		movq mm3, mm7;
		psllq mm2, 32;
		psrlq mm3, 32;
		por mm0, mm2;
		por mm1, mm3;

		/* current_upper */
		movq mm6, [eax];

		/* compute the upper-left pixel for dst on mm2 */
		/* compute the upper-right pixel for dst on mm4 */
		movq mm2, mm0;
		movq mm4, mm1;
		movq mm3, mm0;
		movq mm5, mm1;
		pcmpeqd mm2, mm6;
		pcmpeqd mm4, mm6;
		pcmpeqd mm3, [ecx];
		pcmpeqd mm5, [ecx];
		pandn mm3, mm2;
		pandn mm5, mm4;
		movq mm2, mm0;
		movq mm4, mm1;
		pcmpeqd mm2, mm1;
		pcmpeqd mm4, mm0;
		pandn mm2, mm3;
		pandn mm4, mm5;
		movq mm3, mm2;
		movq mm5, mm4;
		pand mm2, mm6;
		pand mm4, mm6;
		pandn mm3, mm7;
		pandn mm5, mm7;
		por mm2, mm3;
		por mm4, mm5;

		/* set *dst */
		movq mm3, mm2;
		punpckldq mm2, mm4;
		punpckhdq mm3, mm4;
		movq [edx], mm2;
		movq [edx+8], mm3;
	};
#endif
}

/**
 * End the use of the MMX instructions.
 * This function must be called before using any floating-point operations.
 */

static inline void scale2x_mmx_emms(void) {
#if defined(__GNUC__) && defined(__i386__)
	__asm__ __volatile__(
	    "emms"
	);
#elif defined(_MSC_VER) && defined(_M_IX86)
	__asm emms;
#endif
}

/**
 * Scale by a factor of 2 a row of pixels of 16 bits.
 * This function operates like scale2x_8_mmx() but for 16 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows. It must
 * be at least 8 and a multiple of 4.
 * \param dst0 First destination row, double length in pixels.
 * \param dst1 Second destination row, double length in pixels.
 */
static void scale2x_16_mmx(uint16 *dst0, uint16 *dst1, const uint16 *src0, const uint16 *src1, const uint16 *src2, unsigned count) {
	if (count % 4 != 0 || count < 8) {
		Scale2xScalerInternal<uint16, Manip_Nat2Nat_16>::scale2x_def(dst0, dst1, src0, src1, src2, count);
	} else {
		scale2x_16_mmx_border(dst0, src0, src1, src2, count);
		scale2x_16_mmx_border(dst1, src2, src1, src0, count);
	}
}

/**
 * Scale by a factor of 2 a row of pixels of 32 bits.
 * This function operates like scale2x_8_mmx() but for 32 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows. It must
 * be at least 4 and a multiple of 2.
 * \param dst0 First destination row, double length in pixels.
 * \param dst1 Second destination row, double length in pixels.
 */
static void scale2x_32_mmx(uint32 *dst0, uint32 *dst1, const uint32 *src0, const uint32 *src1, const uint32 *src2, unsigned count) {
	if (count % 2 != 0 || count < 4) {
		Scale2xScalerInternal<uint32, Manip_Nat2Nat_32>::scale2x_def(dst0, dst1, src0, src1, src2, count);
	} else {
		scale2x_32_mmx_border(dst0, src0, src1, src2, count);
		scale2x_32_mmx_border(dst1, src2, src1, src0, count);
	}
}

//
// Pentagram Scale2x Implementation
//

static bool Scale2x_16MMX(Texture *tex  , int32 sx, int32 sy, int32 sw, int32 sh,
                          uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
	// Must be at least 3 high
	if (sh < 3 && (clamp_src || tex->height < 3)) return false;

	// Source buffer pointers
	uint16 *texel = reinterpret_cast<uint16 *>(tex->buffer) + (sy * tex->width + sx);
	int tpitch = tex->width;
//	uint16 *tline_end = texel + sw;
	uint16 *tex_end = texel + (sh - 1) * tex->width;

	bool clip_y = true;
	if (sh + sy < tex->height && clamp_src == false) {
		clip_y = false;
		tex_end = texel + sh * tex->width;
	}

	if (sy == 0) {
		scale2x_16_mmx(reinterpret_cast<uint16 *>(pixel),
		               reinterpret_cast<uint16 *>(pixel + pitch),
		               texel, texel, texel + tpitch, sw);
		pixel += pitch * 2;
		texel += tpitch;
	}

	// Src Loop Y
	if (texel != tex_end) do {

			scale2x_16_mmx(reinterpret_cast<uint16 *>(pixel),
			               reinterpret_cast<uint16 *>(pixel + pitch),
			               texel - tpitch, texel, texel + tpitch, sw);
			pixel += pitch * 2;
			texel += tpitch;

		} while (texel != tex_end);

	if (clip_y) {
		scale2x_16_mmx(reinterpret_cast<uint16 *>(pixel),
		               reinterpret_cast<uint16 *>(pixel + pitch),
		               texel - tpitch, texel, texel, sw);
	}

	scale2x_mmx_emms();
	return true;
}

static bool Scale2x_32MMX(Texture *tex  , int32 sx, int32 sy, int32 sw, int32 sh,
                          uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
	// Must be at least 3 high
	if (sh < 3 && (clamp_src || tex->height < 3)) return false;

	// Source buffer pointers
	uint32 *texel = reinterpret_cast<uint32 *>(tex->buffer) + (sy * tex->width + sx);
	int tpitch = tex->width;
//	uint32 *tline_end = texel + sw;
	uint32 *tex_end = texel + (sh - 1) * tex->width;

	bool clip_y = true;
	if (sh + sy < tex->height && clamp_src == false) {
		clip_y = false;
		tex_end = texel + sh * tex->width;
	}

	if (sy == 0) {
		scale2x_32_mmx(reinterpret_cast<uint32 *>(pixel),
		               reinterpret_cast<uint32 *>(pixel + pitch),
		               texel, texel, texel + tpitch, sw);
		pixel += pitch * 2;
		texel += tpitch;
	}

	// Src Loop Y
	if (texel != tex_end) do {

			scale2x_32_mmx(reinterpret_cast<uint32 *>(pixel),
			               reinterpret_cast<uint32 *>(pixel + pitch),
			               texel - tpitch, texel, texel + tpitch, sw);
			pixel += pitch * 2;
			texel += tpitch;

		} while (texel != tex_end);

	if (clip_y) {
		scale2x_32_mmx(reinterpret_cast<uint32 *>(pixel),
		               reinterpret_cast<uint32 *>(pixel + pitch),
		               texel - tpitch, texel, texel, sw);
	}

	scale2x_mmx_emms();
	return true;
}
#endif

Scale2xScaler::Scale2xScaler() : Scaler() {
	Scale16Nat = Scale2xScalerInternal<uint16, Manip_Nat2Nat_16, uint16>::Scale;
	Scale16Sta = Scale2xScalerInternal<uint16, Manip_Sta2Nat_16, uint32>::Scale;

	Scale32Nat = Scale2xScalerInternal<uint32, Manip_Nat2Nat_32, uint32>::Scale;
	Scale32Sta = Scale2xScalerInternal<uint32, Manip_Sta2Nat_32, uint32>::Scale;
	Scale32_A888 = Scale2xScalerInternal<uint32, Manip_Nat2Nat_32, uint32>::Scale;
	Scale32_888A = Scale2xScalerInternal<uint32, Manip_Nat2Nat_32, uint32>::Scale;

#if (defined(__GNUC__) && defined(__i386__)) || (defined(_MSC_VER) && defined(_M_IX86))
	if (SDL_HasMMX()) {
		Scale16Nat = Scale2x_16MMX;

		Scale32Nat = Scale2x_32MMX;
		Scale32_A888 = Scale2x_32MMX;
		Scale32_888A = Scale2x_32MMX;
	}
#endif
}

const uint32 Scale2xScaler::ScaleBits() const {
	return 1 << 2;
}
const bool Scale2xScaler::ScaleArbitrary() const {
	return false;
}

const char *Scale2xScaler::ScalerName() const {
	return "scale2x";
}
const char *Scale2xScaler::ScalerDesc() const {
	return "AdvMame Scale2x Scaler";
}
const char *Scale2xScaler::ScalerCopyright() const {
	return "Copyright (C) 2001, 2002, 2003, 2004 Andrea Mazzoleni";
}

const Scale2xScaler scale2x_scaler;

};
