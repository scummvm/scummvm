/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef QUEENSTRUCTS_H
#define QUEENSTRUCTS_H

namespace Queen {


struct Box {
	uint16 x1, y1, x2, y2;

	bool intersects(uint16 x, uint16 y, uint16 w, uint16 h) const {
		return (x + w > x1) && (y + h > y1) && (x <= x2) && (y <= y2);
	}
};


struct Area {
	int16 mapNeighbours;
	Box box;
	uint16 bottomScaleFactor;
	uint16 topScaleFactor;
	uint16 object;
};


struct WalkOffData {
	int16 entryObj;
	uint16 x;
	uint16 y;
};


struct GraphicData {
	uint16 x, y;
	int16 firstFrame, lastFrame;
	uint16 speed;
};


struct ObjectData {
	int16 name;
	uint16 x;
	uint16 y;
	uint16 description;
	int16 entryObj;
	uint16 room;
	int16 state;
	int16 image;
};


struct ObjectDescription {
	uint16 field1;
	uint16 field2;
	uint16 field3;
	uint16 field4;
};


struct ItemData {
	int16 name;
	int16 description;
	int16 state;
	uint16 bobFrame;
	uint16 sfxDescription;
};


struct ActorData {
	int16 room;
	int16 bobNum;
	uint16 name;
	uint16 gameStateSlot;
	uint16 gameStateValue;
	uint16 color;
	uint16 bobFrameStanding;
	uint16 x;
	uint16 y;
	uint16 anim;
	uint16 bankNum;
	uint16 actorFile;
};


} // End of namespace Queen

#endif
