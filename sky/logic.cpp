/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "sky/logic.h"
#include "sky/debug.h"
#include "sky/compact.h"
#include "sky/skydefs.h"


typedef void (SkyLogic::*LogicTable) ();
static const LogicTable logicTable[] = {
	&SkyLogic::lreturn,
	&SkyLogic::logicScript,	 // 1  script processor
	&SkyLogic::autoRoute,	 // 2  Make a route
	&SkyLogic::arAnim,	 // 3  Follow a route
	&SkyLogic::arTurn,	 // 4  Mega turns araound
	&SkyLogic::alt,		 // 5  Set up new get-to script
	&SkyLogic::anim,	 // 6  Follow a sequence
	&SkyLogic::turn,	 // 7  Mega turning
	&SkyLogic::cursor,	 // 8  id tracks the pointer
	&SkyLogic::talk,	 // 9  count down and animate
	&SkyLogic::listen,	 // 10 player waits for talking id
	&SkyLogic::stopped,	 // 11 wait for id to move
	&SkyLogic::choose,	 // 12 wait for player to click
	&SkyLogic::frames,	 // 13 animate just frames
	&SkyLogic::pause,	 // 14 Count down to 0 and go
	&SkyLogic::waitSync,	 // 15 Set to l_script when sync!=0
	&SkyLogic::simpleAnim,	 // 16 Module anim without x,y's
};

SkyLogic::SkyLogic(SkyDisk *skyDisk, SkyGrid *skyGrid) {
	_skyDisk = skyDisk;
	_skyGrid = skyGrid;
	_skyAutoRoute = new SkyAutoRoute(_skyGrid);

	for (uint i = 0; i < sizeof(_moduleList)/sizeof(uint16*); i++)
		_moduleList[i] = 0;
	_stackPtr = 0;

	initScriptVariables();
}

void SkyLogic::engine() {
	Compact *compact2 = SkyState::fetchCompact(141); // logic list

	while (compact2->logic) { // 0 means end of list
		if (compact2->logic == 0xffff) {
			// Change logic data address
			compact2 = SkyState::fetchCompact(compact2->status);
			continue;
		}

		//cur_id = compact2->logic;
		_compact = SkyState::fetchCompact(compact2->logic);

		// check the id actually wishes to be processed
		if (!(_compact->status & (1 << 6)))
			continue;

		// ok, here we process the logic bit system

		if (_compact->status & (1 << 7))
			_skyGrid->removeObjectFromWalk(_compact);

		SkyDebug::logic(_compact->logic);
		(this->*logicTable[_compact->logic]) ();

		if (_compact->status & (1 << 7))
			_skyGrid->objectToWalk(_compact);

		// a sync sent to the compact is available for one cycle
		// only. that cycle has just ended so remove the sync.
		// presumably the mega has just reacted to it.
		_compact->sync = 0;
	}
}

void SkyLogic::lreturn() {
	error("lreturn: Is this really called?");
}

void SkyLogic::logicScript() {
	// Process the current mega's script
	// If the script finishes then drop back a level

	for (;;) {
		uint16 mode = _compact->mode; // get pointer to current script
		printf("compact mode: %d\n", mode);
		uint16 *scriptNo = (uint16 *)SkyCompact::getCompactElem(_compact, C_BASE_SUB + mode);
		uint16 *offset   = (uint16 *)SkyCompact::getCompactElem(_compact, C_BASE_SUB + mode + 2);

		// FIXME: HACK ALERT!!!!
		if (*scriptNo == 0x27 && *offset == 0)
			*offset = 0x9fa;

		uint32 scr = script(_compact, *scriptNo, *offset);
		*scriptNo = (uint16)(scr & 0xffff);
		*offset   = (uint16)(scr >> 16);

		if (!*offset) // script finished
			_compact->mode -= 4;
		else if (_compact->mode == mode)
			return;
	}
}

void SkyLogic::autoRoute() {
	uint16 *route = 0;
	uint16 ret = _skyAutoRoute->autoRoute(_compact, &route);

	_compact->logic = L_SCRIPT; // continue the script

	if (ret != 1) // route failed
		_compact->downFlag = 1; // return fail to script
	else if (!route) // zero route
		_compact->downFlag = 2; // return fail to script
	else {
		_compact->grafixProg = route; // put graphic prog in
		_compact->downFlag = 0; // route ok
	}

	logicScript();
	return;
}

void SkyLogic::arAnim() {
	error("Stub: SkyLogic::arAnim");
}

void SkyLogic::arTurn() {
	_compact->frame = *_compact->extCompact->turnProg++;

	if (!*_compact->extCompact->turnProg) { // turn done?
		// Back to ar mode
		_compact->extCompact->arAnimIndex = 0;
		_compact->logic = L_AR_ANIM;
	}
}

