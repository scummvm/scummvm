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

#ifndef CINE_OBJECT_H_
#define CINE_OBJECT_H_

typedef struct {
	s16 x;
	s16 y;
	u16 mask;
	s16 frame;
	s16 costume;
	char name[20];
	u16 part;
} objectStruct;

struct overlayHeadElement {
	struct overlayHeadElement *next;
	struct overlayHeadElement *previous;
	u16 objIdx;
	u16 type;
	s16 x;
	s16 y;
	s16 var10;
	s16 var12;
};

typedef struct overlayHeadElement overlayHeadElement;

#define NUM_MAX_OBJECT 255
#define NUM_MAX_OBJECTDATA 255

extern objectStruct objectTable[NUM_MAX_OBJECT];
extern u16 globalVars[NUM_MAX_OBJECTDATA];

extern overlayHeadElement overlayHead;

void unloadAllMasks(void);
void resetMessageHead(void);

void loadObject(char *pObjectName);
void setupObject(u8 objIdx, u16 param1, u16 param2, u16 param3, u16 param4);
void modifyObjectParam(u8 objIdx, u8 paramIdx, s16 newValue);

void loadOverlayElement(u16 objIdx, u16 param);
s8 removeOverlayElement(u16 objIdx, u16 param);

s16 getObjectParam(u16 objIdx, u16 paramIdx);
s16 freeOverlay(u16 objIdx, u16 param);

void addObjectParam(u8 objIdx, u8 paramIdx, s16 newValue);
void subObjectParam(u8 objIdx, u8 paramIdx, s16 newValue);
u8 compareObjectParam(u8 objIdx, u8 param1, s16 param2);

#endif
