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

#ifndef ICB_GFXSTUB_DUTCH_HH
#define ICB_GFXSTUB_DUTCH_HH

namespace ICB {

typedef unsigned int u_int;
typedef unsigned char u_char;
typedef unsigned short u_short;

typedef struct {
	u_int width;     // 640
	u_int height;    // 480
	u_int stride;    // width * 4
	u_char *RGBdata; // width * height * 32-bits
	u_short *Zdata;  // width * height * 16-bits
} RevRenderDevice;

typedef struct {
	int x, y;     // fixed-point 16:16
	int u, v;     // fixed-point 16:16
	u_int colour; // B, G, R, alpha (low->high mem)
} vertex2D;

typedef struct {
	u_int *palette;   /*[256]*/ // windows 32-bit RGB with 1 byte of padding
	u_int width;      // must be power of 2
	u_int height;     // must be power of 2
	u_char *level[9]; // width/1 * height/1 -> width/256 * height/256
} RevTexture;

class TextureHandle;

TextureHandle *RegisterTexture(const RevTexture *revInput);
int UnregisterTexture(TextureHandle *);

int SetTextureState(TextureHandle *texture);
int GetTextureState(TextureHandle *texture);

int SetRenderDevice(RevRenderDevice *renderDev);
int GetRenderDevice(RevRenderDevice *renderDev);

// Colour is verts[0].colour
int DrawFlatUnTexturedPolygon(const vertex2D *verts, int nVerts, u_short z);

int DrawGouraudUnTexturedPolygon(const vertex2D *verts, int nVerts, u_short z);

// Colour is verts[0].colour
int DrawFlatTexturedPolygon(const vertex2D *verts, int nVerts, u_short z);

int DrawGouraudTexturedPolygon(const vertex2D *verts, int nVerts, u_short z);

// Colour is verts[0].colour
int DrawFlatTexturedTransparentPolygon(const vertex2D *verts, int nVerts, u_short z);

int DrawGouraudTexturedTransparentPolygon(const vertex2D *verts, int nVerts, u_short z);

static const int GFXLIB_TRANSPARENT_COLOUR = 0xDEADBEAF;

// This value CANNOT be changed

static const int GFXLIB_TRANSPARENT_INDEX = 0x00000000;

void ClearProcessorState(void);

} // End of namespace ICB

#endif // #ifndef GFXSTUB_DUTCH_HH
