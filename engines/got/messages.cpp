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

#include "got/messages.h"

namespace Got {

MouseMessage::MouseMessage(Common::EventType type,
						   const Common::Point &pos) : Message(), _pos(pos) {
	switch (type) {
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
		_button = MB_RIGHT;
		break;
	case Common::EVENT_MBUTTONDOWN:
	case Common::EVENT_MBUTTONUP:
		_button = MB_MIDDLE;
		break;
	default:
		_button = MB_LEFT;
		break;
	}
}

} // namespace Got