void SkyLogic::alt() {
	// change the current script

	_compact->logic = L_SCRIPT;
	uint16 *scriptNo = (uint16 *)SkyCompact::getCompactElem(_compact, C_BASE_SUB + _compact->mode);
	uint16 *offset   = (uint16 *)SkyCompact::getCompactElem(_compact, C_BASE_SUB + _compact->mode + 2);
	*scriptNo = _compact->extCompact->alt;
	*offset = 0;

	logicScript();
}

void SkyLogic::anim() {
	error("Stub: SkyLogic::anim");
}

void SkyLogic::turn() {
	if (*_compact->extCompact->turnProg) {
		_compact->frame = *_compact->extCompact->turnProg++;
		return;
	}

	// turn_to_script:

	_compact->extCompact->arAnimIndex = 0;
	_compact->logic = L_SCRIPT;

	logicScript();
}

void SkyLogic::cursor() {
	error("Stub: SkyLogic::cursor");
}

void SkyLogic::talk() {
	error("Stub: SkyLogic::talk");
}

void SkyLogic::listen() {
	error("Stub: SkyLogic::listen");
}

void SkyLogic::stopped() {
	error("Stub: SkyLogic::stopped");
}

void SkyLogic::choose() {
	error("Stub: SkyLogic::choose");
}

void SkyLogic::frames() {
	error("Stub: SkyLogic::frames");
}

void SkyLogic::pause() {
	error("Stub: SkyLogic::pause");
}

void SkyLogic::waitSync() {
	error("Stub: SkyLogic::waitSync");
}

void SkyLogic::simpleAnim() {
	// follow an animation sequence module
	// whilst ignoring the coordinate data

	uint16 *grafixProg = _compact->grafixProg;

	// *grafix_prog: command
	while (*grafixProg) {
		if (*grafixProg != SEND_SYNC) {
			grafixProg++;
			grafixProg++; // skip coordinates

			// *grafix_prog: frame
			if (*grafixProg >= 64)
				_compact->frame = *grafixProg;
			else
				_compact->frame = *grafixProg + _compact->offset;

			grafixProg++;
			_compact->grafixProg = grafixProg;
			return;
		}

		grafixProg++;
		// *grafix_prog: id to sync
		Compact *compact2 = SkyState::fetchCompact(*grafixProg);
		grafixProg++;

		// *grafix_prog: sync
		compact2->sync = *grafixProg;
		grafixProg++;
	}

	_compact->downFlag = 0; // return 'ok' to script
	_compact->logic = L_SCRIPT;
	logicScript();
}

void SkyLogic::checkModuleLoaded(uint16 moduleNo) {
	if (!_moduleList[moduleNo])
		_moduleList[moduleNo] = (uint16 *)_skyDisk->loadFile((uint16)moduleNo + F_MODULE_0, NULL);
}

void SkyLogic::push(uint32 a) {
	assert(_stackPtr < 19);
	_stack[_stackPtr] = a;
	_stackPtr++;
}

uint32 SkyLogic::pop() {
	assert(_stackPtr > 0);
	--_stackPtr;
	return _stack[_stackPtr];
}

