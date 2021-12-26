/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_RESOURCES_ACTOR_H
#define ASYLUM_RESOURCES_ACTOR_H

#include "common/array.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "common/stream.h"

#include "asylum/shared.h"

#include "asylum/resources/inventory.h"

namespace Asylum {

class AsylumEngine;
class GraphicResource;
class Screen;

struct ActionArea;
struct GraphicFrame;

struct ActorData : public Common::Serializable {
	uint32 count;
	int32 current;
	Common::Point points[120];
	ActorDirection directions[120];

	ActorData() {
		count = 0;
		current = 0;
		memset(&directions, 0, sizeof(directions));
	}

	virtual ~ActorData() {}

	void load(Common::SeekableReadStream *stream) {
		count = stream->readUint32LE();

		current = stream->readUint32LE();

		for (int32 i = 0; i < 120; i++) {
			points[i].x = stream->readSint32LE();
			points[i].y = stream->readSint32LE();
		}

		for (int32 i = 0; i < 120; i++)
			directions[i] = (ActorDirection)stream->readSint32LE();
	}

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s) {
		s.syncAsUint32LE(count);
		s.syncAsSint32LE(current);

		for (int32 i = 0; i < ARRAYSIZE(points); i++) {
			s.syncAsSint32LE(points[i].x);
			s.syncAsSint32LE(points[i].y);
		}

		for (int32 i = 0; i < ARRAYSIZE(directions); i++)
			s.syncAsSint32LE(directions[i]);
	}
};

class Actor : public Common::Serializable {
public:
	Actor(AsylumEngine *engine, ActorIndex index);
	virtual ~Actor() {};

	//////////////////////////////////////////////////////////////////////////
	// Public variables & accessors
	//////////////////////////////////////////////////////////////////////////
	int32 flags;
	int32 actionType; // ActionType enum value
	Inventory inventory;

	void setActionIndex2(int32 index) { _actionIdx2 = index; }
	void setObjectIndex(int32 index) { _objectIndex = index; }
	void setDirection(ActorDirection dir) { _direction = dir; }
	void setFrameCount(int32 count) { _frameCount = count; }
	void setFrameIndex(int32 number) { _frameIndex = number; }
	void setLastScreenUpdate(int32 tick) { _lastScreenUpdate = tick; }
	void setNumberFlag01(int32 number) { _numberFlag01 = number; }
	void setPriority(int32 priority) { _priority = priority; }
	void setResourceId(ResourceId id) { _resourceId = id; }
	void setSoundResourceId(ResourceId id) { _soundResourceId = id; }
	void setStatus(ActorStatus status) { _status = status; }
	void setTransparency(int32 val) { _transparency = val; }
	void setTickCount(int32 tickCount) { _tickCount = tickCount; }

	void setField934(int32 val) { _field_934 = val; }
	void setField938(int32 val) { _field_938 = val; }
	void setField944(int32 val) { _field_944 = val; }

	int32          getActionIndex3() { return _actionIdx3; }
	Common::Rect  *getBoundingRect() { return &_boundingRect; }
	ActorDirection getDirection() { return _direction; }
	uint32         getFrameCount() { return _frameCount; }
	uint32         getFrameIndex() { return _frameIndex; }
	char          *getName() { return (char *)&_name; }
	ActorIndex     getNextActorIndex() { return _nextActorIndex; }
	int32          getNumberValue01() { return _numberValue01; }
	Common::Point *getPoint() { return &_point; }
	Common::Point *getPoint1() { return &_point1; }
	Common::Point *getPoint2() { return &_point2; }
	int32          getPriority() { return _priority; }
	ResourceId     getResourceId() { return _resourceId; }
	ResourceId     getResourcesId(uint32 index) { return _graphicResourceIds[index]; }
	int32          getScriptIndex() { return _scriptIndex; }
	bool           shouldInvertPriority() { return _invertPriority; }
	ResourceId     getSoundResourceId() { return _soundResourceId; }
	ActorStatus    getStatus()    { return _status; }
	int32          getTickCount() { return _tickCount; }

	int32          getField934() { return _field_934; }
	int32          getField944() { return _field_944; }
	int32          getField948() { return _field_948; }
	int32          getField94C() { return _field_94C; }

	// For saving
	ActorData     *getData() { return &_data; }

