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

#ifndef STARK_MOVEMENT_TURN_H
#define STARK_MOVEMENT_TURN_H

#include "engines/stark/movement/movement.h"

namespace Stark {

namespace Resources {
class FloorPositionedItem;
}

/**
 * Make an item turn on itself towards a target direction
 */
class Turn : public Movement {
public:
	Turn(Resources::FloorPositionedItem *item);
	virtual ~Turn();

	// Movement API
	void onGameLoop() override;
	uint32 getType() const override;
	void saveLoad(ResourceSerializer *serializer) override;

	/** Set the direction to turn towards */
	void setTargetDirection(const Math::Vector3d &direction);

	/** Override the default rotation speed */
	void setSpeed(float speed);

private:
	float getAngularSpeed() const;

	Resources::FloorPositionedItem *_item3D;
	Math::Vector3d _targetDirection;
	float _turnSpeed;
};

} // End of namespace Stark

#endif // STARK_MOVEMENT_TURN_H
