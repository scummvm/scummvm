/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "common/endian.h"
#include "common/foreach.h"
#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/material.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/resource.h"
#include "engines/grim/set.h"
#include "engines/grim/emi/costumeemi.h"
#include "engines/grim/emi/modelemi.h"
#include "engines/grim/emi/animationemi.h"
#include "engines/grim/emi/skeleton.h"

namespace Grim {

struct Vector3int {
	uint16 _x;
	uint16 _y;
	uint16 _z;
	void setVal(uint16 x, uint16 y, uint16 z) {
		_x = x; _y = y; _z = z;
	}
};

struct BoneInfo {
	int _incFac;
	int _joint;
	float _weight;
};

Common::String readLAString(Common::ReadStream *ms) {
	int strLength = ms->readUint32LE();
	char *readString = new char[strLength];
	ms->read(readString, strLength);

	Common::String retVal(readString);
	delete[] readString;

	return retVal;
}

void EMIMeshFace::loadFace(Common::SeekableReadStream *data) {
	_flags = data->readUint32LE();
	_hasTexture = data->readUint32LE();

	if (_hasTexture)
		_texID = data->readUint32LE();
	_faceLength = data->readUint32LE();
	_faceLength = _faceLength / 3;
	int x = 0, y = 0, z = 0;
	_indexes = new Vector3int[_faceLength];
	int j = 0;
	for (uint32 i = 0; i < _faceLength; i ++) {
		if (g_grim->getGamePlatform() == Common::kPlatformPS2) {
			x = data->readUint32LE();
			y = data->readUint32LE();
			z = data->readUint32LE();
		} else {
			x = data->readUint16LE();
			y = data->readUint16LE();
			z = data->readUint16LE();
		}
		_indexes[j++].setVal(x, y, z);
	}
}

EMIMeshFace::~EMIMeshFace() {
	delete[] _indexes;
}

void EMIModel::setTex(uint32 index) {
	if (index < _numTextures && _mats[index]) {
		_mats[index]->select();
		g_driver->setBlendMode(_texFlags[index] & BlendAdditive);
	}
}

void EMIModel::loadMesh(Common::SeekableReadStream *data) {
	//int strLength = 0; // Usefull for PS2-strings

	Common::String nameString = readLAString(data);

	for (uint l = 0; l < nameString.size(); ++l) {
		if (nameString[l] == '\\') {
			nameString.setChar('/', l);
		}
	}
	_meshName = nameString;
	_radius = data->readFloatLE();
	_center->readFromStream(data);

	_boxData->readFromStream(data);
	_boxData2->readFromStream(data);

	_numTexSets = data->readUint32LE();
	_setType = data->readUint32LE();
	_numTextures = data->readUint32LE();

	_texNames = new Common::String[_numTextures];
	_texFlags = new uint32[_numTextures];

	for (uint32 i = 0; i < _numTextures; i++) {
		_texNames[i] = readLAString(data);
		_texFlags[i] = data->readUint32LE();
		if (_texFlags[i] & ~(BlendAdditive)) {
			Debug::debug(Debug::Models, "Model %s has unknown flags (%d) for texture %s", nameString.c_str(), _texFlags[i], _texNames[i].c_str());
		}
	}

	prepareTextures();

	int type = data->readUint32LE();
	// Check that it is one of the known types
	//3  is no texture vertecies
	//18 is no normals
	//19 is regular
	assert(type == 19 || type == 18 || type == 3);

	_numVertices = data->readUint32LE();

	_lighting = new Math::Vector3d[_numVertices];
	for (int i = 0; i < _numVertices; i++) {
		_lighting[i].set(1.0f, 1.0f, 1.0f);
	}

	// Vertices
	_vertices = new Math::Vector3d[_numVertices];
	_drawVertices = new Math::Vector3d[_numVertices];
	for (int i = 0; i < _numVertices; i++) {
		_vertices[i].readFromStream(data);
		_drawVertices[i] = _vertices[i];
	}
	_normals = new Math::Vector3d[_numVertices];
	_drawNormals = new Math::Vector3d[_numVertices];
	if (type != 18) {
		for (int i = 0; i < _numVertices; i++) {
			_normals[i].readFromStream(data);
			_drawNormals[i] = _normals[i];
		}
	}
	_colorMap = new EMIColormap[_numVertices];
	for (int i = 0; i < _numVertices; ++i) {
		_colorMap[i].r = data->readByte();
		_colorMap[i].g = data->readByte();
		_colorMap[i].b = data->readByte();
		_colorMap[i].a = data->readByte();
	}
	if (type != 3) {
		_texVerts = new Math::Vector2d[_numVertices];
		for (int i = 0; i < _numVertices; i++) {
			_texVerts[i].readFromStream(data);
		}
	}
	// Faces

	_numFaces = data->readUint32LE();
	if (data->eos()) {
		_numFaces = 0;
		_faces = nullptr;
		return;
	}

	_faces = new EMIMeshFace[_numFaces];

	for (uint32 j = 0; j < _numFaces; j++) {
		_faces[j].setParent(this);
		_faces[j].loadFace(data);
	}

	int hasBones = data->readUint32LE();

	if (hasBones == 1) {
		_numBones = data->readUint32LE();
		_boneNames = new Common::String[_numBones];
		for (int i = 0; i < _numBones; i++) {
			_boneNames[i] = readLAString(data);
		}

		_numBoneInfos =  data->readUint32LE();
		_boneInfos = new BoneInfo[_numBoneInfos];

		for (int i = 0; i < _numBoneInfos; i++) {
			_boneInfos[i]._incFac = data->readUint32LE();
			_boneInfos[i]._joint = data->readUint32LE();
			_boneInfos[i]._weight = data->readFloatLE();
		}
	} else {
		_numBones = 0;
		_numBoneInfos = 0;
	}
	prepareForRender();
}

void EMIModel::setSkeleton(Skeleton *skel) {
	if (_skeleton == skel) {
		return;
	}
	_skeleton = skel;
	if (!skel || !_numBoneInfos) {
		return;
	}
	delete[] _vertexBoneInfo; _vertexBoneInfo = nullptr;
	_vertexBoneInfo = new int[_numBoneInfos];
	for (int i = 0; i < _numBoneInfos; i++) {
		_vertexBoneInfo[i] = _skeleton->findJointIndex(_boneNames[_boneInfos[i]._joint]);
	}
}

void EMIModel::prepareForRender() {
	if (!_skeleton || !_vertexBoneInfo)
		return;

	for (int i = 0; i < _numVertices; i++) {
		_drawVertices[i].set(0.0f, 0.0f, 0.0f);
		_drawNormals[i].set(0.0f, 0.0f, 0.0f);
	}

	int boneVert = -1;
	for (int i = 0; i < _numBoneInfos; i++) {
		if (_boneInfos[i]._incFac == 1) {
			boneVert++;
		}

		int jointIndex = _vertexBoneInfo[i];
		const Math::Matrix4 &jointMatrix = _skeleton->_joints[jointIndex]._finalMatrix;
		const Math::Matrix4 &bindPose = _skeleton->_joints[jointIndex]._absMatrix;

		Math::Vector3d vert = _vertices[boneVert];
		bindPose.inverseTranslate(&vert);
		bindPose.inverseRotate(&vert);
		jointMatrix.transform(&vert, true);
		_drawVertices[boneVert] += vert * _boneInfos[i]._weight;

		Math::Vector3d normal = _normals[boneVert];
		bindPose.inverseRotate(&normal);
		jointMatrix.transform(&normal, false);
		_drawNormals[boneVert] += normal * _boneInfos[i]._weight;
	}

	for (int i = 0; i < _numVertices; i++) {
		_drawNormals[i].normalize();
	}

	g_driver->updateEMIModel(this);
}

void EMIModel::prepareTextures() {
	_mats = new Material*[_numTextures];
	for (uint32 i = 0; i < _numTextures; i++) {
		_mats[i] = _costume->loadMaterial(_texNames[i], false);
	}
}

void EMIModel::draw() {
	prepareForRender();

	Actor *actor = _costume->getOwner();
	Math::Matrix4 modelToWorld = actor->getFinalMatrix();

	if (!actor->isInOverworld()) {
		Math::AABB bounds = calculateWorldBounds(modelToWorld);
		if (bounds.isValid() && !g_grim->getCurrSet()->getFrustum().isInside(bounds))
			return;
	}

	if (!g_driver->supportsShaders()) {
		// If shaders are not available, we calculate lighting in software.
		Actor::LightMode lightMode = actor->getLightMode();
		if (lightMode != Actor::LightNone) {
			if (lightMode != Actor::LightStatic)
				_lightingDirty = true;

			if (_lightingDirty) {
				updateLighting(modelToWorld);
				_lightingDirty = false;
			}
		}
	} else {
		if (actor->getLightMode() == Actor::LightNone) {
			g_driver->disableLights();
		}
	}
	// We will need to add a call to the skeleton, to get the modified vertices, but for now,
	// I'll be happy with just static drawing
	for (uint32 i = 0; i < _numFaces; i++) {
		setTex(_faces[i]._texID);
		g_driver->drawEMIModelFace(this, &_faces[i]);
	}

	if (g_driver->supportsShaders() && actor->getLightMode() == Actor::LightNone) {
		g_driver->enableLights();
	}
}

void EMIModel::updateLighting(const Math::Matrix4 &modelToWorld) {
	// Current lighting implementation mimics the NormDyn mode of the original game, even if
	// FastDyn is requested. We assume that FastDyn mode was used only for the purpose of
	// performance optimization, but NormDyn mode is visually superior in all cases.

	Common::Array<Grim::Light *> activeLights;
	bool hasAmbient = false;

	Actor *actor = _costume->getOwner();

	foreach(Light *l, g_grim->getCurrSet()->getLights(actor->isInOverworld())) {
		if (l->_enabled) {
			activeLights.push_back(l);
			if (l->_type == Light::Ambient)
				hasAmbient = true;
		}
	}

	for (int i = 0; i < _numVertices; i++) {
		Math::Vector3d &result = _lighting[i];
		result.set(0.0f, 0.0f, 0.0f);

		Math::Vector3d normal = _drawNormals[i];
		Math::Vector3d vertex = _drawVertices[i];
		modelToWorld.transform(&vertex, true);
		modelToWorld.transform(&normal, false);

		for (uint j = 0; j < activeLights.size(); ++j) {
			Light *l = activeLights[j];
			float shade = l->_intensity;
		
			if (l->_type != Light::Ambient) {
				// Direction of incident light
				Math::Vector3d dir = l->_dir;

				if (l->_type != Light::Direct) {
					dir = l->_pos - vertex;
					float distSq = dir.getSquareMagnitude();
					if (distSq > l->_falloffFar * l->_falloffFar)
						continue;

					dir.normalize();

					if (distSq > l->_falloffNear * l->_falloffNear) {
						float dist = sqrt(distSq);
						float attn = 1.0f - (dist - l->_falloffNear) / (l->_falloffFar - l->_falloffNear);
						shade *= attn;
					}
				}

				if (l->_type == Light::Spot) {
					float cosAngle = l->_dir.dotProduct(dir);
					if (cosAngle < 0.0f)
						continue;

					float angle = acos(fminf(cosAngle, 1.0f));
					if (angle > l->_penumbraangle)
						continue;

					if (angle > l->_umbraangle)
						shade *= 1.0f - (angle - l->_umbraangle) / (l->_penumbraangle - l->_umbraangle);
				}

				float dot = MAX(0.0f, normal.dotProduct(dir));
				shade *= dot;
			}

			Math::Vector3d color;
			color.x() = l->_color.getRed() / 255.0f;
			color.y() = l->_color.getGreen() / 255.0f;
			color.z() = l->_color.getBlue() / 255.0f;

			result += color * shade;
		}

		if (!hasAmbient) {
			// If the set does not specify an ambient light, a default ambient light is used
			// instead. The effect of this is visible for example in the set gmi.
			result += Math::Vector3d(0.5f, 0.5f, 0.5f);
		}

		float max = MAX(MAX(result.x(), result.y()), result.z());
		if (max > 1.0f) {
			result.x() = result.x() / max;
			result.y() = result.y() / max;
			result.z() = result.z() / max;
		}
	}
}

void EMIModel::getBoundingBox(int *x1, int *y1, int *x2, int *y2) const {
	int winX1, winY1, winX2, winY2;
	g_driver->getScreenBoundingBox(this, &winX1, &winY1, &winX2, &winY2);
	if (winX1 != -1 && winY1 != -1 && winX2 != -1 && winY2 != -1) {
		*x1 = MIN(*x1, winX1);
		*y1 = MIN(*y1, winY1);
		*x2 = MAX(*x2, winX2);
		*y2 = MAX(*y2, winY2);
	}
}

Math::AABB EMIModel::calculateWorldBounds(const Math::Matrix4 &matrix) const {
	Math::AABB bounds;
	for (int i = 0; i < _numVertices; i++) {
		bounds.expand(_drawVertices[i]);
	}
	bounds.transform(matrix);
	return bounds;
}

EMIModel::EMIModel(const Common::String &filename, Common::SeekableReadStream *data, EMICostume *costume) :
		_fname(filename), _costume(costume) {
	_meshAlphaMode = Actor::AlphaOff;
	_meshAlpha = 1.0;
	_numVertices = 0;
	_vertices = nullptr;
	_drawVertices = nullptr;
	_normals = nullptr;
	_drawNormals = nullptr;
	_colorMap = nullptr;
	_texVerts = nullptr;
	_numFaces = 0;
	_faces = nullptr;
	_numTextures = 0;
	_texNames = nullptr;
	_mats = nullptr;
	_numBones = 0;
	_boneInfos = nullptr;
	_numBoneInfos = 0;
	_vertexBoneInfo = nullptr;
	_skeleton = nullptr;
	_radius = 0;
	_center = new Math::Vector3d();
	_boxData = new Math::Vector3d();
	_boxData2 = new Math::Vector3d();
	_numTexSets = 0;
	_setType = 0;
	_boneNames = nullptr;
	_lighting = nullptr;
	_lightingDirty = true;
	_texFlags = nullptr;

	loadMesh(data);
	g_driver->createEMIModel(this);
}

EMIModel::~EMIModel() {
	g_driver->destroyEMIModel(this);

	delete[] _vertices;
	delete[] _drawVertices;
	delete[] _normals;
	delete[] _drawNormals;
	delete[] _colorMap;
	delete[] _texVerts;
	delete[] _faces;
	delete[] _texNames;
	delete[] _mats;
	delete[] _boneInfos;
	delete[] _vertexBoneInfo;
	delete[] _boneNames;
	delete[] _lighting;
	delete[] _texFlags;
	delete _center;
	delete _boxData;
	delete _boxData2;
}

} // end of namespace Grim
