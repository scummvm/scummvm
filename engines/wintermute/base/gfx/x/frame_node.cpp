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

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/base_renderer3d.h"
#include "engines/wintermute/base/gfx/x/material.h"
#include "engines/wintermute/base/gfx/x/frame_node.h"
#include "engines/wintermute/base/gfx/x/modelx.h"
#include "engines/wintermute/base/gfx/x/loader_x.h"
#include "engines/wintermute/dcgf.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
FrameNode::FrameNode(BaseGame *inGame) : BaseNamedObject(inGame) {
	_transformationMatrix.setToIdentity();
	_originalMatrix.setToIdentity();
	_combinedMatrix.setToIdentity();

	for (int i = 0; i < 2; i++) {
		_transPos[i] = Math::Vector3d(0.0f, 0.0f, 0.0f);
		_transScale[i] = Math::Vector3d(1.0f, 1.0f, 1.0f);
		_transRot[i] = Math::Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
		_lerpValue[i] = 0.0f;

		_transUsed[i] = false;
	}
}

//////////////////////////////////////////////////////////////////////////
FrameNode::~FrameNode(void) {
	// remove child frames
	for (uint32 i = 0; i < _frames.size(); i++) {
		delete _frames[i];
	}

	_frames.clear();

	// remove meshes
	for (uint32 i = 0; i < _meshes.size(); i++) {
		delete _meshes[i];
	}

	_meshes.clear();
}

//////////////////////////////////////////////////////////////////////////
Math::Matrix4 *FrameNode::getCombinedMatrix() {
	return &_combinedMatrix;
}

//////////////////////////////////////////////////////////////////////////
Math::Matrix4 *FrameNode::getOriginalMatrix() {
	return &_originalMatrix;
}

//////////////////////////////////////////////////////////////////////////
void FrameNode::setTransformationMatrix(Math::Matrix4 *mat) {
	_transformationMatrix = *mat;
}

