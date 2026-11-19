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
#include "hollywood/runaway/scene6240.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Door", 0, 453, 5, {301, 302}, {0, 3}},
	{"Stairs", 782, 408, 7, {303, 304}, {0, 1}},
	{"Flower pot", 504, 452, 1, {305, 306}, {1, 1}},
	{"Watering can", 1286, 435, 0, {307, 308}, {0, 0}},
	{"Small room", 742, 332, 5, {309, 310}, {0, 0}},
	{"Door knob", 610, 467, 1, {311, 312}, {0, 0}},
	{"Plant", 1393, 427, 0, {313, 314}, {0, 0}},
	{"Bonsai tree", 955, 396, 1, {315, 316}, {0, 0}}
};

const RunawayResponse kResponses[] = {
	{6240, 301, 1, 0},   // Look out into town.
	{6240, 303, 2, 0},   // Look up the stairs.
	{6240, 305, 3, 0},   // Examine the pot of soil.
	{6240, 307, 5, 0},   // Examine the watering can.
	{6240, 308, 6, 0},   // Leave the watering to the plants' owner.
	{6240, 312, 10, 0},  // Try to remove the carved door knob.
	{6240, 315, 13, 0},  // Examine the bonsai.
	{6240, 316, 14, 0},  // Leave the bonsai where it belongs.
	{6240, 317, 15, 0}   // Refuse to water the plants with an inventory item.
};

const RunawayItemAction kItemActions[] = {
	{6240, 16, 7, 317, false}, {6240, 16, 8, 317, false},
	{6240, 24, 7, 317, false}, {6240, 24, 8, 317, false}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6240 = {
	6240, 1728, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {70, 70, 70}, {50, 50, 50}, {40, 40, 40}}, Common::Point(45, 427),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene6240::Scene6240(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene6240::usesAlternateActor() const {
	return false;
}

bool Scene6240::loadCustomResources() {
	if (!_plantAnimation.load(_archive, 4, 36)) {
		return false;
	}
	for (uint i = 0; i < ARRAYSIZE(_plantPatches); ++i) {
		if (!_archive.readVariableChunk(6 + i, _plantPatches[i])) {
			return false;
		}
	}
	return true;
}

bool Scene6240::applyCustomBackgroundPatches() {
	return !_town.flowerPotTaken || applyPatch(9);
}

void Scene6240::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[i] >= 6 && _maps[i] <= 8) {
			_maps[i] = 0;
		}
		if (_maps[256 + i] == 3 && _town.flowerPotTaken) {
			_maps[256 + i] = 0;
		}
	}
}

void Scene6240::runCustomEntrySequence() {
	if (_entryScene == 6241) {
		_position = Common::Point(782, 408);
		_walkPath.reset(_position);
		walkTo(Common::Point(783, 408), 3, false, true);
	} else {
		walkTo(Common::Point(88, 403), _town.saloonVisited ? 1 : 3);
		if (!_town.saloonVisited) {
			deferTownAction(400);
		}
	}
}

void Scene6240::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	_plantAnimation.draw(viewport, 0, _camera.x, -_camera.y);
}

void Scene6240::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	// The original moves these plants within the room before applying the camera.
	applyRunawayPatch(_plantPatches[2], viewport, 0, 440 - _camera.x / 3 - _camera.x);
	applyRunawayPatch(_plantPatches[1], viewport, 0, 800 - _camera.x / 4 - _camera.x);
	applyRunawayPatch(_plantPatches[0], viewport, 0, 820 - _camera.x / 3 - _camera.x);
}

float Scene6240::actorHeight() const {
	return _position.y * 1.00515461f - 83.762886f;
}

void Scene6240::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
	params.depthY[2] = 599;
	params.depthY[3] = 351;
	params.depthY[4] = 351;
}

Common::U32String Scene6240::hotspotName(byte id) const {
	if (id == 5 && _town.gardeningShed) {
		return Common::U32String("Gardening shed");
	}
	if (id == 7 && _town.carnivorousPlantSeen) {
		return Common::U32String("Meat-eating plant");
	}
	return RunawayRoomScene::hotspotName(id);
}

void Scene6240::findWalkPath(Common::Point target) {
	walkTown(target);
}

bool Scene6240::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = MIN<int>(target.x, 1510);
	return false;
}

RunawayRoomScene::ActionResult Scene6240::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Return to the saloon porch.
		leaveTownRoom(6181);
		break;
	case 304: // Climb the stairs to Kevin's room.
		beginRoomAnimation(304, 5, 19);
		break;
	case 1304:
		leaveTownRoom(6300);
		break;
	case 306: // Take the flower pot.
		say(4);
		deferTownAction(406);
		break;
	case 406:
		_town.flowerPotTaken = 1;
		_inventory.add(21);
		say(4, 1);
		return kBackgroundChanged;
	case 309: // Identify the gardening shed.
	case 310: // Search the shed for useful items.
		if (!_town.gardeningShed) {
			say(7);
		} else if (action == 309) {
			say(7, 3);
			break;
		}
		walkTo(Common::Point(742, 332), 5, false, true);
		deferTownAction(action == 309 ? 409 : 410);
		break;
	case 409:
		say(7, 1);
		deferTownAction(419);
		break;
	case 419:
		walkTo(Common::Point(855, 347), 3, false, true);
		deferTownAction(429);
		break;
	case 429:
		say(7, 2);
		_town.gardeningShed = 1;
		break;
	case 410:
		if (!_town.gardeningShed) {
			say(7, 1);
			say(7, 2);
			_town.gardeningShed = 1;
		}
		deferTownAction(420);
		break;
	case 420: // Search the tools; tobacco is useful only after learning about Oscar.
		_audio.playEffect(15);
		if (_town.gardeningShed == 1) {
			say(8);
			deferTownAction(421);
		} else if (_town.oscarTobacco == 1 && !_town.tobaccoLeavesTaken) {
			say(8, 2);
			deferTownAction(422);
		} else {
			say(8, 4);
			deferTownAction(423);
		}
		break;
	case 421:
		_town.gardeningShed = 2;
		_inventory.add(22);
		say(8, 1);
		if (_town.oscarTobacco == 1 && !_town.tobaccoLeavesTaken) {
			say(8, 2);
			deferTownAction(422);
		} else {
			deferTownAction(423);
		}
		break;
	case 422:
		_town.tobaccoLeavesTaken = 1;
		_inventory.add(27);
		say(8, 3);
		deferTownAction(423);
		break;
	case 423: // Step back out of the shed.
		walkTo(Common::Point(855, 347), 3, false, true);
		break;
	case 311: // Recognize the carved artichoke.
		say(9, _town.artichokeSeen);
		_town.artichokeSeen = 1;
		break;
	case 313: // Identify the carnivorous plant.
		say(11, _town.carnivorousPlantSeen);
		_town.carnivorousPlantSeen = 1;
		break;
	case 314: // Refuse to take the carnivorous plant.
		if (!_town.carnivorousPlantSeen) {
			dispatchCustomSceneAction(313);
		}
		say(12, _random.getRandomBit());
		break;
	case 400: // First visit to the greenhouse.
		_town.saloonVisited = 1;
		say(0);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene6240::handleAnimationFrameHook() {
	if (_roomAction == 304 && (_actionStep == 11 || _actionStep == 18)) {
		_audio.playFootstep(3, _random.getRandomBit());
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
