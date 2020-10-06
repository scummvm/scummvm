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

#include "engines/icb/common/px_common.h"
#include "engines/icb/gfx/gfxstub_dutch.h"
#include "engines/icb/gfx/gfxstub_rev_dutch.h"

namespace ICB {

#ifndef ENABLE_OPENGL

typedef struct {
	int x0, x1;
	int count;
	int a0, r0, g0, b0;
	int a1, r1, g1, b1;
	int u0, v0;
	int u1, v1;
} span_t;

#define MAX_SCREEN_HEIGHT 4096
span_t spans[MAX_SCREEN_HEIGHT];

#define GETBValue(rgb) ((u_char)(rgb))
#define GETGValue(rgb) ((u_char)(((u_short)(rgb)) >> 8))
#define GETRValue(rgb) ((u_char)((rgb) >> 16))
#define GETAValue(rgb) ((u_char)((rgb) >> 24))

extern int mip_map_level;

typedef struct {
	char *pRGB;
	int RGBPitch;
	int RGBBytesPerPixel;

	char *pZ;
	int ZPitch;
	int ZBytesPerPixel;
} MyRenderDevice;

MyRenderDevice myRenDev = {NULL, 0, 0, NULL, 0, 0};
RevRenderDevice *lastRevRenDev = NULL;

int SetRenderDevice(RevRenderDevice *renderDev) {
	lastRevRenDev = NULL;
	if (renderDev->RGBdata == NULL)
		return 1;
	if (renderDev->Zdata == NULL)
		return 1;
	if (renderDev->width <= 0)
		return 1;
	if (renderDev->width > 2048)
		return 1;
	if (renderDev->height <= 0)
		return 1;
	if (renderDev->height > 2048)
		return 1;

	myRenDev.pRGB = (char *)(renderDev->RGBdata);
	myRenDev.pZ = (char *)(renderDev->Zdata);
	myRenDev.RGBBytesPerPixel = 4;
	myRenDev.RGBPitch = renderDev->stride;
	myRenDev.ZBytesPerPixel = 2;
	myRenDev.ZPitch = renderDev->width * myRenDev.ZBytesPerPixel;
	lastRevRenDev = renderDev;
	return 0;
}

TextureHandle myTexHan;

int SetTextureState(TextureHandle *texture) {
	myTexHan = *texture;
	return 0;
}

int UnregisterTexture(TextureHandle *texture) {
	int i;

	for (i = 0; i < 9; i++)
		if ((texture->pRGBA[i]) != NULL)
			delete[](texture->pRGBA[i]);

	if (texture->palette != NULL)
		delete[](texture->palette);

	delete texture;

	// always return zero
	return 0;
}

TextureHandle *RegisterTexture(const RevTexture *revInput) {
	int i;
	TextureHandle *th = new TextureHandle();

	th->w = revInput->width;
	th->h = revInput->height;

	for (i = 0; i < 9; i++)
		th->pRGBA[i] = NULL;

	if (revInput->palette[0] == 0xDEADBEAF) {
		th->bpp = 4;
		th->palette = NULL;
		th->pRGBA[0] = revInput->level[0];
	} else {
		// Complain if width or height > 256 < 1
		if ((th->w < 1) || (th->w > 256)) {
			delete th;
			return NULL;
		}
		if ((th->h < 1) || (th->h > 256)) {
			delete th;
			return NULL;
		}

		// Complain if the width or height are not powers of 2
		for (i = 0; i < 8; i++) {
			// ERROR
			if (((th->w >> i) << i) != th->w) {
				if ((th->w >> i) != 0) {
					delete th;
					return NULL;
				}
			}
			// ERROR
			if (((th->h >> i) << i) != th->h) {
				if ((th->h >> i) != 0) {
					delete th;
					return NULL;
				}
			}
		}

		th->bpp = 1;
		th->palette = new u_int[256];
		for (i = 0; i < 256; i++)
			th->palette[i] = revInput->palette[i];

		int size = th->w * th->h * th->bpp;
		for (i = 0; i < 9; i++) {
			th->pRGBA[i] = new u_char[size];
			memcpy(th->pRGBA[i], revInput->level[i], size);
			size /= 4;
			if (size / th->bpp == 0)
				break;
		}
	}

	return th;
}

void ClearProcessorState() { return; }

int DrawGouraudTexturedPolygon(const vertex2D *verts, int nVerts, u_short z) {
	int i, j, topvert, bottomvert, leftvert, rightvert, nextvert;
	int itopy, ibottomy, spantopy, spanbottomy, count;
	int x, a, r, g, b, u, v;
	int ixslope, iaslope, irslope, igslope, ibslope, iuslope, ivslope;
	float topy, bottomy, height, width, prestep;
	float xslope, aslope, rslope, gslope, bslope, uslope, vslope;
	float y;
	span_t *pspan;

	if (myRenDev.pRGB == NULL)
		return 0;
	if (myTexHan.pRGBA[mip_map_level] == NULL)
		return 0;

	// Test for clockwise polygons
	int l0x = (verts[0].x - verts[1].x) >> 16;
	int l0y = (verts[0].y - verts[1].y) >> 16;
	int l1x = (verts[2].x - verts[1].x) >> 16;
	int l1y = (verts[2].y - verts[1].y) >> 16;
	if (l0x * l1y > l0y * l1x) {
		return 0;
	}

	topy = 999999.0f;
	bottomy = -999999.0f;
	topvert = 0;
	bottomvert = 0;
	for (i = 0; i < nVerts; i++) {
		y = (float)(verts[i].y / 65536.0f);
		if (y < topy) {
			topy = y;
			topvert = i;
		}
		if (y > bottomy) {
			bottomy = y;
			bottomvert = i;
		}
	}

	itopy = (int)ceil(topy);
	ibottomy = (int)ceil(bottomy);

	// reject polygons that don't cross a scan
	if (ibottomy == itopy)
		return 1;

	// Scan out the left edge
	pspan = spans;
	leftvert = topvert;

	do {
		nextvert = leftvert - 1;
		if (nextvert < 0)
			nextvert = nVerts - 1;
		y = (float)(verts[leftvert].y / 65536.0f);
		spantopy = (int)ceil(y);
		y = (float)(verts[nextvert].y / 65536.0f);
		spanbottomy = (int)ceil(y);
		if (spantopy < spanbottomy) {
			height = (float)((verts[nextvert].y - verts[leftvert].y) / 65536.0f);
			width = (float)((verts[nextvert].x - verts[leftvert].x) / 65536.0f);

			xslope = width / height;
			uslope = (float)((verts[nextvert].u - verts[leftvert].u) / 65536.0f) / height;
			vslope = (float)((verts[nextvert].v - verts[leftvert].v) / 65536.0f) / height;
			aslope = (GETAValue(verts[nextvert].colour) - GETAValue(verts[leftvert].colour)) / height;
			rslope = (GETRValue(verts[nextvert].colour) - GETRValue(verts[leftvert].colour)) / height;
			gslope = (GETGValue(verts[nextvert].colour) - GETGValue(verts[leftvert].colour)) / height;
			bslope = (GETBValue(verts[nextvert].colour) - GETBValue(verts[leftvert].colour)) / height;

			prestep = spantopy - (float)(verts[leftvert].y / 65536.0f);

			x = (int)((((verts[leftvert].x) / 65536.0f) + (xslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			u = (int)((((verts[leftvert].u) / 65536.0f) + (uslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			v = (int)((((verts[leftvert].v) / 65536.0f) + (vslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			a = (int)((GETAValue(verts[leftvert].colour) + (aslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			r = (int)((GETRValue(verts[leftvert].colour) + (rslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			g = (int)((GETGValue(verts[leftvert].colour) + (gslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			b = (int)((GETBValue(verts[leftvert].colour) + (bslope * prestep)) * 65536.0) + ((1 << 16) - 1);

			ixslope = (int)(xslope * 65536.0);
			iuslope = (int)(uslope * 65536.0);
			ivslope = (int)(vslope * 65536.0);
			iaslope = (int)(aslope * 65536.0);
			irslope = (int)(rslope * 65536.0);
			igslope = (int)(gslope * 65536.0);
			ibslope = (int)(bslope * 65536.0);

			for (j = spantopy; j < spanbottomy; j++) {
				pspan->x0 = x >> 16;
				pspan->u0 = u >> 16;
				pspan->v0 = v >> 16;
				pspan->a0 = a >> 16;
				pspan->r0 = r >> 16;
				pspan->g0 = g >> 16;
				pspan->b0 = b >> 16;
				x += ixslope;
				u += iuslope;
				v += ivslope;
				a += iaslope;
				r += irslope;
				g += igslope;
				b += ibslope;
				pspan++;
			}
		}

		leftvert--;
		if (leftvert < 0)
			leftvert = nVerts - 1;
	} while (leftvert != bottomvert);

	// Scan out the right edge
	pspan = spans;
	rightvert = topvert;

	do {
		nextvert = (rightvert + 1) % nVerts;
		y = (float)(verts[rightvert].y / 65536.0f);
		spantopy = (int)ceil(y);
		y = (float)(verts[nextvert].y / 65536.0f);
		spanbottomy = (int)ceil(y);
		if (spantopy < spanbottomy) {
			height = (float)((verts[nextvert].y - verts[rightvert].y) / 65536.0f);
			width = (float)((verts[nextvert].x - verts[rightvert].x) / 65536.0f);

			xslope = width / height;
			uslope = (float)((verts[nextvert].u - verts[rightvert].u) / 65536.0f) / height;
			vslope = (float)((verts[nextvert].v - verts[rightvert].v) / 65536.0f) / height;
			aslope = (GETAValue(verts[nextvert].colour) - GETAValue(verts[rightvert].colour)) / height;
			rslope = (GETRValue(verts[nextvert].colour) - GETRValue(verts[rightvert].colour)) / height;
			gslope = (GETGValue(verts[nextvert].colour) - GETGValue(verts[rightvert].colour)) / height;
			bslope = (GETBValue(verts[nextvert].colour) - GETBValue(verts[rightvert].colour)) / height;

			prestep = spantopy - (float)(verts[rightvert].y / 65536.0f);

			x = (int)((((verts[rightvert].x) / 65536.0f) + (xslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			u = (int)((((verts[rightvert].u) / 65536.0f) + (uslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			v = (int)((((verts[rightvert].v) / 65536.0f) + (vslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			a = (int)((GETAValue(verts[rightvert].colour) + (aslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			r = (int)((GETRValue(verts[rightvert].colour) + (rslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			g = (int)((GETGValue(verts[rightvert].colour) + (gslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			b = (int)((GETBValue(verts[rightvert].colour) + (bslope * prestep)) * 65536.0) + ((1 << 16) - 1);

			ixslope = (int)(xslope * 65536.0);
			iuslope = (int)(uslope * 65536.0);
			ivslope = (int)(vslope * 65536.0);
			iaslope = (int)(aslope * 65536.0);
			irslope = (int)(rslope * 65536.0);
			igslope = (int)(gslope * 65536.0);
			ibslope = (int)(bslope * 65536.0);

			for (j = spantopy; j < spanbottomy; j++) {
				pspan->x1 = x >> 16;
				pspan->u1 = u >> 16;
				pspan->v1 = v >> 16;
				pspan->a1 = a >> 16;
				pspan->r1 = r >> 16;
				pspan->g1 = g >> 16;
				pspan->b1 = b >> 16;
				x += ixslope;
				u += iuslope;
				v += ivslope;
				a += iaslope;
				r += irslope;
				g += igslope;
				b += ibslope;
				pspan++;
			}
		}

		rightvert = (rightvert + 1) % nVerts;
	} while (rightvert != bottomvert);

	// Draw the spans
	pspan = spans;

	int mipw = myTexHan.w >> mip_map_level;
	int miph = myTexHan.h >> mip_map_level;

	for (i = itopy; i < ibottomy; i++) {
		count = pspan->x1 - pspan->x0;
		if (count > 0) {
			x = pspan->x0;
			u = (pspan->u0 << 8);
			v = (pspan->v0 << 8);
			a = pspan->a0 << 8;
			r = pspan->r0 << 8;
			g = pspan->g0 << 8;
			b = pspan->b0 << 8;

			iuslope = ((pspan->u1 << 8) - u) / count;
			ivslope = ((pspan->v1 << 8) - v) / count;
			iaslope = ((pspan->a1 << 8) - a) / count;
			irslope = ((pspan->r1 << 8) - r) / count;
			igslope = ((pspan->g1 << 8) - g) / count;
			ibslope = ((pspan->b1 << 8) - b) / count;

			char *left = myRenDev.pRGB + (myRenDev.RGBPitch * i) + myRenDev.RGBBytesPerPixel * x;
			char *zleft = myRenDev.pZ + (myRenDev.ZPitch * i) + myRenDev.ZBytesPerPixel * pspan->x0;
			do {
				int pu = (u >> (8 + mip_map_level));
				int pv = (v >> (8 + mip_map_level));

				if (pu < 0)
					pu = 0;
				if (pu >= mipw)
					pu = mipw - 1;

				if (pv < 0)
					pv = 0;
				if (pv >= miph)
					pv = miph - 1;

				u_int toff = (pu + (pv * mipw)) * myTexHan.bpp;
				u_char *texel = ((u_char *)(myTexHan.pRGBA[mip_map_level])) + toff;

				// RGB data
				int ta, tr, tg, tb;
				if (myTexHan.bpp > 3) {
					ta = *(texel + 3);
					tr = *(texel + 2);
					tg = *(texel + 1);
					tb = *(texel + 0);
				} else {
					// Palette data
					u_char index = *texel;
					u_int colour = myTexHan.palette[index];
					ta = ((colour >> 24) & 0xFF);
					tr = ((colour >> 16) & 0xFF);
					tg = ((colour >> 8) & 0xFF);
					tb = ((colour >> 0) & 0xFF);
				}

				// BGR : 128 = scale of 1.0
				int pr = ((r >> 8) * tr);
				int pg = ((g >> 8) * tg);
				int pb = ((b >> 8) * tb);

				if (pr < 0)
					pr = 0;
				if (pg < 0)
					pg = 0;
				if (pb < 0)
					pb = 0;

				pr = pr >> 7;
				pg = pg >> 7;
				pb = pb >> 7;

				if (pr > 255)
					pr = 255;
				if (pg > 255)
					pg = 255;
				if (pb > 255)
					pb = 255;

				*(left + 0) = (char)pb;
				*(left + 1) = (char)pg;
				*(left + 2) = (char)pr;
				*(left + 3) = (char)ta; // use the texture alpha value
				*(u_short *)(zleft + 0) = z;

				left += myRenDev.RGBBytesPerPixel;
				zleft += myRenDev.ZBytesPerPixel;
				x++;
				u += iuslope;
				v += ivslope;
				r += irslope;
				g += igslope;
				b += ibslope;
				count--;
			} while (count > 0);
		}
		pspan++;
	}
	return 1;
}

int DrawFlatUnTexturedPolygon(const vertex2D *verts, int nVerts, u_short z) {
	int i, j, topvert, bottomvert, leftvert, rightvert, nextvert;
	int itopy, ibottomy, ixslope, spantopy, spanbottomy, x, count;
	float topy, bottomy, xslope, height, width, prestep;
	float y;
	span_t *pspan;

	if (myRenDev.pRGB == NULL)
		return 0;

	// Test for clockwise polygons
	int l0x = (verts[0].x - verts[1].x) >> 16;
	int l0y = (verts[0].y - verts[1].y) >> 16;
	int l1x = (verts[2].x - verts[1].x) >> 16;
	int l1y = (verts[2].y - verts[1].y) >> 16;
	if (l0x * l1y > l0y * l1x) {
		return 0;
	}

	// The colour is in vertex 0
	u_char a0 = GETAValue(verts[0].colour);
	u_char r0 = GETRValue(verts[0].colour);
	u_char g0 = GETGValue(verts[0].colour);
	u_char b0 = GETBValue(verts[0].colour);

	topy = 999999.0f;
	bottomy = -999999.0f;
	topvert = 0;
	bottomvert = 0;
	for (i = 0; i < nVerts; i++) {
		y = (float)(verts[i].y / 65536.0f);
		if (y < topy) {
			topy = y;
			topvert = i;
		}
		if (y > bottomy) {
			bottomy = y;
			bottomvert = i;
		}
	}

	itopy = (int)ceil(topy);
	ibottomy = (int)ceil(bottomy);

	// reject polygons that don't cross a scan
	if (ibottomy == itopy)
		return 1;

	// Scan out the left edge
	pspan = spans;
	leftvert = topvert;

	do {
		nextvert = leftvert - 1;
		if (nextvert < 0)
			nextvert = nVerts - 1;
		y = (float)(verts[leftvert].y / 65536.0f);
		spantopy = (int)ceil(y);
		y = (float)(verts[nextvert].y / 65536.0f);
		spanbottomy = (int)ceil(y);
		if (spantopy < spanbottomy) {
			height = (float)((int)(verts[nextvert].y - verts[leftvert].y) / 65536.0f);
			width = (float)((int)(verts[nextvert].x - verts[leftvert].x) / 65536.0f);

			xslope = width / height;

			prestep = spantopy - (float)((verts[leftvert].y) / 65536.0f);

			x = (int)((((verts[leftvert].x) / 65536.0f) + (xslope * prestep)) * 65536.0) + ((1 << 16) - 1);

			ixslope = (int)(xslope * 65536.0);

			for (j = spantopy; j < spanbottomy; j++) {
				pspan->x0 = x >> 16;
				x += ixslope;
				pspan++;
			}
		}

		leftvert--;
		if (leftvert < 0)
			leftvert = nVerts - 1;
	} while (leftvert != bottomvert);

	// Scan out the right edge
	pspan = spans;
	rightvert = topvert;

	do {
		nextvert = (rightvert + 1) % nVerts;
		y = (float)(verts[rightvert].y / 65536.0f);
		spantopy = (int)ceil(y);
		y = (float)(verts[nextvert].y / 65536.0f);
		spanbottomy = (int)ceil(y);
		if (spantopy < spanbottomy) {
			height = (float)((verts[nextvert].y - verts[rightvert].y) / 65536.0f);
			width = (float)((verts[nextvert].x - verts[rightvert].x) / 65536.0f);

			xslope = width / height;

			prestep = spantopy - (float)((verts[rightvert].y) / 65536.0f);

			x = (int)((((verts[rightvert].x) / 65536.0f) + (xslope * prestep)) * 65536.0) + ((1 << 16) - 1);

			ixslope = (int)(xslope * 65536.0);

			for (j = spantopy; j < spanbottomy; j++) {
				pspan->x1 = x >> 16;
				x += ixslope;
				pspan++;
			}
		}

		rightvert = (rightvert + 1) % nVerts;
	} while (rightvert != bottomvert);

	// Draw the spans
	pspan = spans;

	for (i = itopy; i < ibottomy; i++) {
		count = pspan->x1 - pspan->x0;
		if (count > 0) {
			char *left = myRenDev.pRGB + (myRenDev.RGBPitch * i) + myRenDev.RGBBytesPerPixel * pspan->x0;
			char *zleft = myRenDev.pZ + (myRenDev.ZPitch * i) + myRenDev.ZBytesPerPixel * pspan->x0;
			do {
				*(left + 0) = b0;
				*(left + 1) = g0;
				*(left + 2) = r0;
				*(left + 3) = a0;
				left += myRenDev.RGBBytesPerPixel;
				*(u_short *)(zleft + 0) = z;
				zleft += myRenDev.ZBytesPerPixel;
				count--;
			} while (count > 0);
		}
		pspan++;
	}
	return 1;
}

int DrawGouraudUnTexturedPolygon(const vertex2D *verts, int nVerts, u_short z) {
	int i, j, topvert, bottomvert, leftvert, rightvert, nextvert;
	int itopy, ibottomy, spantopy, spanbottomy, count;
	int ixslope, iaslope, irslope, igslope, ibslope;
	int x, a, r, g, b;
	float topy, bottomy, xslope, aslope, rslope, gslope, bslope;
	float height, width, prestep;
	float y;
	span_t *pspan;

	if (myRenDev.pRGB == NULL)
		return 0;

	// Test for clockwise polygons
	int l0x = (verts[0].x - verts[1].x) >> 16;
	int l0y = (verts[0].y - verts[1].y) >> 16;
	int l1x = (verts[2].x - verts[1].x) >> 16;
	int l1y = (verts[2].y - verts[1].y) >> 16;
	if (l0x * l1y > l0y * l1x) {
		return 0;
	}

	topy = 999999.0f;
	bottomy = -999999.0f;
	topvert = 0;
	bottomvert = 0;
	for (i = 0; i < nVerts; i++) {
		y = (float)((verts[i].y) / 65536.0f);
		if (y < topy) {
			topy = y;
			topvert = i;
		}
		if (y > bottomy) {
			bottomy = y;
			bottomvert = i;
		}
	}

	itopy = (int)ceil(topy);
	ibottomy = (int)ceil(bottomy);

	if (ibottomy == itopy)
		return 1; // reject polygons that don't cross a scan

	// Scan out the left edge
	pspan = spans;
	leftvert = topvert;

	do {
		nextvert = leftvert - 1;
		if (nextvert < 0)
			nextvert = nVerts - 1;
		y = (float)((verts[leftvert].y) / 65536.0f);
		spantopy = (int)ceil(y);
		y = (float)((verts[nextvert].y) / 65536.0f);
		spanbottomy = (int)ceil(y);
		if (spantopy < spanbottomy) {
			height = (float)((verts[nextvert].y - verts[leftvert].y) / 65536.0f);
			width = (float)((verts[nextvert].x - verts[leftvert].x) / 65536.0f);

			xslope = width / height;
			aslope = (GETAValue(verts[nextvert].colour) - GETAValue(verts[leftvert].colour)) / height;
			rslope = (GETRValue(verts[nextvert].colour) - GETRValue(verts[leftvert].colour)) / height;
			gslope = (GETGValue(verts[nextvert].colour) - GETGValue(verts[leftvert].colour)) / height;
			bslope = (GETBValue(verts[nextvert].colour) - GETBValue(verts[leftvert].colour)) / height;

			prestep = spantopy - (float)((verts[leftvert].y) / 65536.0f);

			x = (int)((((verts[leftvert].x) / 65536.0f) + (xslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			a = (int)((GETAValue(verts[leftvert].colour) + (aslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			r = (int)((GETRValue(verts[leftvert].colour) + (rslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			g = (int)((GETGValue(verts[leftvert].colour) + (gslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			b = (int)((GETBValue(verts[leftvert].colour) + (bslope * prestep)) * 65536.0) + ((1 << 16) - 1);

			ixslope = (int)(xslope * 65536.0);
			iaslope = (int)(aslope * 65536.0);
			irslope = (int)(rslope * 65536.0);
			igslope = (int)(gslope * 65536.0);
			ibslope = (int)(bslope * 65536.0);

			for (j = spantopy; j < spanbottomy; j++) {
				pspan->x0 = x >> 16;
				pspan->a0 = a >> 16;
				pspan->r0 = r >> 16;
				pspan->g0 = g >> 16;
				pspan->b0 = b >> 16;
				x += ixslope;
				a += iaslope;
				r += irslope;
				g += igslope;
				b += ibslope;
				pspan++;
			}
		}

		leftvert--;
		if (leftvert < 0)
			leftvert = nVerts - 1;
	} while (leftvert != bottomvert);

	// Scan out the right edge
	pspan = spans;
	rightvert = topvert;

	do {
		nextvert = (rightvert + 1) % nVerts;
		y = (float)((verts[rightvert].y) / 65536.0f);
		spantopy = (int)ceil(y);
		y = (float)((verts[nextvert].y) / 65536.0f);
		spanbottomy = (int)ceil(y);
		if (spantopy < spanbottomy) {
			height = (float)((verts[nextvert].y - verts[rightvert].y) / 65536.0f);
			width = (float)((verts[nextvert].x - verts[rightvert].x) / 65536.0f);

			xslope = width / height;
			aslope = (GETAValue(verts[nextvert].colour) - GETAValue(verts[rightvert].colour)) / height;
			rslope = (GETRValue(verts[nextvert].colour) - GETRValue(verts[rightvert].colour)) / height;
			gslope = (GETGValue(verts[nextvert].colour) - GETGValue(verts[rightvert].colour)) / height;
			bslope = (GETBValue(verts[nextvert].colour) - GETBValue(verts[rightvert].colour)) / height;

			prestep = spantopy - (float)((verts[rightvert].y) / 65536.0f);

			x = (int)((((verts[rightvert].x) / 65536.0f) + (xslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			a = (int)((GETAValue(verts[rightvert].colour) + (aslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			r = (int)((GETRValue(verts[rightvert].colour) + (rslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			g = (int)((GETGValue(verts[rightvert].colour) + (gslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			b = (int)((GETBValue(verts[rightvert].colour) + (bslope * prestep)) * 65536.0) + ((1 << 16) - 1);

			ixslope = (int)(xslope * 65536.0);
			iaslope = (int)(aslope * 65536.0);
			irslope = (int)(rslope * 65536.0);
			igslope = (int)(gslope * 65536.0);
			ibslope = (int)(bslope * 65536.0);

			for (j = spantopy; j < spanbottomy; j++) {
				pspan->x1 = x >> 16;
				pspan->a1 = a >> 16;
				pspan->r1 = r >> 16;
				pspan->g1 = g >> 16;
				pspan->b1 = b >> 16;
				x += ixslope;
				a += iaslope;
				r += irslope;
				g += igslope;
				b += ibslope;
				pspan++;
			}
		}

		rightvert = (rightvert + 1) % nVerts;
	} while (rightvert != bottomvert);

	// Draw the spans
	pspan = spans;

	for (i = itopy; i < ibottomy; i++) {
		count = pspan->x1 - pspan->x0;
		if (count > 0) {
			x = pspan->x0;
			a = pspan->a0 << 8;
			r = pspan->r0 << 8;
			g = pspan->g0 << 8;
			b = pspan->b0 << 8;
			iaslope = ((pspan->a1 << 8) - a) / count;
			irslope = ((pspan->r1 << 8) - r) / count;
			igslope = ((pspan->g1 << 8) - g) / count;
			ibslope = ((pspan->b1 << 8) - b) / count;
			char *left = myRenDev.pRGB + (myRenDev.RGBPitch * i) + myRenDev.RGBBytesPerPixel * x;
			char *zleft = myRenDev.pZ + (myRenDev.ZPitch * i) + myRenDev.ZBytesPerPixel * pspan->x0;
			do {
				*(left + 0) = (char)(b >> 8);
				*(left + 1) = (char)(g >> 8);
				*(left + 2) = (char)(r >> 8);
				*(left + 3) = (char)(a >> 8);
				*(u_short *)(zleft + 0) = z;
				zleft += myRenDev.ZBytesPerPixel;
				left += myRenDev.RGBBytesPerPixel;
				x++;
				r += irslope;
				g += igslope;
				b += ibslope;
				count--;
			} while (count > 0);
		}
		pspan++;
	}
	return 1;
}

int DrawFlatTexturedPolygon(const vertex2D *verts, int nVerts, u_short z) {
	int i, j, topvert, bottomvert, leftvert, rightvert, nextvert;
	int itopy, ibottomy, spantopy, spanbottomy, count;
	int x, u, v;
	int ixslope, iuslope, ivslope;
	float topy, bottomy, height, width, prestep;
	float xslope, uslope, vslope;
	float y;
	span_t *pspan;

	if (myRenDev.pRGB == NULL)
		return 0;
	if (myTexHan.pRGBA[mip_map_level] == NULL)
		return 0;

	// Test for clockwise polygons
	int l0x = (verts[0].x - verts[1].x) >> 16;
	int l0y = (verts[0].y - verts[1].y) >> 16;
	int l1x = (verts[2].x - verts[1].x) >> 16;
	int l1y = (verts[2].y - verts[1].y) >> 16;
	if (l0x * l1y > l0y * l1x) {
		return 0;
	}

	// u_char a0 = GETAValue(verts[0].colour);
	u_char r0 = GETRValue(verts[0].colour);
	u_char g0 = GETGValue(verts[0].colour);
	u_char b0 = GETBValue(verts[0].colour);

	topy = 999999.0f;
	bottomy = -999999.0f;
	topvert = 0;
	bottomvert = 0;
	for (i = 0; i < nVerts; i++) {
		y = (float)((verts[i].y) / 65536.0f);
		if (y < topy) {
			topy = y;
			topvert = i;
		}
		if (y > bottomy) {
			bottomy = y;
			bottomvert = i;
		}
	}

	itopy = (int)ceil(topy);
	ibottomy = (int)ceil(bottomy);

	if (ibottomy == itopy)
		return 1; // reject polygons that don't cross a scan

	// Scan out the left edge
	pspan = spans;
	leftvert = topvert;

	do {
		nextvert = leftvert - 1;
		if (nextvert < 0)
			nextvert = nVerts - 1;
		y = (float)((verts[leftvert].y) / 65536.0f);
		spantopy = (int)ceil(y);
		y = (float)((verts[nextvert].y) / 65536.0f);
		spanbottomy = (int)ceil(y);
		if (spantopy < spanbottomy) {
			height = (float)((verts[nextvert].y - verts[leftvert].y) / 65536.0f);
			width = (float)((verts[nextvert].x - verts[leftvert].x) / 65536.0f);

			xslope = width / height;
			uslope = (float)((verts[nextvert].u - verts[leftvert].u) / 65536.0f) / height;
			vslope = (float)((verts[nextvert].v - verts[leftvert].v) / 65536.0f) / height;

			prestep = spantopy - (float)((verts[leftvert].y) / 65536.0f);

			x = (int)((((verts[leftvert].x) / 65536.0f) + (xslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			u = (int)((((verts[leftvert].u) / 65536.0f) + (uslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			v = (int)((((verts[leftvert].v) / 65536.0f) + (vslope * prestep)) * 65536.0) + ((1 << 16) - 1);

			ixslope = (int)(xslope * 65536.0);
			iuslope = (int)(uslope * 65536.0);
			ivslope = (int)(vslope * 65536.0);

			for (j = spantopy; j < spanbottomy; j++) {
				pspan->x0 = x >> 16;
				pspan->u0 = u >> 16;
				pspan->v0 = v >> 16;
				x += ixslope;
				u += iuslope;
				v += ivslope;
				pspan++;
			}
		}

		leftvert--;
		if (leftvert < 0)
			leftvert = nVerts - 1;
	} while (leftvert != bottomvert);

	// Scan out the right edge
	pspan = spans;
	rightvert = topvert;

	do {
		nextvert = (rightvert + 1) % nVerts;
		y = (float)((verts[rightvert].y) / 65536.0f);
		spantopy = (int)ceil(y);
		y = (float)((verts[nextvert].y) / 65536.0f);
		spanbottomy = (int)ceil(y);
		if (spantopy < spanbottomy) {
			height = (float)((verts[nextvert].y - verts[rightvert].y) / 65536.0f);
			width = (float)((verts[nextvert].x - verts[rightvert].x) / 65536.0f);

			xslope = width / height;
			uslope = (float)((verts[nextvert].u - verts[rightvert].u) / 65536.0f) / height;
			vslope = (float)((verts[nextvert].v - verts[rightvert].v) / 65536.0f) / height;

			prestep = spantopy - (float)((verts[rightvert].y) / 65536.0f);

			x = (int)((((verts[rightvert].x) / 65536.0f) + (xslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			u = (int)((((verts[rightvert].u) / 65536.0f) + (uslope * prestep)) * 65536.0) + ((1 << 16) - 1);
			v = (int)((((verts[rightvert].v) / 65536.0f) + (vslope * prestep)) * 65536.0) + ((1 << 16) - 1);

			ixslope = (int)(xslope * 65536.0);
			iuslope = (int)(uslope * 65536.0);
			ivslope = (int)(vslope * 65536.0);

			for (j = spantopy; j < spanbottomy; j++) {
				pspan->x1 = x >> 16;
				pspan->u1 = u >> 16;
				pspan->v1 = v >> 16;
				x += ixslope;
				u += iuslope;
				v += ivslope;
				pspan++;
			}
		}

		rightvert = (rightvert + 1) % nVerts;
	} while (rightvert != bottomvert);

	// Draw the spans
	pspan = spans;

	int mipw = myTexHan.w >> mip_map_level;
	int miph = myTexHan.h >> mip_map_level;

	for (i = itopy; i < ibottomy; i++) {
		count = pspan->x1 - pspan->x0;
		if (count > 0) {
			x = pspan->x0;
			u = (pspan->u0 << 8);
			v = (pspan->v0 << 8);

			iuslope = ((pspan->u1 << 8) - u) / count;
			ivslope = ((pspan->v1 << 8) - v) / count;

			char *left = myRenDev.pRGB + (myRenDev.RGBPitch * i) + myRenDev.RGBBytesPerPixel * x;
			char *zleft = myRenDev.pZ + (myRenDev.ZPitch * i) + myRenDev.ZBytesPerPixel * pspan->x0;
			do {
				int pu = (u >> (8 + mip_map_level));
				int pv = (v >> (8 + mip_map_level));

				if (pu < 0)
					pu = 0;
				if (pu >= mipw)
					pu = mipw - 1;

				if (pv < 0)
					pv = 0;
				if (pv >= miph)
					pv = miph - 1;

				u_int toff = (pu + (pv * mipw)) * myTexHan.bpp;
				u_char *texel = ((u_char *)(myTexHan.pRGBA[mip_map_level])) + toff;

				// RGB data
				int ta, tr, tg, tb;
				if (myTexHan.bpp > 3) {
					ta = *(texel + 3);
					tr = *(texel + 2);
					tg = *(texel + 1);
					tb = *(texel + 0);
				} else {
					// Palette data
					u_char index = *texel;
					u_int colour = myTexHan.palette[index];
					ta = (u_char)((colour >> 24) & 0xFF);
					tr = (u_char)((colour >> 16) & 0xFF);
					tg = (u_char)((colour >> 8) & 0xFF);
					tb = (u_char)((colour >> 0) & 0xFF);
				}
				// BGR : 128 = scale of 1.0
				// int a = a0;
				int r = r0;
				int g = g0;
				int b = b0;

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

				*(left + 0) = (char)b;
				*(left + 1) = (char)g;
				*(left + 2) = (char)r;
				*(left + 3) = (char)ta; // use the texture alpha value

				*(u_short *)(zleft + 0) = z;
				left += myRenDev.RGBBytesPerPixel;
				zleft += myRenDev.ZBytesPerPixel;
				x++;
				u += iuslope;
				v += ivslope;
				count--;
			} while (count > 0);
		}
		pspan++;
	}
	return 1;
}

#endif

} // End of namespace ICB
