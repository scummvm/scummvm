/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "logic.h"
#include "text.h"
#include "sound.h"
#include "eventman.h"
#include "menu.h"
#include "common/util.h"
#include "router.h"
#include "screen.h"
#include "mouse.h"
#include "sword1.h"
#include "music.h"

#include "debug.h"

#define MAX_STACK_SIZE 10
#define SCRIPT_VERSION  13
#define LAST_FRAME 999

uint32 SwordLogic::_scriptVars[NUM_SCRIPT_VARS];

SwordLogic::SwordLogic(ObjectMan *pObjMan, ResMan *resMan, SwordScreen *pScreen, SwordMouse *pMouse, SwordSound *pSound, SwordMusic *pMusic, SwordMenu *pMenu) {
	_objMan = pObjMan;
	_resMan = resMan;
	_screen = pScreen;
	_mouse = pMouse;
	_music = pMusic;
	_sound = pSound;
	_menu = pMenu;
	_textMan = new SwordText(_objMan, _resMan, false);
	_screen->useTextManager(_textMan);
	_router = new SwordRouter(_objMan, _resMan);

	//_collision = new SwordCollision(_objMan, this);
	_eventMan = new EventManager();
	
	memset(_scriptVars, 0, NUM_SCRIPT_VARS * sizeof(uint32));
	for (uint8 cnt = 0; cnt < NON_ZERO_SCRIPT_VARS; cnt++)
		_scriptVars[_scriptVarInit[cnt][0]] = _scriptVarInit[cnt][1];

	_textRunning = _speechRunning = false;
	_speechFinished = true;
}

void SwordLogic::newScreen(uint32 screen) {
	BsObject *compact = (BsObject*)_objMan->fetchObject(PLAYER);

	//Tdebug("locked player");

	if (SwordEngine::_systemVars.justRestoredGame)	// if we've just restored a game - we want George to be exactly as saved
	{
		if (_scriptVars[GEORGE_WALKING])	// except that if George was walking when we saveed the game
		{
			fnStandAt(compact, PLAYER, _scriptVars[CHANGE_X], _scriptVars[CHANGE_Y], _scriptVars[CHANGE_DIR], _scriptVars[CHANGE_STANCE], 0,0);
			fnIdle(compact,PLAYER,0,0,0,0,0,0);
			_scriptVars[GEORGE_WALKING] = 0;
		}

		SwordEngine::_systemVars.justRestoredGame = 0;
	}
	else	// if we haven't just restored a game, set George to stand, etc
	{
		compact->o_screen = _scriptVars[NEW_SCREEN]; //move the mega/player at this point between screens
		//Tdebug("FN_stand_at...");
		fnStandAt(compact, PLAYER, _scriptVars[CHANGE_X], _scriptVars[CHANGE_Y], _scriptVars[CHANGE_DIR], _scriptVars[CHANGE_STANCE], 0,0);
		//Tdebug("FN_change_floor...");
		fnChangeFloor(compact, PLAYER, _scriptVars[CHANGE_PLACE], 0, 0, 0, 0, 0);
		//Tdebug("done");
	}
}

void SwordLogic::engine(void) {
	_eventMan->serviceGlobalEventList();

	for (uint16 sectCnt = 0; sectCnt < TOTAL_SECTIONS; sectCnt++) {
		if (_objMan->sectionAlive(sectCnt)) {
			uint32 numCpts = _objMan->fetchNoObjects(sectCnt);
			for (uint32 cptCnt = 0; cptCnt < numCpts; cptCnt++) {
				uint32 currentId = sectCnt * ITM_PER_SEC + cptCnt;
				BsObject *compact = _objMan->fetchObject(currentId);

				if (compact->o_status & STAT_LOGIC) { // does the object want to be processed?
					if (compact->o_status & STAT_EVENTS) {
						//subscribed to the global-event-switcher? and in logic mode
						switch(compact->o_logic) {
							case LOGIC_pause_for_event:
							case LOGIC_idle:
							case LOGIC_AR_animate:
								_eventMan->checkForEvent(compact);
								break;
						}
					}
					debug(7, "SwordLogic::engine: handling compact %d (%X)", currentId, currentId);
					processLogic(compact, currentId);
					compact->o_sync = 0; // syncs are only available for 1 cycle.
				}

				if ((uint32)compact->o_screen == _scriptVars[SCREEN]) {
					if (compact->o_status & STAT_FORE)
						_screen->addToGraphicList(0, currentId);
					if (compact->o_status & STAT_SORT)
						_screen->addToGraphicList(1, currentId);
					if (compact->o_status & STAT_BACK)
						_screen->addToGraphicList(2, currentId);

					if (compact->o_status & STAT_MOUSE)
						_mouse->addToList(currentId, compact);
				}
			}
		}
	}
	//_collision->checkCollisions();

}

void SwordLogic::processLogic(BsObject *compact, uint32 id) {
	int logicRet;
	do {
		switch(compact->o_logic) {
			case LOGIC_idle:
				logicRet = 0;
				break;
			case LOGIC_pause:
			case LOGIC_pause_for_event:
				if (compact->o_pause) {
					compact->o_pause--;
					logicRet = 0;
				} else {
					compact->o_logic = LOGIC_script;
					logicRet = 1;
				}
				break;
			case LOGIC_quit:
				compact->o_logic = LOGIC_script;
				logicRet = 0;
				break;
			case LOGIC_wait_for_sync:
				if (compact->o_sync) {
					logicRet = 1;
					compact->o_logic = LOGIC_script;
				} else
					logicRet = 0;
				break;
			case LOGIC_choose:
				_scriptVars[CUR_ID] = id;
				logicRet = _menu->logicChooser(compact);
				break;
			case LOGIC_wait_for_talk:
				logicRet = logicWaitTalk(compact);
				break;
			case LOGIC_start_talk:
				logicRet = logicStartTalk(compact);
			case LOGIC_script:
				_scriptVars[CUR_ID] = id;
				logicRet = scriptManager(compact, id);
				break;
			case LOGIC_new_script:
				compact->o_tree.o_script_pc[compact->o_tree.o_script_level] = _newScript;
				compact->o_tree.o_script_id[compact->o_tree.o_script_level] = _newScript;
				compact->o_logic = LOGIC_script;
				logicRet = 1;
				break;
			case LOGIC_AR_animate:
				logicRet = logicArAnimate(compact, id);
				break;
			case LOGIC_restart:
				compact->o_tree.o_script_pc[compact->o_tree.o_script_level] = compact->o_tree.o_script_id[compact->o_tree.o_script_level];
				compact->o_logic = LOGIC_script;
				logicRet=1;
				break;
			case LOGIC_bookmark:
                memcpy(&(compact->o_tree.o_script_level), &(compact->o_bookmark.o_script_level), sizeof(ScriptTree));
				compact->o_logic = LOGIC_script;
				logicRet = 1;
				break;
			case LOGIC_speech:
				logicRet = speechDriver(compact);
				break;
			case LOGIC_full_anim:
				logicRet = fullAnimDriver(compact);
				break;
			case LOGIC_anim:
				logicRet = animDriver(compact);
				break;

			default:
				error("Fatal error: compact %d's logic == %X!", id, compact->o_logic);
				break;
		}
	} while(logicRet);
}

int SwordLogic::logicWaitTalk(BsObject *compact) {
	BsObject *target = _objMan->fetchObject(compact->o_down_flag);
	
	if (target->o_status & STAT_TALK_WAIT) {
		compact->o_logic = LOGIC_script;
		return 1;
	} else {
		return 0;
	}
}

int SwordLogic::logicStartTalk(BsObject *compact) {
	BsObject *target = _objMan->fetchObject(compact->o_down_flag); //holds id of person we're waiting for
	if (target->o_status & STAT_TALK_WAIT) { //response?
		compact->o_logic = LOGIC_script; //back to script again
		return SCRIPT_CONT;
	}
	if (_eventMan->eventValid(compact->o_down_flag))
		return SCRIPT_STOP; //event still valid - keep waiting
	//the event has gone - so back to script with error code
	compact->o_down_flag = 0;
	compact->o_logic = LOGIC_script;
	return SCRIPT_CONT;
}

