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

#ifndef WATCHMAKER_VERTEX_H
#define WATCHMAKER_VERTEX_H


#include "watchmaker/types.h"
#include "watchmaker/windows_hacks.h"

namespace Watchmaker {

// VertexBuffer vertices definition
#pragma pack(1)
struct gVertex {
	float x;                                            // untransformed vertex
	float y;
	float z;
	int32 diffuse;                                      // diffuse color
	float u1;                                           // texture set for primary texture
	float v1;
	float u2;                                           // texture set for lightmaps
	float v2;
};
#pragma pack()

struct pVert {
	float   x, y, z, rhw;                               // transformed vertex
	DWORD   diffuse;                                    // diffuse color
} ;

} // End of namespace Watchmaker

#endif // WATCHMAKER_VERTEX_H
