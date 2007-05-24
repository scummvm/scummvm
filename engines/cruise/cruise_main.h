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

#ifndef CRUISE_CRUISE_MAIN_H
#define CRUISE_CRUISE_MAIN_H

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "common/stdafx.h"
#include "common/scummsys.h"

#include "cruise/overlay.h"
#include "cruise/object.h"
#include "cruise/ctp.h"
#include "cruise/actor.h"
#include "cruise/vars.h"
#include "cruise/font.h"
#include "cruise/volume.h"
#include "cruise/fontCharacterTable.h"
#include "cruise/stack.h"
#include "cruise/script.h"
#include "cruise/various.h"
#include "cruise/stringSupport.h"
#include "cruise/function.h"
#include "cruise/saveload.h"
#include "cruise/linker.h"
#include "cruise/mouse.h"
#include "cruise/gfxModule.h"
#include "cruise/dataLoader.h"
#include "cruise/perso.h"
#include "cruise/menu.h"

#include "cruise/background.h"
#include "cruise/backgroundIncrust.h"

#include "cruise/mainDraw.h"

namespace Cruise {

/*#define DUMP_SCRIPT
#define DUMP_OBJECT*/

#define ASSERT_PTR assert
#define ASSERT assert

bool delphineUnpack(byte *dst, const byte *src, int len);

ovlData3Struct *getOvlData3Entry(int32 scriptNumber, int32 param);
ovlData3Struct *scriptFunc1Sub2(int32 scriptNumber, int32 param);
int16 loadShort(void *ptr);
void resetFileEntry(int32 entryNumber);
void saveShort(void *ptr, int16 var);
void *mallocAndZero(int32 size);
uint8 *mainProc14(uint16 overlay, uint16 idx);
void printInfoBlackBox(const char *string);
void waitForPlayerInput(void);
int loadCtp(uint8 * ctpName);
void loadPakedFileToMem(int fileIdx, uint8 * buffer);
int loadScriptSub1(int scriptIdx, int param);
void resetFileEntryRange(int param1, int param2);
int getProcParam(int overlayIdx, int param2, uint8 * name);
void changeScriptParamInList(int param1, int param2,
    scriptInstanceStruct * pScriptInstance, int newValue, int param3);
uint8 *getDataFromData3(ovlData3Struct * ptr, int param);
int32 prepareWordRender(int32 param, int32 var1, int16 * out2, uint8 * ptr3,
    uint8 * string);
void removeExtention(const char *name, char *buffer);
void resetRaster(uint8 * rasterPtr, int32 rasterSize);
void changeCursor(uint16 cursorType);
void resetPtr2(scriptInstanceStruct * ptr);
void getFileExtention(const char *name, char *buffer);
void *allocAndZero(int size);
void freeStuff2(void);
char *getObjectName(int index, uint8 * string);
void mainLoop(void);

} // End of namespace Cruise

#endif