typedef uint32 (SkyLogic::*McodeTable) (uint32, uint32, uint32);
static  McodeTable mcodeTable[] = {
	&SkyLogic::fnCacheChip,
	&SkyLogic::fnCacheFast,
	&SkyLogic::fnDrawScreen,
	&SkyLogic::fnAr,
	&SkyLogic::fnArAnimate,
	&SkyLogic::fnIdle,
	&SkyLogic::fnInteract,
	&SkyLogic::fnStartSub,
	&SkyLogic::fnTheyStartSub,
	&SkyLogic::fnAssignBase,
	&SkyLogic::fnDiskMouse,
	&SkyLogic::fnNormalMouse,
	&SkyLogic::fnBlankMouse,
	&SkyLogic::fnCrossMouse,
	&SkyLogic::fnCursorRight,
	&SkyLogic::fnCursorLeft,
	&SkyLogic::fnCursorDown,
	&SkyLogic::fnOpenHand,
	&SkyLogic::fnCloseHand,
	&SkyLogic::fnGetTo,
	&SkyLogic::fnSetToStand,
	&SkyLogic::fnTurnTo,
	&SkyLogic::fnArrived,
	&SkyLogic::fnLeaving,
	&SkyLogic::fnSetAlternate,
	&SkyLogic::fnAltSetAlternate,
	&SkyLogic::fnKillId,
	&SkyLogic::fnNoHuman,
	&SkyLogic::fnAddHuman,
	&SkyLogic::fnAddButtons,
	&SkyLogic::fnNoButtons,
	&SkyLogic::fnSetStop,
	&SkyLogic::fnClearStop,
	&SkyLogic::fnPointerText,
	&SkyLogic::fnQuit,
	&SkyLogic::fnSpeakMe,
	&SkyLogic::fnSpeakMeDir,
	&SkyLogic::fnSpeakWait,
	&SkyLogic::fnSpeakWaitDir,
	&SkyLogic::fnChooser,
	&SkyLogic::fnHighlight,
	&SkyLogic::fnTextKill,
	&SkyLogic::fnStopMode,
	&SkyLogic::fnWeWait,
	&SkyLogic::fnSendSync,
	&SkyLogic::fnSendFastSync,
	&SkyLogic::fnSendRequest,
	&SkyLogic::fnClearRequest,
	&SkyLogic::fnCheckRequest,
	&SkyLogic::fnStartMenu,
	&SkyLogic::fnUnhighlight,
	&SkyLogic::fnFaceId,
	&SkyLogic::fnForeground,
	&SkyLogic::fnBackground,
	&SkyLogic::fnNewBackground,
	&SkyLogic::fnSort,
	&SkyLogic::fnNoSpriteEngine,
	&SkyLogic::fnNoSpritesA6,
	&SkyLogic::fnResetId,
	&SkyLogic::fnToggleGrid,
	&SkyLogic::fnPause,
	&SkyLogic::fnRunAnimMod,
	&SkyLogic::fnSimpleMod,
	&SkyLogic::fnRunFrames,
	&SkyLogic::fnAwaitSync,
	&SkyLogic::fnIncMegaSet,
	&SkyLogic::fnDecMegaSet,
	&SkyLogic::fnSetMegaSet,
	&SkyLogic::fnMoveItems,
	&SkyLogic::fnNewList,
	&SkyLogic::fnAskThis,
	&SkyLogic::fnRandom,
	&SkyLogic::fnPersonHere,
	&SkyLogic::fnToggleMouse,
	&SkyLogic::fnMouseOn,
	&SkyLogic::fnMouseOff,
	&SkyLogic::fnFetchX,
	&SkyLogic::fnFetchY,
	&SkyLogic::fnTestList,
	&SkyLogic::fnFetchPlace,
	&SkyLogic::fnCustomJoey,
	&SkyLogic::fnSetPalette,
	&SkyLogic::fnTextModule,
	&SkyLogic::fnChangeName,
	&SkyLogic::fnMiniLoad,
	&SkyLogic::fnFlushBuffers,
	&SkyLogic::fnFlushChip,
	&SkyLogic::fnSaveCoods,
	&SkyLogic::fnPlotGrid,
	&SkyLogic::fnRemoveGrid,
	&SkyLogic::fnEyeball,
	&SkyLogic::fnCursorUp,
	&SkyLogic::fnLeaveSection,
	&SkyLogic::fnEnterSection,
	&SkyLogic::fnRestoreGame,
	&SkyLogic::fnRestartGame,
	&SkyLogic::fnNewSwingSeq,
	&SkyLogic::fnWaitSwingEnd,
	&SkyLogic::fnSkipIntroCode,
	&SkyLogic::fnBlankScreen,
	&SkyLogic::fnPrintCredit,
	&SkyLogic::fnLookAt,
	&SkyLogic::fnLincTextModule,
	&SkyLogic::fnTextKill2,
	&SkyLogic::fnSetFont,
	&SkyLogic::fnStartFx,
	&SkyLogic::fnStopFx,
	&SkyLogic::fnStartMusic,
	&SkyLogic::fnStopMusic,
	&SkyLogic::fnFadeDown,
	&SkyLogic::fnFadeUp,
	&SkyLogic::fnQuitToDos,
	&SkyLogic::fnPauseFx,
	&SkyLogic::fnUnPauseFx,
	&SkyLogic::fnPrintf,
};