int SwordLogic::logicArAnimate(BsObject *compact, uint32 id) {
	WalkData *route;
	int32 walkPc;
	if ((_scriptVars[GEORGE_WALKING] == 0) && (id == PLAYER))
		_scriptVars[GEORGE_WALKING] = 1;
	
	compact->o_resource = compact->o_walk_resource;
	compact->o_status |= STAT_SHRINK;
	route = compact->o_route;

	walkPc				=compact->o_walk_pc;
	compact->o_frame	=route[walkPc].frame;
	compact->o_dir		=route[walkPc].dir;
	compact->o_xcoord	=route[walkPc].x;
	compact->o_ycoord	=route[walkPc].y;
	compact->o_anim_x	=compact->o_xcoord;
	compact->o_anim_y	=compact->o_ycoord;

	if (((_scriptVars[GEORGE_WALKING] == 2) && (walkPc > 5) && (id == PLAYER) && 
		(route[walkPc - 1].step == 5) && (route[walkPc].step == 0)) ||
		((_scriptVars[GEORGE_WALKING] == 3) && (id == PLAYER))) {

			compact->o_frame = 96 + compact->o_dir;						//reset
			if	((compact->o_dir != 2) && (compact->o_dir != 6)) { // on verticals and diagonals stand where george is
				compact->o_xcoord	= route[walkPc - 1].x;
				compact->o_ycoord	= route[walkPc - 1].y;
				compact->o_anim_x	= compact->o_xcoord;
				compact->o_anim_y	= compact->o_ycoord;
			}
			compact->o_logic = LOGIC_script;
			compact->o_down_flag = 0;		//0 means error
			_scriptVars[GEORGE_WALKING] = 0;
			route[compact->o_walk_pc+1].frame = 512;					//end of sequence
			if (_scriptVars[MEGA_ON_GRID] == 2)
				_scriptVars[MEGA_ON_GRID] = 0;
	}
	compact->o_walk_pc++;

	if	(route[compact->o_walk_pc].frame == 512)					//end of sequence
	{
		compact->o_logic = LOGIC_script;
		if (((_scriptVars[GEORGE_WALKING] == 2) || (_scriptVars[GEORGE_WALKING] == 1)) &&
			(id == PLAYER)) {
				_scriptVars[GEORGE_WALKING] = 0;
				if (_scriptVars[MEGA_ON_GRID] == 2)
					_scriptVars[MEGA_ON_GRID] = 0;
		}
	}
	return 0;
}

int SwordLogic::speechDriver(BsObject *compact) {
	if ((!_speechClickDelay) && (_mouse->testEvent() & BS1L_BUTTON_DOWN))
		_speechFinished = true;
	if (_speechClickDelay)
		_speechClickDelay--;

	if (_speechRunning) {
		if (_sound->speechFinished())
			_speechFinished = true;
	} else {
		if (!compact->o_speech_time)
			_speechFinished = true;
		else
			compact->o_speech_time--;
	}
	if (_speechFinished) {
		if (_speechRunning)
			_sound->stopSpeech();
		compact->o_logic = LOGIC_script;
		if (_textRunning) {
			_textMan->releaseText(compact->o_text_id);
			_objMan->fetchObject(compact->o_text_id)->o_status = 0; // kill compact linking text sprite
		}
		_speechRunning = _textRunning = false;
		_speechFinished = true;
	}
	if (compact->o_anim_resource) {
		uint8 *animData = ((uint8*)_resMan->openFetchRes(compact->o_anim_resource)) + sizeof(Header);
		int32 numFrames = READ_LE_UINT32(animData);
		animData += 4;
		compact->o_anim_pc++; // go to next frame of anim

		if (_speechFinished || (compact->o_anim_pc >= numFrames) || 
			(_speechRunning && (_sound->amISpeaking() == 0)))
				compact->o_anim_pc = 0; //set to frame 0, closed mouth
		
		AnimUnit *animPtr = (AnimUnit*)(animData + sizeof(AnimUnit) * compact->o_anim_pc);
		if (!(compact->o_status & STAT_SHRINK)) {
			compact->o_anim_x = FROM_LE_32(animPtr->animX);
			compact->o_anim_y = FROM_LE_32(animPtr->animY);
		}
		compact->o_frame = FROM_LE_32(animPtr->animFrame);
		_resMan->resClose(compact->o_anim_resource);
	}
	return 0;
}

int SwordLogic::fullAnimDriver(BsObject *compact) {
	if (compact->o_sync) { // return to script immediately if we've received a sync
		compact->o_logic = LOGIC_script;
		return 1;
	}
	uint8 *data = ((uint8*)_resMan->openFetchRes(compact->o_anim_resource)) + sizeof(Header);
	uint16 numFrames = READ_LE_UINT32(data);
	data += 4;
	AnimUnit *animPtr = (AnimUnit*)(data + compact->o_anim_pc * sizeof(AnimUnit));

	compact->o_anim_x = compact->o_xcoord = FROM_LE_32(animPtr->animX);
	compact->o_anim_y = compact->o_ycoord = FROM_LE_32(animPtr->animY);
	compact->o_frame = FROM_LE_32(animPtr->animFrame);

	compact->o_anim_pc++;
	if (compact->o_anim_pc == numFrames)
		compact->o_logic = LOGIC_script;

	_resMan->resClose(compact->o_anim_resource);
	return 0;
}

int SwordLogic::animDriver(BsObject *compact) {
	if (compact->o_sync) {
		compact->o_logic = LOGIC_script;
		return 1;
	}
	uint8 *data = ((uint8*)_resMan->openFetchRes(compact->o_anim_resource)) + sizeof(Header);
	uint16 numFrames = READ_LE_UINT32(data);
	AnimUnit *animPtr = (AnimUnit*)(data + 4 + compact->o_anim_pc * sizeof(AnimUnit));

	if (!(compact->o_status & STAT_SHRINK)) {
		compact->o_anim_x = animPtr->animX;
		compact->o_anim_y = animPtr->animY;
	}

	compact->o_frame = animPtr->animFrame;
	compact->o_anim_pc++;
	if (compact->o_anim_pc == numFrames)
		compact->o_logic = LOGIC_script;

	_resMan->resClose(compact->o_anim_resource);
	return 0;
}

void SwordLogic::updateScreenParams(void) {
	BsObject *compact = (BsObject*)_objMan->fetchObject(PLAYER);
	_screen->setScrolling((int16)(compact->o_xcoord - _scriptVars[FEET_X]), 
						  (int16)(compact->o_ycoord - _scriptVars[FEET_Y]));
}

int SwordLogic::scriptManager(BsObject *compact, uint32 id) {
	int ret;
	do {
		uint32 level = compact->o_tree.o_script_level;
		uint32 script = compact->o_tree.o_script_id[level];
		SwordDebug::interpretScript(id, level, script, compact->o_tree.o_script_pc[level] & ITM_ID);
		ret = interpretScript(compact, id, _resMan->lockScript(script), script, compact->o_tree.o_script_pc[level] & ITM_ID);
		_resMan->unlockScript(script);
		if (!ret) {
			if (compact->o_tree.o_script_level)
				compact->o_tree.o_script_level--;
			else
				error("ScriptManager: basescript %d for cpt %d ended!", script, id);
		} else
			compact->o_tree.o_script_pc[level] = ret;
	} while (!ret);
    return 1;
	//Logic continues - but the script must have changed logic mode
	//this is a radical change from S2.0 where once a script finished there
	//was no more processing for that object on that cycle - the Logic_engine terminated.
	//This meant that new logics that needed immediate action got a first call from the
	//setup function. This was a bit tweeky. This technique ensures that the script is a
	//totally seamless concept that takes up zero cycle time. The only downside is that
	//an FN_quit becomes slightly more convoluted, but so what you might ask.
}

void SwordLogic::runMouseScript(BsObject *cpt, int32 scriptId) {
	Header *script = _resMan->lockScript(scriptId);
	debug(9, "running mouse script %d", scriptId);
	interpretScript(cpt, _scriptVars[SPECIAL_ITEM], script, scriptId, scriptId);
	_resMan->unlockScript(scriptId);
}

