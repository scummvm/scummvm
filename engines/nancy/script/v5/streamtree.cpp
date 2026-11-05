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

#include "common/algorithm.h"
#include "common/debug.h"

#include "engines/nancy/nancy.h"

#include "engines/nancy/script/graphnode.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/record.h"
#include "engines/nancy/script/records.h"
#include "engines/nancy/script/scenecontext.h"

#include "engines/nancy/script/v5/streamtree.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/state/scenestream.h"

namespace Nancy {
namespace State {

Script::StreamNodeState *SceneStreamManager::getNodeState(const Common::String &name) {
	if (name.equalsIgnoreCase(getMainStreamName())) {
		return &_mainNodeState;
	}

	SceneStream *stream = find(name);
	return stream ? &stream->getNodeState() : nullptr;
}

int SceneStreamManager::receiveNancy26Message(lua_State *L, const Common::String &sender, const Common::String &name, const Common::String &action) {
	Script::StreamNodeState *state = getNodeState(name);
	if (!state) {
		return 0;
	}

	Common::String upperAction = action;
	upperAction.toUppercase();
	if (upperAction == "ENABLE") {
		state->isEnabled = true;
	} else if (upperAction == "DISABLE") {
		state->isEnabled = false;
	} else if (upperAction == "PLAY") {
		state->isPaused = false;
	} else if (upperAction == "PAUSE") {
		state->isPaused = true;
	} else if (upperAction == "LOCK") {
		state->isPaused = true;
		state->isEnabled = false;
	} else if (upperAction == "UNLOCK") {
		state->isPaused = false;
		state->isEnabled = true;
	} else if (upperAction == "ISPAUSED") {
		lua_pushboolean(L, state->isPaused);
		return 1;
	}

	debugC(1, kDebugScene, "%s asked stream %s to %s", sender.c_str(), name.c_str(), action.c_str());
	return 0;
}

} // End of namespace State
} // End of namespace Nancy
