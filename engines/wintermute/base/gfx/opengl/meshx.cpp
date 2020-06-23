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

#include "common/math.h"
#include "common/util.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/material.h"
#include "engines/wintermute/base/gfx/opengl/meshx.h"
#include "engines/wintermute/base/gfx/opengl/shadow_volume.h"
#include "engines/wintermute/base/gfx/x/frame_node.h"
#include "engines/wintermute/base/gfx/x/loader_x.h"
#include "engines/wintermute/base/gfx/x/modelx.h"
#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/math/math_util.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
MeshX::MeshX(BaseGame *inGame) : BaseNamedObject(inGame),
                                 _BBoxStart(0.0f, 0.0f, 0.0f), _BBoxEnd(0.0f, 0.0f, 0.0f),
                                 _numAttrs(0), _maxFaceInfluence(0),
                                 _vertexData(nullptr), _vertexPositionData(nullptr),
                                 _vertexCount(0), _indexData(nullptr), _indexCount(0),
                                 _skinAdjacency(nullptr), _adjacency(nullptr), _skinnedMesh(false) {
}

//////////////////////////////////////////////////////////////////////////
MeshX::~MeshX() {
	delete[] _adjacency;
	delete[] _skinAdjacency;
	delete[] _vertexData;
	delete[] _vertexPositionData;
	delete[] _indexData;

	for (uint32 i = 0; i < _materials.size(); i++) {
		delete _materials[i];
	}

	_materials.clear();
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::loadFromX(const Common::String &filename, XFileLexer &lexer) {
	bool res = true;

	lexer.advanceToNextToken(); // skip the name
	lexer.advanceOnOpenBraces();
	_vertexCount = readInt(lexer);

	// vertex format for .X meshes will be position + normals + textures
	_vertexData = new float[kVertexComponentCount * _vertexCount]();
	_vertexPositionData = new float[3 * _vertexCount]();

	parsePositionCoords(lexer);

	int faceCount = readInt(lexer);
	// we should be able to assume for now that
	// we are only dealing with triangles
	_indexData = new uint16[faceCount * 3]();
	_indexCount = faceCount * 3;

	parseFaces(lexer, faceCount);

	while (!lexer.eof()) {
		if (lexer.tokenIsIdentifier("MeshTextureCoords")) {
			lexer.advanceToNextToken();
			lexer.advanceOnOpenBraces();

			parseTextureCoords(lexer);
		} else if (lexer.tokenIsIdentifier("MeshNormals")) {
			lexer.advanceToNextToken();
			lexer.advanceOnOpenBraces();

			parseNormalCoords(lexer);
		} else if (lexer.tokenIsIdentifier("MeshMaterialList")) {
			lexer.advanceToNextToken();
			lexer.advanceOnOpenBraces();

			parseMaterials(lexer, faceCount, filename);
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
			int maxSkinWeightsPerVertex = readInt(lexer);
			int maxSkinWeightsPerFace = readInt(lexer);
			int boneCount = readInt(lexer);

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
			return true;
		} else {
			warning("MeshX::loadFromX unknown token %i encountered", lexer.getTypeOfToken());
			lexer.advanceToNextToken();
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::generateMesh() {
	return true;
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
			warning("MeshX::findBones could not find bone %s", skinWeightsList[i]._boneName.c_str());
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

		for (uint32 i = 0; i < _vertexCount; ++i) {
			for (int j = 0; j < 3; ++j) {
				_vertexData[i * kVertexComponentCount + kPositionOffset + j] = 0.0f;
			}
		}

		for (uint boneIndex = 0; boneIndex < skinWeightsList.size(); ++boneIndex) {
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
	} else { // update static
		warning("MeshX::update update of static mesh is not implemented yet");
	}

	updateBoundingBox();

	return res;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::updateShadowVol(ShadowVolume *shadow, Math::Matrix4 &modelMat, const Math::Vector3d &light, float extrusionDepth) {
	if (_vertexData == nullptr) {
		return false;
	}

	return shadow->addMesh(_adjacency, modelMat, light, extrusionDepth);
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::render(ModelX *model) {
	if (_vertexData == nullptr) {
		return false;
	}

	bool res = false;

	// is this correct?
	for (uint32 i = 0; i < _numAttrs; i++) {
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _materials[i]->_diffuse.data);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, _materials[i]->_specular.data);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, _materials[i]->_emissive.data);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, _materials[i]->_shininess);

		// set texture (if any)

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		static_cast<BaseSurfaceOpenGL3D *>(_materials[i]->getSurface())->setTexture();
		glInterleavedArrays(GL_T2F_N3F_V3F, 0, _vertexData);
		glDrawElements(GL_TRIANGLES, _indexRanges[i + 1] - _indexRanges[i], GL_UNSIGNED_SHORT, _indexData + _indexRanges[i]);

		// maintain polycount
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	return res;
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
		return generateMesh();
	} else {
		return true;
	}
}

bool MeshX::parsePositionCoords(XFileLexer &lexer) {
	for (uint i = 0; i < _vertexCount; ++i) {
		for (int j = 0; j < 3; ++j) {
			_vertexPositionData[i * 3 + j] = readFloat(lexer);
			_vertexData[i * kVertexComponentCount + kPositionOffset + j] = _vertexPositionData[i * 3 + j];
		}

		_vertexPositionData[i * 3 + 2] *= -1.0f;
		_vertexData[i * kVertexComponentCount + kPositionOffset + 2] *= -1.0f;

		lexer.advanceToNextToken(); // skip semicolon
	}

	return true;
}

bool MeshX::parseFaces(XFileLexer &lexer, int faceCount) {
	for (int i = 0; i < faceCount; ++i) {
		int indexCount = readInt(lexer);

		// we can add something to triangulize faces later if the need arises
		if (indexCount != 3) {
			warning("MeshX::loadFromX non triangle faces are not supported yet");
			return false;
		}

		for (int j = 0; j < 3; ++j) {
			_indexData[i * 3 + j] = readInt(lexer);
		}

		// swap to change winding and make it consistent with the coordinate mirroring
		SWAP(_indexData[i * 3 + 0], _indexData[i * 3 + 2]);

		lexer.advanceToNextToken(); // skip semicolon
	}

	return true;
}

bool MeshX::parseTextureCoords(XFileLexer &lexer) {
	// should be the same as _vertexCount
	int textureCoordCount = readInt(lexer);

	for (int i = 0; i < textureCoordCount; ++i) {
		_vertexData[i * kVertexComponentCount + kTextureCoordOffset + 0] = readFloat(lexer);
		_vertexData[i * kVertexComponentCount + kTextureCoordOffset + 1] = readFloat(lexer);
		lexer.advanceToNextToken(); // skip semicolon
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
	uint vertexNormalCount = readInt(lexer);
	assert(vertexNormalCount == _vertexCount);

	for (uint i = 0; i < vertexNormalCount; ++i) {
		_vertexData[i * kVertexComponentCount + kNormalOffset] = readFloat(lexer);
		_vertexData[i * kVertexComponentCount + kNormalOffset + 1] = readFloat(lexer);
		// mirror z coordinate to change to OpenGL coordinate system
		_vertexData[i * kVertexComponentCount + kNormalOffset + 2] = -readFloat(lexer);
		lexer.advanceToNextToken(); // skip semicolon
	}

	// we ignore face normals for now
	while (!lexer.reachedClosedBraces()) {
		lexer.advanceToNextToken();
	}

	lexer.advanceToNextToken(); // skip closed braces

	return true;
}

bool MeshX::parseMaterials(XFileLexer &lexer, int faceCount, const Common::String &filename) {
	// there can be unused materials inside a .X file
	// so this piece of information is probably useless
	int materialCount = readInt(lexer);
	// should be the same as faceCount
	int faceMaterialCount = readInt(lexer);
	assert(faceMaterialCount = faceCount);

	// from looking at the wme3d sources and MSDN,
	// I would say that faces using the same material
	// are layed out as a contiguous block and the
	// material index is only increasing
	// in case this isn't true it might be a good
	// idea to split the mesh
	_indexRanges.push_back(0);
	int currentMaterialIndex = readInt(lexer);

	for (int i = 1; i < faceMaterialCount; ++i) {
		int currentMaterialIndexTmp = readInt(lexer);

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
		} else {
			warning("MeshX::loadFromX unknown token %i encountered while loading materials", lexer.getTypeOfToken());
			break;
		}
	}

	lexer.advanceToNextToken(); // skip closed braces

	return true;
}

bool MeshX::parseSkinWeights(XFileLexer &lexer) {
	skinWeightsList.resize(skinWeightsList.size() + 1);
	SkinWeights &currSkinWeights = skinWeightsList.back();

	currSkinWeights._boneName = readString(lexer);

	int weightCount = readInt(lexer);
	currSkinWeights._vertexIndices.resize(weightCount);
	currSkinWeights._vertexWeights.resize(weightCount);

	for (int i = 0; i < weightCount; ++i) {
		currSkinWeights._vertexIndices[i] = readInt(lexer);
	}

	if (weightCount == 0) {
		lexer.advanceToNextToken();
	}

	for (int i = 0; i < weightCount; ++i) {
		currSkinWeights._vertexWeights[i] = readFloat(lexer);
	}

	if (weightCount == 0) {
		lexer.advanceToNextToken();
	}

	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {
			currSkinWeights._offsetMatrix(c, r) = readFloat(lexer);
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

	lexer.advanceToNextToken(); // semicolon of matrix
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
