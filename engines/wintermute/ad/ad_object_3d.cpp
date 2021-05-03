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

#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_object_3d.h"
#include "engines/wintermute/ad/ad_scene.h"
#include "engines/wintermute/ad/ad_scene_geometry.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/gfx/shadow_volume.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/x/modelx.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/utils/utils.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdObject3D, false)

//////////////////////////////////////////////////////////////////////////
AdObject3D::AdObject3D(BaseGame *inGame) : AdObject(inGame),
										   _tempSkelAnim(nullptr),
										   _lastPosVector(0.0f, 0.0f, 0.0f),
										   _dropToFloor(true),
										   _velocity(1.0f),
										   _angVelocity(1.0f),
										   _ambientLightColor(0x00000000),
										   _hasAmbientLightColor(false),
										   _shadowVolume(nullptr) {
	_is3D = true;
	_state = _nextState = STATE_READY;
	_shadowType = SHADOW_STENCIL;
}

//////////////////////////////////////////////////////////////////////////
AdObject3D::~AdObject3D() {
	_tempSkelAnim = nullptr; // ref only
	delete _shadowVolume;

	clearIgnoredLights();
}

//////////////////////////////////////////////////////////////////////////
void AdObject3D::clearIgnoredLights() {
	for (uint32 i = 0; i < _ignoredLights.size(); ++i) {
		delete _ignoredLights[i];
	}

	_ignoredLights.clear();
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::addIgnoredLight(char *lightName) {
	for (uint32 i = 0; i < _ignoredLights.size(); i++) {
		if (_ignoredLights[i] == lightName) {
			return true;
		}
	}

	_ignoredLights.add(lightName);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::removeIgnoredLight(char *lightName) {
	for (uint32 i = 0; i < _ignoredLights.size(); i++) {
		if (_ignoredLights[i] == lightName) {
			_ignoredLights.remove_at(i);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::update() {
	AdGame *adGame = (AdGame *)_gameRef;

	// drop to floor
	if (_dropToFloor && adGame->_scene && adGame->_scene->_sceneGeometry) {
		_posVector.y() = adGame->_scene->_sceneGeometry->getHeightAt(_posVector, 5.0f);
	}

	getMatrix(&_worldMatrix);

	// setup 2D position
	if (_posVector != _lastPosVector) {
		_lastPosVector = _posVector;

		convert3DTo2D(&_worldMatrix, &_posX, &_posY);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::convert3DTo2D(Math::Matrix4 *worldMat, int32 *posX, int32 *posY) {
	Math::Vector3d origin(0.0f, 0.0f, 0.0f);
	_gameRef->_renderer3D->project(*worldMat, origin, *posX, *posY);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::display() {
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::setupLights() {
	AdGame *adGame = (AdGame *)_gameRef;

	if (adGame->_scene && adGame->_scene->_sceneGeometry) {
		return adGame->_scene->_sceneGeometry->enableLights(_posVector, _ignoredLights);
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool AdObject3D::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SkipTo
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SkipTo") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();

		skipTo(x, y);
		stack->pushNULL();

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	// SkipTo3D
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SkipTo3D") == 0) {
		stack->correctParams(3);
		_posVector.x() = stack->pop()->getFloat();
		_posVector.y() = stack->pop()->getFloat();
		// scripts will expect a Direct3D coordinate system
		_posVector.z() = -stack->pop()->getFloat();

		stack->pushNULL();

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetBonePosition2D
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetBonePosition2D") == 0) {
		stack->correctParams(1);
		const char *boneName = stack->pop()->getString();
		int32 x = 0, y = 0;
		getBonePosition2D(boneName, &x, &y);

		ScValue *val = stack->getPushValue();
		if (val) {
			val->setProperty("X", x);
			val->setProperty("Y", y);
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetBonePosition3D
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetBonePosition3D") == 0) {
		stack->correctParams(1);
		const char *boneName = stack->pop()->getString();
		Math::Vector3d pos(0, 0, 0);
		getBonePosition3D(boneName, &pos);

		ScValue *val = stack->getPushValue();

		if (val) {
			val->setProperty("X", pos.x());
			val->setProperty("Y", pos.y());
			val->setProperty("Z", pos.z());
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	// AddIgnoredLight
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddIgnoredLight") == 0) {
		stack->correctParams(1);
		char *lightName = nullptr;
		BaseUtils::setString(&lightName, stack->pop()->getString());
		stack->pushBool(addIgnoredLight(lightName));
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	// RemoveIgnoredLight
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveIgnoredLight") == 0) {
		stack->correctParams(1);
		char *lightName = nullptr;
		BaseUtils::setString(&lightName, stack->pop()->getString());
		stack->pushBool(removeIgnoredLight(lightName));
		return true;
	}

	else {
		return AdObject::scCallMethod(script, stack, thisStack, name);
	}
}

//////////////////////////////////////////////////////////////////////////
ScValue *AdObject3D::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Angle
	//////////////////////////////////////////////////////////////////////////
	if (name == "Angle") {
		float tmp = 0.0f;
		_scValue->setFloat(tmp);
		_angle = tmp;
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// PosX
	//////////////////////////////////////////////////////////////////////////
	else if (name == "PosX") {
		_scValue->setFloat(_posVector.x());
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// PosY
	//////////////////////////////////////////////////////////////////////////
	else if (name == "PosY") {
		_scValue->setFloat(_posVector.y());
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// PosZ
	//////////////////////////////////////////////////////////////////////////
	else if (name == "PosZ") {
		// scripts will expect a Direct3D coordinate system
		_scValue->setFloat(-_posVector.z());
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Velocity
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Velocity") {
		_scValue->setFloat(_velocity);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// AngularVelocity / AngVelocity
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AngularVelocity" || name == "AngVelocity") {
		_scValue->setFloat(_angVelocity);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// DropToFloor
	//////////////////////////////////////////////////////////////////////////
	else if (name == "DropToFloor") {
		_scValue->setBool(_dropToFloor);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ShadowType
	//////////////////////////////////////////////////////////////////////////
	else if (name == "ShadowType") {
		_scValue->setInt(_shadowType);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Shadow (obsolete)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Shadow") {
		_scValue->setBool(_shadowType > SHADOW_NONE);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SimpleShadow (obsolete)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SimpleShadow") {
		_scValue->setBool(_shadowType == SHADOW_SIMPLE);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ShadowColor
	//////////////////////////////////////////////////////////////////////////
	else if (name == "ShadowColor") {
		_scValue->setInt(_shadowColor);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Scale") {
		_scValue->setFloat(_scale3D * 100.0f);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// DrawBackfaces
	//////////////////////////////////////////////////////////////////////////
	else if (name == "DrawBackfaces") {
		_scValue->setBool(_drawBackfaces);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmbientLightColor
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AmbientLightColor") {
		if (_hasAmbientLightColor) {
			_scValue->setInt(_ambientLightColor);
		} else {
			_scValue->setNULL();
		}

		return _scValue;
	}

	else {
		return AdObject::scGetProperty(name);
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::scSetProperty(const char *name, ScValue *value) {

	//////////////////////////////////////////////////////////////////////////
	// Angle
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Angle") == 0) {
		_angle = value->getFloat();
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	// PosX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PosX") == 0) {
		_posVector.x() = value->getFloat();
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	// PosY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PosY") == 0) {
		_posVector.y() = value->getFloat();
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	// PosZ
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PosZ") == 0) {
		// scripts will expect a Direct3D coordinate system
		_posVector.z() = -value->getFloat();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "X") == 0) {
		_posX = value->getInt();
		AdGame *adGame = (AdGame *)_gameRef;
		Math::Vector3d pos;
		if (adGame->_scene->_sceneGeometry && adGame->_scene->_sceneGeometry->convert2Dto3D(_posX, _posY, &pos)) {
			_posVector = pos;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Y") == 0) {
		_posY = value->getInt();
		AdGame *adGame = (AdGame *)_gameRef;
		Math::Vector3d pos;
		if (adGame->_scene->_sceneGeometry && adGame->_scene->_sceneGeometry->convert2Dto3D(_posX, _posY, &pos)) {
			_posVector = pos;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Velocity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Velocity") == 0) {
		_velocity = value->getFloat();
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	// AngularVelocity / AngVelocity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AngularVelocity") == 0 || strcmp(name, "AngVelocity") == 0) {
		_angVelocity = value->getFloat();
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	// DropToFloor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DropToFloor") == 0) {
		_dropToFloor = value->getBool();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Shadow (obsolete)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Shadow") == 0) {
		if (value->getBool()) {
			_shadowType = SHADOW_STENCIL;
		} else {
			_shadowType = SHADOW_NONE;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// ShadowType
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ShadowType") == 0) {
		_shadowType = (TShadowType)value->getInt();
		if (_shadowType < 0) {
			_shadowType = SHADOW_NONE;
		}

		if (_shadowType > SHADOW_STENCIL) {
			_shadowType = SHADOW_STENCIL;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// SimpleShadow (obsolete)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SimpleShadow") == 0) {
		if (value->getBool()) {
			_shadowType = SHADOW_SIMPLE;
		} else {
			_shadowType = SHADOW_STENCIL;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// ShadowColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ShadowColor") == 0) {
		_shadowColor = value->getInt();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Scale") == 0) {
		_scale3D = value->getFloat() / 100.0f;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// DrawBackfaces
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DrawBackfaces") == 0) {
		_drawBackfaces = value->getBool();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmbientLightColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmbientLightColor") == 0) {
		if (value->isNULL()) {
			_ambientLightColor = 0x00000000;
			_hasAmbientLightColor = false;
		} else {
			_ambientLightColor = value->getInt();
			_hasAmbientLightColor = true;
		}
		return true;
	}

	else {
		return AdObject::scSetProperty(name, value);
	}
}

//////////////////////////////////////////////////////////////////////////
const char *AdObject3D::scToString() {
	return "[ad object3d]";
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::persist(BasePersistenceManager *persistMgr) {
	AdObject::persist(persistMgr);

	persistMgr->transferFloat(TMEMBER(_angVelocity));
	persistMgr->transferBool(TMEMBER(_dropToFloor));
	persistMgr->transferCharPtr(TMEMBER(_tempSkelAnim));
	persistMgr->transferVector3d(TMEMBER(_lastPosVector));
	persistMgr->transferFloat(TMEMBER(_velocity));

	if (!persistMgr->getIsSaving()) {
		_shadowVolume = nullptr;
	}

	_ignoredLights.persist(persistMgr);

	persistMgr->transferUint32(TMEMBER(_ambientLightColor));
	persistMgr->transferBool(TMEMBER(_hasAmbientLightColor));

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::skipTo(int x, int y, bool tolerant) {
	AdGame *adGame = (AdGame *)_gameRef;
	Math::Vector3d pos;

	bool success;
	if (tolerant) {
		success = adGame->_scene->_sceneGeometry && adGame->_scene->_sceneGeometry->convert2Dto3DTolerant(x, y, &pos);
	} else {
		success = adGame->_scene->_sceneGeometry && adGame->_scene->_sceneGeometry->convert2Dto3D(x, y, &pos);
	}

	if (success) {
		_posVector = pos;
		_posX = x;
		_posY = y;
		return true;
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
ShadowVolume *AdObject3D::getShadowVolume() {
	if (_shadowVolume == nullptr) {
		_shadowVolume = _gameRef->_renderer3D->createShadowVolume();
	}

	return _shadowVolume;
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::getBonePosition2D(const char *boneName, int32 *x, int32 *y) {
	if (!_modelX) {
		return false;
	}

	AdGame *adGame = (AdGame *)_gameRef;

	if (!adGame->_scene || !adGame->_scene->_sceneGeometry)
		return false;

	Math::Matrix4 *boneMat = _modelX->getBoneMatrix(boneName);

	if (!boneMat) {
		return false;
	}

	Math::Matrix4 bonePosMat = *boneMat * _worldMatrix;

	Math::Vector4d vectBone4 = Math::Vector4d(0.0f, 0.0f, 0.0f, 0.0f);
	bonePosMat.transformVector(&vectBone4);
	Math::Vector3d vectBone(vectBone4.x(), vectBone4.y(), vectBone4.z());

	adGame->_scene->_sceneGeometry->convert3Dto2D(&vectBone, x, y);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::getBonePosition3D(const char *boneName, Math::Vector3d *pos, Math::Vector3d *offset) {
	if (!_modelX) {
		return false;
	}

	Math::Matrix4 *boneMat = _modelX->getBoneMatrix(boneName);
	if (!boneMat) {
		return false;
	}

	Math::Matrix4 bonePosMat = *boneMat * _worldMatrix;
	*pos = Math::Vector3d(0.0f, 0.0f, 0.0f);

	if (offset) {
		*pos = *offset;
	}

	bonePosMat.transform(pos, true);

	return true;
}

} // namespace Wintermute
