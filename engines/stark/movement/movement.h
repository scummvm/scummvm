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

#ifndef STARK_MOVEMENT_MOVEMENT_H
#define STARK_MOVEMENT_MOVEMENT_H

#include "math/vector3d.h"

namespace Stark {

namespace Resources {
class ItemVisual;
}

class ResourceSerializer;

/**
 * Abstract movement of an item on the current location's floor
 */
class Movement {
public:
	Movement(Resources::ItemVisual *item);
	virtual ~Movement();

	enum MovementType {
		kTypeWalk            = 1,
		kTypeFollowPath      = 2,
		kTypeFollowPathLight = 3,
		kTypeTurn            = 4
	};

	/** Movement factory */
	static Movement *construct(uint32 type, Resources::ItemVisual *item);

	/** Obtain the effective movement type */
	virtual uint32 getType() const = 0;

	/**
	 * Initiate the movement
	 */
	virtual void start();

	/**
	 * Stop / abort the movement
	 */
	virtual void stop(bool force = false);

	/**
	 * Called once per game loop
	 */
	virtual void onGameLoop() = 0;

	/**
	 * Has the movement stopped?
	 */
	bool hasEnded() const;

	/**
	 * Has the movement reached its destination successfully?
	 */
	virtual bool hasReachedDestination() const;

	/**
	 * Persist / restore the state of the movement so it can be resumed using 'start'
	 */
	virtual void saveLoad(ResourceSerializer *serializer) = 0;

protected:
	enum TurnDirection {
		kTurnNone,
		kTurnLeft,
		kTurnRight
	};

	const float _defaultTurnAngleSpeed; // Degrees per ms

	float computeAngleBetweenVectorsXYPlane(const Math::Vector3d &v1, const Math::Vector3d &v2) const;

	bool _ended;
	Resources::ItemVisual *_item;
};

} // End of namespace Stark

#endif // STARK_MOVEMENT_MOVEMENT_H
