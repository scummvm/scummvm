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

#ifndef TWINE_SCRIPTLIFE_H
#define TWINE_SCRIPTLIFE_H

#include "common/scummsys.h"
#include "twine/scene/actor.h"

namespace TwinE {

struct LifeScriptContext {
	int32 actorIdx;
	ActorStruct *actor;
	Common::MemorySeekableReadWriteStream stream;
	uint8 *opcodePtr; // local opcode script pointer

	LifeScriptContext(int32 _actorIdx, ActorStruct *_actor) : actorIdx(_actorIdx), actor(_actor), stream(_actor->_lifeScript, _actor->_lifeScriptSize) {
		assert(actor->_offsetLife >= 0);
		stream.skip(_actor->_offsetLife);
		updateOpcodePos();
	}

	void setOpcode(uint8 opcode) {
		*opcodePtr = opcode;
	}

	void updateOpcodePos() {
		opcodePtr = actor->_lifeScript + stream.pos();
	}
};

/**
 * Returns @c -1 Need implementation, @c 0 Condition false, @c 1 - Condition true
 */
typedef int32 ScriptLifeFunc(TwinEEngine *engine, LifeScriptContext &ctx);

struct ScriptLifeFunction {
	const char *name;
	ScriptLifeFunc *function;
};

/** Script condition operators */
enum LifeScriptOperators {
	/*==*/kEqualTo = 0,
	/*> */kGreaterThan = 1,
	/*< */kLessThan = 2,
	/*>=*/kGreaterThanOrEqualTo = 3,
	/*<=*/kLessThanOrEqualTo = 4,
	/*!=*/kNotEqualTo = 5
};

class ScriptLife {
public:
	virtual ~ScriptLife() {}
	/**
	 * Process actor life script
	 * @param actorIdx Current processed actor index
	 */
	virtual void doLife(int32 actorIdx) = 0;
};

} // namespace TwinE

#endif
