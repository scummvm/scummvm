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
	{"Path", 1279, 267, 0, {301, 302}, {0, 1}},
	{"Barn Doors", 1259, 354, 7, {303, 304}, {1, 0}},
	{"Bottle", 0, 0, 0, {305, 306}, {1, 1}},
	{"Window", 1279, 267, 4, {1, 302}, {0, 1}},
	{"Motorcycle", 164, 581, 0, {307, 308}, {1, 0}},
	{"Stud", 0, 0, 7, {309, 310}, {1, 1}},
	{"?", 0, 0, 4, {311, 312}, {0, 0}},
	{"Piece Of Metal", 0, 0, 7, {313, 314}, {0, 1}},
	{"Barrel", 0, 0, 1, {315, 316}, {1, 0}},
	{"Barrel", 0, 0, 4, {317, 318}, {0, 0}},
	{"Helicopter", 0, 0, 7, {319, 320}, {0, 0}},
	{"?", 253, 418, 4, {321, 322}, {0, 0}},
	{"Bucket", 0, 0, 1, {323, 324}, {1, 0}},
	{"", 253, 418, 4, {0, 1}, {0, 0}},
	{"", 1053, 403, 3, {0, 1}, {0, 0}},
	{"Beer Can", 0, 0, 0, {0, 0}, {0, 0}},
	{"Peanut Butter", 0, 0, 0, {0, 0}, {0, 0}},
	{"Foot Rest", 0, 0, 0, {0, 0}, {0, 0}},
};

const RunawayResponse kResponses[] = {
	{5100, 308, 16, 0},
	{5100, 311, 18, 0},
	{5100, 312, 19, 0},
	{5100, 316, 22, 0},
	{5100, 318, 24, 0},
	{5100, 319, 25, 0},
	{5100, 320, 26, 0},
	{5100, 322, 8, 0},
	{5100, 324, 28, 0},
};

const RunawayItemAction kItemActions[] = {
	{5100, 2, 5, 327, true},
	{5100, 9, 6, 16, false},
	{5100, 9, 12, 328, true},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5100 = {
	5100, 1880, 700, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {100, 95, 90}, {60, 55, 50}, {90, 85, 80}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(1279, 267),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

RunawayRoomScene::ActionResult RunawayRoomScene::execute5100(uint16 action) {
	if (action != 302)
		return kUnhandled;
	leaveDesertRoom(5010);
	return kHandled;
}

} // End of namespace Hollywood
