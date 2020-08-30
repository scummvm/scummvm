/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "engines/wintermute/ext/wme_commandlinehelper.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(SXCommandLineHelper, false)

BaseScriptable *makeSXCommandLineHelper(BaseGame *inGame, ScStack *stack) {
	return new SXCommandLineHelper(inGame,  stack);
}


//////////////////////////////////////////////////////////////////////////
SXCommandLineHelper::SXCommandLineHelper(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	stack->correctParams(0);
	_gameRef->LOG(0, "new SXCommandLineHelper()");
}


//////////////////////////////////////////////////////////////////////////
SXCommandLineHelper::~SXCommandLineHelper() {
}


//////////////////////////////////////////////////////////////////////////
const char *SXCommandLineHelper::scToString() {
	return "[commandlinehelper object]";
}


//////////////////////////////////////////////////////////////////////////
bool SXCommandLineHelper::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
ScValue *SXCommandLineHelper::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Parameters (RO)
	// Used to launch demo: "Pizza Morgana: Episode 1 - Monsters and Manipulations in the Magical Forest"
	//////////////////////////////////////////////////////////////////////////
	if (name == "Parameters") {
		_scValue->setString("Pizza.exe DEMO");
		return _scValue;
	}

	else {
		return _scValue;
	}
}


//////////////////////////////////////////////////////////////////////////
bool SXCommandLineHelper::scSetProperty(const char *name, ScValue *value) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool SXCommandLineHelper::persist(BasePersistenceManager *persistMgr) {
	BaseScriptable::persist(persistMgr);

	return STATUS_OK;
}

} // End of namespace Wintermute