static const uint32 forwardList1b[] = {
	JOBS_SPEECH,
	JOBS_S4,
	JOBS_ALARMED,
	JOEY_RECYCLE,
	SHOUT_SSS,
	JOEY_MISSION,
	TRANS_MISSION,
	SLOT_MISSION,
	CORNER_MISSION,
	JOEY_LOGIC,
	GORDON_SPEECH,
	JOEY_BUTTON_MISSION,
	LOB_DAD_SPEECH,
	LOB_SON_SPEECH,
	GUARD_SPEECH,
	MANTRACH_SPEECH,
	WRECK_SPEECH,
	ANITA_SPEECH,
	LAMB_FACTORY,
	FORE_SPEECH,
	JOEY_42_MISS,
	JOEY_JUNCTION_MISS,
	WELDER_MISSION,
	JOEY_WELD_MISSION,
	RADMAN_SPEECH,
	LINK_7_29,
	LINK_29_7,
	LAMB_TO_3,
	LAMB_TO_2,
	BURKE_SPEECH,
	BURKE_1,
	BURKE_2,
	DR_BURKE_1,
	JASON_SPEECH,
	JOEY_BELLEVUE,
	ANCHOR_SPEECH,
	ANCHOR_MISSION,
	JOEY_PC_MISSION,
	HOOK_MISSION,
	TREVOR_SPEECH,
	JOEY_FACTORY,
	HELGA_SPEECH,
	JOEY_HELGA_MISSION,
	GALL_BELLEVUE,
	GLASS_MISSION,
	LAMB_FACT_RETURN,
	LAMB_LEAVE_GARDEN,
	LAMB_START_29,
	LAMB_BELLEVUE,
	CABLE_MISSION,
	FOSTER_TOUR,
	LAMB_TOUR,
	FOREMAN_LOGIC,
	LAMB_LEAVE_FACTORY,
	LAMB_BELL_LOGIC,
	LAMB_FACT_2,
	START90,
	0,
	0,
	LINK_28_31,
	LINK_31_28,
	EXIT_LINC,
	DEATH_SCRIPT,
};

static const uint32 forwardList2b[] = {
	STD_ON,
	STD_EXIT_LEFT_ON,
	STD_EXIT_RIGHT_ON,
	ADVISOR_188,
	SHOUT_ACTION,
	MEGA_CLICK,
	MEGA_ACTION,
};

static const uint32 forwardList3b[] = {
	DANI_SPEECH,
	DANIELLE_GO_HOME,
	SPUNKY_GO_HOME,
	HENRI_SPEECH,
	BUZZER_SPEECH,
	FOSTER_VISIT_DANI,
	DANIELLE_LOGIC,
	JUKEBOX_SPEECH,
	VINCENT_SPEECH,
	EDDIE_SPEECH,
	BLUNT_SPEECH,
	DANI_ANSWER_PHONE,
	SPUNKY_SEE_VIDEO,
	SPUNKY_BARK_AT_FOSTER,
	SPUNKY_SMELLS_FOOD,
	BARRY_SPEECH,
	COLSTON_SPEECH,
	GALL_SPEECH,
	BABS_SPEECH,
	CHUTNEY_SPEECH,
	FOSTER_ENTER_COURT,
};

static const uint32 forwardList4b[] = {
	WALTER_SPEECH,
	JOEY_MEDIC,
	JOEY_MED_LOGIC,
	JOEY_MED_MISSION72,
	KEN_LOGIC,
	KEN_SPEECH,
	KEN_MISSION_HAND,
	SC70_IRIS_OPENED,
	SC70_IRIS_CLOSED,
	FOSTER_ENTER_BOARDROOM,
	BORED_ROOM,
	FOSTER_ENTER_NEW_BOARDROOM,
	HOBS_END,
	SC82_JOBS_SSS,
};

static const uint32 forwardList5b[] = {
	SET_UP_INFO_WINDOW,
	SLAB_ON,
	UP_MOUSE,
	DOWN_MOUSE,
	LEFT_MOUSE,
	RIGHT_MOUSE,
};

#define RESULT 1
#define TEXT1 54
void SkyLogic::initScriptVariables() {
	for (uint i = 0; i < sizeof(_scriptVariables)/sizeof(uint32); i++)
		_scriptVariables[i] = 0;

	_scriptVariables[3] = 141;
	_scriptVariables[110] = 62;
	_scriptVariables[146] = 1;
	_scriptVariables[147] = 2;
	_scriptVariables[451] = 8371;
	_scriptVariables[641] = 1;
	_scriptVariables[679] = 1;
	_scriptVariables[694] = 3;
	_scriptVariables[705] = 1;
	_scriptVariables[710] = 1;
	_scriptVariables[711] = 1;
	_scriptVariables[712] = 1;
	_scriptVariables[720] = 1;
	_scriptVariables[721] = 1;
	_scriptVariables[794] = 1;
	_scriptVariables[798] = 1;
	_scriptVariables[799] = 1;
	_scriptVariables[805] = 1;
	_scriptVariables[806] = 1;
	_scriptVariables[807] = 16731;
	_scriptVariables[808] = 1;
	_scriptVariables[809] = 2;
	_scriptVariables[818] = 1;
	_scriptVariables[819] = 1;
	_scriptVariables[820] = 1;
	_scriptVariables[821] = 1;
	_scriptVariables[822] = 1;

	memcpy(_scriptVariables + 353, forwardList1b, sizeof(forwardList1b));
	memcpy(_scriptVariables + 657, forwardList2b, sizeof(forwardList2b));
	memcpy(_scriptVariables + 722, forwardList3b, sizeof(forwardList3b));
	memcpy(_scriptVariables + 664, forwardList4b, sizeof(forwardList4b));
	memcpy(_scriptVariables + 506, forwardList5b, sizeof(forwardList5b));
}

