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

#ifndef BSLOGIC_H
#define BSLOGIC_H
// combination of logic.c and scr_int.c

#include "sworddefs.h"
#include "objectman.h"
#include "common/util.h"

#define NON_ZERO_SCRIPT_VARS 95
#define NUM_SCRIPT_VARS		 1179

class SwordText;
class SwordSound;
class EventManager;
class SwordMenu;
class SwordRouter;
class SwordScreen;
class SwordMouse;
class SwordMusic;

class SwordLogic;
typedef int (SwordLogic::*BSMcodeTable)(BsObject *, int32, int32, int32, int32, int32, int32, int32);

class SwordLogic {
public:
	SwordLogic(ObjectMan *pObjMan, ResMan *resMan, SwordScreen *pScreen, SwordMouse *pMouse, SwordSound *pSound, SwordMusic *pMusic, SwordMenu *pMenu);
	~SwordLogic(void);
	void initialize(void);
	void newScreen(uint32 screen);
	void engine(void);
	void updateScreenParams(void);
	void runMouseScript(BsObject *cpt, int32 scriptId);

	static uint32 _scriptVars[NUM_SCRIPT_VARS];
// public for startPositions()
	int fnAddObject		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnEnterSection	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnPlaySequence	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnMegaSet		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnNoSprite		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);

// public for mouse (menu looking)
	int cfnPresetScript	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
private:
	ObjectMan *_objMan;
	ResMan *_resMan;
	SwordScreen *_screen;
	SwordSound *_sound;
	SwordMouse *_mouse;
	SwordRouter *_router;
	SwordText *_textMan;
	EventManager *_eventMan;
	SwordMenu *_menu;
	SwordMusic *_music;
	uint32 _newScript; // <= ugly, but I can't avoid it.
	bool _speechRunning, _speechFinished, _textRunning;
	uint8 _speechClickDelay;
	Common::RandomSource _rnd;

	int scriptManager(BsObject *compact, uint32 id);
	void processLogic(BsObject *compact, uint32 id);
	int interpretScript(BsObject *compact, int id, Header *scriptModule, int scriptBase, int scriptNum);

	int logicWaitTalk(BsObject *compact);
	int logicStartTalk(BsObject *compact);
	int logicArAnimate(BsObject *compact, uint32 id);
	int speechDriver(BsObject *compact);
	int fullAnimDriver(BsObject *compact);
	int animDriver(BsObject *compact);

	static BSMcodeTable _mcodeTable[100];

	uint16 inRange(uint16 a, uint16 b, uint16 c);

//- mcodeTable:
	int fnBackground	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnForeground	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSort			(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnAnim			(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetFrame		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnFullAnim		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnFullSetFrame	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnFadeDown		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnFadeUp		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnCheckFade		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetSpritePalette(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetWholePalette(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetFadeTargetPalette(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetPaletteToFade(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetPaletteToCut(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);

	int fnIdle			(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnPause			(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnPauseSeconds	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnQuit			(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnKillId		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSuicide		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnNewScript		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSubScript		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnRestartScript	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetBookmark	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnGotoBookmark	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSendSync		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnWaitSync		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	
	int cfnClickInteract(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int cfnSetScript	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);

	int fnInteract		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnIssueEvent	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnCheckForEvent	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnWipeHands		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnISpeak		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnTheyDo		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnTheyDoWeWait	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnWeWait		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnChangeSpeechText(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnTalkError		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnStartTalk		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnCheckForTextLine(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnAddTalkWaitStatusBit(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnRemoveTalkWaitStatusBit(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);

	int fnNoHuman		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnAddHuman		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnBlankMouse	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnNormalMouse	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnLockMouse		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnUnlockMouse	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetMousePointer(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetMouseLuggage(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnMouseOn		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnMouseOff		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnChooser		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnEndChooser	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnStartMenu		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnEndMenu		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	
	int cfnReleaseMenu	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	
	int fnAddSubject	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnRemoveObject	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnLeaveSection	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnChangeFloor	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnWalk			(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnTurn			(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnStand			(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnStandAt		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnFace			(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnFaceXy		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnIsFacing		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnGetTo			(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnGetToError	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnGetPos		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnGetGamepadXy	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnPlayFx		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnStopFx		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnPlayMusic		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnStopMusic		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnInnerSpace	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnRandom		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetScreen		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnPreload		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnCheckCD		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnRestartGame	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnQuitGame		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnDeathScreen	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetParallax	(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnTdebug		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);

	int fnRedFlash		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnBlueFlash		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnYellow		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnGreen			(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnPurple		(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnBlack			(BsObject *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	static const uint32 _scriptVarInit[NON_ZERO_SCRIPT_VARS][2];
};

#endif //BSLOGIC_H
