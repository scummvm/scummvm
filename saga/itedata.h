/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

#ifndef SAGA_ITEDATA_H
#define SAGA_ITEDATA_H

namespace Saga {

enum ActorFlags {
	kProtagonist	= 0x01,	// Actor is protagonist
	kFollower		= 0x02,	// Actor is follower
	kCycle			= 0x04, // Actor stand has a cycle
	kFaster			= 0x08, // Actor is fast
	kFastest		= 0x10, // Actor is faster
	kExtended		= 0x20  // Actor uses extended sprites
};

// TODO: This doesn't quite correspond to the original Actor struct, so I'm not
// sure if I got it right.

struct ActorTableData {
	byte flags;
	byte nameIndex;
	int32 sceneIndex;
	int16 x;
	int16 y;
	int16 z;
	int32 spriteListResourceId;
	int32 frameListResourceId;
	byte scriptEntrypointNumber;
	byte speechColor;
	byte currentAction;
	byte facingDirection;
	byte actionDirection;
};

#define ITE_ACTORCOUNT 181

extern ActorTableData ITE_ActorTable[ITE_ACTORCOUNT];

enum {
	kObjUseWith = 0x01,
	kObjNotFlat = 0x02
};

struct ObjectTableData {
	byte nameIndex;
	int32 sceneIndex;
	int16 x;
	int16 y;
	int16 z;
	int32 spriteListResourceId;
	byte scriptEntrypointNumber;
	uint16 interactBits;
};

struct FxTable {
	int res;
	int vol;
};

#define ITE_OBJECTCOUNT 39
#define ITE_SFXCOUNT 63

extern ObjectTableData ITE_ObjectTable[ITE_OBJECTCOUNT];
extern FxTable ITE_SfxTable[ITE_SFXCOUNT];

extern const char *ITEinterfaceTextStrings[][45];

} // End of namespace Saga

#endif
