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
 * This file is based on WME Steam Plugin.
 * https://archive.softwareheritage.org/browse/origin/directory/?origin_url=https://bitbucket.org/MnemonicWME/wme_steam_plugin
 * Copyright (c) 2013 Jan Nedoma
 */

#include "engines/metaengine.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/ext/wme_steam.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(SXSteamAPI, false)

BaseScriptable *makeSXSteamAPI(BaseGame *inGame, ScStack *stack) {
	return new SXSteamAPI(inGame,  stack);
}

//////////////////////////////////////////////////////////////////////////
Common::AchievementsInfo getAchievementsInfo() {
	const MetaEngine *meta = g_engine->getMetaEngine();
	const Common::String target = BaseEngine::instance().getGameTargetName();
	return meta->getAchievementsInfo(target);
}

//////////////////////////////////////////////////////////////////////////
SXSteamAPI::SXSteamAPI(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	stack->correctParams(0);
	init();
}

//////////////////////////////////////////////////////////////////////////
void SXSteamAPI::init() {
	_achievementsInfo = getAchievementsInfo();
	AchMan.setActiveDomain(_achievementsInfo);
}


//////////////////////////////////////////////////////////////////////////
SXSteamAPI::~SXSteamAPI() {
}


//////////////////////////////////////////////////////////////////////////
const char *SXSteamAPI::scToString() {
	return "[steamapi object]";
}


//////////////////////////////////////////////////////////////////////////
bool SXSteamAPI::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// RequestStats()
	// There are currently no known games that are using this
	// So, all the initialization should be done at the constructor instead
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "RequestStats") == 0) {
		stack->correctParams(0);
		stack->pushBool(AchMan.isReady());
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetAchievement(string id)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetAchievement") == 0) {
		stack->correctParams(1);
		const char *id = stack->pop()->getString();
		stack->pushBool(AchMan.setAchievement(id));
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// IsAchieved(string id)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsAchieved") == 0) {
		stack->correctParams(1);
		const char *id = stack->pop()->getString();
		stack->pushBool(AchMan.isAchieved(id));
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// ClearAchievement(string id)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ClearAchievement") == 0) {
		stack->correctParams(1);
		const char *id = stack->pop()->getString();
		stack->pushBool(AchMan.clearAchievement(id));
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetAchievementId(int index)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetAchievementId") == 0) {
		stack->correctParams(1);
		uint32 index = (uint32) stack->pop()->getInt();

		const Common::AchievementDescription *descr = AchMan.getAchievementDescription(index);
		if (descr) {
			stack->pushString(descr->id.c_str());
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetStat(string id, int|float value)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetStat") == 0) {
		stack->correctParams(2);
		const char *id = stack->pop()->getString();
		ScValue *val = stack->pop();

		if (val->isFloat()) {
			stack->pushBool(AchMan.setStatFloat(id, val->getFloat()));
		} else {
			stack->pushBool(AchMan.setStatInt(id, val->getInt()));
		}

		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetStatInt(string id)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetStatInt") == 0) {
		stack->correctParams(1);
		const char *id = stack->pop()->getString();
		stack->pushInt(AchMan.getStatInt(id));
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetStatFloat(string id)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetStatFloat") == 0) {
		stack->correctParams(1);
		const char *id = stack->pop()->getString();
		stack->pushFloat(AchMan.getStatFloat(id));
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// ResetAllStats(bool includingAchievements)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ResetAllStats") == 0) {
		stack->correctParams(1);
		bool includingAchievements = stack->pop()->getBool();

		bool result = AchMan.resetAllStats();
		if (includingAchievements) {
			result = result && AchMan.resetAllAchievements();
		}

		stack->pushBool(result);
		return STATUS_OK;
	}

	else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *SXSteamAPI::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("steamapi");
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// SteamAvailable (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SteamAvailable") {
		_scValue->setBool(AchMan.isReady());
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// StatsAvailable (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "StatsAvailable") {
		_scValue->setBool(AchMan.isReady());
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// NumAchievements (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "NumAchievements") {
		_scValue->setInt(AchMan.getAchievementCount());
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// AppId (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AppId") {
		_scValue->setInt(atoi(_achievementsInfo.appId.c_str()));
		return _scValue;
	}

	else {
		return _scValue;
	}
}


//////////////////////////////////////////////////////////////////////////
bool SXSteamAPI::scSetProperty(const char *name, ScValue *value) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool SXSteamAPI::persist(BasePersistenceManager *persistMgr) {
	BaseScriptable::persist(persistMgr);

	if (!persistMgr->getIsSaving()) {
		init();
	}

	return STATUS_OK;
}

} // End of namespace Wintermute
