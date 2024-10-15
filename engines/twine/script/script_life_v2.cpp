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

#include "twine/script/script_life_v2.h"
#include "twine/audio/sound.h"
#include "twine/audio/music.h"
#include "twine/movies.h"
#include "twine/menu/interface.h"
#include "twine/scene/animations.h"
#include "twine/parser/anim3ds.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/scene/buggy.h"
#include "twine/scene/movements.h"
#include "twine/scene/wagon.h"
#include "twine/script/script_move_v2.h"
#include "twine/shared.h"
#include "twine/twine.h"

namespace TwinE {

static const ScriptLifeFunction function_map[] = {
	{"END", ScriptLife::lEND},
	{"NOP", ScriptLife::lNOP},
	{"SNIF", ScriptLife::lSNIF},
	{"OFFSET", ScriptLife::lOFFSET},
	{"NEVERIF", ScriptLife::lNEVERIF},
	{"", ScriptLife::lEMPTY}, // unused
	{"NO_IF", ScriptLife::lNO_IF},
	{"", ScriptLife::lEMPTY}, // unused
	{"", ScriptLife::lEMPTY}, // unused
	{"", ScriptLife::lEMPTY}, // unused
	{"PALETTE", ScriptLifeV2::lPALETTE},
	{"RETURN", ScriptLife::lRETURN},
	{"IF", ScriptLife::lIF},
	{"SWIF", ScriptLife::lSWIF},
	{"ONEIF", ScriptLife::lONEIF},
	{"ELSE", ScriptLife::lELSE},
	{"ENDIF", ScriptLife::lEMPTY}, // End of a conditional statement (e.g. IF)
	{"BODY", ScriptLife::lBODY},
	{"BODY_OBJ", ScriptLife::lBODY_OBJ},
	{"ANIM", ScriptLife::lANIM},
	{"ANIM_OBJ", ScriptLife::lANIM_OBJ},
	{"SET_CAMERA", ScriptLifeV2::lSET_CAMERA},
	{"CAMERA_CENTER", ScriptLifeV2::lCAMERA_CENTER},
	{"SET_TRACK", ScriptLife::lSET_TRACK},
	{"SET_TRACK_OBJ", ScriptLife::lSET_TRACK_OBJ},
	{"MESSAGE", ScriptLife::lMESSAGE},
	{"FALLABLE", ScriptLife::lFALLABLE},
	{"SET_DIRMODE", ScriptLife::lSET_DIRMODE},
	{"SET_DIRMODE_OBJ", ScriptLife::lSET_DIRMODE_OBJ},
	{"CAM_FOLLOW", ScriptLife::lCAM_FOLLOW},
	{"SET_BEHAVIOUR", ScriptLife::lSET_BEHAVIOUR},
	{"SET_FLAG_CUBE", ScriptLife::lSET_FLAG_CUBE},
	{"COMPORTEMENT", ScriptLife::lCOMPORTEMENT},
	{"SET_COMPORTEMENT", ScriptLife::lSET_COMPORTEMENT},
	{"SET_COMPORTEMENT_OBJ", ScriptLife::lSET_COMPORTEMENT_OBJ},
	{"END_COMPORTEMENT", ScriptLife::lEND_COMPORTEMENT},
	{"SET_FLAG_GAME", ScriptLifeV2::lSET_FLAG_GAME},
	{"KILL_OBJ", ScriptLife::lKILL_OBJ},
	{"SUICIDE", ScriptLife::lSUICIDE},
	{"USE_ONE_LITTLE_KEY", ScriptLife::lUSE_ONE_LITTLE_KEY},
	{"GIVE_GOLD_PIECES", ScriptLife::lGIVE_GOLD_PIECES},
	{"END_LIFE", ScriptLife::lEND_LIFE},
	{"STOP_L_TRACK", ScriptLife::lSTOP_L_TRACK},
	{"RESTORE_L_TRACK", ScriptLife::lRESTORE_L_TRACK},
	{"MESSAGE_OBJ", ScriptLife::lMESSAGE_OBJ},
	{"INC_CHAPTER", ScriptLife::lINC_CHAPTER},
	{"FOUND_OBJECT", ScriptLife::lFOUND_OBJECT},
	{"SET_DOOR_LEFT", ScriptLife::lSET_DOOR_LEFT},
	{"SET_DOOR_RIGHT", ScriptLife::lSET_DOOR_RIGHT},
	{"SET_DOOR_UP", ScriptLife::lSET_DOOR_UP},
	{"SET_DOOR_DOWN", ScriptLife::lSET_DOOR_DOWN},
	{"GIVE_BONUS", ScriptLife::lGIVE_BONUS},
	{"CHANGE_CUBE", ScriptLife::lCHANGE_CUBE},
	{"OBJ_COL", ScriptLife::lOBJ_COL},
	{"BRICK_COL", ScriptLife::lBRICK_COL},
	{"OR_IF", ScriptLife::lOR_IF},
	{"INVISIBLE", ScriptLife::lINVISIBLE},
	{"SHADOW_OBJ", ScriptLifeV2::lSHADOW_OBJ},
	{"POS_POINT", ScriptLife::lPOS_POINT},
	{"SET_MAGIC_LEVEL", ScriptLife::lSET_MAGIC_LEVEL},
	{"SUB_MAGIC_POINT", ScriptLife::lSUB_MAGIC_POINT},
	{"SET_LIFE_POINT_OBJ", ScriptLife::lSET_LIFE_POINT_OBJ},
	{"SUB_LIFE_POINT_OBJ", ScriptLife::lSUB_LIFE_POINT_OBJ},
	{"HIT_OBJ", ScriptLife::lHIT_OBJ},
	{"PLAY_ACF", ScriptLifeV2::lPLAY_ACF},
	{"ECLAIR", ScriptLifeV2::lECLAIR},
	{"INC_CLOVER_BOX", ScriptLife::lINC_CLOVER_BOX},
	{"SET_USED_INVENTORY", ScriptLife::lSET_USED_INVENTORY},
	{"ADD_CHOICE", ScriptLife::lADD_CHOICE},
	{"ASK_CHOICE", ScriptLife::lASK_CHOICE},
	{"INIT_BUGGY", ScriptLifeV2::lINIT_BUGGY},
	{"MEMO_ARDOISE", ScriptLifeV2::lMEMO_ARDOISE},
	{"SET_HOLO_POS", ScriptLife::lSET_HOLO_POS},
	{"CLR_HOLO_POS", ScriptLife::lCLR_HOLO_POS},
	{"ADD_FUEL", ScriptLife::lADD_FUEL},
	{"SUB_FUEL", ScriptLife::lSUB_FUEL},
	{"SET_GRM", ScriptLife::lSET_GRM},
	{"SET_CHANGE_CUBE", ScriptLifeV2::lSET_CHANGE_CUBE},
	{"MESSAGE_ZOE", ScriptLifeV2::lMESSAGE_ZOE}, // lSAY_MESSAGE
	{"FULL_POINT", ScriptLife::lFULL_POINT},
	{"BETA", ScriptLife::lBETA},
	{"FADE_TO_PAL", ScriptLifeV2::lFADE_TO_PAL},
	{"ACTION", ScriptLifeV2::lACTION},
	{"SET_FRAME", ScriptLifeV2::lSET_FRAME},
	{"SET_SPRITE", ScriptLifeV2::lSET_SPRITE},
	{"SET_FRAME_3DS", ScriptLifeV2::lSET_FRAME_3DS},
	{"IMPACT_OBJ", ScriptLifeV2::lIMPACT_OBJ},
	{"IMPACT_POINT", ScriptLifeV2::lIMPACT_POINT},
	{"ADD_MESSAGE", ScriptLifeV2::lADD_MESSAGE},
	{"BUBBLE", ScriptLifeV2::lBUBBLE},
	{"NO_CHOC", ScriptLifeV2::lNO_CHOC},
	{"ASK_CHOICE_OBJ", ScriptLife::lASK_CHOICE_OBJ},
	{"CINEMA_MODE", ScriptLifeV2::lCINEMA_MODE},
	{"SAVE_HERO", ScriptLifeV2::lSAVE_HERO},
	{"RESTORE_HERO", ScriptLifeV2::lRESTORE_HERO},
	{"ANIM_SET", ScriptLife::lANIM_SET},
	{"RAIN", ScriptLifeV2::lRAIN}, // LM_PLUIE
	{"GAME_OVER", ScriptLife::lGAME_OVER},
	{"THE_END", ScriptLife::lTHE_END},
	{"ESCALATOR", ScriptLifeV2::lESCALATOR},
	{"PLAY_MUSIC", ScriptLifeV2::lPLAY_MUSIC},
	{"TRACK_TO_VAR_GAME", ScriptLifeV2::lTRACK_TO_VAR_GAME},
	{"VAR_GAME_TO_TRACK", ScriptLifeV2::lVAR_GAME_TO_TRACK},
	{"ANIM_TEXTURE", ScriptLifeV2::lANIM_TEXTURE},
	{"ADD_MESSAGE_OBJ", ScriptLifeV2::lADD_MESSAGE_OBJ},
	{"BRUTAL_EXIT", ScriptLife::lBRUTAL_EXIT},
	{"REM", ScriptLife::lEMPTY}, // unused
	{"LADDER", ScriptLifeV2::lLADDER},
	{"SET_ARMOR", ScriptLifeV2::lSET_ARMOR},
	{"SET_ARMOR_OBJ", ScriptLifeV2::lSET_ARMOR_OBJ},
	{"ADD_LIFE_POINT_OBJ", ScriptLifeV2::lADD_LIFE_POINT_OBJ},
	{"STATE_INVENTORY", ScriptLifeV2::lSTATE_INVENTORY},
	{"AND_IF", ScriptLifeV2::lAND_IF},
	{"SWITCH", ScriptLifeV2::lSWITCH},
	{"OR_CASE", ScriptLifeV2::lOR_CASE},
	{"CASE", ScriptLifeV2::lCASE},
	{"DEFAULT", ScriptLife::lEMPTY}, // unused
	{"BREAK", ScriptLifeV2::lBREAK},
	{"END_SWITCH", ScriptLife::lEMPTY}, // unused
	{"SET_HIT_ZONE", ScriptLifeV2::lSET_HIT_ZONE},
	{"SAVE_COMPORTEMENT", ScriptLifeV2::lSAVE_COMPORTEMENT},
	{"RESTORE_COMPORTEMENT", ScriptLifeV2::lRESTORE_COMPORTEMENT},
	{"SAMPLE", ScriptLifeV2::lSAMPLE},
	{"SAMPLE_RND", ScriptLifeV2::lSAMPLE_RND},
	{"SAMPLE_ALWAYS", ScriptLifeV2::lSAMPLE_ALWAYS},
	{"SAMPLE_STOP", ScriptLifeV2::lSAMPLE_STOP},
	{"REPEAT_SAMPLE", ScriptLifeV2::lREPEAT_SAMPLE},
	{"BACKGROUND", ScriptLifeV2::lBACKGROUND},
	{"ADD_VAR_GAME", ScriptLifeV2::lADD_VAR_GAME},
	{"SUB_VAR_GAME", ScriptLifeV2::lSUB_VAR_GAME},
	{"ADD_VAR_CUBE", ScriptLifeV2::lADD_VAR_CUBE},
	{"SUB_VAR_CUBE", ScriptLifeV2::lSUB_VAR_CUBE},
	{"NOP", ScriptLife::lEMPTY}, // unused
	{"SET_RAIL", ScriptLifeV2::lSET_RAIL},
	{"INVERSE_BETA", ScriptLifeV2::lINVERSE_BETA},
	{"NO_BODY", ScriptLifeV2::lNO_BODY},
	{"GIVE_GOLD_PIECES", ScriptLife::lGIVE_GOLD_PIECES},
	{"STOP_L_TRACK_OBJ", ScriptLifeV2::lSTOP_L_TRACK_OBJ},
	{"RESTORE_L_TRACK_OBJ", ScriptLifeV2::lRESTORE_L_TRACK_OBJ},
	{"SAVE_COMPORTEMENT_OBJ", ScriptLifeV2::lSAVE_COMPORTEMENT_OBJ},
	{"RESTORE_COMPORTEMENT_OBJ", ScriptLifeV2::lRESTORE_COMPORTEMENT_OBJ},
	{"SPY", ScriptLife::lEMPTY},       // unused
	{"DEBUG", ScriptLife::lEMPTY},     // unused
	{"DEBUG_OBJ", ScriptLife::lEMPTY}, // unused
	{"POPCORN", ScriptLife::lEMPTY},   // unused
	{"FLOW_POINT", ScriptLifeV2::lFLOW_POINT},
	{"FLOW_OBJ", ScriptLifeV2::lFLOW_OBJ},
	{"SET_ANIM_DIAL", ScriptLifeV2::lSET_ANIM_DIAL},
	{"PCX", ScriptLifeV2::lPCX},
	{"END_MESSAGE", ScriptLifeV2::lEND_MESSAGE},
	{"END_MESSAGE_OBJ", ScriptLifeV2::lEND_MESSAGE_OBJ},
	{"PARM_SAMPLE", ScriptLifeV2::lPARM_SAMPLE},
	{"NEW_SAMPLE", ScriptLifeV2::lNEW_SAMPLE},
	{"POS_OBJ_AROUND", ScriptLifeV2::lPOS_OBJ_AROUND},
	{"PCX_MESS_OBJ", ScriptLifeV2::lPCX_MESS_OBJ}
};

int32 ScriptLifeV2::lPALETTE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 palIndex = engine->_screens->mapLba2Palette(ctx.stream.readByte());
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::PALETTE(%i)", palIndex);
	engine->saveTimer(false);
	HQR::getPaletteEntry(engine->_screens->_ptrPal, Resources::HQR_RESS_FILE, palIndex);
	engine->setPalette(engine->_screens->_ptrPal);
	engine->_screens->_flagPalettePcx = true;
	engine->restoreTimer();
	return 0;
}

