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

#include "common/file.h"
#include "common/util.h"
#include "hollywood/runaway/scene7040.h"

namespace Hollywood {
namespace {

enum CryptDoorState { kBlocked, kCleared, kOpen, kSealed };

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{446, 486, 1, {301, 302}, {0, 0}, {510, 451}}, // 1: Monolith.
	{446, 486, 1, {303, 304}, {0, 1}, {510, 451}}, // 2: Monolith's mouth.
	{0, 599, 5, {305, 306}, {0, 3}, {-100, 650}, 7}, // 3: Path.
	{705, 340, 1, {307, 308}, {0, 0}, {524, 438}, 5}, // 4: Cave in the rock.
	{109, 491, 0, {309, 310}, {0, 1}, {109, 450}} // 5: Bush.
};

const RunawayResponse kResponses[] = {
	{7040, 305, 5, 0}, // Look along the path back to the village.
	{7040, 307, 6, 0}  // Look into the open crypt.
};

const RunawayItemAction kItemActions[] = {
	{7040, 4, 2, 312}, {7040, 8, 2, 311},
	{7040, 0, 2, 313, false}
};

const byte kBlockedKeyFrames[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 23, 23, 23, 23, 23, 23, 24,
	25, 26, 26, 26, 26, 26, 26, 26, 26, 27, 28, 29, 30, 31, 32, 33,
	34, 35, 36, 37, 38, 39, 40, 41, 42, 43
};

const byte kClearKeyholeFrames[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 25, 24, 23, 22,
	21, 20, 19, 20, 21, 22, 23, 24, 25, 26, 25, 24, 23, 22, 21, 20,
	19, 18, 17, 16, 15, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3,
	2, 1, 0
};

const byte kInsertKeyFrames[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 27, 26,
	25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

typedef RunawaySequence S;
const S::Command kCryptVisit[] = {
	{S::kOpen, 3}, {S::kEffect, 32, 255}, {S::kPlay, 0, 511},
	// Wupuchim welcomes Brian into the sanctuary.
	{S::kTalk, 20, 0, 1}, {S::kTalk, 20, 1, 3, 1},
	{S::kTalk, 20, 2, 3}, {S::kTalk, 20, 3, 3, 1}, {S::kTalk, 20, 4, 3},
	{S::kEffect, 33, 255}, {S::kPlay, 524, 608},
	{S::kTalk, 20, 5, 5}, {S::kTalk, 20, 6, 5, 1},
	// Reveal the bottle and explain why Brian must take it away.
	{S::kPlay, 627, 686}, {S::kSpeech, 20, 7, 8}, {S::kPlay, 687, 747}, {S::kWaitSpeech},
	{S::kTalk, 20, 9, 3}, {S::kTalk, 20, 10, 5},
	{S::kTalk, 20, 11, 3}, {S::kTalk, 20, 12, 5},
	{S::kTalk, 20, 13, 5, 1}, {S::kTalk, 20, 14, 5},
	{S::kTalk, 20, 15, 3}, {S::kTalk, 20, 16, 3, 1},
	// Leave the sacred key with Wupuchim and return to Gina.
	{S::kTalk, 20, 17, 3}, {S::kTalk, 20, 18, 3, 1},
	{S::kTalk, 20, 19, 3}, {S::kTalk, 20, 20, 7},
	{S::kTalk, 20, 21, 3}, {S::kTalk, 20, 22, 7},
	{S::kEffect, 34, 255}, {S::kPlay, 760, 933}, {S::kEnd}
};

const S::Command kExamineBottle[] = {
	{S::kImage, 147}, {S::kSpeech, 0, 2}, {S::kWaitSpeech}, {S::kEnd}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene7040 = {
	7040, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {90, 90, 90}, {70, 70, 70}, {55, 55, 55}}, Common::Point(0, 599),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene7040::Scene7040(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene7040::loadCustomResources() {
	return _monolith.load(_archive, 4, 49);
}

void Scene7040::prepareCustomGameplayLoop() {
	_doorFrame = _hopi.cryptDoor == kOpen ? 48 : 0;
}

void Scene7040::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[i] == 3) {
			_maps[i] = 0;
		}
		byte &id = _maps[256 + i];
		switch (id) {
		case 1:
		case 2:
			if (_hopi.cryptDoor == kOpen) {
				id = 0;
			}
			break;
		case 4:
			if (_hopi.cryptDoor != kOpen) {
				id = 1;
			}
			break;
		case 6:
			id = _hopi.cryptDoor == kOpen ? 1 : 0;
			break;
		default:
			break;
		}
	}
}

void Scene7040::runCustomEntrySequence() {
	walkTo(Common::Point(229, 530), 1, true);
	if (!_hopi.cryptEntranceVisited) {
		deferAction(400);
	}
}

void Scene7040::resumeAmbientAudio(bool entering) {
	// The crypt entrance has no wind loop.
	_audio.stopEffect(1);
}

void Scene7040::advanceCustomGameplayLoop(uint32 delta) {
	updateOutdoorMusic();
	if (!_doorDirection) {
		return;
	}
	while (_now - _doorTick >= 80 && _doorDirection) {
		_doorTick += 80;
		_doorFrame += _doorDirection;
		if (_doorFrame == 0 || _doorFrame == 48) {
			_hopi.cryptDoor = _doorDirection > 0 ? kOpen : kSealed;
			_doorDirection = 0;
			updateMaps();
		}
	}
}

void Scene7040::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	_monolith.draw(viewport, _doorFrame, _camera.x, -_camera.y);
}

