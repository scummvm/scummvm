/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef CINE_VARIOUS_H_
#define CINE_VARIOUS_H_

#include "cine/cine.h"

extern int gameType;

typedef char commandeType[20];

s16 makeMenuChoice(const commandeType commandList[], u16 height, u16 X, u16 Y,
    u16 width);
s16 makeMenuChoice2(const commandeType commandList[], u16 height, u16 X, u16 Y,
    u16 width);

extern s16 allowSystemMenu;

typedef struct {
	u8 *ptr;
	u16 len;
} unk1Struct;

#define NUM_MAX_MESSAGE 255

extern unk1Struct messageTable[NUM_MAX_MESSAGE];

struct unkListElementStruct {
	struct unkListElementStruct *next;
	s16 var4;
	s16 var6;
	s16 var8;
	s16 varA;
	s16 varC;
	s16 varE;
	s16 var10;
	s16 var12;
	s16 var14;
	s16 var16;
	s16 var18;
	s16 var1A;
	s16 var1C;
	s16 var1E;
};

typedef struct unkListElementStruct unkListElementStruct;

extern unkListElementStruct unkList;

extern u32 var6;
extern u32 var8;
extern u8 *var9;

extern u16 var2;
extern u16 var3;
extern u16 var4;
extern u16 var5;

extern Common::File palFileHandle;
extern Common::File partFileHandle;

void processPendingUpdates(s16 param);
void closeEngine3(void);
void mainLoopSub1(void);
void mainLoopSub2(u16 param1, u16 param2, u16 param3, u16 param4);

extern u16 errorVar;
extern u8 menuVar;

void gfxFuncGen1(u8 *param1, u8 *param2, u8 *param3, u8 *param4,
    s16 param5);

extern u8 *page0;
extern u8 *page0c;

void ptrGfxFunc13(void);
void gfxFuncGen2(void);

extern u16 allowPlayerInput;

extern u16 checkForPendingDataLoadSwitch;

extern u16 fadeRequired;
extern u16 isDrawCommandEnabled;
extern u16 waitForPlayerClick;
extern u16 var16;
extern u16 var17;
extern u16 var18;
extern u16 var19;
extern u16 var20;
extern u8 var21;

extern s16 playerCommand;

extern char commandBuffer[80];

extern u16 c_palette[256];

extern char currentPrcName[20];
extern char currentRelName[20];
extern char currentObjectName[20];
extern char currentMsgName[20];
extern char newPrcName[20];
extern char newRelName[20];
extern char newObjectName[20];
extern char newMsgName[20];

extern char currentBgName[8][15];
extern char currentCtName[15];
extern char currentPartName[15];

void stopSample(void);
void mainLoopSub3(void);
u16 executePlayerInput(void);

void drawOverlays(void);
void flip(void);

extern u16 mouseUpdateStatus;
extern u16 dummyU16;

void getMouseData(u16 param, u16 *pButton, u16 *pX, u16 *pY);

u16 processKeyboard(u16 param);

void mainLoopSub6(void);

void checkForPendingDataLoad(void);

extern u16 exitEngine;

void hideMouse(void);
void closeEngine7(void);

extern u16 var22;

void removeExtention(char *dest, const char *source);

struct selectedObjStruct {
	s16 idx;
	s16 param;
};

typedef struct selectedObjStruct selectedObjStruct;

extern u16 defaultMenuBoxColor;
extern u16 defaultMenuBoxColor2;

#define NUM_MAX_ZONE 16
extern u16 zoneData[NUM_MAX_ZONE];

void addMessage(u8 param1, s16 param2, s16 param3, s16 param4, s16 param5);

void blitScreen(u8 *frontBuffer, u8 *backbuffer);

struct mouseStatusStruct {
	int left;
	int right;
	int X;
	int Y;
};

typedef struct mouseStatusStruct mouseStatusStruct;

extern s16 additionalBgVScroll;

void addUnkListElement(s16 param0, s16 param1, s16 param2, s16 param3,
    s16 param4, s16 param5, s16 param6, s16 param7, s16 param8);
void resetUnkList();
void processUnkList(void);
#endif
