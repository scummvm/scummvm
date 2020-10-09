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
