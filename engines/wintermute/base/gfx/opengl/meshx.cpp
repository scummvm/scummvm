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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/base/gfx/opengl/material.h"
#include "engines/wintermute/base/gfx/opengl/meshx.h"
#include "engines/wintermute/base/gfx/opengl/shadow_volume.h"
#include "engines/wintermute/base/gfx/x/frame_node.h"
#include "engines/wintermute/base/gfx/x/loader_x.h"
#include "engines/wintermute/base/gfx/x/modelx.h"
#include "engines/wintermute/math/math_util.h"

namespace Wintermute {

// define constant to make it available to the linker
const uint32 MeshX::kNullIndex;

MeshX::MeshX(Wintermute::BaseGame *inGame) : BaseNamedObject(inGame),
	_BBoxStart(0.0f, 0.0f, 0.0f), _BBoxEnd(0.0f, 0.0f, 0.0f),
	_vertexData(nullptr), _vertexPositionData(nullptr),
	_vertexCount(0), _indexData(nullptr), _indexCount(0), _numAttrs(0), _skinnedMesh(false) {
}

MeshX::~MeshX() {
	delete[] _vertexData;
	delete[] _vertexPositionData;
	delete[] _indexData;

	for (uint32 i = 0; i < _materials.size(); i++) {
		delete _materials[i];
	}

	_materials.clear();
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::loadFromX(const Common::String &filename, XFileLexer &lexer, Common::HashMap<Common::String, Material *> materialDefinitions) {
	bool res = true;

	lexer.advanceToNextToken(); // skip the name
	lexer.advanceOnOpenBraces();
	_vertexCount = lexer.readInt();

	// vertex format for .X meshes will be position + normals + textures
	_vertexData = new float[kVertexComponentCount * _vertexCount]();
	_vertexPositionData = new float[3 * _vertexCount]();

	parsePositionCoords(lexer);

	int faceCount = lexer.readInt();
	// we should be able to assume for now that
	// we are only dealing with triangles
	_indexData = new uint16[faceCount * 3]();
	_indexCount = faceCount * 3;

	parseFaces(lexer, faceCount);

	while (!lexer.eof()) {
		if (lexer.tokenIsIdentifier("MeshTextureCoords")) {
			lexer.advanceToNextToken();
			lexer.advanceToNextToken();
			lexer.advanceOnOpenBraces();

			parseTextureCoords(lexer);
		} else if (lexer.tokenIsIdentifier("MeshNormals")) {
			lexer.advanceToNextToken();
			lexer.advanceToNextToken();
			lexer.advanceOnOpenBraces();

			parseNormalCoords(lexer);
		} else if (lexer.tokenIsIdentifier("MeshMaterialList")) {
			lexer.advanceToNextToken();
			lexer.advanceToNextToken();
			lexer.advanceOnOpenBraces();

			parseMaterials(lexer, faceCount, filename, materialDefinitions);
		} else if (lexer.tokenIsIdentifier("Material")) {
			lexer.advanceToNextToken();
			Material *mat = new Material(_gameRef);
			mat->loadFromX(lexer, filename);
			_materials.add(mat);

			// one material = one index range
			_numAttrs = 1;
			_indexRanges.push_back(0);
			_indexRanges.push_back(_indexCount);
		} else if (lexer.tokenIsIdentifier("XSkinMeshHeader")) {
			lexer.advanceToNextToken();
			lexer.advanceOnOpenBraces();

			// if any of this is zero, we should have an unskinned mesh
			int maxSkinWeightsPerVertex = lexer.readInt();
			int maxSkinWeightsPerFace = lexer.readInt();
			int boneCount = lexer.readInt();

			_skinnedMesh = boneCount > 0;

			lexer.advanceToNextToken(); // skip semicolon
		} else if (lexer.tokenIsIdentifier("SkinWeights")) {
			// but now we certainly should have a skinned mesh
			_skinnedMesh = true;
			lexer.advanceToNextToken();
			lexer.advanceOnOpenBraces();

			parseSkinWeights(lexer);
		} else if (lexer.tokenIsIdentifier()) {
			while (!lexer.reachedClosedBraces()) {
				lexer.advanceToNextToken();
			}

			lexer.advanceToNextToken(); // skip closed braces
		} else if (lexer.reachedClosedBraces()) {
			lexer.advanceToNextToken(); // skip closed braces

			generateAdjacency();

			return true;
		} else {
			warning("MeshX::loadFromX unknown token %i encountered", lexer.getTypeOfToken());
			lexer.advanceToNextToken();
		}
	}

	generateAdjacency();

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::generateAdjacency() {
	_adjacency = Common::Array<uint32>(_indexCount, kNullIndex);

	for (uint32 i = 0; i < _indexCount / 3; ++i) {
		for (uint32 j = i + 1; j < _indexCount / 3; ++j) {
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

bool MeshX::adjacentEdge(uint16 index1, uint16 index2, uint16 index3, uint16 index4) {
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
bool MeshX::findBones(FrameNode *rootFrame) {
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
			warning("MeshXOpenGL::findBones could not find bone %s", skinWeightsList[i]._boneName.c_str());
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::update(FrameNode *parentFrame) {
	if (_vertexData == nullptr) {
		return false;
	}

	bool res = false;

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

//		updateNormals();
	} else { // update static
		warning("MeshXOpenGL::update update of static mesh is not implemented yet");
	}

	updateBoundingBox();

	return res;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::updateShadowVol(ShadowVolume *shadow, Math::Matrix4 &modelMat, const Math::Vector3d &light, float extrusionDepth) {
	if (_vertexData == nullptr) {
		return false;
	}

	Math::Vector3d invLight = light;
	Math::Matrix4 matInverseModel = modelMat;
	matInverseModel.inverse();
	matInverseModel.transform(&invLight, false);

	uint32 numEdges = 0;

	Common::Array<bool> isFront(_indexCount / 3, false);

	// First pass : for each face, record if it is front or back facing the light
	for (uint32 i = 0; i < _indexCount / 3; i++) {
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
	Common::Array<uint16> edges(_indexCount * 2, 0);

	// First pass : for each face, record if it is front or back facing the light
	for (uint32 i = 0; i < _indexCount / 3; i++) {
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
bool MeshX::pickPoly(Math::Vector3d *pickRayOrig, Math::Vector3d *pickRayDir) {
	if (_vertexData == nullptr) {
		return false;
	}

	bool res = false;

	for (uint16 i = 0; i < _indexCount; i += 3) {
		uint16 index1 = _indexData[i + 0];
		uint16 index2 = _indexData[i + 1];
		uint16 index3 = _indexData[i + 2];

		Math::Vector3d v0;
		v0.setData(&_vertexData[index1 * kVertexComponentCount + kPositionOffset]);
		Math::Vector3d v1;
		v1.setData(&_vertexData[index2 * kVertexComponentCount + kPositionOffset]);
		Math::Vector3d v2;
		v2.setData(&_vertexData[index3 * kVertexComponentCount + kPositionOffset]);

		Math::Vector3d intersection;
		if (lineIntersectsTriangle(*pickRayOrig, *pickRayDir, v0, v1, v2, intersection.x(), intersection.y(), intersection.z())) {
			res = true;
			break;
		}
	}

	return res;
}

////////////////////////////////////////////////////////////////////////////
bool MeshX::setMaterialSprite(const Common::String &matName, BaseSprite *sprite) {
	for (uint32 i = 0; i < _materials.size(); i++) {
		if (_materials[i]->getName() && _materials[i]->getName() == matName) {
			_materials[i]->setSprite(sprite);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::setMaterialTheora(const Common::String &matName, VideoTheoraPlayer *theora) {
	for (uint32 i = 0; i < _materials.size(); i++) {
		if (_materials[i]->getName() && _materials[i]->getName() == matName) {
			_materials[i]->setTheora(theora);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::invalidateDeviceObjects() {
	// release buffers here

	for (uint32 i = 0; i < _materials.size(); i++) {
		_materials[i]->invalidateDeviceObjects();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::restoreDeviceObjects() {
	for (uint32 i = 0; i < _materials.size(); i++) {
		_materials[i]->restoreDeviceObjects();
	}

	if (_skinnedMesh) {
		return generateAdjacency();
	} else {
		return true;
	}
}

bool MeshX::parsePositionCoords(XFileLexer &lexer) {
	for (uint i = 0; i < _vertexCount; ++i) {
		for (int j = 0; j < 3; ++j) {
			_vertexPositionData[i * 3 + j] = lexer.readFloat();
			_vertexData[i * kVertexComponentCount + kPositionOffset + j] = _vertexPositionData[i * 3 + j];
		}

		_vertexPositionData[i * 3 + 2] *= -1.0f;
		_vertexData[i * kVertexComponentCount + kPositionOffset + 2] *= -1.0f;

		lexer.skipTerminator(); // skip semicolon
	}

	return true;
}

bool MeshX::parseFaces(XFileLexer &lexer, int faceCount) {
	for (int i = 0; i < faceCount; ++i) {
		int indexCount = lexer.readInt();

		// we can add something to triangulize faces later if the need arises
		if (indexCount != 3) {
			warning("MeshXOpenGL::loadFromX non triangle faces are not supported yet");
			return false;
		}

		for (int j = 0; j < 3; ++j) {
			_indexData[i * 3 + j] = lexer.readInt();
		}

		// swap to change winding and make it consistent with the coordinate mirroring
		SWAP(_indexData[i * 3 + 0], _indexData[i * 3 + 2]);

		lexer.skipTerminator(); // skip semicolon
	}

	return true;
}

bool MeshX::parseTextureCoords(XFileLexer &lexer) {
	// should be the same as _vertexCount
	int textureCoordCount = lexer.readInt();

	for (int i = 0; i < textureCoordCount; ++i) {
		_vertexData[i * kVertexComponentCount + kTextureCoordOffset + 0] = lexer.readFloat();
		_vertexData[i * kVertexComponentCount + kTextureCoordOffset + 1] = lexer.readFloat();
		lexer.skipTerminator(); // skip semicolon
	}

	if (lexer.reachedClosedBraces()) {
		lexer.advanceToNextToken();
	} else {
		warning("Missing } in mesh object");
	}

	return true;
}

bool MeshX::parseNormalCoords(XFileLexer &lexer) {
	// should be the same as _vertex count
	uint vertexNormalCount = lexer.readInt();
//	assert(vertexNormalCount == _vertexCount);

	_vertexNormalData = new float[3 * _vertexCount]();

	for (uint i = 0; i < vertexNormalCount; ++i) {
		_vertexData[i * kVertexComponentCount + kNormalOffset] = lexer.readFloat();
		_vertexNormalData[i * 3 + 0] = _vertexData[i * kVertexComponentCount + kNormalOffset];
		_vertexData[i * kVertexComponentCount + kNormalOffset + 1] = lexer.readFloat();
		_vertexNormalData[i * 3 + 1] = _vertexData[i * kVertexComponentCount + kNormalOffset + 1];
		// mirror z coordinate to change to OpenGL coordinate system
		_vertexData[i * kVertexComponentCount + kNormalOffset + 2] = -lexer.readFloat();
		_vertexNormalData[i * 3 + 2] = _vertexData[i * kVertexComponentCount + kNormalOffset + 2];
		lexer.skipTerminator(); // skip semicolon
	}

	// we ignore face normals for now
	while (!lexer.reachedClosedBraces()) {
		lexer.advanceToNextToken();
	}

	lexer.advanceToNextToken(); // skip closed braces

	return true;
}

bool MeshX::parseMaterials(XFileLexer &lexer, int faceCount, const Common::String &filename, Common::HashMap<Common::String, Material *> materialDefinitions) {
	// there can be unused materials inside a .X file
	// so this piece of information is probably useless
	int materialCount = lexer.readInt();
	// should be the same as faceCount
	int faceMaterialCount = lexer.readInt();
	assert(faceMaterialCount = faceCount);

	// from looking at the wme3d sources and MSDN,
	// I would say that faces using the same material
	// are layed out as a contiguous block and the
	// material index is only increasing
	// in case this isn't true it might be a good
	// idea to split the mesh
	_indexRanges.push_back(0);
	int currentMaterialIndex = lexer.readInt();

	for (int i = 1; i < faceMaterialCount; ++i) {
		int currentMaterialIndexTmp = lexer.readInt();

		// again, this assumes that face indices are only increasing
		if (currentMaterialIndex < currentMaterialIndexTmp) {
			currentMaterialIndex = currentMaterialIndexTmp;
			_indexRanges.push_back(3 * i);
		}
	}

	_indexRanges.push_back(3 * faceCount);
	_numAttrs = _indexRanges.size() - 1;

	while (!lexer.eof()) {
		if (lexer.tokenIsIdentifier("Material")) {
			lexer.advanceToNextToken();
			Material *mat = new Material(_gameRef);
			mat->loadFromX(lexer, filename);
			_materials.add(mat);
		} else if (lexer.tokenIsIdentifier()) {
			while (!lexer.reachedClosedBraces()) {
				lexer.advanceToNextToken();
			}

			lexer.advanceToNextToken(); // skip closed braces
		} else if (lexer.reachedClosedBraces()) {
			break;
		} else if (lexer.tokenIsOfType(OPEN_BRACES)) {
			lexer.advanceToNextToken();
			Common::String materialReference = lexer.tokenToString();
			Material *material = materialDefinitions.getVal(materialReference);
			_materials.add(material);
			lexer.advanceToNextToken();
			lexer.advanceToNextToken();
		} else {
			warning("MeshXOpenGL::loadFromX unknown token %i encountered while loading materials", lexer.getTypeOfToken());
			break;
		}
	}

	lexer.advanceToNextToken(); // skip closed braces

	return true;
}

bool MeshX::parseSkinWeights(XFileLexer &lexer) {
	skinWeightsList.resize(skinWeightsList.size() + 1);
	SkinWeights &currSkinWeights = skinWeightsList.back();

	currSkinWeights._boneName = lexer.readString();

	int weightCount = lexer.readInt();
	currSkinWeights._vertexIndices.resize(weightCount);
	currSkinWeights._vertexWeights.resize(weightCount);

	for (int i = 0; i < weightCount; ++i) {
		currSkinWeights._vertexIndices[i] = lexer.readInt();
	}

	if (weightCount == 0) {
		lexer.advanceToNextToken();
	}

	for (int i = 0; i < weightCount; ++i) {
		currSkinWeights._vertexWeights[i] = lexer.readFloat();
	}

	if (weightCount == 0) {
		lexer.advanceToNextToken();
	}

	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {
			currSkinWeights._offsetMatrix(c, r) = lexer.readFloat();
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

	lexer.skipTerminator(); // semicolon of matrix
	lexer.advanceToNextToken(); // closed braces of skin weights object

	return true;
}

void MeshX::updateBoundingBox() {
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
