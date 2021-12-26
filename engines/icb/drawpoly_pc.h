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

#ifndef ICB_DRAWPOLY_PC_H
#define ICB_DRAWPOLY_PC_H

#include "engines/icb/gfx/psx_tman.h"

namespace ICB {

extern uint32 selFace;
extern CVECTOR unlitPoly;
extern uint32 _drawBface;
extern uint32 deadObject;

#if CD_MODE == 0

// Draw a cuboid
void drawSolidBboxPC(SVECTOR *scrn, CVECTOR *rgbIn);

// Debug : Flat, Un-Textured, Self-Luminous, triangles
void drawFUS3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex);

// Debug : Gouraud, Un-Textured, Self-Luminous, triangles
void drawGUS3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex);

// Debug : Flat, Textured, Self-Luminous Triangles
void drawFTS3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex);

// Debug : Gouraud, Textured, Self-Luminous Triangles
void drawGTS3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex);

// Debug : Flat, Un-Textured, Lit, triangles
void drawFUL3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex, SVECTOR *pNormal);

// Debug : Gouraud, Un-Textured, Lit, triangles
void drawGUL3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex, SVECTOR *pNormal);

// Debug : Flat, Textured, Lit Triangles
void drawFTL3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex, SVECTOR *pNormal);

// Debug : Gouraud, Textured, Lit Triangles
void drawGTL3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex, SVECTOR *pNormal);

// Debug : Simple Flat, Un-Textured triangles with no colour in them
void drawTRI3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex);

#endif // #if CD_MODE == 0

// Optimised : Flat, Un-Textured, Self-Luminous, triangles
void fastDrawFUS3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex);

// Optimised : Gouraud, Un-Textured, Self-Luminous, triangles
void fastDrawGUS3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex);

// Optimised : Flat, Textured, Self-Luminous Triangles
void fastDrawFTS3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex);

// Optimised : Gouraud, Textured, Self-Luminous Triangles
void fastDrawGTS3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex);

// Optimised : Flat, Un-Textured, Lit, triangles
void fastDrawFUL3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex, SVECTOR *pNormal);

// Optimised : Gouraud, Un-Textured, Lit, triangles
void fastDrawGUL3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex, SVECTOR *pNormal);

// Optimised : Flat, Textured, Lit Triangles
void fastDrawFTL3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex, SVECTOR *pNormal);

// Optimised : Gouraud, Textured, Lit Triangles
void fastDrawGTL3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex, SVECTOR *pNormal);

// Optimised : Simple Flat, Un-Textured triangles with no colour in them
void fastDrawTRI3PC(uint32 *polyStart, const uint32 n, SVECTORPC *pVertex);

} // End of namespace ICB

#endif // #ifndef DRAWPOLY_PC_H
