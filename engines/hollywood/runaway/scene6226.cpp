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
#include "hollywood/runaway/scene6226.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1023, 331, 3, {301, 302}, {0, 1}, {1023, 331}, 5}, // 1: Door.
	{1001, 245, 3, {301, 303}, {0, 1}, {1001, 245}, 5}, // 2: Door.
	{983, 216, 3, {301, 304}, {0, 1}, {983, 216}, 5}, // 3: Door.
	{834, 370, 7, {305, 306}, {0, 0}, {0, 0}}, // 4: Street.
	{857, 326, 1, {309, 310}, {0, 1}, {872, 312}} // 5: Object on the floor.
};

const RunawayResponse kResponses[] = {
	{6226, 301, 20, 0}, // Look at a door to Kevin's workshop.
	{6226, 305, 21, 0}, // Look down at the street.
	{6226, 306, 22, 0}  // Refuse to jump from the balcony.
};

const RunawayItemAction kItemActions[] = {
	{6226, 21, 4, 307},        // Throw the flower pot into the trough.
	{6226, 0, 4, 308, false}  // Refuse to throw another item into the street.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6226 = {
	6226, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {95, 90, 70}, {65, 60, 40}, {55, 50, 30}}, Common::Point(864, 348),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene6226::Scene6226(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene6226::loadWalkMap() {
	return loadRunawayMask(_archive, 1024, 600, _mask, _originalMaps, 4);
}

bool Scene6226::applyCustomBackgroundPatches() {
	return _town.sandingBlock != kTaken || applyPatch(11);
}

void Scene6226::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[256 + i] == 5 && _town.sandingBlock == kTaken) {
			_maps[256 + i] = 0;
		}
	}
}

void Scene6226::resumeAmbientAudio(bool entering) {
	if (!_town.kevinAway && (entering || !_audio.musicPlaying())) {
		_audio.playMusic(12 + _random.getRandomNumber(4), false);
	}
}

void Scene6226::runCustomEntrySequence() {
	static const Common::Point entries[] = {
		Common::Point(844, 230), Common::Point(836, 263), Common::Point(864, 348)
	};
	_position = entries[_entryScene - 6226];
	_walkPath.reset(_position);
	_facing = 5;
}

float Scene6226::actorHeight() const {
	return _position.y * 1.14457834f - 138.253006f;
}

void Scene6226::setupActorDepth(RunawayActor::RenderParams &params) const {
	static const uint16 depth[] = {0, 263, 195, 490, 396, 227, 247, 349, 322, 0};
	for (uint i = 1; i < ARRAYSIZE(depth); ++i) {
		params.depthY[i] = depth[i];
	}
}

void Scene6226::findWalkPath(Common::Point target) {
	walkTown(target);
}

bool Scene6226::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = CLIP<int>(target.x, 834, 908);
	return false;
}

Common::U32String Scene6226::hotspotName(byte id) const {
	if (id == 5 && _town.sandingBlock == kIdentified) {
		return _strings.hotspotName(_room, 6);
	}
	return RunawayRoomScene::hotspotName(id);
}

RunawayRoomScene::ActionResult Scene6226::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Enter Kevin's workshop through the nearest door.
		leaveTownRoom(6301);
		break;
	case 303: // Enter Kevin's workshop through the middle door.
		leaveTownRoom(6302);
		break;
	case 304: // Enter Kevin's workshop through the far door.
		leaveTownRoom(6303);
		break;
	case 307: // Knock the lost wrench out of the trough with the flower pot.
		if (!_town.wrenchThrown || !_town.wrenchInTroughSeen) {
			say(23);
		} else {
			say(23, 1);
			beginRoomAnimation(307, 12, 49);
		}
		break;
	case 1307: // The pot dislodges the wrench and lands on the porch.
		_audio.playEffect(62);
		_inventory.remove(21);
		_town.wrenchThrown = 2;
		_town.wrenchFound = 1;
		say(23, 2);
		break;
	case 308: // Refuse to throw an unsuitable item from the balcony.
		say(24, _town.wrenchThrown == 1);
		break;
	case 309: // Identify the sanding block on the floor.
		say(25, _town.sandingBlock != kUnexamined);
		_town.sandingBlock = kIdentified;
		break;
	case 310: // Take the sanding block.
		if (_town.sandingBlock == kUnexamined) {
			say(25);
		}
		say(26);
		deferTownAction(410);
		break;
	case 410: // Put the sanding block in Brian's inventory.
		_facing = 4;
		_town.sandingBlock = kTaken;
		_inventory.add(30);
		say(26, 1);
		return kBackgroundChanged;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