uint32 SkyLogic::script(Compact *compact, uint16 scriptNo, uint16 offset) {
script:
	// process a script
	// low level interface to interpreter

	// scriptNo:
	// Bit  0-11 - Script number
	// Bit 12-15 - Module number

	uint16 moduleNo = (uint16)((scriptNo & 0xff00) >> 12);
	printf("scriptNo: %d, moduleNo: %d, offset: %d\n", scriptNo, moduleNo, offset);
	uint16 *scriptData = _moduleList[moduleNo]; // get module address

	printf("File: %d\n", moduleNo + F_MODULE_0);
	if (!scriptData) { // The module has not been loaded
		scriptData = (uint16 *)_skyDisk->loadFile(moduleNo + F_MODULE_0, NULL);
		_moduleList[moduleNo] = scriptData; // module has been loaded
	}

	uint16 *moduleStart = scriptData;

	// Check whether we have an offset or what
	if (offset)
		scriptData = moduleStart + offset;
	else
		scriptData += READ_LE_UINT16(scriptData + (scriptNo & 0x0fff));

	uint32 a, b, c;
	uint16 command, mcode, s;
	uint16 *tmp;
	int16 t;

	for (;;) {
		command = READ_LE_UINT16(scriptData++); // get a command
		SkyDebug::script(command, scriptData);

		switch (command) {
		case 0: // push_variable
			s = READ_LE_UINT16(scriptData++); // get variable number
			push( _scriptVariables[s/4] );
			break;
		case 1: // less_than
			a = pop();
			b = pop();
			if (a < b)
				push(1);
			else
				push(0);
			break;
		case 2: // push_number
			push(READ_LE_UINT16(scriptData++));
			break;
		case 3: // not_equal
			a = pop();
			b = pop();
			if (a != b)
				push(1);
			else
				push(0);
			break;
		case 4: // if_and
			a = pop();
			b = pop();
			if (a && b)
				push(1);
			else
				push(0);
			break;
		case 5: // skip_zero
			s = READ_LE_UINT16(scriptData++);

			a = pop();
			if (!a)
				scriptData += s/2;
			break;
		case 6: // pop_var
			s = READ_LE_UINT16(scriptData++);
			_scriptVariables[s/4] = pop();
			break;
		case 7: // minus
			a = pop();
			b = pop();
			push(b-a);
			break;
		case 8: // plus
			a = pop();
			b = pop();
			push(b+a);
			break;
		case 9: // skip_always
			s = READ_LE_UINT16(scriptData++);
			scriptData += s/2;
			break;
		case 10: // if_or
			a = pop();
			b = pop();
			if (a || b)
				push(1);
			else
				push(0);
			break;
		case 11: // call_mcode
			s = READ_LE_UINT16(scriptData++);

			a = s;
			b = c = 0;
			assert(s <= 3);
			// No, I did not forget the "break"s
			switch (s) {
			case 3:
				c = pop();
			case 2:
				b = pop();
			case 1:
				a = pop();
			}

			// TODO: save stuff (compare asm code)
			mcode = READ_LE_UINT16(scriptData++)/4; // get mcode number
			SkyDebug::mcode(mcode, a, b, c);

			a = (this->*mcodeTable[mcode]) (a, b, c);

			if (!(a & 0xffff))
				return (((scriptData - moduleStart) << 16) | scriptNo);
			break;
		case 12: // more_than
			a = pop();
			b = pop();
			if (a > b)
				push(1);
			else
				push(0);
			break;
		case 14: // switch
			s = READ_LE_UINT16(scriptData++); // get number of cases

			a = pop(); // and value to switch on

			do {
				if (a == *scriptData) {
					scriptData += *(scriptData + 1)/2;
					scriptData++;
					break;
				}
				scriptData += 2;
			} while (--s);

			s = READ_LE_UINT16(scriptData++);
			scriptData += s; // use the default
			scriptData--;
			break;
		case 15: // push_offset
			// Push a compact access
			s = READ_LE_UINT16(scriptData++);
			tmp = (uint16 *)SkyCompact::getCompactElem(compact, s);
			printf("push_offset: %d\n", *tmp);
			push(*tmp);
			break;
		case 16: // pop_offset
			// pop a value into a compact
			s = READ_LE_UINT16(scriptData++);
			tmp = (uint16 *)SkyCompact::getCompactElem(compact, s);
			*tmp = (uint16)pop();
			break;
		case 17: // is_equal
			a = pop();
			b = pop();
			if (a == b)
				push(1);
			else
				push(0);
			break;
		case 18: // skip_nz
			t = READ_LE_UINT16(scriptData++);
			a = pop();
			if (a)
				scriptData += t/2;
			break;
		case 13:
		case 19: // script_exit
			return scriptNo;
		case 20: // restart_script
			goto script;
		default:
			error("Unknown script command: %d", command);
		}
	}
}

