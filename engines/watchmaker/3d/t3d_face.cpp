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

#include "common/stream.h"
#include "watchmaker/3d/t3d_face.h"
#include "watchmaker/3d/t3d_body.h"

namespace Watchmaker {

t3dFACE::t3dFACE(t3dBODY *b, Common::SeekableReadStream &stream) {
	VertexIndex[0] = stream.readSint16LE();                                                                    // Legge VertexIndex0
	VertexIndex[1] = stream.readSint16LE();                                                                    // Legge VertexIndex1
	VertexIndex[2] = stream.readSint16LE();                                                                    // Legge VertexIndex2

	this->n = b->NList[stream.readSint16LE()];                                  // Legge puntatore a normale

	_materialIndex = stream.readSint16LE();                                                                    // Legge indice materiale
	_body = b;
}

bool t3dFACE::isVisible() const {
	if (!n)
		return true;
	else if (n->flag != T3D_NORMAL_VISIBLE)
		return false;

	return true;
}

MaterialPtr t3dFACE::getMaterial() {
	if (_mat) {
		return _mat;
	} else {
		_mat = _body->MatTable[_materialIndex];
		return _mat;
	}
}

const gMaterial *t3dFACE::getMaterial() const {
	if (_mat) {
		return _mat.get();
	} else {
		assert(0);
	}
}


} // End of namespace Watchmaker
