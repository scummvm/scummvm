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

#include "engines/metaengine.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/ad/ad_actor_3dx.h"
#include "engines/wintermute/ext/wme_shadowmanager.h"
#include "engines/wintermute/ext/plugin_event.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(SXShadowManager, false)

BaseScriptable *makeSXShadowManager(BaseGame *inGame, ScStack *stack) {
	return new SXShadowManager(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
SXShadowManager::SXShadowManager(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	stack->correctParams(0);
	
	PluginEventEntry event;
	event._type = WME_EVENT_UPDATE;
	event._callback = callback;
	event._plugin = this;
	_gameRef->pluginEvents().subscribeEvent(event);
	
	_defaultLightPos = DXVector3(1.0f, 200.0f, 1.0f);
	_minShadow = 0.1f;
	_maxShadow = 1.0f;
	_useSmartShadows = false;
	_shadowColor = 0x80000000;
}

//////////////////////////////////////////////////////////////////////////
SXShadowManager::~SXShadowManager() {
	PluginEventEntry event;
	event._type = WME_EVENT_UPDATE;
	event._callback = callback;
	event._plugin = this;
	_gameRef->pluginEvents().unsubscribeEvent(event);
}

//////////////////////////////////////////////////////////////////////////
const char *SXShadowManager::scToString() {
	return "[shadowmanager object]";
}

//////////////////////////////////////////////////////////////////////////
bool SXShadowManager::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// Run()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Run") == 0) {
		stack->correctParams(0);

		run();

		stack->pushBool(true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Stop()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Stop") == 0) {
		stack->correctParams(0);

		stop();

		stack->pushBool(true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddActor(string)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "AddActor") == 0) {
		stack->correctParams(1);

		AdObject *obj = (AdObject *)stack->pop()->getNative();
		if (obj) {
			if (strcmp(obj->scGetProperty("Type")->getString(), "actor3dx") == 0) {
				AdActor3DX *actor = (AdActor3DX *)obj;
				stack->pushBool(addActor(actor));
			}
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveActor(string)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "RemoveActor") == 0) {
		stack->correctParams(1);

		// nothing todo

		stack->pushBool(true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveAllActors()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "RemoveAllActors") == 0) {
		stack->correctParams(0);

		stack->pushBool(removeAllActors());

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetNumLights()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetNumLights") == 0) {
		stack->correctParams(0);

		// nothing todo

		stack->pushInt(0);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetLightInfo()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetLightInfo") == 0) {
		stack->correctParams(1);

		// nothing todo

		stack->pushBool(true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetDefaultLightPos(float, float, float)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetDefaultLightPos") == 0) {
		stack->correctParams(3);

		_defaultLightPos._x = stack->pop()->getFloat();
		_defaultLightPos._y = stack->pop()->getFloat();
		_defaultLightPos._z = stack->pop()->getFloat();

		stack->pushBool(true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// EnableLight(string)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "EnableLight") == 0) {
		stack->correctParams(1);
		const char *lightName = stack->pop()->getString();

		stack->pushBool(enableLight(lightName));

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DisableLight(string)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "DisableLight") == 0) {
		stack->correctParams(1);
		const char *lightName = stack->pop()->getString();

		stack->pushBool(disableLight(lightName));

		return STATUS_OK;
	}

	stack->pushNULL();
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
ScValue *SXShadowManager::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// DefaultLightPos
	//////////////////////////////////////////////////////////////////////////
	if (name == "DefaultLightPos") {
		_scValue->setProperty("x", _defaultLightPos._x);
		_scValue->setProperty("y", _defaultLightPos._y);
		_scValue->setProperty("z", _defaultLightPos._z);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// DefaultLightPosX
	//////////////////////////////////////////////////////////////////////////
	else if (name == "DefaultLightPosX") {
		_scValue->setFloat(_defaultLightPos._x);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// DefaultLightPosY
	//////////////////////////////////////////////////////////////////////////
	else if (name == "DefaultLightPosY") {
		_scValue->setFloat(_defaultLightPos._y);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// DefaultLightPosZ
	//////////////////////////////////////////////////////////////////////////
	else if (name == "DefaultLightPosZ") {
		_scValue->setFloat(_defaultLightPos._z);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MinShadow
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MinShadow") {
		_scValue->setFloat(_minShadow);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxShadow
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MaxShadow") {
		_scValue->setFloat(_maxShadow);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// UseSmartShadows
	//////////////////////////////////////////////////////////////////////////
	else if (name == "UseSmartShadows") {
		_scValue->setBool(_useSmartShadows);
		return _scValue;
	}

	return _scValue;
}


//////////////////////////////////////////////////////////////////////////
bool SXShadowManager::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// DefaultLightPos
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "DefaultLightPos") == 0) {
		_defaultLightPos._x = value->getProp("x")->getFloat();
		_defaultLightPos._y = value->getProp("y")->getFloat();
		_defaultLightPos._z = value->getProp("z")->getFloat();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DefaultLightPosX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DefaultLightPosX") == 0) {
		_defaultLightPos._x = value->getFloat();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DefaultLightPosY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DefaultLightPosY") == 0) {
		_defaultLightPos._y = value->getFloat();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DefaultLightPosZ
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DefaultLightPosZ") == 0) {
		_defaultLightPos._z = value->getFloat();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MinShadow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MinShadow") == 0) {
		_minShadow = value->getFloat();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxShadow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxShadow") == 0) {
		_maxShadow = value->getFloat();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UseSmartShadows
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UseSmartShadows") == 0) {
		_useSmartShadows = value->getBool();
		return STATUS_OK;
	}

	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool SXShadowManager::persist(BasePersistenceManager *persistMgr) {
	BaseScriptable::persist(persistMgr);

	if (!persistMgr->getIsSaving()) {
		PluginEventEntry event;
		event._type = WME_EVENT_UPDATE;
		event._callback = callback;
		event._plugin = this;
		_gameRef->pluginEvents().subscribeEvent(event);
		_actors.clear();
		// Actor list is not get restored, plugin is not design work this way.
		// List get refreshed by game script on scene change.
	}

	persistMgr->transferUint32(TMEMBER(_lastTime));
	persistMgr->transferVector3d(TMEMBER(_defaultLightPos));
	persistMgr->transferFloat(TMEMBER(_minShadow));
	persistMgr->transferFloat(TMEMBER(_maxShadow));
	persistMgr->transferBool(TMEMBER(_useSmartShadows));

	return STATUS_OK;
}

void SXShadowManager::callback(void *eventData1, void *eventData2) {
	SXShadowManager *shadowManager = (SXShadowManager *)eventData2;

	uint32 time = shadowManager->_gameRef->scGetProperty("CurrentTime")->getInt();
	if (time - shadowManager->_lastTime > 20) {
		shadowManager->_lastTime = time;
		shadowManager->update();
	}
}

void SXShadowManager::update() {
	if (_useSmartShadows) {
		// TODO: value should be calculated, but for now it's a const
		_shadowColor = 0x66000000;
		for (auto it = _actors.begin(); it != _actors.end(); ++it) {
			it->first->_shadowLightPos = _defaultLightPos;
			it->first->_shadowColor = _shadowColor;
		}
	}
}

void SXShadowManager::run() {
	_lastTime = _gameRef->scGetProperty("CurrentTime")->getInt();
}

void SXShadowManager::stop() {
}

bool SXShadowManager::addActor(AdActor3DX *actorObj) {
	if (_useSmartShadows) {
		_actors.push_back(Common::Pair<AdActor3DX *, uint32>(actorObj, actorObj->_shadowColor));
	}
	return true;
}

bool SXShadowManager::removeAllActors() {
	for (auto it = _actors.begin(); it != _actors.end(); ++it) {
		it->first->_shadowColor = it->second;
		_actors.erase(it);
	}
	return true;
}

bool SXShadowManager::enableLight(const char *lightName) {
	return true;
}

bool SXShadowManager::disableLight(const char *lightName) {
	return true;
}

} // End of namespace Wintermute
