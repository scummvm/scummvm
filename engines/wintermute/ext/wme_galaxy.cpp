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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/metaengine.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/ext/wme_galaxy.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(SXWMEGalaxyAPI, false)

BaseScriptable *makeSXWMEGalaxyAPI(BaseGame *inGame, ScStack *stack) {
	return new SXWMEGalaxyAPI(inGame,  stack);
}

//////////////////////////////////////////////////////////////////////////
SXWMEGalaxyAPI::SXWMEGalaxyAPI(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	stack->correctParams(0);
	init();
}

//////////////////////////////////////////////////////////////////////////
void SXWMEGalaxyAPI::init() {
	const MetaEngine *meta = g_engine->getMetaEngine();
	const Common::String target = BaseEngine::instance().getGameTargetName();
	AchMan.setActiveDomain(meta->getAchievementsInfo(target));
}


//////////////////////////////////////////////////////////////////////////
SXWMEGalaxyAPI::~SXWMEGalaxyAPI() {
}


//////////////////////////////////////////////////////////////////////////
const char *SXWMEGalaxyAPI::scToString() {
	return "[wmegalaxyapi object]";
}


//////////////////////////////////////////////////////////////////////////
bool SXWMEGalaxyAPI::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// InitGalaxy()
	// Initialization is already done at the constructor instead
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "InitGalaxy") == 0) {
		stack->correctParams(2);
		const char *clientId = stack->pop()->getString();
		const char *clientSecret = stack->pop()->getString();

		AchMan.setSpecialString("clientId", clientId);
		AchMan.setSpecialString("clientSecret", clientSecret);

		stack->pushNULL();
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

	else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *SXWMEGalaxyAPI::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("wmegalaxyapi");
		return _scValue;
	}

	else {
		return _scValue;
	}
}


//////////////////////////////////////////////////////////////////////////
bool SXWMEGalaxyAPI::scSetProperty(const char *name, ScValue *value) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool SXWMEGalaxyAPI::persist(BasePersistenceManager *persistMgr) {
	BaseScriptable::persist(persistMgr);

	if (!persistMgr->getIsSaving()) {
		init();
	}

	return STATUS_OK;
}

} // End of namespace Wintermute
