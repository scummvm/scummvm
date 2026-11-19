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
#include "hollywood/runaway/scene_data.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Path", 1479, 599, 0, {301, 302}, {0, 1}},
	{"Lula", 349, 442, 7, {303, 304}, {0, 1}},
	{"Mariola", 878, 477, 0, {305, 306}, {0, 0}},
	{"Inside Of The Bus", 679, 448, 1, {307, 308}, {0, 1}},
	{"Trunk", 376, 445, 1, {309, 310}, {1, 1}},
	{"Bottle", 878, 477, 0, {311, 312}, {1, 1}},
	{"Chair", 383, 446, 5, {313, 314}, {0, 0}},
	{"?", 349, 442, 5, {315, 316}, {0, 0}},
	{"Sunglasses", 878, 477, 0, {317, 318}, {0, 0}},
	{"Tanning Oil", 0, 0, 0, {0, 0}, {0, 0}},
	{"Lawn Chair", 0, 0, 0, {0, 0}, {0, 0}},
};

const RunawayResponse kResponses[] = {
	{5060, 301, 1, 0},
	{5060, 315, 12, 0},
	{5060, 316, 13, 0},
};

const RunawayItemAction kItemActions[] = {
	{5060, 15, 2, 319, false},
	{5060, 20, 9, 320, true},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5060 = {
	5060, 1380, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {100, 95, 90}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(954, 506),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

RunawayRoomScene::ActionResult RunawayRoomScene::execute5060(uint16 action) {
	switch (action) {
	case 302:
		leaveDesertRoom(5010);
		break;
	case 308:
		beginRoomAnimation(action, 5, 22);
		break;
	case 1308:
		leaveDesertRoom(5170);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Hollywood
