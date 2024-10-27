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

#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_object_3d.h"
#include "engines/wintermute/ad/ad_scene.h"
#include "engines/wintermute/ad/ad_scene_geometry.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/gfx/3dshadow_volume.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/utils/utils.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdObject3D, false)

//////////////////////////////////////////////////////////////////////////
AdObject3D::AdObject3D(BaseGame *inGame) : AdObject(inGame) {
	_is3D = true;

	_velocity = 1.0f;
	_angVelocity = 1.0f;
	_lastPosVector = DXVector3(0.0f, 0.0f, 0.0f);

	_state = _nextState = STATE_READY;
	_dropToFloor = true;
	_shadowType = SHADOW_STENCIL;

	_tempSkelAnim = nullptr;

	_shadowVolume = nullptr;

	_ambientLightColor = 0x00000000;
	_hasAmbientLightColor = false;
}

//////////////////////////////////////////////////////////////////////////
AdObject3D::~AdObject3D() {
	_tempSkelAnim = nullptr; // ref only
	delete _shadowVolume;
	_shadowVolume = nullptr;

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
		if (scumm_stricmp(_ignoredLights[i], lightName) == 0) {
			return true;
		}
	}

	_ignoredLights.add(lightName);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::removeIgnoredLight(char *lightName) {
	for (uint32 i = 0; i < _ignoredLights.size(); i++) {
		if (scumm_stricmp(_ignoredLights[i], lightName) == 0) {
			delete[] _ignoredLights[i];
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
	if (_dropToFloor && adGame->_scene && adGame->_scene->_geom) {
		_posVector._y = adGame->_scene->_geom->getHeightAt(_posVector, 5.0f);
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
bool AdObject3D::convert3DTo2D(DXMatrix *worldMat, int32 *posX, int32 *posY) {
	DXMatrix viewMat, projMat;
	DXVector3 vec2d(0.0f, 0.0f, 0.0f);
	DXVector3 origin(0.0f, 0.0f, 0.0f);
	_gameRef->_renderer3D->getViewTransform(&viewMat);
	_gameRef->_renderer3D->getProjectionTransform(&projMat);

	DXViewport viewport = _gameRef->_renderer3D->getViewPort();

	DXVec3Project(&vec2d, &origin, &viewport, &projMat, &viewMat, worldMat);

	*posX = vec2d._x + _gameRef->_offsetX - _gameRef->_renderer3D->_drawOffsetX;
	*posY = vec2d._y + _gameRef->_offsetY - _gameRef->_renderer3D->_drawOffsetY;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::display() {
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::setupLights() {
	AdGame *adGame = (AdGame *)_gameRef;

	if (adGame->_scene && adGame->_scene->_geom) {
		return adGame->_scene->_geom->enableLights(_posVector, _ignoredLights);
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
		_posVector._x = stack->pop()->getFloat();
		_posVector._y = stack->pop()->getFloat();
		_posVector._z = stack->pop()->getFloat();

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
		DXVector3 pos(0, 0, 0);
		getBonePosition3D(boneName, &pos);

		ScValue *val = stack->getPushValue();

		if (val) {
			val->setProperty("X", pos._x);
			val->setProperty("Y", pos._y);
			val->setProperty("Z", pos._z);
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
		_scValue->setFloat(_angle);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// PosX
	//////////////////////////////////////////////////////////////////////////
	else if (name == "PosX") {
		_scValue->setFloat(_posVector._x);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// PosY
	//////////////////////////////////////////////////////////////////////////
	else if (name == "PosY") {
		_scValue->setFloat(_posVector._y);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// PosZ
	//////////////////////////////////////////////////////////////////////////
	else if (name == "PosZ") {
		_scValue->setFloat(_posVector._z);
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
		_posVector._x = value->getFloat();
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	// PosY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PosY") == 0) {
		_posVector._y = value->getFloat();
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	// PosZ
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PosZ") == 0) {
		_posVector._z = value->getFloat();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "X") == 0) {
		_posX = value->getInt();
		AdGame *adGame = (AdGame *)_gameRef;
		DXVector3 pos;
		if (adGame->_scene->_geom && adGame->_scene->_geom->convert2Dto3D(_posX, _posY, &pos)) {
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
		DXVector3 pos;
		if (adGame->_scene->_geom && adGame->_scene->_geom->convert2Dto3D(_posX, _posY, &pos)) {
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
	DXVector3 pos;

	bool success;
	if (tolerant) {
		success = adGame->_scene->_geom && adGame->_scene->_geom->convert2Dto3DTolerant(x, y, &pos);
	} else {
		success = adGame->_scene->_geom && adGame->_scene->_geom->convert2Dto3D(x, y, &pos);
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
	if (!_xmodel) {
		return false;
	}

	AdGame *adGame = (AdGame *)_gameRef;
	if (!adGame->_scene || !adGame->_scene->_geom)
		return false;

	DXMatrix *boneMat = _xmodel->getBoneMatrix(boneName);
	if (!boneMat) {
		return false;
	}

	DXMatrix bonePosMat;
	DXMatrixMultiply(&bonePosMat, boneMat, &_worldMatrix);

	DXVector4 vectBone4;
	DXVector3 vectBone3(0, 0, 0);
	DXVec3Transform(&vectBone4, &vectBone3, &bonePosMat);
	DXVector3 vectBone(vectBone4._x, vectBone4._y, vectBone4._z);

	adGame->_scene->_geom->convert3Dto2D(&vectBone, x, y);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdObject3D::getBonePosition3D(const char *boneName, DXVector3 *pos, DXVector3 *offset) {
	if (!_xmodel) {
		return false;
	}

	DXMatrix *boneMat = _xmodel->getBoneMatrix(boneName);
	if (!boneMat) {
		return false;
	}

	DXVector3 vz(0, 0, 0);
	if (!offset) {
		offset = &vz;
	}

	DXMatrix bonePosMat;
	DXMatrixMultiply(&bonePosMat, boneMat, &_worldMatrix);

	DXVector4 vectBone4;
	DXVec3Transform(&vectBone4, offset, &bonePosMat);

	pos->_x = vectBone4._x;
	pos->_y = vectBone4._y;
	pos->_z = vectBone4._z;

	return true;
}

} // namespace Wintermute
