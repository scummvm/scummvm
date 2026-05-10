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
#include "engines/wintermute/ext/wme_protection.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(SXProtection, false)

BaseScriptable *makeSXProtection(BaseGame *inGame, ScStack *stack) {
	return new SXProtection(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
SXProtection::SXProtection(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	stack->correctParams(0);
}

//////////////////////////////////////////////////////////////////////////
SXProtection::~SXProtection() {
}

//////////////////////////////////////////////////////////////////////////
const char *SXProtection::scToString() {
	return "[protection object]";
}


//////////////////////////////////////////////////////////////////////////
bool SXProtection::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// CheckExeProtection
	// Reference in game scripts in "Susan Rose: Mysterious Child"
	// Return value can be 0 or 1
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "CheckExeProtection") == 0) {
		stack->correctParams(0);

		stack->pushInt(0);
		return STATUS_OK;
	}


	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
ScValue *SXProtection::scGetProperty(const char *name) {
	_scValue->setNULL();
	return _scValue;
}


//////////////////////////////////////////////////////////////////////////
bool SXProtection::scSetProperty(const char *name, ScValue *value) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool SXProtection::persist(BasePersistenceManager *persistMgr) {
	BaseScriptable::persist(persistMgr);
	return STATUS_OK;
}

} // End of namespace Wintermute