int SwordLogic::interpretScript(BsObject *compact, int id, Header *scriptModule, int scriptBase, int scriptNum) {
	int32 *scriptCode = (int32*)(((uint8*)scriptModule) + sizeof(Header));
	int32 stack[MAX_STACK_SIZE];
	int32 stackIdx = 0;
	int32 offset;
	int32 pc;
	if (memcmp(scriptModule->type, "Script", 6))
		error("Invalid script module!");
	if (scriptModule->version != SCRIPT_VERSION)
		error("Illegal script version!");
	if (scriptNum < 0)
		error("negative script number");
	if ((uint32)scriptNum >= scriptModule->decomp_length)
		error("Script number out of bounds");

	if (scriptNum < scriptCode[0])
		pc = scriptCode[scriptNum + 1];
	else
		pc = scriptNum;
	int32 startOfScript = scriptCode[(scriptBase & ITM_ID) + 1];

	int32 a, b, c, d, e, f;
	int mCodeReturn = 0;
	int32 mCodeNumber = 0, mCodeArguments = 0;
	while (1) {
		assert((stackIdx >= 0) && (stackIdx <= MAX_STACK_SIZE));
		switch (scriptCode[pc++]) {
			case IT_MCODE:
				a = b = c = d = e = f = 0;
				mCodeNumber = scriptCode[pc++];
				mCodeArguments = scriptCode[pc++];
				switch (mCodeArguments) {
					case 6: f = stack[--stackIdx];
					case 5: e = stack[--stackIdx];
					case 4: d = stack[--stackIdx];
					case 3: c = stack[--stackIdx];
					case 2: b = stack[--stackIdx];
					case 1: a = stack[--stackIdx];
					case 0:
						SwordDebug::callMCode(mCodeNumber, mCodeArguments, a, b, c, d, e, f);
						mCodeReturn = (this->*_mcodeTable[mCodeNumber])(compact, id, a, b, c, d, e, f);
						break;
					default:
						warning("mcode[%d]: too many arguments(%d)", mCodeNumber, mCodeArguments);
				}
				if (mCodeReturn == 0) 
					return pc;
				break;
			case IT_PUSHNUMBER:
				debug(9, "IT_PUSH: %d", scriptCode[pc]);
				stack[stackIdx++] = scriptCode[pc++];
				break;
			case IT_PUSHVARIABLE:
				debug(9, "IT_PUSHVARIABLE: ScriptVar[%d] => %d", scriptCode[pc], _scriptVars[scriptCode[pc]]);
				stack[stackIdx++] = _scriptVars[scriptCode[pc++]];
				break;
			case IT_NOTEQUAL:
				stackIdx--;
				debug(9, "IT_NOTEQUAL: RESULT = %d", stack[stackIdx - 1] != stack[stackIdx]);
				stack[stackIdx - 1] = (stack[stackIdx - 1] != stack[stackIdx]);
				break;
			case IT_ISEQUAL:
				stackIdx--;
				debug(9, "IT_ISEQUAL: RESULT = %d", stack[stackIdx - 1] == stack[stackIdx]);
				stack[stackIdx - 1] = (stack[stackIdx - 1] == stack[stackIdx]);
				break;
			case IT_PLUS:
				stackIdx--;
				debug(9, "IT_PLUS: RESULT = %d", stack[stackIdx - 1] + stack[stackIdx]);
				stack[stackIdx - 1] = (stack[stackIdx - 1] + stack[stackIdx]);
				break;
			case IT_TIMES:
				stackIdx--;
				debug(9, "IT_TIMES: RESULT = %d", stack[stackIdx - 1] * stack[stackIdx]);
				stack[stackIdx - 1] = (stack[stackIdx - 1] * stack[stackIdx]);
				break;
			case IT_ANDAND:
				stackIdx--;
				debug(9, "IT_ANDAND: RESULT = %d", stack[stackIdx - 1] && stack[stackIdx]);
				stack[stackIdx - 1] = (stack[stackIdx - 1] && stack[stackIdx]);
				break;
			case IT_OROR:           // ||
				stackIdx--;
				debug(9, "IT_OROR: RESULT = %d", stack[stackIdx - 1] || stack[stackIdx]);
				stack[stackIdx - 1] = (stack[stackIdx - 1] || stack[stackIdx]);
				break;
			case IT_LESSTHAN:
				stackIdx--;
				debug(9, "IT_LESSTHAN: RESULT = %d", stack[stackIdx - 1] < stack[stackIdx]);
				stack[stackIdx - 1] = (stack[stackIdx - 1] < stack[stackIdx]);
				break;
			case IT_NOT:
				debug(9, "IT_NOT: RESULT = %d", stack[stackIdx - 1] ? 0 : 1);
				if (stack[stackIdx - 1])
					stack[stackIdx - 1] = 0;
				else
					stack[stackIdx - 1] = 1;
				break;
			case IT_MINUS:
				stackIdx--;
				debug(9, "IT_MINUS: RESULT = %d", stack[stackIdx - 1] - stack[stackIdx]);
				stack[stackIdx - 1] = (stack[stackIdx - 1] - stack[stackIdx]);
				break;
			case IT_AND:
				stackIdx--;
				debug(9, "IT_AND: RESULT = %d", stack[stackIdx - 1] & stack[stackIdx]);
				stack[stackIdx - 1] = (stack[stackIdx - 1] & stack[stackIdx]);
				break;
			case IT_OR:
				stackIdx--;
				debug(9, "IT_OR: RESULT = %d", stack[stackIdx - 1] | stack[stackIdx]);
				stack[stackIdx - 1] = (stack[stackIdx - 1] | stack[stackIdx]);
				break;
			case IT_GTE:
				stackIdx--;
				debug(9, "IT_GTE: RESULT = %d", stack[stackIdx - 1] >= stack[stackIdx]);
				stack[stackIdx - 1] = (stack[stackIdx - 1] >= stack[stackIdx]);
				break;
			case IT_LTE:
				stackIdx--;
				debug(9, "IT_LTE: RESULT = %d", stack[stackIdx - 1] <= stack[stackIdx]);
				stack[stackIdx - 1] = (stack[stackIdx - 1] <= stack[stackIdx]);
				break;
			case IT_DEVIDE:
				stackIdx--;
				debug(9, "IT_DEVIDE: RESULT = %d", stack[stackIdx - 1] / stack[stackIdx]);
				stack[stackIdx - 1] = (stack[stackIdx - 1] / stack[stackIdx]);
				break;
			case IT_GT:
				stackIdx--;
				debug(9, "IT_GT: RESULT = %d", stack[stackIdx - 1] > stack[stackIdx]);
				stack[stackIdx - 1] = (stack[stackIdx - 1] > stack[stackIdx]);
				break;
			case IT_SCRIPTEND:
				debug(9, "IT_SCRIPTEND");
				return 0;
			case IT_POPVAR:         // pop a variable
                debug(9, "IT_POPVAR: ScriptVars[%d] = %d", scriptCode[pc], stack[stackIdx-1]);
				_scriptVars[scriptCode[pc++]] = stack[--stackIdx];
				break;
			case IT_POPLONGOFFSET:
				offset = scriptCode[pc++];
				debug(9, "IT_POPLONGOFFSET: Cpt[%d] = %d", offset, stack[stackIdx - 1]);
				*((int32 *)((uint8*)compact + offset)) = stack[--stackIdx];
				break;
			case IT_PUSHLONGOFFSET:
				offset = scriptCode[pc++];
				debug(9, "IT_PUSHLONGOFFSET: PUSH Cpt[%d] (==%d)", offset, *((int32 *)((uint8*)compact + offset)));
				stack[stackIdx++] = *((int32 *)((uint8*)compact + offset));
				break;
			case IT_SKIPONFALSE:
				debug(9, "IT_SKIPONFALSE: %d (%s)", scriptCode[pc], (stack[stackIdx-1] ? "IS TRUE (NOT SKIPPED)" : "IS FALSE (SKIPPED)"));
				if (stack[--stackIdx])
					pc++;
				else
					pc += scriptCode[pc];
				break;
			case IT_SKIP:
				debug(9, "IT_SKIP: %d", scriptCode[pc]);
				pc += scriptCode[pc];
				break;
			case IT_SWITCH:         // The mega switch statement
				debug(9, "IT_SWITCH: [SORRY, NO DEBUG INFO]");
				{
					int switchValue = stack[--stackIdx];
					int switchCount = scriptCode[pc++];
					int doneSwitch=0;
					
					for (int cnt = 0; (cnt < switchCount) && (doneSwitch==0); cnt++) {
						if (switchValue == scriptCode[pc]) {
							pc += scriptCode[pc+1];
							doneSwitch=1;
						} else
							pc += 2;
						
					}
					if (doneSwitch == 0)
						pc += scriptCode[pc];
				}
				break;
			case IT_SKIPONTRUE:     // skip if expression true
				debug(9, "IT_SKIPONFALSE: %d (%s)", scriptCode[pc], (stack[stackIdx-1] ? "IS TRUE (SKIPPED)" : "IS FALSE (NOT SKIPPED)"));
				stackIdx--;
				if (stack[stackIdx])
					pc += scriptCode[pc];
				else
					pc++;
				break;
			case IT_PRINTF:
				debug(0, "IT_PRINTF(%d)",stack[stackIdx]);
				break;
			case IT_RESTARTSCRIPT:
				debug(9, "IT_RESTARTSCRIPT");
				pc = startOfScript;
				break;
			case IT_POPWORDOFFSET:
				offset = scriptCode[pc++];
				debug(9, "IT_POPWORDOFFSET: Cpt[%d] = %d", offset, stack[stackIdx - 1] & 0xFFFF);
				*((int32 *)((uint8*)compact + offset)) = stack[--stackIdx] & 0xffff;
				break;
			case IT_PUSHWORDOFFSET:
				offset = scriptCode[pc++];
				debug(9, "IT_PUSHWORDOFFSET: PUSH Cpt[%d] == %d", offset, (*((int32 *)((uint8*)compact + offset))) & 0xffff);
				stack[stackIdx++] = (*((int32 *)((uint8*)compact + offset))) & 0xffff;
				break;
			default:
				error("Invalid operator %d",scriptCode[pc-1]);
				return 0;
		}
	}
}