//////////////////////////////////////////////////////////////////////////
void FrameNode::setTransformation(int slot, Math::Vector3d pos, Math::Vector3d scale, Math::Quaternion rot, float lerpValue) {
	if (slot < 0 || slot > 1)
		return;

	_transPos[slot] = pos;
	_transScale[slot] = scale;
	_transRot[slot] = rot;
	_lerpValue[slot] = lerpValue;

	_transUsed[slot] = true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::loadFromX(const Common::String &filename, XFileLexer &lexer, ModelX *model, Common::Array<MaterialReference> &materialReferences) {
	_gameRef->miniUpdate();

	bool ret = true;

	setName(lexer.tokenToString().c_str());
	lexer.advanceToNextToken();
	lexer.advanceOnOpenBraces();

	while (!lexer.eof()) {
		if (lexer.tokenIsIdentifier("Frame")) {
			lexer.advanceToNextToken();
			FrameNode *child = new FrameNode(_gameRef);
			if (child->loadFromX(filename, lexer, model, materialReferences)) {
				_frames.add(child);
			} else {
				delete child;
			}
		} else if (lexer.tokenIsIdentifier("Mesh")) {
			lexer.advanceToNextToken();
			MeshX *mesh = _gameRef->_renderer3D->createMeshX();

			if (mesh->loadFromX(filename, lexer, materialReferences)) {
				_meshes.add(mesh);
			} else {
				delete mesh;
			}
		} else if (lexer.tokenIsIdentifier("FrameTransformMatrix")) {
			lexer.advanceToNextToken();
			lexer.advanceToNextToken(); // skip optional name
			lexer.advanceOnOpenBraces();

			// TODO: check if this is the right format
			for (int r = 0; r < 4; ++r) {
				for (int c = 0; c < 4; ++c) {
					_transformationMatrix(c, r) = lexer.readFloat();
				}
			}

			// mirror at orign
			_transformationMatrix(2, 3) *= -1.0f;

			// mirror base vectors
			_transformationMatrix(2, 0) *= -1.0f;
			_transformationMatrix(2, 1) *= -1.0f;

			// change handedness
			_transformationMatrix(0, 2) *= -1.0f;
			_transformationMatrix(1, 2) *= -1.0f;

			_originalMatrix = _transformationMatrix;

			lexer.skipTerminator();
			lexer.advanceToNextToken();

		} else if (lexer.reachedClosedBraces()) {
			lexer.advanceToNextToken();
			break;
		} else {
			warning("FrameNode::loadFromX unexpected %i token excountered", lexer.getTypeOfToken());
			ret = false;
			break;
		}
	}

	return ret;
}

bool FrameNode::loadFromXAsRoot(const Common::String &filename, XFileLexer &lexer, ModelX *model, Common::Array<MaterialReference> &materialReferences) {
	// technically, there is no root node in a .X file
	// so we just start parsing it here
	lexer.advanceToNextToken();

	while (!lexer.eof()) {
		if (lexer.tokenIsIdentifier("Frame")) {
			lexer.advanceToNextToken();
			FrameNode *child = new FrameNode(_gameRef);
			if (child->loadFromX(filename, lexer, model, materialReferences)) {
				_frames.add(child);
			} else {
				delete child;
			}
		} else if (lexer.tokenIsIdentifier("Mesh")) {
			lexer.advanceToNextToken();
			MeshX *mesh = _gameRef->_renderer3D->createMeshX();

			if (mesh->loadFromX(filename, lexer, materialReferences)) {
				_meshes.add(mesh);
			} else {
				delete mesh;
			}
		} else if (lexer.tokenIsIdentifier("AnimTicksPerSecond")) {
			lexer.advanceToNextToken();
			lexer.advanceOnOpenBraces();

			model->_ticksPerSecond = lexer.readInt();
			lexer.advanceToNextToken(); // skip closed braces
		} else if (lexer.tokenIsIdentifier("AnimationSet")) {
			lexer.advanceToNextToken();
			model->loadAnimationSet(lexer, filename);
		} else if (lexer.tokenIsIdentifier("template")) {
			// we can ignore templates
			while (!lexer.eof()) {
				if (lexer.reachedClosedBraces()) {
					break;
				}

				lexer.advanceToNextToken();
			}

			lexer.advanceToNextToken();
		} else if(lexer.tokenIsIdentifier("Material")) {
			lexer.advanceToNextToken();
			MaterialReference materialReference;

			materialReference._name = lexer.tokenToString();
			materialReference._material = new Material(_gameRef);
			materialReference._material->loadFromX(lexer, filename);

			materialReferences.push_back(materialReference);
		} else if (lexer.tokenIsOfType(NULL_CHAR)) {
			// prevents some unnecessary warnings
			lexer.advanceToNextToken();
		} else {
			warning("FrameNode::loadFromXAsRoot unknown token %i encountered", lexer.getTypeOfToken());
			lexer.advanceToNextToken(); // just ignore it for the moment
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::findBones(FrameNode *rootFrame) {
	// find the bones of the meshes
	for (uint32 i = 0; i < _meshes.size(); i++) {
		_meshes[i]->findBones(rootFrame);
	}

	// find the bones for the child frames
	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i]->findBones(rootFrame);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
FrameNode *FrameNode::findFrame(const char *frameName) {
	if (getName() && scumm_stricmp(getName(), frameName) == 0) {
		return this;
	} else {
		for (uint32 i = 0; i < _frames.size(); i++) {
			FrameNode *foundFrame = _frames[i]->findFrame(frameName);
			if (foundFrame) {
				return foundFrame;
			}
		}
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::updateMatrices(Math::Matrix4 &parentMat) {
	if (_transUsed[0]) {
		Math::Vector3d transPos = _transPos[0];
		Math::Vector3d transScale = _transScale[0];
		Math::Quaternion transRot = _transRot[0];
		float lerpValue = _lerpValue[0];

		if (_transUsed[1]) {
			transScale = (1 - lerpValue) * transScale + lerpValue * _transScale[1];
			transRot = transRot.slerpQuat(_transRot[1], lerpValue);
			transPos = (1 - lerpValue) * transPos + lerpValue * _transPos[1];
		}

		// prepare local transformation matrix
		_transformationMatrix.setToIdentity();

		Math::Matrix4 scaleMat;
		scaleMat.setToIdentity();
		scaleMat(0, 0) = transScale.x();
		scaleMat(1, 1) = transScale.y();
		scaleMat(2, 2) = transScale.z();
		Math::Matrix4 rotMat;
		transRot.toMatrix(rotMat);
		Math::Matrix4 posMat;
		posMat.setToIdentity();
		posMat.translate(transPos);

		_transformationMatrix = posMat * rotMat * scaleMat;
	}

	_transUsed[0] = _transUsed[1] = false;

	// multiply by parent transformation
	_combinedMatrix = parentMat * _transformationMatrix;

	// update child frames
	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i]->updateMatrices(_combinedMatrix);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::updateMeshes() {
	bool res = true;

	// update meshes
	for (uint32 i = 0; i < _meshes.size(); i++) {
		res = _meshes[i]->update(this);
		if (!res) {
			return res;
		}
	}

	// render child frames
	for (uint32 i = 0; i < _frames.size(); i++) {
		res = _frames[i]->updateMeshes();
		if (!res) {
			return res;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::resetMatrices() {
	_transformationMatrix = _originalMatrix;

	// update child frames
	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i]->resetMatrices();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::updateShadowVol(ShadowVolume *shadow, Math::Matrix4 &modelMat, const Math::Vector3d &light, float extrusionDepth) {
	bool res = true;

	// meshes
	for (uint32 i = 0; i < _meshes.size(); i++) {
		res = _meshes[i]->updateShadowVol(shadow, modelMat, light, extrusionDepth);
		if (!res) {
			return res;
		}
	}

	// child frames
	for (uint32 i = 0; i < _frames.size(); i++) {
		res = _frames[i]->updateShadowVol(shadow, modelMat, light, extrusionDepth);
		if (!res) {
			return res;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::render(ModelX *model) {
	bool res = true;

	// render meshes
	for (uint32 i = 0; i < _meshes.size(); i++) {
		res = _meshes[i]->render(model);
		if (!res) {
			return res;
		}
	}

	// render child frames
	for (uint32 i = 0; i < _frames.size(); i++) {
		res = _frames[i]->render(model);
		if (!res) {
			return res;
		}
	}
	return true;
}

bool FrameNode::renderFlatShadowModel() {
	bool res = true;

	for (uint32 i = 0; i < _meshes.size(); i++) {
		res = _meshes[i]->renderFlatShadowModel();
		if (!res) {
			return res;
		}
	}

	for (uint32 i = 0; i < _frames.size(); i++) {
		res = _frames[i]->renderFlatShadowModel();
		if (!res) {
			return res;
		}
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::pickPoly(Math::Vector3d *pickRayOrig, Math::Vector3d *pickRayDir) {
	bool found = false;
	for (uint32 i = 0; i < _meshes.size(); i++) {
		found = _meshes[i]->pickPoly(pickRayOrig, pickRayDir);
		if (found) {
			return true;
		}
	}

	for (uint32 i = 0; i < _frames.size(); i++) {
		found = _frames[i]->pickPoly(pickRayOrig, pickRayDir);
		if (found) {
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::getBoundingBox(Math::Vector3d *boxStart, Math::Vector3d *boxEnd) {
	for (uint32 i = 0; i < _meshes.size(); i++) {
		boxStart->x() = MIN(boxStart->x(), _meshes[i]->_BBoxStart.x());
		boxStart->y() = MIN(boxStart->y(), _meshes[i]->_BBoxStart.y());
		boxStart->z() = MIN(boxStart->z(), _meshes[i]->_BBoxStart.z());

		boxEnd->x() = MAX(boxEnd->x(), _meshes[i]->_BBoxEnd.x());
		boxEnd->y() = MAX(boxEnd->y(), _meshes[i]->_BBoxEnd.y());
		boxEnd->z() = MAX(boxEnd->z(), _meshes[i]->_BBoxEnd.z());
	}

	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i]->getBoundingBox(boxStart, boxEnd);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::hasChildren() {
	return _frames.size() > 0;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::setMaterialSprite(char *matName, BaseSprite *sprite) {
	for (uint32 i = 0; i < _meshes.size(); i++) {
		_meshes[i]->setMaterialSprite(matName, sprite);
	}

	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i]->setMaterialSprite(matName, sprite);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::setMaterialTheora(char *matName, VideoTheoraPlayer *theora) {
	for (uint32 i = 0; i < _meshes.size(); i++) {
		_meshes[i]->setMaterialTheora(matName, theora);
	}

	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i]->setMaterialTheora(matName, theora);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::invalidateDeviceObjects() {
	for (uint32 i = 0; i < _meshes.size(); i++) {
		_meshes[i]->invalidateDeviceObjects();
	}

	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i]->invalidateDeviceObjects();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::restoreDeviceObjects() {
	for (uint32 i = 0; i < _meshes.size(); i++) {
		_meshes[i]->restoreDeviceObjects();
	}

	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i]->restoreDeviceObjects();
	}

	return true;
}

} // namespace Wintermute
