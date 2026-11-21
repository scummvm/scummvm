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
#include "hollywood/runaway/scene7010.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{0, 0, 7, {301, 302}, {0, 0}, {153, 397}}, // 1: Entrance.
	{249, 411, 1, {303, 304}, {0, 1}, {282, 366}}, // 2: Skeleton.
	{0, 0, 1, {305, 306}, {0, 0}, {281, 360}}, // 3: Drawings.
	{1180, 307, 1, {307, 308}, {0, 0}, {1193, 299}}, // 4: Rock.
	{1180, 308, 1, {309, 310}, {1, 0}, {1193, 299}}, // 5: Edge of the cliff.
	{189, 505, 7, {311, 312}, {1, 0}, {130, 468}}, // 6: Nail.
	{0, 0, 0, {313, 314}, {0, 0}, {0, 0}}, // 7: Rope.
	{507, 444, 3, {318, 319}, {1, 1}, {629, 485}}, // 8: Long piece of wood.
	{507, 444, 3, {320, 321}, {1, 1}, {629, 485}} // 9: Metal object.
};

const RunawayResponse kResponses[] = {
	{7010, 301, 1, 0}, // Look back into the mine.
	{7010, 302, 2, 0}, // Refuse to return without Gina.
	{7010, 305, 5, 0}, // Examine the cave drawings.
	{7010, 306, 6, 0}, // Leave the drawings alone.
	{7010, 308, 8, 0}, // Try moving the rock.
	{7010, 310, 10, 0} // Refuse to jump off the cliff.
};

const RunawayItemAction kItemActions[] = {
	{7010, 6, 4, 322}, {7010, 6, 6, 317, false},
	{7010, 15, 6, 315}, {7010, 16, 5, 316}
};

typedef RunawaySequence S;
const S::Command kArrival[] = {
	{S::kOpen, 1}, {S::kMusic, 18, 255, 1}, {S::kStill, 0},
	{S::kTalk, 0, 0}, {S::kTalk, 0, 1}, {S::kTalk, 0, 2},
	{S::kMusic, 19, 255}, {S::kEffect, 31, 255},
	{S::kSpeech, 0, 3, 3}, {S::kPlay, 1, 110}, {S::kWaitSpeech},
	{S::kSpeech, 0, 4, 4}, {S::kPlay, 111, 249}, {S::kWaitSpeech},
	{S::kSpeech, 0, 5, 5}, {S::kPlay, 250, 316}, {S::kWaitSpeech},
	{S::kEnd}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene7010 = {
	7010, 1400, 760, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {55, 45, 45}, {65, 55, 55}, {90, 80, 80}}, Common::Point(1180, 307),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene7010::Scene7010(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

const char *Scene7010::actionCaptionVariant(byte id, bool look, byte item) const {
	if (!look && !item && id == 2) {
		if (_hopi.femurTaken) {
			return "more_bones";
		}
		if (_hopi.timber > 2) {
			return "bone";
		}
	}
	return nullptr;
}

bool Scene7010::loadCustomResources() {
	return _bats.load(_archive, 4, 50) && _nail.load(_archive, 5, 2) &&
		_anchor.load(_archive, 8, 1) && _lowerRope.load(_archive, 10, 56) && loadForeground(0, 11);
}

bool Scene7010::applyCustomBackgroundPatches() {
	if (_hopi.timber && !applyPatch(_hopi.timber == 3 ? 13 : 12)) {
		return false;
	}
	return !_hopi.femurTaken || applyPatch(14);
}

void Scene7010::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		switch (id) {
		case 6:
			id = _hopi.nail == 0 ? 6 : 0;
			break;
		case 7:
			id = 0;
			break;
		case 8:
			id = _hopi.timber == 0 ? 8 : 0;
			break;
		case 9:
			id = _hopi.timber == 0 ? 8 : _hopi.timber == 3 ? 0 : 9;
			break;
		case 11:
			id = _hopi.nail == 2 ? 6 : 4;
			break;
		default:
			break;
		}
	}
}

