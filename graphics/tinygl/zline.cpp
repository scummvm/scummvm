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

template <bool kInterpRGB, bool kInterpZ, bool kDepthWrite>
FORCEINLINE static void putPixel(FrameBuffer *buffer, int pixelOffset,
                                 const Graphics::PixelFormat &cmode, unsigned int *pz, unsigned int &z, int &color, unsigned int &r,
                                 unsigned int &g, unsigned int &b) {
	if (buffer->scissorPixel(pixelOffset)) {
		return;
	}
	if (kInterpZ) {
		if (buffer->compareDepth(z, *pz)) {
			if (kInterpRGB) {
				buffer->writePixel(pixelOffset, RGB_TO_PIXEL(r, g, b));
			} else {
				buffer->writePixel(pixelOffset, color);
			}
			if (kDepthWrite) {
				*pz = z;
			}
		}
	} else {
		if (kInterpRGB) {
			buffer->writePixel(pixelOffset, RGB_TO_PIXEL(r, g, b));
		} else {
			buffer->writePixel(pixelOffset, color);
		}
	}
}

template <bool kInterpRGB, bool kInterpZ, bool kDepthWrite>
FORCEINLINE static void drawLine(FrameBuffer *buffer, ZBufferPoint *p1, ZBufferPoint *p2,
                                 int &pixelOffset, const Graphics::PixelFormat &cmode, unsigned int *pz, unsigned int &z, int &color,
                                 unsigned int &r, unsigned int &g, unsigned int &b, int dx, int dy, int inc_1, int inc_2) {
	int n = dx;
	int rinc, ginc, binc;
	int zinc;
	if (kInterpZ) {
		zinc = (p2->z - p1->z) / n;
	}
	if (kInterpRGB) {
		rinc = ((p2->r - p1->r) << 8) / n;
		ginc = ((p2->g - p1->g) << 8) / n;
		binc = ((p2->b - p1->b) << 8) / n;
	}
	int a = 2 * dy - dx;
	dy = 2 * dy;
	dx = 2 * dx - dy;
	int pp_inc_1 = (inc_1);
	int pp_inc_2 = (inc_2);
	do {
		putPixel<kInterpRGB, kInterpZ, kDepthWrite>(buffer, pixelOffset, cmode, pz, z, color, r, g, b);
		if (kInterpZ) {
			z += zinc;
		}
		if (kInterpRGB) {
			r += rinc;
			g += ginc;
			b += binc;
		}
		if (a > 0) {
			pixelOffset += pp_inc_1;
			if (kInterpZ) {
				pz += inc_1;
			}
			a -= dx;
		} else {
			pixelOffset += pp_inc_2;
			if (kInterpZ) {
				pz += inc_2;
			}
			a += dy;
		}
	} while (--n >= 0);
}

template <bool kInterpRGB, bool kInterpZ, bool kDepthWrite>
void FrameBuffer::fillLineGeneric(ZBufferPoint *p1, ZBufferPoint *p2, int color) {
	int dx, dy, sx;
	unsigned int r, g, b;
	unsigned int *pz = NULL;
	unsigned int z;
	int pixelOffset;

	if (p1->y > p2->y || (p1->y == p2->y && p1->x > p2->x)) {
		ZBufferPoint *tmp;
		tmp = p1;
		p1 = p2;
		p2 = tmp;
	}
	sx = xsize;
	pixelOffset = xsize * p1->y + p1->x;
	if (kInterpZ) {
		pz = _zbuf + (p1->y * sx + p1->x);
		z = p1->z;
	}
	dx = p2->x - p1->x;
	dy = p2->y - p1->y;
	if (kInterpRGB) {
		r = p2->r << 8;
		g = p2->g << 8;
		b = p2->b << 8;
	}

	if (dx == 0 && dy == 0) {
		putPixel<kInterpRGB, kInterpZ, kDepthWrite>(this, pixelOffset, cmode, pz, z, color, r, g, b);
	} else if (dx > 0) {
		if (dx >= dy) {
			drawLine<kInterpRGB, kInterpZ, kDepthWrite>(this, p1, p2, pixelOffset, cmode, pz, z, color, r, g, b, dx, dy, sx + 1, 1);
		} else {
			drawLine<kInterpRGB, kInterpZ, kDepthWrite>(this, p1, p2, pixelOffset, cmode, pz, z, color, r, g, b, dx, dy, sx + 1, sx);
		}
	} else {
		dx = -dx;
		if (dx >= dy) {
			drawLine<kInterpRGB, kInterpZ, kDepthWrite>(this, p1, p2, pixelOffset, cmode, pz, z, color, r, g, b, dx, dy, sx - 1, -1);
		} else {
			drawLine<kInterpRGB, kInterpZ, kDepthWrite>(this, p1, p2, pixelOffset, cmode, pz, z, color, r, g, b, dx, dy, sx - 1, sx);
		}
	}
}

