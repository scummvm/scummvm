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
private:
	TwinEEngine *_engine;
	const ScriptLifeFunction* _functionMap;
	size_t _functionMapSize;

public:
	static int32 lEMPTY(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lEND(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lNOP(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSNIF(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lOFFSET(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lNEVERIF(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lNO_IF(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lLABEL(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lRETURN(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lIF(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSWIF(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lONEIF(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lELSE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lBODY(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lBODY_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lANIM(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lANIM_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_LIFE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_LIFE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_TRACK(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_TRACK_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lMESSAGE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lFALLABLE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_DIRMODE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_DIRMODE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lCAM_FOLLOW(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_BEHAVIOUR(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_FLAG_CUBE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lCOMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_COMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_COMPORTEMENT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lEND_COMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_FLAG_GAME(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lKILL_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSUICIDE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lUSE_ONE_LITTLE_KEY(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lGIVE_GOLD_PIECES(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lEND_LIFE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSTOP_L_TRACK(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lRESTORE_L_TRACK(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lMESSAGE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lINC_CHAPTER(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lFOUND_OBJECT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_DOOR_LEFT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_DOOR_RIGHT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_DOOR_UP(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_DOOR_DOWN(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lGIVE_BONUS(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lCHANGE_CUBE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lOBJ_COL(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lBRICK_COL(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lOR_IF(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lINVISIBLE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lZOOM(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lPOS_POINT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_MAGIC_LEVEL(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSUB_MAGIC_POINT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_LIFE_POINT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSUB_LIFE_POINT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lHIT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lPLAY_FLA(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lINC_CLOVER_BOX(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_USED_INVENTORY(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lADD_CHOICE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lASK_CHOICE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lBIG_MESSAGE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lINIT_PINGOUIN(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_HOLO_POS(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lCLR_HOLO_POS(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lADD_FUEL(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSUB_FUEL(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_GRM(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSAY_MESSAGE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSAY_MESSAGE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lFULL_POINT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lBETA(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lGRM_OFF(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lFADE_PAL_RED(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lFADE_ALARM_RED(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lFADE_ALARM_PAL(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lFADE_RED_PAL(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lFADE_RED_ALARM(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lFADE_PAL_ALARM(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lEXPLODE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);

	static int32 lASK_CHOICE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_DARK_PAL(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_NORMAL_PAL(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lMESSAGE_SENDELL(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lANIM_SET(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lHOLOMAP_TRAJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lGAME_OVER(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lTHE_END(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lPLAY_CD_TRACK(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lPROJ_ISO(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lPROJ_3D(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lTEXT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lCLEAR_TEXT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lBRUTAL_EXIT(TwinEEngine *engine, LifeScriptContext &ctx);

public:
	ScriptLife(TwinEEngine *engine, const ScriptLifeFunction* functionMap, size_t entries);

	virtual ~ScriptLife() {}
	/**
	 * Process actor life script
	 * @param actorIdx Current processed actor index
	 */
	void doLife(int32 actorIdx);
};

} // namespace TwinE

#endif