int32 ScriptLifeV2::lFADE_TO_PAL(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 palIndex = engine->_screens->mapLba2Palette(ctx.stream.readByte());
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::FADE_TO_PAL(%i)", palIndex);
	// TODO: implement
	return -1;
}

int32 ScriptLifeV2::lPLAY_MUSIC(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lPLAY_MUSIC()");
	const int32 val = lPLAY_CD_TRACK(engine, ctx);
	if (engine->isLBA2()) {
		engine->_scene->_cubeJingle = 255;
		engine->_music->_nextMusic = -1;
		if (engine->_gameState->hasGameFlag(157) > 0) {
			engine->_music->_stopLastMusic = false;
		}
	}
	return val;
}

int32 ScriptLifeV2::lTRACK_TO_VAR_GAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 num = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lTRACK_TO_VAR_GAME(%i)", (int)num);
	engine->_gameState->setGameFlag(num, MAX<int32>(0, ctx.actor->_labelTrack));
	return 0;
}

#define TM_END 0
#define TM_NOP 1
#define TM_BODY 2
#define TM_ANIM 3
#define TM_GOTO_POINT 4
#define TM_WAIT_ANIM 5
#define TM_LOOP 6
#define TM_ANGLE 7
#define TM_POS_POINT 8
#define TM_LABEL 9
#define TM_GOTO 10
#define TM_STOP 11
#define TM_GOTO_SYM_POINT 12
#define TM_WAIT_NB_ANIM 13
#define TM_SAMPLE 14
#define TM_GOTO_POINT_3D 15
#define TM_SPEED 16
#define TM_BACKGROUND 17
#define TM_WAIT_NB_SECOND 18
#define TM_NO_BODY 19
#define TM_BETA 20
#define TM_OPEN_LEFT 21
#define TM_OPEN_RIGHT 22
#define TM_OPEN_UP 23
#define TM_OPEN_DOWN 24
#define TM_CLOSE 25
#define TM_WAIT_DOOR 26
#define TM_SAMPLE_RND 27
#define TM_SAMPLE_ALWAYS 28
#define TM_SAMPLE_STOP 29
#define TM_PLAY_ACF 30
#define TM_REPEAT_SAMPLE 31
#define TM_SIMPLE_SAMPLE 32
#define TM_FACE_TWINSEN 33
#define TM_ANGLE_RND 34
#define TM_REM 35
#define TM_WAIT_NB_DIZIEME 36
#define TM_DO 37
#define TM_SPRITE 38
#define TM_WAIT_NB_SECOND_RND 39
#define TM_AFF_TIMER 40
#define TM_SET_FRAME 41
#define TM_SET_FRAME_3DS 42
#define TM_SET_START_3DS 43
#define TM_SET_END_3DS 44
#define TM_START_ANIM_3DS 45
#define TM_STOP_ANIM_3DS 46
#define TM_WAIT_ANIM_3DS 47
#define TM_WAIT_FRAME_3DS 48
#define TM_WAIT_NB_DIZIEME_RND 49
#define TM_DECALAGE 50
#define TM_FREQUENCE 51
#define TM_VOLUME 52

