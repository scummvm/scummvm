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
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/ext/wme_vlink.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(SXVlink, false)

BaseScriptable *makeSXVlink(BaseGame *inGame, ScStack *stack) {
	return new SXVlink(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
SXVlink::SXVlink(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	stack->correctParams(1);
	uint32 handle = (uint32)stack->pop()->getInt();
	if (handle != 'D3DH') {
		warning("SXVlink() Invalid D3D handle");
	}
}

//////////////////////////////////////////////////////////////////////////
SXVlink::~SXVlink() {
}

//////////////////////////////////////////////////////////////////////////
const char *SXVlink::scToString() {
	return "[binkvideo object]";
}

//////////////////////////////////////////////////////////////////////////
bool SXVlink::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// Play(string path)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Play") == 0) {
		stack->correctParams(1);
		const char *path = stack->pop()->getString();

		warning("SXVlink::Play(%s) missing implementation", path);

		stack->pushNULL();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetVolume(int level)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetVolume") == 0) {
		stack->correctParams(1);
		int level = stack->pop()->getInt();

		warning("SXVlink::SetVolume(%d) missing implementation", level);

		stack->pushNULL();
		return STATUS_OK;
	}

	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
ScValue *SXVlink::scGetProperty(const Common::String &name) {
	_scValue->setNULL();
	return _scValue;
}


//////////////////////////////////////////////////////////////////////////
bool SXVlink::scSetProperty(const char *name, ScValue *value) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool SXVlink::persist(BasePersistenceManager *persistMgr) {
	BaseScriptable::persist(persistMgr);
	return STATUS_OK;
}

} // End of namespace Wintermute
