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

	int16 xDiff() const {
		return (int16)(x1 - x2);
	}

	int16 yDiff() const {
		return (int16)(y1 - y2);
	}

	bool intersects(uint16 x, uint16 y, uint16 w, uint16 h) const {
		return (x + w > x1) && (y + h > y1) && (x <= x2) && (y <= y2);
	}

	bool contains(uint16 x, uint16 y) const {
		return (x >= x1) && (x <= x2) && (y >= y1) && (y <= y2);
	}
};


struct Area {
	int16 mapNeighbours;
	Box box;
	uint16 bottomScaleFactor;
	uint16 topScaleFactor;
	uint16 object;

	uint16 calcScale(int16 y) const {
		uint16 dy = box.y2 - box.y1;
		uint16 ds = topScaleFactor - bottomScaleFactor;
		uint16 scale = ((((y - box.y1) * 100) / dy) * ds) / 100 + bottomScaleFactor;
		if (scale == 0) {
			scale = 100;
		}
		return scale;
	}

	int16 scaleDiff() const {
		return (int16)(topScaleFactor - bottomScaleFactor);
	}
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
	int16 name; // FIXME: rename to 'object'
	uint16 x;
	uint16 y;
	uint16 description;
	int16 entryObj;
	uint16 room;
	int16 state;
	int16 image;
};


struct ObjectDescription {
	uint16 object;
	uint16 type; // see select.c l.75-101
	uint16 lastDescription;
	uint16 seenCount;
};


struct ItemData {
	int16 item;
	int16 description;
	int16 state;
	uint16 bobFrame;
	uint16 sfxDescription;
};


struct ActorData {
	int16 room;
	int16 bobNum;
	uint16 name;
	int16 gameStateSlot;
	int16 gameStateValue;
	uint16 color;
	uint16 bobFrameStanding;
	uint16 x;
	uint16 y;
	uint16 anim;
	uint16 bankNum;
	uint16 actorFile;
};


struct CmdListData {
	int16 verb;
	int16 nounObj1;
	int16 nounObj2;
	int16 song; // >0: playbefore, <0: playafter
	bool setAreas;
	bool setObjects;
	bool setItems;
	bool setConditions;
	int16 image;
	int16 specialSection; // see execute.c l.423-451
};


struct CmdArea {
	int16 id;
	int16 area; // <0: turn off, >0: turn on
	int16 room;
};


struct CmdObject {
	int16 id;
	int16 dstObj; // >0: show, <0: hide
	int16 srcObj; // >0: copy from srcObj, -1: delete dstObj
};


struct CmdInventory {
	int16 id;
	int16 dstItem; // <0: delete, >0: add
	int16 srcItem; // >0: valid
};


struct CmdGameState {
	int16 id;
	int16 gameStateSlot;
	int16 gameStateValue;
	int16 speakValue;
};


struct FurnitureData {
	int16 room;
	int16 gameStateValue;
};


struct GraphicAnim {
	int16 frame1;
	int16 frame2;
	int16 frame3;
};


} // End of namespace Queen

#endif