// Allows forcing a track to a label instead of an offset (basically, like in the tool!)
int16 ScriptLifeV2::searchOffsetTrack(ActorStruct *ptrobj, uint8 label) {
	uint8 *ptrtrack;
	int16 offsettrack = 0;
	uint8 macro;

	for (;;) {
		ptrtrack = ptrobj->_ptrTrack + offsettrack;

		macro = *ptrtrack++;
		offsettrack++;

		switch (macro) {
		case TM_LABEL:
			if (*ptrtrack == label) {
				ptrobj->_labelTrack = label; /* label */
				ptrobj->_offsetLabelTrack = (int16)(offsettrack - 1);
				return ((int16)(offsettrack - 1));
			}
			offsettrack++;
			break;

		case TM_BODY:
		case TM_GOTO_POINT:
		case TM_GOTO_POINT_3D:
		case TM_GOTO_SYM_POINT:
		case TM_POS_POINT:
		case TM_BACKGROUND:
		case TM_SET_FRAME:
		case TM_SET_FRAME_3DS:
		case TM_SET_START_3DS:
		case TM_SET_END_3DS:
		case TM_START_ANIM_3DS:
		case TM_WAIT_FRAME_3DS:
		case TM_VOLUME:
			offsettrack++;
			break;

		case TM_ANIM:
		case TM_SAMPLE:
		case TM_SAMPLE_RND:
		case TM_SAMPLE_ALWAYS:
		case TM_SAMPLE_STOP:
		case TM_REPEAT_SAMPLE:
		case TM_SIMPLE_SAMPLE:
		case TM_WAIT_NB_ANIM:
		case TM_ANGLE:
		case TM_FACE_TWINSEN:
		case TM_OPEN_LEFT:
		case TM_OPEN_RIGHT:
		case TM_OPEN_UP:
		case TM_OPEN_DOWN:
		case TM_BETA:
		case TM_GOTO:
		case TM_SPEED:
		case TM_SPRITE:
		case TM_DECALAGE:
		case TM_FREQUENCE:
			offsettrack += 2;
			break;

		case TM_ANGLE_RND:
		case TM_LOOP:
			offsettrack += 4;
			break;

		case TM_WAIT_NB_SECOND:
		case TM_WAIT_NB_SECOND_RND:
		case TM_WAIT_NB_DIZIEME:
		case TM_WAIT_NB_DIZIEME_RND:
			offsettrack += 5;
			break;

		case TM_PLAY_ACF: {
			char *ptr, c;
			char string[256];
			int32 n;

			ptr = string;
			n = 0;
			do {
				c = *ptrtrack++;
				*ptr++ = c;
				n++;
			} while (c != 0);

			offsettrack += n;
			break;
		}
		case TM_END:
			return -1; // label not found
		}
	}
	return -1;
}

