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

#include "ultima/ultima1/views/startup.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

#define DELAY_SECONDS 3

#define COLOR_LINE 1        // blue
#define COLOR_FOREGROUND 15 // white

bool Startup::msgFocus(const FocusMessage &msg) {
	delaySeconds(DELAY_SECONDS);
	return View::msgFocus(msg);
}

void Startup::timeout() {
	showTitle();
}

void Startup::draw() {
	auto s = getSurface();
	s.clear();

	// Two decorative bars, each three scanlines tall, above/below the Ultima I line
	s.fillRect(Common::Rect(112, 58, 201, 61), COLOR_LINE);
	s.fillRect(Common::Rect(112, 74, 201, 77), COLOR_LINE);

	s.setColor(COLOR_FOREGROUND);
	s.writeString(Common::Point(16, 8), "Ultima I");
	s.writeString(Common::Point(8, 11), "The First Age of Darkness");
	s.writeString(Common::Point(0, 21), "Copyright (C) 1987 Origin Systems, Inc.");
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
