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

	void readFrom(byte *&ptr) {
		x1 = READ_BE_UINT16(ptr); ptr += 2;
		y1 = READ_BE_UINT16(ptr); ptr += 2;
		x2 = READ_BE_UINT16(ptr); ptr += 2;
		y2 = READ_BE_UINT16(ptr); ptr += 2;
	}

	int16 xDiff() const {
		return (int16)(x2 - x1);
	}

	int16 yDiff() const {
		return (int16)(y2 - y1);
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

	void readFrom(byte *&ptr) {
		mapNeighbours = (int16)READ_BE_UINT16(ptr); ptr += 2;
		box.readFrom(ptr);
		bottomScaleFactor = READ_BE_UINT16(ptr); ptr += 2;
		topScaleFactor = READ_BE_UINT16(ptr); ptr += 2;
		object = READ_BE_UINT16(ptr); ptr += 2;
	}

	uint16 calcScale(int16 y) const {
		uint16 dy = box.y2 - box.y1;
		int16 ds = (int16)(topScaleFactor - bottomScaleFactor);
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

	void readFrom(byte *& ptr) {
		entryObj = (int16)READ_BE_UINT16(ptr); ptr += 2;
		x = READ_BE_UINT16(ptr); ptr += 2;
		y = READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct GraphicData {
	uint16 x, y;
	int16 firstFrame, lastFrame;
	uint16 speed;

	void readFrom(byte *& ptr) {
		x = READ_BE_UINT16(ptr); ptr += 2;
		y = READ_BE_UINT16(ptr); ptr += 2;
		firstFrame = (int16)READ_BE_UINT16(ptr); ptr += 2;
		lastFrame = (int16)READ_BE_UINT16(ptr); ptr += 2;
		speed = READ_BE_UINT16(ptr); ptr += 2;
	}
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

	void readFrom(byte *& ptr) {
		name = (int16)READ_BE_UINT16(ptr); ptr += 2;
		x = READ_BE_UINT16(ptr); ptr += 2;
		y = READ_BE_UINT16(ptr); ptr += 2;
		description = READ_BE_UINT16(ptr); ptr += 2;
		entryObj = (int16)READ_BE_UINT16(ptr); ptr += 2;
		room = READ_BE_UINT16(ptr); ptr += 2;
		state = (int16)READ_BE_UINT16(ptr); ptr += 2;
		image = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct ObjectDescription {
	uint16 object;
	uint16 type; // see select.c l.75-101
	uint16 lastDescription;
	uint16 seenCount;

	void readFrom(byte *&ptr) {
		object = READ_BE_UINT16(ptr); ptr += 2;
		type = READ_BE_UINT16(ptr); ptr += 2;
		lastDescription = READ_BE_UINT16(ptr); ptr += 2;
		seenCount = READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct ItemData {
	int16 item;
	int16 description;
	int16 state;
	uint16 bobFrame;
	uint16 sfxDescription;

	void readFrom(byte *&ptr) {
		item = (int16)READ_BE_UINT16(ptr); ptr += 2;
		description = (int16)READ_BE_UINT16(ptr); ptr += 2;
		state = (int16)READ_BE_UINT16(ptr); ptr += 2;
		bobFrame = READ_BE_UINT16(ptr); ptr += 2;
		sfxDescription = READ_BE_UINT16(ptr); ptr += 2;
	}
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

	void readFrom(byte *&ptr) {
		room = (int16)READ_BE_UINT16(ptr); ptr += 2;
		bobNum = (int16)READ_BE_UINT16(ptr); ptr += 2;
		name = READ_BE_UINT16(ptr); ptr += 2;
		gameStateSlot = (int16)READ_BE_UINT16(ptr); ptr += 2;
		gameStateValue = (int16)READ_BE_UINT16(ptr); ptr += 2;
		color = READ_BE_UINT16(ptr); ptr += 2;
		bobFrameStanding = READ_BE_UINT16(ptr); ptr += 2;
		x = READ_BE_UINT16(ptr); ptr += 2;
		y = READ_BE_UINT16(ptr); ptr += 2;
		anim = READ_BE_UINT16(ptr); ptr += 2;
		bankNum = READ_BE_UINT16(ptr); ptr += 2;
		actorFile = READ_BE_UINT16(ptr); ptr += 2;
	}

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

	void readFrom(byte *&ptr) {
		verb = (int16)READ_BE_UINT16(ptr); ptr += 2;
		nounObj1 = (int16)READ_BE_UINT16(ptr); ptr += 2;
		nounObj2 = (int16)READ_BE_UINT16(ptr); ptr += 2;
		song = (int16)READ_BE_UINT16(ptr); ptr += 2;
		setAreas = READ_BE_UINT16(ptr) != 0; ptr += 2;
		setObjects = READ_BE_UINT16(ptr) != 0; ptr += 2;
		setItems = READ_BE_UINT16(ptr) != 0; ptr += 2;
		setConditions = READ_BE_UINT16(ptr) != 0; ptr += 2;
		image = (int16)READ_BE_UINT16(ptr); ptr += 2;
		specialSection = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct CmdArea {
	int16 id;
	int16 area; // <0: turn off, >0: turn on
	int16 room;

	void readFrom(byte *&ptr) {
		id = (int16)READ_BE_UINT16(ptr); ptr += 2;
		area = (int16)READ_BE_UINT16(ptr); ptr += 2;
		room = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct CmdObject {
	int16 id;
	int16 dstObj; // >0: show, <0: hide
	int16 srcObj; // >0: copy from srcObj, -1: delete dstObj

	void readFrom(byte *&ptr) {
		id = (int16)READ_BE_UINT16(ptr); ptr += 2;
		dstObj = (int16)READ_BE_UINT16(ptr); ptr += 2;
		srcObj = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct CmdInventory {
	int16 id;
	int16 dstItem; // <0: delete, >0: add
	int16 srcItem; // >0: valid

	void readFrom(byte *&ptr) {
		id = (int16)READ_BE_UINT16(ptr); ptr += 2;
		dstItem = (int16)READ_BE_UINT16(ptr); ptr += 2;
		srcItem = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct CmdGameState {
	int16 id;
	int16 gameStateSlot;
	int16 gameStateValue;
	int16 speakValue;

	void readFrom(byte *&ptr) {
		id = (int16)READ_BE_UINT16(ptr); ptr += 2;
		gameStateSlot = (int16)READ_BE_UINT16(ptr); ptr += 2;
		gameStateValue = (int16)READ_BE_UINT16(ptr); ptr += 2;
		speakValue = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct FurnitureData {
	int16 room;
	int16 gameStateValue;

	void readFrom(byte *&ptr) {
		room = (int16)READ_BE_UINT16(ptr); ptr += 2;
		gameStateValue = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct GraphicAnim {
	int16 frame1;
	int16 frame2;
	int16 frame3;

	void readFrom(byte *&ptr) {
		frame1 = (int16)READ_BE_UINT16(ptr); ptr += 2;
		frame2 = (int16)READ_BE_UINT16(ptr); ptr += 2;
		frame3 = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct Person {
	const ActorData *actor; // P_ROOM, P_BNUM, P_GAMES, P_VALUE, P_COLOR, P_STAND, P_X, P_Y
	const char *name; // P_NAMEstr
	const char *anim; // P_ANIMstr
	uint16 bobFrame; // SFRAME
	//! As the bank number may change, we can't re-use actor->bankNum
	uint16 bankNum; // P_BANK
};



} // End of namespace Queen

#endif
