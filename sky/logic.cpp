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

uint16 SkyLogic::_screen;

typedef void (SkyLogic::*LogicTable) ();
static const LogicTable logicTable[] = {
	&SkyLogic::nop,
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

SkyLogic::SkyLogic(SkyDisk *skyDisk, SkyGrid *skyGrid, SkyText *skyText, SkyMusicBase *skyMusic, SkyMouse *skyMouse, SkySound *skySound, uint32 gameVersion) {
	_skyDisk = skyDisk;
	_skyGrid = skyGrid;
	_skyText = skyText;
	_skyMusic = skyMusic;
	_skySound = skySound;
	_skyMouse = skyMouse;
	_gameVersion = gameVersion;
	_skyAutoRoute = new SkyAutoRoute(_skyGrid);

	for (uint i = 0; i < sizeof(_moduleList)/sizeof(uint16*); i++)
		_moduleList[i] = 0;
	_stackPtr = 0;
	
	initScriptVariables();
}

void SkyLogic::engine() {
	uint16 *logicList = (uint16 *)SkyState::fetchCompact(_scriptVariables[LOGIC_LIST_NO]);

	while (uint16 id = *logicList++) { // 0 means end of list
		if (id == 0xffff) {
			// Change logic data address
			logicList = (uint16 *)SkyState::fetchCompact(*logicList);
			continue;
		}

		_scriptVariables[CUR_ID] = id;
		_compact = SkyState::fetchCompact(id);

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

void SkyLogic::nop() {}

void SkyLogic::logicScript() {
	// Process the current mega's script
	// If the script finishes then drop back a level

	for (;;) {
		uint16 mode = _compact->mode; // get pointer to current script
		printf("compact mode: %d\n", mode);
		uint16 *scriptNo = (uint16 *)SkyCompact::getCompactElem(_compact, C_BASE_SUB + mode);
		uint16 *offset   = (uint16 *)SkyCompact::getCompactElem(_compact, C_BASE_SUB + mode + 2);

		uint32 scr = script(*scriptNo, *offset);
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
	// Follow a route
	// Mega should be in getToMode

	// only check collisions on character boundaries
	if ((_compact->xcood & 7) || (_compact->ycood & 7)) {
		mainAnim();
		return;
	}

	// On character boundary. Have we been told to wait?
	// if not - are WE colliding?

	if (_compact->extCompact->waitingFor == 0xffff) { // 1st cycle of re-route does
		mainAnim();
		return;
	}

	if (_compact->extCompact->waitingFor) {
		// ok, we've been told we've hit someone
		// we will wait until we are no longer colliding
		// with them. here we check to see if we are (still) colliding.
		// if we are then run the stop script. if not clear the flag
		// and continue.

		// remember - this could be the first ar cycle for some time,
		// we might have been told to wait months ago. if we are
		// waiting for one person then another hits us then
		// c_waiting_for will be replaced by the new mega - this is
		// fine because the later collision will almost certainly
		// take longer to clear than the earlier one.

		if (collide(SkyState::fetchCompact(_compact->extCompact->waitingFor)))
			error("stop_and_wait not implemented\n");

		// we are not in fact hitting this person so clr & continue
		// it must have registered some time ago

		_compact->extCompact->waitingFor = 0; // clear id flag
	}

	// ok, our turn to check for collisions

	uint16 *logicList = (uint16 *)SkyState::fetchCompact(_scriptVariables[LOGIC_LIST_NO]);
	Compact *cpt = 0;

	uint16 id;
	while ((id = *logicList++) != 0) { // get an id

		if (id == 0xffff) { // address change?
			logicList = (uint16 *)SkyState::fetchCompact(*logicList); // get new logic list
			continue;
		}

		if (id == (uint16)(_scriptVariables[CUR_ID] & 0xffff)) // is it us?
			continue;

		_scriptVariables[HIT_ID] = id; // save target id for any possible c_mini_bump
		cpt = SkyState::fetchCompact(id); // let's have a closer look

		if (!(cpt->status & (1 << ST_COLLISION_BIT))) // can it collide?
			continue;

		if (cpt->screen != _compact->screen) // is it on our screen?
			continue;

		if (collide(cpt)) { // check for a hit
			// ok, we've hit a mega
			// is it moving... or something else?

			if (cpt->logic != L_AR_ANIM) { // check for following route
				// it is doing something else
				// we restart our get-to script
				// first tell it to wait for us - in case it starts moving
				// ( *it may have already hit us and stopped to wait )

				_compact->extCompact->waitingFor = 0xffff; // effect 1 cycle collision skip
				// tell it it is waiting for us
				cpt->extCompact->waitingFor = (uint16)(_scriptVariables[CUR_ID] & 0xffff);
				// restart current script
				*(uint16 *)SkyCompact::getCompactElem(_compact, C_BASE_SUB + _compact->mode + 2) = 0;
				_compact->logic = L_SCRIPT;
				logicScript();
				return;
			}

			script(_compact->extCompact->miniBump, 0);
			return;
		}
	}

	// ok, there was no collisions
	// now check for interaction request
	// *note: the interaction is always set up as an action script

	if (_compact->extCompact->request) {
		_compact->mode = C_ACTION_MODE; // put into action mode
		_compact->extCompact->actionSub = _compact->extCompact->request;
		_compact->extCompact->actionSub_off = 0;
		_compact->extCompact->request = 0; // trash request
		_compact->logic = L_SCRIPT;
		logicScript();
		return;
	}

	// any flag? - or any change?
	// if change then re-run the current script, which must be
	// a position independent get-to		 ----

	if (!_compact->extCompact->atWatch) { // any flag set?
		mainAnim();
		return;
	}

	// ok, there is an at watch - see if it's changed

	if (_compact->extCompact->atWas == _scriptVariables[_compact->extCompact->atWatch/4]) { // still the same?
		mainAnim();
		return;
	}

	// changed so restart the current script
	// *not suitable for base initiated ARing
	*(uint16 *)SkyCompact::getCompactElem(_compact, C_BASE_SUB + _compact->mode + 2) = 0;

	_compact->logic = L_SCRIPT;
	logicScript();
}

void SkyLogic::mainAnim() {
	_compact->extCompact->waitingFor = 0; // clear possible zero-zero skip

	uint16 *sequence = _compact->grafixProg;
	if (!*sequence) {
		// ok, move to new anim segment
		sequence += 2;
		if (!*sequence) { // end of route?
			// ok, sequence has finished

			// will start afresh if new sequence continues in last direction
			_compact->extCompact->arAnimIndex = 0;

			_compact->downFlag = 0; // pass back ok to script
			_compact->logic = L_SCRIPT;
			logicScript();
			return;
		}

		_compact->grafixProg = sequence;
		_compact->extCompact->arAnimIndex = 0; // reset position
	}

	uint16 dir = 0;
	while ((dir = _compact->extCompact->dir) != *(sequence + 1)) {
		// ok, setup turning
		_compact->extCompact->dir = *(sequence + 1);

		uint16 **tt = (uint16 **)SkyCompact::getCompactElem(_compact,
				C_TURN_TABLE + _compact->extCompact->megaSet + dir * 20);
		if (tt[_compact->extCompact->dir]) {
			_compact->extCompact->turnProg = tt[_compact->extCompact->dir];
			_compact->logic = L_AR_TURNING;
			arTurn();
			return;
		}
	};

	uint16 **animList = (uint16 **)SkyCompact::getCompactElem(_compact,
			C_ANIM_UP + _compact->extCompact->megaSet + dir * 4);

	uint16 arAnimIndex = _compact->extCompact->arAnimIndex;
	if (!(*animList)[arAnimIndex/2]) {
		 arAnimIndex = 0;
		_compact->extCompact->arAnimIndex = 0; // reset
	}

	_compact->extCompact->arAnimIndex += S_LENGTH;

	*sequence       -= (*animList)[(S_COUNT + arAnimIndex)/2]; // reduce the distance to travel
	_compact->frame  = (*animList)[(S_FRAME + arAnimIndex)/2]; // new graphic frame
	_compact->xcood += (*animList)[(S_AR_X  + arAnimIndex)/2]; // update x coordinate
	_compact->ycood += (*animList)[(S_AR_Y  + arAnimIndex)/2]; // update y coordinate
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
	// Follow an animation sequence

	uint16 *grafixProg = _compact->grafixProg;

	while (*grafixProg) {
		if (*grafixProg == LF_START_FX) { // do fx
			grafixProg++;
			uint16 sound = *grafixProg++;
			uint16 volume = *grafixProg++;

			// channel 0
			fnStartFx(sound, 0, volume);
		} else if (*grafixProg >= LF_START_FX) { // do sync
			grafixProg++;

			Compact *cpt = SkyState::fetchCompact(*grafixProg++);

			cpt->sync = *grafixProg++;
		} else { // put coordinates and frame in
			_compact->xcood = *grafixProg++;
			_compact->ycood = *grafixProg++;

			_compact->frame = *grafixProg++ | _compact->offset;
			_compact->grafixProg = grafixProg;
			return;
		}
	}

	_compact->downFlag = 0;
	_compact->logic = L_SCRIPT;
	logicScript();
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
	// Stay in this mode until id in c_get_to_flag leaves l_talk mode

	Compact *cpt = SkyState::fetchCompact(_compact->flag);

	if (cpt->logic == L_TALK)
		return;

	_compact->logic = L_SCRIPT;
	logicScript();
}

void SkyLogic::stopped() {
	error("Stub: SkyLogic::stopped");
}

void SkyLogic::choose() {
	error("Stub: SkyLogic::choose");
}

void SkyLogic::frames() {
	if (!_compact->sync)
		simpleAnim();

	_compact->downFlag = 0; // return 'ok' to script
	_compact->logic = L_SCRIPT;
	logicScript();
}

void SkyLogic::pause() {
	if (--_compact->flag)
		return;

	_compact->logic = L_SCRIPT;
	logicScript();
	return;
}

void SkyLogic::waitSync() {
	// checks c_sync, when its non 0
	// the id is put back into script mode
	// use this instead of loops in the script

	if (!_compact->sync)
		return;

	_compact->logic = L_SCRIPT;
	logicScript();
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

bool SkyLogic::collide(Compact *cpt) {
	MegaSet *m1 = (MegaSet *)SkyCompact::getCompactElem(_compact, C_GRID_WIDTH + _compact->extCompact->megaSet);
	MegaSet *m2 = (MegaSet *)SkyCompact::getCompactElem(cpt, C_GRID_WIDTH + cpt->extCompact->megaSet);

	uint16 x = cpt->xcood; // target's base coordinates
	x &= 0xfff8;
	uint16 y = cpt->ycood;
	y &= 0xfff8;

	// The collision is direction dependant
	switch (_compact->extCompact->dir) {
	case 0: // looking up
		x -= m1->colOffset; // compensate for inner x offsets
		x += m2->colOffset;

		if ((x + m2->colWidth) < _compact->xcood) // their rightmost
			return false;

		x -= m1->colWidth; // our left, their right
		if (x >= _compact->xcood)
			return false;

		y += 8; // bring them down a line
		if (y == _compact->ycood)
			return true;

		y += 8; // bring them down a line
		if (y == _compact->ycood)
			return true;

		return false;
	case 1: // looking down
		x -= m1->colOffset; // compensate for inner x offsets
		x += m2->colOffset;

		if ((x + m2->colWidth) >= _compact->xcood) // their rightmoast
			return false;

		x -= m1->colWidth; // our left, their right
		if (x >= _compact->xcood)
			return false;

		y -= 8; // bring them up a line
		if (y == _compact->ycood)
			return true;

		y -= 8; // bring them up a line
		if (y == _compact->ycood)
			return true;
		
		return false;
	case 2: // looking left

		if (y != _compact->ycood)
			return false;

		x += m2->lastChr;
		if (x == _compact->xcood)
			return true;

		x -= 8; // out another one
		if (x == _compact->xcood)
			return true;

		return false;
	case 3: // looking right
	case 4: // talking (not sure if this makes sense...) 

		if (y != _compact->ycood)
			return false;

		x -= m1->lastChr; // last block
		if (x == _compact->xcood)
			return true;

		x -= 8; // out another block
		if (x != _compact->xcood)
			return false;

		return true;
	default:
		error("Unknown Direction: %d", _compact->extCompact->dir);
	}
}

void SkyLogic::runGetOff() {
	uint32 getOff = _scriptVariables[GET_OFF];
	_scriptVariables[GET_OFF] = 0;
	if (getOff)
		script((uint16)(getOff & 0xffff), (uint16)(getOff >> 16));
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

void SkyLogic::initScriptVariables() {
	for (uint i = 0; i < sizeof(_scriptVariables)/sizeof(uint32); i++)
		_scriptVariables[i] = 0;

	_scriptVariables[2] = 141;
	_scriptVariables[109] = 62;
	_scriptVariables[145] = 1;
	_scriptVariables[146] = 2;
	_scriptVariables[450] = 8371;
	_scriptVariables[640] = 1;
	_scriptVariables[678] = 1;
	_scriptVariables[693] = 3;
	_scriptVariables[704] = 1;
	_scriptVariables[709] = 1;
	_scriptVariables[710] = 1;
	_scriptVariables[711] = 1;
	_scriptVariables[79] = 1;
	_scriptVariables[720] = 1;
	_scriptVariables[793] = 1;
	_scriptVariables[797] = 1;
	_scriptVariables[798] = 1;
	_scriptVariables[804] = 1;
	_scriptVariables[805] = 1;
	_scriptVariables[806] = 16731;
	_scriptVariables[807] = 1;
	_scriptVariables[808] = 2;
	_scriptVariables[817] = 1;
	_scriptVariables[818] = 1;
	_scriptVariables[819] = 1;
	_scriptVariables[820] = 1;
	_scriptVariables[821] = 1;

	memcpy(_scriptVariables + 352, forwardList1b, sizeof(forwardList1b));
	memcpy(_scriptVariables + 656, forwardList2b, sizeof(forwardList2b));
	memcpy(_scriptVariables + 721, forwardList3b, sizeof(forwardList3b));
	memcpy(_scriptVariables + 663, forwardList4b, sizeof(forwardList4b));
	memcpy(_scriptVariables + 505, forwardList5b, sizeof(forwardList5b));
}

uint32 SkyLogic::script(uint16 scriptNo, uint16 offset) {
script:
	// process a script
	// low level interface to interpreter

	// scriptNo:
	// Bit  0-11 - Script number
	// Bit 12-15 - Module number

	uint16 moduleNo = (uint16)((scriptNo & 0xff00) >> 12);
	printf("Doing Script %x\n", (offset << 16) | scriptNo);
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
	uint16 command, s;
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
			{
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

				uint16 mcode = READ_LE_UINT16(scriptData++)/4; // get mcode number
				SkyDebug::mcode(mcode, a, b, c);

				Compact *saveCpt = _compact;
				uint32 ret = (this->*mcodeTable[mcode]) (a, b, c);
				_compact = saveCpt;

				if (!ret)
					return (((scriptData - moduleStart) << 16) | scriptNo);
			}
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
			tmp = (uint16 *)SkyCompact::getCompactElem(_compact, s);
			push(*tmp);
			break;
		case 16: // pop_offset
			// pop a value into a compact
			s = READ_LE_UINT16(scriptData++);
			tmp = (uint16 *)SkyCompact::getCompactElem(_compact, s);
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
	warning("Stub: fnCacheChip");
	return 1;
}

uint32 SkyLogic::fnCacheFast(uint32 a, uint32 b, uint32 c) {
	warning("Stub: fnCacheFast");
	return 1;
}

uint32 SkyLogic::fnDrawScreen(uint32 a, uint32 b, uint32 c) {
	warning("Stub: fnDrawScreen");
	return 1;
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
	// set the player idling
	_compact->logic = 0;
	return 1;
}

uint32 SkyLogic::fnInteract(uint32 targetId, uint32 b, uint32 c) {
	_compact->mode += 4; // next level up
	_compact->logic = L_SCRIPT;
	Compact *cpt = SkyState::fetchCompact(targetId);

	uint16 *scriptNo = (uint16 *)SkyCompact::getCompactElem(_compact, C_BASE_SUB + _compact->mode);
	uint16 *offset   = (uint16 *)SkyCompact::getCompactElem(_compact, C_BASE_SUB + _compact->mode + 2);

	*scriptNo = cpt->actionScript;
	*offset = 0;

	return 0;
}

uint32 SkyLogic::fnStartSub(uint32 scr, uint32 b, uint32 c) {
	_compact->mode += 4;
	uint16 *scriptNo = (uint16 *)SkyCompact::getCompactElem(_compact, C_BASE_SUB + _compact->mode);
	uint16 *offset   = (uint16 *)SkyCompact::getCompactElem(_compact, C_BASE_SUB + _compact->mode + 2);
	*scriptNo = (uint16)(scr & 0xffff);
	*offset   = (uint16)(scr >> 16);
	return 0;
}

uint32 SkyLogic::fnTheyStartSub(uint32 mega, uint32 scr, uint32 c) {
	Compact *cpt = SkyState::fetchCompact(mega);
	cpt->mode += 4;
	uint16 *scriptNo = (uint16 *)SkyCompact::getCompactElem(cpt, C_BASE_SUB + _compact->mode);
	uint16 *offset   = (uint16 *)SkyCompact::getCompactElem(cpt, C_BASE_SUB + _compact->mode + 2);
	*scriptNo = (uint16)(scr & 0xffff);
	*offset   = (uint16)(scr >> 16);
	return 1;
}

uint32 SkyLogic::fnAssignBase(uint32 id, uint32 scr, uint32 c) {
	Compact *cpt = SkyState::fetchCompact(id);
	cpt->mode = C_BASE_MODE;
	cpt->logic = L_SCRIPT;
	cpt->baseSub     = (uint16)(scr & 0xffff);
	cpt->baseSub_off = (uint16)(scr >> 16);
	return 1;
}

uint32 SkyLogic::fnDiskMouse(uint32 a, uint32 b, uint32 c) {
	warning("Stub: fnDiskMouse");
	return 1;
}

uint32 SkyLogic::fnNormalMouse(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnNormalMouse");
}

uint32 SkyLogic::fnBlankMouse(uint32 a, uint32 b, uint32 c) {
	return _skyMouse->fnBlankMouse();
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
	Compact *cpt = SkyState::fetchCompact(_compact->place);
	uint16 *getToTable = cpt->getToTable;

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

uint32 SkyLogic::fnTurnTo(uint32 dir, uint32 b, uint32 c) {
	// turn compact to direction dir

	uint16 curDir = _compact->extCompact->dir * 20; // get current direction
	_compact->extCompact->dir = (uint16)(dir & 0xffff); // set new direction

	uint16 **tt = (uint16 **)SkyCompact::getCompactElem(_compact,
			C_TURN_TABLE + _compact->extCompact->megaSet + curDir);

	if (!tt[dir])
		return 1; // keep going

	_compact->extCompact->turnProg = tt[dir]; // put turn program in
	_compact->logic = L_TURNING;

	turn();

	return 0; // drop out of script
}

uint32 SkyLogic::fnArrived(uint32 scriptVar, uint32 b, uint32 c) {
	_compact->extCompact->leaving = (uint16)(scriptVar & 0xffff);
	_scriptVariables[scriptVar/4]++;
	return 1;
}

uint32 SkyLogic::fnLeaving(uint32 a, uint32 b, uint32 c) {
	_compact->extCompact->atWatch = 0;

	if (_compact->extCompact->leaving) {
		_scriptVariables[_compact->extCompact->leaving/4]--;
		_compact->extCompact->leaving = 0; // I shall do this only once
	}

	return 1; // keep going
}

uint32 SkyLogic::fnSetAlternate(uint32 scr, uint32 b, uint32 c) {
	_compact->extCompact->alt = (uint16)(scr & 0xffff);
	_compact->logic = L_ALT;
	return 0;
}

uint32 SkyLogic::fnAltSetAlternate(uint32 target, uint32 scr, uint32 c) {
	Compact *cpt = SkyState::fetchCompact(target);
	cpt->extCompact->alt = (uint16)(scr & 0xffff);
	cpt->logic = L_ALT;
	return 0;
}

uint32 SkyLogic::fnKillId(uint32 id, uint32 b, uint32 c) {
	if (id) {
		Compact *cpt = SkyState::fetchCompact(id);
		if (cpt->status & (1 << 7))
			_skyGrid->removeObjectFromWalk(cpt);
		cpt->status = 0;
	}
	return 1;
}

uint32 SkyLogic::fnNoHuman(uint32 a, uint32 b, uint32 c) {
	if (!_scriptVariables[MOUSE_STOP]) {
		_scriptVariables[MOUSE_STOP] &= 1;
		runGetOff();
		fnBlankMouse(a, b, c);
	}
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
	_scriptVariables[MOUSE_STOP] |= 1;
	return 1;
}

uint32 SkyLogic::fnClearStop(uint32 a, uint32 b, uint32 c) {
	_scriptVariables[MOUSE_STOP] = 0;
	return 1;
}

uint32 SkyLogic::fnPointerText(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnPointerText");
}

uint32 SkyLogic::fnQuit(uint32 a, uint32 b, uint32 c) {
	return 0;
}

uint32 SkyLogic::fnSpeakMe(uint32 a, uint32 b, uint32 c) {
	warning("Stub: fnSpeakMe");
	return 0;
}

uint32 SkyLogic::fnSpeakMeDir(uint32 a, uint32 b, uint32 c) {
	warning("Stub: fnSpeakMeDir");
	return 0;
}

uint32 SkyLogic::fnSpeakWait(uint32 id, uint32 message, uint32 animation) {
	_compact->flag = (uint16)(id & 0xffff);
	_compact->logic = L_LISTEN;
	return fnSpeakMe(id, message, animation);
}

uint32 SkyLogic::fnSpeakWaitDir(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnSpeakWaitDir");
}

uint32 SkyLogic::fnChooser(uint32 a, uint32 b, uint32 c) {
	
	// setup the text questions to be clicked on
	// read from TEXT1 until 0

//	systemFlags |= 1 << SF_CHOOSING; // can't save/restore while choosing

	_scriptVariables[THE_CHOSEN_ONE] = 0; // clear result

	uint32 *p = _scriptVariables + TEXT1;
	uint16 ycood = TOP_LEFT_Y; // rolling coordinate

	while (*p) {
		uint32 textNum = *p++;

		struct lowTextManager_t lowText = _skyText->lowTextManager(textNum, GAME_SCREEN_WIDTH, 0, 241, 0);
		
		uint8 *data = lowText.textData;

		// stipple the text
		uint16 height = ((dataFileHeader *)data)->s_height;
		uint16 width = ((dataFileHeader *)data)->s_width;
		width >>= 1;

		for (uint16 i = height; i > 0; i--) {
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

uint32 SkyLogic::fnHighlight(uint32 itemNo, uint32 pen, uint32 c) {
	error("Stub: fnHighlight");
}

uint32 SkyLogic::fnTextKill(uint32 a, uint32 b, uint32 c) {
	// Kill of text items that are mouse detectable

	uint32 id = FIRST_TEXT_COMPACT;

	for (int i = 10; i > 0; i--) {
		Compact *cpt = SkyState::fetchCompact(id);
		if (cpt->status & (1 << 4))
			cpt->status = 0;
		id++;
	}
	return 1;
}

uint32 SkyLogic::fnStopMode(uint32 a, uint32 b, uint32 c) {
	_compact->logic = L_STOPPED;
	return 0;
}

uint32 SkyLogic::fnWeWait(uint32 id, uint32 b, uint32 c) {
	error("Stub: fnWeWait");
}

uint32 SkyLogic::fnSendSync(uint32 mega, uint32 sync, uint32 c) {
	Compact *cpt = SkyState::fetchCompact(mega);
	cpt->sync = (uint16)(sync & 0xffff);
	return 0;
}

uint32 SkyLogic::fnSendFastSync(uint32 mega, uint32 sync, uint32 c) {
	Compact *cpt = SkyState::fetchCompact(mega);
	cpt->sync = (uint16)(sync & 0xffff);
	return 1;
}

uint32 SkyLogic::fnSendRequest(uint32 target, uint32 scr, uint32 c) {
	Compact *cpt = SkyState::fetchCompact(target);
	cpt->extCompact->request = (uint16)(scr & 0xffff);
	return 0;
}

uint32 SkyLogic::fnClearRequest(uint32 target, uint32 b, uint32 c) {
	Compact *cpt = SkyState::fetchCompact(target);
	cpt->extCompact->request = 0;
	return 1;
}

uint32 SkyLogic::fnCheckRequest(uint32 a, uint32 b, uint32 c) {
	// check for interaction request
	
	if (!_compact->extCompact->request)
		return 1;

	_compact->mode = C_ACTION_MODE; // into action mode

	_compact->extCompact->actionSub = _compact->extCompact->request;
	_compact->extCompact->actionSub_off = 0;

	_compact->extCompact->request = 0; // trash request
	return 0; // drop from script
}

uint32 SkyLogic::fnStartMenu(uint32 a, uint32 b, uint32 c) {
	error("Stub: fnStartMenu");
}

uint32 SkyLogic::fnUnhighlight(uint32 item, uint32 b, uint32 c) {
	Compact *cpt = SkyState::fetchCompact(item);
	cpt->frame--;
	cpt->getToFlag = 0;
	return 1;
}

uint32 SkyLogic::fnFaceId(uint32 otherId, uint32 b, uint32 c) {
	// return the direction to turn to face another id
	// pass back result in c_just_flag

	Compact *cpt = SkyState::fetchCompact(otherId);

	int16 x = _compact->xcood - cpt->xcood;

	if (x < 0) { // we're to the left
		x = -x;
		_compact->getToFlag = 3;
	} else { // it's to the left
		_compact->getToFlag = 2;
	}

	// now check y

	// we must find the true bottom of the sprite
	// it is not enough to use y coord because changing
	// sprite offsets can ruin the formula - instead we
	// will use the bottom of the mouse collision area

	int16 y = _compact->ycood - (cpt->ycood - cpt->mouseRelY - cpt->mouseSizeY);

	if (y < 0) { // it's below
		y = -y;
		if (y >= x)
			_compact->getToFlag = 1;
	} else { // it's above
		if (y >= x)
			_compact->getToFlag = 0;
	}
	return 1;
}

uint32 SkyLogic::fnForeground(uint32 sprite, uint32 b, uint32 c) {
	// Make sprite a foreground sprite
	Compact *cpt = SkyState::fetchCompact(sprite);
	cpt->status &= 0xfff8;
	cpt->status |= ST_FOREGROUND;
	return 1;
}

uint32 SkyLogic::fnBackground(uint32 a, uint32 b, uint32 c) {
	// Make us a background sprite
	_compact->status &= 0xfff8;
	_compact->status |= ST_BACKGROUND;
	return 1;
}

uint32 SkyLogic::fnNewBackground(uint32 sprite, uint32 b, uint32 c) {
	// Make sprite a background sprite
	Compact *cpt = SkyState::fetchCompact(sprite);
	cpt->status &= 0xfff8;
	cpt->status |= ST_BACKGROUND;
	return 1;
}

uint32 SkyLogic::fnSort(uint32 mega, uint32 b, uint32 c) {
	Compact *cpt = SkyState::fetchCompact(mega);
	cpt->status &= 0xfff8;
	cpt->status |= ST_SORT;
	return 1;
}

uint32 SkyLogic::fnNoSpriteEngine(uint32 a, uint32 b, uint32 c) {
	// stop the compact printing
	// remove foreground, background & sort
	_compact->status &= 0xfff8;
	return 1;	
}

uint32 SkyLogic::fnNoSpritesA6(uint32 us, uint32 b, uint32 c) {
	// stop the compact printing
	// remove foreground, background & sort
	Compact *cpt = SkyState::fetchCompact(us);
	cpt->status &= 0xfff8;
	return 1;	
}

uint32 SkyLogic::fnResetId(uint32 id, uint32 resetBlock, uint32 c) {
	// used when a mega is to be restarted
	// eg - when a smaller mega turn to larger
	// - a mega changes rooms...

	Compact *cpt = SkyState::fetchCompact(id);
	uint16 *rst = (uint16 *)SkyState::fetchCompact(resetBlock);

	uint16 off;
	while ((off = *rst++) != 0xffff)
		*(uint16 *)SkyCompact::getCompactElem(cpt, off) = *rst++;
	return 1;
}

uint32 SkyLogic::fnToggleGrid(uint32 a, uint32 b, uint32 c) {
	// Toggle a mega's grid plotting
	_compact->status ^= ST_GRID_PLOT;
	return 1;
}

uint32 SkyLogic::fnPause(uint32 cycles, uint32 b, uint32 c) {
	// Set mega to l_pause
	_compact->flag = (uint16)(cycles & 0xffff);
	_compact->logic = L_PAUSE;
	return 0; // drop out of script
}

uint32 SkyLogic::fnRunAnimMod(uint32 animNo, uint32 b, uint32 c) {
	uint16 *animation = (uint16 *)SkyState::fetchCompact(animNo);
	uint16 sprite = *animation++; // get sprite set
	_compact->offset = sprite;
	_compact->grafixProg = animation;
	_compact->logic = L_MOD_ANIMATE;
	anim();
	return 0; // drop from script
}

uint32 SkyLogic::fnSimpleMod(uint32 animSeqNo, uint32 b, uint32 c) {
	uint16 *animSeq = (uint16 *)SkyState::fetchCompact(animSeqNo);

	_compact->offset = *animSeq++;
	assert(*animSeq != 0);
	_compact->grafixProg = animSeq;
	_compact->logic = L_SIMPLE_MOD;
	simpleAnim();
	return 0;
}

uint32 SkyLogic::fnRunFrames(uint32 sequenceNo, uint32 b, uint32 c) {
	uint16 *sequence = (uint16 *)SkyState::fetchCompact(sequenceNo);

	_compact->logic = L_FRAMES;
	_compact->offset = *sequence++;
	_compact->grafixProg = sequence;
	simpleAnim();
	return 0;
}

uint32 SkyLogic::fnAwaitSync(uint32 a, uint32 b, uint32 c) {
	if (_compact->sync)
		return 1;

	_compact->logic = L_WAIT_SYNC;
	return 0;
}

uint32 SkyLogic::fnIncMegaSet(uint32 a, uint32 b, uint32 c) {
	_compact->extCompact->megaSet += NEXT_MEGA_SET;
	return 1;
}

uint32 SkyLogic::fnDecMegaSet(uint32 a, uint32 b, uint32 c) {
	_compact->extCompact->megaSet -= NEXT_MEGA_SET;
	return 1;
}

uint32 SkyLogic::fnSetMegaSet(uint32 mega, uint32 setNo, uint32 c) {
	Compact *cpt = SkyState::fetchCompact(mega);
	cpt->extCompact->megaSet = (uint16) (setNo * NEXT_MEGA_SET);
	return 1;
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

uint32 SkyLogic::fnPersonHere(uint32 id, uint32 room, uint32 c) {
	Compact *cpt = SkyState::fetchCompact(id);
	_scriptVariables[RESULT] = cpt->screen == room ? 1 : 0;
	return 1;
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

uint32 SkyLogic::fnEnterSection(uint32 sectionNo, uint32 b, uint32 c) {
	
	if (SkyState::isDemo(_gameVersion))
		if (sectionNo > 2)
			error("End of demo");

	_scriptVariables[CUR_SECTION] = sectionNo;

	if (sectionNo == 5) //linc section - has different mouse icons
		_skyMouse->replaceMouseCursors(60302);

	else
		if (sectionNo != _currentSection) {

			_currentSection = sectionNo;
			_saveCurrentSection = sectionNo;

			sectionNo++;
			_skyMusic->loadSection((byte)sectionNo);
			_skySound->loadSection((byte)sectionNo);
			_skyGrid->loadGrids();

		}
			
	return 1;
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
	warning("Stub: fnSkipIntroCode");
	return 1;
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
	warning("Stub: fnStartFx");
	return 1;
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

