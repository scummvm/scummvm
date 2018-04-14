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

#include "engines/stark/formats/biffmesh.h"

#include "engines/stark/formats/biff.h"
#include "engines/stark/services/archiveloader.h"

#include "common/hashmap.h"

namespace Stark {
namespace Formats {

enum BiffMeshObjectType {
	kMeshObjectSceneData = 0x5a4aa94,
	kMeshObjectBase      = 0x5a4aa89,
	kMeshObjectTri       = 0x5a4aa8d,
	kMeshObjectMaterial  = 0x5a4aa8e
};

class MeshObjectSceneData : public BiffObject {
public:
	static const uint32 TYPE = kMeshObjectSceneData;

	MeshObjectSceneData() :
			BiffObject(),
			_animStart(0),
			_animEnd(0) {
		_type = TYPE;
	}

	// BiffObject API
	void readData(ArchiveReadStream *stream, uint32 dataLength) override {
		_animStart = stream->readUint32LE();
		_animEnd = stream->readUint32LE();
	}

private:
	uint32 _animStart;
	uint32 _animEnd;
};

class MeshObjectBase : public BiffObject {
public:
	static const uint32 TYPE = kMeshObjectBase;

	MeshObjectBase() :
			BiffObject() {
		_type = TYPE;
	}

	// BiffObject API
	void readData(ArchiveReadStream *stream, uint32 dataLength) override {
		_name = stream->readString16();
	}

private:
	Common::String _name;
};

class MeshObjectTri : public BiffObject {
public:
	static const uint32 TYPE = kMeshObjectTri;

	MeshObjectTri() :
			BiffObject(),
			_hasPhysics(false) {
		_type = TYPE;
	}

	struct KeyFrame {
		uint32 time;
		Math::Quaternion essentialRotation;
		float determinant;
		Math::Quaternion stretchRotation;
		Math::Vector3d scale;
		Math::Vector3d translation;
	};

	struct Vertex {
		Common::String animName1;
		Common::String animName2;
		float animInfluence1;
		float animInfluence2;
		Math::Vector3d position;
	};

	struct RawFace {
		uint32 vertexIndex[3];
		uint32 normalIndex[3];
		uint32 textureVertexIndex[3];
		uint32 materialId;
		uint32 smoothingGroup;
	};

	// BiffObject API
	void readData(ArchiveReadStream *stream, uint32 dataLength) override {
		_name = stream->readString16();

		uint32 keyFrameCount = stream->readUint32LE();
		for (uint i = 0; i < keyFrameCount; i++) {
			KeyFrame keyFrame;
			keyFrame.time = stream->readUint32LE();
			keyFrame.essentialRotation = stream->readQuaternion();
			keyFrame.determinant = stream->readFloat();
			keyFrame.stretchRotation = stream->readQuaternion();
			keyFrame.scale = stream->readVector3();
			keyFrame.translation = stream->readVector3();

			_keyFrames.push_back(keyFrame);
		}

		if (_version >= 2) {
			uint32 uvKeyFrameCount = stream->readUint32LE();
			assert(uvKeyFrameCount == 0); // Reading the uv keyframes is not implemented
			uint32 attributeCount = stream->readUint32LE();
			assert(attributeCount == 0); // Reading the attributes is not implemented
		}

		uint32 vertexCount = stream->readUint32LE();
		for (uint i = 0; i < vertexCount; i++) {
			Vertex vertex;
			vertex.animName1 = stream->readString16();
			vertex.animName2 = stream->readString16();
			vertex.animInfluence1 = stream->readFloat();
			vertex.animInfluence2 = stream->readFloat();
			vertex.position = stream->readVector3();

			_rawVertices.push_back(vertex);
		}

		uint32 normalCount = stream->readUint32LE();
		for (uint i = 0; i < normalCount; i++) {
			_rawNormals.push_back(stream->readVector3());
		}

		uint32 textureVertexCount = stream->readUint32LE();
		for (uint i = 0; i < textureVertexCount; i++) {
			_rawTexturePositions.push_back(stream->readVector3());
		}

		uint32 faceCount = stream->readUint32LE();
		for (uint i = 0; i < faceCount; i++) {
			RawFace face;
			face.vertexIndex[0] = stream->readUint32LE();
			face.vertexIndex[1] = stream->readUint32LE();
			face.vertexIndex[2] = stream->readUint32LE();
			face.normalIndex[0] = stream->readUint32LE();
			face.normalIndex[1] = stream->readUint32LE();
			face.normalIndex[2] = stream->readUint32LE();
			face.textureVertexIndex[0] = stream->readUint32LE();
			face.textureVertexIndex[1] = stream->readUint32LE();
			face.textureVertexIndex[2] = stream->readUint32LE();
			face.materialId = stream->readUint32LE();
			face.smoothingGroup = stream->readUint32LE();

			_rawFaces.push_back(face);
		}

		_hasPhysics = stream->readByte();
	}

