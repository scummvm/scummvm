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
#include "engines/wintermute/ext/wme_blackandwhite.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(SXBlackAndWhite, false)

BaseScriptable *makeSXBlackAndWhite(BaseGame *inGame, ScStack *stack) {
	return new SXBlackAndWhite(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
SXBlackAndWhite::SXBlackAndWhite(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	stack->correctParams(0);
}

//////////////////////////////////////////////////////////////////////////
SXBlackAndWhite::~SXBlackAndWhite() {
}

//////////////////////////////////////////////////////////////////////////
const char *SXBlackAndWhite::scToString() {
	return "[blackandwhite object]";
}

//////////////////////////////////////////////////////////////////////////
bool SXBlackAndWhite::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// Start()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Start") == 0) {
		stack->correctParams(0);

		// nothing todo

		stack->pushBool(true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Stop()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Stop") == 0) {
		stack->correctParams(0);

		// nothing todo

		stack->pushBool(true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSepia()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetSepia") == 0) {
		stack->correctParams(0);

		setSepia();

		stack->pushBool(true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetBlackAndWhite()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetBlackAndWhite") == 0) {
		stack->correctParams(0);

		setBlackAndWhite();

		stack->pushBool(true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetNormalRender()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetNormalRender") == 0) {
		stack->correctParams(0);

		setNormalRender();

		stack->pushBool(true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetWeightedSepia()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetWeightedSepia") == 0) {
		stack->correctParams(0);

		// nothing todo

		stack->pushBool(true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetWeightedBlackAndWhite()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetWeightedBlackAndWhite") == 0) {
		stack->correctParams(0);

		// nothing todo

		stack->pushBool(true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSepiaBlackAndWhite()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetSepiaBlackAndWhite") == 0) {
		stack->correctParams(0);

		// nothing todo

		stack->pushBool(true);
		return STATUS_OK;
	}


	stack->pushNULL();
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
ScValue *SXBlackAndWhite::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Weight
	//////////////////////////////////////////////////////////////////////////
	if (name == "Weight") {
		// nothing todo
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AllShadersAvailable
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AllShadersAvailable") {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// BlackAndWhiteAvailable
	//////////////////////////////////////////////////////////////////////////
	else if (name == "BlackAndWhiteAvailable") {
		_scValue->setBool(true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SepiaAvailable
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SepiaAvailable") {
		_scValue->setBool(true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WeightedBlackAndWhiteAvailable
	//////////////////////////////////////////////////////////////////////////
	else if (name == "WeightedBlackAndWhiteAvailable") {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WeightedSepiaAvailable
	//////////////////////////////////////////////////////////////////////////
	else if (name == "WeightedSepiaAvailable") {
		_scValue->setBool(false);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SepiaBlackAndWhiteAvailable
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SepiaBlackAndWhiteAvailable") {
		_scValue->setBool(false);
		return _scValue;
	}

	return _scValue;
}


//////////////////////////////////////////////////////////////////////////
bool SXBlackAndWhite::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Weight
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Weight") == 0) {
		// nothing todo
		return STATUS_OK;
	}

	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool SXBlackAndWhite::persist(BasePersistenceManager *persistMgr) {
	BaseScriptable::persist(persistMgr);

	return STATUS_OK;
}

void SXBlackAndWhite::setSepia() {
}

void SXBlackAndWhite::setBlackAndWhite() {
}

void SXBlackAndWhite::setNormalRender() {
}

} // End of namespace Wintermute
