/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Actor data table header file

#ifndef SAGA_ACTORDATA_H
#define SAGA_ACTORDATA_H

namespace Saga {

struct R_ACTORTABLE {
	byte unknown4;
	byte unknown5; 
	byte name_index;
	int32 unknown8; // 1
	int16 x; // x
	int16 y; // y
	int16 unknown10; // 4
	int32 spritelist_rn;
	int32 spriteindex_rn;
	byte unknown24; // 5
	byte color;
	byte unknown39; // 6
	byte unknown3a; // 7
	byte unknown3b; // 8
};

#define R_ACTORCOUNT 181

extern int ActorOrientationLUT[];
extern R_ACTORTABLE ActorTable[R_ACTORCOUNT];

} // End of namespace Saga

#endif
