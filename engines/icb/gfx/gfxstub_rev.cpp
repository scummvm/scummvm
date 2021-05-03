/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "common/scummsys.h"

#include "engines/icb/gfx/gfxstub.h"
#include "engines/icb/gfx/gfxstub_dutch.h"
#include "engines/icb/gfx/gfxstub_rev_dutch.h"
#include "engines/icb/p4_generic_pc.h"

namespace ICB {

#define MAKERGB(r, g, b, a) ((uint32)(((uint8)(b) | ((uint16)((uint8)(g)) << 8)) | (((uint32)(uint8)(r)) << 16) | (((uint32)(uint8)(a)) << 24)))

#define TEMP_TEXTURE_WIDTH 256
#define TEMP_TEXTURE_HEIGHT 256
#define BYTES_PER_COLOUR 4

uint8 pcRGBA[TEMP_TEXTURE_WIDTH * TEMP_TEXTURE_HEIGHT * BYTES_PER_COLOUR];

int32 bpp = 0;

#define RGBBytesPerPixel    4                                 // 32 bit
#define RGBWidth            SCREEN_WIDTH                      // width
#define RGBHeight           SCREEN_DEPTH                      // height
#define RGBPitch            (RGBWidth * RGBBytesPerPixel)     // pitch
#define ZPitch              (ZBytesPerPixel * SCREEN_WIDTH)   // z-pitch
#define ZBytesPerPixel      2                                 // 16bit z-buffer

// The big screen bitmap to draw everything into
extern char *pRGB;

// The z buffer bitmap
extern char *pZ;

#define MAX_POLYGON_VERTS 8
vertex2D verts[8];
vertex2D clipverts[8];

int32 SimpleReject(vertex2D *vertices) {
	int64 l0x = (vertices[0].x - vertices[1].x);
	int64 l0y = (vertices[0].y - vertices[1].y);
	int64 l1x = (vertices[2].x - vertices[1].x);
	int64 l1y = (vertices[2].y - vertices[1].y);

	int64 prod1 = (l0x * l1y) >> 32;
	int64 prod2 = (l1x * l0y) >> 32;

	if (prod1 >= prod2) {
		return 1;
	}

	return 0;
}

int32 ClipPolygon(vertex2D *inverts, int32 inNverts, vertex2D *outverts, int32 *outNverts);

void startDrawing() {}

void endDrawing() {
	ClearProcessorState();
}

// Real graphic routines
// r0, g0, b0 :  128 means colour scale of 1.0
int32 DrawSprite(int32 x0, int32 y0, short w, short h, uint8 r0, uint8 g0, uint8 b0, uint16 /*u0*/, uint16 v0, uint8 alpha, uint16 z, void *tex) {
	int32 x, y;
	int32 lx0, ly0;
	TextureHandle *pthan = NULL;
	int32 lx1, ly1;

	if (tex == NULL) {
		return 0;
	} else {
		pthan = (TextureHandle *)tex;
	}

	if ((SetTextureState(pthan)) != 0) {
		return 0;
	}

	lx0 = RGBWidth / 2 + x0;
	ly0 = RGBHeight / 2 + y0;
	lx1 = lx0 + w;
	ly1 = ly0 + h;

	if (lx0 < 0)
		lx0 = 0;
	if (ly0 < 0)
		ly0 = 0;
	if (lx1 >= RGBWidth)
		lx1 = RGBWidth - 1;
	if (ly1 >= RGBHeight)
		ly1 = RGBHeight - 1;

	if (lx0 >= RGBWidth)
		return 1;
	if (ly0 >= RGBHeight)
		return 1;

	char *line = pRGB + (RGBPitch * ly0);
	int32 xStart = RGBBytesPerPixel * lx0;
	char *zline = pZ + (ZPitch * ly0);
	int32 xzStart = ZBytesPerPixel * lx0;

	uint8 *texel = (uint8 *)pcRGBA + (v0 * TEMP_TEXTURE_WIDTH) * BYTES_PER_COLOUR;

	for (y = ly0; y < ly1; y++) {
		char *row = line + xStart;
		char *zrow = zline + xzStart;
		for (x = lx0; x < lx1; x++) {
			int32 r = r0;
			int32 g = g0;
			int32 b = b0;
			int32 tr = *(texel + 2);
			int32 tg = *(texel + 1);
			int32 tb = *(texel + 0);

			r = (r * tr);
			g = (g * tg);
			b = (b * tb);

			if (r < 0)
				r = 0;
			if (g < 0)
				g = 0;
			if (b < 0)
				b = 0;

			r = r >> 7;
			g = g >> 7;
			b = b >> 7;

			if (r > 255)
				r = 255;
			if (g > 255)
				g = 255;
			if (b > 255)
				b = 255;

			*(row + 0) = (char)b;
			*(row + 1) = (char)g;
			*(row + 2) = (char)r;
			*(row + 3) = alpha;
			*(uint16 *)(zrow + 0) = z;
			row += RGBBytesPerPixel;
			zrow += ZBytesPerPixel;
		}
		line += RGBPitch;
		texel += TEMP_TEXTURE_WIDTH * BYTES_PER_COLOUR;
	}
	return 1;
}

// Single coloured rectangle
int32 DrawTile(int32 x0, int32 y0, short w, short h, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, uint16 z) {
	int32 x, y;
	int32 lx0, ly0;
	int32 lx1, ly1;

	lx0 = RGBWidth / 2 + x0;
	ly0 = RGBHeight / 2 + y0;
	lx1 = lx0 + w;
	ly1 = ly0 + h;

	if (lx0 < 0)
		lx0 = 0;
	if (ly0 < 0)
		ly0 = 0;
	if (lx1 >= RGBWidth)
		lx1 = RGBWidth - 1;
	if (ly1 >= RGBHeight)
		ly1 = RGBHeight - 1;

	if (lx0 >= RGBWidth)
		return 1;
	if (ly0 >= RGBHeight)
		return 1;

	char *line = pRGB + (RGBPitch * ly0);
	int32 xStart = RGBBytesPerPixel * lx0;
	char *zline = pZ + (ZPitch * ly0);
	int32 xzStart = ZBytesPerPixel * lx0;

	for (y = ly0; y < ly1; y++) {
		char *row = line + xStart;
		char *zrow = zline + xzStart;
		for (x = lx0; x < lx1; x++) {
			*(row + 0) = b0;
			*(row + 1) = g0;
			*(row + 2) = r0;
			*(row + 3) = alpha;
			*(uint16 *)(zrow + 0) = z;
			row += RGBBytesPerPixel;
			zrow += ZBytesPerPixel;
		}
		line += RGBPitch;
	}
	return 1;
}

// Single flat coloured line : 2 points, 1 colour
int32 DrawLineF2(int32 x0, int32 y0, int32 x1, int32 y1, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, uint16 z) {
	// Uses Bressnham's incremental algorithm!
	// we pass a colour

	int32 dx, dy;
	int32 dxmod, dymod;
	int32 ince, incne;
	int32 d;
	int32 x, y;
	int32 addTo;

	x0 = (int16)(RGBWidth / 2 + x0);
	x1 = (int16)(RGBWidth / 2 + x1);
	y0 = (int16)(RGBHeight / 2 + y0);
	y1 = (int16)(RGBHeight / 2 + y1);

	// Make sure we're going from left to right
	if (x1 < x0) {
		x = x1;
		x1 = x0;
		x0 = (int16)x;
		y = y1;
		y1 = y0;
		y0 = (int16)y;
	}
	dx = x1 - x0;
	dy = y1 - y0;

	if (dx < 0)
		dxmod = -dx;
	else
		dxmod = dx;

	if (dy < 0)
		dymod = -dy;
	else
		dymod = dy;

	if (dxmod >= dymod) {
		if (dy > 0) {
			d = 2 * dy - dx;
			ince = 2 * dy;
			incne = 2 * (dy - dx);
			x = x0;
			y = y0;
			if ((x >= 0) && (x < RGBWidth) && (y >= 0) && (y < RGBHeight)) {
				int32 index = y * RGBPitch + (x * RGBBytesPerPixel);
				pRGB[index + 0] = b0;
				pRGB[index + 1] = g0;
				pRGB[index + 2] = r0;
				pRGB[index + 3] = alpha;
				index = y * ZPitch + (x * ZBytesPerPixel);
				*(uint16 *)(pZ + index + 0) = z;
			}

			while (x < x1) {
				if (d <= 0) {
					d += ince;
					x += 1;
				} else {
					d += incne;
					x += 1;
					y += 1;
				}
				if ((x >= 0) && (x < RGBWidth) && (y >= 0) && (y < RGBHeight)) {
					int32 index = y * RGBPitch + (x * RGBBytesPerPixel);
					pRGB[index + 0] = b0;
					pRGB[index + 1] = g0;
					pRGB[index + 2] = r0;
					pRGB[index + 3] = alpha;
					index = y * ZPitch + (x * ZBytesPerPixel);
					*(uint16 *)(pZ + index + 0) = z;
				}
			}
		} else {
			addTo = y0;
			y0 = 0;
			y1 = (int16)(y1 - addTo);
			y1 = (int16)-y1;
			dy = y1 - y0;

			d = 2 * dy - dx;
			ince = 2 * dy;
			incne = 2 * (dy - dx);
			x = x0;
			y = y0;
			if ((x >= 0) && (x < RGBWidth) && (addTo - y >= 0) && (addTo - y < RGBHeight)) {
				int32 index = (addTo - y) * RGBPitch + (x * RGBBytesPerPixel);
				pRGB[index + 0] = b0;
				pRGB[index + 1] = g0;
				pRGB[index + 2] = r0;
				pRGB[index + 3] = alpha;
				index = (addTo - y) * ZPitch + (x * ZBytesPerPixel);
				*(uint16 *)(pZ + index + 0) = z;
			}

			while (x < x1) {
				if (d <= 0) {
					d += ince;
					x += 1;
				} else {
					d += incne;
					x += 1;
					y += 1;
				}
				if ((x >= 0) && (x < RGBWidth) && (addTo - y >= 0) && (addTo - y < RGBHeight)) {
					int32 index = (addTo - y) * RGBPitch + (x * RGBBytesPerPixel);
					pRGB[index + 0] = b0;
					pRGB[index + 1] = g0;
					pRGB[index + 2] = r0;
					pRGB[index + 3] = alpha;
					index = (addTo - y) * ZPitch + (x * ZBytesPerPixel);
					*(uint16 *)(pZ + index + 0) = z;
				}
			}
		}
	} else {
		// OK, y is now going to be the single increment.
		//  Ensure the line is going top to bottom
		if (y1 < y0) {
			x = x1;
			x1 = x0;
			x0 = (int16)x;
			y = y1;
			y1 = y0;
			y0 = (int16)y;
		}
		dx = x1 - x0;
		dy = y1 - y0;

		if (dx > 0) {
			d = 2 * dx - dy;
			ince = 2 * dx;
			incne = 2 * (dx - dy);
			x = x0;
			y = y0;
			if ((x >= 0) && (x < RGBWidth) && (y >= 0) && (y < RGBHeight)) {
				int32 index = y * RGBPitch + (x * RGBBytesPerPixel);
				pRGB[index + 0] = b0;
				pRGB[index + 1] = g0;
				pRGB[index + 2] = r0;
				pRGB[index + 3] = alpha;
				index = y * ZPitch + (x * ZBytesPerPixel);
				*(uint16 *)(pZ + index + 0) = z;
			}
			while (y < y1) {
				if (d <= 0) {
					d += ince;
					y += 1;
				} else {
					d += incne;
					x += 1;
					y += 1;
				}
				if ((x >= 0) && (x < RGBWidth) && (y >= 0) && (y < RGBHeight)) {
					int32 index = y * RGBPitch + (x * RGBBytesPerPixel);
					pRGB[index + 0] = b0;
					pRGB[index + 1] = g0;
					pRGB[index + 2] = r0;
					pRGB[index + 3] = alpha;
					index = y * ZPitch + (x * ZBytesPerPixel);
					*(uint16 *)(pZ + index + 0) = z;
				}
			}
		} else {
			addTo = x0;
			x0 = 0;
			x1 = (int16)(x1 - addTo);
			x1 = (int16)-x1;
			dx = x1 - x0;

			d = 2 * dx - dy;
			ince = 2 * dx;
			incne = 2 * (dx - dy);
			x = x0;
			y = y0;
			if ((addTo - x >= 0) && (addTo - x < RGBWidth) && (y >= 0) && (y < RGBHeight)) {
				int32 index = y * RGBPitch + ((addTo - x) * RGBBytesPerPixel);
				pRGB[index + 0] = b0;
				pRGB[index + 1] = g0;
				pRGB[index + 2] = r0;
				pRGB[index + 3] = alpha;
				index = y * ZPitch + ((addTo - x) * ZBytesPerPixel);
				*(uint16 *)(pZ + index + 0) = z;
			}

			while (y < y1) {
				if (d <= 0) {
					d += ince;
					y += 1;
				} else {
					d += incne;
					x += 1;
					y += 1;
				}
				if ((addTo - x >= 0) && (addTo - x < RGBWidth) && (y >= 0) && (y < RGBHeight)) {
					int32 index = y * RGBPitch + ((addTo - x) * RGBBytesPerPixel);
					pRGB[index + 0] = b0;
					pRGB[index + 1] = g0;
					pRGB[index + 2] = r0;
					pRGB[index + 3] = alpha;
					index = y * ZPitch + ((addTo - x) * ZBytesPerPixel);
					*(uint16 *)(pZ + index + 0) = z;
				}
			}
		}
	}
	return 1;
}

// two connected lines flat coloured : 3 points, 1 colour
int32 DrawLineF3(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, uint16 z) {
	DrawLineF2(x0, y0, x1, y1, r0, g0, b0, alpha, z);
	DrawLineF2(x1, y1, x2, y2, r0, g0, b0, alpha, z);
	return 0;
}

// three connected lines flat coloured : 4 points, 1 colour
int32 DrawLineF4(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, uint16 z) {
	DrawLineF2(x0, y0, x1, y1, r0, g0, b0, alpha, z);
	DrawLineF2(x1, y1, x2, y2, r0, g0, b0, alpha, z);
	DrawLineF2(x2, y2, x3, y3, r0, g0, b0, alpha, z);
	return 0;
}

// Single gouraud coloured line : 2 points, 2 colours
int32 DrawLineG2(int32 x0, int32 y0, int32 x1, int32 y1, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 alpha, uint16 z) {
	// Uses Bressnham's incremental algorithm!
	// we pass a colour

	int32 dx, dy;
	int32 dxmod, dymod;
	int32 ince, incne;
	int32 d;
	int32 x, y;
	int32 addTo;
	int32 dred;
	int32 dgreen;
	int32 dblue;
	int32 r, g, b;
	int32 lr0, lg0, lb0;
	int32 lr1, lg1, lb1;

	x0 = (int16)(RGBWidth / 2 + x0);
	x1 = (int16)(RGBWidth / 2 + x1);
	y0 = (int16)(RGBHeight / 2 + y0);
	y1 = (int16)(RGBHeight / 2 + y1);

	// Make sure we're going from left to right
	if (x1 < x0) {
		x = x1;
		x1 = x0;
		x0 = (int16)x;
		y = y1;
		y1 = y0;
		y0 = (int16)y;

		lr0 = r1 << 8;
		lr1 = r0 << 8;

		lg0 = g1 << 8;
		lg1 = g0 << 8;

		lb0 = b1 << 8;
		lb1 = b0 << 8;
	} else {
		lr0 = r0 << 8;
		lg0 = g0 << 8;
		lb0 = b0 << 8;

		lr1 = r1 << 8;
		lg1 = g1 << 8;
		lb1 = b1 << 8;
	}
	dx = x1 - x0;
	dy = y1 - y0;

	dred = lr1 - lr0;
	dgreen = lg1 - lg0;
	dblue = lb1 - lb0;

	if (dx < 0)
		dxmod = -dx;
	else
		dxmod = dx;

	if (dy < 0)
		dymod = -dy;
	else
		dymod = dy;

	if ((dxmod == 0) && (dymod == 0))
		return 1;

	if (dxmod >= dymod) {
		dred /= dxmod;
		dgreen /= dxmod;
		dblue /= dxmod;
		if (dy > 0) {
			d = 2 * dy - dx;
			ince = 2 * dy;
			incne = 2 * (dy - dx);
			x = x0;
			y = y0;
			r = lr0;
			g = lg0;
			b = lb0;
			if ((x >= 0) && (x < RGBWidth) && (y >= 0) && (y < RGBHeight)) {
				int32 index = y * RGBPitch + (x * RGBBytesPerPixel);
				pRGB[index + 0] = b0;
				pRGB[index + 1] = g0;
				pRGB[index + 2] = r0;
				pRGB[index + 3] = alpha;
				index = y * ZPitch + (x * ZBytesPerPixel);
				*(uint16 *)(pZ + index + 0) = z;
			}

			while (x < x1) {
				if (d <= 0) {
					d += ince;
					x += 1;
				} else {
					d += incne;
					x += 1;
					y += 1;
				}
				r += dred;
				g += dgreen;
				b += dblue;
				if ((x >= 0) && (x < RGBWidth) && (y >= 0) && (y < RGBHeight)) {
					int32 index = y * RGBPitch + (x * RGBBytesPerPixel);
					pRGB[index + 0] = (uint8)(b >> 8);
					pRGB[index + 1] = (uint8)(g >> 8);
					pRGB[index + 2] = (uint8)(r >> 8);
					index = y * ZPitch + (x * ZBytesPerPixel);
					*(uint16 *)(pZ + index + 0) = z;
				}
			}
		} else {
			addTo = y0;
			y0 = 0;
			y1 = (int16)(y1 - addTo);
			y1 = (int16)-y1;
			dy = y1 - y0;

			d = 2 * dy - dx;
			ince = 2 * dy;
			incne = 2 * (dy - dx);
			x = x0;
			y = y0;
			r = lr0;
			g = lg0;
			b = lb0;
			if ((x >= 0) && (x < RGBWidth) && (addTo - y >= 0) && (addTo - y < RGBHeight)) {
				int32 index = (addTo - y) * RGBPitch + (x * RGBBytesPerPixel);
				pRGB[index + 0] = b0;
				pRGB[index + 1] = g0;
				pRGB[index + 2] = r0;
				pRGB[index + 3] = alpha;
				index = (addTo - y) * ZPitch + (x * ZBytesPerPixel);
				*(uint16 *)(pZ + index + 0) = z;
			}

			while (x < x1) {
				if (d <= 0) {
					d += ince;
					x += 1;
				} else {
					d += incne;
					x += 1;
					y += 1;
				}
				r += dred;
				g += dgreen;
				b += dblue;
				if ((x >= 0) && (x < RGBWidth) && (addTo - y >= 0) && (addTo - y < RGBHeight)) {
					int32 index = (addTo - y) * RGBPitch + (x * RGBBytesPerPixel);
					pRGB[index + 0] = (uint8)(b >> 8);
					pRGB[index + 1] = (uint8)(g >> 8);
					pRGB[index + 2] = (uint8)(r >> 8);
					index = (addTo - y) * ZPitch + (x * ZBytesPerPixel);
					*(uint16 *)(pZ + index + 0) = z;
				}
			}
		}
	} else {
		// OK, y is now going to be the single increment.
		//  Ensure the line is going top to bottom
		if (y1 < y0) {
			x = x1;
			x1 = x0;
			x0 = (int16)x;
			y = y1;
			y1 = y0;
			y0 = (int16)y;

			lr0 = r1 << 8;
			lr1 = r0 << 8;

			lg0 = g1 << 8;
			lg1 = g0 << 8;

			lb0 = b1 << 8;
			lb1 = b0 << 8;
		}

		dx = x1 - x0;
		dy = y1 - y0;

		dred = lr1 - lr0;
		dgreen = lg1 - lg0;
		dblue = lb1 - lb0;

		dred /= dymod;
		dgreen /= dymod;
		dblue /= dymod;

		if (dx > 0) {
			d = 2 * dx - dy;
			ince = 2 * dx;
			incne = 2 * (dx - dy);
			x = x0;
			y = y0;
			r = lr0;
			g = lg0;
			b = lb0;
			if ((x >= 0) && (x < RGBWidth) && (y >= 0) && (y < RGBHeight)) {
				int32 index = y * RGBPitch + (x * RGBBytesPerPixel);
				pRGB[index + 0] = b0;
				pRGB[index + 1] = g0;
				pRGB[index + 2] = r0;
				pRGB[index + 3] = alpha;
				index = y * ZPitch + (x * ZBytesPerPixel);
				*(uint16 *)(pZ + index + 0) = z;
			}
			while (y < y1) {
				if (d <= 0) {
					d += ince;
					y += 1;
				} else {
					d += incne;
					x += 1;
					y += 1;
				}
				r += dred;
				g += dgreen;
				b += dblue;
				if ((x >= 0) && (x < RGBWidth) && (y >= 0) && (y < RGBHeight)) {
					int32 index = y * RGBPitch + (x * RGBBytesPerPixel);
					pRGB[index + 0] = (uint8)(b >> 8);
					pRGB[index + 1] = (uint8)(g >> 8);
					pRGB[index + 2] = (uint8)(r >> 8);
					pRGB[index + 3] = alpha;
					index = y * ZPitch + (x * ZBytesPerPixel);
					*(uint16 *)(pZ + index + 0) = z;
				}
			}
		} else {
			addTo = x0;
			x0 = 0;
			x1 = (int16)(x1 - addTo);
			x1 = (int16)-x1;
			dx = x1 - x0;

			d = 2 * dx - dy;
			ince = 2 * dx;
			incne = 2 * (dx - dy);
			x = x0;
			y = y0;
			r = lr0;
			g = lg0;
			b = lb0;
			if ((addTo - x >= 0) && (addTo - x < RGBWidth) && (y >= 0) && (y < RGBHeight)) {
				int32 index = y * RGBPitch + ((addTo - x) * RGBBytesPerPixel);
				pRGB[index + 0] = b0;
				pRGB[index + 1] = g0;
				pRGB[index + 2] = r0;
				pRGB[index + 3] = alpha;
				index = y * ZPitch + ((addTo - x) * ZBytesPerPixel);
				*(uint16 *)(pZ + index + 0) = z;
			}

			while (y < y1) {
				if (d <= 0) {
					d += ince;
					y += 1;
				} else {
					d += incne;
					x += 1;
					y += 1;
				}
				r += dred;
				g += dgreen;
				b += dblue;
				if ((addTo - x >= 0) && (addTo - x < RGBWidth) && (y >= 0) && (y < RGBHeight)) {
					int32 index = y * RGBPitch + ((addTo - x) * RGBBytesPerPixel);
					pRGB[index + 0] = (uint8)(b >> 8);
					pRGB[index + 1] = (uint8)(g >> 8);
					pRGB[index + 2] = (uint8)(r >> 8);
					pRGB[index + 3] = alpha;
					index = y * ZPitch + ((addTo - x) * ZBytesPerPixel);
					*(uint16 *)(pZ + index + 0) = z;
				}
			}
		}
	}
	return 0;
}

// two connected lines gouraud coloured : 3 points, 3 colours
int32 DrawLineG3(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2, uint8 alpha, uint16 z) {
	DrawLineG2(x0, y0, x1, y1, r0, g0, b0, r1, g1, b1, alpha, z);
	DrawLineG2(x1, y1, x2, y2, r1, g1, b1, r2, g2, b2, alpha, z);
	return 0;
}

// three connected lines gouraud coloured : 4 points, 4 colours
int32 DrawLineG4(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2, uint8 r3,
			   uint8 g3, uint8 b3, uint8 alpha, uint16 z) {
	DrawLineG2(x0, y0, x1, y1, r0, g0, b0, r1, g1, b1, alpha, z);
	DrawLineG2(x1, y1, x2, y2, r1, g1, b1, r2, g2, b2, alpha, z);
	DrawLineG2(x2, y2, x3, y3, r2, g2, b2, r3, g3, b3, alpha, z);
	return 0;
}

// Simple flat coloured triangle : 3 points, 1 colour
int32 DrawFlatTriangle(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, uint16 z) {
	verts[0].x = (RGBWidth / 2 + x0) << 16;
	verts[0].y = (RGBHeight / 2 + y0) << 16;
	verts[1].x = (RGBWidth / 2 + x1) << 16;
	verts[1].y = (RGBHeight / 2 + y1) << 16;
	verts[2].x = (RGBWidth / 2 + x2) << 16;
	verts[2].y = (RGBHeight / 2 + y2) << 16;

	verts[0].colour = MAKERGB(r0, g0, b0, alpha);

	int32 nVerts;
	if (ClipPolygon(verts, 3, clipverts, &nVerts) == 0)
		return 1;

	int32 ret;

	if (nVerts == 3) {
		ret = DrawFlatUnTexturedPolygon(clipverts, 3, z);
	} else {
		int32 i;
		vertex2D newVerts[3];

		ret = 0;

		for (i = 3; i <= nVerts; i++) {
			newVerts[0] = clipverts[0];
			newVerts[1] = clipverts[i - 2];
			newVerts[2] = clipverts[i - 1];

			if (!SimpleReject(newVerts))
				ret |= DrawFlatUnTexturedPolygon(newVerts, 3, z);
		}
	}

	return ret;
}

// Simple flat coloured quad : 4 points, 1 colour
int32 DrawFlatQuad(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, uint16 z) {
	// Input Quad is :
	//  0----1
	//  |    |
	//  2----3
	// Output Quad is:
	//  0=0----1=1
	//  |      |
	//  3=2----2=3

	verts[0].x = (RGBWidth / 2 + x0) << 16;
	verts[0].y = (RGBHeight / 2 + y0) << 16;
	verts[1].x = (RGBWidth / 2 + x1) << 16;
	verts[1].y = (RGBHeight / 2 + y1) << 16;
	verts[3].x = (RGBWidth / 2 + x2) << 16;
	verts[3].y = (RGBHeight / 2 + y2) << 16;
	verts[2].x = (RGBWidth / 2 + x3) << 16;
	verts[2].y = (RGBHeight / 2 + y3) << 16;

	verts[0].colour = MAKERGB(r0, g0, b0, alpha);

	int32 nVerts;
	if (ClipPolygon(verts, 4, clipverts, &nVerts) == 0)
		return 1;

	int32 ret;
	vertex2D newVerts[3];
	int32 i;
	ret = 0;

	for (i = 3; i <= nVerts; i++) {
		newVerts[0] = clipverts[0];
		newVerts[1] = clipverts[i - 2];
		newVerts[2] = clipverts[i - 1];

		if (!SimpleReject(newVerts))
			ret |= DrawFlatUnTexturedPolygon(newVerts, 3, z);
	}

	return ret;
}

// Simple gouraud coloured triangle : 3 points, 3 colours
int32 DrawGouraudTriangle(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2, uint8 alpha,
						uint16 z) {
	verts[0].x = (RGBWidth / 2 + x0) << 16;
	verts[0].y = (RGBHeight / 2 + y0) << 16;
	verts[1].x = (RGBWidth / 2 + x1) << 16;
	verts[1].y = (RGBHeight / 2 + y1) << 16;
	verts[2].x = (RGBWidth / 2 + x2) << 16;
	verts[2].y = (RGBHeight / 2 + y2) << 16;

	verts[0].colour = MAKERGB(r0, g0, b0, alpha);
	verts[1].colour = MAKERGB(r1, g1, b1, alpha);
	verts[2].colour = MAKERGB(r2, g2, b2, alpha);

	int32 nVerts;
	if (ClipPolygon(verts, 3, clipverts, &nVerts) == 0)
		return 1;

	int32 ret;

	if (nVerts == 3) {
		ret = DrawGouraudUnTexturedPolygon(clipverts, 3, z);
	} else {
		int32 i;
		vertex2D newVerts[3];

		ret = 0;

		for (i = 3; i <= nVerts; i++) {
			newVerts[0] = clipverts[0];
			newVerts[1] = clipverts[i - 2];
			newVerts[2] = clipverts[i - 1];

			if (!SimpleReject(newVerts))
				ret |= DrawGouraudUnTexturedPolygon(newVerts, 3, z);
		}
	}

	return ret;
}

// Simple gouraud coloured quad : 4 points, 4 colours
int32 DrawGouraudQuad(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2,
					uint8 r3, uint8 g3, uint8 b3, uint8 alpha, uint16 z) {
	// Input Quad is :
	//  0----1
	//  |    |
	//  2----3
	// Output Quad is :
	//  0=0----1=1
	//  |      |
	//  3=2----2=3
	verts[0].x = (RGBWidth / 2 + x0) << 16;
	verts[0].y = (RGBHeight / 2 + y0) << 16;
	verts[1].x = (RGBWidth / 2 + x1) << 16;
	verts[1].y = (RGBHeight / 2 + y1) << 16;
	verts[3].x = (RGBWidth / 2 + x2) << 16;
	verts[3].y = (RGBHeight / 2 + y2) << 16;
	verts[2].x = (RGBWidth / 2 + x3) << 16;
	verts[2].y = (RGBHeight / 2 + y3) << 16;

	verts[0].colour = MAKERGB(r0, g0, b0, alpha);
	verts[1].colour = MAKERGB(r1, g1, b1, alpha);
	verts[3].colour = MAKERGB(r2, g2, b2, alpha);
	verts[2].colour = MAKERGB(r3, g3, b3, alpha);

	int32 nVerts;
	if (ClipPolygon(verts, 4, clipverts, &nVerts) == 0)
		return 1;

	int32 ret;
	vertex2D newVerts[3];
	int32 i;
	ret = 0;

	for (i = 3; i <= nVerts; i++) {
		newVerts[0] = clipverts[0];
		newVerts[1] = clipverts[i - 2];
		newVerts[2] = clipverts[i - 1];

		if (!SimpleReject(newVerts))
			ret |= DrawGouraudUnTexturedPolygon(newVerts, 3, z);
	}

	return ret;
}

// Simple flat coloured triangle : 3 points, 1 colour, 3 UV's
// r0, g0, b0 :  128 means colour scale of 1.0
int32 DrawFlatTriangleTextured(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, uint8 r0, uint8 g0, uint8 b0, uint16 u0, uint16 v0, uint16 u1, uint16 v1, uint16 u2, uint16 v2,
							 uint8 alpha, uint16 z, void *tex) {
	verts[0].x = (RGBWidth / 2 + x0) << 16;
	verts[0].y = (RGBHeight / 2 + y0) << 16;
	verts[1].x = (RGBWidth / 2 + x1) << 16;
	verts[1].y = (RGBHeight / 2 + y1) << 16;
	verts[2].x = (RGBWidth / 2 + x2) << 16;
	verts[2].y = (RGBHeight / 2 + y2) << 16;

	verts[0].u = u0 << 8;
	verts[1].u = u1 << 8;
	verts[2].u = u2 << 8;
	verts[0].v = v0 << 8;
	verts[1].v = v1 << 8;
	verts[2].v = v2 << 8;

	verts[0].colour = MAKERGB(r0, g0, b0, alpha);

	int32 nVerts;
	if (ClipPolygon(verts, 3, clipverts, &nVerts) == 0)
		return 1;

	TextureHandle *pthan = NULL;
	if (tex == NULL) {
		return 0;
	} else {
		pthan = (TextureHandle *)tex;
	}

	if ((SetTextureState(pthan)) != 0) {
		return 0;
	}

	int32 ret;

	if (nVerts == 3) {
		ret = DrawFlatTexturedPolygon(clipverts, 3, z);
	} else {
		int32 i;
		vertex2D newVerts[3];

		ret = 0;

		for (i = 3; i <= nVerts; i++) {
			newVerts[0] = clipverts[0];
			newVerts[1] = clipverts[i - 2];
			newVerts[2] = clipverts[i - 1];

			if (!SimpleReject(newVerts))
				ret |= DrawFlatTexturedPolygon(newVerts, 3, z);
		}
	}

	if (tex == NULL)
		delete pthan;
	return ret;
}

// Simple flat coloured quad : 4 points, 1 colour, 4 UV's
// r0, g0, b0 :  128 means colour scale of 1.0
int32 DrawFlatQuadTextured(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 r0, uint8 g0, uint8 b0, uint16 u0, uint16 v0, uint16 u1, uint16 v1, uint16 u2,
						 uint16 v2, uint16 u3, uint16 v3, uint8 alpha, uint16 z, void *tex) {
	// Input Quad is :
	//  0----1
	//  |    |
	//  2----3
	// Output Quad is:
	//  0=0----1=1
	//  |      |
	//  3=2----2=3
	verts[0].x = (RGBWidth / 2 + x0) << 16;
	verts[0].y = (RGBHeight / 2 + y0) << 16;
	verts[1].x = (RGBWidth / 2 + x1) << 16;
	verts[1].y = (RGBHeight / 2 + y1) << 16;
	verts[3].x = (RGBWidth / 2 + x2) << 16;
	verts[3].y = (RGBHeight / 2 + y2) << 16;
	verts[2].x = (RGBWidth / 2 + x3) << 16;
	verts[2].y = (RGBHeight / 2 + y3) << 16;

	verts[0].colour = MAKERGB(r0, g0, b0, alpha);

	verts[0].u = u0 << 8;
	verts[1].u = u1 << 8;
	verts[3].u = u2 << 8;
	verts[2].u = u3 << 8;

	verts[0].v = v0 << 8;
	verts[1].v = v1 << 8;
	verts[3].v = v2 << 8;
	verts[2].v = v3 << 8;

	int32 nVerts;
	if (ClipPolygon(verts, 4, clipverts, &nVerts) == 0)
		return 1;

	TextureHandle *pthan = NULL;
	if (tex == NULL) {
		return 0;
	} else {
		pthan = (TextureHandle *)tex;
	}

	if ((SetTextureState(pthan)) != 0) {
		return 0;
	}

	int32 ret;
	vertex2D newVerts[3];
	int32 i;
	ret = 0;

	for (i = 3; i <= nVerts; i++) {
		newVerts[0] = clipverts[0];
		newVerts[1] = clipverts[i - 2];
		newVerts[2] = clipverts[i - 1];

		if (!SimpleReject(newVerts))
			ret |= DrawFlatTexturedPolygon(newVerts, 3, z);
	}

	if (tex == NULL)
		delete pthan;
	return ret;
}

int32 tempor = 0;

// Simple gouraud coloured triangle : 3 points, 3 colours
// r0, g0, b0 :  128 means colour scale of 1.0
int32 DrawGouraudTriangleTextured(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2,
								uint16 u0, uint16 v0, uint16 u1, uint16 v1, uint16 u2, uint16 v2, uint8 alpha, uint16 z, void *tex) {
	verts[0].x = (RGBWidth / 2 + x0) << 16;
	verts[0].y = (RGBHeight / 2 + y0) << 16;
	verts[1].x = (RGBWidth / 2 + x1) << 16;
	verts[1].y = (RGBHeight / 2 + y1) << 16;
	verts[2].x = (RGBWidth / 2 + x2) << 16;
	verts[2].y = (RGBHeight / 2 + y2) << 16;
	verts[0].u = u0 << 8;
	verts[1].u = u1 << 8;
	verts[2].u = u2 << 8;
	verts[0].v = v0 << 8;
	verts[1].v = v1 << 8;
	verts[2].v = v2 << 8;

	verts[0].colour = MAKERGB(r0, g0, b0, alpha);
	verts[1].colour = MAKERGB(r1, g1, b1, alpha);
	verts[2].colour = MAKERGB(r2, g2, b2, alpha);

	int32 nVerts;
	if (ClipPolygon(verts, 3, clipverts, &nVerts) == 0)
		return 1;

	TextureHandle *pthan = NULL;
	if (tex == NULL) {
		return 0;
	} else {
		pthan = (TextureHandle *)tex;
	}

	if ((SetTextureState(pthan)) != 0) {
		return 0;
	}

	int32 ret;

	if (nVerts == 3) {
		ret = DrawGouraudTexturedPolygon(clipverts, 3, z);
	} else {
		int32 i;
		vertex2D newVerts[3];

		ret = 0;

		for (i = 3; i <= nVerts; i++) {
			newVerts[0] = clipverts[0];
			newVerts[1] = clipverts[i - 2];
			newVerts[2] = clipverts[i - 1];

			if (!SimpleReject(newVerts))
				ret |= DrawGouraudTexturedPolygon(newVerts, 3, z);
		}
	}

	if (tex == NULL)
		delete pthan;

	return ret;
}

// Simple gouraud coloured quad : 4 points, 4 colours
// r0, g0, b0 :  128 means colour scale of 1.0
int32 DrawGouraudQuadTextured(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2,
							uint8 b2, uint8 r3, uint8 g3, uint8 b3, uint16 u0, uint16 v0, uint16 u1, uint16 v1, uint16 u2, uint16 v2, uint16 u3, uint16 v3, uint8 alpha,
							uint16 z, void *tex) {
	// Input Quad is :
	//  0----1
	//  |    |
	//  2----3
	// Output Quad is:
	//  0=0----1=1
	//  |      |
	//  3=2----2=3
	verts[0].x = (RGBWidth / 2 + x0) << 16;
	verts[0].y = (RGBHeight / 2 + y0) << 16;
	verts[1].x = (RGBWidth / 2 + x1) << 16;
	verts[1].y = (RGBHeight / 2 + y1) << 16;
	verts[3].x = (RGBWidth / 2 + x2) << 16;
	verts[3].y = (RGBHeight / 2 + y2) << 16;
	verts[2].x = (RGBWidth / 2 + x3) << 16;
	verts[2].y = (RGBHeight / 2 + y3) << 16;

	verts[0].colour = MAKERGB(r0, g0, b0, alpha);
	verts[1].colour = MAKERGB(r1, g1, b1, alpha);
	verts[3].colour = MAKERGB(r2, g2, b2, alpha);
	verts[2].colour = MAKERGB(r3, g3, b3, alpha);

	verts[0].u = u0 << 8;
	verts[1].u = u1 << 8;
	verts[3].u = u2 << 8;
	verts[2].u = u3 << 8;

	verts[0].v = v0 << 8;
	verts[1].v = v1 << 8;
	verts[3].v = v2 << 8;
	verts[2].v = v3 << 8;

	int32 nVerts;
	if (ClipPolygon(verts, 4, clipverts, &nVerts) == 0)
		return 1;

	TextureHandle *pthan = NULL;
	if (tex == NULL) {
		return 0;
	} else {
		pthan = (TextureHandle *)tex;
	}

	if ((SetTextureState(pthan)) != 0) {
		return 0;
	}

	int32 ret;
	vertex2D newVerts[3];
	int32 i;
	ret = 0;

	for (i = 3; i <= nVerts; i++) {
		newVerts[0] = clipverts[0];
		newVerts[1] = clipverts[i - 2];
		newVerts[2] = clipverts[i - 1];

		if (!SimpleReject(newVerts))
			ret |= DrawGouraudTexturedPolygon(newVerts, 3, z);
	}

	if (tex == NULL)
		delete pthan;
	return ret;
}

inline void Intersection(vertex2D *out, vertex2D v0, vertex2D v1, int32 p) {

	out->x = v0.x + p * ((v1.x - v0.x) >> 8);
	out->y = v0.y + p * ((v1.y - v0.y) >> 8);
	out->colour = v0.colour;
	out->u = v0.u + p * ((v1.u - v0.u) >> 8);
	out->v = v0.v + p * ((v1.v - v0.v) >> 8);
}

inline void ClipWithTopPlane(vertex2D *inVerts, int32 inNverts, vertex2D *outVerts, int32 *outNverts) {
	vertex2D intersect;
	int32 i, ni;
	int32 j;
	int32 percent;
	uint8 bits[MAX_POLYGON_VERTS];

	int32 yPlane = (0) << 16;

	for (i = 0; i < inNverts; i++)
		bits[i] = (uint8)(inVerts[i].y >= yPlane);

	j = 0;

	for (i = 0; i < inNverts; i++) {
		if (i == inNverts - 1)
			ni = 0;
		else
			ni = i + 1;

		if ((bits[i]) && (bits[ni])) // 1 1 -> put A
			outVerts[j++] = inVerts[i];
		else if (bits[i]) { // 1 0 -> put A, put i
			// put A
			outVerts[j++] = inVerts[i];

			// intersection...
			if (!((inVerts[i].y - inVerts[ni].y) >> 16))
				percent = 256;
			else
				percent = (256 * ((inVerts[i].y >> 16) - yPlane)) / ((inVerts[i].y - inVerts[ni].y) >> 16);
			Intersection(&intersect, inVerts[i], inVerts[ni], percent);

			// This must be true by definition
			intersect.y = yPlane;

			// put i
			outVerts[j++] = intersect;
		} else if (bits[ni]) { // 0 1 -> put i
			// intersection...
			if (!((inVerts[i].y - inVerts[ni].y) >> 16))
				percent = 256;
			else
				percent = (256 * ((inVerts[i].y >> 16) - yPlane)) / ((inVerts[i].y - inVerts[ni].y) >> 16);

			Intersection(&intersect, inVerts[i], inVerts[ni], percent);

			// This must be true by definition
			intersect.y = yPlane;

			// put i
			outVerts[j++] = intersect;
		} else { // 0 0 -> nothing
			 /* nothing */
		}
	}

	*outNverts = j;
}

inline void ClipWithBottomPlane(vertex2D *inVerts, int32 inNverts, vertex2D *outVerts, int32 *outNverts) {
	vertex2D intersect;
	int32 i, ni;
	int32 j;
	int32 percent;
	uint8 bits[MAX_POLYGON_VERTS];

	int32 yPlane = (RGBHeight - 1) << 16;

	for (i = 0; i < inNverts; i++)
		bits[i] = (uint8)(inVerts[i].y < yPlane);

	j = 0;

	for (i = 0; i < inNverts; i++) {
		if (i == inNverts - 1)
			ni = 0;
		else
			ni = i + 1;

		if ((bits[i]) && (bits[ni])) // 1 1 -> put A
			outVerts[j++] = inVerts[i];
		else if (bits[i]) { // 1 0 -> put A, put i
			// put A
			outVerts[j++] = inVerts[i];

			// intersection...
			if (!((inVerts[ni].y - inVerts[i].y) >> 16))
				percent = 256;
			else
				percent = (256 * ((yPlane - inVerts[i].y) >> 16)) / ((inVerts[ni].y - inVerts[i].y) >> 16);
			Intersection(&intersect, inVerts[i], inVerts[ni], percent);

			// This must be true by definition
			intersect.y = yPlane;

			// put i
			outVerts[j++] = intersect;
		} else if (bits[ni]) { // 0 1 -> put i
			// intersection...
			if (!((inVerts[ni].y - inVerts[i].y) >> 16))
				percent = 256;
			else
				percent = (256 * ((yPlane - inVerts[i].y) >> 16)) / ((inVerts[ni].y - inVerts[i].y) >> 16);
			Intersection(&intersect, inVerts[i], inVerts[ni], percent);

			// This must be true by definition
			intersect.y = yPlane;

			// put i
			outVerts[j++] = intersect;
		} else { // 0 0 -> nothing
			 /* nothing */
		}
	}

	*outNverts = j;
}

inline void ClipWithLeftPlane(vertex2D *inVerts, int32 inNverts, vertex2D *outVerts, int32 *outNverts) {
	vertex2D intersect;
	int32 i, ni;
	int32 j;
	int32 percent;
	uint8 bits[MAX_POLYGON_VERTS];

	int32 xPlane = (0) << 16;
	for (i = 0; i < inNverts; i++)
		bits[i] = (uint8)(inVerts[i].x >= xPlane);

	j = 0;

	for (i = 0; i < inNverts; i++) {
		if (i == inNverts - 1)
			ni = 0;
		else
			ni = i + 1;

		if ((bits[i]) && (bits[ni])) // 1 1 -> put A
			outVerts[j++] = inVerts[i];
		else if (bits[i]) { // 1 0 -> put A, put i
			// put A
			outVerts[j++] = inVerts[i];

			// intersection...
			if (!((inVerts[i].x - inVerts[ni].x) >> 16))
				percent = 256;
			else
				percent = (256 * ((inVerts[i].x >> 16) - xPlane)) / ((inVerts[i].x - inVerts[ni].x) >> 16);
			Intersection(&intersect, inVerts[i], inVerts[ni], percent);

			// This must be true by definition
			intersect.x = xPlane;

			// put i
			outVerts[j++] = intersect;
		} else if (bits[ni]) { // 0 1 -> put i
			// intersection...
			if (!((inVerts[i].x - inVerts[ni].x) >> 16))
				percent = 256;
			else
				percent = (256 * ((inVerts[i].x >> 16) - xPlane)) / ((inVerts[i].x - inVerts[ni].x) >> 16);
			Intersection(&intersect, inVerts[i], inVerts[ni], percent);

			// This must be true by definition
			intersect.x = xPlane;

			// put i
			outVerts[j++] = intersect;
		} else { // 0 0 -> nothing
			 /* nothing */
		}
	}

	*outNverts = j;
}

inline void ClipWithRightPlane(vertex2D *inVerts, int32 inNverts, vertex2D *outVerts, int32 *outNverts) {
	vertex2D intersect;
	int32 i, ni;
	int32 j;
	int32 percent;
	uint8 bits[MAX_POLYGON_VERTS];

	int32 xPlane = (RGBWidth - 1) << 16;
	for (i = 0; i < inNverts; i++)
		bits[i] = (uint8)(inVerts[i].x < xPlane);

	j = 0;

	for (i = 0; i < inNverts; i++) {
		if (i == inNverts - 1)
			ni = 0;
		else
			ni = i + 1;

		if ((bits[i]) && (bits[ni])) // 1 1 -> put A
			outVerts[j++] = inVerts[i];
		else if (bits[i]) { // 1 0 -> put A, put i
			// put A
			outVerts[j++] = inVerts[i];

			// intersection...
			if (!((inVerts[ni].x - inVerts[i].x) >> 16))
				percent = 256;
			else
				percent = (256 * ((xPlane - inVerts[i].x) >> 16)) / ((inVerts[ni].x - inVerts[i].x) >> 16);
			Intersection(&intersect, inVerts[i], inVerts[ni], percent);

			// This must be true by definition
			intersect.x = xPlane;

			// put i
			outVerts[j++] = intersect;
		} else if (bits[ni]) { // 0 1 -> put i
			// intersection...
			if (!((inVerts[ni].x - inVerts[i].x) >> 16))
				percent = 256;
			else
				percent = (256 * ((xPlane - inVerts[i].x) >> 16)) / ((inVerts[ni].x - inVerts[i].x) >> 16);
			Intersection(&intersect, inVerts[i], inVerts[ni], percent);

			// This must be true by definition
			intersect.x = xPlane;

			// put i
			outVerts[j++] = intersect;
		} else { // 0 0 -> nothing
			 /* nothing */
		}
	}

	*outNverts = j;
}

// Simple clipping turned on until the Dutch DLL crash bug
// is fixed
// #define SIMPLE_CLIPPING

#ifdef SIMPLE_CLIPPING

// Simple clipping by polygon rejection

int32 ClipPolygon(vertex2D *inverts, int32 inNverts, vertex2D *outverts, int32 *ret) {
	int32 i;

	for (i = 0; i < inNverts; i++) {

		outverts[i] = inverts[i];

		if ((inverts[i].x < 0) || (inverts[i].x > (RGBWidth << 16)) || (inverts[i].y < 0) || (inverts[i].y > (RGBHeight << 16)))
			return 0;
	}

	*ret = inNverts;

	return 1;
}

#else // #ifdef SIMPLE_CLIPPING

// Complex clipping to the screen boundaries

int32 ClipPolygon(vertex2D *inverts, int32 inNverts, vertex2D *outverts, int32 *ret) {
	int32 i;

	int32 outNverts;

	int32 newinNverts;
	vertex2D newinverts[MAX_POLYGON_VERTS];

	if (SimpleReject(inverts))
		return 0;

	for (i = 0; i < inNverts; i++)
		outverts[i] = inverts[i]; // make OUT
	outNverts = inNverts;

	ClipWithLeftPlane(outverts, outNverts, newinverts, &newinNverts);   // OUT-TEMP with left
	ClipWithRightPlane(newinverts, newinNverts, outverts, &outNverts);  // TEMP-OUT with right
	ClipWithBottomPlane(outverts, outNverts, newinverts, &newinNverts); // OUT-TEMP with bottom
	ClipWithTopPlane(newinverts, newinNverts, outverts, &outNverts);    // TEMP-OUT with top

	*ret = outNverts;
	return outNverts;
}

#endif // #ifdef SIMPLE_CLIPPING

} // End of namespace ICB
