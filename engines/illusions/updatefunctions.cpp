/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/updatefunctions.h"
#include "illusions/time.h"
#include "common/algorithm.h"
#include "common/system.h"

namespace Illusions {

// UpdateFunctions

UpdateFunctions::UpdateFunctions() {
	_lastTimerUpdateTime = 0;
}

UpdateFunctions::~UpdateFunctions() {
	// Free update functions
	for (UpdateFunctionListIterator it = _updateFunctions.begin(); it != _updateFunctions.end(); ++it) {
		delete *it;
	}
}

void UpdateFunctions::add(int priority, uint32 sceneId, UpdateFunctionCallback *callback) {
	UpdateFunction *updateFunction = new UpdateFunction();
	updateFunction->_priority = priority;
	updateFunction->_sceneId = sceneId;
	updateFunction->_callback = callback;
	UpdateFunctionListIterator insertionPos = Common::find_if(_updateFunctions.begin(), _updateFunctions.end(),
		FindInsertionPosition(priority));
	_updateFunctions.insert(insertionPos, updateFunction);
}

void UpdateFunctions::update() {
	// Avoid running updates multiple times in the current time slice
	while (_lastTimerUpdateTime == getCurrentTime()) {
		g_system->delayMillis(10); // CHECKME Timer resolution
	}
	_lastTimerUpdateTime = getCurrentTime();
	UpdateFunctionListIterator it = _updateFunctions.begin();
	while (it != _updateFunctions.end()) {
		int r = (*it)->run();
		switch (r) {
		case kUFNext:
			++it;
			break;
		case kUFTerminate:
			delete *it;
			it = _updateFunctions.erase(it);
			break;
		default:
			break;
		}
	}
}

void UpdateFunctions::terminateByScene(uint32 sceneId) {
	for (UpdateFunctionListIterator it = _updateFunctions.begin(); it != _updateFunctions.end(); ++it) {
		if ((*it)->_sceneId == sceneId)
			(*it)->terminate();
	}
}

} // End of namespace Illusions