BSMcodeTable SwordLogic::_mcodeTable[100] = {
	&SwordLogic::fnBackground,
	&SwordLogic::fnForeground,
	&SwordLogic::fnSort,
	&SwordLogic::fnNoSprite,
	&SwordLogic::fnMegaSet,
	&SwordLogic::fnAnim,
	&SwordLogic::fnSetFrame,
	&SwordLogic::fnFullAnim,
	&SwordLogic::fnFullSetFrame,
	&SwordLogic::fnFadeDown,
	&SwordLogic::fnFadeUp,
	&SwordLogic::fnCheckFade,
	&SwordLogic::fnSetSpritePalette,
	&SwordLogic::fnSetWholePalette,
	&SwordLogic::fnSetFadeTargetPalette,
	&SwordLogic::fnSetPaletteToFade,
	&SwordLogic::fnSetPaletteToCut,
	&SwordLogic::fnPlaySequence,
	&SwordLogic::fnIdle,
	&SwordLogic::fnPause,
	&SwordLogic::fnPauseSeconds,
	&SwordLogic::fnQuit,
	&SwordLogic::fnKillId,
	&SwordLogic::fnSuicide,
	&SwordLogic::fnNewScript,
	&SwordLogic::fnSubScript,
	&SwordLogic::fnRestartScript,
	&SwordLogic::fnSetBookmark,
	&SwordLogic::fnGotoBookmark,
	&SwordLogic::fnSendSync,
	&SwordLogic::fnWaitSync,
	&SwordLogic::cfnClickInteract,
	&SwordLogic::cfnSetScript,
	&SwordLogic::cfnPresetScript,
	&SwordLogic::fnInteract,
	&SwordLogic::fnIssueEvent,
	&SwordLogic::fnCheckForEvent,
	&SwordLogic::fnWipeHands,
	&SwordLogic::fnISpeak,
	&SwordLogic::fnTheyDo,
	&SwordLogic::fnTheyDoWeWait,
	&SwordLogic::fnWeWait,
	&SwordLogic::fnChangeSpeechText,
	&SwordLogic::fnTalkError,
	&SwordLogic::fnStartTalk,
	&SwordLogic::fnCheckForTextLine,
	&SwordLogic::fnAddTalkWaitStatusBit,
	&SwordLogic::fnRemoveTalkWaitStatusBit,
	&SwordLogic::fnNoHuman,
	&SwordLogic::fnAddHuman,
	&SwordLogic::fnBlankMouse,
	&SwordLogic::fnNormalMouse,
	&SwordLogic::fnLockMouse,
	&SwordLogic::fnUnlockMouse,
	&SwordLogic::fnSetMousePointer,
	&SwordLogic::fnSetMouseLuggage,
	&SwordLogic::fnMouseOn,
	&SwordLogic::fnMouseOff,
	&SwordLogic::fnChooser,
	&SwordLogic::fnEndChooser,
	&SwordLogic::fnStartMenu,
	&SwordLogic::fnEndMenu,
	&SwordLogic::cfnReleaseMenu,
	&SwordLogic::fnAddSubject,
	&SwordLogic::fnAddObject,
	&SwordLogic::fnRemoveObject,
	&SwordLogic::fnEnterSection,
	&SwordLogic::fnLeaveSection,
	&SwordLogic::fnChangeFloor,
	&SwordLogic::fnWalk,
	&SwordLogic::fnTurn,
	&SwordLogic::fnStand,
	&SwordLogic::fnStandAt,
	&SwordLogic::fnFace,
	&SwordLogic::fnFaceXy,
	&SwordLogic::fnIsFacing,
	&SwordLogic::fnGetTo,
	&SwordLogic::fnGetToError,
	&SwordLogic::fnGetPos,
	&SwordLogic::fnGetGamepadXy,
	&SwordLogic::fnPlayFx,
	&SwordLogic::fnStopFx,
	&SwordLogic::fnPlayMusic,
	&SwordLogic::fnStopMusic,
	&SwordLogic::fnInnerSpace,
	&SwordLogic::fnRandom,
	&SwordLogic::fnSetScreen,
	&SwordLogic::fnPreload,
	&SwordLogic::fnCheckCD,
	&SwordLogic::fnRestartGame,
	&SwordLogic::fnQuitGame,
	&SwordLogic::fnDeathScreen,
	&SwordLogic::fnSetParallax,
	&SwordLogic::fnTdebug,
	&SwordLogic::fnRedFlash,
	&SwordLogic::fnBlueFlash,
	&SwordLogic::fnYellow,
	&SwordLogic::fnGreen,
	&SwordLogic::fnPurple,
	&SwordLogic::fnBlack
};

int SwordLogic::fnBackground(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	
	cpt->o_status &= ~(STAT_FORE | STAT_SORT);
	cpt->o_status |= STAT_BACK;
	return SCRIPT_CONT;
}

int SwordLogic::fnForeground(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status &= ~(STAT_BACK | STAT_SORT);
	cpt->o_status |= STAT_FORE;
	return SCRIPT_CONT;
}

int SwordLogic::fnSort(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status &= ~(STAT_BACK | STAT_FORE);
	cpt->o_status |= STAT_SORT;
	return SCRIPT_CONT;
}

int SwordLogic::fnNoSprite(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status &= ~(STAT_BACK | STAT_FORE | STAT_SORT);
	return SCRIPT_CONT;
}

int SwordLogic::fnMegaSet(BsObject *cpt, int32 id, int32 walk_data, int32 spr, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_mega_resource = walk_data;
	cpt->o_walk_resource = spr;
	return SCRIPT_CONT;
}

int SwordLogic::fnAnim(BsObject *cpt, int32 id, int32 cdt, int32 spr, int32 e, int32 f, int32 z, int32 x) {
	AnimSet *animTab;

	if (cdt && (!spr)) {
		animTab = (AnimSet*)((uint8*)_resMan->openFetchRes(cdt) + sizeof(Header));
		animTab += cpt->o_dir;

		cpt->o_anim_resource = FROM_LE_32(animTab->cdt);
		cpt->o_resource = FROM_LE_32(animTab->spr);
		_resMan->resClose(cdt);
	} else {
		cpt->o_anim_resource = cdt;
		cpt->o_resource = spr;
	}
	if ((cpt->o_anim_resource == 0) || (cpt->o_resource == 0))
		error("fnAnim called width (%d/%d) => (%d/%d)", cdt, spr, cpt->o_anim_resource, cpt->o_resource);
	
	FrameHeader *frameHead = _resMan->fetchFrame(_resMan->openFetchRes(cpt->o_resource), 0);
	if (frameHead->offsetX || frameHead->offsetY) { // boxed mega anim?
		cpt->o_status |= STAT_SHRINK;
		cpt->o_anim_x = cpt->o_xcoord; // set anim coords to 'feet' coords - only need to do this once
		cpt->o_anim_y = cpt->o_ycoord;
	} else {
		// Anim_driver sets anim coords to cdt coords for every frame of a loose anim
		cpt->o_status &= ~STAT_SHRINK;
	}
	_resMan->resClose(cpt->o_resource);

	cpt->o_logic = LOGIC_anim;
	cpt->o_anim_pc = 0;
	cpt->o_sync = 0;
    return SCRIPT_STOP;
}

int SwordLogic::fnSetFrame(BsObject *cpt, int32 id, int32 cdt, int32 spr, int32 frameNo, int32 f, int32 z, int32 x) {
	
	AnimUnit   *animPtr;

	uint8 *data = (uint8*)_resMan->openFetchRes(cdt);
	data += sizeof(Header);
	if (frameNo == LAST_FRAME)
		frameNo = READ_LE_UINT32(data) - 1;
	
	data += 4;
	animPtr = (AnimUnit*)(data + frameNo * sizeof(AnimUnit));

	cpt->o_anim_x = FROM_LE_32(animPtr->animX);
	cpt->o_anim_y = FROM_LE_32(animPtr->animY);
	cpt->o_frame = FROM_LE_32(animPtr->animFrame);

	cpt->o_resource = spr;
	cpt->o_status &= ~STAT_SHRINK;
	_resMan->resClose(cdt);
	return SCRIPT_CONT;
}

int SwordLogic::fnFullAnim(BsObject *cpt, int32 id, int32 anim, int32 graphic, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_logic = LOGIC_full_anim;

	cpt->o_anim_pc = 0;
	cpt->o_anim_resource = anim;
	cpt->o_resource = graphic;
	cpt->o_status &= ~STAT_SHRINK;
	cpt->o_sync = 0;
	return SCRIPT_STOP;
}

