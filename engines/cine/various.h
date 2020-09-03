/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef CINE_VARIOUS_H
#define CINE_VARIOUS_H


#include "common/file.h"
#include "common/keyboard.h"

#include "cine/cine.h"

namespace Cine {

#define kMaxSavegames 100
#define kMaxOrigUiSavegames 20 // 20 fit on screen using original save/load interface

// Maximum size of the command buffer including the trailing zero
#define kMaxCommandBufferSize 80

void initLanguage(Common::Language lang);

int16 makeMenuChoice(const CommandeType commandList[], uint16 height, uint16 X, uint16 Y, uint16 width, int minY = 0, bool recheckValue = false, bool allowEmpty = false);
void makeCommandLine();
void makeFWCommandLine();
void makeOSCommandLine();
void makeActionMenu();
void waitPlayerInput();
void setTextWindow(uint16 param1, uint16 param2, uint16 param3, uint16 param4);

extern int16 disableSystemMenu;
extern bool inMenu;
extern bool runOnlyUntilFreePartRangeFirst200;

extern CommandeType currentSaveName[kMaxSavegames];

struct SeqListElement {
	int16 var4;
	uint16 objIdx; ///< Is this really unsigned?
	int16 var8;
	int16 frame;
	int16 varC;
	int16 varE;
	int16 var10;
	int16 var12;
	int16 var14;
	int16 var16;
	int16 var18;
	int16 var1A;
	int16 var1C;
	int16 var1E;
};

extern uint16 var2;
extern uint16 var3;
extern uint16 var4;
extern uint16 lastType20OverlayBgIdx;
extern uint16 reloadBgPalOnNextFlip;
extern uint16 forbidBgPalReload;
extern uint16 gfxFadeOutCompleted;
extern uint16 gfxFadeInRequested;
extern uint32 safeControlsLastAccessedMs; ///< Time in milliseconds when safe controls were last accessed.
extern int16 lastSafeControlObjIdx; ///< Object index of the last safe control accessed.
extern int16 commandVar1;
extern int16 commandVar2;
extern int16 commandVar3[4];

extern char currentDatName[30];
extern uint16 musicIsPlaying;

extern uint16 errorVar;
extern byte menuVar;

extern uint16 allowPlayerInput;

extern uint16 checkForPendingDataLoadSwitch;

extern uint16 isDrawCommandEnabled;
extern uint16 waitForPlayerClick;
extern uint16 menuCommandLen;
extern bool _paletteNeedUpdate;
extern uint16 _messageLen;

extern int16 playerCommand;

extern char currentPrcName[20];
extern char currentRelName[20];
extern char currentObjectName[20];
extern char currentMsgName[20];
extern char newPrcName[20];
extern char newRelName[20];
extern char newObjectName[20];
extern char newMsgName[20];

extern char currentCtName[15];
extern char currentPartName[15];

void stopSample();
void stopMusicAfterFadeOut();
void playerCommandMouseLeftRightUp(uint16 mouseX, uint16 mouseY);
uint16 executePlayerInput();

void drawOverlays();

extern uint16 mouseUpdateStatus;
extern uint16 dummyU16;

void getMouseData(uint16 param, uint16 *pButton, uint16 *pX, uint16 *pY);

uint16 processKeyboard(uint16 param);

void mainLoopSub6();

void checkForPendingDataLoad();

void hideMouse();

void removeExtention(char *dest, const char *source);

struct SelectedObjStruct {
	int16 idx;
	int16 param;
};

#define NUM_MAX_ZONE 16

void addMessage(byte param1, int16 param2, int16 param3, int16 param4, int16 param5);

void removeMessages();

void removeSeq(uint16 param1, uint16 param2, uint16 param3);
bool isSeqRunning(uint16 param1, uint16 param2, uint16 param3);
void addSeqListElement(uint16 objIdx, int16 param1, int16 param2, int16 frame, int16 param4, int16 param5, int16 param6, int16 param7, int16 param8);
void modifySeqListElement(uint16 objIdx, int16 var4Test, int16 param1, int16 param2, int16 param3, int16 param4);
void processSeqList();

void resetGfxEntityEntry(uint16 objIdx);

bool makeTextEntryMenu(const char *caption, char *string, int strLen, int y);
void moveUsingKeyboard(int x, int y);
int16 getObjectUnderCursor(uint16 x, uint16 y);

} // End of namespace Cine

#endif