void FrameBuffer::plot(ZBufferPoint *p) {
	unsigned int *pz;
	unsigned int r, g, b;

	pz = _zbuf + (p->y * xsize + p->x);
	int col = RGB_TO_PIXEL(p->r, p->g, p->b);
	unsigned int z = p->z;
	if (_depthWrite && _depthTestEnabled)
		putPixel<false, true, true>(this, linesize * p->y + p->x * PSZB, cmode, pz, z, col, r, g, b);
	else 
		putPixel<false, true, false>(this, linesize * p->y + p->x * PSZB, cmode, pz, z, col, r, g, b);
}

void FrameBuffer::fillLineFlatZ(ZBufferPoint *p1, ZBufferPoint *p2, int color) {
	if (_depthWrite && _depthTestEnabled)
		fillLineGeneric<false, true, true>(p1, p2, color);
	else
		fillLineGeneric<false, true, false>(p1, p2, color);
}

// line with color interpolation
void FrameBuffer::fillLineInterpZ(ZBufferPoint *p1, ZBufferPoint *p2) {
	if (_depthWrite && _depthTestEnabled)
		fillLineGeneric<true, true, true>(p1, p2, 0);
	else
		fillLineGeneric<true, true, false>(p1, p2, 0);
}

// no Z interpolation
void FrameBuffer::fillLineFlat(ZBufferPoint *p1, ZBufferPoint *p2, int color) {
	if (_depthWrite && _depthTestEnabled)
		fillLineGeneric<false, false, true>(p1, p2, color);
	else
		fillLineGeneric<false, false, false>(p1, p2, color);
}

void FrameBuffer::fillLineInterp(ZBufferPoint *p1, ZBufferPoint *p2) {
	if (_depthWrite && _depthTestEnabled)
		fillLineGeneric<false, true, true>(p1, p2, 0);
	else
		fillLineGeneric<false, true, false>(p1, p2, 0);
}

void FrameBuffer::fillLineZ(ZBufferPoint *p1, ZBufferPoint *p2) {
	int color1, color2;

	color1 = RGB_TO_PIXEL(p1->r, p1->g, p1->b);
	color2 = RGB_TO_PIXEL(p2->r, p2->g, p2->b);

	// choose if the line should have its color interpolated or not
	if (color1 == color2) {
		fillLineFlatZ(p1, p2, color1);
	} else {
		fillLineInterpZ(p1, p2);
	}
}

void FrameBuffer::fillLine(ZBufferPoint *p1, ZBufferPoint *p2) {
	int color1, color2;

	color1 = RGB_TO_PIXEL(p1->r, p1->g, p1->b);
	color2 = RGB_TO_PIXEL(p2->r, p2->g, p2->b);

	// choose if the line should have its color interpolated or not
	if (color1 == color2) {
		fillLineFlat(p1, p2, color1);
	} else {
		fillLineInterp(p1, p2);
	}
}

} // end of namespace TinyGL
