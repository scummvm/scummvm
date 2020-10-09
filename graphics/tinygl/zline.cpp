/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * This file is based on, or a modified version of code from TinyGL (C) 1997-1998 Fabrice Bellard,
 * which is licensed under the zlib-license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#include "graphics/tinygl/zbuffer.h"

namespace TinyGL {

template <bool kDepthWrite>
FORCEINLINE void FrameBuffer::putPixel(unsigned int pixelOffset, int color, int x, int y, unsigned int z) {
	if (_enableScissor)
		putPixel<kDepthWrite, true>(pixelOffset, color, x, y, z);
	else
		putPixel<kDepthWrite, false>(pixelOffset, color, x, y, z);
}

template <bool kDepthWrite, bool kEnableScissor>
FORCEINLINE void FrameBuffer::putPixel(unsigned int pixelOffset, int color, int x, int y, unsigned int z) {
	if (kEnableScissor && scissorPixel(x, y)) {
		return;
	}
	unsigned int *pz = _zbuf + pixelOffset;
	if (compareDepth(z, *pz)) {
		writePixel<true, true, kDepthWrite>(pixelOffset, color, z);
	}
}

template <bool kEnableScissor>
FORCEINLINE void FrameBuffer::putPixel(unsigned int pixelOffset, int color, int x, int y) {
	if (kEnableScissor && scissorPixel(x, y)) {
		return;
	}
	writePixel<true, true>(pixelOffset, color);
}

template <bool kInterpRGB, bool kInterpZ, bool kDepthWrite>
FORCEINLINE void FrameBuffer::drawLine(const ZBufferPoint *p1, const ZBufferPoint *p2) {
	if (_enableScissor)
		drawLine<kInterpRGB, kInterpZ, kDepthWrite, true>(p1, p2);
	else
		drawLine<kInterpRGB, kInterpZ, kDepthWrite, false>(p1, p2);
}

template <bool kInterpRGB, bool kInterpZ, bool kDepthWrite, bool kEnableScissor>
void FrameBuffer::drawLine(const ZBufferPoint *p1, const ZBufferPoint *p2) {
	// Based on Bresenham's line algorithm, as implemented in
	// https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C
	// with a loop exit condition based on the (unidimensional) taxicab
	// distance between p1 and p2 (which is cheap to compute and
	// rounding-error-free) so that interpolations are possible without
	// code duplication.

	// Where we are in unidimensional framebuffer coordinate
	unsigned int pixelOffset = p1->y * xsize + p1->x;
	// and in 2d
	int x = p1->x;
	int y = p1->y;

	// How to move on each axis, in both coordinates systems
	const int dx = abs(p2->x - p1->x);
	const int inc_x = p1->x < p2->x ? 1 : -1;
	const int dy = abs(p2->y - p1->y);
	const int inc_y = p1->y < p2->y ? 1 : -1;
	const int inc_y_pixel = p1->y < p2->y ? xsize : -xsize;

	// When to move on each axis
	int err = (dx > dy ? dx : -dy) / 2;
	int e2;

	// How many moves
	int n = dx > dy ? dx : dy;

	// kInterpZ
	unsigned int z;
	int sz;

	// kInterpRGB
	int r = p1->r >> (ZB_POINT_RED_BITS - 8);
	int g = p1->g >> (ZB_POINT_GREEN_BITS - 8);
	int b = p1->b >> (ZB_POINT_BLUE_BITS - 8);
	int color = RGB_TO_PIXEL(r, g, b);
	int sr, sg, sb;

        if (kInterpZ) {
		sz = (p2->z - p1->z) / n;
		z = p1->z;
	}
	if (kInterpRGB) {
		sr = ((p2->r - p1->r) / n) >> (ZB_POINT_RED_BITS - 8);
		sg = ((p2->g - p1->g) / n) >> (ZB_POINT_GREEN_BITS - 8);
		sb = ((p2->b - p1->b) / n) >> (ZB_POINT_BLUE_BITS - 8);
	}
	while (n--) {
		if (kInterpZ)
			putPixel<kDepthWrite, kEnableScissor>(pixelOffset, color, x, y, z);
		else
			putPixel<kEnableScissor>(pixelOffset, color, x, y);
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			pixelOffset += inc_x;
			x += inc_x;
		}
		if (e2 < dy) {
			err += dx;
			pixelOffset += inc_y_pixel;
			y += inc_y;
		}
		if (kInterpZ)
			z += sz;
		if (kInterpRGB) {
			r += sr;
			g += sg;
			b += sb;
			color = RGB_TO_PIXEL(r, g, b);
		}
	}
}

void FrameBuffer::plot(ZBufferPoint *p) {
	const unsigned int pixelOffset = p->y * xsize + p->x;
	const int col = RGB_TO_PIXEL(p->r, p->g, p->b);
	const unsigned int z = p->z;
	if (_depthWrite && _depthTestEnabled)
		putPixel<true>(pixelOffset, col, p->x, p->y, z);
	else 
		putPixel<false>(pixelOffset, col, p->x, p->y, z);
}

void FrameBuffer::fillLineFlatZ(ZBufferPoint *p1, ZBufferPoint *p2) {
	if (_depthWrite && _depthTestEnabled)
		drawLine<false, true, true>(p1, p2);
	else
		drawLine<false, true, false>(p1, p2);
}

// line with color interpolation
void FrameBuffer::fillLineInterpZ(ZBufferPoint *p1, ZBufferPoint *p2) {
	if (_depthWrite && _depthTestEnabled)
		drawLine<true, true, true>(p1, p2);
	else
		drawLine<true, true, false>(p1, p2);
}

// no Z interpolation
void FrameBuffer::fillLineFlat(ZBufferPoint *p1, ZBufferPoint *p2) {
	if (_depthWrite && _depthTestEnabled)
		drawLine<false, false, true>(p1, p2);
	else
		drawLine<false, false, false>(p1, p2);
}

void FrameBuffer::fillLineInterp(ZBufferPoint *p1, ZBufferPoint *p2) {
	if (_depthWrite && _depthTestEnabled)
		drawLine<false, true, true>(p1, p2);
	else
		drawLine<false, true, false>(p1, p2);
}

void FrameBuffer::fillLineZ(ZBufferPoint *p1, ZBufferPoint *p2) {
	// choose if the line should have its color interpolated or not
	if (p1->r == p2->r && p1->g == p2->g && p1->b == p2->b)
		fillLineFlatZ(p1, p2);
	else
		fillLineInterpZ(p1, p2);
}

void FrameBuffer::fillLine(ZBufferPoint *p1, ZBufferPoint *p2) {
	// choose if the line should have its color interpolated or not
	if (p1->r == p2->r && p1->g == p2->g && p1->b == p2->b)
		fillLineFlat(p1, p2);
	else
		fillLineInterp(p1, p2);
}

} // end of namespace TinyGL