	void reindex() {
		// The raw data loaded from the BIFF archive needs to be split into faces of identical material.
		// Reserve enough faces in our faces array
		for (uint i = 0; i < _rawFaces.size(); i++) {
			if (_rawFaces[i].materialId >= _faces.size()) {
				_faces.resize(_rawFaces[i].materialId + 1);
			}
			_faces[_rawFaces[i].materialId].materialId = _rawFaces[i].materialId;
		}

		// The raw data loaded from the BIFF archive is multi-indexed, which is not simple to use to draw.
		// Here, we reindex the data so that each vertex owns all of its related attributes, hence requiring
		// a single index list.
		Common::HashMap<VertexKey, uint32, VertexKey::Hash, VertexKey::EqualTo> vertexIndexMap;
		for (uint i = 0; i < _rawFaces.size(); i++) {
			for (uint j = 0; j < 3; j++) {
				VertexKey vertexKey(_rawFaces[i].vertexIndex[j], _rawFaces[i].normalIndex[j], _rawFaces[i].textureVertexIndex[j]);
				if (!vertexIndexMap.contains(vertexKey)) {
					BiffMesh::Vertex vertex;
					vertex.position = _rawVertices[_rawFaces[i].vertexIndex[j]].position;
					vertex.normal = _rawNormals[_rawFaces[i].normalIndex[j]];
					vertex.texturePosition = _rawTexturePositions[_rawFaces[i].textureVertexIndex[j]];

					_vertices.push_back(vertex);

					vertexIndexMap.setVal(vertexKey, _vertices.size() - 1);
				}

				uint32 vertexIndex = vertexIndexMap.getVal(vertexKey);

				// Add the index to a face according to its material
				_faces[_rawFaces[i].materialId].vertexIndices.push_back(vertexIndex);
			}
		}

		// Clear the raw data
		_rawVertices.clear();
		_rawNormals.clear();
		_rawTexturePositions.clear();
		_rawFaces.clear();
	}

	Math::Matrix4 getTransform(uint keyframeIndex) const {
		const KeyFrame &keyframe = _keyFrames[keyframeIndex];

		Math::Matrix4 translation;
		translation.setPosition(keyframe.translation);

		Math::Matrix4 essentialRotation = keyframe.essentialRotation.toMatrix();

		Math::Matrix4 determinant;
		determinant.setValue(0, 0, keyframe.determinant);
		determinant.setValue(1, 1, keyframe.determinant);
		determinant.setValue(2, 2, keyframe.determinant);

		Math::Matrix4 stretchRotation = keyframe.stretchRotation.toMatrix();

		Math::Matrix4 stretchRotationTransposed = stretchRotation;
		stretchRotationTransposed.transpose();

		Math::Matrix4 scale;
		scale.setValue(0, 0, keyframe.scale.x());
		scale.setValue(1, 1, keyframe.scale.y());
		scale.setValue(2, 2, keyframe.scale.z());

		return translation * essentialRotation * determinant * stretchRotationTransposed * scale * stretchRotation;
	}

	const Common::Array<BiffMesh::Vertex> &getVertices() const {
		return _vertices;
	}

	const Common::Array<Face> &getFaces() const {
		return _faces;
	}

private:
	struct VertexKey {
		uint32 _vertexIndex;
		uint32 _normalIndex;
		uint32 _textureVertexIndex;

		VertexKey(uint32 vertexIndex, uint32 normalIndex, uint32 textureVertexIndex) {
			_vertexIndex = vertexIndex;
			_normalIndex = normalIndex;
			_textureVertexIndex = textureVertexIndex;
		}

		struct Hash {
			uint operator() (const VertexKey &x) const {
				return x._vertexIndex + x._normalIndex + x._textureVertexIndex;
			}
		};

		struct EqualTo {
			bool operator() (const VertexKey &x, const VertexKey &y) const {
				return x._vertexIndex == y._vertexIndex &&
						x._normalIndex == y._normalIndex &&
						x._textureVertexIndex == y._textureVertexIndex;
			}
		};
	};

	Common::String _name;

	Common::Array<KeyFrame> _keyFrames;

