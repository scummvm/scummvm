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
#include "engines/wintermute/base/gfx/xframe_node.h"
#include "engines/wintermute/base/gfx/xfile_loader.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/math/math_util.h"

namespace Wintermute {

// define constant to make it available to the linker
const uint32 XMesh::kNullIndex;

XMesh::XMesh(Wintermute::BaseGame *inGame) : BaseNamedObject(inGame) {
	_numAttrs = 0;
	_skinnedMesh = false;

	_BBoxStart = Math::Vector3d(0.0f, 0.0f, 0.0f);
	_BBoxEnd = Math::Vector3d(0.0f, 0.0f, 0.0f);

	_vertexData = nullptr;
	_vertexPositionData = nullptr;
	_vertexNormalData = nullptr;
	_vertexCount = 0;
}

XMesh::~XMesh() {
	delete[] _vertexData;
	delete[] _vertexPositionData;
	delete[] _vertexNormalData;

	_materials.clear();
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::loadFromXData(const Common::String &filename, XFileData *xobj, Common::Array<MaterialReference> &materialReferences) {
	// get name
	if (!XModel::loadName(this, xobj)) {
		BaseEngine::LOG(0, "Error loading mesh name");
		return false;
	}

	XMeshObject *mesh = xobj->getXMeshObject();
	if (!mesh) {
		BaseEngine::LOG(0, "Error loading skin mesh");
		return false;
	}

	_vertexCount = mesh->_numVertices;

	// vertex format for .X meshes will be position + normals + textures
	_vertexData = new float[kVertexComponentCount * _vertexCount]();
	_vertexPositionData = new float[3 * _vertexCount]();
	// we already know how big this is supposed to be
	// TODO: might have to generate normals if file does not contain any
	_vertexNormalData = new float[3 * _vertexCount]();

	parsePositionCoords(mesh);

	int faceCount = mesh->_numFaces;

	Common::Array<int> indexCountPerFace;

	parseFaces(mesh, faceCount, indexCountPerFace);

	uint numChildren = 0;
	xobj->getChildren(numChildren);

	for (uint32 i = 0; i < numChildren; i++) {
		XFileData xchildData;
		XClassType objectType;
		if (xobj->getChild(i, xchildData)) {
			if (xchildData.getType(objectType)) {
				if (objectType == kXClassMeshTextureCoords) {
					parseTextureCoords(&xchildData);
				} else if (objectType == kXClassMeshNormals) {
					parseNormalCoords(&xchildData);
				} else if (objectType == kXClassMeshMaterialList) {
					parseMaterials(&xchildData, faceCount, filename, materialReferences, indexCountPerFace);
				} else if (objectType == kXClassMaterial) {
					Material *mat = new Material(_gameRef);
					mat->loadFromX(&xchildData, filename);
					_materials.add(mat);

					// one material = one index range
					_numAttrs = 1;
					_indexRanges.push_back(0);
					_indexRanges.push_back(_indexData.size());
				} else if (objectType == kXClassSkinMeshHeader) {
					int boneCount = xchildData.getXSkinMeshHeaderObject()->_nBones;
					_skinnedMesh = boneCount > 0;
				} else if (objectType == kXClassSkinWeights) {
					_skinnedMesh = true;
					parseSkinWeights(&xchildData);
				} else if (objectType == kXClassDeclData) {
					parseVertexDeclaration(&xchildData);
				}
			}
		}
	}

	generateAdjacency();

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::generateAdjacency() {
	_adjacency = Common::Array<uint32>(_indexData.size(), kNullIndex);

	for (uint32 i = 0; i < _indexData.size() / 3; ++i) {
		for (uint32 j = i + 1; j < _indexData.size() / 3; ++j) {
			for (int edge1 = 0; edge1 < 3; ++edge1) {
				uint16 index1 = _indexData[i * 3 + edge1];
				uint16 index2 = _indexData[i * 3 + (edge1 + 1) % 3];

				for (int edge2 = 0; edge2 < 3; ++edge2) {
					uint16 index3 = _indexData[j * 3 + edge2];
					uint16 index4 = _indexData[j * 3 + (edge2 + 1) % 3];

					if (_adjacency[i * 3 + edge1] == kNullIndex && _adjacency[j * 3 + edge2] == kNullIndex && adjacentEdge(index1, index2, index3, index4)) {
						_adjacency[i * 3 + edge1] = j;
						_adjacency[j * 3 + edge2] = i;

						break;
					}
				}
			}
		}
	}

	return true;
}

bool XMesh::adjacentEdge(uint16 index1, uint16 index2, uint16 index3, uint16 index4) {
	Math::Vector3d vertex1(_vertexPositionData + 3 * index1);
	Math::Vector3d vertex2(_vertexPositionData + 3 * index2);
	Math::Vector3d vertex3(_vertexPositionData + 3 * index3);
	Math::Vector3d vertex4(_vertexPositionData + 3 * index4);

	// wme uses a function from the D3DX library, which takes in an epsilon for floating point comparison
	// wme passes in zero, so we just do a direct comparison
	if (vertex1 == vertex3 && vertex2 == vertex4) {
		return true;
	} else if (vertex1 == vertex4 && vertex2 == vertex3) {
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::findBones(FrameNode *rootFrame) {
	// normal meshes don't have bones
	if (!_skinnedMesh) {
		return true;
	}

	_boneMatrices.resize(skinWeightsList.size());

	for (uint i = 0; i < skinWeightsList.size(); ++i) {
		FrameNode *frame = rootFrame->findFrame(skinWeightsList[i]._boneName.c_str());

		if (frame) {
			_boneMatrices[i] = frame->getCombinedMatrix();
		} else {
			warning("XMeshOpenGL::findBones could not find bone %s", skinWeightsList[i]._boneName.c_str());
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::update(FrameNode *parentFrame) {
	if (_vertexData == nullptr) {
		return false;
	}

	// update skinned mesh
	if (_skinnedMesh) {
		BaseArray<Math::Matrix4> finalBoneMatrices;
		finalBoneMatrices.resize(_boneMatrices.size());

		for (uint i = 0; i < skinWeightsList.size(); ++i) {
			finalBoneMatrices[i] = *_boneMatrices[i] * skinWeightsList[i]._offsetMatrix;
		}

		// the new vertex coordinates are the weighted sum of the product
		// of the combined bone transformation matrices and the static pose coordinates
		// to be able too add the weighted summands together, we reset everything to zero first
		for (uint32 i = 0; i < _vertexCount; ++i) {
			for (int j = 0; j < 3; ++j) {
				_vertexData[i * kVertexComponentCount + kPositionOffset + j] = 0.0f;
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
				pos.setData(_vertexPositionData + vertexIndex * 3);
				finalBoneMatrices[boneIndex].transform(&pos, true);
				pos *= skinWeightsList[boneIndex]._vertexWeights[i];

				for (uint j = 0; j < 3; ++j) {
					_vertexData[vertexIndex * kVertexComponentCount + kPositionOffset + j] += pos.getData()[j];
				}
			}
		}

		// now we have to update the vertex normals as well, so prepare the bone transformations
		for (uint i = 0; i < skinWeightsList.size(); ++i) {
			finalBoneMatrices[i].transpose();
			finalBoneMatrices[i].inverse();
		}

		// reset so we can form the weighted sums
		for (uint32 i = 0; i < _vertexCount; ++i) {
			for (int j = 0; j < 3; ++j) {
				_vertexData[i * kVertexComponentCount + kNormalOffset + j] = 0.0f;
			}
		}

		for (uint boneIndex = 0; boneIndex < skinWeightsList.size(); ++boneIndex) {
			for (uint i = 0; i < skinWeightsList[boneIndex]._vertexIndices.size(); ++i) {
				uint32 vertexIndex = skinWeightsList[boneIndex]._vertexIndices[i];
				Math::Vector3d pos;
				pos.setData(_vertexNormalData + vertexIndex * 3);
				finalBoneMatrices[boneIndex].transform(&pos, true);
				pos *= skinWeightsList[boneIndex]._vertexWeights[i];

				for (uint j = 0; j < 3; ++j) {
					_vertexData[vertexIndex * kVertexComponentCount + kNormalOffset + j] += pos.getData()[j];
				}
			}
		}

	//updateNormals();
	} else { // update static
		for (uint32 i = 0; i < _vertexCount; ++i) {
			Math::Vector3d pos(_vertexPositionData + 3 * i);
			parentFrame->getCombinedMatrix()->transform(&pos, true);

			for (uint j = 0; j < 3; ++j) {
				_vertexData[i * kVertexComponentCount + kPositionOffset + j] = pos.getData()[j];
			}
		}
	}

	updateBoundingBox();

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XMesh::updateShadowVol(ShadowVolume *shadow, Math::Matrix4 &modelMat, const Math::Vector3d &light, float extrusionDepth) {
	if (_vertexData == nullptr) {
		return false;
	}

	Math::Vector3d invLight = light;
	Math::Matrix4 matInverseModel = modelMat;
	matInverseModel.inverse();
	matInverseModel.transform(&invLight, false);

	uint32 numEdges = 0;

	Common::Array<bool> isFront(_indexData.size() / 3, false);

	// First pass : for each face, record if it is front or back facing the light
	for (uint32 i = 0; i < _indexData.size() / 3; i++) {
		uint16 index0 = _indexData[3 * i + 0];
		uint16 index1 = _indexData[3 * i + 1];
		uint16 index2 = _indexData[3 * i + 2];

		Math::Vector3d v0(_vertexData + index0 * kVertexComponentCount + kPositionOffset);
		Math::Vector3d v1(_vertexData + index1 * kVertexComponentCount + kPositionOffset);
		Math::Vector3d v2(_vertexData + index2 * kVertexComponentCount + kPositionOffset);

		// Transform vertices or transform light?
		Math::Vector3d vNormal = Math::Vector3d::crossProduct(v2 - v1, v1 - v0);

		if (Math::Vector3d::dotProduct(vNormal, invLight) >= 0.0f) {
			isFront[i] = false; // back face
		} else {
			isFront[i] = true; // front face
		}
	}

	// Allocate a temporary edge list
	Common::Array<uint16> edges(_indexData.size() * 2, 0);

	// First pass : for each face, record if it is front or back facing the light
	for (uint32 i = 0; i < _indexData.size() / 3; i++) {
		if (isFront[i]) {
			uint16 wFace0 = _indexData[3 * i + 0];
			uint16 wFace1 = _indexData[3 * i + 1];
			uint16 wFace2 = _indexData[3 * i + 2];

			uint32 adjacent0 = _adjacency[3 * i + 0];
			uint32 adjacent1 = _adjacency[3 * i + 1];
			uint32 adjacent2 = _adjacency[3 * i + 2];

			if (adjacent0 == kNullIndex || isFront[adjacent0] == false) {
				//	add edge v0-v1
				edges[2 * numEdges + 0] = wFace0;
				edges[2 * numEdges + 1] = wFace1;
				numEdges++;
			}
			if (adjacent1 == kNullIndex || isFront[adjacent1] == false) {
				//	add edge v1-v2
				edges[2 * numEdges + 0] = wFace1;
				edges[2 * numEdges + 1] = wFace2;
				numEdges++;
			}
			if (adjacent2 == kNullIndex || isFront[adjacent2] == false) {
				//	add edge v2-v0
				edges[2 * numEdges + 0] = wFace2;
				edges[2 * numEdges + 1] = wFace0;
				numEdges++;
			}
		}
	}

	for (uint32 i = 0; i < numEdges; i++) {
		Math::Vector3d v1(_vertexData + edges[2 * i + 0] * kVertexComponentCount + kPositionOffset);
		Math::Vector3d v2(_vertexData + edges[2 * i + 1] * kVertexComponentCount + kPositionOffset);
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
	if (_vertexData == nullptr) {
		return false;
	}

	bool res = false;

	for (uint16 i = 0; i < _indexData.size(); i += 3) {
		uint16 index1 = _indexData[i + 0];
		uint16 index2 = _indexData[i + 1];
		uint16 index3 = _indexData[i + 2];

		Math::Vector3d v0;
		v0.setData(&_vertexData[index1 * kVertexComponentCount + kPositionOffset]);
		Math::Vector3d v1;
		v1.setData(&_vertexData[index2 * kVertexComponentCount + kPositionOffset]);
		Math::Vector3d v2;
		v2.setData(&_vertexData[index3 * kVertexComponentCount + kPositionOffset]);

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
	// release buffers here

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

	if (_skinnedMesh) {
		return generateAdjacency();
	} else {
		return true;
	}
}

bool XMesh::parsePositionCoords(XMeshObject *mesh) {
	for (uint i = 0; i < _vertexCount; ++i) {
		_vertexPositionData[i * 3 + 0] = mesh->_vertices[i]._x;
		_vertexPositionData[i * 3 + 1] = mesh->_vertices[i]._y;
		_vertexPositionData[i * 3 + 2] = mesh->_vertices[i]._z;
		for (int j = 0; j < 3; ++j) {
			_vertexData[i * kVertexComponentCount + kPositionOffset + j] = _vertexPositionData[i * 3 + j];
		}

		_vertexPositionData[i * 3 + 2] *= -1.0f;
		_vertexData[i * kVertexComponentCount + kPositionOffset + 2] *= -1.0f;
	}

	return true;
}

bool XMesh::parseFaces(XMeshObject *mesh, int faceCount, Common::Array<int> &indexCountPerFace) {
	for (int i = 0; i < faceCount; ++i) {
		XMeshFace *face = &mesh->_faces[i];
		int indexCount = face->_numFaceVertexIndices;
		if (indexCount == 3) {
			uint16 index1 = face->_faceVertexIndices[0];
			uint16 index2 = face->_faceVertexIndices[1];
			uint16 index3 = face->_faceVertexIndices[2];

			_indexData.push_back(index3);
			_indexData.push_back(index2);
			_indexData.push_back(index1);

			indexCountPerFace.push_back(3);
		} else if (indexCount == 4) {
			uint16 index1 = face->_faceVertexIndices[0];
			uint16 index2 = face->_faceVertexIndices[1];
			uint16 index3 = face->_faceVertexIndices[2];
			uint16 index4 = face->_faceVertexIndices[3];

			_indexData.push_back(index3);
			_indexData.push_back(index2);
			_indexData.push_back(index1);

			_indexData.push_back(index4);
			_indexData.push_back(index3);
			_indexData.push_back(index1);

			indexCountPerFace.push_back(6);
		} else {
			warning("XMeshOpenGL::loadFromX faces with more than four vertices are not supported");
			return false;
		}
	}

	return true;
}

bool XMesh::parseTextureCoords(XFileData *xobj) {
	XMeshTextureCoordsObject *texCoords = xobj->getXMeshTextureCoordsObject();
	if (!texCoords)
		return false;
	// should be the same as _vertexCount
	int textureCoordCount = texCoords->_numTextureCoords;

	for (int i = 0; i < textureCoordCount; ++i) {
		_vertexData[i * kVertexComponentCount + kTextureCoordOffset + 0] = texCoords->_textureCoords[i]._u;
		_vertexData[i * kVertexComponentCount + kTextureCoordOffset + 1] = texCoords->_textureCoords[i]._v;
	}

	return true;
}

bool XMesh::parseNormalCoords(XFileData *xobj) {
	XMeshNormalsObject *normals = xobj->getXMeshNormalsObject();
	if (!normals)
		return false;
	// should be the same as _vertex count
	uint vertexNormalCount = normals->_numNormals;
	//assert(vertexNormalCount == _vertexCount);

	Common::Array<float> vertexNormalData;
	vertexNormalData.resize(3 * vertexNormalCount);

	for (uint i = 0; i < vertexNormalCount; ++i) {
		vertexNormalData[i * 3 + 0] = normals->_normals[i]._x;
		vertexNormalData[i * 3 + 1] = normals->_normals[i]._y;
		// mirror z coordinate to change to OpenGL coordinate system
		vertexNormalData[i * 3 + 2] = -normals->_normals[i]._z;
	}

	uint faceNormalCount = normals->_numFaceNormals;
	Common::Array<int> faceNormals;

	for (uint i = 0; i < faceNormalCount; ++i) {
		XMeshFace *normalFace = &normals->_faceNormals[i];
		int indexCount = normalFace->_numFaceVertexIndices;

		if (indexCount == 3) {
			uint16 index1 = normalFace->_faceVertexIndices[0];
			uint16 index2 = normalFace->_faceVertexIndices[1];
			uint16 index3 = normalFace->_faceVertexIndices[2];

			faceNormals.push_back(index3);
			faceNormals.push_back(index2);
			faceNormals.push_back(index1);
		} else if (indexCount == 4) {
			uint16 index1 = normalFace->_faceVertexIndices[0];
			uint16 index2 = normalFace->_faceVertexIndices[1];
			uint16 index3 = normalFace->_faceVertexIndices[2];
			uint16 index4 = normalFace->_faceVertexIndices[3];

			faceNormals.push_back(index3);
			faceNormals.push_back(index2);
			faceNormals.push_back(index1);

			faceNormals.push_back(index4);
			faceNormals.push_back(index3);
			faceNormals.push_back(index1);
		} else {
			warning("XMeshOpenGL::loadFromX faces with more than four vertices are not supported");
			return false;
		}
	}

	assert(_indexData.size() == faceNormals.size());

	for (uint i = 0; i < faceNormals.size(); ++i) {
		uint16 vertexIndex = _indexData[i];
		int normalIndex = faceNormals[i];

		for (int j = 0; j < 3; ++j) {
			_vertexData[vertexIndex * kVertexComponentCount + kNormalOffset + j] = vertexNormalData[3 * normalIndex + j];
			_vertexNormalData[3 * vertexIndex + j] = vertexNormalData[3 * normalIndex + j];
		}
	}

	return true;
}

bool XMesh::parseMaterials(XFileData *xobj, int faceCount, const Common::String &filename, Common::Array<MaterialReference> &materialReferences, const Common::Array<int> &indexCountPerFace) {
	XMeshMaterialListObject *materialList = xobj->getXMeshMaterialListObject();
	if (!materialList)
		return false;

	// there can be unused materials inside a .X file
	// so this piece of information is probably useless
	// should be the same as faceCount
	int faceMaterialCount = materialList->_numFaceIndexes;
	assert(faceMaterialCount == faceCount);

	_indexRanges.push_back(0);
	int currentMaterialIndex = materialList->_faceIndexes[0];
	_materialIndices.push_back(currentMaterialIndex);

	int currentIndex = indexCountPerFace[0];

	for (int i = 1; i < faceMaterialCount; ++i) {
		int currentMaterialIndexTmp = materialList->_faceIndexes[i];

		if (currentMaterialIndex != currentMaterialIndexTmp) {
			currentMaterialIndex = currentMaterialIndexTmp;
			_indexRanges.push_back(currentIndex);
			_materialIndices.push_back(currentMaterialIndex);
		}

		currentIndex += indexCountPerFace[i];
	}

	_indexRanges.push_back(currentIndex);
	_numAttrs = _indexRanges.size() - 1;

	uint numChildren = 0;
	xobj->getChildren(numChildren);

	for (uint32 i = 0; i < numChildren; i++) {
		XFileData xchildData;
		XClassType objectType;
		bool res = xobj->getChild(i, xchildData);
		if (res) {
			res = xchildData.getType(objectType);
			if (res) {
				if (xchildData.isReference()) {
					Common::String materialReference;
					xchildData.getName(materialReference);
					for (uint32 j = 0; j < materialReferences.size(); j++) {
						if (materialReferences[j]._name == materialReference) {
							_materials.add(materialReferences[j]._material);
							break;
						}
					}
				} else if (objectType == kXClassMaterial) {
					Material *mat = new Material(_gameRef);
					mat->loadFromX(&xchildData, filename);
					_materials.add(mat);
					MaterialReference materialReference;
					materialReference._material = mat;
					materialReferences.push_back(materialReference);
				}
			}
		}
	}

	return true;
}

bool XMesh::parseSkinWeights(XFileData *xobj) {
	XSkinWeightsObject *skinWeights = xobj->getXSkinWeightsObject();
	if (!skinWeights)
		return false;

	skinWeightsList.resize(skinWeightsList.size() + 1);
	SkinWeights &currSkinWeights = skinWeightsList.back();

	currSkinWeights._boneName = skinWeights->_transformNodeName;

	int weightCount = skinWeights->_numWeights;
	currSkinWeights._vertexIndices.resize(weightCount);
	currSkinWeights._vertexWeights.resize(weightCount);

	for (int i = 0; i < weightCount; ++i) {
		currSkinWeights._vertexIndices[i] = skinWeights->_vertexIndices[i];
	}

	for (int i = 0; i < weightCount; ++i) {
		currSkinWeights._vertexWeights[i] = skinWeights->_weights[i];
	}

	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {
			currSkinWeights._offsetMatrix(c, r) = skinWeights->_matrixOffset[r * 4 + c];
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

	return true;
}

bool XMesh::parseVertexDeclaration(XFileData *xobj) {
	XDeclDataObject *declData = xobj->getXDeclDataObject();
	if (!declData)
		return false;

	int vertexElementCount = declData->_numElements;

	// size of a vertex measured in four byte blocks
	int vertexSize = 0;
	int normalOffset = -1;
	int textureOffset = -1;

	for (int i = 0; i < vertexElementCount; ++i) {
		int type = declData->_elements[i]._type;
		int method = declData->_elements[i]._method;
		int usage = declData->_elements[i]._usage;
		int usageIndex = declData->_elements[i]._usageIndex;

		debug("Vertex Element: Type: %i, Method: %i, Usage: %i, Usage index: %i", type, method, usage, usageIndex);

		// we only care about normal vectors and texture coords
		switch (usage) {
		case 3:
			normalOffset = vertexSize;
			break;
		case 5:
			textureOffset = vertexSize;
			break;
		}

		// This is a first guess, based on
		// https://docs.microsoft.com/en-us/windows/win32/direct3d9/vertexelement
		switch (type) {
		case 0:
			vertexSize += 1;
			warning("D3DDECLTYPE_FLOAT1 encountered in .X model");
			break;
		case 1:
			vertexSize += 2;
			break;
		case 2:
			vertexSize += 3;
			break;
		case 3:
			vertexSize += 4;
			warning("D3DDECLTYPE_FLOAT4 encountered in .X model");
			break;
		case 4:
			vertexSize += 1;
			warning("D3DDECLTYPE_D3DCOLOR encountered in .X model");
			break;
		case 5:
			vertexSize += 1;
			warning("D3DDECLTYPE_UBYTE4 encountered in .X model");
			break;
		case 6:
			vertexSize += 2;
			warning("D3DDECLTYPE_SHORT2 encountered in .X model");
			break;
		case 7:
			vertexSize += 4;
			warning("D3DDECLTYPE_SHORT4 encountered in .X model");
			break;
		case 8:
			vertexSize += 1;
			warning("D3DDECLTYPE_UBYTE4N encountered in .X model");
			break;
		case 9:
			vertexSize += 2;
			warning("D3DDECLTYPE_SHORT2N encountered in .X model");
			break;
		case 10:
			vertexSize += 4;
			warning("D3DDECLTYPE_SHORT4N encountered in .X model");
			break;
		case 11:
			vertexSize += 2;
			warning("D3DDECLTYPE_USHORT2N encountered in .X model");
			break;
		case 12:
			vertexSize += 4;
			warning("D3DDECLTYPE_USHORT4N encountered in .X model");
			break;
		case 13:
			vertexSize += 3;
			warning("D3DDECLTYPE_UDEC3 encountered in .X model");
			break;
		case 14:
			vertexSize += 3;
			warning("D3DDECLTYPE_DEC3N encountered in .X model");
			break;
		case 15:
			vertexSize += 2;
			warning("D3DDECLTYPE_FLOAT16_2 encountered in .X model");
			break;
		case 16:
			vertexSize += 4;
			warning("D3DDECLTYPE_FLOAT16_4 encountered in .X model");
			break;
		default:
			warning("Unknown type in vertex declaration encountered");
			break;
		}
	}

	int dataSize = declData->_numData;
	Common::Array<uint32> data;
	data.reserve(dataSize);

	for (int i = 0; i < dataSize; ++i) {
		data.push_back(declData->_data[i]);
	}

	assert(dataSize % vertexSize == 0);
	assert(dataSize / vertexSize == static_cast<int>(_vertexCount));

	for (uint i = 0; i < _vertexCount; ++i) {
		if (normalOffset != -1) {
			float *vertexNormalData = reinterpret_cast<float *>(data.data() + vertexSize * i + normalOffset);

			for (int j = 0; j < 3; ++j) {
				_vertexNormalData[3 * i + j] = vertexNormalData[j];
				_vertexData[kVertexComponentCount * i + kNormalOffset + j] = vertexNormalData[j];
			}
		}

		if (textureOffset != -1) {
			float *vertexTextureCoordsData = reinterpret_cast<float *>(data.data() + vertexSize * i + textureOffset);

			_vertexData[kVertexComponentCount * i + kTextureCoordOffset + 0] = vertexTextureCoordsData[0];
			_vertexData[kVertexComponentCount * i + kTextureCoordOffset + 1] = vertexTextureCoordsData[1];
		}
	}

	return true;
}

void XMesh::updateBoundingBox() {
	if (_vertexData == nullptr || _vertexCount == 0) {
		return;
	}

	_BBoxStart.setData(&_vertexData[0 + kPositionOffset]);
	_BBoxEnd.setData(&_vertexData[0 + kPositionOffset]);

	for (uint16 i = 1; i < _vertexCount; ++i) {

		Math::Vector3d v;
		v.setData(&_vertexData[i * kVertexComponentCount + kPositionOffset]);

		_BBoxStart.x() = MIN(_BBoxStart.x(), v.x());
		_BBoxStart.y() = MIN(_BBoxStart.y(), v.y());
		_BBoxStart.z() = MIN(_BBoxStart.z(), v.z());

		_BBoxEnd.x() = MAX(_BBoxEnd.x(), v.x());
		_BBoxEnd.y() = MAX(_BBoxEnd.y(), v.y());
		_BBoxEnd.z() = MAX(_BBoxEnd.z(), v.z());
	}
}

} // namespace Wintermute
