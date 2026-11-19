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
	{"?", 327, 441, 7, {301, 302}, {0, 1}},
	{"?", 574, 434, 1, {303, 304}, {1, 0}},
	{"Chain", 801, 441, 3, {305, 306}, {0, 0}},
	{"Contraptions", 517, 374, 0, {307, 308}, {0, 0}},
	{"Barrel", 510, 377, 0, {309, 310}, {1, 1}},
	{"Bucket", 365, 485, 5, {311, 312}, {1, 1}},
	{"Clock", 801, 441, 3, {313, 314}, {0, 0}},
	{"Bucket", 510, 377, 0, {320, 321}, {0, 1}},
};

const RunawayResponse kResponses[] = {
	{5110, 301, 1, 0},
	{5110, 305, 4, 0},
	{5110, 306, 5, 0},
	{5110, 307, 6, 0},
	{5110, 308, 7, 0},
	{5110, 313, 12, 0},
	{5110, 314, 13, 0},
	{5110, 315, 14, 0},
};

const RunawayItemAction kItemActions[] = {
	{5110, 2, 2, 319, true},
	{5110, 2, 3, 318, false},
	{5110, 2, 5, 315, false},
	{5110, 4, 5, 324, false},
	{5110, 6, 5, 323, false},
	{5110, 7, 5, 316, false},
	{5110, 17, 5, 317, true},
	{5110, 25, 5, 322, true},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5110 = {
	5110, 1024, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {75, 70, 65}, {65, 60, 55}, {55, 50, 45}, {45, 40, 35}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(327, 441),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

RunawayRoomScene::ActionResult RunawayRoomScene::execute5110(uint16 action) {
	RunawaySaveState::DesertItems &items = _inventory.desert();
	switch (action) {
	case 302:
		leaveDesertRoom(5081);
		break;
	case 303:
		say(2, _desert.trainDoorOpen != 0);
		break;
	case 309:
		say(8, _desert.barrel);
		if (!_desert.barrel)
			_desert.barrel = 1;
		break;
	case 310:
		if (!_desert.barrel)
			execute5110(309);
		say(9, _desert.barrel == 2);
		break;
	case 311:
		say(10, _desert.screwSeen != 0);
		_desert.screwSeen = 1;
		break;
	case 312:
		if (_desert.screwTaken)
			say(11, 2);
		else {
			if (!_desert.screwSeen)
				execute5110(311);
			say(11);
			_inventory.add(7);
			_desert.screwTaken = 1;
			say(11, 1);
		}
		break;
	case 316:
		say(16);
		_desert.powderNeeded = 1;
		break;
	case 317:
		if (!_desert.bucket) {
			say(17);
			_desert.powderNeeded = 1;
		} else if (_desert.bucket == 1) {
			say(17, 1);
			beginRoomAnimation(action, 12, 121);
		} else {
			say(17, 3);
		}
		break;
	case 1317:
		_desert.barrel = 2;
		_desert.bucket = 2;
		say(17, 2);
		return kBackgroundChanged;
	case 319:
		if (_desert.trainDoorOpen)
			say(18, 3);
		else {
			say(18);
			beginRoomAnimation(action, 10, 49);
		}
		break;
	case 1319:
		_desert.trainDoorOpen = 1;
		say(18, 1);
		_inventory.add(18);
		say(18, 2);
		return kBackgroundChanged;
	case 320:
		say(19, _desert.bucket != 1);
		break;
	case 321:
		if (_desert.bucket == 1)
			say(20);
		else {
			_desert.bucket = 3;
			items.bucket = 1;
			_inventory.add(25);
			say(20, 1);
			return kBackgroundChanged;
		}
		break;
	case 322:
		if (!_desert.powderNeeded)
			say(21);
		else if (_desert.bucket)
			say(17, 3);
		else {
			say(21, 1);
			beginRoomAnimation(action, 11, 49);
		}
		break;
	case 1322:
		_desert.bucket = 1;
		_inventory.remove(25);
		return kBackgroundChanged;
	case 323:
		if (!_desert.powderNeeded)
			say(14, 0, true);
		else
			say(22, items.helmet == 0);
		break;
	case 324:
		say(_desert.powderNeeded ? 23 : 14, 0, !_desert.powderNeeded);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Hollywood
