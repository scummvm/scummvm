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

/***************************************************************************/
/*                                                                         */
/*  ftcalc.c                                                               */
/*                                                                         */
/*    Arithmetic computations (body).                                      */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftcalc.h"
#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"


/* we need to define a 64-bits data type here */

namespace AGS3 {
namespace FreeType213 {

#ifdef FT2_1_3_LONG64

typedef FT2_1_3_INT64 FT_Int64;

#else

typedef struct FT_Int64_ {
	FT_UInt32 lo;
	FT_UInt32 hi;
} FT_Int64;

#endif /* FT2_1_3_LONG64 */

/* The macro FT2_1_3_COMPONENT is used in trace mode. */
#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_calc


/* The following three functions are available regardless of whether */
/* FT2_1_3_LONG64 is defined.                                             */

FT2_1_3_EXPORT_DEF(FT_Fixed)
FT2_1_3_RoundFix(FT_Fixed a) {
	return (a >= 0) ? (a + 0x8000L) & -0x10000L
					: -((-a + 0x8000L) & -0x10000L);
}

FT2_1_3_EXPORT_DEF(FT_Fixed)
FT2_1_3_CeilFix(FT_Fixed a) {
	return (a >= 0) ? (a + 0xFFFFL) & -0x10000L
					: -((-a + 0xFFFFL) & -0x10000L);
}

FT2_1_3_EXPORT_DEF(FT_Fixed)
FT2_1_3_FloorFix(FT_Fixed a) {
	return (a >= 0) ? a & -0x10000L
					: -((-a) & -0x10000L);
}

FT2_1_3_EXPORT_DEF(FT_Int32)
FT_Sqrt32(FT_Int32 x) {
	FT_ULong val, root, newroot, mask;

	root = 0;
	mask = 0x40000000L;
	val = (FT_ULong)x;

	do {
		newroot = root + mask;
		if (newroot <= val) {
			val -= newroot;
			root = newroot + mask;
		}

		root >>= 1;
		mask >>= 2;

	} while (mask != 0);

	return root;
}


#ifdef FT2_1_3_LONG64

FT2_1_3_EXPORT_DEF(FT_Long)
FT_MulDiv(FT_Long a, FT_Long b, FT_Long c) {
	FT_Int s;
	FT_Long d;

	s = 1;
	if (a < 0) {
		a = -a;
		s = -1;
	}
	if (b < 0) {
		b = -b;
		s = -s;
	}
	if (c < 0) {
		c = -c;
		s = -s;
	}

	d = (FT_Long)(c > 0 ? ((FT_Int64)a * b + (c >> 1)) / c
						: 0x7FFFFFFFL);

	return (s > 0) ? d : -d;
}

FT2_1_3_EXPORT_DEF(FT_Long)
FT2_1_3_MulFix(FT_Long a, FT_Long b) {
	FT_Int s = 1;
	FT_Long c;

	if (a < 0) {
		a = -a;
		s = -1;
	}
	if (b < 0) {
		b = -b;
		s = -s;
	}

	c = (FT_Long)(((FT_Int64)a * b + 0x8000) >> 16);
	return (s > 0) ? c : -c;
}

FT2_1_3_EXPORT_DEF(FT_Long)
FT2_1_3_DivFix(FT_Long a, FT_Long b) {
	FT_Int32 s;
	FT_UInt32 q;

	s = 1;
	if (a < 0) {
		a = -a;
		s = -1;
	}
	if (b < 0) {
		b = -b;
		s = -s;
	}

	if (b == 0)
		/* check for division by 0 */
		q = 0x7FFFFFFFL;
	else
		/* compute result directly */
		q = (FT_UInt32)((((FT_Int64)a << 16) + (b >> 1)) / b);

	return (s < 0 ? -(FT_Long)q : (FT_Long)q);
}

#else /* FT2_1_3_LONG64 */

static void ft_multo64(FT_UInt32 x, FT_UInt32 y, FT_Int64 *z) {
	FT_UInt32 lo1, hi1, lo2, hi2, lo, hi, i1, i2;

	lo1 = x & 0x0000FFFFU;
	hi1 = x >> 16;
	lo2 = y & 0x0000FFFFU;
	hi2 = y >> 16;

	lo = lo1 * lo2;
	i1 = lo1 * hi2;
	i2 = lo2 * hi1;
	hi = hi1 * hi2;

	/* Check carry overflow of i1 + i2 */
	i1 += i2;
	hi += (FT_UInt32)(i1 < i2) << 16;

	hi += i1 >> 16;
	i1 = i1 << 16;

	/* Check carry overflow of i1 + lo */
	lo += i1;
	hi += (lo < i1);

	z->lo = lo;
	z->hi = hi;
}


static FT_UInt32 ft_div64by32(FT_UInt32 hi, FT_UInt32 lo, FT_UInt32 y) {
	FT_UInt32 r, q;
	FT_Int i;

	q = 0;
	r = hi;

	if (r >= y)
		return (FT_UInt32)0x7FFFFFFFL;

	i = 32;
	do {
		r <<= 1;
		q <<= 1;
		r |= lo >> 31;

		if (r >= (FT_UInt32)y) {
			r -= y;
			q |= 1;
		}
		lo <<= 1;
	} while (--i);

	return q;
}

FT2_1_3_EXPORT_DEF(void)
FT2_1_3_Add64(FT_Int64 *x, FT_Int64 *y, FT_Int64 *z) {
	register FT_UInt32 lo, hi, max;

	max = x->lo > y->lo ? x->lo : y->lo;
	lo = x->lo + y->lo;
	hi = x->hi + y->hi + (lo < max);

	z->lo = lo;
	z->hi = hi;
}

FT2_1_3_EXPORT_DEF(FT_Long)
FT_MulDiv(FT_Long a, FT_Long b, FT_Long c) {
	long s;

	if (a == 0 || b == c)
		return a;

	s = a;
	a = ABS(a);
	s ^= b;
	b = ABS(b);
	s ^= c;
	c = ABS(c);

	if (a <= 46340L && b <= 46340L && c <= 176095L && c > 0) {
		a = (a * b + (c >> 1)) / c;
	} else if (c > 0) {
		FT_Int64 temp, temp2;

		ft_multo64(a, b, &temp);

		temp2.hi = 0;
		temp2.lo = (FT_UInt32)(c >> 1);
		FT2_1_3_Add64(&temp, &temp2, &temp);
		a = ft_div64by32(temp.hi, temp.lo, c);
	} else
		a = 0x7FFFFFFFL;

	return (s < 0 ? -a : a);
}

FT2_1_3_EXPORT_DEF(FT_Long)
FT2_1_3_MulFix(FT_Long a, FT_Long b) {
	FT_Long s;
	FT_ULong ua, ub;

	if (a == 0 || b == 0x10000L)
		return a;

	s = a;
	a = ABS(a);
	s ^= b;
	b = ABS(b);

	ua = (FT_ULong)a;
	ub = (FT_ULong)b;

	if (ua <= 2048 && ub <= 1048576L) {
		ua = (ua * ub + 0x8000) >> 16;
	} else {
		FT_ULong al = ua & 0xFFFF;

		ua = (ua >> 16) * ub + al * (ub >> 16) +
			 ((al * (ub & 0xFFFF) + 0x8000) >> 16);
	}

	return (s < 0 ? -(FT_Long)ua : (FT_Long)ua);
}

FT2_1_3_EXPORT_DEF(FT_Long)
FT2_1_3_DivFix(FT_Long a, FT_Long b) {
	FT_Int32 s;
	FT_UInt32 q;

	s = a;
	a = ABS(a);
	s ^= b;
	b = ABS(b);

	if (b == 0) {
		/* check for division by 0 */
		q = 0x7FFFFFFFL;
	} else if ((a >> 16) == 0) {
		/* compute result directly */
		q = (FT_UInt32)((a << 16) + (b >> 1)) / (FT_UInt32)b;
	} else {
		/* we need more bits; we have to do it by hand */
		FT_Int64 temp, temp2;

		temp.hi = (FT_Int32)(a >> 16);
		temp.lo = (FT_UInt32)(a << 16);
		temp2.hi = 0;
		temp2.lo = (FT_UInt32)(b >> 1);
		FT2_1_3_Add64(&temp, &temp2, &temp);
		q = ft_div64by32(temp.hi, temp.lo, b);
	}

	return (s < 0 ? -(FT_Int32)q : (FT_Int32)q);
}

FT2_1_3_EXPORT_DEF(void)
FT2_1_3_MulTo64(FT_Int32 x, FT_Int32 y, FT_Int64 *z) {
	FT_Int32 s;

	s = x;
	x = ABS(x);
	s ^= y;
	y = ABS(y);

	ft_multo64(x, y, z);

	if (s < 0) {
		z->lo = (FT_UInt32) - (FT_Int32)z->lo;
		z->hi = ~z->hi + !(z->lo);
	}
}

/* apparently, the second version of this code is not compiled correctly */
/* on Mac machines with the MPW C compiler..  tsss, tsss, tss...         */

#if 1

FT2_1_3_EXPORT_DEF(FT_Int32)
FT2_1_3_Div64by32(FT_Int64 *x, FT_Int32 y) {
	FT_Int32 s;
	FT_UInt32 q, r, i, lo;

	s = x->hi;
	if (s < 0) {
		x->lo = (FT_UInt32) - (FT_Int32)x->lo;
		x->hi = ~x->hi + !x->lo;
	}
	s ^= y;
	y = ABS(y);

	/* Shortcut */
	if (x->hi == 0) {
		if (y > 0)
			q = x->lo / y;
		else
			q = 0x7FFFFFFFL;

		return (s < 0 ? -(FT_Int32)q : (FT_Int32)q);
	}

	r = x->hi;
	lo = x->lo;

	if (r >= (FT_UInt32)y) /* we know y is to be treated as unsigned here */
		return (s < 0 ? 0x80000001UL : 0x7FFFFFFFUL);
	/* Return Max/Min Int32 if division overflow. */
	/* This includes division by zero! */
	q = 0;
	for (i = 0; i < 32; i++) {
		r <<= 1;
		q <<= 1;
		r |= lo >> 31;

		if (r >= (FT_UInt32)y) {
			r -= y;
			q |= 1;
		}
		lo <<= 1;
	}

	return (s < 0 ? -(FT_Int32)q : (FT_Int32)q);
}

#else /* 0 */

FT2_1_3_EXPORT_DEF(FT_Int32)
FT2_1_3_Div64by32(FT_Int64 *x, FT_Int32 y) {
	FT_Int32 s;
	FT_UInt32 q;

	s = x->hi;
	if (s < 0) {
		x->lo = (FT_UInt32) - (FT_Int32)x->lo;
		x->hi = ~x->hi + !x->lo;
	}
	s ^= y;
	y = ABS(y);

	/* Shortcut */
	if (x->hi == 0) {
		if (y > 0)
			q = (x->lo + (y >> 1)) / y;
		else
			q = 0x7FFFFFFFL;

		return (s < 0 ? -(FT_Int32)q : (FT_Int32)q);
	}

	q = ft_div64by32(x->hi, x->lo, y);

	return (s < 0 ? -(FT_Int32)q : (FT_Int32)q);
}

#endif /* 0 */

#endif /* FT2_1_3_LONG64 */


/* a not-so-fast but working 16.16 fixed point square root function */

FT2_1_3_EXPORT_DEF(FT_Int32)
FT_SqrtFixed(FT_Int32 x) {
	FT_UInt32 root, rem_hi, rem_lo, test_div;
	FT_Int count;

	root = 0;

	if (x > 0) {
		rem_hi = 0;
		rem_lo = x;
		count = 24;
		do {
			rem_hi = (rem_hi << 2) | (rem_lo >> 30);
			rem_lo <<= 2;
			root <<= 1;
			test_div = (root << 1) + 1;

			if (rem_hi >= test_div) {
				rem_hi -= test_div;
				root += 1;
			}
		} while (--count);
	}

	return (FT_Int32)root;
}

} // End of namespace FreeType213
} // End of namespace AGS3
