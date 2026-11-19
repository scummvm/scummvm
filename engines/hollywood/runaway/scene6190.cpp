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
#include "hollywood/runaway/scene6190.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Town", 1250, 347, 3, {301, 302}, {0, 3}},
	{"Bucket of paint", 560, 282, 0, {303, 304}, {1, 0}},
	{"Hole", 757, 446, 5, {306, 307}, {1, 0}},
	{"Shelves", 778, 314, 7, {308, 309}, {1, 1}},
	{"Metal object", 778, 314, 7, {310, 311}, {0, 1}},
	{"Desk", 479, 232, 5, {312, 313}, {1, 1}},
	{"Cash register", 759, 334, 4, {314, 315}, {0, 1}},
	{"Couch", 601, 391, 5, {316, 317}, {1, 0}}
};

const RunawayResponse kResponses[] = {
	{6190, 301, 1, 0},   // Look toward town.
	{6190, 303, 2, 0},   // Look at the bucket of paint.
	{6190, 304, 3, 0},   // Try to take the bucket of paint.
	{6190, 306, 5, 0},   // Look at the hole in the floor.
	{6190, 310, 9, 0},   // Look at the metal object.
	{6190, 312, 11, 0},  // Look at Douglas's desk.
	{6190, 314, 13, 0},  // Look at the cash register.
	{6190, 315, 14, 0},  // Open the empty cash register.
	{6190, 316, 15, 0},  // Look at the couch.
	{6190, 317, 16, 0}   // Try to sit on the couch.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6190 = {
	6190, 1460, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {95, 90, 70}, {65, 60, 40}, {65, 60, 40}, {75, 70, 50}, {90, 85, 65}, {95, 90, 70}},
	Common::Point(1250, 347),
	kResponses, ARRAYSIZE(kResponses), nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene6190::Scene6190(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene6190::usesAlternateActor() const {
	return false;
}

bool Scene6190::applyCustomBackgroundPatches() {
	return (!_town.bankStampTaken || applyPatch(8)) &&
		(_town.bankShelves != 2 || applyPatch(9));
}

void Scene6190::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		// The bucket only appears after the catapult has hit the bank.
		if (id == 2) {
			id = 0;
		} else if (id == 5 && _town.bankShelves != 1) {
			id = 4;
		}
	}
}

void Scene6190::runCustomEntrySequence() {
	if (!_town.bankVisited) {
		walkTo(Common::Point(1075, 330), 7);
		deferTownAction(400);
	} else {
		walkTo(Common::Point(669, 413), 7);
	}
}

float Scene6190::actorHeight() const {
	return (_position.y * 0.426829278f + 16.4634151f + _position.x * 0.0980392173f + 41.7647057f) / 2;
}

void Scene6190::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
	params.depthY[2] = 388;
}

RunawayRoomScene::ActionResult Scene6190::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Return to the street.
		leaveTownRoom(6224);
		break;
	case 307: // Try to enter the rubble-filled hole.
		say(6, _town.basementKnown);
		break;
	case 308: // Examine the shelves and reveal the metal object.
		say(7, _town.bankShelves);
		if (!_town.bankShelves) {
			_town.bankShelves = 1;
		}
		return kBackgroundChanged;
	case 309: // Search the shelves.
		say(8, _town.bankShelves == 2);
		if (_town.bankShelves != 2) {
			_town.bankShelves = 1;
		}
		return kBackgroundChanged;
	case 311: // Take the ancient stapler and its staples.
		say(10);
		deferTownAction(411);
		break;
	case 411: // Collect the ancient stapler.
		_town.bankShelves = 2;
		_inventory.add(20);
		say(10, 1);
		return kBackgroundChanged;
	case 313: // Search the desk and take the bank stamp.
		if (_town.bankStampTaken) {
			say(12, 2);
		} else {
			_town.bankStampTaken = 1;
			_inventory.add(32);
			say(12, 1);
		}
		return kBackgroundChanged;
	case 400: // First visit: inspect the damage from the locomotive.
		say(0);
		deferTownAction(401);
		break;
	case 401: // Walk farther into the bank on the first visit.
		walkTo(Common::Point(669, 413), 7);
		deferTownAction(402);
		break;
	case 402: // Finish Brian's first inspection of the ruined bank.
		say(0, 1);
		_town.bankVisited = 1;
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
