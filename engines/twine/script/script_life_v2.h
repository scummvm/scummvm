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

#ifndef TWINE_SCRIPTLIFEV2_H
#define TWINE_SCRIPTLIFEV2_H

#include "twine/script/script_life.h"

namespace TwinE {

class TwinEEngine;

class ScriptLifeV2 : public ScriptLife {
public:
	static int32 lPLAY_MUSIC(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lTRACK_TO_VAR_GAME(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lVAR_GAME_TO_TRACK(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lANIM_TEXTURE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lADD_MESSAGE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lADD_MESSAGE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lBUBBLE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lNO_CHOC(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSAVE_HERO(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lRESTORE_HERO(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lCINEMA_MODE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lESCALATOR(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lRAIN(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lCAMERA_CENTER(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_CAMERA(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSHADOW_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lPLAY_ACF(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lECLAIR(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lINIT_BUGGY(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lMEMO_ARDOISE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_CHANGE_CUBE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lMESSAGE_ZOE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lFADE_TO_PAL(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lACTION(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_FRAME(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_SPRITE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_FRAME_3DS(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lIMPACT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lIMPACT_POINT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lPALETTE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lLADDER(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_ARMOR(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_ARMOR_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lADD_LIFE_POINT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSTATE_INVENTORY(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lAND_IF(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSWITCH(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lOR_CASE (TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lCASE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lDEFAULT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lBREAK(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lEND_SWITCH(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_HIT_ZONE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSAVE_COMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lRESTORE_COMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSAMPLE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSAMPLE_RND(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSAMPLE_ALWAYS(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSAMPLE_STOP (TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lREPEAT_SAMPLE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lBACKGROUND(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lADD_VAR_GAME(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSUB_VAR_GAME(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lADD_VAR_CUBE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSUB_VAR_CUBE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_RAIL(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lINVERSE_BETA(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lNO_BODY(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSTOP_L_TRACK_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lRESTORE_L_TRACK_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSAVE_COMPORTEMENT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lRESTORE_COMPORTEMENT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSPY(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lDEBUG(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lDEBUG_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lPOPCORN(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lFLOW_POINT(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lFLOW_OBJ (TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lSET_ANIM_DIAL(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lPCX(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lEND_MESSAGE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lEND_MESSAGE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lPARM_SAMPLE(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lNEW_SAMPLE (TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lPOS_OBJ_AROUND(TwinEEngine *engine, LifeScriptContext &ctx);
	static int32 lPCX_MESS_OBJ(TwinEEngine *engine, LifeScriptContext &ctx);

	ScriptLifeV2(TwinEEngine *engine);
};

} // namespace TwinE

#endif
