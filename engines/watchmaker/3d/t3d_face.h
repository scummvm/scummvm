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
private:
	int16      MatVertexIndex[3] = {};    // Vertices indices in material                 6
	uint16 _materialIndex = 0;
	MaterialPtr _mat = nullptr;                       // pointer to material                          4
	t3dBODY *_body = nullptr;
public:
	int16      VertexIndex[3] = {};       // Vertices indices in mesh                     6

	void setMatVertexIndex(int index, int16 value) {
		MatVertexIndex[index] = value;
	}
	uint16 getMatVertexIndex(int index) {
		return MatVertexIndex[index];
	}
public:
	MaterialPtr lightmap;                  // pointer to lightmap (or 2nd material)        4

	t3dFACE(t3dBODY *b, Common::SeekableReadStream &stream);

	bool hasMaterialFlag(uint32 flag) {
		return getMaterial()->hasFlag(flag);
	}
	MaterialPtr getMaterial();
	const gMaterial *getMaterial() const;
	uint16 getMaterialIndex() const {
		return _materialIndex;
	}
	void setMaterialIndex(uint32 index) {
		_materialIndex = index;
		_mat = nullptr;
		getMaterial();
	}

	void checkVertices() {
		for (int i = 0; i < 3; i++) {
			assert((int)getMaterial()->VertsList.size() > MatVertexIndex[i]);
		}
	}

	bool isVisible() const;
};

} // End of namespace Watchmaker

#endif // SCUMMVM_T3D_FACE_H
