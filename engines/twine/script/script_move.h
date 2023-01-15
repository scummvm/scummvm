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

#ifndef TWINE_SCRIPTMOVE_H
#define TWINE_SCRIPTMOVE_H

#include "common/scummsys.h"
#include "twine/scene/actor.h"

namespace TwinE {

struct MoveScriptContext {
	int32 actorIdx;
	ActorStruct *actor;
	int32 numRepeatSample = 1;

	Common::MemorySeekableReadWriteStream stream;

	MoveScriptContext(int32 _actorIdx, ActorStruct *_actor) : actorIdx(_actorIdx), actor(_actor), stream(actor->_moveScript, actor->_moveScriptSize) {
		assert(actor->_offsetTrack >= 0);
		stream.skip(actor->_offsetTrack);
	}

	void undo(int32 bytes) {
		assert(bytes >= 0);
		// the additional 1 byte is for the opcode
		stream.rewind(bytes + 1);
	}
};

/**
 * Returns @c -1 Need implementation, @c 0 Condition false, @c 1 - Condition true
 */
typedef int32 ScriptMoveFunc(TwinEEngine *engine, MoveScriptContext &ctx);

struct ScriptMoveFunction {
	const char *name;
	ScriptMoveFunc *function;
};

class ScriptMove {
private:
	TwinEEngine *_engine;
	const ScriptMoveFunction* _functionMap;
	size_t _functionMapSize;

public:
	ScriptMove(TwinEEngine *engine, const ScriptMoveFunction* functionMap, size_t entries);
	virtual ~ScriptMove() {}
	/**
	 * Process actor move script
	 * @param actorIdx Current processed actor index
	 */
	void doTrack(int32 actorIdx);
};

} // namespace TwinE

#endif
