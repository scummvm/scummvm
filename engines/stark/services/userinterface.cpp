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

#include "engines/stark/services/userinterface.h"

#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/speech.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"

namespace Stark {

UserInterface::UserInterface() {
}

UserInterface::~UserInterface() {
}

void UserInterface::skipCurrentSpeeches() {
	Global *global = StarkServices::instance().global;
	Current *current = global->getCurrent();

	if (!current) {
		return; // No current location, nothing to do
	}

	// Get all speeches
	Common::Array<Speech *> speeches;
	speeches.push_back(global->getLevel()->listChildrenRecursive<Speech>());
	speeches.push_back(current->getLevel()->listChildrenRecursive<Speech>());
	speeches.push_back(current->getLocation()->listChildrenRecursive<Speech>());

	// Stop them
	for (uint i = 0; i < speeches.size(); i++) {
		Speech *speech = speeches[i];
		if (speech->isPlaying()) {
			speech->stop();
		}
	}
}

void UserInterface::scrollLocation(int32 dX, int32 dY) {
	Global *global = StarkServices::instance().global;
	Current *current = global->getCurrent();

	if (!current) {
		return; // No current location, nothing to do
	}

	Location *location = current->getLocation();

	Common::Point scroll = location->getScrollPosition();
	scroll.x += dX;
	scroll.y += dY;
	location->setScrollPosition(scroll);
}

} // End of namespace Stark