int SwordLogic::fnFullSetFrame(BsObject *cpt, int32 id, int32 cdt, int32 spr, int32 frameNo, int32 f, int32 z, int32 x) {
	uint8 *data = (uint8*)_resMan->openFetchRes(cdt) + sizeof(Header);

	if (frameNo == LAST_FRAME)
		frameNo = READ_LE_UINT32(data) - 1;
	data += 4;

	AnimUnit *animPtr = (AnimUnit*)(data + sizeof(AnimUnit) * frameNo);
	cpt->o_anim_x = cpt->o_xcoord = FROM_LE_32(animPtr->animX);
	cpt->o_anim_y = cpt->o_ycoord = FROM_LE_32(animPtr->animY);
	cpt->o_frame = FROM_LE_32(animPtr->animFrame);

	cpt->o_resource = spr;
	cpt->o_status &= ~STAT_SHRINK;

	_resMan->resClose(cdt);
	return SCRIPT_CONT;
}

int SwordLogic::fnFadeDown(BsObject *cpt, int32 id, int32 speed, int32 d, int32 e, int32 f, int32 z, int32 x) {
	warning("fnFadeDown speed = %d", speed);
	_screen->fadeDownPalette();
	return SCRIPT_CONT;
}

int SwordLogic::fnFadeUp(BsObject *cpt, int32 id, int32 speed, int32 d, int32 e, int32 f, int32 z, int32 x) {
	warning("fnFadeUp speed = %d", speed);
	_screen->fadeUpPalette();
	return SCRIPT_CONT;
}

