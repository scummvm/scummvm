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

#include "engines/stark/model/model.h"

#include "engines/stark/services/archiveloader.h"
#include "engines/stark/model/animhandler.h"
#include "engines/stark/gfx/texture.h"

#include "math/aabb.h"

namespace Stark {

Model::Model() :
		_u1(0),
		_u2(0.0) {

}

Model::~Model() {
	for (Common::Array<VertNode *>::iterator it = _vertices.begin(); it != _vertices.end(); ++it)
		delete *it;

	for (Common::Array<Material *>::iterator it = _materials.begin(); it != _materials.end(); ++it)
		delete *it;

	for (Common::Array<Face *>::iterator it = _faces.begin(); it != _faces.end(); ++it)
		delete *it;

	for (Common::Array<BoneNode *>::iterator it = _bones.begin(); it != _bones.end(); ++it)
		delete *it;
}

void Model::readFromStream(ArchiveReadStream *stream) {
	uint32 id = stream->readUint32LE();
	if (id != 4) {
		error("Wrong magic 1 while reading actor '%d'", id);
	}

	uint32 format = stream->readUint32LE();
	if (format == 256) {
		_u1 = stream->readUint32LE();
	} else if (format == 16) {
		_u1 = 0;
	} else {
		error("Wrong format while reading actor '%d'", format);
	}

	uint32 id2 = stream->readUint32LE();
	if (id2 != 0xDEADBABE) {
		error("Wrong magic 2 while reading actor '%d'", id2);
	}

	_u2 = stream->readFloat();

	uint32 numMaterials = stream->readUint32LE();

	for (uint i = 0; i < numMaterials; ++i) {
		Material *node = new Material();
		node->name = stream->readString();
		stream->readUint32LE(); // CHECKME: Unknown data
		node->texture = stream->readString();
		node->r = stream->readFloat();
		node->g = stream->readFloat();
		node->b = stream->readFloat();
		_materials.push_back(node);
	}

	uint32 numUnknowns = stream->readUint32LE();
	if (numUnknowns != 0) {
		error("Found a mesh with numUnknowns != 0");
	}

	readBones(stream);

	uint32 numMeshes = stream->readUint32LE();
	if (numMeshes != 1) {
		error("Found a mesh with numMeshes != 1 (%d)", numMeshes);
	}

	_name = stream->readString();

	uint32 numFaces = stream->readUint32LE();
	for (uint32 j = 0; j < numFaces; ++j) {
		uint faceVertexIndexOffset = _vertices.size();

		Face *face = new Face();
		face->materialId = stream->readUint32LE();

		uint32 numVertices = stream->readUint32LE();
		for (uint32 k = 0; k < numVertices; ++k) {
			VertNode *vert = new VertNode();
			vert->_pos1 = stream->readVector3();
			vert->_pos2 = stream->readVector3();
			vert->_normal = stream->readVector3();
			vert->_texS = stream->readFloat();
			vert->_texT = stream->readFloat();
			vert->_bone1 = stream->readUint32LE();
			vert->_bone2 = stream->readUint32LE();
			vert->_boneWeight = stream->readFloat();
			_vertices.push_back(vert);
		}

		uint32 numTriangles = stream->readUint32LE();
		face->vertexIndices.resize(numTriangles * 3); // 3 vertex indices per triangle
		for (uint32 k = 0; k < numTriangles; ++k) {
			face->vertexIndices[k * 3 + 0] = stream->readUint32LE() + faceVertexIndexOffset;
			face->vertexIndices[k * 3 + 1] = stream->readUint32LE() + faceVertexIndexOffset;
			face->vertexIndices[k * 3 + 2] = stream->readUint32LE() + faceVertexIndexOffset;
		}

		_faces.push_back(face);
	}

	buildBonesBoundingBoxes();
}

void Model::readBones(ArchiveReadStream *stream) {
	uint32 numBones = stream->readUint32LE();
	for (uint32 i = 0; i < numBones; ++i) {
		BoneNode *node = new BoneNode();
		node->_name = stream->readString();
		node->_u1 = stream->readFloat();

		uint32 len = stream->readUint32LE();
		for (uint32 j = 0; j < len; ++j)
			node->_children.push_back(stream->readUint32LE());

		node->_idx = _bones.size();
		_bones.push_back(node);
	}

	for (uint32 i = 0; i < numBones; ++i) {
		BoneNode *node = _bones[i];
		for (uint j = 0; j < node->_children.size(); ++j) {
			_bones[node->_children[j]]->_parent = i;
		}
	}
}

void Model::buildBonesBoundingBoxes() {
	for (uint i = 0; i < _bones.size(); i++) {
		buildBoneBoundingBox(_bones[i]);
	}
}

void Model::buildBoneBoundingBox(BoneNode *bone) const {
	bone->_boundingBox.reset();

	// Add all the vertices with a non zero weight for the bone to the bone's bounding box
	for (uint k = 0; k < _vertices.size(); k++) {
		VertNode *vert = _vertices[k];

		if (vert->_bone1 == bone->_idx) {
			bone->_boundingBox.expand(vert->_pos1);
		}

		if (vert->_bone2 == bone->_idx) {
			bone->_boundingBox.expand(vert->_pos2);
		}
	}
}

bool Model::intersectRay(const Math::Ray &ray) const {
	for (uint i = 0; i < _bones.size(); i++) {
		if (_bones[i]->intersectRay(ray)) {
			return true;
		}
	}

	return false;
}

void Model::updateBoundingBox() {
	_boundingBox.reset();
	for (uint i = 0; i < _bones.size(); i++) {
		_bones[i]->expandModelSpaceBB(_boundingBox);
	}
}

Math::AABB Model::getBoundingBox() const {
	return _boundingBox;
}

bool BoneNode::intersectRay(const Math::Ray &ray) const {
	Math::Ray localRay = ray;
	localRay.translate(-_animPos);
	localRay.rotate(_animRot.inverse());

	return localRay.intersectAABB(_boundingBox);
}

void BoneNode::expandModelSpaceBB(Math::AABB &aabb) const {
	// Transform the bounding box
	Math::Vector3d min = _boundingBox.getMin();
	Math::Vector3d max = _boundingBox.getMax();

	Math::Vector3d verts[8];
	verts[0].set(min.x(), min.y(), min.z());
	verts[1].set(max.x(), min.y(), min.z());
	verts[2].set(min.x(), max.y(), min.z());
	verts[3].set(min.x(), min.y(), max.z());
	verts[4].set(max.x(), max.y(), min.z());
	verts[5].set(max.x(), min.y(), max.z());
	verts[6].set(min.x(), max.y(), max.z());
	verts[7].set(max.x(), max.y(), max.z());

	for (int i = 0; i < 8; ++i) {
		_animRot.transform(verts[i]);
		verts[i] += _animPos;
		aabb.expand(verts[i]);
	}
}

} // End of namespace Stark
