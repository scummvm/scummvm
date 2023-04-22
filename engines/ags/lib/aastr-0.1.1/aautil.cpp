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
 * aautil.c --- helpers for anti-aliasing routines for Allegro
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

#include "ags/lib/aastr-0.1.1/aautil.h"
#include "ags/globals.h"
#include "ags/lib/allegro/color.h"

namespace AGS3 {

/* Multiply b by a (0 <= a <= aa_SIZE). */
#define MUL(a, b) ((b) * (a))

static struct {
	int transparent;
	unsigned int r;
	unsigned int g;
	unsigned int b;
	int roffset24;
	int goffset24;
	int boffset24;
} _aa;

/*
 * Prepare offsets for direct access to 24bpp bitmap.
 */
void _aa_prepare_for_24bpp() {
	_aa.roffset24 = _G(_rgb_r_shift_24) / 8;
	_aa.goffset24 = _G(_rgb_g_shift_24) / 8;
	_aa.boffset24 = _G(_rgb_b_shift_24) / 8;
}

/*
 * Add r, g, b values of pixels.
 */
void _aa_add_rgb8(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num) {
	unsigned char *sline;
	int sx, sx1i, sx1f, sx2i, sx2f;
	int sy, sy1i, sy1f, sy2i, sy2f;
	unsigned int r1, g1, b1;
	unsigned int r2, g2, b2;
	unsigned int scolor;

	sy1i = _sy1 >> aa_BITS;
	sy = sy1i;

	/* First line.  */
	sx1i = _sx1 >> aa_BITS;
	sx = sx1i;
	sline = _src->line[sy] + sx;

	sx1f = aa_SIZE - (_sx1 & aa_MASK);
	scolor = *sline;
	r1 = MUL(getr8(scolor), sx1f);
	g1 = MUL(getg8(scolor), sx1f);
	b1 = MUL(getb8(scolor), sx1f);

	sx2i = _sx2 >> aa_BITS;
	for (sline++, sx++; sx < sx2i; sline++, sx++) {
		scolor = *sline;
		r1 += getr8(scolor) << aa_BITS;
		g1 += getg8(scolor) << aa_BITS;
		b1 += getb8(scolor) << aa_BITS;
	}

	sx2f = _sx2 & aa_MASK;
	if (sx2f != 0) {
		scolor = *sline;
		r1 += MUL(getr8(scolor), sx2f);
		g1 += MUL(getg8(scolor), sx2f);
		b1 += MUL(getb8(scolor), sx2f);
	}

	sy1f = aa_SIZE - (_sy1 & aa_MASK);
	r1 = MUL(r1, sy1f);
	g1 = MUL(g1, sy1f);
	b1 = MUL(b1, sy1f);

	/* Middle lines.  */
	sy2i = _sy2 >> aa_BITS;
	if (++sy < sy2i) {
		r2 = g2 = b2 = 0;
		do {
			sx = sx1i;
			sline = _src->line[sy] + sx;

			scolor = *sline;
			r2 += MUL(getr8(scolor), sx1f);
			g2 += MUL(getg8(scolor), sx1f);
			b2 += MUL(getb8(scolor), sx1f);

			for (sline++, sx++; sx < sx2i; sline++, sx++) {
				scolor = *sline;
				r2 += getr8(scolor) << aa_BITS;
				g2 += getg8(scolor) << aa_BITS;
				b2 += getb8(scolor) << aa_BITS;
			}

			if (sx2f != 0) {
				scolor = *sline;
				r2 += MUL(getr8(scolor), sx2f);
				g2 += MUL(getg8(scolor), sx2f);
				b2 += MUL(getb8(scolor), sx2f);
			}
		} while (++sy < sy2i);

		r1 += r2 << aa_BITS;
		g1 += g2 << aa_BITS;
		b1 += b2 << aa_BITS;
	}

	/* Last line.  */
	sy2f = _sy2 & aa_MASK;
	if (sy2f != 0) {
		sx = sx1i;
		sline = _src->line[sy] + sx;

		scolor = *sline;
		r2 = MUL(getr8(scolor), sx1f);
		g2 = MUL(getg8(scolor), sx1f);
		b2 = MUL(getb8(scolor), sx1f);

		for (sline++, sx++; sx < sx2i; sline++, sx++) {
			scolor = *sline;
			r2 += getr8(scolor) << aa_BITS;
			g2 += getg8(scolor) << aa_BITS;
			b2 += getb8(scolor) << aa_BITS;
		}

		if (sx2f != 0) {
			scolor = *sline;
			r2 += MUL(getr8(scolor), sx2f);
			g2 += MUL(getg8(scolor), sx2f);
			b2 += MUL(getb8(scolor), sx2f);
		}

		r1 += MUL(r2, sy2f);
		g1 += MUL(g2, sy2f);
		b1 += MUL(b2, sy2f);
	}

	if (_num == (aa_SIZE * aa_SIZE)) {
		_aa.r = r1 >> (2 * aa_BITS);
		_aa.g = g1 >> (2 * aa_BITS);
		_aa.b = b1 >> (2 * aa_BITS);
	} else {
		_aa.r = r1 / _num;
		_aa.g = g1 / _num;
		_aa.b = b1 / _num;
	}
}

#ifdef ALLEGRO_COLOR16
void _aa_add_rgb15(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num) {
	unsigned short *sline;
	int sx, sx1i, sx1f, sx2i, sx2f;
	int sy, sy1i, sy1f, sy2i, sy2f;
	unsigned int r1, g1, b1;
	unsigned int r2, g2, b2;
	unsigned int scolor;

	sy1i = _sy1 >> aa_BITS;
	sy = sy1i;

	/* First line.  */
	sx1i = _sx1 >> aa_BITS;
	sx = sx1i;
	sline = (unsigned short *)(_src->line[sy]) + sx;

	sx1f = aa_SIZE - (_sx1 & aa_MASK);
	scolor = *sline;
	r1 = MUL(getr15(scolor), sx1f);
	g1 = MUL(getg15(scolor), sx1f);
	b1 = MUL(getb15(scolor), sx1f);

	sx2i = _sx2 >> aa_BITS;
	for (sline++, sx++; sx < sx2i; sline++, sx++) {
		scolor = *sline;
		r1 += getr15(scolor) << aa_BITS;
		g1 += getg15(scolor) << aa_BITS;
		b1 += getb15(scolor) << aa_BITS;
	}

	sx2f = _sx2 & aa_MASK;
	if (sx2f != 0) {
		scolor = *sline;
		r1 += MUL(getr15(scolor), sx2f);
		g1 += MUL(getg15(scolor), sx2f);
		b1 += MUL(getb15(scolor), sx2f);
	}

	sy1f = aa_SIZE - (_sy1 & aa_MASK);
	r1 = MUL(r1, sy1f);
	g1 = MUL(g1, sy1f);
	b1 = MUL(b1, sy1f);

	/* Middle lines.  */
	sy2i = _sy2 >> aa_BITS;
	if (++sy < sy2i) {
		r2 = g2 = b2 = 0;
		do {
			sx = sx1i;
			sline = (unsigned short *)(_src->line[sy]) + sx;

			scolor = *sline;
			r2 += MUL(getr15(scolor), sx1f);
			g2 += MUL(getg15(scolor), sx1f);
			b2 += MUL(getb15(scolor), sx1f);

			for (sline++, sx++; sx < sx2i; sline++, sx++) {
				scolor = *sline;
				r2 += getr15(scolor) << aa_BITS;
				g2 += getg15(scolor) << aa_BITS;
				b2 += getb15(scolor) << aa_BITS;
			}

			if (sx2f != 0) {
				scolor = *sline;
				r2 += MUL(getr15(scolor), sx2f);
				g2 += MUL(getg15(scolor), sx2f);
				b2 += MUL(getb15(scolor), sx2f);
			}
		} while (++sy < sy2i);

		r1 += r2 << aa_BITS;
		g1 += g2 << aa_BITS;
		b1 += b2 << aa_BITS;
	}

	/* Last line.  */
	sy2f = _sy2 & aa_MASK;
	if (sy2f != 0) {
		sx = sx1i;
		sline = (unsigned short *)(_src->line[sy]) + sx;

		scolor = *sline;
		r2 = MUL(getr15(scolor), sx1f);
		g2 = MUL(getg15(scolor), sx1f);
		b2 = MUL(getb15(scolor), sx1f);

		for (sline++, sx++; sx < sx2i; sline++, sx++) {
			scolor = *sline;
			r2 += getr15(scolor) << aa_BITS;
			g2 += getg15(scolor) << aa_BITS;
			b2 += getb15(scolor) << aa_BITS;
		}

		if (sx2f != 0) {
			scolor = *sline;
			r2 += MUL(getr15(scolor), sx2f);
			g2 += MUL(getg15(scolor), sx2f);
			b2 += MUL(getb15(scolor), sx2f);
		}

		r1 += MUL(r2, sy2f);
		g1 += MUL(g2, sy2f);
		b1 += MUL(b2, sy2f);
	}

	if (_num == (aa_SIZE * aa_SIZE)) {
		_aa.r = r1 >> (2 * aa_BITS);
		_aa.g = g1 >> (2 * aa_BITS);
		_aa.b = b1 >> (2 * aa_BITS);
	} else {
		_aa.r = r1 / _num;
		_aa.g = g1 / _num;
		_aa.b = b1 / _num;
	}
}

void _aa_add_rgb16(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num) {
	unsigned short *sline;
	int sx, sx1i, sx1f, sx2i, sx2f;
	int sy, sy1i, sy1f, sy2i, sy2f;
	unsigned int r1, g1, b1;
	unsigned int r2, g2, b2;
	unsigned int scolor;

	sy1i = _sy1 >> aa_BITS;
	sy = sy1i;

	/* First line.  */
	sx1i = _sx1 >> aa_BITS;
	sx = sx1i;
	sline = (unsigned short *)(_src->line[sy]) + sx;

	sx1f = aa_SIZE - (_sx1 & aa_MASK);
	scolor = *sline;
	r1 = MUL(getr16(scolor), sx1f);
	g1 = MUL(getg16(scolor), sx1f);
	b1 = MUL(getb16(scolor), sx1f);

	sx2i = _sx2 >> aa_BITS;
	for (sline++, sx++; sx < sx2i; sline++, sx++) {
		scolor = *sline;
		r1 += getr16(scolor) << aa_BITS;
		g1 += getg16(scolor) << aa_BITS;
		b1 += getb16(scolor) << aa_BITS;
	}

	sx2f = _sx2 & aa_MASK;
	if (sx2f != 0) {
		scolor = *sline;
		r1 += MUL(getr16(scolor), sx2f);
		g1 += MUL(getg16(scolor), sx2f);
		b1 += MUL(getb16(scolor), sx2f);
	}

	sy1f = aa_SIZE - (_sy1 & aa_MASK);
	r1 = MUL(r1, sy1f);
	g1 = MUL(g1, sy1f);
	b1 = MUL(b1, sy1f);

	/* Middle lines.  */
	sy2i = _sy2 >> aa_BITS;
	if (++sy < sy2i) {
		r2 = g2 = b2 = 0;
		do {
			sx = sx1i;
			sline = (unsigned short *)(_src->line[sy]) + sx;

			scolor = *sline;
			r2 += MUL(getr16(scolor), sx1f);
			g2 += MUL(getg16(scolor), sx1f);
			b2 += MUL(getb16(scolor), sx1f);

			for (sline++, sx++; sx < sx2i; sline++, sx++) {
				scolor = *sline;
				r2 += getr16(scolor) << aa_BITS;
				g2 += getg16(scolor) << aa_BITS;
				b2 += getb16(scolor) << aa_BITS;
			}

			if (sx2f != 0) {
				scolor = *sline;
				r2 += MUL(getr16(scolor), sx2f);
				g2 += MUL(getg16(scolor), sx2f);
				b2 += MUL(getb16(scolor), sx2f);
			}
		} while (++sy < sy2i);

		r1 += r2 << aa_BITS;
		g1 += g2 << aa_BITS;
		b1 += b2 << aa_BITS;
	}

	/* Last line.  */
	sy2f = _sy2 & aa_MASK;
	if (sy2f != 0) {
		sx = sx1i;
		sline = (unsigned short *)(_src->line[sy]) + sx;

		scolor = *sline;
		r2 = MUL(getr16(scolor), sx1f);
		g2 = MUL(getg16(scolor), sx1f);
		b2 = MUL(getb16(scolor), sx1f);

		for (sline++, sx++; sx < sx2i; sline++, sx++) {
			scolor = *sline;
			r2 += getr16(scolor) << aa_BITS;
			g2 += getg16(scolor) << aa_BITS;
			b2 += getb16(scolor) << aa_BITS;
		}

		if (sx2f != 0) {
			scolor = *sline;
			r2 += MUL(getr16(scolor), sx2f);
			g2 += MUL(getg16(scolor), sx2f);
			b2 += MUL(getb16(scolor), sx2f);
		}

		r1 += MUL(r2, sy2f);
		g1 += MUL(g2, sy2f);
		b1 += MUL(b2, sy2f);
	}

	if (_num == (aa_SIZE * aa_SIZE)) {
		_aa.r = r1 >> (2 * aa_BITS);
		_aa.g = g1 >> (2 * aa_BITS);
		_aa.b = b1 >> (2 * aa_BITS);
	} else {
		_aa.r = r1 / _num;
		_aa.g = g1 / _num;
		_aa.b = b1 / _num;
	}
}
#endif

#ifdef ALLEGRO_COLOR24
void _aa_add_rgb24(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num) {
	unsigned char *sline;
	int sx, sx1i, sx1f, sx2i, sx2f;
	int sy, sy1i, sy1f, sy2i, sy2f;
	unsigned int r1, g1, b1;
	unsigned int r2, g2, b2;

	sy1i = _sy1 >> aa_BITS;
	sy = sy1i;

	/* First line.  */
	sx1i = _sx1 >> aa_BITS;
	sx = sx1i;
	sline = _src->line[sy] + sx * 3;

	sx1f = aa_SIZE - (_sx1 & aa_MASK);
	r1 = MUL(sline[_aa.roffset24], sx1f);
	g1 = MUL(sline[_aa.goffset24], sx1f);
	b1 = MUL(sline[_aa.boffset24], sx1f);

	sx2i = _sx2 >> aa_BITS;
	for (sline += 3, sx++; sx < sx2i; sline += 3, sx++) {
		r1 += sline[_aa.roffset24] << aa_BITS;
		g1 += sline[_aa.goffset24] << aa_BITS;
		b1 += sline[_aa.boffset24] << aa_BITS;
	}

	sx2f = _sx2 & aa_MASK;
	if (sx2f != 0) {
		r1 += MUL(sline[_aa.roffset24], sx2f);
		g1 += MUL(sline[_aa.goffset24], sx2f);
		b1 += MUL(sline[_aa.boffset24], sx2f);
	}

	sy1f = aa_SIZE - (_sy1 & aa_MASK);
	r1 = MUL(r1, sy1f);
	g1 = MUL(g1, sy1f);
	b1 = MUL(b1, sy1f);

	/* Middle lines.  */
	sy2i = _sy2 >> aa_BITS;
	if (++sy < sy2i) {
		r2 = g2 = b2 = 0;
		do {
			sx = sx1i;
			sline = _src->line[sy] + sx * 3;

			r2 += MUL(sline[_aa.roffset24], sx1f);
			g2 += MUL(sline[_aa.goffset24], sx1f);
			b2 += MUL(sline[_aa.boffset24], sx1f);

			for (sline += 3, sx++; sx < sx2i; sline += 3, sx++) {
				r2 += sline[_aa.roffset24] << aa_BITS;
				g2 += sline[_aa.goffset24] << aa_BITS;
				b2 += sline[_aa.boffset24] << aa_BITS;
			}

			if (sx2f != 0) {
				r2 += MUL(sline[_aa.roffset24], sx2f);
				g2 += MUL(sline[_aa.goffset24], sx2f);
				b2 += MUL(sline[_aa.boffset24], sx2f);
			}
		} while (++sy < sy2i);

		r1 += r2 << aa_BITS;
		g1 += g2 << aa_BITS;
		b1 += b2 << aa_BITS;
	}

	/* Last line.  */
	sy2f = _sy2 & aa_MASK;
	if (sy2f != 0) {
		sx = sx1i;
		sline = _src->line[sy] + sx * 3;

		r2 = MUL(sline[_aa.roffset24], sx1f);
		g2 = MUL(sline[_aa.goffset24], sx1f);
		b2 = MUL(sline[_aa.boffset24], sx1f);

		for (sline += 3, sx++; sx < sx2i; sline += 3, sx++) {
			r2 += sline[_aa.roffset24] << aa_BITS;
			g2 += sline[_aa.goffset24] << aa_BITS;
			b2 += sline[_aa.boffset24] << aa_BITS;
		}

		if (sx2f != 0) {
			r2 += MUL(sline[_aa.roffset24], sx2f);
			g2 += MUL(sline[_aa.goffset24], sx2f);
			b2 += MUL(sline[_aa.boffset24], sx2f);
		}

		r1 += MUL(r2, sy2f);
		g1 += MUL(g2, sy2f);
		b1 += MUL(b2, sy2f);
	}

	if (_num == (aa_SIZE * aa_SIZE)) {
		_aa.r = r1 >> (2 * aa_BITS);
		_aa.g = g1 >> (2 * aa_BITS);
		_aa.b = b1 >> (2 * aa_BITS);
	} else {
		_aa.r = r1 / _num;
		_aa.g = g1 / _num;
		_aa.b = b1 / _num;
	}
}
#endif

#ifdef ALLEGRO_COLOR32
void _aa_add_rgb32(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num) {
	unsigned int *sline;
	int sx, sx1i, sx1f, sx2i, sx2f;
	int sy, sy1i, sy1f, sy2i, sy2f;
	unsigned int r1, g1, b1;
	unsigned int r2, g2, b2;
	unsigned int scolor;

	sy1i = _sy1 >> aa_BITS;
	sy = sy1i;

	/* First line.  */
	sx1i = _sx1 >> aa_BITS;
	sx = sx1i;
	sline = (unsigned int *)(_src->line[sy]) + sx;

	sx1f = aa_SIZE - (_sx1 & aa_MASK);
	scolor = *sline;
	r1 = MUL(getr32(scolor), sx1f);
	g1 = MUL(getg32(scolor), sx1f);
	b1 = MUL(getb32(scolor), sx1f);

	sx2i = _sx2 >> aa_BITS;
	for (sline++, sx++; sx < sx2i; sline++, sx++) {
		scolor = *sline;
		r1 += getr32(scolor) << aa_BITS;
		g1 += getg32(scolor) << aa_BITS;
		b1 += getb32(scolor) << aa_BITS;
	}

	sx2f = _sx2 & aa_MASK;
	if (sx2f != 0) {
		scolor = *sline;
		r1 += MUL(getr32(scolor), sx2f);
		g1 += MUL(getg32(scolor), sx2f);
		b1 += MUL(getb32(scolor), sx2f);
	}

	sy1f = aa_SIZE - (_sy1 & aa_MASK);
	r1 = MUL(r1, sy1f);
	g1 = MUL(g1, sy1f);
	b1 = MUL(b1, sy1f);

	/* Middle lines.  */
	sy2i = _sy2 >> aa_BITS;
	if (++sy < sy2i) {
		r2 = g2 = b2 = 0;
		do {
			sx = sx1i;
			sline = (unsigned int *)(_src->line[sy]) + sx;

			scolor = *sline;
			r2 += MUL(getr32(scolor), sx1f);
			g2 += MUL(getg32(scolor), sx1f);
			b2 += MUL(getb32(scolor), sx1f);

			for (sline++, sx++; sx < sx2i; sline++, sx++) {
				scolor = *sline;
				r2 += getr32(scolor) << aa_BITS;
				g2 += getg32(scolor) << aa_BITS;
				b2 += getb32(scolor) << aa_BITS;
			}

			if (sx2f != 0) {
				scolor = *sline;
				r2 += MUL(getr32(scolor), sx2f);
				g2 += MUL(getg32(scolor), sx2f);
				b2 += MUL(getb32(scolor), sx2f);
			}
		} while (++sy < sy2i);

		r1 += r2 << aa_BITS;
		g1 += g2 << aa_BITS;
		b1 += b2 << aa_BITS;
	}

	/* Last line.  */
	sy2f = _sy2 & aa_MASK;
	if (sy2f != 0) {
		sx = sx1i;
		sline = (unsigned int *)(_src->line[sy]) + sx;

		scolor = *sline;
		r2 = MUL(getr32(scolor), sx1f);
		g2 = MUL(getg32(scolor), sx1f);
		b2 = MUL(getb32(scolor), sx1f);

		for (sline++, sx++; sx < sx2i; sline++, sx++) {
			scolor = *sline;
			r2 += getr32(scolor) << aa_BITS;
			g2 += getg32(scolor) << aa_BITS;
			b2 += getb32(scolor) << aa_BITS;
		}

		if (sx2f != 0) {
			scolor = *sline;
			r2 += MUL(getr32(scolor), sx2f);
			g2 += MUL(getg32(scolor), sx2f);
			b2 += MUL(getb32(scolor), sx2f);
		}

		r1 += MUL(r2, sy2f);
		g1 += MUL(g2, sy2f);
		b1 += MUL(b2, sy2f);
	}

	if (_num == (aa_SIZE * aa_SIZE)) {
		_aa.r = r1 >> (2 * aa_BITS);
		_aa.g = g1 >> (2 * aa_BITS);
		_aa.b = b1 >> (2 * aa_BITS);
	} else {
		_aa.r = r1 / _num;
		_aa.g = g1 / _num;
		_aa.b = b1 / _num;
	}
}
#endif

/*
 * Putting pixel to destination bitmap.
 */
void _aa_put_rgb8(byte *addr, int _x) {
	bmp_write8(addr + _x, makecol8(_aa.r, _aa.g, _aa.b));
}
#ifdef ALLEGRO_COLOR16
void _aa_put_rgb15(byte *addr, int _x) {
	bmp_write15(addr + sizeof(short) * _x, makecol15(_aa.r, _aa.g, _aa.b));
}

void _aa_put_rgb16(byte *addr, int _x) {
	bmp_write16(addr + sizeof(short) * _x, makecol16(_aa.r, _aa.g, _aa.b));
}
#endif
#ifdef ALLEGRO_COLOR24
void _aa_put_rgb24(byte *addr, int _x) {
	bmp_write24(addr + 3 * _x, makecol24(_aa.r, _aa.g, _aa.g));
}
#endif
#ifdef ALLEGRO_COLOR32
void _aa_put_rgb32(byte *addr, int _x) {
	bmp_write32(addr + sizeof(int) * _x, makecol32(_aa.r, _aa.g, _aa.b));
}
#endif

/*
 * Add masked r, g, b values of pixels.
 */
void _aa_masked_add_rgb8(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num) {
	unsigned char *sline;
	int sx, sx1i, sx1f, sx2i, sx2f;
	int sy, sy1i, sy1f, sy2i, sy2f;
	uint32_t r1, g1, b1;
	uint32_t r2, g2, b2, t2;
	uint32_t scolor;

	sy1i = _sy1 >> aa_BITS;
	sy = sy1i;

	/* First line.  */
	sx1i = _sx1 >> aa_BITS;
	sx = sx1i;
	sline = _src->line[sy] + sx;

	sx1f = aa_SIZE - (_sx1 & aa_MASK);
	scolor = *sline;
	if (scolor != MASK_COLOR_8) {
		r1 = MUL(getr8(scolor), sx1f);
		g1 = MUL(getg8(scolor), sx1f);
		b1 = MUL(getb8(scolor), sx1f);
		_G(t1) = 0;
	} else {
		r1 = g1 = b1 = 0;
		_G(t1) = sx1f;
	}

	sx2i = _sx2 >> aa_BITS;
	for (sline++, sx++; sx < sx2i; sline++, sx++) {
		scolor = *sline;
		if (scolor != MASK_COLOR_8) {
			r1 += getr8(scolor) << aa_BITS;
			g1 += getg8(scolor) << aa_BITS;
			b1 += getb8(scolor) << aa_BITS;
		} else
			_G(t1) += aa_SIZE;
	}

	sx2f = _sx2 & aa_MASK;
	if (sx2f != 0) {
		scolor = *sline;
		if (scolor != MASK_COLOR_8) {
			r1 += MUL(getr8(scolor), sx2f);
			g1 += MUL(getg8(scolor), sx2f);
			b1 += MUL(getb8(scolor), sx2f);
		} else
			_G(t1) += sx2f;
	}

	sy1f = aa_SIZE - (_sy1 & aa_MASK);
	r1 = MUL(r1, sy1f);
	g1 = MUL(g1, sy1f);
	b1 = MUL(b1, sy1f);
	_G(t1) = MUL(_G(t1), sy1f);

	/* Middle lines.  */
	sy2i = _sy2 >> aa_BITS;
	if (++sy < sy2i) {
		r2 = g2 = b2 = t2 = 0;
		do {
			sx = sx1i;
			sline = _src->line[sy] + sx;

			scolor = *sline;
			if (scolor != MASK_COLOR_8) {
				r2 += MUL(getr8(scolor), sx1f);
				g2 += MUL(getg8(scolor), sx1f);
				b2 += MUL(getb8(scolor), sx1f);
			} else
				t2 += sx1f;

			for (sline++, sx++; sx < sx2i; sline++, sx++) {
				scolor = *sline;
				if (scolor != MASK_COLOR_8) {
					r2 += getr8(scolor) << aa_BITS;
					g2 += getg8(scolor) << aa_BITS;
					b2 += getb8(scolor) << aa_BITS;
				} else
					t2 += aa_SIZE;
			}

			if (sx2f != 0) {
				scolor = *sline;
				if (scolor != MASK_COLOR_8) {
					r2 += MUL(getr8(scolor), sx2f);
					g2 += MUL(getg8(scolor), sx2f);
					b2 += MUL(getb8(scolor), sx2f);
				} else
					t2 += sx2f;
			}
		} while (++sy < sy2i);

		r1 += r2 << aa_BITS;
		g1 += g2 << aa_BITS;
		b1 += b2 << aa_BITS;
		_G(t1) += t2 << aa_BITS;
	}

	/* Last line.  */
	sy2f = _sy2 & aa_MASK;
	if (sy2f != 0) {
		sx = sx1i;
		sline = _src->line[sy] + sx;

		scolor = *sline;
		if (scolor != MASK_COLOR_8) {
			r2 = MUL(getr8(scolor), sx1f);
			g2 = MUL(getg8(scolor), sx1f);
			b2 = MUL(getb8(scolor), sx1f);
			t2 = 0;
		} else {
			r2 = g2 = b2 = 0;
			t2 = sx1f;
		}

		for (sline++, sx++; sx < sx2i; sline++, sx++) {
			scolor = *sline;
			if (scolor != MASK_COLOR_8) {
				r2 += getr8(scolor) << aa_BITS;
				g2 += getg8(scolor) << aa_BITS;
				b2 += getb8(scolor) << aa_BITS;
			} else
				t2 += aa_SIZE;
		}

		if (sx2f != 0) {
			scolor = *sline;
			if (scolor != MASK_COLOR_8) {
				r2 += MUL(getr8(scolor), sx2f);
				g2 += MUL(getg8(scolor), sx2f);
				b2 += MUL(getb8(scolor), sx2f);
			} else
				t2 += sx2f;
		}

		r1 += MUL(r2, sy2f);
		g1 += MUL(g2, sy2f);
		b1 += MUL(b2, sy2f);
		_G(t1) += MUL(t2, sy2f);
	}

	if (_num >= (2 * _G(t1))) {
		if (_num == (aa_SIZE * aa_SIZE)) {
			_aa.r = r1 >> (2 * aa_BITS);
			_aa.g = g1 >> (2 * aa_BITS);
			_aa.b = b1 >> (2 * aa_BITS);
		} else {
			_aa.r = r1 / _num;
			_aa.g = g1 / _num;
			_aa.b = b1 / _num;
		}
		_aa.transparent = 0;
	} else
		_aa.transparent = 1;
}

#ifdef ALLEGRO_COLOR16
void _aa_masked_add_rgb15(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num) {
	unsigned short *sline;
	int sx, sx1i, sx1f, sx2i, sx2f;
	int sy, sy1i, sy1f, sy2i, sy2f;
	unsigned int r1, g1, b1;
	unsigned int r2, g2, b2, t2;
	unsigned int scolor;

	sy1i = _sy1 >> aa_BITS;
	sy = sy1i;

	/* First line.  */
	sx1i = _sx1 >> aa_BITS;
	sx = sx1i;
	sline = (unsigned short *)(_src->line[sy]) + sx;

	sx1f = aa_SIZE - (_sx1 & aa_MASK);
	scolor = *sline;
	if (scolor != MASK_COLOR_15) {
		r1 = MUL(getr15(scolor), sx1f);
		g1 = MUL(getg15(scolor), sx1f);
		b1 = MUL(getb15(scolor), sx1f);
		_G(t1) = 0;
	} else {
		r1 = g1 = b1 = 0;
		_G(t1) = sx1f;
	}

	sx2i = _sx2 >> aa_BITS;
	for (sline++, sx++; sx < sx2i; sline++, sx++) {
		scolor = *sline;
		if (scolor != MASK_COLOR_15) {
			r1 += getr15(scolor) << aa_BITS;
			g1 += getg15(scolor) << aa_BITS;
			b1 += getb15(scolor) << aa_BITS;
		} else
			_G(t1) += aa_SIZE;
	}

	sx2f = _sx2 & aa_MASK;
	if (sx2f != 0) {
		scolor = *sline;
		if (scolor != MASK_COLOR_15) {
			r1 += MUL(getr15(scolor), sx2f);
			g1 += MUL(getg15(scolor), sx2f);
			b1 += MUL(getb15(scolor), sx2f);
		} else
			_G(t1) += sx2f;
	}

	sy1f = aa_SIZE - (_sy1 & aa_MASK);
	r1 = MUL(r1, sy1f);
	g1 = MUL(g1, sy1f);
	b1 = MUL(b1, sy1f);
	_G(t1) = MUL(_G(t1), sy1f);

	/* Middle lines.  */
	sy2i = _sy2 >> aa_BITS;
	if (++sy < sy2i) {
		r2 = g2 = b2 = t2 = 0;
		do {
			sx = sx1i;
			sline = (unsigned short *)(_src->line[sy]) + sx;

			scolor = *sline;
			if (scolor != MASK_COLOR_15) {
				r2 += MUL(getr15(scolor), sx1f);
				g2 += MUL(getg15(scolor), sx1f);
				b2 += MUL(getb15(scolor), sx1f);
			} else
				t2 += sx1f;

			for (sline++, sx++; sx < sx2i; sline++, sx++) {
				scolor = *sline;
				if (scolor != MASK_COLOR_15) {
					r2 += getr15(scolor) << aa_BITS;
					g2 += getg15(scolor) << aa_BITS;
					b2 += getb15(scolor) << aa_BITS;
				} else
					t2 += aa_SIZE;
			}

			if (sx2f != 0) {
				scolor = *sline;
				if (scolor != MASK_COLOR_15) {
					r2 += MUL(getr15(scolor), sx2f);
					g2 += MUL(getg15(scolor), sx2f);
					b2 += MUL(getb15(scolor), sx2f);
				} else
					t2 += sx2f;
			}
		} while (++sy < sy2i);

		r1 += r2 << aa_BITS;
		g1 += g2 << aa_BITS;
		b1 += b2 << aa_BITS;
		_G(t1) += t2 << aa_BITS;
	}

	/* Last line.  */
	sy2f = _sy2 & aa_MASK;
	if (sy2f != 0) {
		sx = sx1i;
		sline = (unsigned short *)(_src->line[sy]) + sx;

		scolor = *sline;
		if (scolor != MASK_COLOR_15) {
			r2 = MUL(getr15(scolor), sx1f);
			g2 = MUL(getg15(scolor), sx1f);
			b2 = MUL(getb15(scolor), sx1f);
			t2 = 0;
		} else {
			r2 = g2 = b2 = 0;
			t2 = sx1f;
		}

		for (sline++, sx++; sx < sx2i; sline++, sx++) {
			scolor = *sline;
			if (scolor != MASK_COLOR_15) {
				r2 += getr15(scolor) << aa_BITS;
				g2 += getg15(scolor) << aa_BITS;
				b2 += getb15(scolor) << aa_BITS;
			} else
				t2 += aa_SIZE;
		}

		if (sx2f != 0) {
			scolor = *sline;
			if (scolor != MASK_COLOR_15) {
				r2 += MUL(getr15(scolor), sx2f);
				g2 += MUL(getg15(scolor), sx2f);
				b2 += MUL(getb15(scolor), sx2f);
			} else
				t2 += sx2f;
		}

		r1 += MUL(r2, sy2f);
		g1 += MUL(g2, sy2f);
		b1 += MUL(b2, sy2f);
		_G(t1) += MUL(t2, sy2f);
	}

	if (_num >= (2 * _G(t1))) {
		if (_num == (aa_SIZE * aa_SIZE)) {
			_aa.r = r1 >> (2 * aa_BITS);
			_aa.g = g1 >> (2 * aa_BITS);
			_aa.b = b1 >> (2 * aa_BITS);
		} else {
			_aa.r = r1 / _num;
			_aa.g = g1 / _num;
			_aa.b = b1 / _num;
		}
		_aa.transparent = 0;
	} else
		_aa.transparent = 1;
}

void _aa_masked_add_rgb16(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num) {
	unsigned short *sline;
	int sx, sx1i, sx1f, sx2i, sx2f;
	int sy, sy1i, sy1f, sy2i, sy2f;
	unsigned int r1, g1, b1;
	unsigned int r2, g2, b2, t2;
	unsigned int scolor;

	sy1i = _sy1 >> aa_BITS;
	sy = sy1i;

	/* First line.  */
	sx1i = _sx1 >> aa_BITS;
	sx = sx1i;
	sline = (unsigned short *)(_src->line[sy]) + sx;

	sx1f = aa_SIZE - (_sx1 & aa_MASK);
	scolor = *sline;
	if (scolor != MASK_COLOR_16) {
		r1 = MUL(getr16(scolor), sx1f);
		g1 = MUL(getg16(scolor), sx1f);
		b1 = MUL(getb16(scolor), sx1f);
		_G(t1) = 0;
	} else {
		r1 = g1 = b1 = 0;
		_G(t1) = sx1f;
	}

	sx2i = _sx2 >> aa_BITS;
	for (sline++, sx++; sx < sx2i; sline++, sx++) {
		scolor = *sline;
		if (scolor != MASK_COLOR_16) {
			r1 += getr16(scolor) << aa_BITS;
			g1 += getg16(scolor) << aa_BITS;
			b1 += getb16(scolor) << aa_BITS;
		} else
			_G(t1) += aa_SIZE;
	}

	sx2f = _sx2 & aa_MASK;
	if (sx2f != 0) {
		scolor = *sline;
		if (scolor != MASK_COLOR_16) {
			r1 += MUL(getr16(scolor), sx2f);
			g1 += MUL(getg16(scolor), sx2f);
			b1 += MUL(getb16(scolor), sx2f);
		} else
			_G(t1) += sx2f;
	}

	sy1f = aa_SIZE - (_sy1 & aa_MASK);
	r1 = MUL(r1, sy1f);
	g1 = MUL(g1, sy1f);
	b1 = MUL(b1, sy1f);
	_G(t1) = MUL(_G(t1), sy1f);

	/* Middle lines.  */
	sy2i = _sy2 >> aa_BITS;
	if (++sy < sy2i) {
		r2 = g2 = b2 = t2 = 0;
		do {
			sx = sx1i;
			sline = (unsigned short *)(_src->line[sy]) + sx;

			scolor = *sline;
			if (scolor != MASK_COLOR_16) {
				r2 += MUL(getr16(scolor), sx1f);
				g2 += MUL(getg16(scolor), sx1f);
				b2 += MUL(getb16(scolor), sx1f);
			} else
				t2 += sx1f;

			for (sline++, sx++; sx < sx2i; sline++, sx++) {
				scolor = *sline;
				if (scolor != MASK_COLOR_16) {
					r2 += getr16(scolor) << aa_BITS;
					g2 += getg16(scolor) << aa_BITS;
					b2 += getb16(scolor) << aa_BITS;
				} else
					t2 += aa_SIZE;
			}

			if (sx2f != 0) {
				scolor = *sline;
				if (scolor != MASK_COLOR_16) {
					r2 += MUL(getr16(scolor), sx2f);
					g2 += MUL(getg16(scolor), sx2f);
					b2 += MUL(getb16(scolor), sx2f);
				} else
					t2 += sx2f;
			}
		} while (++sy < sy2i);

		r1 += r2 << aa_BITS;
		g1 += g2 << aa_BITS;
		b1 += b2 << aa_BITS;
		_G(t1) += t2 << aa_BITS;
	}

	/* Last line.  */
	sy2f = _sy2 & aa_MASK;
	if (sy2f != 0) {
		sx = sx1i;
		sline = (unsigned short *)(_src->line[sy]) + sx;

		scolor = *sline;
		if (scolor != MASK_COLOR_16) {
			r2 = MUL(getr16(scolor), sx1f);
			g2 = MUL(getg16(scolor), sx1f);
			b2 = MUL(getb16(scolor), sx1f);
			t2 = 0;
		} else {
			r2 = g2 = b2 = 0;
			t2 = sx1f;
		}

		for (sline++, sx++; sx < sx2i; sline++, sx++) {
			scolor = *sline;
			if (scolor != MASK_COLOR_16) {
				r2 += getr16(scolor) << aa_BITS;
				g2 += getg16(scolor) << aa_BITS;
				b2 += getb16(scolor) << aa_BITS;
			} else
				t2 += aa_SIZE;
		}

		if (sx2f != 0) {
			scolor = *sline;
			if (scolor != MASK_COLOR_16) {
				r2 += MUL(getr16(scolor), sx2f);
				g2 += MUL(getg16(scolor), sx2f);
				b2 += MUL(getb16(scolor), sx2f);
			} else
				t2 += sx2f;
		}

		r1 += MUL(r2, sy2f);
		g1 += MUL(g2, sy2f);
		b1 += MUL(b2, sy2f);
		_G(t1) += MUL(t2, sy2f);
	}

	if (_num >= (2 * _G(t1))) {
		if (_num == (aa_SIZE * aa_SIZE)) {
			_aa.r = r1 >> (2 * aa_BITS);
			_aa.g = g1 >> (2 * aa_BITS);
			_aa.b = b1 >> (2 * aa_BITS);
		} else {
			_aa.r = r1 / _num;
			_aa.g = g1 / _num;
			_aa.b = b1 / _num;
		}
		_aa.transparent = 0;
	} else
		_aa.transparent = 1;
}
#endif

#ifdef ALLEGRO_COLOR24
void _aa_masked_add_rgb24(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num) {
	unsigned char *sline;
	int sx, sx1i, sx1f, sx2i, sx2f;
	int sy, sy1i, sy1f, sy2i, sy2f;
	unsigned int r1, g1, b1;
	unsigned int r2, g2, b2, t2;
	unsigned int scolor;

	sy1i = _sy1 >> aa_BITS;
	sy = sy1i;

	/* First line.  */
	sx1i = _sx1 >> aa_BITS;
	sx = sx1i;
	sline = _src->line[sy] + sx * 3;

	sx1f = aa_SIZE - (_sx1 & aa_MASK);
#ifdef USE_24BIT_AS_CHARS
	scolor = ((unsigned int)(sline[0])
	          | ((unsigned int)(sline[1]) << 8)
	          | ((unsigned int)(sline[2]) << 16));
#else
	scolor = ((unsigned int)(((unsigned short *)sline)[0])
	          | ((unsigned int)(sline[2]) << 16));
#endif
	if (scolor != MASK_COLOR_24) {
		r1 = MUL(getr24(scolor), sx1f);
		g1 = MUL(getg24(scolor), sx1f);
		b1 = MUL(getb24(scolor), sx1f);
		_G(t1) = 0;
	} else {
		r1 = g1 = b1 = 0;
		_G(t1) = sx1f;
	}

	sx2i = _sx2 >> aa_BITS;
	for (sline += 3, sx++; sx < sx2i; sline += 3, sx++) {
#ifdef USE_24BIT_AS_CHARS
		scolor = ((unsigned int)(sline[0])
		          | ((unsigned int)(sline[1]) << 8)
		          | ((unsigned int)(sline[2]) << 16));
#else
		scolor = ((unsigned int)(((unsigned short *)sline)[0])
		          | ((unsigned int)(sline[2]) << 16));
#endif
		if (scolor != MASK_COLOR_24) {
			r1 += getr24(scolor) << aa_BITS;
			g1 += getg24(scolor) << aa_BITS;
			b1 += getb24(scolor) << aa_BITS;
		} else
			_G(t1) += aa_SIZE;
	}

	sx2f = _sx2 & aa_MASK;
	if (sx2f != 0) {
#ifdef USE_24BIT_AS_CHARS
		scolor = ((unsigned int)(sline[0])
		          | ((unsigned int)(sline[1]) << 8)
		          | ((unsigned int)(sline[2]) << 16));
#else
		scolor = ((unsigned int)(((unsigned short *)sline)[0])
		          | ((unsigned int)(sline[2]) << 16));
#endif
		if (scolor != MASK_COLOR_24) {
			r1 += MUL(getr24(scolor), sx2f);
			g1 += MUL(getg24(scolor), sx2f);
			b1 += MUL(getb24(scolor), sx2f);
		} else
			_G(t1) += sx2f;
	}

	sy1f = aa_SIZE - (_sy1 & aa_MASK);
	r1 = MUL(r1, sy1f);
	g1 = MUL(g1, sy1f);
	b1 = MUL(b1, sy1f);
	_G(t1) = MUL(_G(t1), sy1f);

	/* Middle lines.  */
	sy2i = _sy2 >> aa_BITS;
	if (++sy < sy2i) {
		r2 = g2 = b2 = t2 = 0;
		do {
			sx = sx1i;
			sline = _src->line[sy] + sx * 3;

#ifdef USE_24BIT_AS_CHARS
			scolor = ((unsigned int)(sline[0])
			          | ((unsigned int)(sline[1]) << 8)
			          | ((unsigned int)(sline[2]) << 16));
#else
			scolor = ((unsigned int)(((unsigned short *)sline)[0])
			          | ((unsigned int)(sline[2]) << 16));
#endif
			if (scolor != MASK_COLOR_24) {
				r2 += MUL(getr24(scolor), sx1f);
				g2 += MUL(getg24(scolor), sx1f);
				b2 += MUL(getb24(scolor), sx1f);
			} else
				t2 += sx1f;

			for (sline += 3, sx++; sx < sx2i; sline += 3, sx++) {
#ifdef USE_24BIT_AS_CHARS
				scolor = ((unsigned int)(sline[0])
				          | ((unsigned int)(sline[1]) << 8)
				          | ((unsigned int)(sline[2]) << 16));
#else
				scolor = ((unsigned int)(((unsigned short *)sline)[0])
				          | ((unsigned int)(sline[2]) << 16));
#endif
				if (scolor != MASK_COLOR_24) {
					r2 += getr24(scolor) << aa_BITS;
					g2 += getg24(scolor) << aa_BITS;
					b2 += getb24(scolor) << aa_BITS;
				} else
					t2 += aa_SIZE;
			}

			if (sx2f != 0) {
#ifdef USE_24BIT_AS_CHARS
				scolor = ((unsigned int)(sline[0])
				          | ((unsigned int)(sline[1]) << 8)
				          | ((unsigned int)(sline[2]) << 16));
#else
				scolor = ((unsigned int)(((unsigned short *)sline)[0])
				          | ((unsigned int)(sline[2]) << 16));
#endif
				if (scolor != MASK_COLOR_24) {
					r2 += MUL(getr24(scolor), sx2f);
					g2 += MUL(getg24(scolor), sx2f);
					b2 += MUL(getb24(scolor), sx2f);
				} else
					t2 += sx2f;
			}
		} while (++sy < sy2i);

		r1 += r2 << aa_BITS;
		g1 += g2 << aa_BITS;
		b1 += b2 << aa_BITS;
		_G(t1) += t2 << aa_BITS;
	}

	/* Last line.  */
	sy2f = _sy2 & aa_MASK;
	if (sy2f != 0) {
		sx = sx1i;
		sline = _src->line[sy] + sx * 3;

#ifdef USE_24BIT_AS_CHARS
		scolor = ((unsigned int)(sline[0])
		          | ((unsigned int)(sline[1]) << 8)
		          | ((unsigned int)(sline[2]) << 16));
#else
		scolor = ((unsigned int)(((unsigned short *)sline)[0])
		          | ((unsigned int)(sline[2]) << 16));
#endif
		if (scolor != MASK_COLOR_24) {
			r2 = MUL(getr24(scolor), sx1f);
			g2 = MUL(getg24(scolor), sx1f);
			b2 = MUL(getb24(scolor), sx1f);
			t2 = 0;
		} else {
			r2 = g2 = b2 = 0;
			t2 = sx1f;
		}

		for (sline += 3, sx++; sx < sx2i; sline += 3, sx++) {
#ifdef USE_24BIT_AS_CHARS
			scolor = ((unsigned int)(sline[0])
			          | ((unsigned int)(sline[1]) << 8)
			          | ((unsigned int)(sline[2]) << 16));
#else
			scolor = ((unsigned int)(((unsigned short *)sline)[0])
			          | ((unsigned int)(sline[2]) << 16));
#endif
			if (scolor != MASK_COLOR_24) {
				r2 += getr24(scolor) << aa_BITS;
				g2 += getg24(scolor) << aa_BITS;
				b2 += getb24(scolor) << aa_BITS;
			} else
				t2 += aa_SIZE;
		}

		if (sx2f != 0) {
#ifdef USE_24BIT_AS_CHARS
			scolor = ((unsigned int)(sline[0])
			          | ((unsigned int)(sline[1]) << 8)
			          | ((unsigned int)(sline[2]) << 16));
#else
			scolor = ((unsigned int)(((unsigned short *)sline)[0])
			          | ((unsigned int)(sline[2]) << 16));
#endif
			if (scolor != MASK_COLOR_24) {
				r2 += MUL(getr24(scolor), sx2f);
				g2 += MUL(getg24(scolor), sx2f);
				b2 += MUL(getb24(scolor), sx2f);
			} else
				t2 += sx2f;
		}

		r1 += MUL(r2, sy2f);
		g1 += MUL(g2, sy2f);
		b1 += MUL(b2, sy2f);
		_G(t1) += MUL(t2, sy2f);
	}

	if (_num >= (2 * _G(t1))) {
		if (_num == (aa_SIZE * aa_SIZE)) {
			_aa.r = r1 >> (2 * aa_BITS);
			_aa.g = g1 >> (2 * aa_BITS);
			_aa.b = b1 >> (2 * aa_BITS);
		} else {
			_aa.r = r1 / _num;
			_aa.g = g1 / _num;
			_aa.b = b1 / _num;
		}
		_aa.transparent = 0;
	} else
		_aa.transparent = 1;
}
#endif

#ifdef ALLEGRO_COLOR32
void _aa_masked_add_rgb32(BITMAP *_src, int _sx1, int _sx2, int _sy1, int _sy2, uint32_t _num) {
	unsigned int *sline;
	int sx, sx1i, sx1f, sx2i, sx2f;
	int sy, sy1i, sy1f, sy2i, sy2f;
	unsigned int r1, g1, b1;
	unsigned int r2, g2, b2, t2;
	unsigned int scolor;

	sy1i = _sy1 >> aa_BITS;
	sy = sy1i;

	/* First line.  */
	sx1i = _sx1 >> aa_BITS;
	sx = sx1i;
	sline = (unsigned int *)(_src->line[sy]) + sx;

	sx1f = aa_SIZE - (_sx1 & aa_MASK);
	scolor = *sline;
	if (scolor != MASK_COLOR_32) {
		r1 = MUL(getr32(scolor), sx1f);
		g1 = MUL(getg32(scolor), sx1f);
		b1 = MUL(getb32(scolor), sx1f);
		_G(t1) = 0;
	} else {
		r1 = g1 = b1 = 0;
		_G(t1) = sx1f;
	}

	sx2i = _sx2 >> aa_BITS;
	for (sline++, sx++; sx < sx2i; sline++, sx++) {
		scolor = *sline;
		if (scolor != MASK_COLOR_32) {
			r1 += getr32(scolor) << aa_BITS;
			g1 += getg32(scolor) << aa_BITS;
			b1 += getb32(scolor) << aa_BITS;
		} else
			_G(t1) += aa_SIZE;
	}

	sx2f = _sx2 & aa_MASK;
	if (sx2f != 0) {
		scolor = *sline;
		if (scolor != MASK_COLOR_32) {
			r1 += MUL(getr32(scolor), sx2f);
			g1 += MUL(getg32(scolor), sx2f);
			b1 += MUL(getb32(scolor), sx2f);
		} else
			_G(t1) += sx2f;
	}

	sy1f = aa_SIZE - (_sy1 & aa_MASK);
	r1 = MUL(r1, sy1f);
	g1 = MUL(g1, sy1f);
	b1 = MUL(b1, sy1f);
	_G(t1) = MUL(_G(t1), sy1f);

	/* Middle lines.  */
	sy2i = _sy2 >> aa_BITS;
	if (++sy < sy2i) {
		r2 = g2 = b2 = t2 = 0;
		do {
			sx = sx1i;
			sline = (unsigned int *)(_src->line[sy]) + sx;

			scolor = *sline;
			if (scolor != MASK_COLOR_32) {
				r2 += MUL(getr32(scolor), sx1f);
				g2 += MUL(getg32(scolor), sx1f);
				b2 += MUL(getb32(scolor), sx1f);
			} else
				t2 += sx1f;

			for (sline++, sx++; sx < sx2i; sline++, sx++) {
				scolor = *sline;
				if (scolor != MASK_COLOR_32) {
					r2 += getr32(scolor) << aa_BITS;
					g2 += getg32(scolor) << aa_BITS;
					b2 += getb32(scolor) << aa_BITS;
				} else
					t2 += aa_SIZE;
			}

			if (sx2f != 0) {
				scolor = *sline;
				if (scolor != MASK_COLOR_32) {
					r2 += MUL(getr32(scolor), sx2f);
					g2 += MUL(getg32(scolor), sx2f);
					b2 += MUL(getb32(scolor), sx2f);
				} else
					t2 += sx2f;
			}
		} while (++sy < sy2i);

		r1 += r2 << aa_BITS;
		g1 += g2 << aa_BITS;
		b1 += b2 << aa_BITS;
		_G(t1) += t2 << aa_BITS;
	}

	/* Last line.  */
	sy2f = _sy2 & aa_MASK;
	if (sy2f != 0) {
		sx = sx1i;
		sline = (unsigned int *)(_src->line[sy]) + sx;

		scolor = *sline;
		if (scolor != MASK_COLOR_32) {
			r2 = MUL(getr32(scolor), sx1f);
			g2 = MUL(getg32(scolor), sx1f);
			b2 = MUL(getb32(scolor), sx1f);
			t2 = 0;
		} else {
			r2 = g2 = b2 = 0;
			t2 = sx1f;
		}

		for (sline++, sx++; sx < sx2i; sline++, sx++) {
			scolor = *sline;
			if (scolor != MASK_COLOR_32) {
				r2 += getr32(scolor) << aa_BITS;
				g2 += getg32(scolor) << aa_BITS;
				b2 += getb32(scolor) << aa_BITS;
			} else
				t2 += aa_SIZE;
		}

		if (sx2f != 0) {
			scolor = *sline;
			if (scolor != MASK_COLOR_32) {
				r2 += MUL(getr32(scolor), sx2f);
				g2 += MUL(getg32(scolor), sx2f);
				b2 += MUL(getb32(scolor), sx2f);
			} else
				t2 += sx2f;
		}

		r1 += MUL(r2, sy2f);
		g1 += MUL(g2, sy2f);
		b1 += MUL(b2, sy2f);
		_G(t1) += MUL(t2, sy2f);
	}

	if (_num >= (2 * _G(t1))) {
		if (_num == (aa_SIZE * aa_SIZE)) {
			_aa.r = r1 >> (2 * aa_BITS);
			_aa.g = g1 >> (2 * aa_BITS);
			_aa.b = b1 >> (2 * aa_BITS);
		} else {
			_aa.r = r1 / _num;
			_aa.g = g1 / _num;
			_aa.b = b1 / _num;
		}
		_aa.transparent = 0;
	} else
		_aa.transparent = 1;
}
#endif

/*
 * Putting pixel to destination bitmap.
 */
void _aa_masked_put_rgb8(byte *addr, int _x) {
	if (!_aa.transparent)
		bmp_write8(addr + _x, makecol8(_aa.r, _aa.g, _aa.b));
}

#ifdef ALLEGRO_COLOR16
void _aa_masked_put_rgb15(byte *addr, int _x) {
	if (!_aa.transparent)
		bmp_write15(addr + sizeof(short) * _x, makecol15(_aa.r, _aa.g, _aa.b));
}

void _aa_masked_put_rgb16(byte *addr, int _x) {
	if (!_aa.transparent)
		bmp_write16(addr + sizeof(short) * _x, makecol16(_aa.r, _aa.g, _aa.b));
}
#endif

#ifdef ALLEGRO_COLOR24
void _aa_masked_put_rgb24(byte *addr, int _x) {
	if (!_aa.transparent)
		bmp_write24(addr + 3 * _x, makecol24(_aa.r, _aa.g, _aa.b));
}
#endif

#ifdef ALLEGRO_COLOR32
void _aa_masked_put_rgb32(byte *addr, int _x) {
	if (!_aa.transparent)
		bmp_write32(addr + sizeof(int) * _x, makecol32(_aa.r, _aa.g, _aa.b));
}
#endif

} // namespace AGS3
