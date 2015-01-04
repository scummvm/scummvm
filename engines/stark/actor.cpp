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

#include "engines/stark/actor.h"

#include "engines/stark/skeleton.h"
#include "engines/stark/texture.h"

#include "common/stream.h"

namespace Stark {

Actor::Actor() : _skeleton(NULL), _texture(NULL) {

}

Actor::~Actor() {
	for (Common::Array<MaterialNode *>::iterator it = _materials.begin(); it != _materials.end(); ++it)
		delete *it;

	for (Common::Array<MeshNode *>::iterator it = _meshes.begin(); it != _meshes.end(); ++it)
		delete *it;

	if (_skeleton)
		delete _skeleton;
}

void Actor::readFromStream(Common::ReadStream *stream) {
	_id = stream->readUint32LE();
	uint32 format = stream->readUint32LE();
	uint32 u1 = stream->readUint32LE();
	uint32 id2 = stream->readUint32LE();
	if (id2 != 0xDEADBABE) {
		error("Wrong magic while reading actor");
	}

	uint32 u2 = stream->readUint32LE();


	uint32 numMaterials = stream->readUint32LE();

	for (uint i = 0; i < numMaterials; ++i) {
		MaterialNode *node = new MaterialNode();
		uint32 len = stream->readUint32LE();
		char *ptr = new char[len];
		stream->read(ptr, len);
		node->_name = Common::String(ptr, len);
		uint32 u3 = stream->readUint32LE();
		delete[] ptr;
		len = stream->readUint32LE();
		ptr = new char[len];
		stream->read(ptr, len);
		node->_texName = Common::String(ptr, len);
		delete[] ptr;
		ptr = new char[12];
		stream->read(ptr, 12);
		node->_r = get_float(ptr);
		node->_g = get_float(ptr + 4);
		node->_b = get_float(ptr + 8);
		_materials.push_back(node);
	}

	uint32 numUnknowns = stream->readUint32LE();

	for (uint32 i = 0; i < numUnknowns; ++i) {
		UnknownNode *node = new UnknownNode();
		char *ptr = new char[16];
		stream->read(ptr, 16);
		node->_u1 = get_float(ptr);
		node->_u2 = get_float(ptr + 4);
		node->_u3 = get_float(ptr + 8);
		node->_u4 = get_float(ptr + 12);
	}

	_skeleton = new Skeleton();
	_skeleton->readFromStream(stream);

	uint32 numMeshes = stream->readUint32LE();

	for (uint32 i = 0; i < numMeshes; ++i) {
		MeshNode *node = new MeshNode();

		uint32 len = stream->readUint32LE();
		char *ptr = new char[len];
		stream->read(ptr, len);

		node->_name = Common::String(ptr, len);
		delete[] ptr;
		len = stream->readUint32LE();
		for (uint32 j = 0; j < len; ++j) {
			FaceNode *face = new FaceNode();
			face->_matIdx = stream->readUint32LE();

			uint32 childCount = stream->readUint32LE();
			for (uint32 k = 0; k < childCount; ++k) {
				VertNode *vert = new VertNode();
				ptr = new char[14 * 4];
				stream->read(ptr, 14 * 4);
				vert->_pos1 = Math::Vector3d(get_float(ptr), get_float(ptr + 4), get_float(ptr + 8));
				vert->_pos2 = Math::Vector3d(get_float(ptr + 12), get_float(ptr + 16), get_float(ptr + 20));
				vert->_normal = Math::Vector3d(get_float(ptr + 24), get_float(ptr + 28), get_float(ptr + 32));
				vert->_texS = get_float(ptr + 36);
				vert->_texT = get_float(ptr + 40);
				vert->_bone1 = READ_LE_UINT32(ptr + 44);
				vert->_bone2 = READ_LE_UINT32(ptr + 48);
				vert->_boneWeight = get_float(ptr + 52);
				delete[] ptr;
				face->_verts.push_back(vert);
			}

			childCount = stream->readUint32LE();
			for (uint32 k = 0; k < childCount; ++k) {
				TriNode *tri = new TriNode();
				tri->_vert1 = stream->readUint32LE();
				tri->_vert2 = stream->readUint32LE();
				tri->_vert3 = stream->readUint32LE();
				face->_tris.push_back(tri);
			}

			node->_faces.push_back(face);
		}

		_meshes.push_back(node);
	}
}

void Actor::setAnim(SkeletonAnim *anim)
{
	_skeleton->setAnim(anim);
}

void Actor::setTexture(Texture *texture) {
	_texture = texture;
}

} // End of namespace Stark
