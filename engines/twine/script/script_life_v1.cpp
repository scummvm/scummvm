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

#include "twine/script/script_life_v1.h"
#include "common/debug.h"
#include "twine/twine.h"
#include "twine/text.h"
#include "twine/audio/music.h"
#include "twine/scene/gamestate.h"

namespace TwinE {

/**
 * Turn on bubbles while actors talk.
 * @note Opcode @c 0x59
 */
int32 ScriptLifeV1::lBUBBLE_ON(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::BUBBLE_ON()");
	engine->_text->_showDialogueBubble = true;
	return 0;
}

/**
 * Turn off bubbles while actors talk.
 * @note Opcode @c 0x5A
 */
int32 ScriptLifeV1::lBUBBLE_OFF(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::BUBBLE_OFF()");
	engine->_text->_showDialogueBubble = false;
	return 0;
}

/**
 * Play Midis (Parameter = Midis Index)
 * @note Opcode @c 0x41
 */
int32 ScriptLifeV1::lPLAY_MIDI(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 midiIdx = ctx.stream.readByte();
	engine->_music->playMusic(midiIdx);
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::PLAY_MIDI(%i)", (int)midiIdx);
	return 0;
}

/**
 * Stop the current played midi.
 * @note Opcode @c 0x63
 */
int32 ScriptLifeV1::lMIDI_OFF(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::MIDI_OFF()");
	engine->_music->stopMusicMidi();
	return 0;
}

/**
 * Set a new value for the game flag (Paramter = Game Flag Index, Parameter = Value)
 * @note Opcode @c 0x24
 */
int32 ScriptLifeV1::lSET_FLAG_GAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	const uint8 flagIdx = ctx.stream.readByte();
	const uint8 flagValue = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_FLAG_GAME(%i, %i)", (int)flagIdx, (int)flagValue);
	engine->_gameState->setGameFlag(flagIdx, flagValue);
	return 0;
}

