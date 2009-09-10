/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef ASYLUM_ACTOR_H_
#define ASYLUM_ACTOR_H_

#include "asylum/respack.h"
#include "asylum/graphics.h"

namespace Asylum {

class Screen;
struct ActionArea;

// TODO investigate other actor resources (from other
// scenes) to see if the unused blocks in the actor
// definition are in fact used elsewhere
enum ActorResources {
	kSound1 = 0,
	kSound2 = 1,
	kSound3 = 2,
	kSound4 = 3,
	//kUnused = 4,	// 0
	kFlags  = 5,

	kWalkN  = 6,
	kWalkNW = 7,
	kWalkW  = 8,
	kWalkSW = 9,
	kWalkS  = 10,

	kFaceN  = 11,
	kFaceNW = 12,
	kFaceW  = 13,
	kFaceSW = 14,
	kFaceS  = 15,

	kFaceAndShakeN  = 16,
	kFaceAndShakeNW = 17,
	kFaceAndShakeW  = 18,
	kFaceAndShakeSW = 19,
	kFaceAndShakeS  = 20,

	kStretchN  = 21,
	kStretchNW = 22,
	kStretchW  = 23,
	kStretchSW = 24,
	kStretchS  = 25,

	kFidgetN  = 26,
	kFidgetNW = 27,
	kFidgetW  = 28,
	kFidgetSW = 29,
	kFidgetS  = 30,

	// These are 0
	/*
	kUnknown1 = 31,
	kUnknown2 = 32,
	kUnknown3 = 33,
	kUnknown4 = 34,
	kUnknown5 = 35,
	*/

	kPickupN  = 36,
	kPickupNW = 37,
	kPickupW  = 38,
	kPickupSW = 39,
	kPickupS  = 40,

	kOperateN  = 41,
	kOperateNW = 42,
	kOperateW  = 43,
	kOperateSW = 44,
	kOperateS  = 45,

	kOperate2N  = 46,
	kOperate2NW = 47,
	kOperate2W  = 48,
	kOperate2SW = 49,
	kOperate2S  = 50,

	// These are 0
	/*
	kUnknown6  = 51,
	kUnknown7  = 52,
	kUnknown8  = 53,
	kUnknown9  = 54,
	kUnknown10 = 55,
	kUnknown11 = 56,
	kUnknown12 = 57,
	kUnknown13 = 58,
	kUnknown14 = 59
	*/

	// Horizontally flipped animations (100 + regular anim Id)
	kWalkNE = 107,
	kWalkE  = 108,
	kWalkSE = 109,

	kFaceNE = 112,
	kFaceE  = 113,
	kFaceSE = 114
	// TODO: finish these
};

class Actor {
public:
	Actor();
	virtual ~Actor();

	/**
	 * Initialize the 500 byte resource index from the scene
	 * file (at offset 0xA73B6).
	 *
	 * TODO remove this or add it in the right place
	 */
	void setRawResources(uint8* data);

	bool visible() { return flags & 0x01; }
	void visible(bool value);
	void setPostion(uint32 targetX, uint32 targetY);
	void setDirection(int dir);

	void faceTarget(int targetId, int targetType);

	// FIXME
	// I don't really like how this is used in the scene constructor
	void setResourcePack(ResourcePack *res) { _resPack = res; }

	// OLD METHODS
	void setWalkArea(ActionArea *target); // TODO depreciate
	void setAction(int action);
	void setActionByIndex(int index);
	void drawActorAt(uint16 curX, uint16 curY);
	void drawActor();
	void walkTo(uint16 curX, uint16 curY);
	void disable(int param);

	int getCurrentAction() { return _currentAction; }

	uint32		 x;
	uint32		 y;
	uint32		 grResId;
	uint32		 field_C;
	uint32		 frameNum;
	uint32		 frameCount;
	uint32		 x1;
	uint32		 y1;
	uint32		 x2;
	uint32		 y2;
	Common::Rect boundingRect;
	uint32		 direction;
	uint32		 field_3C;
	uint32		 field_40;
	uint32		 field_44;
	uint32		 field_48;
	uint32		 flags;
	uint32		 field_50;
	uint32		 field_54;
	uint32		 field_58;
	uint32		 field_5C;
	uint32		 field_60;
	uint32		 actionIdx3;
	// TODO field_68 till field_617
	uint32		 reaction[8];
	uint32		 field_638;
	uint32		 walkingSound1;
	uint32		 walkingSound2;
	uint32		 walkingSound3;
	uint32		 walkingSound4;
	uint32		 field_64C;
	uint32		 field_650;
	uint32		 grResTable[55];
	char		 name[256];
	uint32		 field_830[20];
	uint32		 field_880[20];
	uint32		 field_8D0[20];
	uint32		 actionIdx2;
	uint32		 field_924;
	uint32		 tickValue1;
	uint32		 field_92C;
	uint32		 flags2;
	uint32		 field_934;
	uint32		 field_938;
	uint32		 soundResId; // field_93C
	uint32       field_940;
	uint32       field_944;
	// TODO field_948 till field_978
	uint32		 actionIdx1;
	// TODO field_980 till field_9A0

private:
	ResourcePack    *_resPack;
	GraphicResource *_graphic;
	uint32          _resources[61];
	uint8           _currentFrame;
	int             _currentAction;
	ActionArea 	    *_currentWalkArea;

	GraphicFrame *getFrame();

}; // end of class MainActor

} // end of namespace Asylum

#endif
