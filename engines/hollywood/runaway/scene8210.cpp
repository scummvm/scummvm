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
#include "hollywood/runaway/scene8210.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{430, 540, 4, {301, 302}, {0, 3}, {430, 599}, 6}, // 1: Door.
	{430, 540, 4, {301, 302}, {0, 3}, {430, 599}, 7}, // 2: Door.
	{294, 513, 0, {303, 304}, {1, 1}, {288, 471}}, // 3: Doctor's bag.
	{449, 504, 0, {305, 306}, {0, 0}, {449, 467}}, // 4: Dead body.
	{757, 508, 1, {307, 308}, {1, 0}, {794, 450}}, // 5: Bucket.
	{660, 507, 0, {309, 310}, {0, 0}, {660, 465}}, // 6: Plate.
	{449, 504, 0, {311, 312}, {0, 0}, {449, 467}}, // 7: Pitcher.
	{341, 502, 0, {313, 314}, {1, 0}, {341, 471}} // 8: Bottle.
};

const RunawayResponse kResponses[] = {
	{8210, 301, 1, 0}, // Look toward the exit.
	{8210, 303, 2, 0}, // Examine the doctor's bag.
	{8210, 305, 4, 0}, // Examine the dead body.
	{8210, 306, 5, 0}, // Refuse to search the dead man's pockets.
	{8210, 309, 8, 0}, // Examine the plate.
	{8210, 311, 9, 0}  // Examine the pitcher.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8210 = {
	8210, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {65, 60, 65}, {55, 50, 55}}, Common::Point(430, 540),
	kResponses, ARRAYSIZE(kResponses), nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene8210::Scene8210(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

void Scene8210::resumeAmbientAudio(bool entering) {
	resumeFinaleJailAudio(false);
}

void Scene8210::advanceCustomGameplayLoop(uint32 delta) {
	updateFinaleJailAudio();
}

void Scene8210::runCustomEntrySequence() {
	_facing = 0;
	_town.cellRefusals = 0;
}

bool Scene8210::updateCustomSequence() {
	if (!_rummaging) {
		return false;
	}
	if (_audio.effectPlaying() && (int32)(_now - _rummageUntil) < 0) {
		return true;
	}
	_rummaging = false;
	return false;
}

float Scene8210::actorHeight() const {
	return _position.y * 0.413793117f + 229.482758f;
}

void Scene8210::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
}

void Scene8210::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

bool Scene8210::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = CLIP<int>(target.x, 67, 869);
	return false;
}

int8 Scene8210::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (!item && !use && ((id == 5 && _town.bucketSeen) || (id == 8 && _town.bottleSeen))) {
		return false;
	}
	return approach;
}

RunawayRoomScene::ActionResult Scene8210::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Return to the sheriff's office.
		leaveRoom(8201);
		break;
	case 304: { // Open the doctor's bag for another search.
		static const byte frames[] = {0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
		say(3, 1);
		beginRoomAnimation(304, 4, 28, frames, ARRAYSIZE(frames), true, 60);
		break;
	}
	case 1304: // Rummage through the open bag.
		_audio.playEffect(25);
		_rummaging = true;
		_rummageUntil = _now + 2000;
		holdRoomAnimation(410, 4, 28, 16);
		break;
	case 1410: // Continue searching through the medical tools.
		_audio.playEffect(26, 204);
		_rummaging = true;
		_rummageUntil = _now + 2000;
		holdRoomAnimation(411, 4, 28, 16);
		break;
	case 1411: // Take the scalpel only after discovering the hard pods.
		if (_finale.scalpelNeeded && !_inventory.contains(19)) {
			_inventory.add(19);
			say(3, 5);
		} else {
			say(3, 2);
			holdRoomAnimation(412, 4, 28, 16);
		}
		break;
	case 1412: { // Close the bag after an unsuccessful search.
		static const byte frames[] = {16, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};
		beginRoomAnimation(413, 4, 28, frames, ARRAYSIZE(frames), true, 60);
		break;
	}
	case 1413: // Finish closing the doctor's bag.
		break;
	case 307: // Identify the bucket's unpleasant contents.
		_facing = 4;
		if (!_town.bucketSeen) {
			say(6);
			say(6, 1);
			_town.bucketSeen = 1;
		} else {
			say(6, 2);
		}
		break;
	case 308: // Examine the bucket before refusing to take it.
		if (!_town.bucketSeen) {
			walkTo(Common::Point(757, 508), 4, false, true);
			deferAction(408);
		} else {
			dispatchCustomSceneAction(310);
		}
		break;
	case 408: // Finish examining the bucket and give the refusal.
		dispatchCustomSceneAction(307);
		deferAction(310);
		break;
	case 310: // Refuse to take the plate.
	case 312: // Refuse to take the pitcher.
	case 314: // Refuse to take the bottle.
		if (_town.cellRefusals < 2) {
			say(7, _town.cellRefusals);
			++_town.cellRefusals;
		} else {
			say(7, 2 + _random.getRandomBit());
		}
		break;
	case 313: // Identify the empty whisky bottle.
		say(10, _town.bottleSeen);
		_town.bottleSeen = 1;
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
