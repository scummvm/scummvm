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

#include "common/stack.h"
#include "engines/nancy/detection.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

void DependencyRecord::reset() {
	satisfied = false;
	for (auto &child : children) {
		child.reset();
	}
}

void ActionRecord::finishExecution() {
	switch (_execType) {
	case kOneShot:
		_isDone = true;
		_state = kBegin;
		break;
	case kRepeating: {
		_isDone = false;
		_isActive = false;
		_state = kBegin;

		_dependencies.reset();

		break;
	}
	default:
		_state = kBegin;
		break;
	}
}

void Unimplemented::execute() {
	debugC(Nancy::kDebugActionRecord, "Unimplemented or changed ActionRecord type %u", _type);
	_isDone = true;
}

} // End of namespace Action
} // End of namespace Nancy
