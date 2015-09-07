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
			BiffObject() {
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
			BiffObject() {
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

	struct Face {
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

			_vertices.push_back(vertex);
		}

		uint32 normalCount = stream->readUint32LE();
		for (uint i = 0; i < normalCount; i++) {
			_normals.push_back(stream->readVector3());
		}

		uint32 textureVertexCount = stream->readUint32LE();
		for (uint i = 0; i < textureVertexCount; i++) {
			_texturePositions.push_back(stream->readVector3());
		}

		uint32 faceCount = stream->readUint32LE();
		for (uint i = 0; i < faceCount; i++) {
			Face face;
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

			_faces.push_back(face);
		}

		_hasPhysics = stream->readByte();
	}

private:
	Common::String _name;

	Common::Array<KeyFrame> _keyFrames;
	Common::Array<Vertex> _vertices;
	Common::Array<Face> _faces;

	Common::Array<Math::Vector3d> _normals;
	Common::Array<Math::Vector3d> _texturePositions;

	bool _hasPhysics;
};

class MeshObjectMaterial : public BiffObject {
public:
	static const uint32 TYPE = kMeshObjectMaterial;

	MeshObjectMaterial() :
			BiffObject() {
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

void BiffMeshReader::read(ArchiveReadStream *stream) {
	BiffArchive archive = BiffArchive(stream, &biffObjectBuilder);
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

} // End of namespace Formats
} // End of namespace Stark
