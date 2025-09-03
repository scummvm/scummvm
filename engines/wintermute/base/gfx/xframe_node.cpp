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

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/gfx/base_renderer3d.h"
#include "engines/wintermute/base/gfx/xmaterial.h"
#include "engines/wintermute/base/gfx/xframe_node.h"
#include "engines/wintermute/base/gfx/xmath.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/base/gfx/xfile_loader.h"
#include "engines/wintermute/dcgf.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
FrameNode::FrameNode(BaseGame *inGame) : BaseNamedObject(inGame) {
	DXMatrixIdentity(&_transformationMatrix);
	DXMatrixIdentity(&_originalMatrix);
	DXMatrixIdentity(&_combinedMatrix);

	for (int i = 0; i < 2; i++) {
		_transPos[i] = DXVector3(0.0f, 0.0f, 0.0f);
		_transScale[i] = DXVector3(1.0f, 1.0f, 1.0f);
		_transRot[i] = DXQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
		_lerpValue[i] = 0.0f;

		_transUsed[i] = false;
	}
}

//////////////////////////////////////////////////////////////////////////
FrameNode::~FrameNode() {
	// remove child frames
	for (int32 i = 0; i < _frames.getSize(); i++) {
		delete _frames[i];
	}
	_frames.removeAll();

	// remove meshes
	for (int32 i = 0; i < _meshes.getSize(); i++) {
		delete _meshes[i];
	}
	_meshes.removeAll();
}

//////////////////////////////////////////////////////////////////////////
DXMatrix *FrameNode::getCombinedMatrix() {
	return &_combinedMatrix;
}

//////////////////////////////////////////////////////////////////////////
DXMatrix *FrameNode::getOriginalMatrix() {
	return &_originalMatrix;
}

//////////////////////////////////////////////////////////////////////////
void FrameNode::setTransformationMatrix(DXMatrix *mat) {
	_transformationMatrix = *mat;
}

