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
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/math/math_util.h"
#include "engines/wintermute/utils/path_util.h"

namespace Wintermute {

XMesh::XMesh(Wintermute::BaseGame *inGame) : BaseNamedObject(inGame) {
	_skinMesh = nullptr;
	_blendedMesh = nullptr;
	_staticMesh = nullptr;

	_boneMatrices = nullptr;
	_adjacency = nullptr;

	_BBoxStart = _BBoxEnd = Math::Vector3d(0.0f, 0.0f, 0.0f);
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

	auto fvf = mesh->getFVF();
	uint32 vertexSize = DXGetFVFVertexSize(fvf) / sizeof(float);
	float *vertexBuffer = (float *)mesh->getVertexBuffer().ptr();
	uint32 offset = 0, normalOffset = 0;

	if (fvf & DXFVF_XYZ) {
		offset += sizeof(DXVector3) / sizeof(float);
	}
	if (fvf & DXFVF_NORMAL) {
		normalOffset = offset;
	}

	for (uint32 i = 0; i < mesh->getNumVertices(); ++i) {
		// mirror z coordinate to change to OpenGL coordinate system
		vertexBuffer[i * vertexSize + 2] *= -1.0f;

		if (fvf & DXFVF_NORMAL) {
			// mirror z coordinate to change to OpenGL coordinate system
			vertexBuffer[i * vertexSize + normalOffset + 2] *= -1.0f;
		}
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

	bool res = false;

	// update skinned mesh
	if (_skinMesh) {
		int numBones = _skinMesh->getNumBones();
		DXMatrix *boneMatrices = new DXMatrix[numBones];

		// prepare final matrices
		for (int i = 0; i < numBones; ++i) {
			Math::Matrix4 offsetMatrix;
			for (int r = 0; r < 4; ++r) {
				for (int c = 0; c < 4; ++c) {
					offsetMatrix(c, r) = _skinMesh->getBoneOffsetMatrix(i)->_m4x4[r * 4 + c];
				}
			}
			Math::Matrix4 boneMatrix;
			boneMatrix.setData(_boneMatrices[i]->_m4x4);
			boneMatrices[i] = DXMatrix((boneMatrix * offsetMatrix).getData());
		}

		// generate skinned mesh
		res = _skinMesh->updateSkinnedMesh(boneMatrices, _blendedMesh);
		delete [] boneMatrices;

		if (!res) {
			BaseEngine::LOG(0, "Error updating skinned mesh");
			return res;
		}

		// update mesh bounding box
		byte *points = _blendedMesh->getVertexBuffer().ptr();

		DXVector3 BBoxStart = DXVector3(_BBoxStart.x(), _BBoxStart.y(), _BBoxStart.z());
		DXVector3 BBoxEnd = DXVector3(_BBoxEnd.x(), _BBoxEnd.y(), _BBoxEnd.z());
		DXComputeBoundingBox((DXVector3 *)points, _blendedMesh->getNumVertices(), DXGetFVFVertexSize(_blendedMesh->getFVF()), &BBoxStart, &BBoxEnd);
		_BBoxStart = Math::Vector3d(BBoxStart._x, BBoxStart._y, BBoxStart._z);
		_BBoxEnd = Math::Vector3d(BBoxEnd._x, BBoxEnd._y, BBoxEnd._z);
		// if you want something done right...
		if (isnan(_BBoxEnd.x())) {
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
			_BBoxStart = Math::Vector3d(minX, minY, minZ);
			_BBoxEnd = Math::Vector3d(maxX, maxY, maxZ);
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
			Math::Vector3d newVertex = Math::Vector3d(v._x, v._y, v._z);
			Math::Matrix4 combinedMatrix;
			combinedMatrix.setData(parentFrame->getCombinedMatrix()->_m4x4);
			combinedMatrix.transform(&newVertex, true);

			((DXVector3 *)(newPoints + i * fvfSize))->_x = newVertex.x();
			((DXVector3 *)(newPoints + i * fvfSize))->_y = newVertex.y();
			((DXVector3 *)(newPoints + i * fvfSize))->_z = newVertex.z();
		}

		// update bounding box
		DXVector3 BBoxStart = DXVector3(_BBoxStart.x(), _BBoxStart.y(), _BBoxStart.z());
		DXVector3 BBoxEnd = DXVector3(_BBoxEnd.x(), _BBoxEnd.y(), _BBoxEnd.z());
		DXComputeBoundingBox((DXVector3 *)newPoints, _blendedMesh->getNumVertices(), DXGetFVFVertexSize(_blendedMesh->getFVF()), &BBoxStart, &BBoxEnd);
		_BBoxStart = Math::Vector3d(BBoxStart._x, BBoxStart._y, BBoxStart._z);
		_BBoxEnd = Math::Vector3d(BBoxEnd._x, BBoxEnd._y, BBoxEnd._z);
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::updateShadowVol(ShadowVolume *shadow, Math::Matrix4 &modelMat, const Math::Vector3d &light, float extrusionDepth) {
	if (!_blendedMesh)
		return false;

	float *vertexData = (float *)_blendedMesh->getVertexBuffer().ptr();
	if (vertexData == nullptr) {
		return false;
	}
	uint32 vertexSize = DXGetFVFVertexSize(_blendedMesh->getFVF()) / sizeof(float);

	Math::Vector3d invLight = light;
	Math::Matrix4 matInverseModel = modelMat;
	matInverseModel.inverse();
	matInverseModel.transform(&invLight, false);

	uint32 numEdges = 0;

	uint32 *indexData = (uint32 *)_blendedMesh->getIndexBuffer().ptr();
	uint32 indexDataSize = _blendedMesh->getIndexBuffer().size() / sizeof(uint32);
	Common::Array<bool> isFront(indexDataSize / 3, false);

	// First pass : for each face, record if it is front or back facing the light
	for (uint32 i = 0; i < indexDataSize / 3; i++) {
		uint16 index0 = indexData[3 * i + 0];
		uint16 index1 = indexData[3 * i + 1];
		uint16 index2 = indexData[3 * i + 2];

		Math::Vector3d v0(vertexData + index0 * vertexSize);
		Math::Vector3d v1(vertexData + index1 * vertexSize);
		Math::Vector3d v2(vertexData + index2 * vertexSize);

		// Transform vertices or transform light?
		Math::Vector3d vNormal = Math::Vector3d::crossProduct(v2 - v1, v1 - v0);

		if (Math::Vector3d::dotProduct(vNormal, invLight) >= 0.0f) {
			isFront[i] = false; // back face
		} else {
			isFront[i] = true; // front face
		}
	}

	// Allocate a temporary edge list
	Common::Array<uint32> edges(indexDataSize * 2, 0);

	// First pass : for each face, record if it is front or back facing the light
	for (uint32 i = 0; i < indexDataSize / 3; i++) {
		if (isFront[i]) {
			uint32 wFace0 = indexData[3 * i + 0];
			uint32 wFace1 = indexData[3 * i + 1];
			uint32 wFace2 = indexData[3 * i + 2];

			uint32 adjacent0 = _adjacency[3 * i + 0];
			uint32 adjacent1 = _adjacency[3 * i + 1];
			uint32 adjacent2 = _adjacency[3 * i + 2];

			if (adjacent0 == 0xFFFFFFFF || isFront[adjacent0] == false) {
				//	add edge v0-v1
				edges[2 * numEdges + 0] = wFace0;
				edges[2 * numEdges + 1] = wFace1;
				numEdges++;
			}
			if (adjacent1 == 0xFFFFFFFF || isFront[adjacent1] == false) {
				//	add edge v1-v2
				edges[2 * numEdges + 0] = wFace1;
				edges[2 * numEdges + 1] = wFace2;
				numEdges++;
			}
			if (adjacent2 == 0xFFFFFFFF || isFront[adjacent2] == false) {
				//	add edge v2-v0
				edges[2 * numEdges + 0] = wFace2;
				edges[2 * numEdges + 1] = wFace0;
				numEdges++;
			}
		}
	}

	for (uint32 i = 0; i < numEdges; i++) {
		Math::Vector3d v1(vertexData + edges[2 * i + 0] * vertexSize);
		Math::Vector3d v2(vertexData + edges[2 * i + 1] * vertexSize);
		Math::Vector3d v3 = v1 - invLight * extrusionDepth;
		Math::Vector3d v4 = v2 - invLight * extrusionDepth;

		// Add a quad (two triangles) to the vertex list
		shadow->addVertex(v1);
		shadow->addVertex(v2);
		shadow->addVertex(v3);
		shadow->addVertex(v2);
		shadow->addVertex(v4);
		shadow->addVertex(v3);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::pickPoly(Math::Vector3d *pickRayOrig, Math::Vector3d *pickRayDir) {
	if (!_blendedMesh)
		return false;

	uint32 fvfSize = DXGetFVFVertexSize(_blendedMesh->getFVF());
	uint32 numFaces = _blendedMesh->getNumFaces();

	// lock vertex buffer
	byte *points = _blendedMesh->getVertexBuffer().ptr();

	// lock index buffer
	uint32 *indices = (uint32 *)_blendedMesh->getIndexBuffer().ptr();


	bool found = false;
	Math::Vector3d intersection;

	for (uint32 i = 0; i < numFaces; i++) {
		DXVector3 vp0 = *(DXVector3 *)(points + indices[3 * i + 0] * fvfSize);
		DXVector3 vp1 = *(DXVector3 *)(points + indices[3 * i + 1] * fvfSize);
		DXVector3 vp2 = *(DXVector3 *)(points + indices[3 * i + 2] * fvfSize);
		Math::Vector3d v0 = Math::Vector3d(vp0._x, vp0._y, vp0._z);
		Math::Vector3d v1 = Math::Vector3d(vp1._x, vp1._y, vp1._z);
		Math::Vector3d v2 = Math::Vector3d(vp2._x, vp2._y, vp2._z);

		if (isnan(v0.x()))
			continue;

		found = intersectTriangle(*pickRayOrig, *pickRayDir, v0, v1, v2, intersection.x(), intersection.y(), intersection.z()) != false;
		if (found)
			break;
	}

	return found;
}

////////////////////////////////////////////////////////////////////////////
bool XMesh::setMaterialSprite(const Common::String &matName, BaseSprite *sprite) {
	for (uint32 i = 0; i < _materials.size(); i++) {
		if (_materials[i]->getName() && _materials[i]->getName() == matName) {
			_materials[i]->setSprite(sprite);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::setMaterialTheora(const Common::String &matName, VideoTheoraPlayer *theora) {
	for (uint32 i = 0; i < _materials.size(); i++) {
		if (_materials[i]->getName() && _materials[i]->getName() == matName) {
			_materials[i]->setTheora(theora);
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
