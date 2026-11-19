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
	{"Path", 1123, 416, 3, {302, 301}, {0, 1}},
	{"", 456, 405, 7, {0, 1}, {0, 0}},
	{"Rocking Chair", 573, 400, 7, {304, 305}, {0, 0}},
	{"Tires", 644, 397, 1, {310, 311}, {0, 0}},
	{"Door", 456, 405, 7, {306, 307}, {0, 0}},
	{"Trap Door", 618, 386, 0, {308, 309}, {1, 1}},
	{"Bucket", 605, 399, 7, {312, 313}, {1, 1}},
};

const RunawayResponse kResponses[] = {
	{5020, 303, 1, 0},
	{5020, 305, 4, 0},
	{5020, 306, 5, 0},
	{5020, 307, 6, 0},
	{5020, 308, 7, 0},
	{5020, 310, 9, 0},
	{5020, 311, 10, 0},
};

const RunawayItemAction kItemActions[] = {
	{5020, 1, 2, 303, true},
	{5020, 2, 2, 303, true},
	{5020, 3, 2, 303, true},
	{5020, 4, 2, 303, true},
	{5020, 5, 2, 303, true},
	{5020, 6, 2, 303, true},
	{5020, 7, 2, 303, true},
	{5020, 8, 2, 303, true},
	{5020, 9, 2, 303, true},
	{5020, 10, 2, 303, true},
	{5020, 11, 2, 303, true},
	{5020, 12, 2, 303, true},
	{5020, 13, 2, 303, true},
	{5020, 14, 2, 303, true},
	{5020, 15, 2, 303, true},
	{5020, 16, 2, 303, true},
	{5020, 17, 2, 303, true},
	{5020, 18, 2, 303, true},
	{5020, 19, 2, 303, true},
	{5020, 20, 2, 303, true},
	{5020, 21, 2, 303, true},
	{5020, 22, 2, 303, true},
	{5020, 23, 2, 303, true},
	{5020, 24, 2, 303, true},
	{5020, 25, 2, 303, true},
	{5020, 26, 2, 303, true},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5020 = {
	5020, 1024, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {90, 85, 80}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(953, 406),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

RunawayRoomScene::ActionResult RunawayRoomScene::execute5020(uint16 action) {
	switch (action) {
	case 302:
		leaveDesertRoom(5010);
		break;
	case 309:
		if (!_desert.hatchSeen) {
			_desert.hatchSeen = 1;
			say(8);
		}
		leaveDesertRoom(5031);
		break;
	case 313:
		_desert.bucketTaken = 1;
		_inventory.add(25);
		say(12);
		return kBackgroundChanged;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Hollywood
