/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARK_MOVEMENT_WALK_H
#define STARK_MOVEMENT_WALK_H

#include "engines/stark/movement/movement.h"

#include "common/array.h"

namespace Stark {

class StringPullingPath;

namespace Resources {
class FloorPositionedItem;
}

/**
 * Make an item walk / run to its destination on the current
 * location's floor
 */
class Walk : public Movement {
public:
	Walk(Resources::FloorPositionedItem *item);
	virtual ~Walk();

	// Movement API
	void start() override;
	void stop(bool force = false) override;
	void onGameLoop() override;
	bool hasReachedDestination() const override;
	uint32 getType() const override;
	void saveLoad(ResourceSerializer *serializer) override;

	/** Set the destination */
	void setDestination(const Math::Vector3d &destination);
	void setDestinationWithoutHeight(Math::Vector3d destination);

	/** Change the destination and recompute the path */
	void changeDestination(const Math::Vector3d &destination);

	/** Set the running flag */
	void setRunning();

private:
	void doWalk();
	void doWalkCollisionSimple();
	void doWalkCollisionAvoid();

	float computeDistancePerGameLoop() const;
	float getAngularSpeed() const;

	void changeItemAnim();
	void updatePath() const;

	void queueDestinationToAvoidItem(Resources::FloorPositionedItem *item, const Math::Vector3d &destination);
	bool isItemAlreadyAvoided(Resources::FloorPositionedItem *item) const;

	static bool isPointNearPath(const Math::Vector3d &point3d, const Math::Vector3d &pathStart3d, const Math::Vector3d &pathEnd3d);

	Resources::FloorPositionedItem *_item3D;
	StringPullingPath *_path;

	Math::Vector3d _destination;
	Common::Array<Math::Vector3d> _destinations;

	Common::Array<Resources::ItemVisual *> _avoidedItems;

	bool _running;
	bool _reachedDestination;
	TurnDirection _turnDirection;

	int32 _collisionWaitTimeout;
	int32 _collisionWaitCount;
	Math::Vector3d _previousPosition;
	Math::Vector3d _currentTarget;
};

} // End of namespace Stark

#endif // STARK_MOVEMENT_WALK_H
