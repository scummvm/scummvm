/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/gfx/psx_pcgpu.h"
#include "engines/icb/gfx/psx_poly.h"
#include "engines/icb/gfx/gfxstub.h"

#include "common/textconsole.h"

namespace ICB {

// Defaults for the OT list
#define UNLINKED_ADDR (reinterpret_cast<void *>(static_cast<uintptr>(0xDEADBEAF)))
#define UNLINKED_LEN (0x6666)

// For storing user data in the OT entry e.g. texture pointer
void *OTusrData;

// No ABR support : at the moment
uint16 psxTP;

// The emulation of VRAM : 16-bit pixels 1024x512 big
uint16 psxVRAM[VRAM_WIDTH * VRAM_HEIGHT];

// Set VRAM to a certain colour
int32 ClearImage(RECT16 *rect, uint8 r, uint8 g, uint8 b) {
	uint16 colour;
	int32 x, y;
	int32 i;

	// Convert 24-bit colour to 15-bit
	colour = (uint16)((r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10));

	for (y = rect->y; y < rect->y + rect->h; y++) {
		for (x = rect->x; x < rect->x + rect->w; x++) {
			i = x + y * VRAM_WIDTH;
			psxVRAM[i] = colour;
		}
	}
	return 1;
}

// Fill VRAM with data
uint16 LoadClut(uint32 *clut, int32 x, int32 y) {
	RECT16 rect;
	setRECT(&rect, (int16)x, (int16)y, 256, 1);
	LoadImage(&rect, clut);
	return (uint16)getClut(x, y);
}

// Fill VRAM with data
int32 LoadImage(RECT16 *rect, uint32 *p) {
	int32 x, y, i;
	uint16 *p16 = (uint16 *)p;

	for (y = rect->y; y < rect->y + rect->h; y++) {
		for (x = rect->x; x < rect->x + rect->w; x++) {
			i = x + y * VRAM_WIDTH;
			psxVRAM[i] = *p16;
			p16++;
		}
	}
	return 1;
}

// Move data around within VRAM
int32 MoveImage(RECT16 *rect, int32 x, int32 y) {
	int32 x0, y0, i0;
	int32 x1, y1, i1;

	y1 = y;
	for (y0 = rect->y; y0 < rect->y + rect->h; y0++) {
		x1 = x;
		for (x0 = rect->x; x0 < rect->x + rect->w; x0++) {
			i0 = x0 + y0 * VRAM_WIDTH;
			i1 = x1 + y1 * VRAM_WIDTH;
			psxVRAM[i1] = psxVRAM[i0];
			x1++;
		}
		y1++;
	}
	return 1;
}

// Setup the linked list for the OT tags from back to the front
OT_tag *ClearOTagR(OT_tag *ot, uint32 size) {
	int32 i = size - 1;
	while (i > 0) {
		ot[i].addr = (void *)&ot[i - 1];
		ot[i].len = UNLINKED_LEN;
		i--;
	}
	ot[0].addr = UNLINKED_ADDR;
	ot[0].len = UNLINKED_LEN;
	return ot;
}

// Setup the linked list for the OT tags from front to back
OT_tag *ClearOTag(OT_tag *ot, uint32 size) {
	uint32 i = 0;
	while (i < (size - 1)) {
		ot[i].addr = (void *)&ot[i + 1];
		ot[i].len = UNLINKED_LEN;

		i++;
	}
	ot[size - 1].addr = UNLINKED_ADDR;
	ot[size - 1].len = UNLINKED_LEN;
	return ot;
}

int32 nPrims;

// Draw the damm things
void DrawOTag(OT_tag *ot) {
	OT_tag *prim = ot;
	nPrims = 0;

	while (prim->addr != UNLINKED_ADDR) {
		if (prim->len != UNLINKED_LEN) {
			DrawPrim((void *)prim);
		}
		// Move to the next link in the chain
		prim = (OT_tag *)(prim->addr);
	}
}

// Draw the base primitive

// Should also be checking the lengths to make sure we haven't
// got corrupt data
void DrawPrim(void *prim) {
	P_TAG *p = (P_TAG *)prim;
	uint32 code = p->code;
	uint32 len = p->len;
	uint16 z0 = p->z0;
	void *usr = p->usr;
	uint8 alpha = 0x00;

	// Catch special primitives
	// DR_TPAGE
	if ((code & 0xe1) == 0xe1) {
		psxTP = (uint16)(*((uint32 *)&(p->r0)) & 0xFFFF);
		len -= GPUSIZE_DR_TPAGE; // we have used one word of data up
		// Move the pointer onto the correct place
		p++; // move past the P_TAG

		// handle multiple primitives in one block
		code = p->code;
		if (len > 0) {
			len -= GPUSIZE_TAG;
		}

		// Get the base alpha value
		alpha = ((P_HEADER *)(p))->code;
		// Modify that based on tbe ABR value
		uint8 abr = (uint8)((psxTP >> 5) & 0x3);
		if (abr == 0) {
			// alpha is 0-64, so 32 is average
			alpha = 0xC0 | 0x20; // average of background + foreground
		} else if (abr == 1)
			alpha = 0x40; // additive
		else if (abr == 2)
			alpha = 0x80; // subtractive
		else if (abr == 3)
			// alpha is 0-64 : no error checking
			alpha |= 0xC0; // alpha blend
		else
			alpha = 0x00; // ignore any other modes
	}

	nPrims++;
	// Decode the primitive type and then respond appropiately
	// Mask off the primitive modifiers (bottom two bits)
	// bit 0 = shade text
	// bit 1 = semi-trans
	switch (code & 0xFC) {
	// Flat Coloured Rectangle
	// TILE: code:0x60 length:4
	case GPUCODE_TILE: {
		TILE *pr = (TILE *)p;
		if (len != GPUSIZE_TILE) {
			warning("Primitive %p length %d != TILE length %d\n", prim, (uint32)len, GPUSIZE_TILE);
			return;
		}
		DrawTile(pr->x0, pr->y0, pr->w, pr->h, pr->r0, pr->g0, pr->b0, alpha, z0);
		break;
	}
	// Flat Coloured Rectangle 1 pixel width & height
	// TILE_1: code:0x68 length:3
	case GPUCODE_TILE_1: {
		TILE_1 *pr = (TILE_1 *)p;
		if (len != GPUSIZE_TILE_1) {
			warning("Primitive %p length %d != TILE_1 length %d\n", prim, (uint32)len, GPUSIZE_TILE_1);
			return;
		}
		DrawTile(pr->x0, pr->y0, 1, 1, pr->r0, pr->g0, pr->b0, alpha, z0);
		break;
	}
	// Flat Coloured Rectangle 8 pixel width & height
	// TILE_8: code:0x70 length:3
	case GPUCODE_TILE_8: {
		TILE_8 *pr = (TILE_8 *)p;
		if (len != GPUSIZE_TILE_8) {
			warning("Primitive %p length %d != TILE_8 length %d\n", prim, (uint32)len, GPUSIZE_TILE_8);
			return;
		}
		DrawTile(pr->x0, pr->y0, 8, 8, pr->r0, pr->g0, pr->b0, alpha, z0);
		break;
	}
	// Flat Coloured Rectangle 16 pixel width & height
	// TILE_16: code:0x78 length:3
	case GPUCODE_TILE_16: {
		TILE_16 *pr = (TILE_16 *)p;
		if (len != GPUSIZE_TILE_16) {
			warning("Primitive %p length %d != TILE_16 length %d\n", prim, (uint32)len, GPUSIZE_TILE_16);
			return;
		}
		DrawTile(pr->x0, pr->y0, 16, 16, pr->r0, pr->g0, pr->b0, alpha, z0);
		break;
	}
	// Flat Coloured single line
	// LineF2 = code: 0x40 length:4
	case GPUCODE_LINE_F2: {
		LINE_F2 *pr = (LINE_F2 *)p;
		if (len != GPUSIZE_LINE_F2) {
			warning("Primitive %p length %d != LineF2 length %d\n", prim, (uint32)len, GPUSIZE_LINE_F2);
			return;
		}
		/*
		         printf( "LineF2: (%d,%d) (%d,%d) RGB0:%d %d %d\n",
		                 pr->x0, pr->y0, pr->x1, pr->y1, pr->r0, pr->g0, pr->b0 );
		*/
		DrawLineF2(pr->x0, pr->y0, pr->x1, pr->y1, pr->r0, pr->g0, pr->b0, alpha, z0);
		break;
	}
	// Flat Coloured double line (not closed)
	// LineF3 = code: 0x48 length:6
	case GPUCODE_LINE_F3: {
		LINE_F3 *pr = (LINE_F3 *)p;
		if (len != GPUSIZE_LINE_F3) {
			warning("Primitive %p length %d != LineF3 length %d\n", prim, (uint32)len, GPUSIZE_LINE_F3);
			return;
		}
		/*
		         printf( "LineF3: (%d,%d) (%d,%d) (%d,%d) RGB0:%d %d %d\n",
		                 pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2,
		                 pr->r0, pr->g0, pr->b0 );
		*/
		DrawLineF3(pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2, pr->r0, pr->g0, pr->b0, alpha, z0);
		break;
	}
	// Flat Coloured triple line (not closed)
	// LineF4 = code: 0x4c length:7
	case GPUCODE_LINE_F4: {
		LINE_F4 *pr = (LINE_F4 *)p;
		if (len != GPUSIZE_LINE_F4) {
			warning("Primitive %p length %d != LineF4 length %d\n", prim, (uint32)len, GPUSIZE_LINE_F4);
			return;
		}
		/*
		         printf( "LineF4: (%d,%d) (%d,%d) (%d,%d) (%d %d) RGB0:%d %d %d\n",
		                 pr->x0, pr->y0, pr->x1, pr->y1,
		                 pr->x2, pr->y2, pr->x3, pr->y3,
		                 pr->r0, pr->g0, pr->b0 );
		*/
		DrawLineF4(pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2, pr->x3, pr->y3, pr->r0, pr->g0, pr->b0, alpha, z0);
		break;
	}

	// Gouraud Coloured single line
	// LineG2 = code: 0x50 length:5
	case GPUCODE_LINE_G2: {
		LINE_G2 *pr = (LINE_G2 *)p;
		if (len != GPUSIZE_LINE_G2) {
			warning("Primitive %p length %d != LineG2 length %d\n", prim, (uint32)len, GPUSIZE_LINE_G2);
			return;
		}
		/*
		         printf( "LineG2: (%d,%d) (%d,%d) RGB0:%d %d %d RGB1:%d %d %d\n",
		                 pr->x0, pr->y0, pr->x1, pr->y1,
		                 pr->r0, pr->g0, pr->b0, pr->r1, pr->g1, pr->b1 );
		*/
		DrawLineG2(pr->x0, pr->y0, pr->x1, pr->y1, pr->r0, pr->g0, pr->b0, pr->r1, pr->g1, pr->b1, alpha, z0);
		break;
	}
	// Gouraud Coloured double line (not closed)
	// LineG3 = code: 0x58 length:8
	case GPUCODE_LINE_G3: {
		LINE_G3 *pr = (LINE_G3 *)p;
		if (len != GPUSIZE_LINE_G3) {
			warning("Primitive %p length %d != LineG3 length %d\n", prim, (uint32)len, GPUSIZE_LINE_G3);
			return;
		}
		/*
		         printf( "LineG3: (%d,%d) (%d,%d) (%d,%d) RGB0:%d %d %d RGB1:%d %d %d\n",
		                 pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2,
		                 pr->r0, pr->g0, pr->b0, pr->r1, pr->g1, pr->b1 );
		*/
		DrawLineG3(pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2, pr->r0, pr->g0, pr->b0, pr->r1, pr->g1, pr->b1, pr->r2, pr->g2, pr->b2, alpha, z0);
		break;
	}
	// Gouraud Coloured triple line (not closed)
	// LineG4 = code: 0x5c length:10
	case GPUCODE_LINE_G4: {
		LINE_G4 *pr = (LINE_G4 *)p;
		if (len != GPUSIZE_LINE_G4) {
			warning("Primitive %p length %d != LineG4 length %d\n", prim, (uint32)len, GPUSIZE_LINE_G4);
			return;
		}
		/*
		         printf( "LineG4: (%d,%d) (%d,%d) (%d,%d) (%d %d) \
		RGB0:%d %d %d RGB1:%d %d %d RGB2:%d %d %d RGB3:%d %d %d\n",
		                 pr->x0, pr->y0, pr->x1, pr->y1,
		                 pr->x2, pr->y2, pr->x3, pr->y3,
		                 pr->r0, pr->g0, pr->b0, pr->r1, pr->g1, pr->b1,
		                 pr->r2, pr->g2, pr->b2, pr->r3, pr->g3, pr->b3 );
		*/
		DrawLineG4(pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2, pr->x3, pr->y3, pr->r0, pr->g0, pr->b0, pr->r1, pr->g1, pr->b1, pr->r2, pr->g2, pr->b2, pr->r3, pr->g3,
		           pr->b3, alpha, z0);
		break;
	}

	// Flat Coloured Non-textured triangle
	// PolyF3 = code:0x20 length:5
	case GPUCODE_POLY_F3: {
		POLY_F3 *pr = (POLY_F3 *)p;
		if (len != GPUSIZE_POLY_F3) {
			warning("Primitive %p length %d != PolyF3 length %d\n", prim, (uint32)len, GPUSIZE_POLY_F3);
			return;
		}
		/*
		         printf( "PolyF3: (%d,%d) (%d,%d) (%d,%d) RGB0:%d %d %d\n",
		                 pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2,
		                 pr->r0, pr->g0, pr->b0 );
		*/
		DrawFlatTriangle(pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2, pr->r0, pr->g0, pr->b0, alpha, z0);
		break;
	}

	// Flat Coloured Non-textured quad
	// PolyF4 = code:0x28 length:6
	case GPUCODE_POLY_F4: {
		POLY_F4 *pr = (POLY_F4 *)p;
		if (len != GPUSIZE_POLY_F4) {
			warning("Primitive %p length %d != PolyF4 length %d\n", prim, (uint32)len, GPUSIZE_POLY_F4);
			return;
		}
		/*
		         printf( "PolyF4: (%d,%d) (%d,%d) (%d,%d) (%d %d) RGB0:%d %d %d\n",
		                 pr->x0, pr->y0, pr->x1, pr->y1,
		                 pr->x2, pr->y2, pr->x3, pr->y3,
		                 pr->r0, pr->g0, pr->b0 );
		*/
		DrawFlatQuad(pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2, pr->x3, pr->y3, pr->r0, pr->g0, pr->b0, alpha, z0);
		break;
	}

	// Gouraud Coloured Non-textured triangle
	// PolyG3 = code:0x30 length:7
	case GPUCODE_POLY_G3: {
		POLY_G3 *pr = (POLY_G3 *)p;
		if (len != GPUSIZE_POLY_G3) {
			warning("Primitive %p length %d != PolyG3 length %d\n", prim, (uint32)len, GPUSIZE_POLY_G3);
			return;
		}
		/*
		         printf( "PolyG3: (%d,%d) (%d,%d) (%d,%d) \
		RGB0:%d %d %d RGB1:%d %d %d RGB2:%d %d %d\n",
		                 pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2,
		                 pr->r0, pr->g0, pr->b0, pr->r1, pr->g1, pr->b1,
		                 pr->r2, pr->g2, pr->b2 );
		*/
		DrawGouraudTriangle(pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2, pr->r0, pr->g0, pr->b0, pr->r1, pr->g1, pr->b1, pr->r2, pr->g2, pr->b2, alpha, z0);
		break;
	}
	// Gouraud Coloured Non-textured quad
	// PolyG4 = code:0x38 length:9
	case GPUCODE_POLY_G4: {
		POLY_G4 *pr = (POLY_G4 *)p;
		if (len != GPUSIZE_POLY_G4) {
			warning("Primitive %p length %d != PolyG4 length %d\n", prim, (uint32)len, GPUSIZE_POLY_G4);
			return;
		}
		/*
		         printf( "PolyG4: (%d,%d) (%d,%d) (%d,%d) (%d %d) \
		RGB0:%d %d %d RGB1:%d %d %d RGB2:%d %d %d RGB3:%d %d %d\n",
		                 pr->x0, pr->y0, pr->x1, pr->y1,
		                 pr->x2, pr->y2, pr->x3, pr->y3,
		                 pr->r0, pr->g0, pr->b0, pr->r1, pr->g1, pr->b1,
		                 pr->r2, pr->g2, pr->b2, pr->r3, pr->g3, pr->b3 );
		*/
		DrawGouraudQuad(pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2, pr->x3, pr->y3, pr->r0, pr->g0, pr->b0, pr->r1, pr->g1, pr->b1, pr->r2, pr->g2, pr->b2, pr->r3,
		                pr->g3, pr->b3, alpha, z0);
		break;
	}
	// Flat Coloured Textured triangle
	// PolyFT3 = code:0x24 length:8
	case GPUCODE_POLY_FT3: {
		POLY_FT3 *pr = (POLY_FT3 *)p;
		if (len != GPUSIZE_POLY_FT3) {
			warning("Primitive %p length %d != PolyFT3 length %d\n", prim, (uint32)len, GPUSIZE_POLY_FT3);
			return;
		}
		/*
		         printf( "PolyFT3: (%d,%d) (%d,%d) (%d,%d)\n(%d,%d) (%d,%d) (%d,%d)\nRGB0:%d %d %d\n",
		                 pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2,
		                 pr->u0, pr->v0, pr->u1, pr->v1, pr->u2, pr->v2,
		                 pr->r0, pr->g0, pr->b0 );
		*/
		DrawFlatTriangleTextured(pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2, pr->r0, pr->g0, pr->b0, pr->u0, pr->v0, pr->u1, pr->v1, pr->u2, pr->v2, alpha, z0, usr);
		break;
	}
	// Flat Coloured Textured quad
	// PolyFT4 = code:0x2c length:10
	case GPUCODE_POLY_FT4: {
		POLY_FT4 *pr = (POLY_FT4 *)p;
		if (len != GPUSIZE_POLY_FT4) {
			warning("Primitive %p length %d != PolyFT4 length %d\n", prim, (uint32)len, GPUSIZE_POLY_FT4);
			return;
		}
		/*
		         printf( "PolyFT4: (%d,%d) (%d,%d) (%d,%d) (%d,%d)\n(%d,%d) (%d,%d) (%d,%d) (%d,%d)\nRGB0:%d %d %d\n",
		                 pr->x0, pr->y0, pr->x1, pr->y1,
		                 pr->x2, pr->y2, pr->x3, pr->y3,
		                 pr->u0, pr->v0, pr->u1, pr->v1,
		                 pr->u2, pr->u2, pr->u3, pr->u3,
		                 pr->r0, pr->g0, pr->b0 );
		*/

		DrawFlatQuadTextured(pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2, pr->x3, pr->y3, pr->r0, pr->g0, pr->b0, pr->u0, pr->v0, pr->u1, pr->v1, pr->u2, pr->v2, pr->u3,
		                     pr->v3, alpha, z0, usr);
		break;
	}
	// Gouraud Coloured Textured triangle
	// PolyGT3 = code:0x34 length:10
	case GPUCODE_POLY_GT3: {
		POLY_GT3 *pr = (POLY_GT3 *)p;
		if (len != GPUSIZE_POLY_GT3) {
			warning("Primitive %p length %d != PolyGT3 length %d\n", prim, (uint32)len, GPUSIZE_POLY_GT3);
			return;
		}
		/*
		         printf( "PolyGT3: (%d,%d) (%d,%d) (%d,%d)\n(%d,%d) (%d,%d) (%d,%d)\nRGB0:%d %d %d RGB1:%d %d %d RGB2:%d %d %d\n",
		                 pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2,
		                 pr->u0, pr->v0, pr->u1, pr->v1, pr->u2, pr->v2,
		                 pr->r0, pr->g0, pr->b0, pr->r1, pr->g1, pr->b1,
		                 pr->r2, pr->g2, pr->b2 );
		*/

		DrawGouraudTriangleTextured(pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2, pr->r0, pr->g0, pr->b0, pr->r1, pr->g1, pr->b1, pr->r2, pr->g2, pr->b2, pr->u0, pr->v0,
		                            pr->u1, pr->v1, pr->u2, pr->v2, alpha, z0, usr);
		break;
	}
	// Gouraud Coloured Textured quad
	// PolyGT4 = code:0x3c length:13
	case GPUCODE_POLY_GT4: {
		POLY_GT4 *pr = (POLY_GT4 *)p;
		if (len != GPUSIZE_POLY_GT4) {
			warning("Primitive %p length %d != PolyGT4 length %d\n", prim, (uint32)len, GPUSIZE_POLY_GT4);
			return;
		}
		/*
		         printf( "PolyGT4: (%d,%d) (%d,%d) (%d,%d) (%d,%d)\n(%d,%d) (%d,%d) (%d,%d) (%d,%d)\nRGB0:%d %d %d RGB1:%d %d %d RGB2:%d %d $d RGB3:%d %d %d\n",
		                 pr->x0, pr->y0, pr->x1, pr->y1,
		                 pr->x2, pr->y2, pr->x3, pr->y3,
		                 pr->u0, pr->v0, pr->u1, pr->v1,
		                 pr->u2, pr->u2, pr->u3, pr->u3,
		                 pr->r0, pr->g0, pr->b0,
		                 pr->r1, pr->g1, pr->b1,
		                 pr->r2, pr->g2, pr->b2,
		                 pr->r3, pr->g3, pr->b3 );
		*/
		DrawGouraudQuadTextured(pr->x0, pr->y0, pr->x1, pr->y1, pr->x2, pr->y2, pr->x3, pr->y3, pr->r0, pr->g0, pr->b0, pr->r1, pr->g1, pr->b1, pr->r2, pr->g2, pr->b2,
		                        pr->r3, pr->g3, pr->b3, pr->u0, pr->v0, pr->u1, pr->v1, pr->u2, pr->v2, pr->u3, pr->v3, alpha, z0, usr);
		break;
	}
	// Flat Coloured Textured Tile
	// SPRT = code:0x64 length:5
	case GPUCODE_SPRT: {
		SPRT *pr = (SPRT *)p;
		if (len != GPUSIZE_SPRT) {
			warning("Primitive %p length %d != SPRT length %d\n", prim, (uint32)len, GPUSIZE_SPRT);
			return;
		}
		DrawSprite(pr->x0, pr->y0, pr->w, pr->h, pr->r0, pr->g0, pr->b0, pr->u0, pr->v0, alpha, z0, usr);
		break;
	}
	// Flat Coloured Textured Tile 8 pixels x 8 pixels
	// SPRT_8 = code:0x74 length:4
	case GPUCODE_SPRT_8: {
		SPRT_8 *pr = (SPRT_8 *)p;
		if (len != GPUSIZE_SPRT_8) {
			warning("Primitive %p length %d != SPRT_8 length %d\n", prim, (uint32)len, GPUSIZE_SPRT_8);
			return;
		}
		DrawSprite(pr->x0, pr->y0, 8, 8, pr->r0, pr->g0, pr->b0, pr->u0, pr->v0, alpha, z0, usr);
		break;
	}
	// Flat Coloured Textured Tile 16 pixels x 16 pixels
	// SPRT_16 = code:0x7c length:4
	case GPUCODE_SPRT_16: {
		SPRT_16 *pr = (SPRT_16 *)p;
		if (len != GPUSIZE_SPRT_16) {
			warning("Primitive %p length %d != SPRT_16 length %d\n", prim, (uint32)len, GPUSIZE_SPRT_16);
			return;
		}
		DrawSprite(pr->x0, pr->y0, 16, 16, pr->r0, pr->g0, pr->b0, pr->u0, pr->v0, alpha, z0, usr);
		break;
	}
	default: {
		warning("Unknown Primitive\n");
		break;
	}
	}
}

} // End of namespace ICB
