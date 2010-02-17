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

class Scene;
class Screen;
class ActionArea;

// TODO: check if the names match the actor type
enum ActorType {
	kMax     = 0,
	kSarah   = 1,
    kCyclops = 2,
    kAztec   = 3
};

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

	bool visible() {
		return flags & 0x01;
	}

	void visible(bool value);

	/** .text:0040A260
	 * Initialize the x1/y1 values of the actor, update the active animation frame
	 * and, if the current direction isn't 8, update the actor's direction
	 */
	void setPosition(int32 newX, int32 newY, int32 newDirection, int32 frame);

	void faceTarget(int32 targetId, int32 targetType);

	/** .text:00401320
	 * TODO
	 */
	void updateActor_401320();


	// FIXME
	// I don't really like how this is used in the scene constructor
	void setResourcePack(ResourcePack *res) { _resPack = res; }

	// FIXME Hack to get a reference to the parent scene
	// into the actor instance
	void setScene(Scene *scene) { _scene = scene; }

	// OLD METHODS
	// TODO ALL of these need to be depreciated in favour
	// of the proper functions from the original
	void setWalkArea(ActionArea *target);
	void setAction(int32 action);
	void setActionByIndex(int32 index);
	void drawActorAt(int32 curX, int32 curY);
	void drawActor();
	void walkTo(int32 curX, int32 curY);

	int32 currentAction; // TODO depreciate

	int32  x;
	int32  y;
	uint32 grResId;
	int32  field_C; // BarrierIndex? Mask index?
	uint32 frameNum;
	uint32 frameCount;
	int32  x1;
	int32  y1;
	int32  x2;
	int32  y2;
	Common::Rect boundingRect;
	int32  direction;
	int32  field_3C;
	int32  updateType;
	int32  field_44;
	int32  priority;
	int32  flags;
	int32  field_50;
	int32  field_54;
	int32  field_58;
	int32  field_5C;
	int32  field_60;
	int32  actionIdx3;
	// TODO field_68 till field_617
	int32  reaction[8];
	int32  field_638;
	int32  walkingSound1;
	int32  walkingSound2;
	int32  walkingSound3;
	int32  walkingSound4;
	int32  field_64C;
	int32  field_650;
	int32  grResTable[55];
	char   name[256];
	int32  field_830[20];
	int32  field_880[20];
	int32  field_8D0[20];
	int32  actionIdx2;
	int32  field_924;
	int32  tickValue1;
	int32  field_92C;
	int32  flags2;
	int32  field_934;
	int32  field_938;
	int32  soundResId; // field_93C
	int32  numberValue01;
	int32  field_944;
	int32  field_948;
	int32  field_94C;
	int32  numberFlag01;
	int32  numberStringWidth;
	int32  numberStringX;
	int32  numberStringY;
	char   numberString01[8];
	int32  field_964;
	int32  field_968;
	int32  field_96C;
	int32  field_970;
	int32  field_974;
	int32  field_978;
	int32  actionIdx1;
	int32  field_980;
	int32  field_984;
	int32  field_988;
	int32  field_98C;
	int32  field_990;
	int32  field_994;
	int32  field_998;
	int32  field_99C;
	int32  field_9A0;

private:
	Scene *_scene;
	int32 _resources[61];

	ResourcePack    *_resPack;
	GraphicResource *_graphic;
	ActionArea      *_currentWalkArea;

	GraphicFrame *getFrame();
	int32 getAngle(int32 ax1, int32 ay1, int32 ax2, int32 ay2);

}; // end of class MainActor

} // end of namespace Asylum

#endif
