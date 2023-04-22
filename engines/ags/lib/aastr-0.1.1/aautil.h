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

/*
 * aautil.h --- helpers for anti-aliasing routines for Allegro
 *
 * This file is gift-ware.  This file is given to you freely
 * as a gift.  You may use, modify, redistribute, and generally hack
 * it about in any way you like, and you do not have to give anyone
 * anything in return.
 *
 * I do not accept any responsibility for any effects, adverse or
 * otherwise, that this code may have on just about anything that
 * you can think of.  Use it at your own risk.
 *
 * Copyright (C) 1998, 1999  Michael Bukin
 */

#ifndef AGS_LIB_AASTR_AAUTIL_H
#define AGS_LIB_AASTR_AAUTIL_H

#include "ags/lib/allegro.h"

namespace AGS3 {

/*
 * Change aa_BITS, and never aa_SIZE or aa_MASK.
 * 8 or 4 are probably the fastest for i386+.
 * Recompile the package after changing aa_BITS.
 */
#ifdef aa_BITS
#undef aa_BITS
#endif
#define aa_BITS     8

#if ((aa_BITS < 0) || (aa_BITS > 12))
#error aa_BITS must be (0 <= aa_BITS <= 12)
#endif

#define aa_SIZE     (1UL << aa_BITS)
#define aa_MASK     (aa_SIZE - 1)

#define aa_MAX_SIZE (1UL << 12)
#define aa_MAX_NUM  (aa_MAX_SIZE * aa_MAX_SIZE)


/* Prepare Bresenham line parameters.  */
#define aa_PREPARE(inc,dd,i1,i2,_yw,_xw)            \
	{                               \
	int xw = (_xw);                       \
	int yw = (_yw);                       \
	\
	if ((xw == 0) || ((yw < xw) && (yw > -xw))) {         \
		(inc) = 0;                          \
	}                             \
	else {                            \
		(inc) = yw / xw;                        \
		yw %= xw;                           \
	}                             \
	if (yw < 0) {                         \
		(inc) -= 1;                         \
		yw += xw;                           \
	}                             \
	(i2) = ((dd) = ((i1) = 2 * yw) - xw) - xw;            \
	}

/* Advance to the next point.  */
#define aa_ADVANCE(y,inc,dd,i1,i2)              \
	{                               \
	if ((dd) >= 0)                        \
		(y) += (inc) + 1, (dd) += (i2);             \
	else                              \
		(y) += (inc), (dd) += (i1);                 \
	}


#ifdef __cplusplus
extern "C" {
#endif

/* Prepare offsets for direct access to 24bpp bitmap.  */
void _aa_prepare_for_24bpp(void);

/* Add r,g,b values from source bitmap.  */
void _aa_add_rgb8(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num);
#ifdef ALLEGRO_COLOR16
void _aa_add_rgb15(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num);
void _aa_add_rgb16(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num);
#endif
#ifdef ALLEGRO_COLOR24
void _aa_add_rgb24(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num);
#endif
#ifdef ALLEGRO_COLOR32
void _aa_add_rgb32(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num);
#endif

/* Put pixel to destination bitmap.  */
void _aa_put_rgb8(byte *addr, int _x);
#ifdef ALLEGRO_COLOR16
void _aa_put_rgb15(byte *addr, int _x);
void _aa_put_rgb16(byte *addr, int _x);
#endif
#ifdef ALLEGRO_COLOR24
void _aa_put_rgb24(byte *addr, int _x);
#endif
#ifdef ALLEGRO_COLOR32
void _aa_put_rgb32(byte *addr, int _x);
#endif

/* Add r,g,b and transparency values from source bitmap.  */
void _aa_masked_add_rgb8(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2,
                         uint32_t _num);
#ifdef ALLEGRO_COLOR16
void _aa_masked_add_rgb15(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2,
                          uint32_t _num);
void _aa_masked_add_rgb16(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2,
                          uint32_t _num);
#endif
#ifdef ALLEGRO_COLOR24
void _aa_masked_add_rgb24(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2,
                          uint32_t _num);
#endif
#ifdef ALLEGRO_COLOR32
void _aa_masked_add_rgb32(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2,
                          uint32_t _num);
#endif

/* Put masked pixel to destination bitmap.  */
void _aa_masked_put_rgb8(byte *addr, int _x);
#ifdef ALLEGRO_COLOR16
void _aa_masked_put_rgb15(byte *addr, int _x);
void _aa_masked_put_rgb16(byte *addr, int _x);
#endif
#ifdef ALLEGRO_COLOR24
void _aa_masked_put_rgb24(byte *addr, int _x);
#endif
#ifdef ALLEGRO_COLOR32
void _aa_masked_put_rgb32(byte *addr, int _x);
#endif

#ifdef __cplusplus
}
#endif

} // namespace AGS3

#endif
