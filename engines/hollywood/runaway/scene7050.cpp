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
#include "hollywood/runaway/scene7050.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{428, 518, 1, {301, 302}, {1, 1}, {531, 472}}, // 1: Statue.
	{717, 541, 0, {303, 304}, {1, 1}, {717, 517}}, // 2: Object resting on the stone.
	{60, 599, 5, {305, 306}, {0, 0}, {0, 650}, 7}, // 3: Inside of the cave.
	{356, 501, 7, {307, 308}, {1, 0}, {321, 470}}, // 4: Cross.
	{664, 536, 1, {309, 310}, {1, 0}, {715, 477}}, // 5: Cross.
	{467, 445, 1, {311, 312}, {0, 3}, {453, 424}, 5} // 6: Village.
};

const RunawayResponse kResponses[] = {
	{7050, 302, 2, 0}, // Try opening the solid statue.
	{7050, 305, 5, 0}, // Look towards the mine shaft.
	{7050, 306, 6, 0}, // Refuse to leave without Gina.
	{7050, 308, 8, 0}, // Leave the left cross alone.
	{7050, 310, 8, 0}, // Leave the right cross alone.
	{7050, 311, 9, 0}  // Examine the sheltered village.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene7050 = {
	7050, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {70, 70, 70}, {50, 50, 50}, {30, 30, 30}, {60, 60, 60}}, Common::Point(467, 445),
	kResponses, ARRAYSIZE(kResponses), nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene7050::Scene7050(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene7050::loadCustomResources() {
	Common::Array<byte> patch;
	_statue.create(kRunawayScreenWidth, kRunawayViewportHeight, _screen.format);
	// Bank 147 is the pickup gesture used at the statue.
	return _archive.readVariableChunk(4, patch) && applyRunawayPatch(patch, _statue) && _actor.loadAction(147, 8);
}

bool Scene7050::applyCustomBackgroundPatches() {
	return _hopi.tomahawk != kTaken || applyPatch(5);
}

void Scene7050::applyCustomHotspotState() {
	if (_hopi.tomahawk == kTaken) {
		for (uint i = 0; i < 256; ++i) {
			if (_maps[256 + i] == 2) {
				_maps[256 + i] = 0;
			}
		}
	}
}

void Scene7050::runCustomEntrySequence() {
	if (!_hopi.statueVisited) {
		walkTo(Common::Point(415, 509), 1);
		deferAction(400);
	} else {
		walkTo(Common::Point(428, 518), 5);
	}
}

void Scene7050::resumeAmbientAudio(bool entering) {
	_audio.stopEffect(1);
}

void Scene7050::advanceCustomGameplayLoop(uint32 delta) {
	updateOutdoorMusic();
}

bool Scene7050::drawCustomComposite(Graphics::ManagedSurface &viewport) {
	if (!_showStatue) {
		return false;
	}
	viewport.blitFrom(_statue);
	return true;
}

void Scene7050::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 481;
	params.depthY[2] = 553;
}

float Scene7050::actorHeight() const {
	return _position.y * 0.364963502f + 6.386861324f;
}

void Scene7050::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

bool Scene7050::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = CLIP<int>(target.x, 60, 734);
	return false;
}

int8 Scene7050::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (!use && !item) {
		switch (id) {
		case 2:
			return _hopi.tomahawk == kUnexamined;
		case 4:
			return !_hopi.leftCrossSeen;
		case 5:
			return !_hopi.rightCrossSeen;
		default:
			break;
		}
	}
	return approach;
}

Common::U32String Scene7050::hotspotName(byte id) const {
	if (id == 2 && _hopi.tomahawk != kUnexamined) {
		return _inventory.name(11);
	}
	return RunawayRoomScene::hotspotName(id);
}

bool Scene7050::updateCustomSequence() {
	if (!_pickingUp) {
		return false;
	}
	if (_now - _pickupStarted < 8 * 60) {
		return true;
	}
	_pickingUp = false;
	return false;
}

void Scene7050::updateActor(uint elapsed, int scrollDirection, bool inventoryOpen) {
	if (_pickingUp) {
		_actor.setActionFrame(MIN<uint>(7, (_now - _pickupStarted) / 60));
		return;
	}
	RunawayRoomScene::updateActor(elapsed, scrollDirection, inventoryOpen);
}

RunawayRoomScene::ActionResult Scene7050::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 400: // First arrival beneath the statue.
		say(0);
		deferAction(401);
		break;
	case 401: // Show the intimidating face on the first visit.
		_hopi.statueVisited = 1;
		_showStatue = true;
		say(0, 1);
		deferAction(402);
		break;
	case 301: // Examine the statue's face again.
		_showStatue = true;
		say(1);
		deferAction(402);
		break;
	case 402: // Return from the close-up after Brian finishes speaking.
		_showStatue = false;
		break;
	case 303: // Identify the object on the stone as a tomahawk.
		say(3, _hopi.tomahawk == kUnexamined ? 0 : 1);
		_hopi.tomahawk = kIdentified;
		break;
	case 304: // Inspect the tomahawk before deciding to take it.
		if (_hopi.tomahawk == kUnexamined) {
			say(3);
			_hopi.tomahawk = kIdentified;
		}
		say(4);
		deferAction(405);
		break;
	case 405: // Reach down for the tomahawk.
		_pickupStarted = _now;
		_pickingUp = true;
		deferAction(406);
		break;
	case 406: // Remove the tomahawk from the stone and reveal its pipe.
		_hopi.tomahawk = kTaken;
		_inventory.add(11);
		say(4, 1);
		return kBackgroundChanged;
	case 307: { // Examine the left cross and compare it with the right one.
		const byte variation = _hopi.rightCrossSeen ? (_hopi.leftCrossSeen ? 2 : 1) : 0;
		_hopi.leftCrossSeen = 1;
		say(7, variation);
		break;
	}
	case 309: { // Examine the right cross and compare it with the left one.
		const byte variation = _hopi.leftCrossSeen ? (_hopi.rightCrossSeen ? 2 : 1) : 0;
		_hopi.rightCrossSeen = 1;
		say(7, variation);
		break;
	}
	case 312: // Return to Gina in the village.
		leaveRoom(7021);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
