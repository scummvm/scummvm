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

#ifndef CINE_ANIM_H_
#define CINE_ANIM_H_

extern uint16 frameVar0;

struct animHeaderStruct {
	uint8 field_0;
	uint8 field_1;
	uint8 field_2;
	uint8 field_3;
	uint16 frameWidth;
	uint16 frameHeight;
	uint8 field_8;
	uint8 field_9;
	uint8 field_A;
	uint8 field_B;
	uint8 field_C;
	uint8 field_D;
	uint16 numFrames;
	uint8 field_10;
	uint8 field_11;
	uint8 field_12;
	uint8 field_13;
	uint16 field_14;
};

typedef struct animHeaderStruct animHeaderStruct;

struct animDataEntry {
	char name[9];
	uint8 param;
};

typedef struct animDataEntry animDataEntry;

extern animDataEntry animData[];

void loadResource(char *animName);
void loadAbs(char *resourceName, uint16 idx);
void loadResourcesFromSave();

#endif
