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

#include "common/system.h"
#include "graphics/paletteman.h"
#include "ultima/ultima0/views/startup.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

void Startup::draw() {
	auto s = getSurface();
	s.writeString(Common::Point(5, 10), "Ultima 0 - Akalabeth!");
	s.writeString(Common::Point(2, 19), "Ready?");
}

bool Startup::msgKeypress(const KeypressMessage &msg) {
	close();
	return true;
}

bool Startup::msgMouseDown(const MouseDownMessage &msg) {
	close();
	return true;
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
