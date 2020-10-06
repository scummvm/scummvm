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

#ifndef ICB_DEANPOLY_H
#define ICB_DEANPOLY_H

namespace ICB {

#define GPUCODE_MODE_SHADE_TEX (1 << 0)
#define GPUCODE_MODE_SEMI_TRANS (1 << 1)

// GPU Primitive codes. Please remember that 3 and 4 point lines require the pad field setting to 0x55555555.
#define GPUCODE_POLY_F3 (0x20)
#define GPUCODE_POLY_FT3 (0x24)
#define GPUCODE_POLY_G3 (0x30)
#define GPUCODE_POLY_GT3 (0x34)
#define GPUCODE_POLY_F4 (0x28)
#define GPUCODE_POLY_FT4 (0x2c)
#define GPUCODE_POLY_G4 (0x38)
#define GPUCODE_POLY_GT4 (0x3c)

#define GPUCODE_SPRT (0x64)
#define GPUCODE_SPRT_8 (0x74)
#define GPUCODE_SPRT_16 (0x7c)

#define GPUCODE_TILE (0x60)
#define GPUCODE_TILE_1 (0x68)
#define GPUCODE_TILE_8 (0x70)
#define GPUCODE_TILE_16 (0x78)

#define GPUCODE_LINE_F2 (0x40)
#define GPUCODE_LINE_G2 (0x50)
#define GPUCODE_LINE_F3 (0x48)
#define GPUCODE_LINE_G3 (0x58)
#define GPUCODE_LINE_F4 (0x4c)
#define GPUCODE_LINE_G4 (0x5c)

#define GPUSIZE_POLY_F3 (4 + 3)
#define GPUSIZE_POLY_FT3 (7 + 3)
#define GPUSIZE_POLY_G3 (6 + 3)
#define GPUSIZE_POLY_GT3 (9 + 3)
#define GPUSIZE_POLY_F4 (5 + 4)
#define GPUSIZE_POLY_FT4 (9 + 4)
#define GPUSIZE_POLY_G4 (8 + 4)
#define GPUSIZE_POLY_GT4 (12 + 4)

#define GPUSIZE_SPRT (4 + 1)
#define GPUSIZE_SPRT_8 (3 + 1)
#define GPUSIZE_SPRT_16 (3 + 1)

#define GPUSIZE_TILE (3 + 1)
#define GPUSIZE_TILE_1 (2 + 1)
#define GPUSIZE_TILE_8 (2 + 1)
#define GPUSIZE_TILE_16 (2 + 1)

#define GPUSIZE_LINE_F2 (3 + 2)
#define GPUSIZE_LINE_G2 (4 + 2)
#define GPUSIZE_LINE_F3 (5 + 3)
#define GPUSIZE_LINE_G3 (7 + 3)
#define GPUSIZE_LINE_F4 (6 + 4)
#define GPUSIZE_LINE_G4 (9 + 4)

#define GPUSIZE_DR_TPAGE (1)
#define GPUSIZE_TAG (4)

#define GPUSIZE_TPOLY_F3 (GPUSIZE_DR_TPAGE + GPUSIZE_POLY_F3 + GPUSIZE_TAG)
#define GPUSIZE_TPOLY_F4 (GPUSIZE_DR_TPAGE + GPUSIZE_POLY_F4 + GPUSIZE_TAG)
#define GPUSIZE_TPOLY_G3 (GPUSIZE_DR_TPAGE + GPUSIZE_POLY_G3 + GPUSIZE_TAG)
#define GPUSIZE_TPOLY_G4 (GPUSIZE_DR_TPAGE + GPUSIZE_POLY_G4 + GPUSIZE_TAG)

#define GPUSIZE_TLINE_F2 (GPUSIZE_DR_TPAGE + GPUSIZE_LINE_F3 + GPUSIZE_TAG)
#define GPUSIZE_TLINE_G2 (GPUSIZE_DR_TPAGE + GPUSIZE_LINE_G2 + GPUSIZE_TAG)
#define GPUSIZE_TLINE_F3 (GPUSIZE_DR_TPAGE + GPUSIZE_LINE_F3 + GPUSIZE_TAG)
#define GPUSIZE_TLINE_G3 (GPUSIZE_DR_TPAGE + GPUSIZE_LINE_G3 + GPUSIZE_TAG)
#define GPUSIZE_TLINE_F4 (GPUSIZE_DR_TPAGE + GPUSIZE_LINE_F4 + GPUSIZE_TAG)
#define GPUSIZE_TLINE_G4 (GPUSIZE_DR_TPAGE + GPUSIZE_LINE_G4 + GPUSIZE_TAG)

#define GPUSIZE_TSPRT (GPUSIZE_DR_TPAGE + GPUSIZE_SPRT + GPUSIZE_TAG)
#define GPUSIZE_TSPRT_8 (GPUSIZE_DR_TPAGE + GPUSIZE_SPRT_8 + GPUSIZE_TAG)
#define GPUSIZE_TSPRT_16 (GPUSIZE_DR_TPAGE + GPUSIZE_SPRT_16 + GPUSIZE_TAG)

#define GPUSIZE_TTILE (GPUSIZE_DR_TPAGE + GPUSIZE_TILE + GPUSIZE_TAG)
#define GPUSIZE_TTILE_1 (GPUSIZE_DR_TPAGE + GPUSIZE_TILE_1 + GPUSIZE_TAG)
#define GPUSIZE_TTILE_8 (GPUSIZE_DR_TPAGE + GPUSIZE_TILE_8 + GPUSIZE_TAG)
#define GPUSIZE_TTILE_16 (GPUSIZE_DR_TPAGE + GPUSIZE_TILE_16 + GPUSIZE_TAG)

#define setTcode(p, tc) (*(uint32 *)&(((P_TAG *)(p))->r0) = (uint32)(tc))

#define zeroTagPoly(p) (setaddr((&(p)->tag_poly), 0), setlen((&(p)->tag_poly), 0) = 0)

#define setlen(p, _len) (((P_TAG *)(p))->len = (u_char)(_len))

#define setTDrawTPageSize(p, s) setlen((p), (s)), setTcode((p), _get_mode(0, 1, 0))

#define setTDrawTPage(p) setTDrawTPageSize((p), 1)

#define setTPolyF3(p) setTDrawTPageSize((p), GPUSIZE_TPOLY_F3), setcode(&((p)->tag_poly), GPUCODE_POLY_F3), zeroTagPoly((p))
#define setTPolyF4(p) setTDrawTPageSize((p), GPUSIZE_TPOLY_F4), setcode(&((p)->tag_poly), GPUCODE_POLY_F4), zeroTagPoly((p))

#define setTPolyG3(p) setTDrawTPageSize((p), GPUSIZE_TPOLY_G3), setcode(&((p)->tag_poly), GPUCODE_POLY_G3), zeroTagPoly((p))
#define setTPolyG4(p) setTDrawTPageSize((p), GPUSIZE_TPOLY_G4), setcode(&((p)->tag_poly), GPUCODE_POLY_G4), zeroTagPoly((p))

#define setTLineF2(p) setTDrawTPageSize((p), GPUSIZE_TLINE_F2), setcode(&((p)->tag_poly), GPUCODE_LINE_F2), zeroTagPoly((p))
#define setTLineG2(p) setTDrawTPageSize((p), GPUSIZE_TLINE_G2), setcode(&((p)->tag_poly), GPUCODE_LINE_G2), zeroTagPoly((p))
#define setTLineF3(p) setTDrawTPageSize((p), GPUSIZE_TLINE_F3), setcode(&((p)->tag_poly), GPUCODE_LINE_F3), zeroTagPoly((p)), ((p)->pad) = 0x55555555
#define setTLineG3(p) setTDrawTPageSize((p), GPUSIZE_TLINE_G3), setcode(&((p)->tag_poly), GPUCODE_LINE_G3), zeroTagPoly((p)), ((p)->pad) = 0x55555555
#define setTLineF4(p) setTDrawTPageSize((p), GPUSIZE_TLINE_F4), setcode(&((p)->tag_poly), GPUCODE_LINE_F4), zeroTagPoly((p)), ((p)->pad) = 0x55555555
#define setTLineG4(p) setTDrawTPageSize((p), GPUSIZE_TLINE_G4), setcode(&((p)->tag_poly), GPUCODE_LINE_G4), zeroTagPoly((p)), ((p)->pad) = 0x55555555

#define setTSprt(p) setTDrawTPageSize((p), GPUSIZE_TSPRT), setcode(&((p)->tag_poly), GPUCODE_SPRT), zeroTagPoly((p))
#define setTSprt8(p) setTDrawTPageSize((p), GPUSIZE_TSPRT_8), setcode(&((p)->tag_poly), GPUCODE_SPRT_8), zeroTagPoly((p))
#define setTSprt16(p) setTDrawTPageSize((p), GPUSIZE_TSPRT_16), setcode(&((p)->tag_poly), GPUCODE_SPRT_16), zeroTagPoly((p))

#define setTTile(p) setTDrawTPageSize((p), GPUSIZE_TTILE), setcode(&((p)->tag_poly), GPUCODE_TILE), zeroTagPoly((p))
#define setTTile1(p) setTDrawTPageSize((p), GPUSIZE_TTILE_1), setcode(&((p)->tag_poly), GPUCODE_TILE_1), zeroTagPoly((p))
#define setTTile8(p) setTDrawTPageSize((p), GPUSIZE_TTILE_8), setcode(&((p)->tag_poly), GPUCODE_TILE_8), zeroTagPoly((p))
#define setTTile16(p) setTDrawTPageSize((p), GPUSIZE_TTILE_16), setcode(&((p)->tag_poly), GPUCODE_TILE_16), zeroTagPoly((p))

#define setTSemiTrans(p, abe) setSemiTrans(&((p)->tag_poly), (abe))
#define setTABRMode(p, abr) setTcode((p), _get_mode(0, 1, (abr << 5)))
#define setTSprtTPage(p, tp) setTcode((p), _get_mode(0, 1, (tp)))
#define setTSprtTPageABR(p, t, a) setTcode((p), _get_mode(0, 1, (((t)&0x19f) | ((a) << 5))))

#define addPrimSize(ot, p, size) (p)->tag = ((*(ot)) | ((size) << 24)), *((ot)) = (((uint32)(p) << 8) >> 8)

typedef struct __tpoly_f3 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
	int x1, y1;
	int x2, y2;
} TPOLY_F3; // Flat Triangle with ABR control

