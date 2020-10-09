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

#ifndef ICB_GRAPIC_PRIMS
#define ICB_GRAPIC_PRIMS

#include "engines/icb/common/px_bitmap.h"
#include "engines/icb/common/px_types.h"
#include "engines/icb/p4_generic.h"

namespace ICB {

typedef float PXreal;
typedef float PXfloat;
typedef double PXdouble;

typedef struct {
	uint32 x1;
	uint32 z1;
	uint32 x2;
	uint32 z2;
} _line;

typedef struct {
	PXreal x;
	PXreal z;
} _point;

// used in extrapolate line
typedef struct {
	PXdouble x;
	PXdouble z;
} _fpoint;

typedef struct {
	PXdouble x;
	PXdouble z;
} _float_point;

// Might seem silly to create yet another rectangle structure, but this one is needed to help the PSX and the PC
// share some of the Remora and inventory code.
typedef struct {
	int32 nX, nY;
	uint32 nWidth, nHeight;
} _PxBitmapRect;

typedef LRECT DXrect;

int32 twabs(int32 val);

void Draw_horizontal_line(int32 xx, int32 yy, uint32 len, _rgb *pen, uint8 *ad, uint32 pitch);
void Draw_vertical_line(int32 xx, int32 yy, uint32 len, _rgb *pen, uint8 *ad, uint32 pitch);
void Fill_rect(int32 x, int32 y, int32 x2, int32 y2, uint32 pen, int32 z = 0);
void General_draw_line_24_32(int16 x0, int16 y0, int16 x1, int16 y1, _rgb *colour, uint8 *myScreenBuffer, uint32 pitch, int32 surface_width = SCREEN_WIDTH,
                             int32 surface_height = SCREEN_DEPTH);

void General_poly_draw_24_32(_point *pVerts, int32 nNumVerts, _rgb sColour, bool8 bFill, uint8 *pSurface, int32 nPitch, int32 nSurfaceWidth, int32 nSurfaceHeight);

// This draws a sprite to a surface, using the x, y found in the sprite itself.
void SpriteFrameDraw(uint8 *pSurfaceBitmap, uint32 nPitch, uint32 nSurfaceWidth, uint32 nSurfaceHeight, _pxBitmap *pBitmap, uint32 nFrameNumber, uint32 *nTransparencyRef,
                     uint8 nOpacity);

// This draws a sprite to a surface, using a supplied x, y, so the sprite can be moved on the surface.
void SpriteXYFrameDraw(uint8 *pSurfaceBitmap, uint32 nPitch, uint32 nSurfaceWidth, uint32 nSurfaceHeight, _pxBitmap *pBitmap, int32 nX, int32 nY, uint32 nFrameNumber,
                       bool8 bCentre, uint32 *nTransparencyRef, uint8 nOpacity);

// These all make LRECTs (needed by Direct-X) from various sources.
LRECT ConvertPxBitmapRectToRECT(const _PxBitmapRect &sBitmapRect);

// Additive gouraud line .... what else ?
void AdditiveGouraudLine(int16 x0, int16 y0, _rgb c0, int16 x1, int16 y1, _rgb c1, uint32 surface_id);
void BlendedLine(int32 x0, int32 y0, int32 x1, int32 y1, _rgb c, uint32 surface_id);
void BlendedLine(int32 x0, int32 y0, int32 x1, int32 y1, _rgb c, int surface_width, int surface_height, uint32 pitch, uint8 *surface);

DXrect MakeRECTFromSpriteSizes(int32 nX, int32 nY, uint32 nWidth, uint32 nHeight);

} // End of namespace ICB

#endif // #ifndef _GRAPIC_PRIMS