uint32 SkyLogic::fnCacheChip(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnCacheChip");
}

uint32 SkyLogic::fnCacheFast(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnCacheFast");
}

uint32 SkyLogic::fnDrawScreen(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnDrawScreen");
}

uint32 SkyLogic::fnAr(uint32 x, uint32 y, uint32 c) {
	_compact->downFlag = 1; // assume failure in-case logic is interupted by speech (esp Joey)

	_compact->extCompact->arTargetX = (uint16)x;
	_compact->extCompact->arTargetY = (uint16)y;
	_compact->logic = L_AR; // Set to AR mode

	_compact->xcood &= 0xfff8;
	_compact->ycood &= 0xfff8;

	return 0; // drop out of script
}

uint32 SkyLogic::fnArAnimate(uint32 a, uint32 b, uint32 c) {
	_compact->mood = 0; // high level 'not stood still'
	_compact->logic = L_AR_ANIM;
	return 0; // drop out of script
}

uint32 SkyLogic::fnIdle(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnIdle");
}

uint32 SkyLogic::fnInteract(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnInteract");
}

uint32 SkyLogic::fnStartSub(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnStartSub");
}

uint32 SkyLogic::fnTheyStartSub(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnTheyStartSub");
}

uint32 SkyLogic::fnAssignBase(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnAssignBase");
}

uint32 SkyLogic::fnDiskMouse(uint32 a, uint32 b, uint32 c) {
	warning("Stub: fnDiskMouse");
	return 1;
}

uint32 SkyLogic::fnNormalMouse(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnNormalMouse");
}

uint32 SkyLogic::fnBlankMouse(uint32 a, uint32 b, uint32 c) {
	warning("Stub: fnBlankMouse");
	return 1;
}

uint32 SkyLogic::fnCrossMouse(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnCrossMouse");
}

uint32 SkyLogic::fnCursorRight(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnCursorRight");
}

uint32 SkyLogic::fnCursorLeft(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnCursorLeft");
}

uint32 SkyLogic::fnCursorDown(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnCursorDown");
}

uint32 SkyLogic::fnOpenHand(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnOpenHand");
}

uint32 SkyLogic::fnCloseHand(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnCloseHand");
}

uint32 SkyLogic::fnGetTo(uint32 targetPlaceId, uint32 mode, uint32 c) {
	_compact->upFlag = (uint16)mode; // save mode for action script
	_compact->mode += 4; // next level up
	Compact *compact2 = SkyState::fetchCompact(_compact->place);
	uint16 *getToTable = compact2->getToTable;

	while (*getToTable != targetPlaceId)
		getToTable += 2;

	uint16 *scriptNo = (uint16 *)SkyCompact::getCompactElem(_compact, C_BASE_SUB + _compact->mode);
	uint16 *offset   = (uint16 *)SkyCompact::getCompactElem(_compact, C_BASE_SUB + _compact->mode + 2);
	*scriptNo = *(getToTable + 1); // get new script
	*offset = 0;

	return 0; // drop out of script
}

uint32 SkyLogic::fnSetToStand(uint32 a, uint32 b, uint32 c) {
	_compact->mood = 1; // high level stood still

	uint16 *p = (uint16 *)SkyCompact::getCompactElem(_compact, C_STAND_UP
			+ _compact->extCompact->megaSet + _compact->extCompact->dir * 4); 

	_compact->offset = *p++; // get frames offset
	_compact->grafixProg = p;
	_compact->logic = L_SIMPLE_MOD;
	simpleAnim();
	return 0; // drop out of script
}

uint32 SkyLogic::fnTurnTo(uint32 direction, uint32 b, uint32 c) {
	error("Stub: fnTurnTo");
}

uint32 SkyLogic::fnArrived(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnArrived");
}

uint32 SkyLogic::fnLeaving(uint32 a, uint32 b, uint32 c) {
	_compact->extCompact->atWatch = 0;

	if (_compact->extCompact->leaving) {
		_scriptVariables[_compact->extCompact->leaving/4] -= 1; // decrement the script variable
		_compact->extCompact->leaving = 0; // I shall do this only once
	}

	return 1; // keep going
}

uint32 SkyLogic::fnSetAlternate(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSetAlternate");
}

uint32 SkyLogic::fnAltSetAlternate(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnAltSetAlternate");
}

uint32 SkyLogic::fnKillId(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnKillId");
}

uint32 SkyLogic::fnNoHuman(uint32 a, uint32 b, uint32 c) {
	warning("Stub: fnNoHuman");
	return 1;
}

uint32 SkyLogic::fnAddHuman(uint32 a, uint32 b, uint32 c) {
	warning("Stub: fnAddHuman");
	return 1;
}