void ScriptLifeV2::cleanTrack(ActorStruct *ptrobj) {
	if (ptrobj->_offsetTrack != -1) {
		ptrobj->_workFlags.bTRACK_MASTER_ROT = 0;

		uint8 *ptrtrack = ptrobj->_ptrTrack + ptrobj->_offsetTrack;

		switch (*ptrtrack) {
		case TM_FACE_TWINSEN:
			*(int16 *)(ptrtrack + 1) = -1;
			break;

		case TM_WAIT_NB_ANIM:
			*(uint8 *)(ptrtrack + 2) = 0;
			break;

		case TM_WAIT_NB_SECOND:
		case TM_WAIT_NB_DIZIEME:
		case TM_WAIT_NB_SECOND_RND:
		case TM_WAIT_NB_DIZIEME_RND:
			*(uint32 *)(ptrtrack + 2) = 0;
			break;

		case TM_ANGLE:
			*(int16 *)(ptrtrack + 1) &= 0x7FFF;
			break;

		case TM_ANGLE_RND:
			*(int16 *)(ptrtrack + 3) = -1;
			break;

		case TM_LOOP:
			*(ptrtrack + 2) = *(ptrtrack + 1);
			break;
		}
	}
}

int32 ScriptLifeV2::lVAR_GAME_TO_TRACK(TwinEEngine *engine, LifeScriptContext &ctx) {
	cleanTrack(ctx.actor);

	const uint8 num = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lVAR_GAME_TO_TRACK(%i)", (int)num);
	ctx.actor->_offsetTrack = searchOffsetTrack(ctx.actor,
											engine->_gameState->hasGameFlag(num));

	// Shielding against bug of the traveling merchant
	if (ctx.actor->_offsetTrack < 0) {
		ctx.actor->_offsetTrack = searchOffsetTrack(ctx.actor, 0);
	}
	return 0;
}

