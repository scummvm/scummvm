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

#ifndef CRUISE_SCRIPT_H
#define CRUISE_SCRIPT_H

namespace Cruise {

enum scriptTypeEnum {
	scriptType_MinusPROC = -20,
	scriptType_Minus30 = -30,
	scriptType_PROC = 20,
	scriptType_REL = 30
};

typedef enum scriptTypeEnum scriptTypeEnum;

struct scriptInstanceStruct {
	struct scriptInstanceStruct *nextScriptPtr;
	int16 var4;
	uint8 *var6;
	int16 varA;
	int16 scriptNumber;
	int16 overlayNumber;
	int16 sysKey;
	int16 var12;
	scriptTypeEnum type;
	int16 var16;
	int16 var18;
	int16 var1A;
////// EXTRA ! not in original code. Needed for cross platform.
	int16 bitMask;
};

typedef struct scriptInstanceStruct scriptInstanceStruct;

extern scriptInstanceStruct relHead;
extern scriptInstanceStruct procHead;
extern scriptInstanceStruct *currentScriptPtr;

void setupFuncArray(void);
uint8 getByteFromScript(void);

int removeScript(int overlay, int idx, scriptInstanceStruct * headPtr);
uint8 *attacheNewScriptToTail(int16 overlayNumber,
    scriptInstanceStruct * scriptHandlePtr, int16 param, int16 arg0,
    int16 arg1, int16 arg2, scriptTypeEnum scriptType);
void manageScripts(scriptInstanceStruct * scriptHandle);

} // End of namespace Cruise

#endif