uint32 SkyLogic::fnAddButtons(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnAddButtons");
}

uint32 SkyLogic::fnNoButtons(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnNoButtons");
}

uint32 SkyLogic::fnSetStop(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSetStop");
}

uint32 SkyLogic::fnClearStop(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnClearStop");
}

uint32 SkyLogic::fnPointerText(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnPointerText");
}

uint32 SkyLogic::fnQuit(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnQuit");
}

uint32 SkyLogic::fnSpeakMe(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSpeakMe");
}

uint32 SkyLogic::fnSpeakMeDir(uint32 a, uint32 b, uint32 c) {
	warning("Stub: fnSpeakMeDir");
	return 0;
}

uint32 SkyLogic::fnSpeakWait(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSpeakWait");
}

uint32 SkyLogic::fnSpeakWaitDir(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSpeakWaitDir");
}

uint32 SkyLogic::fnChooser(uint32 a, uint32 b, uint32 c) {
	warning("fnChooser: lowTextManager unimplented");
	return 1;
	// setup the text questions to be clicked on
	// read from TEXT1 until 0

//	systemFlags |= 1 << SF_CHOOSING; // can't save/restore while choosing

//	theChosenOne = 0; // clear result

	uint32 *p = _scriptVariables + TEXT1;
	uint16 ycood = TOP_LEFT_Y; // rolling coordinate

	while (*p) {
		uint32 textNum = *p++;

		uint8 *data; // = lowTextManager(textNum, GAME_SCREEN_WIDTH, 0, 241, 0);

		// stipple the text
		uint16 height = ((dataFileHeader *)data)->s_height;
		uint16 width = ((dataFileHeader *)data)->s_width;
		width >>= 1;

		for (uint16 i = height; i > 0; i++) {
			for (uint16 j = width; j > 0; j--) {
				if (!*data) // only change 0's
					*data = 1;
				*data += 2;
			}
			data++;
		}

		_compact->getToFlag = (uint16)(textNum & 0xffff);
		_compact->downFlag = (uint16)(*p++ & 0xffff); // get animation number

		_compact->status |= ST_MOUSE; // mouse detects

		_compact->xcood = TOP_LEFT_X; // set coordinates
		_compact->ycood = ycood;
		ycood += 12;
	}

	if (p == _scriptVariables + TEXT1)
		return 1;

	_compact->logic = L_CHOOSE; // player frozen until choice made
	fnAddHuman(0, 0, 0); // bring back mouse

	return 0;
}

uint32 SkyLogic::fnHighlight(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnHighlight");
}

uint32 SkyLogic::fnTextKill(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnTextKill");
}

uint32 SkyLogic::fnStopMode(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnStopMode");
}

uint32 SkyLogic::fnWeWait(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnWeWait");
}

uint32 SkyLogic::fnSendSync(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSendSync");
}

uint32 SkyLogic::fnSendFastSync(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSendFastSync");
}

uint32 SkyLogic::fnSendRequest(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSendRequest");
}

uint32 SkyLogic::fnClearRequest(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnClearRequest");
}

uint32 SkyLogic::fnCheckRequest(uint32 a, uint32 b, uint32 c) {
	// check for interaction request
	
	a = (a & 0xffffff00) | 1; // assume script continue

	ExtCompact *ecpt = _compact->extCompact;

	if (!ecpt->request)
		return a;

	_compact->mode = C_ACTION_MODE; // into action mode

	ecpt->actionSub = ecpt->request;

	ecpt->request = 0; // trash request
	return 0; // drop from script
}

uint32 SkyLogic::fnStartMenu(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnStartMenu");
}

uint32 SkyLogic::fnUnhighlight(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnUnhighlight");
}

uint32 SkyLogic::fnFaceId(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnFaceId");
}

uint32 SkyLogic::fnForeground(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnForeground");
}

uint32 SkyLogic::fnBackground(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnBackground");
}

uint32 SkyLogic::fnNewBackground(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnNewBackground");
}

uint32 SkyLogic::fnSort(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSort");
}

uint32 SkyLogic::fnNoSpriteEngine(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnNoSpriteEngine");
}

uint32 SkyLogic::fnNoSpritesA6(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnNoSpritesA6");
}

uint32 SkyLogic::fnResetId(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnResetId");
}

uint32 SkyLogic::fnToggleGrid(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnToggleGrid");
}

uint32 SkyLogic::fnPause(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnPause");
}

uint32 SkyLogic::fnRunAnimMod(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnRunAnimMod");
}

uint32 SkyLogic::fnSimpleMod(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSimpleMod");
}

uint32 SkyLogic::fnRunFrames(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnRunFrames");
}

uint32 SkyLogic::fnAwaitSync(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnAwaitSync");
}

