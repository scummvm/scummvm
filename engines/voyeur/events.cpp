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

#include "voyeur/events.h"
#include "voyeur/voyeur.h"

namespace Voyeur {

EventsManager::EventsManager() {
	_cycleStatus = 0;
}

void EventsManager::resetMouse() {
	// No implementation
}

void EventsManager::startMainClockInt() {
	_mainIntNode._intFunc = mainVoyeurIntFunc;
	_mainIntNode._flags = 0;
	_mainIntNode._curTime = 0;
	_mainIntNode._timeReset = _vm->_graphicsManager._palFlag ? 50 : 60;
}

void EventsManager::mainVoyeurIntFunc() {

}

void EventsManager::vStopCycle() {
	_cycleIntNode._flags = 1;
	_cycleStatus &= 2;
}

void EventsManager::sWaitFlip() {
	// TODO: See if this needs a proper wait loop with event polling
	//while (_intPtr._field39) ;

	Common::Array<ViewPortResource *> &viewPorts = *_vm->_graphicsManager._viewPortListPtr;
	for (uint idx = 0; idx < viewPorts.size(); ++idx) {
		ViewPortResource &viewPort = *viewPorts[idx];

		if (_vm->_graphicsManager._saveBack && (viewPort._flags & 0x40)) {
			Common::Rect *clipPtr = _vm->_graphicsManager._clipPtr;
			_vm->_graphicsManager._clipPtr = &viewPort._clipRect;

			if (viewPort._restoreFn)
				(_vm->_graphicsManager.*viewPort._restoreFn)(&viewPort);

			_vm->_graphicsManager._clipPtr = clipPtr;
			viewPort._field40[viewPort._pageIndex] = 0;
			viewPort._flags &= 0xFFBF;
		}
	}
}

} // End of namespace Voyeur
