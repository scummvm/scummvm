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

#ifndef ICB_PSX_PCHMD_H
#define ICB_PSX_PCHMD_H

#include "engines/icb/gfx/psx_pcdefines.h"

namespace ICB {

const int32 HMD_FUS3 = 0x00000048; // Flat, Un-textured, Self-Luminous Triangle
const int32 HMD_FTS3 = 0x00000049; // Flat, Textured, Self-Luminous Triangle
const int32 HMD_GUS3 = 0x0000004c; // Gouraud, Un-textured, Self-Luminous Triangle
const int32 HMD_GTS3 = 0x0000004d; // Gouraud, Textured, Self-Luminous Triangle

const int32 HMD_FUL3 = 0x00000008; // Flat, Un-textured, Lit Triangle
const int32 HMD_FTL3 = 0x00000009; // Flat, Textured, Lit Triangle
const int32 HMD_GUL3 = 0x0000000c; // Gouraud, Un-textured, Lit Triangle
const int32 HMD_GTL3 = 0x0000000d; // Gouraud, Textured, Lit Triangle

const int32 TRIANGLE = 0x00000666; // Just 3 vertices for shadow polygon

const int32 HMD_FUS3_SIZE = 3;
const int32 HMD_GUS3_SIZE = 5;
const int32 HMD_FTS3_SIZE = 5;
const int32 HMD_GTS3_SIZE = 7;
const int32 HMD_FUL3_SIZE = 3;
const int32 HMD_GUL3_SIZE = 4;
const int32 HMD_FTL3_SIZE = 5;
const int32 HMD_GTL3_SIZE = 6;
const int32 TRIANGLE_SIZE = 2;

int32 decodeHMDpolygon(uint32 primType, uint32 pcplatform, uint32 *&pp, uint32 &code0, uint32 &r0, uint32 &g0, uint32 &b0, uint32 &code1, uint32 &r1, uint32 &g1, uint32 &b1, uint32 &code2, uint32 &r2,
					 uint32 &g2, uint32 &b2, uint32 &u0, uint32 &v0, uint32 &u1, uint32 &v1, uint32 &u2, uint32 &v2, uint32 &cba, uint32 &cx, uint32 &cy, uint32 &tsb, uint32 &tp, uint32 &n0, uint32 &vert0,
					 uint32 &n1, uint32 &vert1, uint32 &n2, uint32 &vert2, uint32 dump);

// Handy structures for decoding polygon information

// Flat, Un-textured, Self-Luminous Triangle
// Each polygon is 3 32-bit WORDS
// Bit 31 ----> Bit 0
//
// 8-bits | 8-bits  | 8-bits | 8-bits
// 0x20   | Blue    | Green  | Red
// 16-bits | 8-bits | 8-bits
// --------------------------
//  vp1             | vp0
//  pad             | vp2
typedef struct PolyFUS3 {
	uint8 r0, g0, b0, code0;
	uint16 vp0, vp1;
	uint16 vp2, pad;
} PolyFUS3;

// Gouraud, Un-textured, Self-Luminous Triangle
// Each polygon is 5 32-bit WORDS
// Bit 31 ----> Bit 0
//
//  8-bits | 8-bits | 8-bits | 8-bits
//  0x30   | Blue0  | Green0 | Red0
//  0x30   | Blue1  | Green1 | Red1
//  0x30   | Blue2  | Green2 | Red2
// 16-bits | 8-bits | 8-bits
// --------------------------
//  vp1             | vp0
//  pad             | vp2
typedef struct PolyGUS3 {
	uint8 r0, g0, b0, code0;
	uint8 r1, g1, b1, code1;
	uint8 r2, g2, b2, code2;
	uint16 vp0, vp1;
	uint16 vp2, pad;
} PolyGUS3;

// Flat, Textured, Self-Luminous Triangle
// Each polygon is 5 32-bit WORDS
// Structure is :
// Bit 31 ----> Bit 0
//
//  8-bits | 8-bits | 8-bits | 8-bits
//  0x24   | Blue   | Green  | Red
// 16-bits | 8-bits | 8-bits
// --------------------------
// cba     | v0     | u0
// tsb     | v1     | u1
// vp0     | v2     | u2
// --------------------------
// vp2              | vp1
typedef struct PolyFTS3 {
	uint8 r0, g0, b0, code0;
	uint8 u0, v0;
	uint16 cba;
	uint8 u1, v1;
	uint16 tsb;
	uint8 u2, v2;
	uint16 vp0;
	uint16 vp1, vp2;
} PolyFTS3;

// Gouraud, Textured, Self-Luminous Triangle
// Each polygon is 7 32-bit WORDS
// Structure is :
// Bit 31 ----> Bit 0
//
//  8-bits | 8-bits | 8-bits | 8-bits
//  0x34   | Blue0  | Green0 | Red0
//  0x34   | Blue1  | Green1 | Red1
//  0x34   | Blue2  | Green2 | Red2
// 16-bits | 8-bits | 8-bits
// --------------------------
// cba     | v0     | u0
// tsb     | v1     | u1
// vp0     | v2     | u2
// --------------------------
//  vp2             | vp2
typedef struct PolyGTS3 {
	uint8 r0, g0, b0, code0;
	uint8 r1, g1, b1, code1;
	uint8 r2, g2, b2, code2;
	uint8 u0, v0;
	uint16 cba;
	uint8 u1, v1;
	uint16 tsb;
	uint8 u2, v2;
	uint16 vp0;
	uint16 vp1, vp2;
} PolyGTS3;

// Flat, Un-textured, Lit Triangle
// Each polygon is 3 32-bit WORDS
// Bit 31 ----> Bit 0
//
//  8-bits | 8-bits | 8-bits | 8-bits
//  0x20   | Blue   | Green  | Red
// 16-bits | 16-bits
// ------------------
//  vp0     | np0
//  vp2     | vp1
typedef struct PolyFUL3 {
	uint8 r0, g0, b0, code0;
	uint16 np0, vp0;
	uint16 vp1, vp2;
} PolyFUL3;

// Gouraud, Un-textured, Lit Triangle
// Each polygon is 4 32-bit WORDS
// Bit 31 ----> Bit 0
//
//  8-bits | 8-bits | 8-bits | 8-bits
//  0x20   | Blue   | Green  | Red
// 16-bits | 8-bits | 8-bits
// --------------------------
//  vp0             | np0
//  vp1             | np1
//  vp2             | np2
typedef struct PolyGUL3 {
	uint8 r0, g0, b0, code0;
	uint16 np0, vp0;
	uint16 np1, vp1;
	uint16 np2, vp2;
} PolyGUL3;

// Flat, Textured, Lit Triangle
// Each polygon is 5 32-bit WORDS
// Structure is :
// Bit 31 ----> Bit 0
//
// 16-bits | 8-bits | 8-bits
// --------------------------
// cba     | v0     | u0
// tsb     | v1     | u1
// pad     | v2     | u2
// --------------------------
//  vp0             | np0
//  vp2             | vp1
typedef struct PolyFTL3 {
	uint8 u0, v0;
	uint16 cba;
	uint8 u1, v1;
	uint16 tsb;
	uint8 u2, v2;
	uint16 pad;
	uint16 np0, vp0;
	uint16 vp1, vp2;
} PolyFTL3;

// Gouraud, Textured, Lit Triangle
// Each polygon is 6 32-bit WORDS
// Structure is :
// Bit 31 ----> Bit 0
//
// 16-bits | 8-bits | 8-bits
// --------------------------
// cba     | v0     | u0
// tsb     | v1     | u1
// pad     | v2     | u2
// --------------------------
//  vp0             | np0
//  vp1             | np1
//  vp2             | np2
//
typedef struct PolyGTL3 {
	uint8 u0, v0;
	uint16 cba;
	uint8 u1, v1;
	uint16 tsb;
	uint8 u2, v2;
	uint16 pad;
	uint16 np0, vp0;
	uint16 np1, vp1;
	uint16 np2, vp2;
} PolyGTL3;

#if (_PSX == 0) || (_PSX_ON_PC == 1)

void MatrixToAngles(MATRIX *A, SVECTOR *rotvec);

#endif // #if (_PSX==0) || (_PSX_ON_PC==1)

} // End of namespace ICB

#endif // #ifndef PSX_PCHMD_H