void Scene7010::resumeAmbientAudio(bool entering) {
	_audio.playEffect(12, 204, 1, true);
}

void Scene7010::advanceCustomGameplayLoop(uint32 delta) {
	updateOutdoorMusic();
	if (_roomAction == 420 && !_actionFrames.empty()) {
		_actionFrames[0] = 80 + speechFrame();
	}
}

void Scene7010::runCustomEntrySequence() {
	if (!_hopi.cliffVisited) {
		_sequence.start(kArrival, _text, _now, 'G');
	}
}

void Scene7010::finishCustomFilm() {
	_audio.playMusic(11, false);
	resumeAmbientAudio(false);
	say(0, 6);
	deferAction(400);
}

void Scene7010::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	drawAmbientAnimations(viewport, &_bats, 1);
	if ((_roomAction == 417 && _actionStep >= 54) || _roomAction == 418 || _roomAction == 420) {
		_lowerRope.draw(viewport, 0, _camera.x, -_camera.y);
	}
	if (!_hopi.nail) {
		_nail.draw(viewport, 1, _camera.x, -_camera.y);
	} else if (_hopi.nail == 2) {
		_anchor.draw(viewport, 0, _camera.x, -_camera.y);
	}
}

void Scene7010::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	drawForeground(viewport, 0);
}

float Scene7010::actorHeight() const {
	return (_position.x * -0.192554563f + 332.792053f +
		_position.y * 1.4204545f - 267.613647f) * 0.5f;
}

bool Scene7010::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = CLIP<int>(target.x, 189, 1180);
	target.y = MIN<int>(target.y, 599);
	while (target.y < 599 && !_maps[_mask[target.y * _background.w + target.x]]) {
		++target.y;
	}
	while (target.y > 0 && !_maps[_mask[target.y * _background.w + target.x]]) {
		--target.y;
	}
	return true;
}

void Scene7010::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

int8 Scene7010::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (id == 2 && use && !item) {
		return _hopi.timber == 3;
	}
	if (item == 6 && id == 4) {
		return !_hopi.rockTried;
	}
	if (item == 6 && id == 6) {
		return _hopi.nail == 2;
	}
	return approach;
}

Common::Point Scene7010::customizeSceneActionPosition(byte id, byte item, Common::Point position) const {
	return id == 6 && _hopi.nail == 2 ? Common::Point(1180, 308) : position;
}

byte Scene7010::customizeSceneActionFacing(byte id, byte facing) const {
	return id == 6 && _hopi.nail == 2 ? 1 : facing;
}

Common::Point Scene7010::sceneActionLookPosition(byte id) const {
	if (id == 6 && _hopi.nail == 2) {
		return Common::Point(1193, 299);
	}
	return RunawayRoomScene::sceneActionLookPosition(id);
}

Common::U32String Scene7010::hotspotName(byte id) const {
	if (id == 9 && _hopi.timber == 2) {
		return _strings.hotspotName(_room, 15);
	}
	return RunawayRoomScene::hotspotName(id);
}

void Scene7010::handleAnimationFrameHook() {
	if (_roomAction == 415 && _actionStep == 25) {
		_audio.playEffect(15);
	} else if (_roomAction == 416 && _actionStep == 24) {
		_audio.playEffect(16);
	} else if (_roomAction == 417 && _actionStep == 16) {
		_audio.playEffect(17, 217);
	} else if (_roomAction == 418 && _actionStep == 25) {
		_audio.playEffect(18, 204);
	} else if (_roomAction == 423) {
		_camera.y = MIN<uint>(160, _actionStep * 4);
	}
}

