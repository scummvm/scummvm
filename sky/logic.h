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

class SkyLogic {
public:
	SkyLogic(SkyDisk *skyDisk, SkyGrid *skyGrid, SkyText *skyText);
	void engine();

	void lreturn();
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


	uint32 fnCacheChip(uint32 a, uint32 b, uint32 c);
	uint32 fnCacheFast(uint32 a, uint32 b, uint32 c);
	uint32 fnDrawScreen(uint32 a, uint32 b, uint32 c);
	uint32 fnAr(uint32 a, uint32 b, uint32 c);
	uint32 fnArAnimate(uint32 a, uint32 b, uint32 c);
	uint32 fnIdle(uint32 a, uint32 b, uint32 c);
	uint32 fnInteract(uint32 a, uint32 b, uint32 c);
	uint32 fnStartSub(uint32 a, uint32 b, uint32 c);
	uint32 fnTheyStartSub(uint32 a, uint32 b, uint32 c);
	uint32 fnAssignBase(uint32 a, uint32 b, uint32 c);
	uint32 fnDiskMouse(uint32 a, uint32 b, uint32 c);
	uint32 fnNormalMouse(uint32 a, uint32 b, uint32 c);
	uint32 fnBlankMouse(uint32 a, uint32 b, uint32 c);
	uint32 fnCrossMouse(uint32 a, uint32 b, uint32 c);
	uint32 fnCursorRight(uint32 a, uint32 b, uint32 c);
	uint32 fnCursorLeft(uint32 a, uint32 b, uint32 c);
	uint32 fnCursorDown(uint32 a, uint32 b, uint32 c);
	uint32 fnOpenHand(uint32 a, uint32 b, uint32 c);
	uint32 fnCloseHand(uint32 a, uint32 b, uint32 c);
	uint32 fnGetTo(uint32 a, uint32 b, uint32 c);
	uint32 fnSetToStand(uint32 a, uint32 b, uint32 c);
	uint32 fnTurnTo(uint32 a, uint32 b, uint32 c);
	uint32 fnArrived(uint32 a, uint32 b, uint32 c);
	uint32 fnLeaving(uint32 a, uint32 b, uint32 c);
	uint32 fnSetAlternate(uint32 a, uint32 b, uint32 c);
	uint32 fnAltSetAlternate(uint32 a, uint32 b, uint32 c);
	uint32 fnKillId(uint32 a, uint32 b, uint32 c);
	uint32 fnNoHuman(uint32 a, uint32 b, uint32 c);
	uint32 fnAddHuman(uint32 a, uint32 b, uint32 c);
	uint32 fnAddButtons(uint32 a, uint32 b, uint32 c);
	uint32 fnNoButtons(uint32 a, uint32 b, uint32 c);
	uint32 fnSetStop(uint32 a, uint32 b, uint32 c);
	uint32 fnClearStop(uint32 a, uint32 b, uint32 c);
	uint32 fnPointerText(uint32 a, uint32 b, uint32 c);
	uint32 fnQuit(uint32 a, uint32 b, uint32 c);
	uint32 fnSpeakMe(uint32 a, uint32 b, uint32 c);
	uint32 fnSpeakMeDir(uint32 a, uint32 b, uint32 c);
	uint32 fnSpeakWait(uint32 a, uint32 b, uint32 c);
	uint32 fnSpeakWaitDir(uint32 a, uint32 b, uint32 c);
	uint32 fnChooser(uint32 a, uint32 b, uint32 c);
	uint32 fnHighlight(uint32 a, uint32 b, uint32 c);
	uint32 fnTextKill(uint32 a, uint32 b, uint32 c);
	uint32 fnStopMode(uint32 a, uint32 b, uint32 c);
	uint32 fnWeWait(uint32 a, uint32 b, uint32 c);
	uint32 fnSendSync(uint32 a, uint32 b, uint32 c);
	uint32 fnSendFastSync(uint32 a, uint32 b, uint32 c);
	uint32 fnSendRequest(uint32 a, uint32 b, uint32 c);
	uint32 fnClearRequest(uint32 a, uint32 b, uint32 c);
	uint32 fnCheckRequest(uint32 a, uint32 b, uint32 c);
	uint32 fnStartMenu(uint32 a, uint32 b, uint32 c);
	uint32 fnUnhighlight(uint32 a, uint32 b, uint32 c);
	uint32 fnFaceId(uint32 a, uint32 b, uint32 c);
	uint32 fnForeground(uint32 a, uint32 b, uint32 c);
	uint32 fnBackground(uint32 a, uint32 b, uint32 c);
	uint32 fnNewBackground(uint32 a, uint32 b, uint32 c);
	uint32 fnSort(uint32 a, uint32 b, uint32 c);
	uint32 fnNoSpriteEngine(uint32 a, uint32 b, uint32 c);
	uint32 fnNoSpritesA6(uint32 a, uint32 b, uint32 c);
	uint32 fnResetId(uint32 a, uint32 b, uint32 c);
	uint32 fnToggleGrid(uint32 a, uint32 b, uint32 c);
	uint32 fnPause(uint32 a, uint32 b, uint32 c);
	uint32 fnRunAnimMod(uint32 a, uint32 b, uint32 c);
	uint32 fnSimpleMod(uint32 a, uint32 b, uint32 c);
	uint32 fnRunFrames(uint32 a, uint32 b, uint32 c);
	uint32 fnAwaitSync(uint32 a, uint32 b, uint32 c);
	uint32 fnIncMegaSet(uint32 a, uint32 b, uint32 c);
	uint32 fnDecMegaSet(uint32 a, uint32 b, uint32 c);
	uint32 fnSetMegaSet(uint32 a, uint32 b, uint32 c);
	uint32 fnMoveItems(uint32 a, uint32 b, uint32 c);
	uint32 fnNewList(uint32 a, uint32 b, uint32 c);
	uint32 fnAskThis(uint32 a, uint32 b, uint32 c);
	uint32 fnRandom(uint32 a, uint32 b, uint32 c);
	uint32 fnPersonHere(uint32 a, uint32 b, uint32 c);
	uint32 fnToggleMouse(uint32 a, uint32 b, uint32 c);
	uint32 fnMouseOn(uint32 a, uint32 b, uint32 c);
	uint32 fnMouseOff(uint32 a, uint32 b, uint32 c);
	uint32 fnFetchX(uint32 a, uint32 b, uint32 c);
	uint32 fnFetchY(uint32 a, uint32 b, uint32 c);
	uint32 fnTestList(uint32 a, uint32 b, uint32 c);
	uint32 fnFetchPlace(uint32 a, uint32 b, uint32 c);
	uint32 fnCustomJoey(uint32 a, uint32 b, uint32 c);
	uint32 fnSetPalette(uint32 a, uint32 b, uint32 c);
	uint32 fnTextModule(uint32 a, uint32 b, uint32 c);
	uint32 fnChangeName(uint32 a, uint32 b, uint32 c);
	uint32 fnMiniLoad(uint32 a, uint32 b, uint32 c);
	uint32 fnFlushBuffers(uint32 a, uint32 b, uint32 c);
	uint32 fnFlushChip(uint32 a, uint32 b, uint32 c);
	uint32 fnSaveCoods(uint32 a, uint32 b, uint32 c);
	uint32 fnPlotGrid(uint32 a, uint32 b, uint32 c);
	uint32 fnRemoveGrid(uint32 a, uint32 b, uint32 c);
	uint32 fnEyeball(uint32 a, uint32 b, uint32 c);
	uint32 fnCursorUp(uint32 a, uint32 b, uint32 c);
	uint32 fnLeaveSection(uint32 a, uint32 b, uint32 c);
	uint32 fnEnterSection(uint32 a, uint32 b, uint32 c);
	uint32 fnRestoreGame(uint32 a, uint32 b, uint32 c);
	uint32 fnRestartGame(uint32 a, uint32 b, uint32 c);
	uint32 fnNewSwingSeq(uint32 a, uint32 b, uint32 c);
	uint32 fnWaitSwingEnd(uint32 a, uint32 b, uint32 c);
	uint32 fnSkipIntroCode(uint32 a, uint32 b, uint32 c);
	uint32 fnBlankScreen(uint32 a, uint32 b, uint32 c);
	uint32 fnPrintCredit(uint32 a, uint32 b, uint32 c);
	uint32 fnLookAt(uint32 a, uint32 b, uint32 c);
	uint32 fnLincTextModule(uint32 a, uint32 b, uint32 c);
	uint32 fnTextKill2(uint32 a, uint32 b, uint32 c);
	uint32 fnSetFont(uint32 a, uint32 b, uint32 c);
	uint32 fnStartFx(uint32 a, uint32 b, uint32 c);
	uint32 fnStopFx(uint32 a, uint32 b, uint32 c);
	uint32 fnStartMusic(uint32 a, uint32 b, uint32 c);
	uint32 fnStopMusic(uint32 a, uint32 b, uint32 c);
	uint32 fnFadeDown(uint32 a, uint32 b, uint32 c);
	uint32 fnFadeUp(uint32 a, uint32 b, uint32 c);
	uint32 fnQuitToDos(uint32 a, uint32 b, uint32 c);
	uint32 fnPauseFx(uint32 a, uint32 b, uint32 c);
	uint32 fnUnPauseFx(uint32 a, uint32 b, uint32 c);
	uint32 fnPrintf(uint32 a, uint32 b, uint32 c);

	static uint16 _screen;
	
protected:
	void push(uint32);
	uint32 pop();
	void checkModuleLoaded(uint16 moduleNo);
	uint32 script(Compact *compact, uint16 scriptNo, uint16 offset);
	bool collide(Compact *cpt);
	void initScriptVariables();
	void mainAnim();

	uint16 *_moduleList[16];
	uint32 _stack[20];
	byte _stackPtr;

	Compact *_compact;
	uint32 _scriptVariables[838];

	SkyDisk *_skyDisk;
	SkyGrid *_skyGrid;
	SkyText *_skyText;
	SkyAutoRoute *_skyAutoRoute;
};

#endif