static const ScriptLifeFunction function_map[] = {
	{"END", ScriptLifeV1::lEND},
	{"NOP", ScriptLifeV1::lNOP},
	{"SNIF", ScriptLifeV1::lSNIF},
	{"OFFSET", ScriptLifeV1::lOFFSET},
	{"NEVERIF", ScriptLifeV1::lNEVERIF},
	{"", ScriptLifeV1::lEMPTY}, // unused
	{"NO_IF", ScriptLifeV1::lNO_IF},
	{"", ScriptLifeV1::lEMPTY}, // unused
	{"", ScriptLifeV1::lEMPTY}, // unused
	{"", ScriptLifeV1::lEMPTY}, // unused
	{"LABEL", ScriptLifeV1::lLABEL},
	{"RETURN", ScriptLifeV1::lRETURN},
	{"IF", ScriptLifeV1::lIF},
	{"SWIF", ScriptLifeV1::lSWIF},
	{"ONEIF", ScriptLifeV1::lONEIF},
	{"ELSE", ScriptLifeV1::lELSE},
	{"ENDIF", ScriptLifeV1::lEMPTY}, // End of a conditional statement (e.g. IF)
	{"BODY", ScriptLifeV1::lBODY},
	{"BODY_OBJ", ScriptLifeV1::lBODY_OBJ},
	{"ANIM", ScriptLifeV1::lANIM},
	{"ANIM_OBJ", ScriptLifeV1::lANIM_OBJ},
	{"SET_LIFE", ScriptLifeV1::lSET_LIFE},
	{"SET_LIFE_OBJ", ScriptLifeV1::lSET_LIFE_OBJ},
	{"SET_TRACK", ScriptLifeV1::lSET_TRACK},
	{"SET_TRACK_OBJ", ScriptLifeV1::lSET_TRACK_OBJ},
	{"MESSAGE", ScriptLifeV1::lMESSAGE},
	{"FALLABLE", ScriptLifeV1::lFALLABLE},
	{"SET_DIRMODE", ScriptLifeV1::lSET_DIRMODE},
	{"SET_DIRMODE_OBJ", ScriptLifeV1::lSET_DIRMODE_OBJ},
	{"CAM_FOLLOW", ScriptLifeV1::lCAM_FOLLOW},
	{"SET_BEHAVIOUR", ScriptLifeV1::lSET_BEHAVIOUR},
	{"SET_FLAG_CUBE", ScriptLifeV1::lSET_FLAG_CUBE},
	{"COMPORTEMENT", ScriptLifeV1::lCOMPORTEMENT},
	{"SET_COMPORTEMENT", ScriptLifeV1::lSET_COMPORTEMENT},
	{"SET_COMPORTEMENT_OBJ", ScriptLifeV1::lSET_COMPORTEMENT_OBJ},
	{"END_COMPORTEMENT", ScriptLifeV1::lEND_COMPORTEMENT},
	{"SET_FLAG_GAME", ScriptLifeV1::lSET_FLAG_GAME},
	{"KILL_OBJ", ScriptLifeV1::lKILL_OBJ},
	{"SUICIDE", ScriptLifeV1::lSUICIDE},
	{"USE_ONE_LITTLE_KEY", ScriptLifeV1::lUSE_ONE_LITTLE_KEY},
	{"GIVE_GOLD_PIECES", ScriptLifeV1::lGIVE_GOLD_PIECES},
	{"END_LIFE", ScriptLifeV1::lEND_LIFE},
	{"STOP_L_TRACK", ScriptLifeV1::lSTOP_L_TRACK},
	{"RESTORE_L_TRACK", ScriptLifeV1::lRESTORE_L_TRACK},
	{"MESSAGE_OBJ", ScriptLifeV1::lMESSAGE_OBJ},
	{"INC_CHAPTER", ScriptLifeV1::lINC_CHAPTER},
	{"FOUND_OBJECT", ScriptLifeV1::lFOUND_OBJECT},
	{"SET_DOOR_LEFT", ScriptLifeV1::lSET_DOOR_LEFT},
	{"SET_DOOR_RIGHT", ScriptLifeV1::lSET_DOOR_RIGHT},
	{"SET_DOOR_UP", ScriptLifeV1::lSET_DOOR_UP},
	{"SET_DOOR_DOWN", ScriptLifeV1::lSET_DOOR_DOWN},
	{"GIVE_BONUS", ScriptLifeV1::lGIVE_BONUS},
	{"CHANGE_CUBE", ScriptLifeV1::lCHANGE_CUBE},
	{"OBJ_COL", ScriptLifeV1::lOBJ_COL},
	{"BRICK_COL", ScriptLifeV1::lBRICK_COL},
	{"OR_IF", ScriptLifeV1::lOR_IF},
	{"INVISIBLE", ScriptLifeV1::lINVISIBLE},
	{"ZOOM", ScriptLifeV1::lZOOM},
	{"POS_POINT", ScriptLifeV1::lPOS_POINT},
	{"SET_MAGIC_LEVEL", ScriptLifeV1::lSET_MAGIC_LEVEL},
	{"SUB_MAGIC_POINT", ScriptLifeV1::lSUB_MAGIC_POINT},
	{"SET_LIFE_POINT_OBJ", ScriptLifeV1::lSET_LIFE_POINT_OBJ},
	{"SUB_LIFE_POINT_OBJ", ScriptLifeV1::lSUB_LIFE_POINT_OBJ},
	{"HIT_OBJ", ScriptLifeV1::lHIT_OBJ},
	{"PLAY_FLA", ScriptLifeV1::lPLAY_FLA},
	{"PLAY_MIDI", ScriptLifeV1::lPLAY_MIDI},
	{"INC_CLOVER_BOX", ScriptLifeV1::lINC_CLOVER_BOX},
	{"SET_USED_INVENTORY", ScriptLifeV1::lSET_USED_INVENTORY},
	{"ADD_CHOICE", ScriptLifeV1::lADD_CHOICE},
	{"ASK_CHOICE", ScriptLifeV1::lASK_CHOICE},
	{"BIG_MESSAGE", ScriptLifeV1::lBIG_MESSAGE},
	{"INIT_PINGOUIN", ScriptLifeV1::lINIT_PINGOUIN},
	{"SET_HOLO_POS", ScriptLifeV1::lSET_HOLO_POS},
	{"CLR_HOLO_POS", ScriptLifeV1::lCLR_HOLO_POS},
	{"ADD_FUEL", ScriptLifeV1::lADD_FUEL},
	{"SUB_FUEL", ScriptLifeV1::lSUB_FUEL},
	{"SET_GRM", ScriptLifeV1::lSET_GRM},
	{"SAY_MESSAGE", ScriptLifeV1::lSAY_MESSAGE},
	{"SAY_MESSAGE_OBJ", ScriptLifeV1::lSAY_MESSAGE_OBJ},
	{"FULL_POINT", ScriptLifeV1::lFULL_POINT},
	{"BETA", ScriptLifeV1::lBETA},
	{"GRM_OFF", ScriptLifeV1::lGRM_OFF},
	{"FADE_PAL_RED", ScriptLifeV1::lFADE_PAL_RED},
	{"FADE_ALARM_RED", ScriptLifeV1::lFADE_ALARM_RED},
	{"FADE_ALARM_PAL", ScriptLifeV1::lFADE_ALARM_PAL},
	{"FADE_RED_PAL", ScriptLifeV1::lFADE_RED_PAL},
	{"FADE_RED_ALARM", ScriptLifeV1::lFADE_RED_ALARM},
	{"FADE_PAL_ALARM", ScriptLifeV1::lFADE_PAL_ALARM},
	{"EXPLODE_OBJ", ScriptLifeV1::lEXPLODE_OBJ},
	{"BUBBLE_ON", ScriptLifeV1::lBUBBLE_ON},
	{"BUBBLE_OFF", ScriptLifeV1::lBUBBLE_OFF},
	{"ASK_CHOICE_OBJ", ScriptLifeV1::lASK_CHOICE_OBJ},
	{"SET_DARK_PAL", ScriptLifeV1::lSET_DARK_PAL},
	{"SET_NORMAL_PAL", ScriptLifeV1::lSET_NORMAL_PAL},
	{"MESSAGE_SENDELL", ScriptLifeV1::lMESSAGE_SENDELL},
	{"ANIM_SET", ScriptLifeV1::lANIM_SET},
	{"HOLOMAP_TRAJ", ScriptLifeV1::lHOLOMAP_TRAJ},
	{"GAME_OVER", ScriptLifeV1::lGAME_OVER},
	{"THE_END", ScriptLifeV1::lTHE_END},
	{"MIDI_OFF", ScriptLifeV1::lMIDI_OFF},
	{"PLAY_CD_TRACK", ScriptLifeV1::lPLAY_CD_TRACK},
	{"PROJ_ISO", ScriptLifeV1::lPROJ_ISO},
	{"PROJ_3D", ScriptLifeV1::lPROJ_3D},
	{"TEXT", ScriptLifeV1::lTEXT},
	{"CLEAR_TEXT", ScriptLifeV1::lCLEAR_TEXT},
	{"BRUTAL_EXIT", ScriptLifeV1::lBRUTAL_EXIT}
};

ScriptLifeV1::ScriptLifeV1(TwinEEngine *engine) : ScriptLife(engine, function_map, ARRAYSIZE(function_map)) {
}

} // namespace TwinE
