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
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/ext/wme_windowmode.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(SXSample, false)

BaseScriptable *makeSXSample(BaseGame *inGame, ScStack *stack) {
	return new SXSample(inGame,  stack);
}

//////////////////////////////////////////////////////////////////////////
SXSample::SXSample(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	stack->correctParams(1);

	int value = stack->pop()->getInt();

	inGame->_renderer->setWindowed(value);
}

//////////////////////////////////////////////////////////////////////////
SXSample::~SXSample() {
}

//////////////////////////////////////////////////////////////////////////
const char *SXSample::scToString() {
	return "[sample object]";
}


//////////////////////////////////////////////////////////////////////////
bool SXSample::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
ScValue *SXSample::scGetProperty(const Common::String &name) {
	_scValue->setNULL();
	return _scValue;
}


//////////////////////////////////////////////////////////////////////////
bool SXSample::scSetProperty(const char *name, ScValue *value) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool SXSample::persist(BasePersistenceManager *persistMgr) {
	BaseScriptable::persist(persistMgr);
	return STATUS_OK;
}

} // End of namespace Wintermute