void Scene7040::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
	params.depthY[2] = 440;
}

float Scene7040::actorHeight() const {
	return _position.y * 0.967741907f - 124.677422f;
}

void Scene7040::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

bool Scene7040::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = MIN<int>(target.x, 620);
	return false;
}

int8 Scene7040::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (id == 5 && use && !item) {
		return _hopi.keyholeBlockedKnown && !_inventory.contains(8);
	}
	return approach;
}

Common::Point Scene7040::sceneActionLookPosition(byte id) const {
	if (id == 1 && _hopi.cryptDoor == kOpen) {
		return Common::Point(323, 427);
	}
	return RunawayRoomScene::sceneActionLookPosition(id);
}

void Scene7040::handleAnimationFrameHook() {
	if (_roomAction == 405 && _actionStep == 4) {
		_audio.playEffect(23);
	} else if (_roomAction == 406 && _actionStep == 19) {
		_audio.playEffect(24, 204);
	} else if (_roomAction == 407 && _actionStep == 22) {
		_audio.playEffect(22, 204);
	} else if (_roomAction == 410) {
		if (_actionStep == 22) {
			_audio.playEffect(25, 204);
		} else if (_actionStep == 42) {
			_audio.playEffect(28);
			_doorDirection = 1;
			_doorTick = _now;
		}
	}
}

void Scene7040::startCryptFilm() {
	_filmCommands.clear();
	const bool gog = Common::File::exists("DATAVC00.000");
	for (uint i = 0; i < ARRAYSIZE(kCryptVisit); ++i) {
		S::Command command = kCryptVisit[i];
		if (gog && command.operation == S::kTalk) {
			// The GOG movie replaces the CD's portrait ranges with single shots.
			static const uint16 stills[] = {
				512, 513, 513, 513, 513, 599, 599, 0, 0, 721, 722, 723,
				724, 724, 724, 725, 725, 725, 725, 725, 726, 727, 728
			};
			_filmCommands.push_back(S::Command(S::kStill, stills[command.b]));
		} else if (gog && command.operation == S::kPlay) {
			const uint offset = command.a >= 760 ? 31 : command.a >= 627 ? 27 : command.a >= 524 ? 10 : 0;
			command.a -= offset;
			command.b -= offset;
		}
		_filmCommands.push_back(command);
	}
	_sequence.start(_filmCommands.data(), _text, _now, 'G');
}

void Scene7040::finishCustomFilm() {
	if (_bottleFilm) {
		_bottleFilm = false;
		say(0, 3);
		return;
	}
	_inventory.remove(4);
	_inventory.add(9);
	_position = Common::Point(446, 486);
	_walkPath.reset(_position);
	_facing = 5;
	_doorFrame = 48;
	_doorDirection = -1;
	_doorTick = _now;
	_audio.playEffect(29);
	_audio.playMusic(11, false);
	say(0, 1);
	deferAction(420);
}

