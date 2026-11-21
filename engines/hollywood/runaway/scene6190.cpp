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
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1250, 347, 3, {301, 302}, {0, 3}, {1459, 435}, 6}, // 1: Town.
	{560, 282, 0, {303, 304}, {1, 0}, {550, 255}}, // 2: Bucket of paint.
	{757, 446, 5, {306, 307}, {1, 0}, {656, 528}}, // 3: Hole.
	{778, 314, 7, {308, 309}, {1, 1}, {747, 290}}, // 4: Shelves.
	{778, 314, 7, {310, 311}, {0, 1}, {747, 290}}, // 5: Metal object.
	{479, 232, 5, {312, 313}, {1, 1}, {450, 251}}, // 6: Desk.
	{759, 334, 4, {314, 315}, {0, 1}, {763, 334}}, // 7: Cash register.
	{601, 391, 5, {316, 317}, {1, 0}, {508, 480}} // 8: Couch.
};

const RunawayResponse kResponses[] = {
	{6190, 301, 1, 0},   // Look toward town.
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

const char *Scene6190::actionCaptionVariant(byte id, bool look, byte item) const {
	return !look && !item && id == 2 && _town.catapult == 3 ? "basement" : nullptr;
}

bool Scene6190::usesAlternateActor() const {
	return false;
}

bool Scene6190::applyCustomBackgroundPatches() {
	return (!_town.catapult || applyPatch(_town.catapult == 3 ? 5 : 4)) &&
		(!_town.bankStampTaken || applyPatch(8)) &&
		(_town.bankShelves != 2 || applyPatch(9));
}

void Scene6190::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		// The bucket only appears after the catapult has hit the bank.
		if (id == 2 && !_town.catapult) {
			id = 0;
		} else if (id == 5 && _town.bankShelves != 1) {
			id = 4;
		}
	}
}

void Scene6190::runCustomEntrySequence() {
	if (_entryScene == 6191) {
		_position = Common::Point(561, 288);
		_walkPath.reset(_position);
		_facing = 4;
		beginRoomAnimation(405, 7, 24);
	} else if (_town.catapult == 3 && !_town.bankImpactSeen) {
		walkTo(Common::Point(560, 282), 0);
		deferTownAction(403);
	} else if (!_town.bankVisited) {
		walkTo(Common::Point(1075, 330), 7);
		deferTownAction(400);
	} else {
		walkTo(Common::Point(669, 413), 7);
	}
	if (_town.catapult && _town.bankVisited) {
		_town.bankVisited = 2;
	}
}

void Scene6190::handleAnimationFrameHook() {
	if (_roomAction == 405 && (_actionStep == 4 || _actionStep == 14 || _actionStep == 19 || _actionStep == 22)) {
		_audio.playFootstep(3, _random.getRandomBit(), 128, 553);
	}
}

Common::U32String Scene6190::hotspotName(byte id) const {
	if (id == 2 && _town.catapult == 3) {
		return _strings.hotspotName(_room, 9);
	}
	return RunawayRoomScene::hotspotName(id);
}

int8 Scene6190::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (id == 2 && use && _town.catapult == 3) {
		return 1;
	}
	return approach;
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
	case 303: // Examine the paint bucket or the new entrance to the basement.
		say(2, _town.catapult == 3);
		break;
	case 304: // Leave the paint alone, or climb down through the statue's hole.
		if (_town.catapult == 3) {
			if (!_town.basementVisited) {
				say(4);
			}
			beginRoomAnimation(404, 6, 24);
		} else {
			say(3);
		}
		break;
	case 1404: // Enter the bank basement.
		leaveTownRoom(6250);
		break;
	case 1405: // Finish climbing back into the bank.
		_audio.playMusic(12 + _random.getRandomNumber(4), false);
		break;
	case 403: // Inspect the hole made by Saturn's statue.
		_town.bankImpactSeen = 1;
		_town.bankVisited = 2;
		say(0, 2);
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
		_town.bankVisited = _town.catapult ? 2 : 1;
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
