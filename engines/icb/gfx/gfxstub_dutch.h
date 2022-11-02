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

#ifndef ICB_GFXSTUB_DUTCH_HH
#define ICB_GFXSTUB_DUTCH_HH

namespace ICB {

typedef struct {
	uint32 width;     // 640
	uint32 height;    // 480
	uint32 stride;    // width * 4
	uint8 *RGBdata; // width * height * 32-bits
	uint16 *Zdata;  // width * height * 16-bits
} RevRenderDevice;

typedef struct {
	int32 x, y;     // fixed-point 16:16
	int32 u, v;     // fixed-point 16:16
	uint32 colour; // B, G, R, alpha (low->high mem)
} vertex2D;

typedef struct {
	uint32 *palette;   /*[256]*/ // windows 32-bit RGB with 1 byte of padding
	uint32 width;      // must be power of 2
	uint32 height;     // must be power of 2
	uint8 *level[9]; // width/1 * height/1 -> width/256 * height/256
} RevTexture;

class TextureHandle;

TextureHandle *RegisterTexture(const RevTexture *revInput);
int32 UnregisterTexture(TextureHandle *);

int32 SetTextureState(TextureHandle *texture);
int32 GetTextureState(TextureHandle *texture);

int32 SetRenderDevice(RevRenderDevice *renderDev);
int32 GetRenderDevice(RevRenderDevice *renderDev);

// Colour is verts[0].colour
int32 DrawFlatUnTexturedPolygon(const vertex2D *verts, int32 nVerts, uint16 z);

int32 DrawGouraudUnTexturedPolygon(const vertex2D *verts, int32 nVerts, uint16 z);

// Colour is verts[0].colour
int32 DrawFlatTexturedPolygon(const vertex2D *verts, int32 nVerts, uint16 z);

int32 DrawGouraudTexturedPolygon(const vertex2D *verts, int32 nVerts, uint16 z);

// Colour is verts[0].colour
int32 DrawFlatTexturedTransparentPolygon(const vertex2D *verts, int32 nVerts, uint16 z);

int32 DrawGouraudTexturedTransparentPolygon(const vertex2D *verts, int32 nVerts, uint16 z);

static const int32 GFXLIB_TRANSPARENT_COLOUR = 0xDEADBEAF;

// This value CANNOT be changed

static const int32 GFXLIB_TRANSPARENT_INDEX = 0x00000000;

void ClearProcessorState(void);

} // End of namespace ICB

#endif // #ifndef GFXSTUB_DUTCH_HH
