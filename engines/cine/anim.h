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

extern u16 frameVar0;

struct animHeaderStruct {
	u8 field_0;
	u8 field_1;
	u8 field_2;
	u8 field_3;
	u16 frameWidth;
	u16 frameHeight;
	u8 field_8;
	u8 field_9;
	u8 field_A;
	u8 field_B;
	u8 field_C;
	u8 field_D;
	u16 numFrames;
	u8 field_10;
	u8 field_11;
	u8 field_12;
	u8 field_13;
	u16 field_14;
};

typedef struct animHeaderStruct animHeaderStruct;

struct animDataEntry {
	char name[9];
	u8 param;
};

typedef struct animDataEntry animDataEntry;

extern animDataEntry animData[];

void loadResource(char *animName);
void loadAbs(char *resourceName, u16 idx);
void loadResourcesFromSave();

#endif
