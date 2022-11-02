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

namespace ICB {

#define REVERSE_OT
// #define FORWARD_OT

// For reverse OT
#ifdef REVERSE_OT
#define OT_FIRST OT_SIZE - 1
#define CLEAROTLIST ClearOTagR
#endif // #ifdef REVERSE_OT

#ifdef FORWARD_OT
#define OT_FIRST 0
#define CLEAROTLIST ClearOTag
#endif // #ifdef FORWARD_OT

// The emulation of VRAM : 16-bit pixels 1024x512 big
#define VRAM_WIDTH 1024
#define VRAM_HEIGHT 512
extern uint16 psxVRAM[VRAM_WIDTH * VRAM_HEIGHT];
extern uint16 psxTP;

// Generic routines in gfxstub.cpp
void InitDrawing(void);
void drawOTList(void);
void ResetZRange(void);
void recoverFromOTcrash(void);

// External graphic routines to be supplied
void startDrawing(void);
void endDrawing(void);

// Real graphic routines
int32 DrawSprite(int32 x0, int32 y0, int16 w, int16 h, uint8 r0, uint8 g0, uint8 b0, uint16 u0, uint16 v0, uint8 alpha, uint16 z, void *tex);

// Single coloured rectangle
int32 DrawTile(int32 x0, int32 y0, int16 w, int16 h, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, uint16 z);

// Single flat coloured line : 2 points, 1 colour
int32 DrawLineF2(int32 x0, int32 y0, int32 x1, int32 y1, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, uint16 z);

// two connected lines flat coloured : 3 points, 1 colour
int32 DrawLineF3(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, uint16 z);

// three connected lines flat coloured : 4 points, 1 colour
int32 DrawLineF4(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, uint16 z);

// Single gouraud coloured line : 2 points, 2 colours
int32 DrawLineG2(int32 x0, int32 y0, int32 x1, int32 y1, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 alpha, uint16 z);

// two connected lines gouraud coloured : 3 points, 3 colours
int32 DrawLineG3(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2, uint8 alpha, uint16 z);

// three connected lines gouraud coloured : 4 points, 4 colours
int32 DrawLineG4(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2, uint8 r3,
			   uint8 g3, uint8 b3, uint8 alpha, uint16 z);

// Simple flat coloured triangle : 3 points, 1 colour
int32 DrawFlatTriangle(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, uint16 z);

// Simple flat coloured quad : 4 points, 1 colour
int32 DrawFlatQuad(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, uint16 z);

// Simple gouraud coloured triangle : 3 points, 3 colours
int32 DrawGouraudTriangle(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2, uint8 alpha,
						uint16 z);

// Simple gouraud coloured quad : 4 points, 4 colours
int32 DrawGouraudQuad(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2,
					uint8 r3, uint8 g3, uint8 b3, uint8 alpha, uint16 z);

// Simple flat coloured triangle : 3 points, 1 colour, 3 UV's
int32 DrawFlatTriangleTextured(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, uint8 r0, uint8 g0, uint8 b0, uint16 u0, uint16 v0, uint16 u1, uint16 v1, uint16 u2, uint16 v2,
							 uint8 alpha, uint16 z, void *tex);

// Simple flat coloured quad : 4 points, 1 colour, 4 UV's
int32 DrawFlatQuadTextured(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 r0, uint8 g0, uint8 b0, uint16 u0, uint16 v0, uint16 u1, uint16 v1, uint16 u2,
						 uint16 v2, uint16 u3, uint16 v3, uint8 alpha, uint16 z, void *tex);

// Simple gouraud coloured triangle : 3 points, 3 colours
int32 DrawGouraudTriangleTextured(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2,
								uint16 u0, uint16 v0, uint16 u1, uint16 v1, uint16 u2, uint16 v2, uint8 alpha, uint16 z, void *tex);

// Simple gouraud coloured quad : 4 points, 4 colours
int32 DrawGouraudQuadTextured(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2,
							uint8 b2, uint8 r3, uint8 g3, uint8 b3, uint16 u0, uint16 v0, uint16 u1, uint16 v1, uint16 u2, uint16 v2, uint16 u3, uint16 v3, uint8 alpha,
							uint16 z, void *tex);

} // End of namespace ICB
