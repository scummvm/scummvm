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
	{"Path", 0, 550, 0, {301, 302}, {0, 3}},
	{"Recipient", 95, 358, 7, {305, 306}, {0, 1}},
	{"Hole that Gina fell into", 483, 318, 0, {303, 304}, {1, 1}},
	{"Stone", 656, 347, 1, {307, 308}, {0, 0}},
	{"Entrance to the mine", 656, 347, 1, {309, 310}, {0, 1}},
	{"Mining car", 0, 0, 0, {311, 312}, {0, 0}},
	{"Chest", 0, 0, 4, {313, 314}, {0, 0}}
};

const RunawayResponse kResponses[] = {
	{6080, 301, 1, 0},   // Look toward the helicopter's landing place.
	{6080, 311, 10, 0},  // Examine the mining car's broken wheel.
	{6080, 312, 11, 0},  // Try to move the car.
	{6080, 313, 12, 0},  // Examine the empty tool chest.
	{6080, 314, 13, 0},  // Refuse to take the chest.
	{6080, 315, 14, 0},  // Throw an item down the shaft.
	{6080, 316, 15, 0},  // Try to lower the rope.
	{6080, 317, 16, 0}   // The strap is too short for the shaft.
};

const RunawayItemAction kItemActions[] = {
	{6080, 11, 3, 317, false}, // Use the strap at the shaft.
	{6080, 19, 3, 316, false}, // Use the rope at the shaft.
	{6080, 0, 3, 315, false}   // Throw another item down the shaft.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6080 = {
	6080, 1412, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {95, 90, 80}}, Common::Point(141, 457),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

void RunawayRoomScene::start6080() {
	if (_entryScene == 6081) {
		_position = Common::Point(483, 318);
		_walkPath.reset(_position);
		_facing = 0;
	} else if (_entryScene == 6082) {
		_position = Common::Point(844, 362);
		_walkPath.reset(_position);
		_facing = 7;
		_mineOscarPresent = true;
		say(20, 0, false, 1);
		say(20, 1);
		say(20, 2, false, 1);
		beginRoomAnimationRange(403, 9, 135, 6, 133, false, 60);
	} else {
		_facing = 1;
		if (!_town.mineVisited) {
			say(0);
			deferTownAction(400);
		}
	}
}

void RunawayRoomScene::updateMaps6080() {
	for (uint i = 0; i < 256; ++i) {
		// Zone 6 is only used by scripted approaches to the mine.
		if (_maps[i] == 6) {
			_maps[i] = 0;
		}
		byte &id = _maps[256 + i];
		if (id == 2 && _town.oilCan == 2) {
			id = 0;
		} else if (id == 4 && _town.rockMoved) {
			id = 5;
		}
	}
}

void RunawayRoomScene::drawScene6080(Graphics::ManagedSurface &viewport, bool foreground) {
	if (foreground) {
		if (_position.x > 737) {
			drawForeground(viewport, 0);
		}
		if (_position.x > 754 && _position.y < 398) {
			drawForeground(viewport, 1);
		}
		return;
	}
	if (!_mineOscarPresent || _town.rockMoved) {
		_ambient.draw(viewport, _town.rockMoved, _camera.x, -_camera.y);
	}
	if (_mineOscarPresent && !drawingRoomAnimation()) {
		const byte mouth = speaker() ? speechFrame() % 6 : 0;
		const uint frame = _town.rockMoved ? (mouth ? 128 + mouth : 134) : mouth;
		_npc.draw(viewport, frame, _camera.x, -_camera.y);
	}
}

RunawayRoomScene::ActionResult RunawayRoomScene::execute6080(uint16 action) {
	switch (action) {
	case 302: // Return to the overview.
		leaveTownRoom(6350);
		break;
	case 303: // Look down the shaft.
	case 304: // Try to descend its broken ladder.
		leaveTownRoom(action == 303 ? 6014 : 6015);
		break;
	case 305: // Identify the oil can.
		say(4, _town.oilCan != 0);
		if (!_town.oilCan) {
			_town.oilCan = 1;
		}
		break;
	case 306: // Take the oil can.
		if (!_town.oilCan) {
			execute6080(305);
		}
		deferTownAction(406);
		break;
	case 406:
		_town.oilCan = 2;
		_inventory.add(23);
		say(5);
		return kBackgroundChanged;
	case 307: // Examine the rock blocking the mine.
		say(6, _town.rockMoved ? 1 : _town.sushiMet ? 2 : 0);
		break;
	case 308: // Try to move the rock.
		say(7, _town.rockMoved);
		break;
	case 309: // Examine the entrance.
		say(8, _town.rockMoved);
		break;
	case 310: // Enter only with the map, light and rope.
		if (!_town.rockMoved) {
			say(9);
		} else if (!_inventory.contains(17)) {
			say(9, 1);
		} else if (!_inventory.contains(18) || !_inventory.contains(19)) {
			say(9, 2);
		} else {
			leaveTownRoom(7000);
		}
		break;
	case 400: // First visit: approach the blocked entrance.
		walkTo(Common::Point(656, 347), 1);
		deferTownAction(401);
		break;
	case 401:
		say(6, 2);
		_town.mineVisited = 1;
		break;
	case 1403: // Oscar has pushed the rock aside.
		_town.rockMoved = 1;
		say(20, 3, false, 1);
		say(20, 4);
		say(20, 5, false, 1);
		say(20, 6);
		say(20, 7, false, 1);
		deferTownAction(404);
		return kBackgroundChanged;
	case 404: // Watch Oscar head back to the house.
		_facing = 5;
		beginRoomAnimation(404, 10, 128, nullptr, 0, false, 60);
		break;
	case 1404:
		_mineOscarPresent = false;
		walkTo(Common::Point(734, 402), 5, true);
		deferTownAction(405);
		break;
	case 405:
		say(20, 8);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void RunawayRoomScene::mineAnimationFrame() {
	if (_roomAction == 403) {
		const uint frame = _actionFrames[_actionStep];
		if (frame == 41) {
			_audio.playEffect(64);
		}
		if (frame == 18 || frame == 26 || frame == 33 || frame == 44 || frame == 53 ||
				frame == 62 || frame == 82 || frame == 93 || frame == 111 || frame == 118 || frame == 125) {
			_audio.playFootstep(2, _random.getRandomBit(), 77);
		}
	} else if (_roomAction == 404) {
		static const byte footsteps[] = {10, 18, 25, 31, 37, 44, 50, 55, 62, 68, 74, 80, 87, 93, 99, 105, 112, 118, 124};
		for (uint i = 0; i < ARRAYSIZE(footsteps); ++i) {
			if (_actionStep == footsteps[i]) {
				_audio.playFootstep(2, _random.getRandomBit(), MIN<uint>(255, 51 + i * 13));
				break;
			}
		}
	}
}

} // End of namespace Hollywood
