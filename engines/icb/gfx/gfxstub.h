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
extern u_short psxVRAM[VRAM_WIDTH * VRAM_HEIGHT];
extern u_short psxTP;

// Generic routines in gfxstub.cpp
void InitDrawing(void);
void drawOTList(void);
void ResetZRange(void);
void recoverFromOTcrash(void);

// External graphic routines to be supplied
void startDrawing(void);
void endDrawing(void);

// Real graphic routines
int DrawSprite(int x0, int y0, short w, short h, uint8 r0, uint8 g0, uint8 b0, u_short u0, u_short v0, uint8 alpha, u_short z, void *tex);

// Single coloured rectangle
int DrawTile(int x0, int y0, short w, short h, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, u_short z);

// Single flat coloured line : 2 points, 1 colour
int DrawLineF2(int x0, int y0, int x1, int y1, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, u_short z);

// two connected lines flat coloured : 3 points, 1 colour
int DrawLineF3(int x0, int y0, int x1, int y1, int x2, int y2, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, u_short z);

// three connected lines flat coloured : 4 points, 1 colour
int DrawLineF4(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, u_short z);

// Single gouraud coloured line : 2 points, 2 colours
int DrawLineG2(int x0, int y0, int x1, int y1, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 alpha, u_short z);

// two connected lines gouraud coloured : 3 points, 3 colours
int DrawLineG3(int x0, int y0, int x1, int y1, int x2, int y2, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2, uint8 alpha, u_short z);

// three connected lines gouraud coloured : 4 points, 4 colours
int DrawLineG4(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2, uint8 r3,
               uint8 g3, uint8 b3, uint8 alpha, u_short z);

// Simple flat coloured triangle : 3 points, 1 colour
int DrawFlatTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, u_short z);

// Simple flat coloured quad : 4 points, 1 colour
int DrawFlatQuad(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint8 r0, uint8 g0, uint8 b0, uint8 alpha, u_short z);

// Simple gouraud coloured triangle : 3 points, 3 colours
int DrawGouraudTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2, uint8 alpha,
                        u_short z);

// Simple gouraud coloured quad : 4 points, 4 colours
int DrawGouraudQuad(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2,
                    uint8 r3, uint8 g3, uint8 b3, uint8 alpha, u_short z);

// Simple flat coloured triangle : 3 points, 1 colour, 3 UV's
int DrawFlatTriangleTextured(int x0, int y0, int x1, int y1, int x2, int y2, uint8 r0, uint8 g0, uint8 b0, u_short u0, u_short v0, u_short u1, u_short v1, u_short u2, u_short v2,
                             uint8 alpha, u_short z, void *tex);

// Simple flat coloured quad : 4 points, 1 colour, 4 UV's
int DrawFlatQuadTextured(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint8 r0, uint8 g0, uint8 b0, u_short u0, u_short v0, u_short u1, u_short v1, u_short u2,
                         u_short v2, u_short u3, u_short v3, uint8 alpha, u_short z, void *tex);

// Simple gouraud coloured triangle : 3 points, 3 colours
int DrawGouraudTriangleTextured(int x0, int y0, int x1, int y1, int x2, int y2, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2,
                                u_short u0, u_short v0, u_short u1, u_short v1, u_short u2, u_short v2, uint8 alpha, u_short z, void *tex);

// Simple gouraud coloured quad : 4 points, 4 colours
int DrawGouraudQuadTextured(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint8 r0, uint8 g0, uint8 b0, uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2,
                            uint8 b2, uint8 r3, uint8 g3, uint8 b3, u_short u0, u_short v0, u_short u1, u_short v1, u_short u2, u_short v2, u_short u3, u_short v3, uint8 alpha,
                            u_short z, void *tex);

} // End of namespace ICB
