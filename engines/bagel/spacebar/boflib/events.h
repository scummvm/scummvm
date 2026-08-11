
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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BAGEL_BOFLIB_EVENTS_H
#define BAGEL_BOFLIB_EVENTS_H

#include "common/events.h"

namespace Bagel {
namespace SpaceBar {

/**
 * Enum for custom ScummVM events
 */
enum BagelEventType {
	// EVENT user stores a message num in mouse.x, and param in mouse.y
	EVENT_USER = 1001,
	// TIMER events
	EVENT_TIMER = 1002
};

} // namespace SpaceBar
} // namespace Bagel

#endif