int32 ScriptLifeV2::lANIM_TEXTURE(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_interface->_animateTexture = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lANIM_TEXTURE(%i)", (int)engine->_interface->_animateTexture);
	return 0;
}

int32 ScriptLifeV2::lADD_MESSAGE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const TextId textIdx = (TextId)ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lADD_MESSAGE_OBJ(%i)", (int)textIdx);
	// TODO: implement me
	return -1;
}

int32 ScriptLifeV2::lADD_MESSAGE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const TextId textIdx = (TextId)ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lADD_MESSAGE(%i)", (int)textIdx);
	// TODO: implement me
	return -1;
}

int32 ScriptLifeV2::lCAMERA_CENTER(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 angle = ClampAngle(ToAngle(ctx.stream.readByte() * 1024));
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lCAMERA_CENTER(%i)", (int)angle);
	// TODO: implement me - see centerOnActor in grid
	// AddBetaCam = num ;
	// CameraCenter( 2 ) ;
	// FirstTime = AFF_ALL_FLIP ;
	return -1;
}

int32 ScriptLifeV2::lBUBBLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_text->_showDialogueBubble = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::BUBBLE(%s)", engine->_text->_showDialogueBubble ? "true" : "false");
	return 0;
}

int32 ScriptLifeV2::lNO_CHOC(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 val = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lNO_CHOC(%i)", (int)val);
	ctx.actor->_flags.bNoElectricShock = val;
	return 0;
}

int32 ScriptLifeV2::lCINEMA_MODE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 val = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lCINEMA_MODE(%i)", (int)val);
	// TODO: implement me
	return -1;
}

int32 ScriptLifeV2::lSAVE_HERO(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::SAVE_HERO()");
	engine->_actor->_saveHeroBehaviour = engine->_actor->_heroBehaviour;
	ActorStruct *actor = engine->_scene->getActor(OWN_ACTOR_SCENE_INDEX);
	actor->_saveGenBody = actor->_genBody;
	return 0;
}

int32 ScriptLifeV2::lRESTORE_HERO(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::RESTORE_HERO()");
	engine->_actor->setBehaviour(engine->_actor->_saveHeroBehaviour);
	ActorStruct *actor = engine->_scene->getActor(OWN_ACTOR_SCENE_INDEX);
	engine->_actor->initBody(actor->_saveGenBody, OWN_ACTOR_SCENE_INDEX);
	return 0;
}

int32 ScriptLifeV2::lRAIN(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 num = (int)ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lRAIN(%i)", (int)num);
	int32 n = engine->_redraw->addOverlay(OverlayType::koRain, 0, 0, 0, 0, OverlayPosType::koNormal, 1);
	if (n != -1) {
		// Rain n/10s
		engine->_redraw->overlayList[n].timerEnd = engine->timerRef + engine->toSeconds(num / 10);
		engine->_flagRain = true;
		engine->_sound->startRainSample();
	}
	return 0;
}

int32 ScriptLifeV2::lESCALATOR(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint8 info1 = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lESCALATOR(%i, %i)", (int)num, (int)info1);
	for (int n = 0; n < engine->_scene->_sceneNumZones; n++) {
		ZoneStruct &zone = engine->_scene->_sceneZones[n];
		if (zone.type == ZoneType::kEscalator && zone.num == num) {
			zone.infoData.generic.info1 = info1;
		}
	}
	return 0;
}

