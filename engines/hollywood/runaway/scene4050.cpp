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
	{"Door", 227, 503, 7, {301, 339}, {0, 1}},
	{"Shelves", 311, 493, 7, {302, 303}, {0, 0}},
	{"Susan", 382, 529, 1, {304, 305}, {0, 1}},
	{"Electric lathe", 395, 529, 1, {306, 307}, {0, 0}},
	{"Microscope", 460, 543, 1, {308, 309}, {0, 0}},
	{"Water dispenser", 637, 570, 1, {310, 311}, {0, 0}},
	{"Wastepaper basket", 703, 575, 1, {312, 313}, {1, 0}},
	{"Tank", 844, 505, 1, {314, 315}, {0, 0}},
	{"Machine", 884, 580, 1, {316, 317}, {0, 0}},
	{"Laser", 992, 556, 1, {318, 319}, {0, 0}},
	{"Shelves", 1327, 521, 1, {320, 321}, {0, 0}},
	{"Briefcase", 1277, 514, 7, {322, 323}, {1, 1}},
	{"Skeleton", 1211, 510, 7, {324, 325}, {0, 0}},
	{"Strange brush", 824, 531, 7, {337, 338}, {1, 1}},
};

const MuseumResponse kResponses[] = {
	{kMuseumLaboratory, 301, 1, 0},
	{kMuseumLaboratory, 306, 6, 0},
	{kMuseumLaboratory, 307, 7, 0},
	{kMuseumLaboratory, 308, 8, 0},
	{kMuseumLaboratory, 309, 9, 0},
	{kMuseumLaboratory, 310, 10, 0},
	{kMuseumLaboratory, 311, 11, 0},
	{kMuseumLaboratory, 314, 12, 0},
	{kMuseumLaboratory, 316, 13, 0},
	{kMuseumLaboratory, 319, 15, 0},
	{kMuseumLaboratory, 320, 16, 0},
	{kMuseumLaboratory, 321, 17, 0},
	{kMuseumLaboratory, 322, 18, 0},
	{kMuseumLaboratory, 324, 20, 0},
	{kMuseumLaboratory, 325, 21, 0},
	{kMuseumLaboratory, 326, 22, 2},
	{kMuseumLaboratory, 332, 27, 0},
	{kMuseumLaboratory, 334, 29, 0},
	{kMuseumLaboratory, 337, 31, 0},
};

const MuseumItemAction kItemActions[] = {
	{kMuseumLaboratory, 7, 2, 326},
	{kMuseumLaboratory, 0, 2, 15},
	{kMuseumLaboratory, 5, 3, 328},
	{kMuseumLaboratory, 7, 3, 327},
	{kMuseumLaboratory, 8, 3, 329},
	{kMuseumLaboratory, 11, 3, 330},
	{kMuseumLaboratory, 12, 3, 330},
	{kMuseumLaboratory, 15, 3, 331},
	{kMuseumLaboratory, 0, 3, 30},
	{kMuseumLaboratory, 14, 4, 333},
	{kMuseumLaboratory, 0, 4, 332},
	{kMuseumLaboratory, 0, 5, 334},
	{kMuseumLaboratory, 0, 7, 19},
	{kMuseumLaboratory, 12, 10, 336},
	{kMuseumLaboratory, 0, 10, 335},
	{kMuseumLaboratory, 0, 11, 15},
};

const MuseumExit kExits[] = {
	{kMuseumLaboratory, 339, kMuseumHall, 873, 311, true, true},
};

} // End of anonymous namespace

const MuseumRoom kRunawayScene4050 = {
	kMuseumLaboratory, 1552, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {60, 60, 70}, {70, 70, 80}, {60, 60, 70}},
	Common::Point(382, 529),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	kExits, ARRAYSIZE(kExits)
};

void RunawayMuseumScene::updateMaps4050() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (id == 4 || id == 15 || id == 19 || (_talcTaken && id == 14))
			id = 0;
		else if (id == 16)
			id = 13;
		else if (id == 17)
			id = 10;
		else if (id == 18)
			id = 3;
	}
}

RunawayMuseumScene::ActionResult RunawayMuseumScene::execute4050(uint16 action) {
	switch (action) {
	case 323: // Take the varnish from the briefcase.
		if (_varnishTaken) {
			say(19, 2);
			return kHandled;
		}
		_varnishTaken = true;
		_inventory.add(9);
		say(19);
		say(19, 1);
		return kHandled;
	case 338: // Take the talcum powder.
		_talcTaken = true;
		_inventory.add(10);
		say(31);
		return kBackgroundChanged;
	default:
		break;
	}
	return kUnhandled;
}

float RunawayMuseumScene::actorHeight4050() const {
	const int x = _position.x, y = _position.y;
	const byte zone = walkZone();
	if (zone == 4)
		return y * 1.3125f - 341.0625f;
	if (zone == 3)
		return x * 0.089147285f + 291.697662f + y - 578;
	if (zone == 5)
		return 291.697662f + 992 * 0.089147285f - (583 - y) * 0.5882353f - 1;
	if (x < 396)
		return 327;
	if (x < 913)
		return x * 0.089147285f + 291.697662f;
	if (x < 1174)
		return x * 0.09578544f + 285.643677f;
	return MIN(y, 587) * 0.8181818f - 82.272728f;
}

bool RunawayMuseumScene::loadScene4050() {
	return _ambient.load(_archive, 8, 75) && _overlay.load(_archive, 22, 1) &&
		loadForeground(0, 4) && loadForeground(1, 5) && loadForeground(2, 6) && loadForeground(3, 7);
}

bool RunawayMuseumScene::applyBackground4050() {
	return !_talcTaken || applyPatch(11);
}

void RunawayMuseumScene::drawForeground4050(Graphics::ManagedSurface &viewport) {
	if (_position.x > 1200)
		drawForeground(viewport, 0);
	if (_position.x > 789 && _position.x < 846 && _position.y < 541)
		drawForeground(viewport, 1);
	if (_position.x < 430)
		drawForeground(viewport, 2);
	if (_position.y < 501)
		drawForeground(viewport, 3);
}

} // End of namespace Hollywood
