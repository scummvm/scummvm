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

#ifndef SKYLOGIC_H
#define SKYLOGIC_H

#include "sky/sky.h"
#include "sky/disk.h"
#include "sky/grid.h"
#include "sky/autoroute.h"
#include "sky/music/musicbase.h"
#include "sky/mouse.h"
#include "sky/screen.h"
#include "sky/control.h"

enum scriptVariableOffsets {
	RESULT = 0,
	SCREEN = 1,
	LOGIC_LIST_NO = 2,
	MOUSE_LIST_NO = 6,
	DRAW_LIST_NO = 8,
	CUR_ID = 12,
	MOUSE_STATUS = 13,
	MOUSE_STOP = 14,
	BUTTON = 15,
	SPECIAL_ITEM = 17,
	GET_OFF = 18,
	CURSOR_ID = 22,
	SAFEX = 25,
	SAFEY = 26,
	PLAYER_X = 27,
	PLAYER_Y = 28,
	PLAYER_MOOD = 29,
	PLAYER_SCREEN = 30,
	HIT_ID = 37,
	LAYER_0_ID = 41,
	LAYER_1_ID = 42,
	LAYER_2_ID = 43,
	LAYER_3_ID = 44,
	GRID_1_ID = 45,
	GRID_2_ID = 46,
	GRID_3_ID = 47,
	THE_CHOSEN_ONE = 51,
	TEXT1 = 53,
	MENU_LENGTH = 100,
	SCROLL_OFFSET = 101,
	MENU = 102,
	OBJECT_HELD = 103,
	RND = 115,
	CUR_SECTION = 143,
	CONSOLE_TYPE = 345,
	REICH_DOOR_FLAG = 470,
	FS_COMMAND = 643,
	ENTER_DIGITS = 644,
	LINC_DIGIT_0 = 646,
	LINC_DIGIT_1 = 647,
	LINC_DIGIT_2 = 648,
	LINC_DIGIT_3 = 649,
	LINC_DIGIT_4 = 650,
	LINC_DIGIT_5 = 651,
	LINC_DIGIT_6 = 651,
	LINC_DIGIT_7 = 653,
	LINC_DIGIT_8 = 654,
	LINC_DIGIT_9 = 655
};

class SkyAutoRoute;
class SkyScreen;
class SkyMouse;
class SkyControl;

class SkyLogic {
public:
	SkyLogic(SkyScreen *skyScreen, SkyDisk *skyDisk, SkyText *skyText, SkyMusicBase *skyMusic, SkyMouse *skyMouse, SkySound *skySound);
	void engine();
	bool checkProtection(void);
	void useControlInstance(SkyControl *control) { _skyControl = control; };

	void nop();
	void logicScript();
	void autoRoute();
	void arAnim();
	void arTurn();
	void alt();
	void anim();
	void turn();
	void cursor();
	void talk();
	void listen();
	void stopped();
	void choose();
	void frames();
	void pause();
	void waitSync();
	void simpleAnim();
	uint16 mouseScript(uint32 scrNum, Compact *scriptComp);
	uint16 script(uint16 scriptNo, uint16 offset);