	/////////////////////////////////////////////////////////////////////////
	// Data
	/////////////////////////////////////////////////////////////////////////

	/**
	 * Loads the actor
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
	 * Query if this actor is on screen.
	 *
	 * @return true if on screen, false if not.
	 */
	bool isOnScreen();

	/**
	 * Shows this actor.
	 */
	void show() { setVisible(true); }

	/**
	 * Hides this actor.
	 */
	void hide() { setVisible(false); }

	/////////////////////////////////////////////////////////////////////////
	// Drawing & update
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Draws the actor
	 */
	void draw();

	/**
	 * Draw number text (called on scene drawing)
	 */
	void drawNumber();

	/**
	 * Updates the actor.
	 */
	void update();

	/**
	 * Enables the actor
	 */
	void enable() { changeStatus(kActorStatusEnabled); }

	/**
	 * Changes the actor status.
	 *
	 * @param status The status.
	 */
	void changeStatus(ActorStatus status);

	/////////////////////////////////////////////////////////////////////////
	// Direction & position
	/////////////////////////////////////////////////////////////////////////

	/**
	 * Updates the actor's mirror image.
	 */
	void updateReflectionData();

	/**
	 * Updates resource Id using the actor direction.
	 *
	 * @param direction The direction.
	 */
	void changeDirection(ActorDirection direction);

	/**
	 * Face a target from a certain direction
	 *
	 * @param target Identifier for the target.
	 * @param from   direction to face from.
	 */
	void faceTarget(uint32 target, DirectionFrom from);

	/**
	 * Initialize the x1/y1 values of the actor, update the active animation frame and, if the current direction isn't 8, update the actor's direction.
	 *
	 * @param newX 		   The new x coordinate.
	 * @param newY 		   The new y coordinate.
	 * @param newDirection The new direction.
	 * @param frame 	   The frame.
	 */
	void setPosition(int16 newX, int16 newY, ActorDirection newDirection, uint32 frame);

	/**
	 * Query if a graphic resource is present.
	 *
	 * @param index Zero-based index of the graphic resource.
	 *
	 * @return true if the graphic resource is present.
	 */
	bool canChangeStatus(int index) const;

	/**
	 * Adjust coordinates.
	 *
	 * @param point The point.
	 */
	void adjustCoordinates(Common::Point *point);

	/////////////////////////////////////////////////////////////////////////
	// Misc
	/////////////////////////////////////////////////////////////////////////

	/**
	 * Stop the actor related sounds
	 */
	void stopSound();

	/**
	 * Convert this object into a string representation.
	 *
	 * @param shortString toggle whether to output a summary or
	 *                    detailed view of the actor object
	 * @return A string representation of this object.
	 */
	Common::String toString(bool shortString = true);

	/**
	 * Clears actor data fields
	 */
	void clearFields();

	bool canReach(const Common::Point &point);
	void forceTo(int16 actorX, int16 actorY, bool doSpeech);
	void setupReflectionData(ActorIndex nextActor, int32 actionAreaId, ActorDirection nextDirection, const Common::Point &nextPosition, bool invertPriority, const Common::Point &nextPositionOffset);
	bool aNicePlaceToTalk(Common::Point *point, int32 *param);
	bool canMove(Common::Point *point, ActorDirection direction, uint32 count, bool hasDelta);
	void move(ActorDirection dir, uint32 distance);
	bool testActorCollision(Common::Point *point, ActorDirection direction);
	void drawInventory();
	void stopWalking();

	/**
	 * Query if the object resource is present in the resource table between indices 10 & 20
	 *
	 * @return true if resource present between 15 & 20, false if not.
	 */
	bool checkBoredStatus() const;

	//////////////////////////////////////////////////////////////////////////
	// Static update methods
	//////////////////////////////////////////////////////////////////////////
	static void crowsReturn(AsylumEngine *engine);

	/**
	 * Updates the player appearance in chapter 9.
	 *
	 * @param [in,out] engine If non-null, the engine.
	 * @param nextPlayer 	  The next player index
	 */
	static void morphInto(AsylumEngine *engine, int nextPlayer);

