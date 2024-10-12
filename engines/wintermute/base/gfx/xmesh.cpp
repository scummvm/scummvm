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
#include "engines/wintermute/base/gfx/xskinmesh_loader.h"
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

	_BBoxStart = Math::Vector3d(0.0f, 0.0f, 0.0f);
	_BBoxEnd = Math::Vector3d(0.0f, 0.0f, 0.0f);
}

XMesh::~XMesh() {
	delete _skinMesh;
	_skinMesh = nullptr;
	delete _blendedMesh;
	_blendedMesh = nullptr;
	delete _staticMesh;
	_staticMesh = nullptr;
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
	//DXBuffer bufBoneOffset;
	//uint32 numFaces;
	uint32 numMaterials;
	DXMesh *mesh;
	DXSkinInfo *skinInfo;

	auto res = DXLoadSkinMesh(xobj, bufMaterials, numMaterials, &skinInfo, &mesh);
	if (!res) {
		BaseEngine::LOG(0, "Error loading skin mesh");
		return false;
	}

	XSkinMeshLoader *meshLoader = new XSkinMeshLoader(mesh);
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

	//numFaces = _skinMesh->getNumFaces();

	uint32 numBones = _skinMesh->getNumBones();

	// Process skinning data
	if (numBones) {
		// bones are available
		//_boneMatrices = new DXMatrix*[numBones];

		generateMesh();

		_blendedMesh->_meshLoader = meshLoader;
		_skinMesh->_mesh->_meshLoader = new XSkinMeshLoader(_skinMesh->_mesh);
	} else {
		// no bones are found, blend the mesh and use it as a static mesh
		_skinMesh->getOriginalMesh(&_staticMesh);
		_staticMesh->cloneMesh(&_blendedMesh);

		_staticMesh->_meshLoader = new XSkinMeshLoader(_staticMesh);

		delete _skinMesh;
		_skinMesh = nullptr;

		if (_blendedMesh) {
			_blendedMesh->_meshLoader = meshLoader;
			//numFaces = _blendedMesh->getNumFaces();
			//_adjacency = new uint32[numFaces * 3];
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

	for (uint index = 0; index < numBones; index++) {
		SkinWeights currSkinWeights;
		DXBone *bone = skinInfo->getBone(index);
		currSkinWeights._boneName = bone->_name;

		int weightCount = bone->_numInfluences;
		currSkinWeights._vertexIndices.resize(weightCount);
		currSkinWeights._vertexWeights.resize(weightCount);

		for (int i = 0; i < weightCount; ++i) {
			currSkinWeights._vertexIndices[i] = bone->_vertices[i];
		}

		for (int i = 0; i < weightCount; ++i) {
			currSkinWeights._vertexWeights[i] = bone->_weights[i];
		}

		for (int r = 0; r < 4; ++r) {
			for (int c = 0; c < 4; ++c) {
				currSkinWeights._offsetMatrix(c, r) = bone->_transform._m4x4[r * 4 + c];
			}
		}

		// mirror at orign
		currSkinWeights._offsetMatrix(2, 3) *= -1.0f;

		// mirror base vectors
		currSkinWeights._offsetMatrix(2, 0) *= -1.0f;
		currSkinWeights._offsetMatrix(2, 1) *= -1.0f;

		// change handedness
		currSkinWeights._offsetMatrix(0, 2) *= -1.0f;
		currSkinWeights._offsetMatrix(1, 2) *= -1.0f;

		meshLoader->_skinWeightsList.push_back(currSkinWeights);
	}

	bufMaterials.free();
	//bufBoneOffset.free();

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::generateMesh() {
	//uint32 numFaces = _skinMesh->getNumFaces();

	delete _blendedMesh;
	_blendedMesh = nullptr;

	//delete[] _adjacency;
	//_adjacency = new uint32[numFaces * 3];

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

	auto skinWeightsList = _blendedMesh->_meshLoader->_skinWeightsList;

	_boneMatrices.resize(skinWeightsList.size());
	// get the buffer with the names of the bones
	for (uint i = 0; i < skinWeightsList.size(); ++i) {
		// find a frame with the same name
		FrameNode *frame = rootFrame->findFrame(skinWeightsList[i]._boneName.c_str());

		if (frame) {
			// get a *pointer* to its world matrix
			_boneMatrices[i] = frame->getCombinedMatrix();
		} else {
			BaseEngine::LOG(0, "Warning: Cannot find frame '%s'", skinWeightsList[i]._boneName.c_str());
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::update(FrameNode *parentFrame) {
	if (!_blendedMesh)
		return false;

	float *vertexDstData = (float *)_blendedMesh->getVertexBuffer().ptr();
	if (vertexDstData == nullptr) {
		return false;
	}
	auto fvf = _blendedMesh->getFVF();
	uint32 vertexSize = DXGetFVFVertexSize(fvf) / sizeof(float);
	uint32 offset = 0, normalOffset = 0;
	if (fvf & DXFVF_XYZ) {
		offset += sizeof(DXVector3) / sizeof(float);
	}
	if (fvf & DXFVF_NORMAL) {
		normalOffset = offset;
	}

	uint32 vertexCount = _blendedMesh->getNumVertices();
	auto skinWeightsList = _blendedMesh->_meshLoader->_skinWeightsList;

	// update skinned mesh
	if (_skinMesh) {
		float *vertexSrcData = (float *)_skinMesh->_mesh->getVertexBuffer().ptr();
		BaseArray<Math::Matrix4> finalBoneMatrices;
		finalBoneMatrices.resize(_boneMatrices.size());

		// prepare final matrices
		for (uint i = 0; i < skinWeightsList.size(); ++i) {
			finalBoneMatrices[i] = *_boneMatrices[i] * skinWeightsList[i]._offsetMatrix;
		}

		// the new vertex coordinates are the weighted sum of the product
		// of the combined bone transformation matrices and the static pose coordinates
		// to be able too add the weighted summands together, we reset everything to zero first
		for (uint32 i = 0; i < vertexCount; ++i) {
			for (int j = 0; j < 3; ++j) {
				vertexDstData[i * vertexSize + j] = 0.0f;
			}
		}

		for (uint boneIndex = 0; boneIndex < skinWeightsList.size(); ++boneIndex) {
			// to every vertex which is affected by the bone, we add the product
			// of the bone transformation with the coordinates of the static pose,
			// weighted by the weight for the particular vertex
			// repeating this procedure for all bones gives the new pose
			for (uint i = 0; i < skinWeightsList[boneIndex]._vertexIndices.size(); ++i) {
				uint32 vertexIndex = skinWeightsList[boneIndex]._vertexIndices[i];
				Math::Vector3d pos;
				pos.setData(vertexSrcData + vertexIndex * vertexSize);
				finalBoneMatrices[boneIndex].transform(&pos, true);
				pos *= skinWeightsList[boneIndex]._vertexWeights[i];

				for (uint j = 0; j < 3; ++j) {
					vertexDstData[vertexIndex * vertexSize + j] += pos.getData()[j];
				}
			}
		}

		// now we have to update the vertex normals as well, so prepare the bone transformations
		for (uint i = 0; i < skinWeightsList.size(); ++i) {
			finalBoneMatrices[i].transpose();
			finalBoneMatrices[i].inverse();
		}

		// reset so we can form the weighted sums
		for (uint32 i = 0; i < vertexCount; ++i) {
			for (int j = 0; j < 3; ++j) {
				vertexDstData[i * vertexSize + normalOffset + j] = 0.0f;
			}
		}

		for (uint boneIndex = 0; boneIndex < skinWeightsList.size(); ++boneIndex) {
			for (uint i = 0; i < skinWeightsList[boneIndex]._vertexIndices.size(); ++i) {
				uint32 vertexIndex = skinWeightsList[boneIndex]._vertexIndices[i];
				Math::Vector3d pos;
				pos.setData(vertexSrcData + vertexIndex * vertexSize + normalOffset);
				finalBoneMatrices[boneIndex].transform(&pos, true);
				pos *= skinWeightsList[boneIndex]._vertexWeights[i];

				for (uint j = 0; j < 3; ++j) {
					vertexDstData[vertexIndex * vertexSize + normalOffset + j] += pos.getData()[j];
				}
			}
		}

		//updateNormals();
	} else { // update static
		for (uint32 i = 0; i < vertexCount; ++i) {
			float *vertexSrcData = (float *)_staticMesh->getVertexBuffer().ptr();
			Math::Vector3d pos(vertexSrcData + i * vertexSize);
			parentFrame->getCombinedMatrix()->transform(&pos, true);

			for (uint j = 0; j < 3; ++j) {
				vertexDstData[i * vertexSize + j] = pos.getData()[j];
			}
		}
	}

	updateBoundingBox();

	return true;
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
	Common::Array<uint16> edges(indexDataSize * 2, 0);

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

	float *vertexData = (float *)_blendedMesh->getVertexBuffer().ptr();
	if (vertexData == nullptr) {
		return false;
	}
	uint32 vertexSize = DXGetFVFVertexSize(_blendedMesh->getFVF()) / sizeof(float);

	bool res = false;

	uint32 *indexData = (uint32 *)_blendedMesh->getIndexBuffer().ptr();
	uint32 indexDataSize = _blendedMesh->getIndexBuffer().size() / sizeof(uint32);
	for (uint32 i = 0; i < indexDataSize; i += 3) {
		uint32 index1 = indexData[i + 0];
		uint32 index2 = indexData[i + 1];
		uint32 index3 = indexData[i + 2];

		Math::Vector3d v0;
		v0.setData(&vertexData[index1 * vertexSize]);
		Math::Vector3d v1;
		v1.setData(&vertexData[index2 * vertexSize]);
		Math::Vector3d v2;
		v2.setData(&vertexData[index3 * vertexSize]);

		if (isnan(v0.x()))
			continue;

		Math::Vector3d intersection;
		if (lineIntersectsTriangle(*pickRayOrig, *pickRayDir, v0, v1, v2, intersection.x(), intersection.y(), intersection.z())) {
			res = true;
			break;
		}
	}

	return res;
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

void XMesh::updateBoundingBox() {
	float *vertexData = (float *)_blendedMesh->getVertexBuffer().ptr();
	if (vertexData == nullptr) {
		return;
	}
	uint32 vertexSize = DXGetFVFVertexSize(_blendedMesh->getFVF()) / sizeof(float);
	uint32 vertexCount = _blendedMesh->getNumVertices();
	if (vertexCount == 0) {
		return;
	}

	_BBoxStart.setData(&vertexData[0]);
	_BBoxEnd.setData(&vertexData[0]);

	for (uint16 i = 1; i < vertexCount; ++i) {
		Math::Vector3d v;
		v.setData(&vertexData[i * vertexSize]);

		_BBoxStart.x() = MIN(_BBoxStart.x(), v.x());
		_BBoxStart.y() = MIN(_BBoxStart.y(), v.y());
		_BBoxStart.z() = MIN(_BBoxStart.z(), v.z());

		_BBoxEnd.x() = MAX(_BBoxEnd.x(), v.x());
		_BBoxEnd.y() = MAX(_BBoxEnd.y(), v.y());
		_BBoxEnd.z() = MAX(_BBoxEnd.z(), v.z());
	}
}

} // namespace Wintermute