	bool fnCacheChip(uint32 a, uint32 b, uint32 c);
	bool fnCacheFast(uint32 a, uint32 b, uint32 c);
	bool fnDrawScreen(uint32 a, uint32 b, uint32 c);
	bool fnAr(uint32 a, uint32 b, uint32 c);
	bool fnArAnimate(uint32 a, uint32 b, uint32 c);
	bool fnIdle(uint32 a, uint32 b, uint32 c);
	bool fnInteract(uint32 a, uint32 b, uint32 c);
	bool fnStartSub(uint32 a, uint32 b, uint32 c);
	bool fnTheyStartSub(uint32 a, uint32 b, uint32 c);
	bool fnAssignBase(uint32 a, uint32 b, uint32 c);
	bool fnDiskMouse(uint32 a, uint32 b, uint32 c);
	bool fnNormalMouse(uint32 a, uint32 b, uint32 c);
	bool fnBlankMouse(uint32 a, uint32 b, uint32 c);
	bool fnCrossMouse(uint32 a, uint32 b, uint32 c);
	bool fnCursorRight(uint32 a, uint32 b, uint32 c);
	bool fnCursorLeft(uint32 a, uint32 b, uint32 c);
	bool fnCursorDown(uint32 a, uint32 b, uint32 c);
	bool fnOpenHand(uint32 a, uint32 b, uint32 c);
	bool fnCloseHand(uint32 a, uint32 b, uint32 c);
	bool fnGetTo(uint32 a, uint32 b, uint32 c);
	bool fnSetToStand(uint32 a, uint32 b, uint32 c);
	bool fnTurnTo(uint32 a, uint32 b, uint32 c);
	bool fnArrived(uint32 a, uint32 b, uint32 c);
	bool fnLeaving(uint32 a, uint32 b, uint32 c);
	bool fnSetAlternate(uint32 a, uint32 b, uint32 c);
	bool fnAltSetAlternate(uint32 a, uint32 b, uint32 c);
	bool fnKillId(uint32 a, uint32 b, uint32 c);
	bool fnNoHuman(uint32 a, uint32 b, uint32 c);
	bool fnAddHuman(uint32 a, uint32 b, uint32 c);
	bool fnAddButtons(uint32 a, uint32 b, uint32 c);
	bool fnNoButtons(uint32 a, uint32 b, uint32 c);
	bool fnSetStop(uint32 a, uint32 b, uint32 c);
	bool fnClearStop(uint32 a, uint32 b, uint32 c);
	bool fnPointerText(uint32 a, uint32 b, uint32 c);
	bool fnQuit(uint32 a, uint32 b, uint32 c);
	bool fnSpeakMe(uint32 targetId, uint32 mesgNum, uint32 animNum);
	bool fnSpeakMeDir(uint32 targetId, uint32 mesgNum, uint32 animNum);
	bool fnSpeakWait(uint32 a, uint32 b, uint32 c);
	bool fnSpeakWaitDir(uint32 a, uint32 b, uint32 c);
	bool fnChooser(uint32 a, uint32 b, uint32 c);
	bool fnHighlight(uint32 a, uint32 b, uint32 c);
	bool fnTextKill(uint32 a, uint32 b, uint32 c);
	bool fnStopMode(uint32 a, uint32 b, uint32 c);
	bool fnWeWait(uint32 a, uint32 b, uint32 c);
	bool fnSendSync(uint32 a, uint32 b, uint32 c);
	bool fnSendFastSync(uint32 a, uint32 b, uint32 c);
	bool fnSendRequest(uint32 a, uint32 b, uint32 c);
	bool fnClearRequest(uint32 a, uint32 b, uint32 c);
	bool fnCheckRequest(uint32 a, uint32 b, uint32 c);
	bool fnStartMenu(uint32 a, uint32 b, uint32 c);
	bool fnUnhighlight(uint32 a, uint32 b, uint32 c);
	bool fnFaceId(uint32 a, uint32 b, uint32 c);
	bool fnForeground(uint32 a, uint32 b, uint32 c);
	bool fnBackground(uint32 a, uint32 b, uint32 c);
	bool fnNewBackground(uint32 a, uint32 b, uint32 c);
	bool fnSort(uint32 a, uint32 b, uint32 c);
	bool fnNoSpriteEngine(uint32 a, uint32 b, uint32 c);
	bool fnNoSpritesA6(uint32 a, uint32 b, uint32 c);
	bool fnResetId(uint32 a, uint32 b, uint32 c);
	bool fnToggleGrid(uint32 a, uint32 b, uint32 c);
	bool fnPause(uint32 a, uint32 b, uint32 c);
	bool fnRunAnimMod(uint32 a, uint32 b, uint32 c);
	bool fnSimpleMod(uint32 a, uint32 b, uint32 c);
	bool fnRunFrames(uint32 a, uint32 b, uint32 c);
	bool fnAwaitSync(uint32 a, uint32 b, uint32 c);
	bool fnIncMegaSet(uint32 a, uint32 b, uint32 c);
	bool fnDecMegaSet(uint32 a, uint32 b, uint32 c);
	bool fnSetMegaSet(uint32 a, uint32 b, uint32 c);
	bool fnMoveItems(uint32 a, uint32 b, uint32 c);
	bool fnNewList(uint32 a, uint32 b, uint32 c);
	bool fnAskThis(uint32 a, uint32 b, uint32 c);
	bool fnRandom(uint32 a, uint32 b, uint32 c);
	bool fnPersonHere(uint32 a, uint32 b, uint32 c);
	bool fnToggleMouse(uint32 a, uint32 b, uint32 c);
	bool fnMouseOn(uint32 a, uint32 b, uint32 c);
	bool fnMouseOff(uint32 a, uint32 b, uint32 c);
	bool fnFetchX(uint32 a, uint32 b, uint32 c);
	bool fnFetchY(uint32 a, uint32 b, uint32 c);
	bool fnTestList(uint32 a, uint32 b, uint32 c);
	bool fnFetchPlace(uint32 a, uint32 b, uint32 c);
	bool fnCustomJoey(uint32 a, uint32 b, uint32 c);
	bool fnSetPalette(uint32 a, uint32 b, uint32 c);
	bool fnTextModule(uint32 a, uint32 b, uint32 c);
	bool fnChangeName(uint32 a, uint32 b, uint32 c);
	bool fnMiniLoad(uint32 a, uint32 b, uint32 c);
	bool fnFlushBuffers(uint32 a, uint32 b, uint32 c);
	bool fnFlushChip(uint32 a, uint32 b, uint32 c);
	bool fnSaveCoods(uint32 a, uint32 b, uint32 c);
	bool fnPlotGrid(uint32 a, uint32 b, uint32 c);
	bool fnRemoveGrid(uint32 a, uint32 b, uint32 c);
	bool fnEyeball(uint32 a, uint32 b, uint32 c);
	bool fnCursorUp(uint32 a, uint32 b, uint32 c);
	bool fnLeaveSection(uint32 a, uint32 b, uint32 c);
	bool fnEnterSection(uint32 sectionNo, uint32 b, uint32 c);
	bool fnRestoreGame(uint32 a, uint32 b, uint32 c);
	bool fnRestartGame(uint32 a, uint32 b, uint32 c);
	bool fnNewSwingSeq(uint32 a, uint32 b, uint32 c);
	bool fnWaitSwingEnd(uint32 a, uint32 b, uint32 c);
	bool fnSkipIntroCode(uint32 a, uint32 b, uint32 c);
	bool fnBlankScreen(uint32 a, uint32 b, uint32 c);
	bool fnPrintCredit(uint32 a, uint32 b, uint32 c);
	bool fnLookAt(uint32 a, uint32 b, uint32 c);
	bool fnLincTextModule(uint32 a, uint32 b, uint32 c);
	bool fnTextKill2(uint32 a, uint32 b, uint32 c);
	bool fnSetFont(uint32 a, uint32 b, uint32 c);
	bool fnStartFx(uint32 a, uint32 b, uint32 c);
	bool fnStopFx(uint32 a, uint32 b, uint32 c);
	bool fnStartMusic(uint32 a, uint32 b, uint32 c);
	bool fnStopMusic(uint32 a, uint32 b, uint32 c);
	bool fnFadeDown(uint32 a, uint32 b, uint32 c);
	bool fnFadeUp(uint32 a, uint32 b, uint32 c);
	bool fnQuitToDos(uint32 a, uint32 b, uint32 c);
	bool fnPauseFx(uint32 a, uint32 b, uint32 c);
	bool fnUnPauseFx(uint32 a, uint32 b, uint32 c);
	bool fnPrintf(uint32 a, uint32 b, uint32 c);

	void stdSpeak(Compact *target, uint32 textNum, uint32 animNum, uint32 base);
	
	static uint32 _scriptVariables[838];
	SkyGrid *_skyGrid;
	
protected:
	void push(uint32);
	uint32 pop();
	void checkModuleLoaded(uint16 moduleNo);
	bool collide(Compact *cpt);
	void initScriptVariables();
	void mainAnim();
	void runGetOff();
	void stopAndWait();

	uint16 *_moduleList[16];
	uint32 _stack[20];
	byte _stackPtr;

	Compact *_compact;

	uint32 _objectList[30];

	uint32 _currentSection;

	RandomSource _rnd;
	
	SkyScreen *_skyScreen;
	SkyDisk *_skyDisk;
	SkyText *_skyText;
	SkyMusicBase *_skyMusic;
	SkySound *_skySound;
	SkyAutoRoute *_skyAutoRoute;
	SkyMouse *_skyMouse;
	SkyControl *_skyControl;
};

#endif