typedef struct __tpoly_f4 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
	int x1, y1;
	int x2, y2;
	int x3, y3;
} TPOLY_F4; // Flat Quadrangle with ABR control

typedef struct __tpoly_g3 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
	u_char r1, g1, b1, pad1;
	int x1, y1;
	u_char r2, g2, b2, pad2;
	int x2, y2;
} TPOLY_G3; // Gouraud Triangle with ABR control

typedef struct __tpoly_g4 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
	u_char r1, g1, b1, pad1;
	int x1, y1;
	u_char r2, g2, b2, pad2;
	int x2, y2;
	u_char r3, g3, b3, pad3;
	int x3, y3;
} TPOLY_G4; // Gouraud Quadrangle with ABR control

typedef struct __tline_f2 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
	int x1, y1;
} TLINE_F2; // Unconnected Flat Line with ABR control

typedef struct __tline_g2 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
	u_char r1, g1, b1, p1;
	int x1, y1;
} TLINE_G2; // Unconnected Gouraud Line with ABR control

typedef struct __tline_f3 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
	int x1, y1;
	int x2, y2;
	uint32 pad;
} TLINE_F3; // 2 connected Flat Line with ABR control

typedef struct __tline_g3 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
	u_char r1, g1, b1, p1;
	int x1, y1;
	u_char r2, g2, b2, p2;
	int x2, y2;
	uint32 pad;
} TLINE_G3; // 2 connected Gouraud Line with ABR control

