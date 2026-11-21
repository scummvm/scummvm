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
#include "hollywood/runaway/scene6210.h"

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
	{6210, 301, 1, 0}, // Look toward the exit.
	{6210, 303, 2, 0}, // Examine the doctor's bag.
	{6210, 305, 4, 0}, // Examine the dead body.
	{6210, 306, 5, 0}, // Refuse to search the dead man's pockets.
	{6210, 309, 8, 0}, // Examine the plate.
	{6210, 311, 9, 0}  // Examine the pitcher.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6210 = {
	6210, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {65, 60, 65}, {55, 50, 55}},
	Common::Point(430, 540),
	kResponses, ARRAYSIZE(kResponses), nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene6210::Scene6210(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

const char *Scene6210::actionCaptionVariant(byte id, bool look, byte item) const {
	return !look && !item && id == 3 && _town.bagSearched ? "searched" : nullptr;
}

void Scene6210::resumeAmbientAudio(bool entering) {
	if (!_audio.musicPlaying()) {
		_audio.playMusic(18);
		_audio.setMusicVolume(191);
	} else if (entering) {
		_audio.setMusicVolume(191);
	}
}

void Scene6210::runCustomEntrySequence() {
	_facing = 0;
	_town.cellOpen = _town.bodySeen = 1;
	_town.cellRefusals = 0;
}

float Scene6210::actorHeight() const {
	return _position.y * 0.413793117f + 229.482758f;
}

void Scene6210::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
}

int8 Scene6210::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (!item && !use && ((id == 5 && _town.bucketSeen) || (id == 8 && _town.bottleSeen))) {
		return false;
	}
	return approach;
}

RunawayRoomScene::ActionResult Scene6210::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Return to the sheriff's office.
		leaveTownRoom(6201);
		break;
	case 304: // Search the doctor's bag.
		say(3, _town.bagSearched);
		_town.bagSearched = 1;
		beginRoomAnimationRange(304, 4, 28, 0, 16);
		break;
	case 1304: { // Rummage inside the open bag.
		byte frames[16];
		memset(frames, 16, sizeof(frames));
		beginRoomAnimation(410, 4, 28, frames, ARRAYSIZE(frames));
		_audio.playEffect(60);
		break;
	}
	case 1410: // Only take medical tools once their purpose is known.
		if (_town.safeSeen && !_town.stethoscopeTaken) {
			_town.stethoscopeTaken = 1;
			_inventory.add(7);
			say(3, 3);
			if (_town.oscarTobacco && !_town.linimentTaken) {
				deferTownAction(411);
			}
		} else if (_town.oscarTobacco && !_town.linimentTaken) {
			dispatchCustomSceneAction(411);
		} else {
			say(3, 2);
			holdRoomAnimation(412, 4, 28, 16);
		}
		break;
	case 411: // Take the mint liniment for the chewing tobacco.
		_town.linimentTaken = 1;
		_inventory.add(8);
		say(3, 4);
		break;
	case 1412: // Close the bag after an unsuccessful search.
		beginRoomAnimationRange(413, 4, 28, 16, 27);
		break;
	case 1413: // Finish closing the doctor's bag.
		break;
	case 307: // Discover what the bucket was used for.
		_facing = 4;
		if (!_town.bucketSeen) {
			say(6);
			say(6, 1);
			_town.bucketSeen = 1;
		} else {
			say(6, 2);
		}
		break;
	case 308: // Refuse to take the bucket.
		if (!_town.bucketSeen) {
			walkTo(Common::Point(757, 508), 4, true);
			deferTownAction(408);
		} else {
			dispatchCustomSceneAction(310);
		}
		break;
	case 408: // Inspect the bucket before refusing to take it.
		dispatchCustomSceneAction(307);
		deferTownAction(310);
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

void Scene6210::handleAnimationFrameHook() {
	if (_roomAction == 410 && _actionStep == 8) {
		_audio.playEffect(61, 204);
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