RunawayRoomScene::ActionResult Scene7010::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 400: // Finish looking down into the village.
		_facing = 4;
		say(0, 7);
		_hopi.cliffVisited = 1;
		break;
	case 303: // Inspect the skeleton and notice its missing limb.
		if (!_hopi.skeletonSeen) {
			say(3);
			deferAction(403);
		} else {
			say(3, _hopi.femurTaken ? 3 : 2);
		}
		break;
	case 403: // Approach the skeleton for a closer look.
		walkTo(Common::Point(249, 411), 1);
		deferAction(404);
		break;
	case 404: // Finish the first examination of the skeleton.
		say(3, 1);
		_hopi.skeletonSeen = 1;
		break;
	case 304: // Take a bone once Brian has found the mining tool.
		if (!_inventory.contains(15)) {
			say(4);
		} else if (_hopi.femurTaken) {
			say(4, 3);
		} else {
			say(4, 1);
			deferAction(405);
		}
		break;
	case 405: // Collect the femur.
		_hopi.femurTaken = 1;
		_inventory.add(14);
		_audio.playEffect(13, 230);
		say(4, 2);
		return kBackgroundChanged;
	case 307: // Check whether the rock could support the rope.
		say(7, _hopi.rockTried);
		break;
	case 309: // Look over the edge.
		_facing = 4;
		say(9);
		break;
	case 311: // Inspect the nail in the wood or the cliff.
		say(11, _hopi.nail != 0);
		break;
	case 312: // Try pulling the nail out by hand.
		if (_hopi.nail == 2) {
			execute(15);
		} else {
			say(12);
		}
		break;
	case 315: // Use the repaired mining tool to extract the nail.
		if (!_inventory.hopi().toolHandle) {
			say(13);
		} else if (_hopi.nail == 2) {
			execute(15);
		} else {
			say(13, 1);
			deferAction(415);
		}
		break;
	case 415: // Pull the nail from the wooden support.
		_hopi.nail = 1;
		beginRoomAnimation(415, 6, 45);
		return kBackgroundChanged;
	case 1415: // Put the extracted nail in the inventory.
		_inventory.add(16);
		say(13, 2);
		break;
	case 316: // Prepare to drive the nail into the cliff.
		say(14);
		deferAction(416);
		break;
	case 416: // Hammer in the climbing anchor.
		_inventory.remove(16);
		beginRoomAnimation(416, 7, 54);
		break;
	case 1416: // Make the anchored nail available for the rope.
		_hopi.nail = 2;
		say(14, 1);
		return kBackgroundChanged;
	case 317: // Attach the rope to the anchor.
		if (!_hopi.nail) {
			say(1, 2, true);
		} else {
			say(15);
			deferAction(417);
		}
		break;
	case 417: // Tie the rope and climb over the edge.
		_inventory.remove(6);
		_audio.stopMusic();
		beginRoomAnimationRange(417, 9, 200, 0, 80);
		break;
	case 1417: // Brian speaks while hanging from the cliff.
		say(15, 1);
		holdRoomAnimation(420, 9, 200, 80);
		break;
	case 1420: // Continue down the rope after Brian's remark.
		beginRoomAnimationRange(418, 9, 200, 93, 199);
		break;
	case 1418: // Follow Brian down the lower part of the cliff.
		beginRoomAnimation(423, 10, 56);
		break;
	case 1423: // Arrive at the Hopi village.
		leaveRoom(7020);
		break;
	case 318: // Examine the wooden beam.
		_facing = 4;
		say(16);
		break;
	case 319: // Lift the beam to expose the metal object.
		say(17);
		deferAction(419);
		break;
	case 419: // Collect the beam.
		_hopi.timber = 1;
		_inventory.add(10);
		_audio.playEffect(11);
		say(17, 1);
		return kBackgroundChanged;
	case 320: // Identify the exposed object as a mining tool.
		_facing = 4;
		say(18, _hopi.timber == 1 ? 0 : 1);
		_hopi.timber = 2;
		return kBackgroundChanged;
	case 321: // Examine the metal object before picking it up.
		if (_hopi.timber == 1) {
			execute(320);
		}
		say(19);
		deferAction(421);
		break;
	case 421: // Collect the mining tool.
		_hopi.timber = 3;
		_inventory.add(15);
		say(19, 1);
		return kBackgroundChanged;
	case 322: // Try tying the rope around the rock.
		say(20, _hopi.rockTried);
		_hopi.rockTried = 1;
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
