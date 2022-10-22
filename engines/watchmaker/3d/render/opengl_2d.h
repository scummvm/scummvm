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

#ifndef WATCHMAKER_OPENGL2D_H
#define WATCHMAKER_OPENGL2D_H

#include "watchmaker/render.h"
#include "watchmaker/3d/render/opengl_3d.h"

#define MAX_BITMAP_LIST 1024

namespace Watchmaker {

extern int      gStencilBitDepth;

struct gLogo {
	gMaterial   *Material;
	Vertex  Verts[4];
	int         Delay;
};

extern gLogo        Logos[];
extern gMaterial    LogosMaterials[];
extern int          NumLogosMaterials;

extern unsigned int CurLoaderFlags;

} // End of namespace Watchmaker

#endif // WATCHMAKER_OPENGL2D_H
