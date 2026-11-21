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
#include "hollywood/runaway/scene6180.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{202, 526, 7, {301, 302}, {0, 3}, {189, 517}, 4}, // 1: Street.
	{780, 527, 1, {303, 304}, {0, -1}, {1023, 519}, 5}, // 2: Inside the saloon.
	{405, 427, 7, {305, 306}, {0, 0}, {375, 402}}, // 3: Trough.
	{780, 527, 1, {307, 308}, {0, -1}, {1023, 519}}, // 4: Floor board.
	{643, 457, 1, {309, 310}, {0, 0}, {729, 434}} // 5: Window.
};

const RunawayResponse kResponses[] = {
	{6180, 301, 1, 0},  // Look out into the deserted street.
	{6180, 303, 2, 0},  // Look inside the saloon.
	{6180, 307, 6, 0},  // Examine the doors' pressure plate.
	{6180, 309, 8, 0},  // Read the broken window.
	{6180, 310, 9, 0},  // Refuse to break more glass.
	{6180, 311, 10, 0}  // Refuse to collect mud with an inventory item.
};

const RunawayItemAction kItemActions[] = {
	{6180, 8, 3, 311, false}, {6180, 12, 3, 311, false},
	{6180, 13, 3, 311, false}, {6180, 16, 3, 311, false},
	{6180, 23, 3, 311, false}, {6180, 24, 3, 311, false},
	{6180, 25, 3, 311, false}, {6180, 26, 3, 311, false},
	{6180, 31, 3, 311, false}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6180 = {
	6180, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {80, 75, 65}, {60, 55, 45}, {50, 45, 35}}, Common::Point(319, 549),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene6180::Scene6180(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

const char *Scene6180::actionCaptionVariant(byte id, bool look, byte item) const {
	if (!look && !item && id == 3 && _town.wrenchThrown) {
		return _town.wrenchThrown == 1 ? "wrench" : "pot";
	}
	return nullptr;
}

bool Scene6180::loadCustomResources() {
	return _doors.load(_archive) && loadForeground(0, 4) && loadForeground(1, 5);
}

bool Scene6180::applyCustomBackgroundPatches() {
	return !_town.wrenchFound || applyPatch(_town.wrenchFound == 1 ? 10 : 11);
}

void Scene6180::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[i] == 3) {
			_maps[i] = 0;
		}
		if (_maps[256 + i] == 4 && !_town.saloonDoorKnown) {
			_maps[256 + i] = 0;
		}
	}
}

void Scene6180::prepareCustomGameplayLoop() {
	_doors.resume(_now, _position.x > 726);
}

void Scene6180::runCustomEntrySequence() {
	if (_entryScene == 6181) {
		_position = Common::Point(1023, 519);
		_walkPath.reset(_position);
		_doors.resume(_now, true);
		if (_town.wrenchFound == 1) {
			walkTo(Common::Point(434, 433), 7, false, true);
			deferTownAction(410);
		} else if (_town.wrenchThrown == 1 && !_town.wrenchInTroughSeen) {
			walkTo(Common::Point(586, 534), 7, false, true);
			deferTownAction(412);
		} else {
			walkTo(Common::Point(586, 534), 5, false, true);
		}
	} else {
		_facing = 3;
		if (!_town.saloonEntranceVisited) {
			_town.saloonEntranceVisited = 1;
			say(0);
		}
	}
}

void Scene6180::advanceCustomGameplayLoop(uint32 delta) {
	if (_roomAction == 401 && !_actionFrames.empty()) {
		_actionFrames[0] = 24 + speechFrame();
	}
	_doors.update(_now, _position.x > 726, _audio, 13);
}

void Scene6180::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	_doors.draw(viewport, _position.x > 726);
}

void Scene6180::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_position.x < 273) {
		drawForeground(viewport, 0);
	}
	if (_position.x > 714) {
		drawForeground(viewport, 1);
	}
}

float Scene6180::actorHeight() const {
	return _position.y * 1.3565892f - 249.069763f;
}

void Scene6180::findWalkPath(Common::Point target) {
	walkTown(target);
}

bool Scene6180::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	if (_originalMaps[_mask[target.y * _background.w + target.x]] == 3) {
		return true;
	}
	target.x = CLIP<int>(target.x, 202, 679);
	return false;
}

RunawayRoomScene::ActionResult Scene6180::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Leave the porch and return to Douglasville.
		leaveTownRoom(6221);
		break;
	case 304: // Enter through the automatic saloon doors.
		if (!_town.saloonDoorKnown) {
			walkTo(Common::Point(780, 527), 1, false, true);
			deferTownAction(400);
		} else {
			walkTo(Common::Point(1023, 519), 1, false, true);
			deferTownAction(403);
		}
		break;
	case 305: // Examine the trough before or after the wrench lands in it.
		say(4, _town.wrenchThrown);
		break;
	case 306: // Try to overturn the trough, retrieve the buried wrench, or take the flower pot.
		say(5, _town.wrenchThrown);
		break;
	case 308: // Step on the board, or point out that Brian is already on it.
		if (_position == Common::Point(780, 527)) {
			say(7);
		} else {
			walkTo(Common::Point(780, 527), 1, false, true);
		}
		break;
	case 400: // Notice the doors opening without being pushed.
		beginRoomAnimationRange(400, 9, 42, 0, 24);
		break;
	case 1400: // Comment on the automatic saloon doors.
		say(3);
		holdRoomAnimation(401, 9, 42, 24);
		break;
	case 1401: { // Finish Brian's reaction to the automatic doors.
		static const byte frames[] = {24, 36, 37, 38, 39, 40};
		beginRoomAnimation(402, 9, 42, frames, ARRAYSIZE(frames));
		break;
	}
	case 1402: // Reveal the pressure plate and walk through the doors.
		_town.saloonDoorKnown = 1;
		walkTo(Common::Point(1023, 519), 1, false, true);
		deferTownAction(403);
		return kBackgroundChanged;
	case 403: // Enter the saloon after crossing the porch.
		leaveTownRoom(6240);
		break;
	case 410: // Notice the wrench knocked out of the trough by the flower pot.
		say(0, 3);
		deferTownAction(411);
		break;
	case 411: // Pick up the number-ten wrench for Joshua.
		_facing = 4;
		_town.wrenchFound = 2;
		_inventory.add(10);
		say(0, 4);
		return kBackgroundChanged;
	case 412: // Find the wrench buried in the muddy trough.
		say(0, 1);
		walkTo(Common::Point(405, 427), 7, false, true);
		deferTownAction(413);
		break;
	case 413: // Brian needs something to dislodge the wrench.
		_facing = 4;
		_town.wrenchInTroughSeen = 1;
		say(0, 2);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
