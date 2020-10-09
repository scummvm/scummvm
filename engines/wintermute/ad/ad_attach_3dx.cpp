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

#include "engines/wintermute/ad/ad_attach_3dx.h"
#include "engines/wintermute/base/base_active_rect.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/shadow_volume.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/x/modelx.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdAttach3DX, false)

//////////////////////////////////////////////////////////////////////////
AdAttach3DX::AdAttach3DX(BaseGame *inGame, BaseObject *owner) : AdObject3D(inGame), _owner(owner) {
	_dropToFloor = false;
}

//////////////////////////////////////////////////////////////////////////
AdAttach3DX::~AdAttach3DX(void) {
	_owner = nullptr; // ref only
}

//////////////////////////////////////////////////////////////////////////
bool AdAttach3DX::init(const char *modelFile, const char *name, const char *parentBone) {
	delete _modelX;
	_modelX = nullptr;

	_parentBone = parentBone;
	setName(name);

	_modelX = new ModelX(_gameRef, _owner);

	if (!_modelX) {
		return false;
	}

	bool res = _modelX->loadFromFile(modelFile);
	if (!res) {
		delete _modelX;
		_modelX = nullptr;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
bool AdAttach3DX::update() {
	AdObject3D::update();
	if (_modelX) {
		return _modelX->update();
	} else {
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdAttach3DX::displayAttachable(const Math::Matrix4 &viewMat, bool registerObjects) {
	Math::Matrix4 finalMat = _owner->_worldMatrix * viewMat * _worldMatrix;
	_gameRef->_renderer3D->setWorldTransform(finalMat);

	if (_modelX) {
		_modelX->render();

		if (registerObjects && _owner && _owner->_registrable) {
			_gameRef->_renderer->addRectToList(new BaseActiveRect(_gameRef, _owner, _modelX,
			                                                      _modelX->_boundingRect.left,
			                                                      _modelX->_boundingRect.top,
			                                                      _modelX->_boundingRect.right - _modelX->_boundingRect.left,
			                                                      _modelX->_boundingRect.bottom - _modelX->_boundingRect.top,
			                                                      true));
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdAttach3DX::displayShadowVol(const Math::Matrix4 &modelMat, const Math::Vector3d &light, float extrusionDepth, bool update) {
	Math::Matrix4 finalMat = modelMat * _worldMatrix;

	if (_modelX) {
		if (update) {
			getShadowVolume()->reset();
			_modelX->updateShadowVol(getShadowVolume(), finalMat, light, extrusionDepth);
		}

		_gameRef->_renderer3D->setWorldTransform(finalMat);
		getShadowVolume()->renderToStencilBuffer();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
Common::String AdAttach3DX::getParentBone() {
	return _parentBone;
}

//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool AdAttach3DX::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// PlayAnim / PlayAnimAsync
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "PlayAnim") == 0 || strcmp(name, "PlayAnimAsync") == 0) {
		stack->correctParams(1);
		Common::String animName = stack->pop()->getString();
		if (!_modelX || !_modelX->playAnim(0, animName, 0, true)) {
			stack->pushBool(false);
		} else {
			if (strcmp(name, "PlayAnimAsync") != 0)
				script->waitFor(this);
			stack->pushBool(true);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopAnim
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StopAnim") == 0) {
		stack->correctParams(0);
		bool ret = false;
		if (_modelX) {
			ret = _modelX->stopAnim(0);
		}

		stack->pushBool(ret);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopAnimChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StopAnimChannel") == 0) {
		stack->correctParams(1);
		int channel = stack->pop()->getInt();
		bool ret = false;
		if (_modelX) {
			ret = _modelX->stopAnim(channel, 0);
		}

		stack->pushBool(ret);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlayAnimChannel / PlayAnimChannelAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PlayAnimChannel") == 0 || strcmp(name, "PlayAnimChannelAsync") == 0) {
		stack->correctParams(2);
		int channel = stack->pop()->getInt();
		const char *animName = stack->pop()->getString();

		if (!_modelX || !_modelX->playAnim(channel, animName, 0, true)) {
			stack->pushBool(false);
		} else {
			if (strcmp(name, "PlayAnimChannelAsync") != 0) {
				script->waitFor(this);
			}

			stack->pushBool(true);
		}
		return true;
	}

	else {
		return AdObject3D::scCallMethod(script, stack, thisStack, name);
	}
}

//////////////////////////////////////////////////////////////////////////
ScValue *AdAttach3DX::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("attachment");
		return _scValue;
	} else {
		return AdObject3D::scGetProperty(name);
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdAttach3DX::scSetProperty(const char *name, ScValue *value) {
	return AdObject3D::scSetProperty(name, value);
}

//////////////////////////////////////////////////////////////////////////
const char *AdAttach3DX::scToString() {
	return "[attachment object]";
}

//////////////////////////////////////////////////////////////////////////
bool AdAttach3DX::persist(BasePersistenceManager *persistMgr) {
	AdObject3D::persist(persistMgr);

	persistMgr->transferPtr(TMEMBER(_owner));
	persistMgr->transferString(TMEMBER(_parentBone));

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdAttach3DX::invalidateDeviceObjects() {
	if (_modelX) {
		_modelX->invalidateDeviceObjects();
	}

	if (_shadowModel) {
		_shadowModel->invalidateDeviceObjects();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdAttach3DX::restoreDeviceObjects() {
	if (_modelX) {
		_modelX->restoreDeviceObjects();
	}

	if (_shadowModel) {
		_shadowModel->restoreDeviceObjects();
	}

	return true;
}

} // namespace Wintermute
