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

namespace Cine {

extern uint16 frameVar0;

struct animHeaderStruct {
	byte field_0;
	byte field_1;
	byte field_2;
	byte field_3;
	uint16 frameWidth;
	uint16 frameHeight;
	byte field_8;
	byte field_9;
	byte field_A;
	byte field_B;
	byte field_C;
	byte field_D;
	uint16 numFrames;
	byte field_10;
	byte field_11;
	byte field_12;
	byte field_13;
	uint16 field_14;
};

struct animDataEntry {
	char name[9];
	byte color;
};

extern animDataEntry animData[];

void loadResource(const char *animName);
void loadAbs(const char *resourceName, uint16 idx);
void loadResourcesFromSave();

} // End of namespace Cine

#endif
