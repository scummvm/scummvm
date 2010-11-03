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

#ifndef ASYLUM_ACTOR_H
#define ASYLUM_ACTOR_H

#include "asylum/respack.h"
#include "asylum/shared.h"

#include "common/rect.h"

namespace Asylum {

class ActionArea;
class AsylumEngine;
struct GraphicFrame;
class GraphicResource;
class Screen;

class Actor {
public:
	Actor(AsylumEngine *engine, ActorIndex index);
	virtual ~Actor();

	//////////////////////////////////////////////////////////////////////////
	// Public variables & accessors
	//////////////////////////////////////////////////////////////////////////
	int32  x;
	int32  y;
	int32  x1;
	int32  y1;
	int32  x2;
	int32  y2;
	int32 flags;
	int32 actionType; // ActionType enum value


	void setActionIndex2(int32 index) { _actionIdx2 = index; }
	void setBarrierIndex(int32 index) { _barrierIndex = index; }
	void setDirection(ActorDirection direction) { _direction = direction; }
	void setFrameCount(int32 count) { _frameCount = count; }
	void setFrameIndex(int32 number) { _frameIndex = number; }
	void setLastScreenUpdate(int32 tick) { _lastScreenUpdate = tick; }
	void setNumberFlag01(int32 number) { _numberFlag01 = number; }
	void setPriority(int32 priority) { _priority = priority; }
	void setResourceId(ResourceId id) { _resourceId = id; }
	void setStatus(ActorStatus status) { _status = status; }

	void setField638(int32 val) { _field_638 = val; }
	void setField934(int32 val) { _field_934 = val; }
	void setField938(int32 val) { _field_938 = val; }
	void setField944(int32 val) { _field_944 = val; }
	void setField96C(int32 val) { _field_96C = val; }

	int32          getActionIndex3() { return _actionIdx3; }
	Common::Rect  *getBoundingRect() { return &_boundingRect; }
	ActorDirection getDirection() { return _direction; }
	uint32         getFrameCount() { return _frameCount; }
	uint32         getFrameIndex() { return _frameIndex; }
	char          *getName() { return (char *)&_name; }
	int32          getNumberValue01() { return _numberValue01; }
	int32          getReaction(uint32 index) { return _reaction[index]; }
	ResourceId     getResourceId() { return _resourceId; }
	ResourceId     getResourcesId(uint32 index) { return _graphicResourceIds[index]; }
	ResourceId     getSoundResourceId() { return _soundResourceId; }
	ActorStatus    getStatus()    { return _status; }

	int32          getField638() { return _field_638; }
	int32          getField934() { return _field_934; }
	int32          getField944() { return _field_944; }
	int32          getField96C() { return _field_96C; }
	int32          getField974() { return _field_974; }
	int32          getField980() { return _field_980; }
	int32          getPriority() { return _priority; }

	/////////////////////////////////////////////////////////////////////////
	// Data
	/////////////////////////////////////////////////////////////////////////

	/**
	 * Loads the actor data
	 *
	 * @param stream If non-null, the Common::SeekableReadStream to load from
	 */
	void load(Common::SeekableReadStream *stream);

	/////////////////////////////////////////////////////////////////////////
	// Visibility
	/////////////////////////////////////////////////////////////////////////

	/**
	 * Query if this actor is visible.
	 *
	 * @return true if visible, false if not.
	 */
	bool isVisible() { return flags & kActorFlagVisible; }

	/**
	 * Sets actor visibility
	 *
	 * @param value true to set to visible, false to set to hidden.
	 */
	void setVisible(bool value);

	/////////////////////////////////////////////////////////////////////////
	// Drawing & update
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Draws the actor
	 */
	void draw();

	/**
	 * Updates the actor.
	 */
	void update();

	/**
	 * Updates the actor status.
	 *
	 * @param status The status.
	 */
	void updateStatus(ActorStatus status);

	/////////////////////////////////////////////////////////////////////////
	// Direction & position
	/////////////////////////////////////////////////////////////////////////

	/**
	 * Updates the actor direction.
	 */
	void updateDirection();

	/**
	 * Updates resource Id using the actor direction.
	 *
	 * @param direction The direction.
	 */
	void updateFromDirection(ActorDirection direction);

	/**
	 * Face a target from a certain direction
	 *
	 * @param target Identifier for the target.
	 * @param from   direction to face from.
	 */
	void faceTarget(int32 target, DirectionFrom from);

	/**
	 * Initialize the x1/y1 values of the actor, update the active animation frame and, if the current direction isn't 8, update the actor's direction.
	 *
	 * @param newX 		   The new x coordinate.
	 * @param newY 		   The new y coordinate.
	 * @param newDirection The new direction.
	 * @param frame 	   The frame.
	 */
	void setPosition(int32 newX, int32 newY, int32 newDirection, int32 frame);

	/////////////////////////////////////////////////////////////////////////
	// Misc
	/////////////////////////////////////////////////////////////////////////

	/**
	 * Stop the actor related sounds
	 */
	void stopSound();

	/**
	 * Clears actor data fields (TODO what are those fields?)
	 */
	void clearFields() { memset(&_field_970, 0, 52); }

