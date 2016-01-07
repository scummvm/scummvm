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

#ifndef STARK_MOVEMENT_STRING_PULLING_PATH_H
#define STARK_MOVEMENT_STRING_PULLING_PATH_H

#include "common/array.h"

#include "math/vector3d.h"

namespace Stark {

/**
 * Store a path and allow to walk along it smoothly
 *
 * The base principle of the string pulling algorithm is to skip steps
 * if it is possible to walk directly to a later step in straight line.
 */
class StringPullingPath {
public:
	StringPullingPath();

	/** Append a step to the path */
	void addStep(const Math::Vector3d &position);

	/** Reset the steps, and the current target on the path */
	void reset();

	/** Move the walk target forward according to the position */
	Math::Vector3d computeWalkTarget(const Math::Vector3d &fromPosition);

	/** Returns true if this path is not degenerated (empty or single point) */
	bool hasSteps() const;

private:
	Common::Array<Math::Vector3d> _steps;
	uint32 _targetStep;
};

} // End of namespace Stark

#endif // STARK_MOVEMENT_STRING_PULLING_PATH_H