	/**
	 * Gets a direction using the angle between the two vectors.
	 *
	 * @param vec1 The first vector.
	 * @param vec2 The second vector.
	 *
	 * @return The direction
	 */
	static ActorDirection getAngle(const Common::Point &vec1, const Common::Point &vec2);

	/**
	 * Gets the euclidean distance between two points.
	 *
	 * @param point1 The first point.
	 * @param point2 The second point.
	 *
	 * @return the distance.
	 */
	static uint32 euclidianDistance(const Common::Point &point1, const Common::Point &point2);

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s);

private:
	AsylumEngine *_vm;

	// Our current index
	ActorIndex _index;

	//////////////////////////////////////////////////////////////////////////
	// Data
	//////////////////////////////////////////////////////////////////////////
	Common::Point _point;
	ResourceId _resourceId;
	int32  _objectIndex;
	uint32 _frameIndex;
	uint32 _frameCount;
	Common::Point _point1;
	Common::Point _point2;
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
	ResourceId _walkingSound1;
	ResourceId _walkingSound2;
	ResourceId _walkingSound3;
	ResourceId _walkingSound4;
	uint32  _field_64C;
	uint32  _field_650;
	ResourceId  _graphicResourceIds[55];
	char   _name[256];
	int32  _distancesEO[20];
	int32  _distancesNS[20];
	int32  _distancesNSEO[20];
	int32  _actionIdx2;
	int32  _field_924;
	uint32 _lastScreenUpdate;
	int32  _scriptIndex;
	//actionType
	int32  _field_934;
	int32  _field_938;
	ResourceId _soundResourceId; // field_93C
	int32  _numberValue01;
	int32  _field_944; // has collision ?!
	int32  _field_948;
	int32  _field_94C;
	int32  _numberFlag01;
	int16  _numberStringWidth;
	Common::Point _numberPoint;
	char   _numberString01[8];
	int32  _field_968;
	int32  _transparency;
	bool   _processNewDirection;
	bool   _invertPriority;
	ActorDirection _nextDirection;
	int32  _nextActionIndex;
	ActorIndex _nextActorIndex;
	Common::Point _nextPositionOffset;
	Common::Point _nextPosition;
	int32  _field_994;
	int32  _field_998;
	int32  _field_99C;
	int32  _field_9A0;

	//////////////////////////////////////////////////////////////////////////
	// Actor data
	//////////////////////////////////////////////////////////////////////////
	ActorData _data;

	int32 _tickCount;

	uint32 _updateCounter;

	//////////////////////////////////////////////////////////////////////////
	// Update methods
	//////////////////////////////////////////////////////////////////////////
	void updateStatusInteracting();
	void checkPumpkinDeath();
	void updatePumpkin(GameFlag flagToCheck, GameFlag flagToSet, ObjectId objectToUpdate, ObjectId objectToDisable);

	void updateStatusEnabled();
	void updateStatusEnabledProcessStatus(int16 testX, int16 testY, uint32 counter, int16 setX, int16 setY);

	void updateStatusBored();

	void CrowClosesIn();
	void ScareCrowClosesIn();
	void TentacleRises();

	void updateStatusEnabled2();
	void CrowHoveringBeforeKill();
	void CrowStatusQuo();
	void TentacleWigglesForSarah();

	void CrowDives();
	void MaxGetsHit();
	void MaxAttacks();
	void checkScareCrowDeath();
	bool checkCrowDeath();
	void ScareCrowAttacks();
	bool actorsIntersect(ActorIndex actorIndex1, ActorIndex actorIndex2);
	void TentacleWhips();
	void SarahAttacks();

	void MaxGetsSome();
	void SarahGetsSome();

	void TentacleDies();

	void CrowSwoops();
	void ScareCrowRetreats();

	void updateStatusMorphing();

	void actionAreaCheck();

	//////////////////////////////////////////////////////////////////////////
	// Path finding functions
	//////////////////////////////////////////////////////////////////////////
	uint32 _frameNumber;
	bool findLeftPath(Common::Point source, const Common::Point &destination, Common::Array<int> *actions);
	bool findRightPath(Common::Point source,  const Common::Point &destination, Common::Array<int> *actions);
	bool findUpPath(Common::Point source,  const Common::Point &destination, Common::Array<int> *actions);
	bool findDownPath(Common::Point source, const Common::Point &destination, Common::Array<int> *actions);
	bool tryDirection(const Common::Point &source, Common::Array<int> *actions, Common::Point *point, ActorDirection direction, const Common::Point &destination, bool *flag);
	bool canGetToDest(Common::Array<int> *actions, const Common::Point &point, ActorDirection direction, int16 loopcount);
	bool testPolyInLink(const Common::Point &pt, Common::Array<int> *actions);

	//////////////////////////////////////////////////////////////////////////
	// Misc
	//////////////////////////////////////////////////////////////////////////

 	/**
	 * Sets actor visibility
	 *
	 * @param value true to set to visible, false to set to hidden.
	 */
	void setVisible(bool value);

	/**
	 * Sets the volume.
	 */
	void setVolume();

	void TentacleBlocksSarah(const Common::Point &vec1, Common::Point vec2);
	void SarahDies();

	/**
	 * Updates the actor "number" data if the item is "1".
	 *
	 * @param item     The item.
	 * @param point    The coordinates
	 */
	void updateNumbers(uint item, const Common::Point &point);

	/**
	 * Determine if the supplied point is in the action area
	 *
	 * @param pt 			The point.
	 * @param [in,out] area If non-null, the area.
	 *
	 * @return true if in the action area, false otherwise
	 */
	bool isInActionArea(const Common::Point &pt, ActionArea *area);

	//////////////////////////////////////////////////////////////////////////
	// Helper methods
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Updates the actor graphic information
	 *
	 * @param offset The offset used to get the id from the _graphicResourceIds table
	 */
	void updateGraphicData(uint32 offset);

	/**
	 * Gets the graphics flag for queuing the actor graphics (mirrored or normal)
	 *
	 * @return The graphics flag.
	 */
	DrawFlags getGraphicsFlags();

	/**
	 * Gets the absolute value of the walk increment for a frame.
	 *
	 * @param direction  The direction.
	 * @param frameIndex Zero-based index of the frame.
	 *
	 * @return The absolute value of the walk increment.
	 */
	int32 getStride(ActorDirection direction, uint32 frameIndex) const;

	/**
	 * Gets the walk increment for a frame.
	 *
	 * @param direction  The direction.
	 * @param frameIndex Zero-based index of the frame.
	 *
	 * @return The walk increment.
	 */
	int32 getWalkIncrement(ActorDirection direction, uint32 frameIndex) const;

	/**
	 * Updates the coordinates depending on the direction.
	 *
	 * @param direction 	 The direction.
	 * @param delta 		 The delta.
	 * @param [in,out] point If non-null, the point.
	 */
	static void incPosition(ActorDirection direction, int16 delta, Common::Point *point);

	/**
	 * Get the angle between the two vectors
	 *
	 * @param p1 The first vector.
	 * @param p2 The second vector.
	 *
	 * @return the angle
	 */
	static int32 getAngleOfVector(const Common::Point &vec1, const Common::Point &vec2);

	/**
	 * Computes the Scare Crow's strike zone.
	 *
	 * @param rect          The strike zone.
	 * @param direction 	The direction.
	 * @param point 		The point.
	 */
	static void getCrowStrikeZone(Common::Rect *rect, ActorDirection direction, const Common::Point &point);

	/**
	 * Determines the direction of the shortest rotation between two vectors.
	 *
	 * @param vec1 The first vector.
	 * @param vec2 The second vector.
	 *
	 * @return true if CCW, false if CW.
	 */
	static bool determineLeftOrRight(const Common::Point &vec1, const Common::Point &vec2);

	/**
	 * Gets the adjustment for the X-coordinate of the supplied point.
	 *
	 * @param rect  The rectangle.
	 * @param point The test point.
	 *
	 * @return value depending on the horizontal position of the point relative to the rectangle.
	 */
	static int16 pointInRectXAdjust(const Common::Rect &rect, const Common::Point &point);

	/**
	 * Gets the adjustment for the Y-coordinate of the supplied point.
	 *
	 * @param rect  The rectangle.
	 * @param point The test point.
	 *
	 * @return value depending on the vertical position of the point relative to the rectangle.
	 */
	static int16 pointInRectYAdjust(const Common::Rect &rect, const Common::Point &point);

}; // end of class MainActor

} // end of namespace Asylum

#endif // ASYLUM_RESOURCES_ACTOR_H
