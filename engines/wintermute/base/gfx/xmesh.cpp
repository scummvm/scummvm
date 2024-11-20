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

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/base/gfx/3dshadow_volume.h"
#include "engines/wintermute/base/gfx/xmaterial.h"
#include "engines/wintermute/base/gfx/xmesh.h"
#include "engines/wintermute/base/gfx/skin_mesh_helper.h"
#include "engines/wintermute/base/gfx/xframe_node.h"
#include "engines/wintermute/base/gfx/xfile_loader.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/base/gfx/xbuffer.h"
#include "engines/wintermute/base/gfx/xskinmesh.h"
#include "engines/wintermute/base/gfx/3deffect.h"
#include "engines/wintermute/base/gfx/3dutils.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/utils/path_util.h"

#include "math/utils.h"

namespace Wintermute {

XMesh::XMesh(Wintermute::BaseGame *inGame) : BaseNamedObject(inGame) {
	_skinMesh = nullptr;
	_blendedMesh = nullptr;
	_staticMesh = nullptr;

	_boneMatrices = nullptr;
	_adjacency = nullptr;

	_BBoxStart = _BBoxEnd = DXVector3(0.0f, 0.0f, 0.0f);
}

XMesh::~XMesh() {
	delete _skinMesh;
	_skinMesh = nullptr;
	delete _blendedMesh;
	_blendedMesh = nullptr;
	delete _staticMesh;
	_staticMesh = nullptr;

	delete[] _boneMatrices;
	_boneMatrices = nullptr;
	delete[] _adjacency;
	_adjacency = nullptr;

	_materials.clear();
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::loadFromXData(const Common::String &filename, XFileData *xobj) {
	// get name
	if (!XModel::loadName(this, xobj)) {
		BaseEngine::LOG(0, "Error loading mesh name");
		return false;
	}

	// load mesh
	DXBuffer bufMaterials;
	uint32 numMaterials;
	DXMesh *mesh;
	DXSkinInfo *skinInfo;

	auto res = DXLoadSkinMesh(xobj, bufMaterials, numMaterials, &skinInfo, &mesh);
	if (!res) {
		BaseEngine::LOG(0, "Error loading skin mesh");
		return false;
	}
	_skinMesh = new SkinMeshHelper(mesh, skinInfo);

	uint32 numBones = _skinMesh->getNumBones();

	// Process skinning data
	if (numBones) {
		// bones are available
		_boneMatrices = new DXMatrix*[numBones];

		generateMesh();
	} else {
		// no bones are found, blend the mesh and use it as a static mesh
		_skinMesh->getOriginalMesh(&_staticMesh);
		_staticMesh->cloneMesh(&_blendedMesh);

		delete _skinMesh;
		_skinMesh = nullptr;

		if (_blendedMesh) {
			uint32 numFaces = _blendedMesh->getNumFaces();
			_adjacency = new uint32[numFaces * 3];
			_blendedMesh->generateAdjacency(_adjacency);
		}
	}


	// check for materials
	if ((bufMaterials.ptr() == nullptr) || (numMaterials == 0)) {
		// no materials are found, create default material
		Material *mat = new Material(_gameRef);
		mat->_material._diffuse.color._r = 0.5f;
		mat->_material._diffuse.color._g = 0.5f;
		mat->_material._diffuse.color._b = 0.5f;
		mat->_material._specular = mat->_material._diffuse;
		mat->_material._ambient = mat->_material._diffuse;

		_materials.add(mat);
	} else {
		// load the materials
		DXMaterial *fileMats = (DXMaterial *)bufMaterials.ptr();
		for (uint i = 0; i < numMaterials; i++) {
			Material *mat = new Material(_gameRef);
			mat->_material = fileMats[i];
			mat->_material._ambient = mat->_material._diffuse;

			if (fileMats[i]._textureFilename[0] != '\0') {
				mat->setTexture(PathUtil::getDirectoryName(filename) + fileMats[i]._textureFilename, true);
			}

			_materials.add(mat);
		}
	}

	bufMaterials.free();

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::generateMesh() {
	uint32 numFaces = _skinMesh->getNumFaces();

	delete _blendedMesh;
	_blendedMesh = nullptr;

	delete[] _adjacency;
	_adjacency = new uint32[numFaces * 3];

	// blend the mesh
	if (!_skinMesh->generateSkinnedMesh(_adjacency, &_blendedMesh)) {
		BaseEngine::LOG(0, "Error converting to blended mesh");
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::findBones(FrameNode *rootFrame) {
	// normal meshes don't have bones
	if (!_skinMesh)
		return true;

	// get the buffer with the names of the bones
	for (uint32 i = 0; i < _skinMesh->getNumBones(); i++) {
		// find a frame with the same name
		FrameNode *frame = rootFrame->findFrame(_skinMesh->getBoneName(i));
		if (frame) {
			// get a *pointer* to its world matrix
			_boneMatrices[i] = frame->getCombinedMatrix();
		} else {
			BaseEngine::LOG(0, "Warning: Cannot find frame '%s'", _skinMesh->getBoneName(i));
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::update(FrameNode *parentFrame) {
	if (!_blendedMesh)
		return false;

	// update skinned mesh
	if (_skinMesh) {
		int numBones = _skinMesh->getNumBones();
		DXMatrix *boneMatrices = new DXMatrix[numBones];

		// prepare final matrices
		for (int i = 0; i < numBones; i++) {
			DXMatrixMultiply(&boneMatrices[i], _skinMesh->getBoneOffsetMatrix(i), _boneMatrices[i]);
		}

		// generate skinned mesh
		_skinMesh->updateSkinnedMesh(boneMatrices, _blendedMesh);
		delete[] boneMatrices;

		// update mesh bounding box
		byte *points = _blendedMesh->getVertexBuffer().ptr();

		DXComputeBoundingBox((DXVector3 *)points, _blendedMesh->getNumVertices(), DXGetFVFVertexSize(_blendedMesh->getFVF()), &_BBoxStart, &_BBoxEnd);
		// if you want something done right...
		if (isnan(_BBoxEnd._x)) {
			float minX = FLT_MAX;
			float minY = FLT_MAX;
			float minZ = FLT_MAX;
			float maxX = FLT_MIN;
			float maxY = FLT_MIN;
			float maxZ = FLT_MIN;

			uint32 fvfSize = _blendedMesh->getFVF();

			byte *vectBuf = points;
			for (uint32 i = 0; i < _blendedMesh->getNumVertices(); i++) {
				DXVector3 *vect = (DXVector3 *)vectBuf;

				minX = MIN(minX, vect->_x);
				minY = MIN(minY, vect->_y);
				minZ = MIN(minZ, vect->_z);

				maxX = MAX(maxX, vect->_x);
				maxY = MAX(maxY, vect->_y);
				maxZ = MAX(maxZ, vect->_z);

				vectBuf += DXGetFVFVertexSize(fvfSize);
			}
			_BBoxStart = DXVector3(minX, minY, minZ);
			_BBoxEnd = DXVector3(maxX, maxY, maxZ);
		}
	} else {
		// update static mesh
		uint32 fvfSize = DXGetFVFVertexSize(_blendedMesh->getFVF());
		uint32 numVertices = _blendedMesh->getNumVertices();

		// lock static vertex buffer
		byte *oldPoints = _staticMesh->getVertexBuffer().ptr();

		// lock blended vertex buffer
		byte *newPoints = _blendedMesh->getVertexBuffer().ptr();

		for (uint32 i = 0; i < numVertices; i++) {
			DXVector3 v = *(DXVector3 *)(oldPoints + i * fvfSize);
			DXVector4 newVertex;
			DXVec3Transform(&newVertex, &v, parentFrame->getCombinedMatrix());

			((DXVector3 *)(newPoints + i * fvfSize))->_x = newVertex._x;
			((DXVector3 *)(newPoints + i * fvfSize))->_y = newVertex._y;
			((DXVector3 *)(newPoints + i * fvfSize))->_z = newVertex._z;
		}

		// update bounding box
		DXComputeBoundingBox((DXVector3 *)newPoints, _blendedMesh->getNumVertices(), DXGetFVFVertexSize(_blendedMesh->getFVF()), &_BBoxStart, &_BBoxEnd);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::updateShadowVol(ShadowVolume *shadow, DXMatrix *modelMat, DXVector3 *light, float extrusionDepth) {
	if (!_blendedMesh)
		return false;

	return shadow->addMesh(_blendedMesh, _adjacency, modelMat, light, extrusionDepth);
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::pickPoly(DXVector3 *pickRayOrig, DXVector3 *pickRayDir) {
	if (!_blendedMesh)
		return false;

	uint32 fvfSize = DXGetFVFVertexSize(_blendedMesh->getFVF());
	uint32 numFaces = _blendedMesh->getNumFaces();

	// lock vertex buffer
	byte *points = _blendedMesh->getVertexBuffer().ptr();

	// lock index buffer
	uint32 *indices = (uint32 *)_blendedMesh->getIndexBuffer().ptr();


	bool found = false;
	DXVector3 intersection;

	for (uint32 i = 0; i < numFaces; i++) {
		DXVector3 v0 = *(DXVector3 *)(points + indices[3 * i + 0] * fvfSize);
		DXVector3 v1 = *(DXVector3 *)(points + indices[3 * i + 1] * fvfSize);
		DXVector3 v2 = *(DXVector3 *)(points + indices[3 * i + 2] * fvfSize);

		if (isnan(v0._x))
			continue;

		found = C3DUtils::intersectTriangle(*pickRayOrig, *pickRayDir, v0, v1, v2, &intersection._x, &intersection._y, &intersection._z) != false;
		if (found)
			break;
	}

	return found;
}

////////////////////////////////////////////////////////////////////////////
bool XMesh::setMaterialSprite(const Common::String &matName, BaseSprite *sprite) {
	for (uint32 i = 0; i < _materials.size(); i++) {
		if (_materials[i]->getName() && scumm_stricmp(_materials[i]->getName(),  matName.c_str()) == 0) {
			_materials[i]->setSprite(sprite);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::setMaterialTheora(const Common::String &matName, VideoTheoraPlayer *theora) {
	for (uint32 i = 0; i < _materials.size(); i++) {
		if (_materials[i]->getName() && scumm_stricmp(_materials[i]->getName(), matName.c_str()) == 0) {
			_materials[i]->setTheora(theora);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::setMaterialEffect(const Common::String &matName, Effect3D *effect, Effect3DParams *params) {
	for (uint32 i = 0; i < _materials.size(); i++) {
		if (_materials[i]->getName() && scumm_stricmp(_materials[i]->getName(), matName.c_str()) == 0) {
			_materials[i]->setEffect(effect, params);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::removeMaterialEffect(const Common::String &matName) {
	for (uint32 i = 0; i < _materials.size(); i++) {
		if (_materials[i]->getName() && scumm_stricmp(_materials[i]->getName(), matName.c_str()) == 0) {
			_materials[i]->setEffect(nullptr, nullptr);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::invalidateDeviceObjects() {
	if (_skinMesh) {
		delete[] _blendedMesh;
		_blendedMesh = nullptr;
	}

	for (uint32 i = 0; i < _materials.size(); i++) {
		_materials[i]->invalidateDeviceObjects();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::restoreDeviceObjects() {
	for (uint32 i = 0; i < _materials.size(); i++) {
		_materials[i]->restoreDeviceObjects();
	}

	if (_skinMesh) {
		return generateMesh();
	} else {
		return true;
	}
}

} // namespace Wintermute