RunawayRoomScene::ActionResult Scene7040::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 400: // First arrival at the crypt entrance.
		say(0);
		_hopi.cryptEntranceVisited = 1;
		break;
	case 301: // Examine the monolith or its open mechanism.
		say(1, _hopi.cryptDoor == kOpen ? 1 : 0);
		break;
	case 302: // Try moving the monolith by hand.
		say(2, _hopi.cryptDoor == kOpen ? 1 : 0);
		break;
	case 303: // Examine the keyhole before or after clearing it.
		if (_hopi.cryptDoor == kSealed) {
			say(3, 3);
		} else if (!_hopi.keyholeBlockedKnown) {
			say(3);
		} else {
			say(3, _hopi.cryptDoor == kBlocked ? 1 : 2);
		}
		break;
	case 304: // Try reaching into the monolith's mouth.
		if (!_hopi.keyholeBlockedKnown) {
			say(4);
		} else {
			say(4, _hopi.cryptDoor == kBlocked ? 1 : 2);
		}
		break;
	case 306: // Return to the canyon overlook.
		leaveRoom(7031);
		break;
	case 308: // Enter the opened crypt.
		_hopi.cryptVisited = 1;
		say(7);
		deferAction(408);
		break;
	case 309: // Examine the bush and its seed pods.
		say(8);
		_hopi.bushSeen = 1;
		break;
	case 310: // Take a branch once its use in the keyhole is known.
		if (_inventory.contains(8)) {
			say(9, 3);
		} else if (!_hopi.keyholeBlockedKnown) {
			say(9);
		} else {
			if (!_hopi.bushSeen) {
				say(8);
				_hopi.bushSeen = 1;
			}
			say(9, 1);
			deferAction(405);
		}
		break;
	case 405: // Pull a branch from the bush.
		beginRoomAnimation(405, 5, 19);
		break;
	case 1405: // Keep the branch and notice its pods.
		_inventory.add(8);
		say(9, 2);
		break;
	case 311: // Clear the sand with the branch.
		if (_hopi.cryptDoor != kBlocked) {
			say(10, 2);
		} else {
			say(10);
			deferAction(406);
		}
		break;
	case 406: // Scrape the sand out of the keyhole.
		beginRoomAnimation(406, 8, 27, kClearKeyholeFrames, ARRAYSIZE(kClearKeyholeFrames));
		break;
	case 1406: // The keyhole is now clear.
		_hopi.cryptDoor = kCleared;
		updateMaps();
		say(10, 1);
		break;
	case 312: // Insert the sacred key into the monolith's mouth.
		if (_hopi.cryptDoor != kBlocked) {
			say(11, 2);
			deferAction(410);
		} else if (!_hopi.keyholeBlockedKnown) {
			say(11);
			deferAction(407);
		} else {
			say(11, 5);
		}
		break;
	case 407: // Discover that sand prevents the key from fitting.
		beginRoomAnimation(407, 6, 44, kBlockedKeyFrames, ARRAYSIZE(kBlockedKeyFrames));
		break;
	case 1407: // Explain the obstruction and enable taking a branch.
		_facing = 4;
		_hopi.keyholeBlockedKnown = 1;
		say(11, 1);
		break;
	case 410: // Turn the key and start opening the monolith.
		_audio.playMusic(20, false);
		beginRoomAnimation(410, 7, 29, kInsertKeyFrames, ARRAYSIZE(kInsertKeyFrames));
		break;
	case 1410: // Step back while the monolith moves aside.
		say(11, 3);
		deferAction(411);
		break;
	case 411: // Wait for the entrance to open completely.
		if (_doorDirection) {
			deferAction(411);
		} else {
			say(11, 4);
		}
		break;
	case 313: // Reject other items as tools for clearing the keyhole.
		if (!_hopi.keyholeBlockedKnown) {
			say(25, 0, true);
		} else {
			say(12, _hopi.cryptDoor == kBlocked ? 0 : 1);
		}
		break;
	case 408: // Walk down the passage into the sanctuary.
		_audio.playMusic(15);
		walkTo(Common::Point(705, 340), 1, true);
		deferAction(409);
		break;
	case 409: // Meet Wupuchim inside the crypt.
		startCryptFilm();
		break;
	case 420: // Examine the bottle after the entrance seals behind Brian.
		if (_doorDirection) {
			deferAction(420);
		} else {
			_bottleFilm = true;
			_sequence.start(kExamineBottle, _text, _now, 'G');
		}
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
