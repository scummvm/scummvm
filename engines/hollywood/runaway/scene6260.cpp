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

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Town", 0, 510, 5, {301, 302}, {0, 3}},
	{"Driver's cab", 363, 395, 7, {303, 304}, {0, 1}},
	{"Smokestack", 495, 433, 1, {305, 306}, {0, 1}},
	{"Water tank opening", 432, 411, 0, {307, 308}, {0, 0}},
	{"Key", 413, 485, 7, {309, 310}, {1, 1}},
	{"Whistle", 257, 430, 1, {311, 312}, {0, 0}},
	{"Outskirts of town", 1023, 437, 2, {315, 316}, {0, 3}}
};

const RunawayResponse kResponses[] = {
	{6260, 301, 1, 0},  // Look toward Douglasville.
	{6260, 303, 2, 0},  // Examine the damaged cab.
	{6260, 305, 4, 0},  // Examine the smokestack.
	{6260, 306, 5, 0},  // Look inside the sooty smokestack.
	{6260, 307, 6, 0},  // Identify the water inlet.
	{6260, 308, 7, 0},  // The inlet has no cap to open.
	{6260, 311, 10, 0}, // Examine the whistle.
	{6260, 312, 11, 0}  // The whistle is operated from the cab.
};

const RunawayItemAction kItemActions[] = {
	{6260, 23, 4, 313}, // Pour water from the oil can into the tank.
	{6260, 24, 4, 314}  // Try to pour from the bottle.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6260 = {
	6260, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {100, 90, 80}},
	Common::Point(123, 469),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

void RunawayRoomScene::start6260() {
	if (_entryScene == 6261) {
		beginRoomAnimation(401, 5, 88, nullptr, 0, true, 60);
	} else if (_entryScene == 6262) {
		// The steam shot is seen outside before returning to the cab.
		beginRoomAnimation(402, 9, 45, nullptr, 0, true, 60);
		_actionFrames.insert_at(0, 0);
	} else {
		const bool outskirts = _entryScene == 6265;
		_position = outskirts ? Common::Point(825, 458) : Common::Point(123, 469);
		_walkPath.reset(_position);
		_facing = outskirts ? 7 : 1;
		if (!_town.locomotiveVisited) {
			say(0);
			deferTownAction(400);
		}
	}
}

bool RunawayRoomScene::applyBackground6260() {
	const bool keyVisible = _town.locomotiveKey == 1 || _town.locomotiveKey == 2;
	return applyPatch(keyVisible ? 8 : 7);
}

void RunawayRoomScene::updateMaps6260() {
	for (uint i = 0; i < 256; ++i) {
		// The upper zone is reserved for approaches to the cab and inlet.
		if (_maps[i] == 2) {
			_maps[i] = 0;
		}
		if (_maps[256 + i] == 5 && (_town.locomotiveKey == 0 || _town.locomotiveKey == 3)) {
			_maps[256 + i] = 0;
		}
	}
}

void RunawayRoomScene::walk6260(Common::Point target) {
	if (!_pendingHotspot) {
		// Ordinary clicks stay on the ground, even when Brian starts above it.
		target.x = CLIP<int>(target.x, 0, _background.w - 1);
		target.y = CLIP<int>(target.y, 0, _background.h - 1);
		while (target.y < _background.h - 1 && !_maps[_mask[target.y * _background.w + target.x]]) {
			++target.y;
		}
		while (target.y > 0 && !_maps[_mask[target.y * _background.w + target.x]]) {
			--target.y;
		}
	}
	_walkPath.findPath(RunawayWalkMap(_background.w, _background.h, _mask, _originalMaps), target);
}

RunawayRoomScene::ActionResult RunawayRoomScene::execute6260(uint16 action) {
	switch (action) {
	case 302: // Return to the street.
		leaveTownRoom(6225);
		break;
	case 304: // Climb through the cab's broken window.
		if (!_town.cabVisited) {
			say(3);
		}
		beginRoomAnimation(304, 4, 131, nullptr, 0, true, 60);
		break;
	case 1304:
		leaveTownRoom(6263);
		break;
	case 309: // Identify the key ejected from the smokestack.
		_facing = 4;
		say(8);
		if (_town.locomotiveKey == 1) {
			_town.locomotiveKey = 2;
		}
		break;
	case 310: // Pick up the cell key.
		if (_town.locomotiveKey == 1) {
			execute6260(309);
		}
		deferTownAction(410);
		break;
	case 410:
		_town.locomotiveKey = 3;
		_inventory.add(9);
		say(9);
		return kBackgroundChanged;
	case 313: // Fill the tank, one oil can at a time.
		if (!_town.oilCanFull) {
			say(12);
		} else if (_town.waterTank == 5) {
			say(12, 1);
		} else {
			say(12, _town.waterTank ? 3 : 2);
			byte frames[114];
			for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
				if (i <= 37) {
					frames[i] = i;
				} else if (i < 80) {
					frames[i] = 37;
				} else if (i < 85) {
					frames[i] = i - 42;
				} else {
					frames[i] = i < 104 ? 104 - i : 0;
				}
			}
			beginRoomAnimation(313, 10, 43, frames, ARRAYSIZE(frames), true, 60);
			_inventory.select(0);
		}
		break;
	case 1313:
		++_town.waterTank;
		_town.oilCanFull = 0;
		break;
	case 314: // The bottle cannot reach the tank opening.
		if (_town.bottleContents == 1) {
			say(13, 2);
			deferTownAction(414);
		} else {
			say(13, _town.bottleContents == 2 ? 1 : 0);
		}
		break;
	case 414:
		_facing = 4;
		say(13, _town.waterTank ? 4 : 3);
		break;
	case 315: // Look toward the crater.
		say(16, _town.craterSeen);
		break;
	case 316: // Leave through the outskirts.
		leaveTownRoom(6350);
		break;
	case 400: // First visit: explain the locomotive's fittings.
		walkTo(_entryScene == 6265 ? Common::Point(524, 475) : Common::Point(222, 435),
			_entryScene == 6265 ? 7 : 1);
		deferTownAction(403);
		break;
	case 403:
		say(0, 1);
		_town.locomotiveVisited = 1;
		break;
	case 1401: // Finish climbing out of the cab.
		_position = Common::Point(363, 395);
		_walkPath.reset(_position);
		_facing = 3;
		break;
	case 1402: // The burst has dislodged the sheriff's key.
		_town.locomotiveKey = 1;
		leaveTownRoom(6264);
		return kBackgroundChanged;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Hollywood
