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

#include "math/vector3d.h"

namespace Stark {

class StringPullingPath;

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
	void onGameLoop() override;

	/** Set the destination */
	void setDestination(const Math::Vector3d &destination);

	/** Set the running flag */
	void setRunning();

private:
	float computeDistancePerGameLoop() const;
	uint computeDirectionAngle(const Math::Vector3d &direction) const;

	void changeItemAnim();

	StringPullingPath *_path;

	Math::Vector3d _destination;

	void updatePath() const;

	bool _running;
};

} // End of namespace Stark

#endif // STARK_MOVEMENT_WALK_H
