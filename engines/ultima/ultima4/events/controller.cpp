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

#include "ultima/ultima4/events/controller.h"
#include "ultima/ultima4/events/event.h"

namespace Ultima {
namespace Ultima4 {

Controller::Controller(int timerInterval) {
	this->_timerInterval = timerInterval;
}

Controller::~Controller() {
}

/**
 * The event manager will call this method to notify the active
 * controller that a key has been pressed.  The key will be passed on
 * to the virtual keyPressed method.
 */
bool Controller::notifyKeyPressed(int key) {
	bool processed = KeyHandler::globalHandler(key);
	if (!processed)
		processed = keyPressed(key);

	return processed;
}

int Controller::getTimerInterval() {
	return _timerInterval;
}

/**
 * The default timerFired handler for a controller.  By default,
 * timers are ignored, but subclasses can override this method and it
 * will be called every <interval> 1/4 seconds.
 */
void Controller::timerFired() {
}

/**
 * A simple adapter to make a timer callback into a controller method
 * call.
 */
void Controller::timerCallback(void *data) {
	Controller *controller = static_cast<Controller *>(data);
	controller->timerFired();
}

void Controller_startWait() {
	eventHandler->run();
	eventHandler->setControllerDone(false);
	eventHandler->popController();
}

void Controller_endWait() {
	eventHandler->setControllerDone();
}

} // End of namespace Ultima4
} // End of namespace Ultima
