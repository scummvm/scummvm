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

#include "engines/stark/movement/stringpullingpath.h"

#include "engines/stark/resources/floor.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"

#include "math/line3d.h"

namespace Stark {

StringPullingPath::StringPullingPath() :
		_targetStep(1) {
}

void StringPullingPath::addStep(const Math::Vector3d &position) {
	_steps.push_back(position);
}

void StringPullingPath::reset() {
	_steps.clear();
	_targetStep = 1;
}

Math::Vector3d StringPullingPath::computeWalkTarget(const Math::Vector3d &fromPosition) {
	Current *current = StarkGlobal->getCurrent();
	Resources::Floor *floor = current->getFloor();

	// HACK: Sometimes the character gets stuck because of rounding errors
	// If we detect the character is stuck on a step, just make it go to the next one.
	// TODO: Improve the string pulling code so that the targets can also be points between two steps.
	if (fromPosition.getDistanceTo(_steps[_targetStep]) < 1.0 && _targetStep < _steps.size() - 1) {
		_targetStep++;
	}

	for (uint i = _targetStep + 1; i < _steps.size(); i++) {
		Math::Line3d testSegment = Math::Line3d(fromPosition, _steps[i]);
		if (!floor->isSegmentInside(testSegment)) {
			break;
		}

		_targetStep = i;
	}

	return _steps[_targetStep];
}

bool StringPullingPath::hasSteps() const {
	return _steps.size() > 1;
}

} // End of namespace Stark
