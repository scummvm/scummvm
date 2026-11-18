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

const MuseumHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Stairs", -25, 319, 7, {301, 302}, {0, 1}},
	{"Coffee machines", 218, 379, 7, {321, 322}, {1, 1}},
	{"Clive's office door", 607, 323, 7, {311, 312}, {0, 1}},
	{"Fire extinguisher", 696, 319, 7, {332, 333}, {0, 0}},
	{"Wastepaper basket", 696, 319, 7, {313, 314}, {1, 1}},
	{"Door to the restoration laboratory", 873, 311, 7, {315, 316}, {0, 1}},
	{"Security door", 1118, 309, 7, {317, 318}, {1, 1}},
	{"Panel", 1153, 297, 7, {319, 320}, {0, 1}},
	{"Stairs", 1220, 280, 7, {301, 303}, {0, 1}},
	{"Telephone", 1473, 312, 7, {324, 325}, {0, 1}},
	{"Display case", 1596, 351, 1, {326, 327}, {0, 0}},
	{"Stairs", 140, 504, 7, {304, 305}, {0, 1}},
	{"Stairs", 1085, 411, 7, {304, 306}, {0, 1}},
	{"Maintenance worker", 921, 813, 7, {307, 308}, {0, 1}},
	{"Mayan exhibition", 1588, 506, 7, {309, 310}, {0, 1}},
	{"Plaque", 711, 797, 7, {328, 329}, {1, 0}},
	{"Plaque", 1060, 730, 7, {330, 331}, {1, 0}},
};

const MuseumResponse kResponses[] = {
	{kMuseumHall, 301, 1, 0},
	{kMuseumHall, 304, 2, 0},
	{kMuseumHall, 309, 4, 0},
	{kMuseumHall, 311, 5, 0},
	{kMuseumHall, 314, 7, 0},
	{kMuseumHall, 315, 8, 0},
	{kMuseumHall, 317, 9, 0},
	{kMuseumHall, 318, 13, 0},
	{kMuseumHall, 319, 10, 0},
	{kMuseumHall, 321, 11, 0},
	{kMuseumHall, 323, 13, 0},
	{kMuseumHall, 324, 14, 0},
	{kMuseumHall, 326, 16, 0},
	{kMuseumHall, 327, 17, 0},
	{kMuseumHall, 328, 18, 0},
	{kMuseumHall, 329, 19, 0},
	{kMuseumHall, 330, 20, 0},
	{kMuseumHall, 331, 19, 0},
	{kMuseumHall, 332, 21, 0},
	{kMuseumHall, 333, 22, 0},
	{kMuseumHall, 336, 25, 0},
	{kMuseumHall, 339, 29, 0},
	{kMuseumHall, 340, 30, 0},
	{kMuseumHall, 341, 31, 0},
	{kMuseumHall, 343, 34, 0},
	{kMuseumHall, 344, 35, 0},
};

const MuseumItemAction kItemActions[] = {
	{kMuseumHall, 19, 2, 346},
	{kMuseumHall, 0, 5, 19},
	{kMuseumHall, 9, 8, 337},
	{kMuseumHall, 10, 8, 338},
	{kMuseumHall, 1, 14, 11},
	{kMuseumHall, 2, 14, 11},
	{kMuseumHall, 3, 14, 11},
	{kMuseumHall, 4, 14, 11},
	{kMuseumHall, 5, 14, 334},
	{kMuseumHall, 6, 14, 335},
	{kMuseumHall, 7, 14, 336},
	{kMuseumHall, 8, 14, 11},
	{kMuseumHall, 9, 14, 11},
	{kMuseumHall, 10, 14, 11},
	{kMuseumHall, 11, 14, 339},
	{kMuseumHall, 12, 14, 340},
	{kMuseumHall, 13, 14, 341},
	{kMuseumHall, 14, 14, 342},
	{kMuseumHall, 15, 14, 343},
	{kMuseumHall, 16, 14, 11},
	{kMuseumHall, 17, 14, 344},
	{kMuseumHall, 18, 14, 11},
	{kMuseumHall, 19, 14, 345},
};

const MuseumExit kExits[] = {
	{kMuseumHall, 302, kMuseumHall, 533, 848, false},
	{kMuseumHall, 303, kMuseumHall, 1169, 643, false},
	{kMuseumHall, 305, kMuseumHall, 69, 372, true},
	{kMuseumHall, 306, kMuseumHall, 1290, 300, true},
	{kMuseumHall, 310, kMuseumExhibition, 881, 392, false},
	{kMuseumHall, 312, kMuseumOffice, 934, 563, true},
	{kMuseumHall, 316, kMuseumLaboratory, 382, 529, true},
	{kMuseumHall, 320, kMuseumKeypad, 0, 0, true},
};

} // End of anonymous namespace

const MuseumRoom kRunawayScene4060 = {
	kMuseumHall, 1760, 1200, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {40, 40, 40}, {30, 30, 30}, {60, 60, 60}, {30, 30, 30}, {70, 70, 70}, {20, 20, 25}, {50, 50, 50}, {60, 60, 70}, {70, 70, 80}},
	Common::Point(873, 311),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	kExits, ARRAYSIZE(kExits)
};

void RunawayMuseumScene::updateMaps4060() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		// Both floors share the image and mask, but have separate walk zones and hotspots.
		const byte zone = _maps[i];
		if (_upperFloor ? zone < 5 || zone > 6 : zone > 4)
			_maps[i] = 0;
		if (_upperFloor ? id > 11 : id < 12)
			id = 0;
	}
}

void RunawayMuseumScene::changeFloor(bool upper, Common::Point position) {
	_upperFloor = upper;
	_position = position;
	_x = position.x;
	_y = position.y;
	_facing = 3;
	updateMaps();
	updateCamera();
}

RunawayMuseumScene::ActionResult RunawayMuseumScene::execute4060(uint16 action) {
	if (action != 313)
		return kUnhandled;
	if (_coffeeTaken) {
		say(7);
		return kHandled;
	}
	_coffeeTaken = true;
	_inventory.add(19);
	say(6);
	say(6, 1);
	return kHandled;
}

float RunawayMuseumScene::actorHeight4060() const {
	const int x = _position.x, y = _position.y;
	const byte zone = walkZone();
	if (_upperFloor) {
		if (x < 69)
			return x * 1.289855f + 140;
		if (x < 1120)
			return x * -0.04883463f + 241.64595f;
		if (x < 1474)
			return x * -0.01977401f + 209.127121f;
		return x * 0.3125f - 280.3125f;
	}
	if (zone == 7 || zone == 8)
		return y * 0.4370861f - 140.649f;
	if (zone == 12 || zone == 13)
		return y * 0.29007635f - 33.519085f;
	return (y * 0.4347826f - 118.695656f + x * -0.08982036f + 258.263458f) / 2;
}

void RunawayMuseumScene::setupDepth4060(RunawayActor::RenderParams &params) const {
	const byte zone = walkZone();
	params.depthY[1] = 1199;
	params.depthY[2] = zone == 1 ? 1199 : 0;
	params.depthY[3] = zone == 13 || zone == 15 || zone == 16 || zone == 17 ? 1199 : 0;
	params.depthY[4] = zone == 19 || zone == 8 || zone == 7 ? 1199 : 0;
	params.depthY[5] = zone == 17 || zone == 18 ? 1199 : 0;
}

bool RunawayMuseumScene::loadScene4060() {
	return _ambient.load(_archive, 8, 22);
}

} // End of namespace Hollywood