int32 ScriptLifeV2::lSET_CAMERA(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint8 info7 = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSET_CAMERA(%i, %i)", (int)num, (int)info7);
	for (int n = 0; n < engine->_scene->_sceneNumZones; n++) {
		ZoneStruct &zone = engine->_scene->_sceneZones[n];
		if (zone.type == ZoneType::kCamera && zone.num == num) {
			zone.infoData.generic.info7 = info7;
			if (info7)
				zone.infoData.generic.info7 |= ZONE_ON;
			else
				zone.infoData.generic.info7 &= ~(ZONE_ON);

			zone.infoData.generic.info7 &= ~(ZONE_ACTIVE);
		}
	}
	return 0;
}

int32 ScriptLifeV2::lPLAY_ACF(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->saveTimer(false);
	int strIdx = 0;
	char movie[64];
	do {
		const byte c = ctx.stream.readByte();
		movie[strIdx++] = c;
		if (c == '\0') {
			break;
		}
		if (strIdx >= ARRAYSIZE(movie)) {
			error("Max string size exceeded for acf name");
		}
	} while (true);
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lPLAY_ACF(%s)", movie);

	engine->_movie->playMovie(movie);
	// TODO: lba2 is doing more stuff here - reset the cinema mode, init the scene and palette stuff
	engine->setPalette(engine->_screens->_ptrPal);
	engine->restoreTimer();
	engine->_redraw->_firstTime = true;

	return -1;
}

int32 ScriptLifeV2::lSHADOW_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int actorIdx = ctx.stream.readByte();
	const bool castShadow = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::SHADOW_OBJ(%i, %s)", actorIdx, castShadow ? "true" : "false");
	ActorStruct *actor = engine->_scene->getActor(actorIdx);
	if (actor->_lifePoint > 0) {
		actor->_flags.bNoShadow = !castShadow;
	}
	return 0;
}

int32 ScriptLifeV2::lECLAIR(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 num = (int)ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lECLAIR(%i)", (int)num);
	int32 n = engine->_redraw->addOverlay(OverlayType::koFlash, 0, 0, 0, 0, OverlayPosType::koNormal, 1);
	if (n != -1) {
		// Eclair n/10s
		engine->_redraw->overlayList[n].timerEnd = engine->timerRef + engine->toSeconds(num / 10);
	}
	return 0;
}

int32 ScriptLifeV2::lINIT_BUGGY(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lINIT_BUGGY(%i)", (int)num);
	engine->_buggy->initBuggy(ctx.actorIdx, num);
	return 0;
}

int32 ScriptLifeV2::lMEMO_ARDOISE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lMEMO_ARDOISE(%i)", (int)num);
	if (engine->_gameState->hasGameFlag(GAMEFLAG_ARDOISE)) {
		// TODO: implement me
	}
	return -1;
}

int32 ScriptLifeV2::lSET_CHANGE_CUBE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint8 info = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSET_CHANGE_CUBE(%i, %i)", (int)num, (int)info);
	int n = 0;
	while (n < engine->_scene->_sceneNumZones) {
		ZoneStruct &zone = engine->_scene->_sceneZones[n];
		if (zone.type == ZoneType::kCube && zone.num == num) {
			if (info) {
				zone.infoData.generic.info7 |= ZONE_ON;
			} else {
				zone.infoData.generic.info7 &= ~ZONE_ON;
			}
		}
		++n;
	}
	return 0;
}

int32 ScriptLifeV2::lMESSAGE_ZOE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 textIdx = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lMESSAGE_ZOE(%i)", (int)textIdx);
	// TODO: implement me
	return -1;
}

int32 ScriptLifeV2::lACTION(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::ACTION()");
	engine->_movements->setActionNormal(true);
	return 0;
}

int32 ScriptLifeV2::lSET_FRAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int frame = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSET_FRAME(%i)", (int)frame);
	if (!ctx.actor->_flags.bSprite3D) {
		// TODO: ObjectSetFrame(ctx.actorIdx, frame);
	}
	return -1;
}

int32 ScriptLifeV2::lSET_SPRITE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 num = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSET_SPRITE(%i)", (int)num);
	if (ctx.actor->_flags.bSprite3D) {
		engine->_actor->initSprite(num, ctx.actorIdx);
	}
	return 0;
}

int32 ScriptLifeV2::lSET_FRAME_3DS(TwinEEngine *engine, LifeScriptContext &ctx) {
	int sprite = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSET_FRAME_3DS(%i)", (int)sprite);
	if (ctx.actor->_flags.bHasSpriteAnim3D) {
		const T_ANIM_3DS *anim = engine->_resources->getAnim(ctx.actor->A3DS.Num);
		if (sprite >= anim->Fin - anim->Deb) {
			sprite = anim->Fin - anim->Deb;
		}
		sprite += anim->Deb;
		engine->_actor->initSprite(sprite, ctx.actorIdx);
	}
	return 0;
}

