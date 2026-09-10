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

#include "ultima/ultima1/views/space_cockpit.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

bool SpaceCockpit::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_GAMEPLAY);
	return Map::msgFocus(msg);
}

bool SpaceCockpit::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return Map::msgUnfocus(msg);
}

void SpaceCockpit::draw() {
	Map::draw();

	// TODO: the cockpit frame, HUD (Shld:/Fuel:), and the panning starfield
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
