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

#ifndef ULTIMA6_CORE_SCUMMVM_EVENTS_H
#define ULTIMA6_CORE_SCUMMVM_EVENTS_H

#include "common/events.h"

namespace Ultima {
namespace Ultima6 {

enum MouseButton { BUTTON_LEFT = 1, BUTTON_RIGHT = 2, BUTTON_MIDDLE = 3 };

inline bool isMouseDown(Common::EventType type) {
	return type == Common::EVENT_LBUTTONDOWN || type == Common::EVENT_RBUTTONDOWN
		|| type == Common::EVENT_MBUTTONDOWN;
}

inline bool isMouseUp(Common::EventType type) {
	return type == Common::EVENT_LBUTTONUP || type == Common::EVENT_RBUTTONUP
		|| type == Common::EVENT_MBUTTONUP;
}

inline bool shouldQuit() {
	// TODO
	return false;
}

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
