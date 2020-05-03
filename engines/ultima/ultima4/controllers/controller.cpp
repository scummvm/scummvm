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

#include "ultima/ultima4/controllers/controller.h"
#include "ultima/ultima4/events/event_handler.h"
#include "engines/engine.h"

namespace Ultima {
namespace Ultima4 {

Controller::Controller(int timerInterval) {
	this->_timerInterval = timerInterval;
}

Controller::~Controller() {
}

bool Controller::notifyKeyPressed(int key) {
	bool processed = KeyHandler::globalHandler(key);
	if (!processed)
		processed = keyPressed(key);

	return processed;
}

bool Controller::notifyMousePress(const Common::Point &mousePos) {
	return mousePressed(mousePos);
}


int Controller::getTimerInterval() {
	return _timerInterval;
}

void Controller::setActive() {
	// By default, only the Escape action is turned on for controllers,
	// to allow the different sorts of input prompts to be aborted
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
}

void Controller::timerFired() {
}

void Controller::timerCallback(void *data) {
	Controller *controller = static_cast<Controller *>(data);
	controller->timerFired();
}

bool Controller::shouldQuit() const {
	return g_engine->shouldQuit();
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