	/**
	 * TEMPORARY: Initialize the 500 byte resource index from the scene
	 * file (at offset 0xA73B6).
	 *
	 * TODO remove this or add it in the right place
	 */
	void setRawResources(uint8* data);

	// Unknown methods
	bool process(int32 actorX, int32 actorY);
	void processStatus(int32 actorX, int32 actorY, bool doSpeech);
	void process_401830(int32 field980, int32 actionAreaId, int32 field978, int field98C, int32 field990, int32 field974, int32 param8, int32 param9);
	bool process_408B20(Common::Point *point, ActorDirection direction, int count, bool hasDelta);
	void process_41BC00(int32 reactionIndex, int32 numberValue01Add);
	void process_41BCC0(int32 reactionIndex, int32 numberValue01Substract);
	bool process_41BDB0(int32 reactionIndex, bool testNumberValue01);
	void update_40DE20();

private:
	AsylumEngine *_vm;

	// Our current index
	ActorIndex _index;

	// Temporary raw resources
	int32 _resources[61];

	//////////////////////////////////////////////////////////////////////////
	// Data
	//////////////////////////////////////////////////////////////////////////
	ResourceId _resourceId;
	int32  _barrierIndex;
	uint32 _frameIndex;
	uint32 _frameCount;
	// x1, y1, x2, y2
	Common::Rect _boundingRect;
	ActorDirection _direction;
	int32  _field_3C;
	ActorStatus _status;
	int32  _field_44;
	int32  _priority;
	//flags
	int32  _field_50;
	int32  _field_54;
	int32  _field_58;
	int32  _field_5C;
	int32  _field_60;
	int32  _actionIdx3;
	// TODO field_68 till field_617
	int32  _reaction[8];
	int32  _field_638;
	int32  _walkingSound1;
	int32  _walkingSound2;
	int32  _walkingSound3;
	int32  _walkingSound4;
	int32  _field_64C;
	int32  _field_650;
	ResourceId  _graphicResourceIds[55];
	char   _name[256];
	int32  _field_830[20];
	int32  _field_880[20];
	int32  _field_8D0[20];
	int32  _actionIdx2;
	int32  _field_924;
	int32 _lastScreenUpdate;
	int32  _field_92C;
	//actionType
	int32  _field_934;
	int32  _field_938;
	ResourceId _soundResourceId; // field_93C
	int32  _numberValue01;
	int32  _field_944;
	int32  _field_948;
	int32  _field_94C;
	int32  _numberFlag01;
	int32  _numberStringWidth;
	int32  _numberStringX;
	int32  _numberStringY;
	char   _numberString01[8];
	int32  _field_964;
	int32  _field_968;
	int32  _field_96C;
	int32  _field_970;
	int32  _field_974;
	int32  _field_978;
	int32  _actionIdx1;
	int32  _field_980;
	int32  _field_984;
	int32  _field_988;
	int32  _field_98C;
	int32  _field_990;
	int32  _field_994;
	int32  _field_998;
	int32  _field_99C;
	int32  _field_9A0;

	//////////////////////////////////////////////////////////////////////////
	// Update methods
	//////////////////////////////////////////////////////////////////////////
	int32 _actorUpdateCounter;
	bool  _enableFromStatus7;

	void updateStatus3_19();
	void updateStatusEnabled();
	void updateStatus9();
	void updateStatus12_Chapter2();
	void updateStatus12_Chapter2_Actor11();
	void updateStatus12_Chapter11_Actor1();
	void updateStatus12_Chapter11();
	void updateStatus14();
	void updateStatus15_Chapter2();
	void updateStatus15_Chapter2_Player();
	void updateStatus15_Chapter2_Actor11();
	void updateStatus15_Chapter11();
	void updateStatus15_Chapter11_Player();
	void updateStatus16_Chapter2();
	void updateStatus16_Chapter11();
	void updateStatus17_Chapter2();
	void updateStatus18_Chapter2();
	void updateStatus18_Chapter2_Actor11();
	void updateStatus21();

	void updateFinish();

	//////////////////////////////////////////////////////////////////////////
	// Misc
	//////////////////////////////////////////////////////////////////////////
	void setVolume();

	//////////////////////////////////////////////////////////////////////////
	// Helper methods
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Gets a direction using the angle between the two points
	 *
	 * @param ax1 The first ax.
	 * @param ay1 The first ay.
	 * @param ax2 The second ax.
	 * @param ay2 The second ay.
	 *
	 * @return The angle.
	 */
	ActorDirection getDirection(int32 ax1, int32 ay1, int32 ax2, int32 ay2);

	/**
	 * Updates the actor graphic information
	 *
	 * @param offset The offset used to get the id from the _graphicResourceIds table
	 */
	void updateGraphicData(uint32 offset);

	/**
	 * Query if the passed direction is default direction.
	 *
	 * @param index Zero-based index of the graphic resource.
	 *
	 * @return true if default direction, false if not.
	 */
	bool isDefaultDirection(int index);

	/**
	 * Gets the graphics flags for queuing the actor graphics
	 *
	 * @return The graphics flags.
	 */
	int32 getGraphicsFlags();


}; // end of class MainActor

} // end of namespace Asylum

#endif
