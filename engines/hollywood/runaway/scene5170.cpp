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
	{"Outside Of The Bus", 339, 512, 7, {301, 302}, {0, 0}},
	{"Back Of The Bus", 666, 303, 0, {303, 304}, {0, 1}},
	{"Right-Hand Shelf", 727, 436, 1, {305, 306}, {1, 1}},
	{"Axle", 649, 499, 4, {307, 308}, {0, 1}},
	{"Pocket", 113, 540, 5, {309, 310}, {1, 1}},
	{"Sunglasses", 727, 436, 1, {311, 312}, {0, 1}},
	{"Item Under Bed", 462, 448, 7, {313, 314}, {1, 1}},
	{"Middle Shelf", 727, 436, 1, {315, 316}, {1, 0}},
	{"Left-Hand Shelf", 727, 436, 1, {317, 318}, {1, 0}},
	{"Back Of The Bus", 666, 303, 0, {303, 304}, {0, 1}},
	{"Door", 0, 0, 0, {0, 0}, {0, 0}},
	{"Pile Of Lipstick", 0, 0, 0, {0, 0}, {0, 0}},
	{"Makeshift Crank", 0, 0, 0, {0, 0}, {0, 0}},
	{"Handle", 0, 0, 0, {0, 0}, {0, 0}},
	{"Necklaces", 0, 0, 0, {0, 0}, {0, 0}},
	{"Rings And Earrings", 0, 0, 0, {0, 0}, {0, 0}},
	{"Outside Of The Bus", 0, 0, 0, {0, 0}, {0, 0}},
	{"Right-Hand Shelf", 0, 0, 0, {0, 0}, {0, 0}},
};

const RunawayResponse kResponses[] = {
	{5170, 311, 10, 0},
	{5170, 319, 18, 0},
};

const RunawayItemAction kItemActions[] = {
	{5170, 10, 3, 319, false},
	{5170, 11, 3, 319, false},
	{5170, 21, 4, 320, true},
	{5170, 26, 5, 15, false},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5170 = {
	5170, 1024, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {80, 72, 80}, {80, 72, 80}, {85, 77, 85}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(339, 512),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

RunawayRoomScene::ActionResult RunawayRoomScene::execute5170(uint16 action) {
	switch (action) {
	case 301:
		say(1, _desert.busDoor != 0);
		break;
	case 302:
		if (_desert.busDoor == 0 || _desert.busDoor == 2)
			leaveDesertRoom(5061);
		else
			say(2);
		break;
	case 304:
		leaveDesertRoom(5160);
		break;
	case 305:
		say(4, _desert.lipstickShelf == 0 ? 0 : _desert.lipstickShelf == 3 ? 2 : 1);
		if (!_desert.lipstickShelf)
			_desert.lipstickShelf = 1;
		break;
	case 306:
		if (_desert.lipstickShelf < 2) {
			say(5);
			_inventory.add(10);
			_desert.lipstickShelf = 2;
			say(5, 1);
		} else if (_desert.lipstickShelf == 2) {
			if (!_inventory.desert().lipstick) {
				say(5, 2);
			} else {
				say(5, 3);
				_inventory.add(11);
				_desert.lipstickShelf = 3;
				say(5, 4);
			}
		} else {
			say(5, 5);
		}
		return kBackgroundChanged;
	case 307:
		say(6, _desert.crank != 0);
		if (!_desert.crank)
			_desert.crank = 1;
		break;
	case 308:
		if (_desert.crank < 2) {
			say(7);
		} else {
			beginRoomAnimation(action, 13, 29);
			_audio.playEffect(_desert.busDoor == 1 ? 65 : 64);
		}
		break;
	case 1308:
		if (!_desert.busDoor)
			say(7, 1);
		_desert.busDoor = _desert.busDoor == 1 ? 2 : 1;
		return kBackgroundChanged;
	case 309:
		say(8);
		walkTo(Common::Point(339, 512), 3, true);
		break;
	case 310:
		if (_desert.keyTaken) {
			say(9, 2);
		} else {
			say(9);
			_inventory.add(26);
			_desert.keyTaken = 1;
			if (_desert.fridgeSeen)
				say(9, 1);
			else
				say(18, _random.getRandomNumber(2), true);
			walkTo(Common::Point(340, 511), 3, true);
		}
		break;
	case 312:
		say(11);
		_inventory.add(20);
		_desert.glassesTaken = 1;
		say(11, 1);
		return kBackgroundChanged;
	case 313:
		say(12, _desert.vacuum != 0);
		if (!_desert.vacuum)
			_desert.vacuum = 1;
		break;
	case 314:
		if (!_desert.vacuum)
			say(12);
		_inventory.add(22);
		_desert.vacuum = 2;
		say(13);
		return kBackgroundChanged;
	case 315:
		say(14, _desert.necklacesSeen);
		_desert.necklacesSeen = 1;
		break;
	case 316:
		if (!_desert.necklacesSeen)
			execute5170(315);
		say(15);
		break;
	case 317:
		say(16, _desert.ringsSeen);
		_desert.ringsSeen = 1;
		break;
	case 318:
		if (!_desert.ringsSeen)
			execute5170(317);
		say(17);
		break;
	case 320:
		say(19);
		beginRoomAnimation(action, 12, 42);
		break;
	case 1320:
		_inventory.remove(21);
		_desert.crank = 2;
		say(19, 1);
		return kBackgroundChanged;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Hollywood