uint32 SkyLogic::fnIncMegaSet(uint32 a, uint32 b, uint32 c) {
	_compact->extCompact->megaSet += NEXT_MEGA_SET;
	return NEXT_MEGA_SET;
}

uint32 SkyLogic::fnDecMegaSet(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnDecMegaSet");
}

uint32 SkyLogic::fnSetMegaSet(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSetMegaSet");
}

uint32 SkyLogic::fnMoveItems(uint32 listNo, uint32 screenNo, uint32 c) {
	// Move a list of id's to another screen
	uint16 *p = SkyCompact::move_list[listNo];
	for (int i = 0; i < 2; i++) {
		if (!*p)
			return 1;
		Compact *cpt = SkyState::fetchCompact(*p++);
		cpt->screen = (uint16)(screenNo & 0xffff);
	}
	return 1;
}

uint32 SkyLogic::fnNewList(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnNewList");
}

uint32 SkyLogic::fnAskThis(uint32 textNo, uint32 animNo, uint32 c) {
	// find first free position
	uint32 *p = _scriptVariables + TEXT1;
	while (*p)
		p += 2;
	*p++ = textNo;
	*p = animNo;
	return 1;
}

uint32 SkyLogic::fnRandom(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnRandom");
}

uint32 SkyLogic::fnPersonHere(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnPersonHere");
}

uint32 SkyLogic::fnToggleMouse(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnToggleMouse");
}

uint32 SkyLogic::fnMouseOn(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnMouseOn");
}

uint32 SkyLogic::fnMouseOff(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnMouseOff");
}

uint32 SkyLogic::fnFetchX(uint32 a, uint32 b, uint32 c) {
	Compact *cpt = SkyState::fetchCompact(a);
	_scriptVariables[RESULT] = cpt->xcood;
	return 1;
}

uint32 SkyLogic::fnFetchY(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnFetchY");
}

uint32 SkyLogic::fnTestList(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnTestList");
}

uint32 SkyLogic::fnFetchPlace(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnFetchPlace");
}

uint32 SkyLogic::fnCustomJoey(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnCustomJoey");
}

uint32 SkyLogic::fnSetPalette(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSetPalette");
}

uint32 SkyLogic::fnTextModule(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnTextModule");
}

uint32 SkyLogic::fnChangeName(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnChangeName");
}

uint32 SkyLogic::fnMiniLoad(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnMiniLoad");
}

uint32 SkyLogic::fnFlushBuffers(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnFlushBuffers");
}

uint32 SkyLogic::fnFlushChip(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnFlushChip");
}

uint32 SkyLogic::fnSaveCoods(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSaveCoods");
}

uint32 SkyLogic::fnPlotGrid(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnPlotGrid");
}

uint32 SkyLogic::fnRemoveGrid(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnRemoveGrid");
}

uint32 SkyLogic::fnEyeball(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnEyeball");
}

uint32 SkyLogic::fnCursorUp(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnCursorUp");
}

uint32 SkyLogic::fnLeaveSection(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnLeaveSection");
}

uint32 SkyLogic::fnEnterSection(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnEnterSection");
}

uint32 SkyLogic::fnRestoreGame(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnRestoreGame");
}

uint32 SkyLogic::fnRestartGame(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnRestartGame");
}

uint32 SkyLogic::fnNewSwingSeq(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnNewSwingSeq");
}

uint32 SkyLogic::fnWaitSwingEnd(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnWaitSwingEnd");
}

uint32 SkyLogic::fnSkipIntroCode(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSkipIntroCode");
}

uint32 SkyLogic::fnBlankScreen(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnBlankScreen");
}

uint32 SkyLogic::fnPrintCredit(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnPrintCredit");
}

uint32 SkyLogic::fnLookAt(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnLookAt");
}

uint32 SkyLogic::fnLincTextModule(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnLincTextModule");
}

uint32 SkyLogic::fnTextKill2(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnTextKill2");
}

uint32 SkyLogic::fnSetFont(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSetFont");
}

uint32 SkyLogic::fnStartFx(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnStartFx");
}

uint32 SkyLogic::fnStopFx(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnStopFx");
}

uint32 SkyLogic::fnStartMusic(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnStartMusic");
}

uint32 SkyLogic::fnStopMusic(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnStopMusic");
}

uint32 SkyLogic::fnFadeDown(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnFadeDown");
}

uint32 SkyLogic::fnFadeUp(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnFadeUp");
}

uint32 SkyLogic::fnQuitToDos(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnQuitToDos");
}

uint32 SkyLogic::fnPauseFx(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnPauseFx");
}

uint32 SkyLogic::fnUnPauseFx(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnUnPauseFx");
}

uint32 SkyLogic::fnPrintf(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnPrintf");
}

