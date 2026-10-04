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

#include "ultima/ultima3/views/journey_onward.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

void JourneyOnward::draw() {
	auto s = getSurface();
	clearWindow(s);

	if (_G(savegame)._partySize == 0)
		s.writeString(Common::Point(15, 21), "(No Party)");
	else
		s.writeString(Common::Point(9, 21), "(No Active Players)");

	drawSpacePrompt(s);
}

bool JourneyOnward::msgKeypress(const KeypressMessage &msg) {
	if (isSpaceKey(msg))
		close();
	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
