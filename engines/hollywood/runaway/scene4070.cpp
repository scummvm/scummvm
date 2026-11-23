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

#include "common/scummsys.h"
#include "common/util.h"
#include "hollywood/runaway/scene4070.h"

namespace Hollywood {
namespace {

const byte kWillyTurnFrames[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	17, 18, 19, 20, 21, 22, 23, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene4070 = {
	kMuseumWilly, 1024, 600, nullptr, 0, {{100, 100, 100}, {70, 70, 70}}, Common::Point(548, 580),
	nullptr, 0, nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene4070::Scene4070(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {
	_speechColors[1] = 19; // Willy.
}

bool Scene4070::loadCustomResources() {
	return _willyAnimation.load(_archive, 4, 188);
}

void Scene4070::runCustomEntrySequence() {
	_facing = 7;
	_audio.playEffect(57, 102, 1, true);
	beginRoomAnimation(400, 4, 188, kWillyTurnFrames, ARRAYSIZE(kWillyTurnFrames), false, 60);
}

void Scene4070::updateActor(uint elapsed, int scrollDirection, bool inventoryOpen) {
	_actor.setFrame(_facing, 0, speaking() && !speaker() ? speechFrame() : -1);
}

void Scene4070::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (!drawingRoomAnimation() || (_actionHidesActor && _secondaryFrames.empty())) {
		_willyAnimation.draw(viewport, _willyPose + (speaker() ? speechFrame() : 0), 0, 0);
	}
}

void Scene4070::handleAnimationFrameHook() {
	static const AnimationFootstep steps[] = {
		{10, 512}, {14, 512}, {38, 512}, {44, 412}, {51, 312},
		{56, 212}, {62, 112}, {68, 12}
	};
	const byte frame = _actionFrames[_actionStep];
	if (_roomAction == 400 && frame == 17) {
		_audio.setEffectVolume(1, 204);
	} else if (_roomAction == 409 && frame == 14) {
		_audio.setEffectVolume(1, 102);
	} else if (_roomAction == 458) {
		playAnimationFootsteps(frame, 1, steps, ARRAYSIZE(steps));
		switch (frame) {
		case 36: // Brian turns to watch Willy leave.
			_facing = 6;
			break;
		case 44:
			_audio.setEffectVolume(1, 179);
			break;
		case 51:
			_audio.setEffectVolume(1, 128);
			break;
		case 56:
			_audio.setEffectVolume(1, 77);
			break;
		case 62:
			_audio.setEffectVolume(1, 26);
			break;
		}
	}
}

void Scene4070::finishCustomAnimation(uint16 action) {
	switch (action) {
	case 400: // Address Willy after he turns toward Brian.
		if (_entryScene == 4071 || _entryScene == 4072) {
			say(32);
			deferMuseumAction(417);
		} else {
			startWillyConversation();
		}
		break;
	case 417: // Hold out the coffee package or bowl to Willy.
		if (_entryScene == 4071) {
			beginRoomAnimationRange(437, 5, 53, 19, 33, true, 60);
		} else {
			static const byte frames[] = {19, 20, 21, 22, 23, 24, 25, 26, 27, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52};
			beginRoomAnimation(437, 5, 53, frames, ARRAYSIZE(frames), true, 60);
		}
		break;
	case 437: // Keep Brian's hand extended while Willy replies.
		if (_entryScene == 4071) {
			holdRoomAnimation(427, 5, 53, 33);
			say(33, 3, false, 1);
		} else {
			holdRoomAnimation(467, 5, 53, 52);
			say(33, _inventory.museum().coffee != 0, false, 1);
		}
		break;
	case 427: { // Willy takes the coffee package.
		byte willy[31], brian[31];
		for (uint i = 0; i < ARRAYSIZE(willy); ++i) {
			willy[i] = !i || i == 30 ? 54 : 158 + i;
			brian[i] = i < 10 ? 33 : i < 19 ? 24 + i : 18;
		}
		beginRoomAnimation(407, 5, 53, brian, ARRAYSIZE(brian), true, 60);
		setRoomAnimationOverlay(4, 188, willy, ARRAYSIZE(willy));
		break;
	}
	case 467: { // Lower the bowl after Willy refuses the coffee.
		static const byte frames[] = {51, 51, 50, 49, 48, 47, 46, 45, 44, 43, 27, 26, 25, 24, 23, 22, 21, 20, 19};
		beginRoomAnimation(477, 5, 53, frames, ARRAYSIZE(frames), true, 60);
		break;
	}
	case 477: // Explain which coffee Willy needs.
		if (!_inventory.museum().coffee) {
			_museum.needsGroundCoffee = 1;
		} else {
			say(32, 1);
			say(33, 2, false, 1);
			_museum.needsPackage = 1;
		}
		deferMuseumAction(489);
		break;
	case 407: // Willy agrees to refill the coffee machine.
		_inventory.remove(19);
		say(33, 4, false, 1);
		if (_game == kRunawayMuseumDemo) {
			say(36, 1);
			say(33, 5, false, 1);
		} else {
			if (_museum.batteryRequested) {
				say(33, 5, false, 1);
				say(36, 1);
				say(33, 6, false, 1);
			}
			say(33, 7, false, 1);
		}
		_museum.coffeeMachine = 1;
		_museum.willyInLab = 2;
		deferMuseumAction(448);
		break;
	case 408: { // Willy answers his telephone.
		byte frames[32];
		frames[0] = 54;
		for (uint i = 1; i < ARRAYSIZE(frames); ++i) {
			frames[i] = 74 + i;
		}
		_audio.playEffect(15);
		beginRoomAnimation(418, 4, 188, frames, ARRAYSIZE(frames), false);
		break;
	}
	case 418: // Play Willy's telephone conversation.
		_willyPose = 105;
		for (uint i = 0; i < 6; ++i) {
			say(91, i, false, 1);
		}
		deferMuseumAction(428);
		break;
	case 428: { // Willy puts the telephone away.
		byte frames[32];
		for (uint i = 0; i < ARRAYSIZE(frames) - 1; ++i) {
			frames[i] = 105 - i;
		}
		frames[31] = 54;
		beginRoomAnimation(438, 4, 188, frames, ARRAYSIZE(frames), false);
		break;
	}
	case 438: // Willy decides to visit the analysis laboratory.
		_willyPose = 54;
		say(91, 6, false, 1);
		say(81);
		_museum.willyMet = 2;
		_museum.willyInLab = 1;
		_museum.sawCode = 1;
		deferMuseumAction(448);
		break;
	case 448: // Willy leaves the close-up.
		beginRoomAnimation(458, 6, 72, nullptr, 0, false, 60);
		break;
	case 458: // Comment after Willy leaves to fetch stock or refill the coffee machine.
		holdRoomAnimation(409, 6, 72, 71, false);
		say(_museum.willyInLab == 1 ? 38 : 36, _museum.willyInLab == 1 ? 0 : 2);
		break;
	case 489: { // Willy turns back to his work.
		byte frames[ARRAYSIZE(kWillyTurnFrames)];
		for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
			frames[i] = kWillyTurnFrames[ARRAYSIZE(frames) - 1 - i];
		}
		beginRoomAnimation(409, 4, 188, frames, ARRAYSIZE(frames), false, 60);
		break;
	}
	case 409: // Return to the exhibition hall.
		_audio.stopEffect(1);
		_upperFloor = false;
		leaveMuseumRoom(4064, Common::Point(921, 813), 7);
		break;
	}
}

bool Scene4070::handleCustomDialogueReply() {
	if (_museum.willyMet == 1) {
		_dialogueEnding = true;
		_afterConversation = 408;
		return true;
	}
	return false;
}

void Scene4070::handleCustomDialogueChoice(byte effect) {
	if (effect == 5) {
		_museum.batteryRequested = 1;
	}
	if (effect == 10) {
		_dialogue[10][3].player = 22;
		_dialogue[10][3].response = 21;
	}
}

void Scene4070::runCustomDialogueIntro() {
	_inventory.add(13);
	_inventory.museum().businessCardExamined = 1;
	say(50);
	say(80, 4);
}

void Scene4070::finishCustomConversation() {
	deferMuseumAction(489);
}

uint Scene4070::sceneTextNumber() const {
	return 406;
}

void Scene4070::startWillyConversation() {
	startConversation(2);
	static const DialogueChoice willy[][8] = {
		{{0,0,1,6,5,1,255}, {0,1,1,7,6,5,255}, {0,2,1,8,7,1,255}, {0,0,3,9,8,1,255},
		 {0,0,3,10,9,1,255}, {0,3,1,11,10,1,255}, {1,0,0,12,11,0,0}},
		{{1,0,3,13,12,10,255}, {1,0,3,14,13,10,255}, {1,0,3,15,14,10,255}, {1,0,2,16,15,0,0}},
		{{1,0,3,17,16,1,255}, {1,0,1,18,17,1,255}, {1,0,2,19,18,0,0}},
		{{1,0,3,20,19,1,255}, {1,0,3,21,20,1,255}, {1,0,2,22,21,0,0}},
		{{1,0,3,23,22,2,0}, {1,0,3,24,23,2,0}},
		{{1,1,2,25,24,1,255}}
	};
	memcpy(_dialogue[0], willy[0], sizeof(willy[0]));
	for (uint i = 1; i <= 4; ++i) {
		memcpy(_dialogue[9 + i], willy[i], sizeof(willy[i]));
	}
	memcpy(_dialogue[20], willy[5], sizeof(willy[5]));
	_dialogue[0][1].enabled = _inventory.museum().voiceRecorded && _inventory.contains(6) && !_inventory.museum().frozenBattery;
	_dialogue[0][2].enabled = _museum.coffeeSeen && !_museum.coffeeMachine;
	_dialogue[0][5].enabled = _museum.willyMet == 2 && !_museum.labVisited;
	const uint count = 1 + _dialogue[0][1].enabled + _dialogue[0][2].enabled + _dialogue[0][5].enabled;
	_dialogue[0][0].enabled = count <= 4;
	_dialogue[0][3].enabled = count <= 3;
	_dialogue[0][4].enabled = count <= 2;
	if (!_museum.willyMet) {
		_museum.willyMet = 1;
		for (uint i = 0; i < 4; ++i) {
			say(80, i);
			say(90, i, false, 1);
		}
		_dialogueIntro = 1;
	} else {
		say(80, 5);
		say(90, 4, false, 1);
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