	Common::Array<Vertex> _rawVertices;
	Common::Array<RawFace> _rawFaces;
	Common::Array<Math::Vector3d> _rawNormals;
	Common::Array<Math::Vector3d> _rawTexturePositions;

	Common::Array<BiffMesh::Vertex> _vertices;
	Common::Array<Face> _faces;

	bool _hasPhysics;
};

class MeshObjectMaterial : public BiffObject {
public:
	static const uint32 TYPE = kMeshObjectMaterial;

	MeshObjectMaterial() :
			BiffObject(),
			_shading(0),
			_shininess(0),
			_opacity(1),
			_doubleSided(false),
			_textureTiling(0),
			_alphaTiling(0),
			_environementTiling(0),
			_isColorKey(false),
			_colorKey(0) {
		_type = TYPE;
	}

	// BiffObject API
	void readData(ArchiveReadStream *stream, uint32 dataLength) override {
		_name = stream->readString16();
		_texture = stream->readString16();
		_alpha = stream->readString16();
		_environment = stream->readString16();

		_shading = stream->readUint32LE();
		_ambiant = stream->readVector3();
		_diffuse = stream->readVector3();
		_specular = stream->readVector3();

		_shininess = stream->readFloat();
		_opacity = stream->readFloat();

		_doubleSided = stream->readByte();
		_textureTiling = stream->readUint32LE();
		_alphaTiling = stream->readUint32LE();
		_environementTiling = stream->readUint32LE();

		_isColorKey = stream->readByte();
		_colorKey = stream->readUint32LE();

		uint32 attributeCount = stream->readUint32LE();
		assert(attributeCount == 0); // Reading the attributes is not implemented
	}

	Material toMaterial() const {
		Material material;
		material.name = _name;
		material.texture = _texture;
		material.r = _diffuse.x();
		material.g = _diffuse.y();
		material.b = _diffuse.z();
		material.doubleSided = _doubleSided;

		return material;
	}

private:
	Common::String _name;
	Common::String _texture;
	Common::String _alpha;
	Common::String _environment;

	uint32 _shading;
	Math::Vector3d _ambiant;
	Math::Vector3d _diffuse;
	Math::Vector3d _specular;

	float _shininess;
	float _opacity;

	bool _doubleSided;
	uint32 _textureTiling;
	uint32 _alphaTiling;
	uint32 _environementTiling;

	bool _isColorKey;
	uint32 _colorKey;
};

BiffMesh *BiffMeshReader::read(ArchiveReadStream *stream) {
	BiffArchive archive = BiffArchive(stream, &biffObjectBuilder);
	Common::Array<MeshObjectTri *> tris = archive.listObjectsRecursive<MeshObjectTri>();
	Common::Array<MeshObjectMaterial *> materialObjects = archive.listObjectsRecursive<MeshObjectMaterial>();

	if (tris.size() != 1) {
		error("Unexpected tri count in BIFF archive: '%d'", tris.size());
	}

	tris[0]->reindex();

	Common::Array<Material> materials;
	for (uint i = 0; i < materialObjects.size(); i++) {
		materials.push_back(materialObjects[i]->toMaterial());
	}

	BiffMesh *mesh = new BiffMesh(tris[0]->getVertices(), tris[0]->getFaces(), materials);
	mesh->setTransform(tris[0]->getTransform(0));
	return mesh;
}

BiffObject *BiffMeshReader::biffObjectBuilder(uint32 type) {
	switch (type) {
		case kMeshObjectSceneData:
			return new MeshObjectSceneData();
		case kMeshObjectBase:
			return new MeshObjectBase();
		case kMeshObjectTri:
			return new MeshObjectTri();
		case kMeshObjectMaterial:
			return new MeshObjectMaterial();
		default:
			return nullptr;
	}
}

BiffMesh::BiffMesh(const Common::Array<Vertex> &vertices, const Common::Array<Face> &faces,
                   const Common::Array<Material> &materials) :
		_vertices(vertices),
		_faces(faces),
		_materials(materials) {
}

const Common::Array<BiffMesh::Vertex> &BiffMesh::getVertices() const {
	return _vertices;
}

const Common::Array<Face> &BiffMesh::getFaces() const {
	return _faces;
}

const Common::Array<Material> &BiffMesh::getMaterials() const {
	return _materials;
}

void BiffMesh::setTransform(const Math::Matrix4 &transform) {
	_transform = transform;
}

Math::Matrix4 BiffMesh::getTransform() const {
	return _transform;
}

} // End of namespace Formats
} // End of namespace Stark