int32 ScriptLifeV2::lIMPACT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint16 n = ctx.stream.readUint16LE();
	const int16 y = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lIMPACT_OBJ(%i, %i, %i)", (int)num, (int)n, (int)y);
	ActorStruct *otherActor = engine->_scene->getActor(num);
	if (otherActor->_lifePoint > 0) {
		// TODO: DoImpact(n, otherActor->_pos.x, otherActor->_pos.y + y, otherActor->_pos.z, num);
	}
	return -1;
}

int32 ScriptLifeV2::lIMPACT_POINT(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 brickTrackId = ctx.stream.readByte();
	const uint16 n = ctx.stream.readUint16LE();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lIMPACT_POINT(%i, %i)", (int)brickTrackId, (int)n);
	// const IVec3 &pos = engine->_scene->_sceneTracks[brickTrackId];
	// int16 x0 = pos.x;
	// int16 y0 = pos.y;
	// int16 z0 = pos.z;
	// TODO: DoImpact(n, x0, y0, z0, ctx.actorIdx);
	return -1;
}

// ECHELLE
int32 ScriptLifeV2::lLADDER(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint8 info = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lLADDER(%i, %i)", (int)num, (int)info);
	int n = 0;
	while (n < engine->_scene->_sceneNumZones) {
		ZoneStruct &zone = engine->_scene->_sceneZones[n];
		if (zone.type == ZoneType::kLadder && zone.num == num) {
			zone.infoData.generic.info1 = (int32)info;
		}
		++n;
	}
	return 0;
}

// SET_ARMURE
int32 ScriptLifeV2::lSET_ARMOR(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int8 armor = ctx.stream.readSByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSET_ARMOR(%i)", (int)armor);
	ctx.actor->_armor = (int32)armor;
	return 0;
}

// SET_ARMURE_OBJ
int32 ScriptLifeV2::lSET_ARMOR_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const int8 armor = ctx.stream.readSByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSET_ARMOR_OBJ(%i, %i)", (int)num, (int)armor);
	if (ActorStruct *actor = engine->_scene->getActor(num)) {
		actor->_armor = (int32)armor;
	}
	return 0;
}

int32 ScriptLifeV2::lADD_LIFE_POINT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint8 life = ctx.stream.readByte();
	ActorStruct *actor = engine->_scene->getActor(num);
	if (actor->_workFlags.bIsDead) {
		actor->_workFlags.bIsDead = false;
		engine->_actor->initBody(BodyType::btNormal, num);
		engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeRepeat, AnimationTypes::kStanding, num);
	}
	actor->setLife(actor->_lifePoint + life);
	return 0;
}

int32 ScriptLifeV2::lSTATE_INVENTORY(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte(); // num vargame
	const uint8 idObj3D = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSTATE_INVENTORY(%i, %i)", (int)num, (int)idObj3D);
	// TODO: TabInv[num].IdObj3D = idObj3D;
	return -1;
}

int32 ScriptLifeV2::lAND_IF(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSWITCH(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lOR_CASE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lCASE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lDEFAULT(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lBREAK(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lEND_SWITCH(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_HIT_ZONE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint8 info1 = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSET_HIT_ZONE(%i, %i)", (int)num, (int)info1);
	for (int n = 0; n < engine->_scene->_sceneNumZones; n++) {
		ZoneStruct &zone = engine->_scene->_sceneZones[n];
		if (zone.type == ZoneType::kHit && zone.num == num) {
			zone.infoData.generic.info1 = info1;
		}
	}
	return 0;
}

int32 ScriptLifeV2::lSAVE_COMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSAVE_COMPORTEMENT()");
	ctx.actor->_saveOffsetLife = ctx.actor->_offsetLife;
	return 0;
}

int32 ScriptLifeV2::lRESTORE_COMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lRESTORE_COMPORTEMENT()");
	ctx.actor->_offsetLife = ctx.actor->_saveOffsetLife;
	return 0;
}

int32 ScriptLifeV2::lSAMPLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 sample = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSAMPLE(%i)", (int)sample);
	// TODO: HQ_3D_MixSample(sample, 0x1000, 0, 1, ctx.actor->posObj());
	return -1;
}

int32 ScriptLifeV2::lSAMPLE_RND(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 sample = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSAMPLE_RND(%i)", (int)sample);
	// TODO: HQ_3D_MixSample(sample, 0x800, 0x1000, 1, ctx.actor->posObj());
	return -1;
}

int32 ScriptLifeV2::lSAMPLE_ALWAYS(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 sample = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSAMPLE_ALWAYS(%i)", (int)sample);
	// TODO:
	return -1;
}

int32 ScriptLifeV2::lSAMPLE_STOP(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 sample = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSAMPLE_STOP(%i)", (int)sample);
	// TODO:
	return -1;
}

int32 ScriptLifeV2::lREPEAT_SAMPLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 sample = ctx.stream.readSint16LE();
	uint8 repeat = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lREPEAT_SAMPLE(%i, %i)", (int)sample, (int)repeat);
	// TODO: HQ_3D_MixSample(sample, 0x1000, 0, repeat, ctx.actor->posObj());
	return -1;
}

