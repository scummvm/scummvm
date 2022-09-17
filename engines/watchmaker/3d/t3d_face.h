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

#ifndef WATCHMAKER_T3D_FACE_H
#define WATCHMAKER_T3D_FACE_H

#include "watchmaker/3d/types3d.h"
#include "watchmaker/3d/material.h"

namespace Watchmaker {

struct t3dBODY;
struct t3dFACE {
	uint32      flags = 0;                 // face status                                  4
	NormalPtr   n;                         // pointer to the face normal                   4
	int16      VertexIndex[3] = {};       // Vertices indices in mesh                     6
	int16      MatVertexIndex[3] = {};    // Vertices indices in material                 6

private:
	MaterialPtr mat;                       // pointer to material                          4
public:
	MaterialPtr lightmap;                  // pointer to lightmap (or 2nd material)        4

	t3dFACE(t3dBODY *b, Common::SeekableReadStream &stream);

	bool hasMaterialFlag(uint32 flag) { return mat->hasFlag(flag); }
	MaterialPtr getMaterial() { return mat; }
	const gMaterial* getMaterial() const { return mat.get(); }

	void checkVertices() {
		for (int i = 0; i < 3; i++) {
			assert(mat->VertsList.size() > MatVertexIndex[i]);
		}
	}

	bool isVisible() const;
};

} // End of namespace Watchmaker

#endif // SCUMMVM_T3D_FACE_H
