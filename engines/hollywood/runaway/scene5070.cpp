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
	{"Path", 1472, 495, 0, {302, 301}, {0, 1}},
	{"Mariola", 0, 0, 7, {1, 1}, {0, 0}},
	{"Detonator", 1122, 509, 7, {305, 306}, {0, 0}},
	{"Oil Well", 301, 264, 7, {307, 308}, {0, 0}},
	{"Tree", 0, 0, 0, {309, 310}, {0, 0}},
	{"Rock", 0, 0, 7, {313, 314}, {0, 0}},
	{"Puddle Of Oil", 287, 312, 7, {315, 316}, {1, 0}},
};

const RunawayResponse kResponses[] = {
	{5070, 302, 1, 0},
	{5070, 307, 4, 0},
	{5070, 308, 5, 0},
	{5070, 309, 6, 0},
};

const RunawayItemAction kItemActions[] = {
	{5070, 5, 4, 311, true},
	{5070, 20, 7, 317, false},
	{5070, 25, 4, 312, false},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5070 = {
	5070, 1372, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {100, 95, 90}, {80, 75, 70}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(1245, 495),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

RunawayRoomScene::ActionResult RunawayRoomScene::execute5070(uint16 action) {
	RunawaySaveState::DesertItems &items = _inventory.desert();
	switch (action) {
	case 301:
		leaveDesertRoom(5010);
		break;
	case 305:
		say(2, _random.getRandomBit());
		break;
	case 306: {
		const byte variation = _random.getRandomBit();
		say(3, variation);
		if (variation)
			say(3, 2);
		break;
	}
	case 310:
		say(5, 0, true);
		break;
	case 311:
		say(7);
		beginRoomAnimation(action, 11, 40);
		break;
	case 1311:
		_desert.dynamitePlaced = 1;
		items.bombReady = 1;
		_inventory.remove(5);
		say(7, 1);
		if (items.gunReady && items.helicopterReady) {
			say(174, 4, true);
			leaveDesertRoom(5062);
		}
		return kBackgroundChanged;
	case 312:
		if (items.bucket != 1)
			say(17, 0, true);
		else
			say(8, _desert.dynamitePlaced != 0);
		break;
	case 313:
		say(9, _desert.dynamitePlaced != 0);
		break;
	case 314:
		say(10);
		break;
	case 315:
		say(11, _desert.oilSeen != 0);
		_desert.oilSeen = 1;
		break;
	case 316:
		if (!_desert.oilSeen)
			execute5070(315);
		say(12);
		break;
	case 317:
		if (items.glasses)
			say(13, items.glasses + 1);
		else {
			say(13);
			beginRoomAnimation(action, 9, 29);
		}
		break;
	case 1317:
		items.glasses = 1;
		_inventory.select(0);
		say(13, 1);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Hollywood
