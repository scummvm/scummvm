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


#include "common/util.h"
#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Path", 0, 599, 5, {301, 302}, {0, 3}},
	{"House", 402, 374, 1, {303, 304}, {0, 3}},
	{"Well", 491, 495, 3, {305, 306}, {0, 0}},
	{"Stone", 486, 519, 3, {307, 308}, {1, 1}},
	{"Skull", 0, 0, 0, {309, 310}, {0, 0}}
};

const RunawayResponse kResponses[] = {
	{6130, 303, 2, 0},  // Look at Oscar in front of the house.
	{6130, 305, 3, 0},  // Examine the abandoned well.
	{6130, 309, 7, 0},  // Examine the skull.
	{6130, 310, 8, 0}   // Leave Mama Dorita's decoration alone.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6130 = {
	6130, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {100, 95, 90}, {80, 75, 70}}, Common::Point(268, 450),
	kResponses, ARRAYSIZE(kResponses), nullptr, 0, nullptr, 0
};

void RunawayRoomScene::start6130() {
	if (_entryScene == 6131) {
		_position = Common::Point(401, 375);
		_walkPath.reset(_position);
		_facing = 5;
	} else {
		_facing = 1;
		if (!_town.houseVisited) {
			say(0);
			deferTownAction(400);
		}
	}
}

RunawayRoomScene::ActionResult RunawayRoomScene::execute6130(uint16 action) {
	switch (action) {
	case 301: // Look toward Douglasville.
		say(1, _town.streetVisited);
		break;
	case 302: // Return to the overview.
		leaveTownRoom(6350);
		break;
	case 304: // Approach Mama Dorita's doorway.
		leaveTownRoom(6090);
		break;
	case 306: // Refuse to jump into the well.
		say(4, _random.getRandomBit());
		break;
	case 307: // Notice the hole through the stone.
		say(5, _town.stone != 0);
		if (!_town.stone) {
			_town.stone = 1;
		}
		break;
	case 308: // Take the unusually shaped stone.
		if (!_town.stone) {
			execute6130(307);
		}
		say(6);
		deferTownAction(408);
		break;
	case 408:
		_town.stone = 2;
		_inventory.add(29);
		say(6, 1);
		return kBackgroundChanged;
	case 400: // First visit: go and introduce Brian to Oscar.
		walkTo(Common::Point(402, 374), 1);
		deferTownAction(401);
		break;
	case 401:
		_town.houseVisited = 1;
		leaveTownRoom(6090);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Hollywood
