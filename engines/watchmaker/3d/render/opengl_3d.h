/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
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

#ifndef WATCHMAKER_OPENGL_3D_H
#define WATCHMAKER_OPENGL_3D_H

#include "watchmaker/windows_hacks.h"
#include "watchmaker/render.h"

#define MAXTEXTURES     2000

namespace Watchmaker {

struct gAvailFormat {
	//DDPIXELFORMAT SurfaceDesc;
	char    rbits;
	char    gbits;
	char    bbits;
	char    abits;
};

struct Vertex {
	float sx, sy, sz, rhw;
	uint32 color;
};

extern unsigned int gNumTrianglesArray;
extern unsigned int gNumLinesArray;

extern gAvailFormat *gCurrentFormat;
extern int          NumAvailableFormats;
extern gAvailFormat AvailableFormats[];

extern gTexture     gTextureList[];
extern unsigned int gNumTextureList;

extern uint16           gLinesArray[];
extern unsigned int gNumLinesArray;
extern unsigned int gNumTrianglesArray;
extern Vertex   gTriangles[];
extern int  g_lpD3DPointsBuffer;
extern void    *g_lpD3DUserVertexBuffer;
extern unsigned int             g_dwD3DUserVertexBufferCounter;
extern unsigned int             gNumPointsBuffer;


} // End of namespace Watchmaker

#endif // WATCHMAKER_OPENGL_3D_H
