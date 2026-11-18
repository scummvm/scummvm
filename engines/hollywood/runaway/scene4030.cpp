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
	{"Door", 1165, 594, 2, {322, 301}, {0, 0}},
	{"Pair of books", 355, 595, 1, {302, 303}, {0, 1}},
	{"Crack", 355, 595, 1, {304, 305}, {1, 0}},
	{"Money", 550, 647, 7, {306, 307}, {0, 0}},
	{"Papers", 550, 647, 7, {308, 319}, {1, 0}},
	{"Device", 550, 647, 7, {309, 310}, {1, 1}},
	{"Shelves", 815, 508, 1, {311, 312}, {0, 0}},
	{"Secret safe", 550, 647, 7, {313, 314}, {0, 0}},
	{"Shelves", 104, 536, 7, {315, 316}, {0, 0}},
	{"Window", 369, 499, 7, {317, 318}, {1, 0}},
};

const RunawayResponse kResponses[] = {
	{kMuseumOffice, 305, 4, 0},
	{kMuseumOffice, 306, 5, 0},
	{kMuseumOffice, 307, 6, 0},
	{kMuseumOffice, 311, 11, 0},
	{kMuseumOffice, 312, 12, 0},
	{kMuseumOffice, 313, 13, 0},
	{kMuseumOffice, 314, 14, 0},
	{kMuseumOffice, 315, 15, 0},
	{kMuseumOffice, 316, 16, 0},
	{kMuseumOffice, 317, 17, 0},
	{kMuseumOffice, 318, 18, 0},
	{kMuseumOffice, 319, 8, 0},
	{kMuseumOffice, 320, 19, 0},
	{kMuseumOffice, 322, 22, 0},
};

const RunawayItemAction kItemActions[] = {
	{kMuseumOffice, 17, 3, 321},
	{kMuseumOffice, 0, 3, 320},
};

const RunawayExit kExits[] = {
	{kMuseumOffice, 301, kMuseumHall, 607, 323, true, true},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene4030 = {
	kMuseumOffice, 1200, 670, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {50, 50, 50}, {80, 80, 60}},
	Common::Point(934, 563),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	kExits, ARRAYSIZE(kExits)
};

void RunawayRoomScene::updateMaps4030() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (_books != kTaken && (id == 2 || id == 3))
			id = 2;
		else if (_books == kTaken && id == 2)
			id = 0;
		if (!_safeOpen && (id == 4 || id == 5 || id == 6 || id == 8))
			id = 0;
		else if (_safeOpen && id == 8)
			id = 0;
		if (_recorder == kTaken && id == 6)
			id = 4;
	}
}

Common::U32String RunawayRoomScene::hotspotName4030(byte id) const {
	if (id == 2 && _books != kUnexamined)
		return _inventory.name(16);
	if (id == 5)
		return _moneyIdentified ? Common::U32String("Money") : Common::U32String("Papers");
	if (id == 6)
		return _recorder != kUnexamined ? _inventory.name(5) : Common::U32String("Device");
	return Common::U32String(hotspot(id)->name);
}

RunawayRoomScene::ActionResult RunawayRoomScene::execute4030(uint16 action) {
	if (action == 301 && _safeOpen) {
		say(21);
		return kHandled;
	}
	switch (action) {
	case 302: // Examine the books.
		say(_books != kUnexamined ? 111 : 1, 0, _books != kUnexamined);
		_books = MAX(_books, kIdentified);
		return kHandled;
	case 303: // Take the books, revealing the crack.
		_books = kTaken;
		_inventory.add(16);
		say(2);
		say(2, 1);
		say(2, 2);
		return kBackgroundChanged;
	case 304:
		say(3, _safeOpen);
		return kHandled;
	case 308:
		say(7, _moneyIdentified);
		_moneyIdentified = true;
		return kHandled;
	case 309:
		say(9, _recorder != kUnexamined);
		_recorder = MAX(_recorder, kIdentified);
		return kHandled;
	case 310: // Take the recorder.
		_recorder = kTaken;
		_inventory.add(5);
		say(10);
		say(10, 1);
		return kBackgroundChanged;
	case 321: // Toggle the safe with the strange key.
		_safeOpen = !_safeOpen;
		_inventory.select(0);
		if (_safeOpen) {
			say(20);
			say(20, 1);
		}
		return kBackgroundChanged;
	default:
		break;
	}
	return kUnhandled;
}

float RunawayRoomScene::actorHeight4030() const {
	const int x = _position.x, y = _position.y;
	return x < 459 ? y * 0.369863f + 208.301376f : y * 0.516304f + 110.625f;
}

bool RunawayRoomScene::loadScene4030() {
	return loadForeground(0, 4) && loadForeground(1, 5) && loadForeground(2, 6) &&
		loadForeground(3, 7) && loadForeground(4, 12);
}

bool RunawayRoomScene::applyBackground4030() {
	if (_books == kTaken && !applyPatch(8))
		return false;
	if (!_safe.load(_archive, _recorder == kTaken ? 15 : 14, 13))
		return false;
	_safe.draw(_background, _safeOpen ? 12 : 0, 0, 0);
	return true;
}

void RunawayRoomScene::drawForeground4030(Graphics::ManagedSurface &viewport) {
	const byte zone = walkZone();
	if (zone == 3)
		drawForeground(viewport, 0);
	if (zone == 4 && _books == kUnexamined)
		drawForeground(viewport, 1);
	if (zone == 5)
		drawForeground(viewport, 2);
	drawForeground(viewport, 3);
	if (zone == 4 && _books != kUnexamined)
		drawForeground(viewport, 4);
}

} // End of namespace Hollywood
