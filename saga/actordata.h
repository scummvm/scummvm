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

enum {
	kProtagonist	= 0x01, // Actor is protagonist
	kFollower	= 0x02, // Actor is follower
	kCycle		= 0x04, // Actor stand has a cycle
	kFaster		= 0x08, // Actor is fast
	kFastest	= 0x10, // Actor is faster
	kExtended	= 0x20  // Actor uses extended sprites
};

// TODO: This doesn't quite correspond to the original Actor struct, so I'm not
// sure if I got it right.

struct R_ACTORTABLE {
	byte type;		// Always 1 (remove this?)
	byte flags;
	byte name_index;
	int32 scene_index;
	int16 x;
	int16 y;
	int16 z;
	int32 spritelist_rn;
	int32 spriteindex_rn;
	byte script_rn;
	byte color;
	byte action;
	byte facing_dir;
	byte action_dir;
};

#define R_ACTORCOUNT 181

extern int ActorOrientationLUT[];
extern R_ACTORTABLE ActorTable[R_ACTORCOUNT];

} // End of namespace Saga

#endif
