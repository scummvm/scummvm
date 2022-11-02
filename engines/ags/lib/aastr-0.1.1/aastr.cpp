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
 * aastr.c --- anti-aliased stretching for Allegro
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

#include "ags/lib/aastr-0.1.1/aastr.h"
#include "ags/lib/aastr-0.1.1/aautil.h"

namespace AGS3 {

/*
 * Engine of anti-aliased stretching.
 */
static void _aa_stretch_blit(BITMAP *_src, BITMAP *_dst,
                             int _sx, int _sy, int _sw, int _sh,
                             int _dx, int _dy, int _dw, int _dh, int _masked) {
	int sx, sy, dx, dy, ydx, ysx;
	int xinc, yinc, dsx, dsy;
	int xi1, xi2, xdd, yxdd;
	int yi1, yi2, ydd;
	int dxbeg, dxend, dybeg, dyend;
	unsigned long num;
	void (*add)(BITMAP * _src, int _sx1, int _sx2, int _sy1, int _sy2, unsigned long _num);
	void (*put)(byte * _addr, int _x);

	if ((_dw <= 0) || (_dh <= 0) || (_sw <= 0) || (_sh <= 0))
		return;

	if (_dst->clip) {
		dybeg = ((_dy > _dst->ct) ? _dy : _dst->ct);
		dyend = (((_dy + _dh) < _dst->cb) ? (_dy + _dh) : _dst->cb);
		if (dybeg >= dyend)
			return;

		dxbeg = ((_dx > _dst->cl) ? _dx : _dst->cl);
		dxend = (((_dx + _dw) < _dst->cr) ? (_dx + _dw) : _dst->cr);
		if (dxbeg >= dxend)
			return;
	} else {
		dxbeg = _dx;
		dybeg = _dy;
		dxend = _dx + _dw;
		dyend = _dy + _dh;
	}

	_sx <<= aa_BITS;
	_sw <<= aa_BITS;
	dsx = _sw / _dw;

	if (dsx < (int)aa_SIZE) {
		/* Exploding by x.  */
		_dw--;
		_sw -= aa_SIZE;
		dsx = aa_SIZE;
	}

	_sy <<= aa_BITS;
	_sh <<= aa_BITS;
	dsy = _sh / _dh;

	if (dsy < (int)aa_SIZE) {
		/* Exploding by y.  */
		_dh--;
		_sh -= aa_SIZE;
		dsy = aa_SIZE;
	}

	num = dsx * dsy;

	if (num > aa_MAX_NUM) {
		if (dsx > (int)aa_MAX_SIZE)
			dsx = aa_MAX_SIZE;
		if (dsy > (int)aa_MAX_SIZE)
			dsy = aa_MAX_SIZE;
		num = dsx * dsy;
	}

	/* Walk in x direction up to dxbeg and save Bresenham state there.
	 * Later, it will be used to restart at any line.  */
	aa_PREPARE(xinc, yxdd, xi1, xi2, _sw, _dw);
	for (ydx = _dx, ysx = _sx; ydx < dxbeg; ydx++) {
		aa_ADVANCE(ysx, xinc, yxdd, xi1, xi2);
	}

	/* Color manipulation routines.  */
	if (is_screen_bitmap(_src))
		return;
	else {
		switch (bitmap_color_depth(_src)) {
		case 8:
			add = ((_masked != 0) ? _aa_masked_add_rgb8 : _aa_add_rgb8);
			break;
#ifdef ALLEGRO_COLOR16
		case 15:
			add = ((_masked != 0) ? _aa_masked_add_rgb15 : _aa_add_rgb15);
			break;
		case 16:
			add = ((_masked != 0) ? _aa_masked_add_rgb16 : _aa_add_rgb16);
			break;
#endif
#ifdef ALLEGRO_COLOR24
		case 24:
			add = ((_masked != 0) ? _aa_masked_add_rgb24 : _aa_add_rgb24);
			_aa_prepare_for_24bpp();
			break;
#endif
#ifdef ALLEGRO_COLOR32
		case 32:
			add = ((_masked != 0) ? _aa_masked_add_rgb32 : _aa_add_rgb32);
			break;
#endif
		default:
			return;
		}
	}

	if (is_planar_bitmap(_dst))
		return;
	else {
		switch (bitmap_color_depth(_dst)) {
		case 8:
			put = ((_masked != 0) ? _aa_masked_put_rgb8 : _aa_put_rgb8);
			break;
#ifdef ALLEGRO_COLOR16
		case 15:
			put = ((_masked != 0) ? _aa_masked_put_rgb15 : _aa_put_rgb15);
			break;
		case 16:
			put = ((_masked != 0) ? _aa_masked_put_rgb16 : _aa_put_rgb16);
			break;
#endif
#ifdef ALLEGRO_COLOR24
		case 24:
			put = ((_masked != 0) ? _aa_masked_put_rgb24 : _aa_put_rgb24);
			_aa_prepare_for_24bpp();
			break;
#endif
#ifdef ALLEGRO_COLOR32
		case 32:
			put = ((_masked != 0) ? _aa_masked_put_rgb32 : _aa_put_rgb32);
			break;
#endif
		default:
			return;
		}
	}

	/* Walk in y until we reach first non-clipped line.  */
	aa_PREPARE(yinc, ydd, yi1, yi2, _sh, _dh);
	for (dy = _dy, sy = _sy; dy < dybeg; dy++) {
		aa_ADVANCE(sy, yinc, ydd, yi1, yi2);
	}

	bmp_select(_dst);

	/* Stretch all non-clipped lines.  */
	for (; dy < dyend; dy++) {
		byte *daddr = bmp_write_line(_dst, dy);

		for (dx = ydx, sx = ysx, xdd = yxdd; dx < dxend; dx++) {
			(*add)(_src, sx, sx + dsx, sy, sy + dsy, num);
			(*put)(daddr, dx);

			aa_ADVANCE(sx, xinc, xdd, xi1, xi2);
		}

		aa_ADVANCE(sy, yinc, ydd, yi1, yi2);
	}

	bmp_unwrite_line(_dst);
}

/*
 * Anti-aliased bitmap stretching with blit.
 */
void aa_stretch_blit(BITMAP *_src, BITMAP *_dst,
                     int _sx, int _sy, int _sw, int _sh,
                     int _dx, int _dy, int _dw, int _dh) {
	_aa_stretch_blit(_src, _dst, _sx, _sy, _sw, _sh, _dx, _dy, _dw, _dh, 0);
}

/*
 * Anti-aliased bitmap stretching with blit (masked).
 */
void aa_stretch_sprite(BITMAP *_dst, BITMAP *_src, int _dx, int _dy, int _dw, int _dh) {
	_aa_stretch_blit(_src, _dst, 0, 0, _src->w, _src->h, _dx, _dy, _dw, _dh, 1);
}

} // namespace AGS3
