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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MACS2_CHARACTER_H
#define MACS2_CHARACTER_H

#include "common/array.h"
#include "common/rect.h"
#include "macs2/gameobjects.h"

namespace Macs2 {

struct AnimFrame;

class Character {
private:
	Common::Point _startPosition;

	uint32 _startTime = 0;
	uint32 _duration = 0;

	// If this is set, a lerp to a location becomes picking up
	Macs2::GameObject *_pickedUpObject = nullptr;

	// Handle when the character has moved into a non-walkable area, push them out if
	// they did and return true, return false otherwise
	bool handleWalkability(Character *c);

	uint16 lookupWalkability(const Common::Point &p) const;
	bool shouldStepVerticalMotion() const;
	bool isAnimationMirrored() const;
	// Returns false if we are at the end of the path already or the path is not valid
	bool walkAlongPath();
	void startLerpTo(const Common::Point &target, uint32 duration, bool ignoreObstacles = false);
	bool isWalkable(const Common::Point &p) const;

public:
	Character();

	// Frame counter for pickup animation (runtime+0x215).
	// Increments each frame while orientation == 0x11.
	// At _pickupFrameStart: item is transferred to inventory.
	// At _pickupFrameEnd: animation ends, orientation restored.
	uint16 _pickupFrameCounter = 0;
	bool _pickupItemTransferred = false;
	bool _markedForDeletion = false;

	ObjectOrientation _previousOrientation = OrientationNone;

	// Walk state from walkAlongPath (1008:1b8f) - runtime offsets +0x00..+0x0A, +0x18, +0x33
	Common::Point _targetPosition;  // runtime[+0x00, +0x02]: next waypoint
	int16 _stepDeltaX = 0;          // runtime[+0x04]: abs(endX - startX)
	int16 _stepDeltaY = 0;          // runtime[+0x06]: abs(endY - startY)
	int16 _stepError = 0;           // runtime[+0x18]: Bresenham error accumulator
	bool _stepDirectionSet = false; // runtime[+0x33]: direction has been calculated

	Common::Array<uint16> _path;
	int16 _currentPathIndex = 0;
	Common::Point _pathFinalDestination;
	Common::Array<uint8> _pathfindingOverlay;

	Macs2::GameObject *_gameObject = nullptr;
	uint16 _motionTargetVerticalOffset = 0;
	uint16 _motionVerticalOffsetDelta = 0;
	uint16 _motionDistanceUnits = 0;
	uint16 _motionProgress = 0;
	uint16 _motionStartVerticalOffset = 0;

	bool calculatePath(Common::Point target);
	void setWalkTarget(const Common::Point &target, bool snap);
	void startPickup(Macs2::GameObject *object);
	/** Remaining walk polyline: current position, unused path nodes, final destination. */
	void getPathPolyline(Common::Array<Common::Point> &out) const;

	const Common::Point &getPosition() const;
	void setPosition(const Common::Point &newPosition);

	uint16 getVerticalOffset() const;
	bool hasPendingVerticalMotion() const;
	bool fillCurrentAnimationFrame(uint16 advanceMode, Macs2::AnimFrame &out) const;
	Macs2::AnimFrame *getCurrentPortrait(bool onRightSide = false, uint16 frameIndex = 0);

	void update();
};

void resetCharacterWalkPath(Character *character);

} // namespace Macs2

#endif
