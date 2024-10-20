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

#include "engines/wintermute/ad/ad_attach_3dx.h"
#include "engines/wintermute/base/base_active_rect.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/3dshadow_volume.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdAttach3DX, false)

//////////////////////////////////////////////////////////////////////////
AdAttach3DX::AdAttach3DX(BaseGame *inGame, BaseObject *owner) : AdObject3D(inGame) {
	_owner = owner;
	_dropToFloor = false;
}

//////////////////////////////////////////////////////////////////////////
AdAttach3DX::~AdAttach3DX() {
	_owner = nullptr; // ref only
}

//////////////////////////////////////////////////////////////////////////
bool AdAttach3DX::init(const char *modelFile, const char *name, const char *parentBone) {
	delete _xmodel;
	_xmodel = nullptr;

	_parentBone = parentBone;
	setName(name);

	_xmodel = new XModel(_gameRef, _owner);
	if (!_xmodel) {
		return false;
	}

	bool res = _xmodel->loadFromFile(modelFile);
	if (!res) {
		delete _xmodel;
		_xmodel = nullptr;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
bool AdAttach3DX::update() {
	AdObject3D::update();
	if (_xmodel) {
		return _xmodel->update();
	} else {
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdAttach3DX::displayAttachable(DXMatrix *viewMat, bool registerObjects) {
	DXMatrix finalMat;
	DXMatrixMultiply(&finalMat, &_worldMatrix, viewMat);
	_gameRef->_renderer3D->setWorldTransform(finalMat);

	if (_xmodel) {
		_xmodel->render();

		if (registerObjects && _owner && _owner->_registrable) {
			_gameRef->_renderer->addRectToList(new BaseActiveRect(_gameRef, _owner, _xmodel,
			                                                      _xmodel->_boundingRect.left,
			                                                      _xmodel->_boundingRect.top,
			                                                      _xmodel->_boundingRect.right - _xmodel->_boundingRect.left,
			                                                      _xmodel->_boundingRect.bottom - _xmodel->_boundingRect.top,
			                                                      true));
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdAttach3DX::displayShadowVol(DXMatrix *modelMat, DXVector3 *light, float extrusionDepth, bool update) {
	DXMatrix finalMat;
	DXMatrixMultiply(&finalMat, &_worldMatrix, modelMat);

	if (_xmodel) {
		if (update) {
			getShadowVolume()->reset();
			_xmodel->updateShadowVol(getShadowVolume(), &finalMat, light, extrusionDepth);
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
		if (!_xmodel || !_xmodel->playAnim(0, animName, 0, true)) {
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
		if (_xmodel) {
			ret = _xmodel->stopAnim(0);
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
		if (_xmodel) {
			ret = _xmodel->stopAnim(channel, 0);
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

		if (!_xmodel || !_xmodel->playAnim(channel, animName, 0, true)) {
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
	if (_xmodel) {
		_xmodel->invalidateDeviceObjects();
	}

	if (_shadowModel) {
		_shadowModel->invalidateDeviceObjects();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdAttach3DX::restoreDeviceObjects() {
	if (_xmodel) {
		_xmodel->restoreDeviceObjects();
	}

	if (_shadowModel) {
		_shadowModel->restoreDeviceObjects();
	}

	return true;
}

} // namespace Wintermute