typedef struct __tline_f4 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
	int x1, y1;
	int x2, y2;
	int x3, y3;
	uint32 pad;
} TLINE_F4; // 3 connected Flat Line Quadrangle with ABR control

typedef struct __tline_g4 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
	u_char r1, g1, b1, p1;
	int x1, y1;
	u_char r2, g2, b2, p2;
	int x2, y2;
	u_char r3, g3, b3, p3;
	int x3, y3;
	uint32 pad;
} TLINE_G4; // 3 connected Gouraud Line with ABR control

// Sprite Primitive Definitions
typedef struct __tsprt {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
	u_char u0, v0;
	u_short clut;
	short w, h;
} TSPRT; // Free size Sprite with TPage/ABR control

typedef struct __tsprt_16 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
	u_char u0, v0;
	u_short clut;
} TSPRT_16; // 16x16 Sprite with TPage/ABR control

typedef struct __tsprt_8 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
	u_char u0, v0;
	u_short clut;
} TSPRT_8; // 8x8 Sprite with TPage/ABR control

typedef struct __ttile {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
	short w, h;
} TTILE; // free size Tile with ABR control

typedef struct __ttile16 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
} TTILE_16; // 16x16 Tile with ABR control

typedef struct __ttile_8 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
} TTILE_8; // 8x8 Tile with ABR control

typedef struct __ttile_1 {
	P_TAG tag;
	OT_tag tag_poly;
	u_char r0, g0, b0, code;
	int x0, y0;
} TTILE_1; // 1x1 Tile with ABR control

} // End of namespace ICB

#endif // __DEANPOLY_H
