/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
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

#include "engines/myst3/archive.h"
#include "engines/myst3/nodecube.h"
#include "engines/myst3/myst3.h"

#include "common/debug.h"

namespace Myst3 {

NodeCube::NodeCube(Myst3Engine *vm, uint16 id) :
		Node(vm, id) {
	_is3D = true;

	for (int i = 0; i < 6; i++) {
		ResourceDescription jpegDesc = _vm->getFileDescription("", id, i + 1, Archive::kCubeFace);

		if (!jpegDesc.isValid())
			error("Face %d does not exist", id);

		_faces[i] = new Face(_vm);
		_faces[i]->setTextureFromJPEG(&jpegDesc);
	}
}

NodeCube::~NodeCube() {
}

void NodeCube::draw() {
	// Update the OpenGL textures if needed
	for (uint i = 0; i < 6; i++) {
		if (_faces[i]->isTextureDirty() && isFaceVisible(i)) {
			_faces[i]->uploadTexture();
		}
	}

	Texture *textures[6];
	for (uint i = 0; i < 6; i++) {
		textures[i] = _faces[i]->_texture;
	}

	_vm->_gfx->drawCube(textures);
}

bool NodeCube::isFaceVisible(uint faceId) {
	return _vm->_gfx->isCubeFaceVisible(faceId);
}

} // End of namespace Myst3