int SwordLogic::fnCheckFade(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {

	_scriptVars[RETURN_VALUE] = (uint8)_screen->stillFading();
	return SCRIPT_CONT;
}

int SwordLogic::fnSetSpritePalette(BsObject *cpt, int32 id, int32 spritePal, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_screen->fnSetPalette(184, 72, (uint8*)_resMan->openFetchRes(spritePal));
	_resMan->resClose(spritePal);
	return SCRIPT_CONT;
}

int SwordLogic::fnSetWholePalette(BsObject *cpt, int32 id, int32 spritePal, int32 d, int32 e, int32 f, int32 z, int32 x) {
	uint8 *pal = (uint8*)_resMan->openFetchRes(spritePal);
	pal[0] = pal[1] = pal[2] = 0;
	_screen->fnSetPalette(0, 256, pal);
	_resMan->resClose(spritePal);
	return SCRIPT_CONT;
}

int SwordLogic::fnSetFadeTargetPalette(BsObject *cpt, int32 id, int32 spritePal, int32 d, int32 e, int32 f, int32 z, int32 x) {
	uint8 *pal = (uint8*)_resMan->openFetchRes(spritePal);
	pal[0] = pal[1] = pal[2] = 0;
	_resMan->resClose(spritePal);
	return SCRIPT_CONT;
}

int SwordLogic::fnSetPaletteToFade(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	warning("called unknown routine: fnSetPaletteToFade()");
	return SCRIPT_CONT;
}

int SwordLogic::fnSetPaletteToCut(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	warning("Called unknown routine: fnSetPaletteToCut()");
	return SCRIPT_CONT;
}

int SwordLogic::fnPlaySequence(BsObject *cpt, int32 id, int32 sequenceId, int32 d, int32 e, int32 f, int32 z, int32 x) {
	warning("fnPlaySequence(%d) called", sequenceId);
	return SCRIPT_CONT;
}

int SwordLogic::fnIdle(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_tree.o_script_level = 0; // force to level 0
	cpt->o_logic = LOGIC_idle;
	return SCRIPT_STOP;
}

int SwordLogic::fnPause(BsObject *cpt, int32 id, int32 pause, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_pause = pause;
	cpt->o_logic = LOGIC_pause;
	return SCRIPT_STOP;
}

int SwordLogic::fnPauseSeconds(BsObject *cpt, int32 id, int32 pause, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_pause = pause * FRAME_RATE;
	cpt->o_logic = LOGIC_pause;
	return SCRIPT_STOP;
}

int SwordLogic::fnQuit(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_logic = LOGIC_quit;
	return SCRIPT_STOP;
}

int SwordLogic::fnKillId(BsObject *cpt, int32 id, int32 target, int32 d, int32 e, int32 f, int32 z, int32 x) {
	BsObject *targetObj = _objMan->fetchObject(target);
	targetObj->o_status = 0;
	return SCRIPT_CONT;
}

int SwordLogic::fnSuicide(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status = 0;
	cpt->o_logic = LOGIC_quit;
	return SCRIPT_STOP;
}

int SwordLogic::fnNewScript(BsObject *cpt, int32 id, int32 script, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_logic = LOGIC_new_script;
	_newScript = script;
	return SCRIPT_STOP;
}

int SwordLogic::fnSubScript(BsObject *cpt, int32 id, int32 script, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_tree.o_script_level++;
	if (cpt->o_tree.o_script_level == TOTAL_script_levels)
		error("Compact %d: script level exceeded in fnSubScript.", id);
	cpt->o_tree.o_script_pc[cpt->o_tree.o_script_level] = script;
	cpt->o_tree.o_script_id[cpt->o_tree.o_script_level] = script;
	return SCRIPT_STOP;
}

int SwordLogic::fnRestartScript(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_logic = LOGIC_restart;
	return SCRIPT_STOP;
}

int SwordLogic::fnSetBookmark(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	memcpy(&cpt->o_bookmark.o_script_level, &cpt->o_tree.o_script_level, sizeof(ScriptTree));
	return SCRIPT_CONT;
}

int SwordLogic::fnGotoBookmark(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_logic = LOGIC_bookmark;
	return SCRIPT_STOP;
}

int SwordLogic::fnSendSync(BsObject *cpt, int32 id, int32 sendId, int32 syncValue, int32 e, int32 f, int32 z, int32 x) {
	BsObject *target = _objMan->fetchObject(sendId);
	target->o_sync = syncValue;
	return SCRIPT_CONT;
}

int SwordLogic::fnWaitSync(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_logic = LOGIC_wait_for_sync;
	return SCRIPT_STOP;
}

int SwordLogic::cfnClickInteract(BsObject *cpt, int32 id, int32 target, int32 d, int32 e, int32 f, int32 z, int32 x) {
	BsObject *tar = _objMan->fetchObject(target);
	cpt = _objMan->fetchObject(PLAYER);
	cpt->o_tree.o_script_level = 0;
	cpt->o_tree.o_script_pc[0] = tar->o_interact;
	cpt->o_tree.o_script_id[0] = tar->o_interact;
	cpt->o_logic = LOGIC_script;
	return SCRIPT_STOP;
}

int SwordLogic::cfnSetScript(BsObject *cpt, int32 id, int32 target, int32 script, int32 e, int32 f, int32 z, int32 x) {
	BsObject *tar = _objMan->fetchObject(target);
	tar->o_tree.o_script_level = 0;
	tar->o_tree.o_script_pc[0] = script;
	tar->o_tree.o_script_id[0] = script;
	tar->o_logic = LOGIC_script;
	return SCRIPT_CONT;
}

int SwordLogic::cfnPresetScript(BsObject *cpt, int32 id, int32 target, int32 script, int32 e, int32 f, int32 z, int32 x) {
	BsObject *tar = _objMan->fetchObject(target);
	tar->o_tree.o_script_level = 0;
	tar->o_tree.o_script_pc[0] = script;
	tar->o_tree.o_script_id[0] = script;
	if (tar->o_logic == LOGIC_idle)
		tar->o_logic = LOGIC_script;
	return SCRIPT_CONT;
}

int SwordLogic::fnInteract(BsObject *cpt, int32 id, int32 target, int32 d, int32 e, int32 f, int32 z, int32 x) {
	BsObject *tar = _objMan->fetchObject(target);
	cpt->o_place = tar->o_place;

	BsObject *floorObject = _objMan->fetchObject(tar->o_place);
	cpt->o_scale_a = floorObject->o_scale_a;
	cpt->o_scale_b = floorObject->o_scale_b;

	cpt->o_tree.o_script_level++;
	cpt->o_tree.o_script_pc[cpt->o_tree.o_script_level] = tar->o_interact;
	cpt->o_tree.o_script_id[cpt->o_tree.o_script_level] = tar->o_interact;

	return SCRIPT_STOP;
}

int SwordLogic::fnIssueEvent(BsObject *cpt, int32 id, int32 event, int32 delay, int32 e, int32 f, int32 z, int32 x) {
	_eventMan->fnIssueEvent(cpt, id, event, delay);
	return SCRIPT_CONT;
}

int SwordLogic::fnCheckForEvent(BsObject *cpt, int32 id, int32 pause, int32 d, int32 e, int32 f, int32 z, int32 x) {
	return _eventMan->fnCheckForEvent(cpt, id, pause);
}

int SwordLogic::fnWipeHands(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_scriptVars[OBJECT_HELD] = 0;
	_mouse->setLuggage(0, 0);
	_menu->refresh(MENU_TOP);
	return SCRIPT_CONT;
}

int SwordLogic::fnISpeak(BsObject *cpt, int32 id, int32 cdt, int32 textNo, int32 spr, int32 f, int32 z, int32 x) {
	_speechClickDelay = 3;
	_mouse->flushEvents(); // prevent player from accidently clicking text away within first three frames
	cpt->o_logic = LOGIC_speech;

	// first setup the talk animation
	if (cdt && (!spr)) { // if 'cdt' is non-zero but 'spr' is zero - 'cdt' is an anim table tag
		AnimSet *animTab = (AnimSet*)((uint8*)_resMan->openFetchRes(cdt) + sizeof(Header));
		animTab += cpt->o_dir;
		
		cpt->o_anim_resource = FROM_LE_32(animTab->cdt);
		if (animTab->cdt)
			cpt->o_resource = FROM_LE_32(animTab->spr);
		_resMan->resClose(cdt);
	} else {
		cpt->o_anim_resource = cdt;
		if (cdt)
			cpt->o_resource = spr;
	}
	cpt->o_anim_pc = 0; // start anim from first frame
	if (cpt->o_anim_resource) {
		if (!cpt->o_resource)
			error("ID %d: Can't run anim with cdt=%d, spr=%d", id, cdt, spr);
		
		FrameHeader *frameHead = _resMan->fetchFrame(_resMan->openFetchRes(cpt->o_resource), 0);
		if (frameHead->offsetX && frameHead->offsetY) { // is this a boxed mega?
			cpt->o_status |= STAT_SHRINK;
			cpt->o_anim_x = cpt->o_xcoord;
			cpt->o_anim_y = cpt->o_ycoord;
		} else
			cpt->o_status &= ~STAT_SHRINK;

		_resMan->resClose(cpt->o_resource);
	}
	if (SwordEngine::_systemVars.playSpeech)
		_speechRunning = _sound->startSpeech(textNo >> 16, textNo & 0xFFFF);
	else
		_speechRunning = false;
    _speechFinished = false;
	if (SwordEngine::_systemVars.showText || (!_speechRunning)) {
		_textRunning = true;

		char *text = _objMan->lockText(textNo);
		cpt->o_speech_time = strlen(text) + 5;
		uint32 textCptId = _textMan->lowTextManager((uint8*)text, cpt->o_speech_width, (uint8)cpt->o_speech_pen);
		_objMan->unlockText(textNo);

		BsObject * textCpt = _objMan->fetchObject(textCptId);

		// the graphic is a property of SwordText, so we don't lock/unlock it.
		uint16 textSpriteWidth  = _textMan->giveSpriteData(cpt->o_target)->width;
		uint16 textSpriteHeight = _textMan->giveSpriteData(cpt->o_target)->height;

		textCpt->o_screen = cpt->o_screen;
		cpt->o_text_id = textCptId;

		// now set text coords, above the player, usually

#define TEXT_MARGIN 3 // distance kept from edges of screen
#define ABOVE_HEAD 20 // distance kept above talking sprite
		uint16 textX, textY;
		if ((id == GEORGE) || ((id == NICO) && (_scriptVars[SCREEN] == 10)) && (!cpt->o_anim_resource)) {
			// if George is doing Voice-Over text (centered at the bottom of the screen)
			textX = _scriptVars[SCROLL_OFFSET_X] + 128 + (640 / 2) - textSpriteWidth / 2;
			textY = _scriptVars[SCROLL_OFFSET_Y] + 128 + 400;
		} else {
			if ((id == GEORGE) && (_scriptVars[SCREEN] == 79))
				textX = cpt->o_mouse_x2; // move it off george's head
			else
				textX = (cpt->o_mouse_x1 + cpt->o_mouse_x2) / 2 - textSpriteWidth / 2;

			textY = cpt->o_mouse_y1 - textSpriteHeight - ABOVE_HEAD;
		}
		// now ensure text is within visible screen
		uint16 textLeftMargin, textRightMargin, textTopMargin, textBottomMargin;
		textLeftMargin   = SCREEN_LEFT_EDGE   + TEXT_MARGIN + _scriptVars[SCROLL_OFFSET_X];
		textRightMargin  = SCREEN_RIGHT_EDGE  - TEXT_MARGIN + _scriptVars[SCROLL_OFFSET_X] - textSpriteWidth;
		textTopMargin    = SCREEN_TOP_EDGE    + TEXT_MARGIN + _scriptVars[SCROLL_OFFSET_Y];
		textBottomMargin = SCREEN_BOTTOM_EDGE - TEXT_MARGIN + _scriptVars[SCROLL_OFFSET_Y] - textSpriteHeight;

		textCpt->o_anim_x = textCpt->o_xcoord = inRange(textLeftMargin, textX, textRightMargin);
		textCpt->o_anim_y = textCpt->o_ycoord = inRange(textTopMargin,  textY, textBottomMargin);
	}
	return SCRIPT_STOP;
}

//send instructions to mega in conversation with player
//the instruction is interpreted by the script mega_interact
int SwordLogic::fnTheyDo(BsObject *cpt, int32 id, int32 tar, int32 instruc, int32 param1, int32 param2, int32 param3, int32 x) {
	BsObject *target;
	target = _objMan->fetchObject(tar);
	target->o_down_flag = instruc; // instruction for the mega
	target->o_ins1 = param1;
	target->o_ins2 = param2;
	target->o_ins3 = param3;
    return SCRIPT_CONT;
}

//send an instruction to mega we're talking to and wait
//until it has finished before returning to script
int SwordLogic::fnTheyDoWeWait(BsObject *cpt, int32 id, int32 tar, int32 instruc, int32 param1, int32 param2, int32 param3, int32 x) {
	BsObject *target;
	target = _objMan->fetchObject(tar);
	target->o_down_flag = instruc; // instruction for the mega
	target->o_ins1 = param1;
	target->o_ins2 = param2;
	target->o_ins3 = param3;
	target->o_status &= ~STAT_TALK_WAIT;

	cpt->o_logic = LOGIC_wait_for_talk;
	cpt->o_down_flag = tar;
	return SCRIPT_STOP;
}

int SwordLogic::fnWeWait(BsObject *cpt, int32 id, int32 tar, int32 d, int32 e, int32 f, int32 z, int32 x) {
	BsObject *target = _objMan->fetchObject(tar);
	target->o_status &= ~STAT_TALK_WAIT;

	cpt->o_logic = LOGIC_wait_for_talk;
	cpt->o_down_flag = tar;

	return SCRIPT_STOP;
}

int SwordLogic::fnChangeSpeechText(BsObject *cpt, int32 id, int32 tar, int32 width, int32 pen, int32 f, int32 z, int32 x) {
	BsObject *target = _objMan->fetchObject(tar);
	target->o_speech_width = width;
	target->o_speech_pen = pen;
	return SCRIPT_STOP;
}

//mega_interact has received an instruction it does not understand -
//The game is halted for debugging. Maybe we'll remove this later.
int SwordLogic::fnTalkError(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	error("fnTalkError for id %d, instruction %d", id, cpt->o_down_flag);
	return SCRIPT_STOP;
}

int SwordLogic::fnStartTalk(BsObject *cpt, int32 id, int32 target, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_down_flag = target;
	cpt->o_logic = LOGIC_start_talk;
	return SCRIPT_STOP;
}

int SwordLogic::fnCheckForTextLine(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_scriptVars[RETURN_VALUE] = _objMan->fnCheckForTextLine(id);
    return SCRIPT_CONT;
}

int SwordLogic::fnAddTalkWaitStatusBit(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status |= STAT_TALK_WAIT;
	return SCRIPT_CONT;
}

int SwordLogic::fnRemoveTalkWaitStatusBit(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status &= ~STAT_TALK_WAIT;
	return SCRIPT_CONT;
}

int SwordLogic::fnNoHuman(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_mouse->fnNoHuman();
	return SCRIPT_CONT;
}

int SwordLogic::fnAddHuman(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_mouse->fnAddHuman();
	return SCRIPT_CONT;
}

int SwordLogic::fnBlankMouse(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_mouse->fnBlankMouse();
	return SCRIPT_CONT;
}

int SwordLogic::fnNormalMouse(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_mouse->fnNormalMouse();
	return SCRIPT_CONT;
}

int SwordLogic::fnLockMouse(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_mouse->fnLockMouse();
	return SCRIPT_CONT;
}

int SwordLogic::fnUnlockMouse(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_mouse->fnUnlockMouse();
	return SCRIPT_CONT;
}

int SwordLogic::fnSetMousePointer(BsObject *cpt, int32 id, int32 tag, int32 rate, int32 e, int32 f, int32 z, int32 x) {
	_mouse->setPointer(tag, rate);
	return SCRIPT_CONT;
}

int SwordLogic::fnSetMouseLuggage(BsObject *cpt, int32 id, int32 tag, int32 rate, int32 e, int32 f, int32 z, int32 x) {
	_mouse->setLuggage(tag, rate);
	return SCRIPT_CONT;
}

int SwordLogic::fnMouseOn(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status |= STAT_MOUSE;
	return SCRIPT_CONT;
}

int SwordLogic::fnMouseOff(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status &= ~STAT_MOUSE;
	return SCRIPT_CONT;
}

int SwordLogic::fnChooser(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_menu->fnChooser(cpt);
	return SCRIPT_STOP;
}

int SwordLogic::fnEndChooser(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_menu->fnEndChooser();
	return SCRIPT_CONT;
}

int SwordLogic::fnStartMenu(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_menu->fnStartMenu();
	return SCRIPT_CONT;
}

int SwordLogic::fnEndMenu(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_menu->fnEndMenu();
	return SCRIPT_CONT;
}

int SwordLogic::cfnReleaseMenu(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_menu->cfnReleaseMenu();
	return SCRIPT_STOP;
}

int SwordLogic::fnAddSubject(BsObject *cpt, int32 id, int32 sub, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_menu->fnAddSubject(sub);
	return SCRIPT_CONT;
}

int SwordLogic::fnAddObject(BsObject *cpt, int32 id, int32 objectNo, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_scriptVars[POCKET_1 + objectNo - 1] = 1; // basically means: carrying object objectNo = true;
	return SCRIPT_CONT;
}

int SwordLogic::fnRemoveObject(BsObject *cpt, int32 id, int32 objectNo, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_scriptVars[POCKET_1 + objectNo - 1] = 0;
	return SCRIPT_CONT;
}

int SwordLogic::fnEnterSection(BsObject *cpt, int32 id, int32 screen, int32 d, int32 e, int32 f, int32 z, int32 x) {
	if (screen >= TOTAL_SECTIONS)
		error("mega %d tried entering section %d", id, screen);

	if (cpt->o_type == TYPE_PLAYER)
		_scriptVars[NEW_SCREEN] = screen;
	else
		cpt->o_screen = screen; // move the mega
	_objMan->megaEntering(screen);
	return SCRIPT_CONT;
}

int SwordLogic::fnLeaveSection(BsObject *cpt, int32 id, int32 oldScreen, int32 d, int32 e, int32 f, int32 z, int32 x) {
	if (oldScreen >= TOTAL_SECTIONS)
		error("mega %d leaving section %d", id, oldScreen);
	_objMan->megaLeaving(oldScreen, id);
	return SCRIPT_CONT;
}

int SwordLogic::fnChangeFloor(BsObject *cpt, int32 id, int32 floor, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_place = floor;
	BsObject *floorCpt = _objMan->fetchObject(floor);
	cpt->o_scale_a = floorCpt->o_scale_a;
	cpt->o_scale_b = floorCpt->o_scale_b;
    return SCRIPT_CONT;
}

int SwordLogic::fnWalk(BsObject *cpt, int32 id, int32 x, int32 y, int32 dir, int32 stance, int32 a, int32 b) {
	if (stance > 0)
		dir = 9;
	cpt->o_walk_pc = 0;
	cpt->o_route[1].frame = 512; // end of sequence
	if (id == PLAYER)
		_router->setPlayerTarget(x, y, dir, stance);

	int32 routeRes = _router->routeFinder(id, cpt, x, y, dir);

	if (id == PLAYER) {
		_router->resetExtraData();
		if ((routeRes == 1) || (routeRes == 2)) {
			_scriptVars[MEGA_ON_GRID] = 0;
			_scriptVars[REROUTE_GEORGE] = 0;
		}
	}
	if ((routeRes == 1) || (routeRes == 2)) {
		cpt->o_down_flag = 1; // 1 means okay.
		// if both mouse buttons were pressed on an exit => skip george's walk
		if ((id == GEORGE) && (_mouse->testEvent() == MOUSE_BOTH_BUTTONS)) {
			_mouse->flushEvents();
			int32 target = _scriptVars[CLICK_ID];
			// exceptions: compacts that use hand pointers but are not actually exits
			if ((target != LEFT_SCROLL_POINTER) && (target != RIGHT_SCROLL_POINTER) &&
				(target != FLOOR_63) && (target != ROOF_63) && (target != GUARD_ROOF_63) &&
				(target != LEFT_TREE_POINTER_71) && (target != RIGHT_TREE_POINTER_71)) {

				target = _objMan->fetchObject(_scriptVars[CLICK_ID])->o_mouse_on;
				if ((target >= SCR_exit0) && (target <= SCR_exit9)) {
					fnStandAt(cpt,id,x,y,dir,stance,0,0);
					return SCRIPT_STOP;
				}
			}
		}
		cpt->o_logic = LOGIC_AR_animate;
		return SCRIPT_STOP;
	} else if (routeRes == 3)
		cpt->o_down_flag = 1; // pretend it was successful
	else
		cpt->o_down_flag = 0; // 0 means error
    
	return SCRIPT_CONT;
}

int SwordLogic::fnTurn(BsObject *cpt, int32 id, int32 dir, int32 stance, int32 c, int32 d, int32 a, int32 b) {
	if (stance > 0)
		dir = 9;
	int route = _router->routeFinder(id, cpt, cpt->o_xcoord, cpt->o_ycoord, dir);
	
	if	(route)
		cpt->o_down_flag = 1;		//1 means ok
	else
		cpt->o_down_flag = 0;		//0 means error

	cpt->o_logic = LOGIC_AR_animate;
	cpt->o_walk_pc = 0;						//reset

	return SCRIPT_STOP;
}

int SwordLogic::fnStand(BsObject *cpt, int32 id, int32 dir, int32 stance, int32 c, int32 d, int32 a, int32 b) {
	if ((dir < 0) || (dir > 8)) {
		warning("fnStand:: invalid direction %d", dir);
		return SCRIPT_CONT;
	}
	if (dir == 8)
		dir = cpt->o_dir;
	cpt->o_resource = cpt->o_walk_resource;
	cpt->o_status |= STAT_SHRINK;
	cpt->o_anim_x = cpt->o_xcoord;
	cpt->o_anim_y = cpt->o_ycoord;
	cpt->o_frame = 96 + dir;
	cpt->o_dir = dir;
	return SCRIPT_STOP;
}

int SwordLogic::fnStandAt(BsObject *cpt, int32 id, int32 x, int32 y, int32 dir, int32 stance, int32 a, int32 b) {
	if ((dir < 0) || (dir > 8)) {
		warning("fnStandAt:: invalid direction %d", dir);
		return SCRIPT_CONT;
	}
	if (dir == 8)
		dir = cpt->o_dir;
	cpt->o_xcoord = x;
	cpt->o_ycoord = y;
	return fnStand(cpt, id, dir, stance, 0, 0, 0, 0);
}

int SwordLogic::fnFace(BsObject *cpt, int32 id, int32 targetId, int32 b, int32 c, int32 d, int32 a, int32 z) {
	BsObject *target = _objMan->fetchObject(targetId);
	int32 x, y;
	if ((target->o_type == TYPE_MEGA) || (target->o_type == TYPE_PLAYER)) {
		x = target->o_xcoord;
		y = target->o_ycoord;
	} else {
		x = (target->o_mouse_x1 + target->o_mouse_x2) / 2;
		y = (target->o_mouse_y1 + target->o_mouse_y2) / 2;
	}
	int32 megaTarDir = _router->whatTarget(cpt->o_xcoord, cpt->o_ycoord, x, y);
	fnTurn(cpt, id, megaTarDir, 0, 0, 0, 0, 0);
	return SCRIPT_STOP;
}

int SwordLogic::fnFaceXy(BsObject *cpt, int32 id, int32 x, int32 y, int32 c, int32 d, int32 a, int32 b) {
	int megaTarDir = _router->whatTarget(cpt->o_xcoord, cpt->o_ycoord, x, y);
	fnTurn(cpt, id, megaTarDir, 0, 0, 0, 0, 0);
	return SCRIPT_STOP;
}

int SwordLogic::fnIsFacing(BsObject *cpt, int32 id, int32 targetId, int32 b, int32 c, int32 d, int32 a, int32 z) {
	BsObject *target = _objMan->fetchObject(targetId);
	int32 x, y, dir;
	if ((target->o_type == TYPE_MEGA) || (target->o_type == TYPE_PLAYER)) {
		x = target->o_xcoord;
		y = target->o_ycoord;
		dir = target->o_dir;
	} else
		error("fnIsFacing:: Target isn't a mega!");

	int32 lookDir = _router->whatTarget(x, y, cpt->o_xcoord, cpt->o_ycoord);
	lookDir -= dir;
	lookDir = abs(lookDir);

	if (lookDir > 4)
		lookDir = 8 - lookDir;

	_scriptVars[RETURN_VALUE] = lookDir;
	return SCRIPT_STOP;
}

int SwordLogic::fnGetTo(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	BsObject *place = _objMan->fetchObject(cpt->o_place);
	
	cpt->o_tree.o_script_level++;
	cpt->o_tree.o_script_pc[cpt->o_tree.o_script_level] = place->o_get_to_script;
	cpt->o_tree.o_script_id[cpt->o_tree.o_script_level] = place->o_get_to_script;
    return SCRIPT_STOP;
}

int SwordLogic::fnGetToError(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	debug(1, "fnGetToError: compact %d at place %d no get-to for target %d, click_id %d\n", id, cpt->o_place, cpt->o_target, _scriptVars[CLICK_ID]);
	return SCRIPT_CONT;
}

int SwordLogic::fnRandom(BsObject *compact, int32 id, int32 min, int32 max, int32 e, int32 f, int32 z, int32 x) {
	_scriptVars[RETURN_VALUE] = _rnd.getRandomNumberRng(min, max);
	return SCRIPT_CONT;
}

int SwordLogic::fnGetPos(BsObject *cpt, int32 id, int32 targetId, int32 b, int32 c, int32 d, int32 z, int32 x) {
	BsObject *target = _objMan->fetchObject(targetId);
	if ((target->o_type == TYPE_MEGA) || (target->o_type == TYPE_PLAYER)) {
		_scriptVars[RETURN_VALUE]   = target->o_xcoord;
		_scriptVars[RETURN_VALUE_2] = target->o_ycoord;
	} else {
		_scriptVars[RETURN_VALUE]   = (target->o_mouse_x1 + target->o_mouse_x2) / 2;
		_scriptVars[RETURN_VALUE_2] = target->o_mouse_y2;
	}
	_scriptVars[RETURN_VALUE_3] = target->o_dir;

	int32 megaSeperation;
	if (targetId == DUANE)
		megaSeperation = 70; // George & Duane stand with feet 70 pixels apart when at full scale
	else if (targetId == BENOIR)
		megaSeperation = 61; // George & Benoir
	else
		megaSeperation = 42; // George & Nico/Goinfre stand with feet 42 pixels apart when at full scale

	if (target->o_status & STAT_SHRINK) {
		int32 scale = (target->o_scale_a * target->o_ycoord + target->o_scale_b) / 256;
		_scriptVars[RETURN_VALUE_4] = (megaSeperation * scale) / 256;
		debug(1, "fnGetPos: scaled megaSeperation = %d", _scriptVars[RETURN_VALUE_4]);
	} else
		_scriptVars[RETURN_VALUE_4] = megaSeperation;
	return SCRIPT_CONT;
}

int SwordLogic::fnGetGamepadXy(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	// playstation only
	return SCRIPT_CONT;
}

int SwordLogic::fnPlayFx(BsObject *cpt, int32 id, int32 fxNo, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_scriptVars[RETURN_VALUE] = _sound->addToQueue(fxNo);
	return SCRIPT_CONT;
}

int SwordLogic::fnStopFx(BsObject *cpt, int32 id, int32 fxNo, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_sound->fnStopFx(fxNo);
	//_sound->removeFromQueue(fxNo);
	return SCRIPT_CONT;
}

int SwordLogic::fnPlayMusic(BsObject *cpt, int32 id, int32 tuneId, int32 loopFlag, int32 c, int32 d, int32 z, int32 x) {
	if (loopFlag == LOOPED)
		SwordEngine::_systemVars.currentMusic = tuneId; // so it gets restarted when saving & reloading
	else
		SwordEngine::_systemVars.currentMusic = 0;

	_music->startMusic(tuneId, loopFlag);
	return SCRIPT_CONT;
}

int SwordLogic::fnStopMusic(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	SwordEngine::_systemVars.currentMusic = 0;
	_music->fadeDown();
	return SCRIPT_CONT;
}

int SwordLogic::fnInnerSpace(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	error("fnInnerSpace() not working.");
	return SCRIPT_STOP;
}

int SwordLogic::fnSetScreen(BsObject *cpt, int32 id, int32 target, int32 screen, int32 c, int32 d, int32 z, int32 x) {
	_objMan->fetchObject(target)->o_screen = screen;
	return SCRIPT_CONT;
}

int SwordLogic::fnPreload(BsObject *cpt, int32 id, int32 resId, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_resMan->resOpen(resId);
	_resMan->resClose(resId);
	return SCRIPT_CONT;
}

int SwordLogic::fnCheckCD(BsObject *cpt, int32 id, int32 screen, int32 b, int32 c, int32 d, int32 z, int32 x) {
	warning("fnCheckCd called");
	// Not sure if we really have to check that here. I think we can do it in the main loop
	// and leave a dummy here.
	return SCRIPT_CONT;
}

int SwordLogic::fnRestartGame(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	SwordEngine::_systemVars.saveGameFlag = 3;
	cpt->o_logic = LOGIC_quit;
	return SCRIPT_STOP;
}

int SwordLogic::fnQuitGame(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	error("fnQuitGame() called");
	return SCRIPT_STOP;
}

int SwordLogic::fnDeathScreen(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	SwordEngine::_systemVars.saveGameFlag = 1;
	SwordEngine::_systemVars.snrStatus = 1;
	if (_scriptVars[FINALE_OPTION_FLAG] == 4) // successful end of game!
		SwordEngine::_systemVars.deathScreenFlag = 2;
	else
		SwordEngine::_systemVars.deathScreenFlag = 1;

	cpt->o_logic = LOGIC_quit;
	return SCRIPT_STOP;
}

int SwordLogic::fnSetParallax(BsObject *cpt, int32 id, int32 screen, int32 resId, int32 c, int32 d, int32 z, int32 x) {
	_screen->fnSetParallax(screen, resId);
	return SCRIPT_CONT;
}

int SwordLogic::fnTdebug(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	debug(1, "Script TDebug id %d code %d, %d", id, a, b);
	return SCRIPT_CONT;
}

int SwordLogic::fnRedFlash(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_screen->fnFlash(FLASH_RED);
	return SCRIPT_CONT;
}

int SwordLogic::fnBlueFlash(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_screen->fnFlash(FLASH_BLUE);
	return SCRIPT_CONT;
}

int SwordLogic::fnYellow(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_screen->fnFlash(BORDER_YELLOW);
	return SCRIPT_CONT;
}

int SwordLogic::fnGreen(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_screen->fnFlash(BORDER_GREEN);
	return SCRIPT_CONT;
}

int SwordLogic::fnPurple(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_screen->fnFlash(BORDER_PURPLE);
	return SCRIPT_CONT;
}

int SwordLogic::fnBlack(BsObject *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_screen->fnFlash(BORDER_BLACK);
	return SCRIPT_CONT;
}

uint16 SwordLogic::inRange(uint16 a, uint16 b, uint16 c) {
	return (a > b)? a : (((b > c) ? c : b));
}

const uint32 SwordLogic::_scriptVarInit[NON_ZERO_SCRIPT_VARS][2] = {
	{  42,  448}, {  43,  378}, {  51,    1}, {  92,    1}, { 147,   71}, { 201,   1},
	{ 209,    1}, { 215,    1}, { 242,    2}, { 244,    1}, { 246,    3}, { 247,   1},
	{ 253,    1}, { 297,    1}, { 398,    1}, { 508,    1}, { 605,    1}, { 606,   1},
	{ 701,    1}, { 709,    1}, { 773,    1}, { 843,    1}, { 907,    1}, { 923,   1},
	{ 966,    1}, { 988,    2}, {1058,    1}, {1059,    2}, {1060,    3}, {1061,   4},
	{1062,    5}, {1063,    6}, {1064,    7}, {1065,    8}, {1066,    9}, {1067,  10},
	{1068,   11}, {1069,   12}, {1070,   13}, {1071,   14}, {1072,   15}, {1073,  16},
	{1074,   17}, {1075,   18}, {1076,   19}, {1077,   20}, {1078,   21}, {1079,  22},
	{1080,   23}, {1081,   24}, {1082,   25}, {1083,   26}, {1084,   27}, {1085,  28},
	{1086,   29}, {1087,   30}, {1088,   31}, {1089,   32}, {1090,   33}, {1091,  34},
	{1092,   35}, {1093,   36}, {1094,   37}, {1095,   38}, {1096,   39}, {1097,  40},
	{1098,   41}, {1099,   42}, {1100,   43}, {1101,   44}, {1102,   48}, {1103,  45},
	{1104,   47}, {1105,   49}, {1106,   50}, {1107,   52}, {1108,   54}, {1109,  56},
	{1110,   57}, {1111,   58}, {1112,   59}, {1113,   60}, {1114,   61}, {1115,  62},
	{1116,   63}, {1117,   64}, {1118,   65}, {1119,   66}, {1120,   67}, {1121,  68},
	{1122,   69}, {1123,   71}, {1124,   72}, {1125,   73}, {1126,   74}
};