//////////////////////////////////////////////////////////////////////////
void FrameNode::setTransformation(int slot, DXVector3 pos, DXVector3 scale, DXQuaternion rot, float lerpValue) {
	if (slot < 0 || slot > 1)
		return;

	_transPos[slot] = pos;
	_transScale[slot] = scale;
	_transRot[slot] = rot;
	_lerpValue[slot] = lerpValue;

	_transUsed[slot] = true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::loadFromXData(const Common::String &filename, XModel *model, XFileData *xobj) {
	_gameRef->miniUpdate();

	bool res = true;

	// get the type of the object
	XClassType objectType = kXClassUnknown;
	res = xobj->getType(objectType);
	if (!res) {
		BaseEngine::LOG(0, "Error getting object type");
		return res;
	}

	if (objectType == kXClassMesh) { // load a child mesh
		XMesh *mesh = _gameRef->_renderer3D->createXMesh();
		res = mesh->loadFromXData(filename, xobj);
		if (res) {
			_meshes.add(mesh);
			return true;
		} else {
			SAFE_DELETE(mesh);
			return false;
		}
	} else if (objectType == kXClassFrameTransformMatrix) { // load the transformation matrix
		XFrameTransformMatrixObject *frameTransformMatrix = xobj->getXFrameTransformMatrixObject();
		if (!frameTransformMatrix) {
			BaseEngine::LOG(0, "Error loading transformation matrix");
			return false;
		} else {
			for (int i = 0; i < 16; ++i) {
				_transformationMatrix._m4x4[i] = frameTransformMatrix->_frameMatrix[i];
			}
			_originalMatrix = _transformationMatrix;
			return true;
		}
	} else if (objectType == kXClassAnimationSet) { // load animation set
		return model->loadAnimationSet(filename, xobj);
	} else if (objectType == kXClassAnimation) { // load a single animation (shouldn't happen here)
		return model->loadAnimation(filename, xobj);
	} else if (objectType == kXClassFrame) { // create a new child frame
		FrameNode *childFrame = new FrameNode(_gameRef);

		// get the name of the child frame
		res = XModel::loadName(childFrame, xobj);
		if (!res) {
			BaseEngine::LOG(0, "Error loading frame name");
			SAFE_DELETE(childFrame);
			return res;
		}

		// Enumerate child objects.
		res = false;
		uint32 numChildren = 0;
		xobj->getChildren(numChildren);
		for (uint32 i = 0; i < numChildren; i++) {
			XFileData xchildData;
			res = xobj->getChild(i, xchildData);
			if (res)
				res = childFrame->loadFromXData(filename, model, &xchildData);
		}
		if (res)
			_frames.add(childFrame);
		else
			delete childFrame;
		return res;
	} else if (objectType == kXClassAnimTicksPerSecond) {
		if (!xobj->getXAnimTicksPerSecondObject()) {
			BaseEngine::LOG(0, "Error loading ticks per seconds info");
			return res;
		} else {
			model->_ticksPerSecond = xobj->getXAnimTicksPerSecondObject()->_animTicksPerSecond;
			return true;
		}
	}

	return true;
}

bool FrameNode::mergeFromXData(const Common::String &filename, XModel *model, XFileData *xobj) {
	bool res = true;

	// get the type of the object
	XClassType objectType;
	res = xobj->getType(objectType);
	if (!res) {
		BaseEngine::LOG(0, "Error getting object type");
		return res;
	} else if (objectType == kXClassAnimationSet) { // load animation set
		return model->loadAnimationSet(filename, xobj);
	} else if (objectType == kXClassAnimation) { // load a single animation (shouldn't happen here)
		return model->loadAnimation(filename, xobj);
	} else if (objectType == kXClassFrame) { // scan child frames
		// Enumerate child objects.
		res = false;

		uint32 numChildren = 0;
		xobj->getChildren(numChildren);
		for (uint32 i = 0; i < numChildren; i++) {
			XFileData xchildData;
			res = xobj->getChild(i, xchildData);
			if (res)
				res = mergeFromXData(filename, model, &xchildData);
		}
		return res;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::findBones(FrameNode *rootFrame) {
	// find the bones of the meshes
	for (int32 i = 0; i < _meshes.getSize(); i++) {
		_meshes[i]->findBones(rootFrame);
	}

	// find the bones for the child frames
	for (int32 i = 0; i < _frames.getSize(); i++) {
		_frames[i]->findBones(rootFrame);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
FrameNode *FrameNode::findFrame(const char *frameName) {
	if (_name && strcmp(_name, frameName) == 0) {
		return this;
	} else {
		for (int32 i = 0; i < _frames.getSize(); i++) {
			FrameNode *foundFrame = _frames[i]->findFrame(frameName);
			if (foundFrame) {
				return foundFrame;
			}
		}
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::updateMatrices(DXMatrix *parentMat) {
	if (_transUsed[0]) {
		DXVector3 transPos = _transPos[0];
		DXVector3 transScale = _transScale[0];
		DXQuaternion transRot = _transRot[0];
		float lerpValue = _lerpValue[0];

		if (_transUsed[1]) {
			DXVec3Lerp(&transScale, &transScale, &_transScale[1], lerpValue);
			DXQuaternionSlerp(&transRot, &transRot, &_transRot[1], lerpValue);
			DXVec3Lerp(&transPos, &transPos, &_transPos[1], lerpValue);
		}

		// prepare local transformation matrix
		DXMatrixIdentity(&_transformationMatrix);
	
		DXMatrix scaleMat;
		DXMatrixScaling(&scaleMat, transScale._x, transScale._y, transScale._z);
		DXMatrixMultiply(&_transformationMatrix, &_transformationMatrix, &scaleMat);

		DXMatrix rotMat;
		DXMatrixRotationQuaternion(&rotMat, &transRot);
		DXMatrixMultiply(&_transformationMatrix, &_transformationMatrix, &rotMat);

		DXMatrix posMat;
		DXMatrixTranslation(&posMat, transPos._x, transPos._y, transPos._z);
		DXMatrixMultiply(&_transformationMatrix, &_transformationMatrix, &posMat);
	}
	_transUsed[0] = _transUsed[1] = false;

	// multiply by parent transformation
	DXMatrixMultiply(&_combinedMatrix, &_transformationMatrix, parentMat);

	// update child frames
	for (int32 i = 0; i < _frames.getSize(); i++) {
		_frames[i]->updateMatrices(&_combinedMatrix);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::updateMeshes() {
	bool res = true;

	// update meshes
	for (int32 i = 0; i < _meshes.getSize(); i++) {
		res = _meshes[i]->update(this);
		if (!res) {
			return res;
		}
	}

	// render child frames
	for (int32 i = 0; i < _frames.getSize(); i++) {
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
	for (int32 i = 0; i < _frames.getSize(); i++) {
		_frames[i]->resetMatrices();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::updateShadowVol(ShadowVolume *shadow, DXMatrix *modelMat, DXVector3 *light, float extrusionDepth) {
	bool res = true;

	// meshes
	for (int32 i = 0; i < _meshes.getSize(); i++) {
		res = _meshes[i]->updateShadowVol(shadow, modelMat, light, extrusionDepth);
		if (!res) {
			return res;
		}
	}

	// child frames
	for (int32 i = 0; i < _frames.getSize(); i++) {
		res = _frames[i]->updateShadowVol(shadow, modelMat, light, extrusionDepth);
		if (!res) {
			return res;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::render(XModel *model) {
	bool res = true;

	// render meshes
	for (int32 i = 0; i < _meshes.getSize(); i++) {
		res = _meshes[i]->render(model);
		if (!res) {
			return res;
		}
	}

	// render child frames
	for (int32 i = 0; i < _frames.getSize(); i++) {
		res = _frames[i]->render(model);
		if (!res) {
			return res;
		}
	}
	return true;
}

bool FrameNode::renderFlatShadowModel(uint32 shadowColor) {
	bool res = true;

	for (int32 i = 0; i < _meshes.getSize(); i++) {
		res = _meshes[i]->renderFlatShadowModel(shadowColor);
		if (!res) {
			return res;
		}
	}

	for (int32 i = 0; i < _frames.getSize(); i++) {
		res = _frames[i]->renderFlatShadowModel(shadowColor);
		if (!res) {
			return res;
		}
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::pickPoly(DXVector3 *pickRayOrig, DXVector3 *pickRayDir) {
	bool found = false;
	for (int32 i = 0; i < _meshes.getSize(); i++) {
		found = _meshes[i]->pickPoly(pickRayOrig, pickRayDir);
		if (found) {
			return true;
		}
	}

	for (int32 i = 0; i < _frames.getSize(); i++) {
		found = _frames[i]->pickPoly(pickRayOrig, pickRayDir);
		if (found) {
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::getBoundingBox(DXVector3 *boxStart, DXVector3 *boxEnd) {
	for (int32 i = 0; i < _meshes.getSize(); i++) {
		boxStart->_x = MIN(boxStart->_x, _meshes[i]->_BBoxStart._x);
		boxStart->_y = MIN(boxStart->_y, _meshes[i]->_BBoxStart._y);
		boxStart->_z = MIN(boxStart->_z, _meshes[i]->_BBoxStart._z);

		boxEnd->_x = MAX(boxEnd->_x, _meshes[i]->_BBoxEnd._x);
		boxEnd->_y = MAX(boxEnd->_y, _meshes[i]->_BBoxEnd._y);
		boxEnd->_z = MAX(boxEnd->_z, _meshes[i]->_BBoxEnd._z);
	}

	for (int32 i = 0; i < _frames.getSize(); i++) {
		_frames[i]->getBoundingBox(boxStart, boxEnd);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::hasChildren() {
	return _frames.getSize() > 0;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::setMaterialSprite(char *matName, BaseSprite *sprite) {
	for (int32 i = 0; i < _meshes.getSize(); i++) {
		_meshes[i]->setMaterialSprite(matName, sprite);
	}

	for (int32 i = 0; i < _frames.getSize(); i++) {
		_frames[i]->setMaterialSprite(matName, sprite);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::setMaterialTheora(char *matName, VideoTheoraPlayer *theora) {
	for (int32 i = 0; i < _meshes.getSize(); i++) {
		_meshes[i]->setMaterialTheora(matName, theora);
	}

	for (int32 i = 0; i < _frames.getSize(); i++) {
		_frames[i]->setMaterialTheora(matName, theora);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::setMaterialEffect(char *matName, Effect3D *effect, Effect3DParams *params) {
	for (int32 i = 0; i < _meshes.getSize(); i++) {
		_meshes[i]->setMaterialEffect(matName, effect, params);
	}

	for (int32 i = 0; i < _frames.getSize(); i++) {
		_frames[i]->setMaterialEffect(matName, effect, params);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::removeMaterialEffect(const char *matName) {
	for (int32 i = 0; i < _meshes.getSize(); i++) {
		_meshes[i]->removeMaterialEffect(matName);
	}

	for (int32 i = 0; i < _frames.getSize(); i++) {
		_frames[i]->removeMaterialEffect(matName);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::invalidateDeviceObjects() {
	for (int32 i = 0; i < _meshes.getSize(); i++) {
		_meshes[i]->invalidateDeviceObjects();
	}

	for (int32 i = 0; i < _frames.getSize(); i++) {
		_frames[i]->invalidateDeviceObjects();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool FrameNode::restoreDeviceObjects() {
	for (int32 i = 0; i < _meshes.getSize(); i++) {
		_meshes[i]->restoreDeviceObjects();
	}

	for (int32 i = 0; i < _frames.getSize(); i++) {
		_frames[i]->restoreDeviceObjects();
	}

	return true;
}

} // namespace Wintermute
