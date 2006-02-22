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

#ifndef CINE_SCRIPT_H_
#define CINE_SCRIPT_H_

#define SCRIPT_STACK_SIZE 50

typedef struct {
	byte *ptr;
	u16 var4;
	s16 stack[SCRIPT_STACK_SIZE];
} scriptStruct;

#define NUM_MAX_SCRIPT 50

extern scriptStruct scriptTable[NUM_MAX_SCRIPT];

void computeScriptStack(byte *scriptPtr, s16 *stackPtr, u16 scriptSize);
void decompileScript(byte *scriptPtr, s16 *stackPtr, u16 scriptSize,
					 u16 scriptIdx);
void dumpScript(char *dumpName);

#define OP_loadPart                     0x3F
#define OP_loadNewPrcName               0x41
#define OP_requestCheckPendingDataLoad  0x42
#define OP_endScript                    0x50
#define OP_changeDataDisk               0x6B
#define OP_79                           0x79

void addScriptToList0(u16 idx);
s16 checkCollision(s16 objIdx, s16 x, s16 y, s16 numZones, s16 zoneIdx);

void runObjectScript(s16 entryIdx);
s16 stopObjectScript(s16 entryIdx);

void executeList1(void);
void executeList0(void);

void purgeList1(void);
void purgeList0(void);

#endif