int32 ScriptLifeV2::lBACKGROUND(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_FLAG_GAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const int16 val = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::SET_FLAG_GAME(%i, %i)", (int)num, (int)val);
	engine->_gameState->setGameFlag(num, val);

	if (num == GAMEFLAG_MONEY) {
		if (engine->_scene->_planet >= 2) {
			engine->_gameState->setKashes(val);
		} else {
			engine->_gameState->setZlitos(val);
		}
	}

	return 0;
}

int32 ScriptLifeV2::lADD_VAR_GAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const int16 val = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lADD_VAR_GAME(%i, %i)", (int)num, (int)val);
	int16 currentVal = engine->_gameState->hasGameFlag(num);
	if ((int)currentVal + (int)val < 32767) {
		currentVal += val;
	} else {
		currentVal = 32767;
	}

	if (num == GAMEFLAG_MONEY) {
		if (engine->_scene->_planet >= 2) {
			engine->_gameState->setKashes(currentVal);
		} else {
			engine->_gameState->setZlitos(currentVal);
		}
	}
	engine->_gameState->setGameFlag(num, currentVal);
	return 0;
}

int32 ScriptLifeV2::lSUB_VAR_GAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const int16 val = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lADD_VAR_GAME(%i, %i)", (int)num, (int)val);
	int16 currentVal = engine->_gameState->hasGameFlag(num);
	if ((int)currentVal - (int)val > -32768) {
		currentVal -= val;
	} else {
		currentVal = -32768;
	}

	if (num == GAMEFLAG_MONEY) {
		if (engine->_scene->_planet >= 2) {
			engine->_gameState->setKashes(currentVal);
		} else {
			engine->_gameState->setZlitos(currentVal);
		}
	}
	engine->_gameState->setGameFlag(num, currentVal);
	return 0;
}

int32 ScriptLifeV2::lADD_VAR_CUBE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint8 amount = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lADD_VAR_CUBE(%i, %i)", (int)num, (int)amount);
	uint8 &current = engine->_scene->_listFlagCube[num];
	if ((int16)current + (int16)amount < 255) {
		current += amount;
	} else {
		current = 255;
	}
	return 0;
}

int32 ScriptLifeV2::lSUB_VAR_CUBE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint8 amount = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSUB_VAR_CUBE(%i, %i)", (int)num, (int)amount);
	uint8 &current = engine->_scene->_listFlagCube[num];
	if ((int16)current - (int16)amount > 0) {
		current -= amount;
	} else {
		current = 0;
	}
	return 0;
}

int32 ScriptLifeV2::lSET_RAIL(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	const uint8 info1 = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lSET_RAIL(%i, %i)", (int)num, (int)info1);
	for (int n = 0; n < engine->_scene->_sceneNumZones; n++) {
		ZoneStruct &zone = engine->_scene->_sceneZones[n];
		if (zone.type == ZoneType::kRail && zone.num == num) {
			zone.infoData.generic.info1 = info1;
		}
	}
	return 0;
}

int32 ScriptLifeV2::lINVERSE_BETA(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.actor->_beta = ClampAngle(ctx.actor->_beta + LBAAngles::ANGLE_180);

	if (ctx.actor->_controlMode == ControlMode::kWagon) {
#if 0 // TODO:
		ctx.actor->Info1 = 1; // reinit speed wagon

		// to be clean
		APtObj = ctx.actor;
#endif

		// SizeSHit contains the number of the brick under the wagon hack
		// test front axle position
		engine->_wagon->AdjustEssieuWagonAvant(ctx.actor->SizeSHit);
		// test rear axle position
		engine->_wagon->AdjustEssieuWagonArriere(ctx.actor->SizeSHit);
	}

	// To tell an object that it is no longer being carried by me
	engine->_actor->checkCarrier(ctx.actorIdx);
	return -1;
}

int32 ScriptLifeV2::lNO_BODY(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lNO_BODY()");
	engine->_actor->initBody(BodyType::btNone, ctx.actorIdx);
	return 0;
}

int32 ScriptLifeV2::lSTOP_L_TRACK_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lRESTORE_L_TRACK_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSAVE_COMPORTEMENT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lRESTORE_COMPORTEMENT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSPY(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lDEBUG(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lDEBUG_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lPOPCORN(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lPOPCORN()");
	// empty on purpose
	return 0;
}

int32 ScriptLifeV2::lFLOW_POINT(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lFLOW_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_ANIM_DIAL(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lPCX(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lEND_MESSAGE(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lEND_MESSAGE()");
	// empty on purpose
	return 0;
}

int32 ScriptLifeV2::lEND_MESSAGE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScriptsLife, "LIFE::lEND_MESSAGE_OBJ(%i)", (int)num);
	return 0;
}

int32 ScriptLifeV2::lPARM_SAMPLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lNEW_SAMPLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lPOS_OBJ_AROUND(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lPCX_MESS_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

ScriptLifeV2::ScriptLifeV2(TwinEEngine *engine) : ScriptLife(engine, function_map, ARRAYSIZE(function_map)) {
}

} // namespace TwinE
