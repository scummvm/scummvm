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
	{"Path", -100, 484, 7, {302, 301}, {0, 1}},
	{"Wooden Box", 103, 518, 1, {303, 304}, {1, 0}},
	{"Shed", 301, 478, 7, {305, 307}, {0, 1}},
	{"Door", 301, 478, 7, {306, 307}, {0, 1}},
	{"Explosives", 301, 478, 7, {308, 309}, {0, 1}},
	{"Water Tank", 620, 357, 7, {310, 311}, {0, 0}},
	{"Train Car", 987, 378, 1, {312, 313}, {0, 1}},
	{"Train Car Interior", 987, 378, 1, {312, 313}, {1, 1}},
	{"Ant-Hill", 67, 512, 0, {316, 317}, {0, 0}},
	{"Sign", 255, 546, 7, {318, 319}, {1, 0}},
	{"Ladder", 620, 357, 7, {320, 321}, {0, 0}},
	{"Sawdust", 0, 0, 0, {0, 0}, {0, 0}},
};

const RunawayResponse kResponses[] = {
	{5080, 302, 1, 0},
	{5080, 308, 7, 0},
	{5080, 310, 9, 0},
	{5080, 311, 10, 0},
	{5080, 319, 16, 0},
	{5080, 320, 17, 0},
	{5080, 321, 18, 0},
	{5080, 324, 19, 0},
	{5080, 325, 20, 0},
};

const RunawayItemAction kItemActions[] = {
	{5080, 0, 6, 325, false},
	{5080, 2, 4, 324, false},
	{5080, 6, 3, 326, false},
	{5080, 6, 4, 326, false},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5080 = {
	5080, 1432, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {100, 95, 90}, {80, 75, 70}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(128, 526),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

RunawayRoomScene::ActionResult RunawayRoomScene::execute5080(uint16 action) {
	RunawaySaveState::DesertItems &items = _inventory.desert();
	switch (action) {
	case 301:
		leaveDesertRoom(5010);
		break;
	case 303:
		say(2);
		break;
	case 305:
		say(4, _desert.antsDistracted);
		break;
	case 306:
		say(5, _desert.shedTried);
		break;
	case 307:
		if (_desert.antsDistracted) {
			say(14, 0, true);
		} else {
			beginRoomAnimation(action, 25, 28);
		}
		break;
	case 1307:
		say(6, _desert.shedTried);
		_desert.shedTried = 1;
		break;
	case 309:
		_desert.dynamite = 2;
		_inventory.add(5);
		say(8);
		return kBackgroundChanged;
	case 313:
		if (!_desert.trainEntered) {
			say(12);
			_desert.trainEntered = 1;
		}
		beginRoomAnimationRange(action, 23, 35, 0, 27);
		break;
	case 1313:
		leaveDesertRoom(5110);
		break;
	case 316:
		say(13, _desert.antsDistracted ? 2 : 0);
		break;
	case 317:
		if (_desert.antsDistracted) {
			say(14, 2);
		} else {
			say(14);
			say(14, 1);
		}
		break;
	case 326:
		if (!items.antTrickKnown) {
			say(3, 0, true);
		} else if (items.helmet != 3) {
			say(17, 0, true);
		} else {
			say(21);
			walkTo(Common::Point(564, 497), 7, true);
			beginRoomAnimation(action, 26, 38);
		}
		break;
	case 1326:
		_desert.antsDistracted = 1;
		_desert.dynamite = 1;
		items.helmet = 4;
		for (byte variation = 1; variation <= 4; ++variation)
			say(21, variation);
		return kBackgroundChanged;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Hollywood
