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
#include "hollywood/runaway/scene8300.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1397, 456, 1, {301, 302}, {0, 3}, {1397, 456}, 6}, // 1: Stairs.
	{40, 436, 7, {303, 304}, {0, 1}, {40, 436}, 4}, // 2: Door.
	{389, 381, 7, {303, 305}, {0, 1}, {389, 381}, 4}, // 3: Door.
	{644, 342, 7, {303, 306}, {0, 1}, {644, 342}, 4}, // 4: Door.
	{741, 518, 7, {307, 308}, {0, 0}, {726, 495}}, // 5: Crane controls.
	{0, 0, 0, {0, 0}, {0, 0}, {778, 599}},
	{862, 438, 7, {311, 312}, {0, 0}, {930, 417}}, // 7: Catapult.
	{862, 438, 7, {313, 314}, {0, 1}, {849, 423}}, // 8: Lever.
	{0, 0, 0, {0, 0}, {0, 0}, {996, 402}},
	{400, 452, 7, {317, 318}, {0, 0}, {400, 452}}, // 10: Water tank.
	{274, 489, 7, {319, 320}, {1, 1}, {233, 457}}, // 11: Tool panel.
	{0, 0, 1, {321, 322}, {0, 0}, {788, 341}}, // 12: Bed.
	{687, 525, 5, {323, 324}, {0, 1}, {494, 548}, 3} // 13: Saturn.
};

const RunawayResponse kResponses[] = {
	{8300, 301, 1, 0},  // Look down the stairs.
	{8300, 303, 2, 0},  // Look through a balcony door toward the hotel.
	{8300, 308, 4, 12}, // The crane is no longer needed.
	{8300, 311, 7, 1},  // Recognize Saturn's catapult.
	{8300, 312, 8, 1},  // Leave the catapult facing the mountain.
	{8300, 313, 9, 1},  // Identify the catapult lever.
	{8300, 314, 10, 7}, // The catapult is empty.
	{8300, 317, 13, 0}, // Examine the water tank.
	{8300, 318, 14, 0}, // Refuse to waste water.
	{8300, 319, 15, 1}, // Notice the missing number-ten wrench.
	{8300, 320, 16, 0}, // Leave Saturn's tools alone.
	{8300, 334, 27, 0}  // Refuse to fill an unsuitable container.
};

const RunawayItemAction kItemActions[] = {
	{8300, 8, 10, 334, false}, {8300, 12, 10, 334, false},
	{8300, 13, 10, 334, false}, {8300, 21, 10, 334, false},
	{8300, 10, 11, 338, false}
};

// The three sculpture variants share this sequence, including speech at step 90.
const byte kSaturnFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 0, 0, 0,
	37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56,
	57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 0, 0, 0, 0, 0, 0, 0,
	70, 71, 72, 73, 74, 75, 76, 77, 77, 77, 77, 78, 79, 80, 81, 76, 77, 78, 79, 80,
	81, 82, 77, 77, 77, 77, 77, 77, 77, 77, 76, 75, 74, 73, 72, 71, 70, 0
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8300 = {
	8300, 1620, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {90, 90, 90}, {80, 80, 80}, {70, 70, 70}, {60, 60, 60}},
	Common::Point(1397, 456), kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene8300::Scene8300(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	Scene6300(audio, inventory, strings, game) {}

bool Scene8300::loadCustomResources() {
	return loadWorkshopForeground() && loadForeground(0, 8) && loadForeground(1, 9) && loadForeground(2, 10);
}

bool Scene8300::applyCustomBackgroundPatches() {
	// Saturn makes progress during the séance and again when Oscar pursues the gangsters.
	_sculptureStage = _finale.gangsters == 2 ? 2 : _finale.seanceComplete ? 1 : 0;
	return (!_sculptureStage || applyPatch(5 + _sculptureStage)) &&
		_saturn.load(_archive, 11 + _sculptureStage, 84);
}

void Scene8300::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[i] >= 7 && _maps[i] <= 9) {
			_maps[i] = 0;
		}
	}
}

void Scene8300::prepareCustomGameplayLoop() {
	_saturnTick = _now;
}

void Scene8300::resumeAmbientAudio(bool entering) {
	_audio.stopEffect(2);
	if (entering || !_audio.musicPlaying()) {
		_audio.playMusic(24);
	}
}

void Scene8300::runCustomEntrySequence() {
	static const Common::Point entries[] = {
		Common::Point(1397, 456), Common::Point(158, 522), Common::Point(492, 425), Common::Point(719, 359)
	};
	_position = entries[_entryScene - 8300];
	_walkPath.reset(_position);
	_facing = _entryScene == 8300 ? 5 : 3;
	updateCamera();
	if (_entryScene == 8300) {
		walkTo(Common::Point(1369, 459), 5, true);
	}
}

void Scene8300::advanceCustomGameplayLoop(uint32 delta) {
	while (_now - _saturnTick >= 80) {
		_saturnTick += 80;
		advanceSaturn();
	}
}

void Scene8300::advanceSaturn() {
	switch (_saturnState) {
	case kWorking:
		if (_waitingForSaturn) {
			_saturnState = kTurning;
			_saturnStep = 80;
		} else if (!_random.getRandomNumber(19)) {
			if (_random.getRandomBit()) {
				_saturnState = kChiseling;
				_saturnStep = 1;
			} else {
				_saturnState = kStartGesture;
				_saturnStep = 40;
			}
		}
		break;
	case kStartGesture:
		if (_saturnStep < 59) {
			++_saturnStep;
		} else {
			_saturnState = _waitingForSaturn ? kFinishGesture : kGesture;
		}
		break;
	case kGesture:
		if (_waitingForSaturn || !_random.getRandomNumber(59)) {
			_saturnState = kFinishGesture;
			++_saturnStep;
		}
		break;
	case kFinishGesture:
	case kChiseling:
	case kReturning: {
		const uint last = _saturnState == kFinishGesture ? 73 : _saturnState == kChiseling ? 36 : 117;
		if (_saturnStep < last) {
			++_saturnStep;
			if (_saturnState == kChiseling && _saturnStep == 20 && _camera.x > 580) {
				_audio.playEffect(65, 204);
			}
		} else {
			_saturnState = kWorking;
			_saturnStep = 0;
		}
		break;
	}
	case kTurning:
		if (_saturnStep < 87) {
			++_saturnStep;
		} else {
			_saturnState = kTalking;
			_saturnStep = 90;
			_waitingForSaturn = false;
		}
		break;
	case kTalking:
		break;
	}
}

bool Scene8300::updateCustomSequence() {
	return _waitingForSaturn;
}

void Scene8300::drawSaturn(Graphics::ManagedSurface &viewport) {
	uint step = _saturnStep;
	if (_saturnState == kTalking && speaker()) {
		step += speechFrame();
	}
	_saturn.draw(viewport, kSaturnFrames[step], _camera.x, 0);
}

void Scene8300::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (_position.y >= 526) {
		drawSaturn(viewport);
	}
}

void Scene8300::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_position.y < 513) {
		drawForeground(viewport, _sculptureStage);
	}
	if (_position.y < 526) {
		drawSaturn(viewport);
	}
	drawWorkshopForeground(viewport);
}

Common::U32String Scene8300::hotspotName(byte id) const {
	return RunawayRoomScene::hotspotName(id);
}

RunawayRoomScene::ActionResult Scene8300::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Return downstairs to Rutger's greenhouse.
		leaveRoom(8241);
		break;
	case 304: // Go through the nearest balcony door.
		leaveRoom(8228);
		break;
	case 305: // Go through the middle balcony door.
		leaveRoom(8227);
		break;
	case 306: // Go through the far balcony door.
		leaveRoom(8226);
		break;
	case 307: // Recognize the crane controls from the earlier visit.
		_town.craneKnown = 1;
		say(3);
		break;
	case 321: // Examine Saturn's bed.
		say(17, _random.getRandomBit());
		break;
	case 322: // Decline to lie down in the bed.
		say(18, _random.getRandomBit());
		break;
	case 323: // Look at Saturn or comment on his current sculpture.
		if (_random.getRandomBit()) {
			say(19);
		} else {
			_facing = 4;
			say(19, 2);
		}
		break;
	case 324: // Greet Saturn before he turns away from his work.
		say(_finale.saturnMet ? 80 : 81);
		deferAction(400);
		break;
	case 338: // The earlier wrench interaction is inactive in this chapter.
		break;
	case 400: // Let Saturn finish his current gesture and face Brian.
		_waitingForSaturn = true;
		deferAction(401);
		break;
	case 401: // Begin the conversation once Saturn is facing Brian.
		startSaturnConversation();
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene8300::startSaturnConversation() {
	startConversation(1);
	static const DialogueChoice topics[] = {
		{1, 0, 3, 4, 4, 1, 255}, {1, 0, 3, 5, 5, 1, 255},
		{0, 0, 1, 6, 6, 3, 255}, {1, 0, 0, 10, 10, 1, 0}
	};
	memcpy(_dialogue[0], topics, sizeof(topics));
	_dialogue[0][2].enabled = _finale.mediumTests >= 3 && !_finale.trailerFound;
	_dialogue[10][0] = {1, 0, 2, 7, 7, 2, 255};
	if (_finale.saturnMet) {
		say(90, 0, false, 1);
	} else {
		say(91, 0, false, 1);
		for (uint i = 1; i <= 3; ++i) {
			say(81, i);
			say(91, i, false, 1);
		}
		_finale.saturnMet = 1;
	}
}

void Scene8300::handleCustomDialogueChoice(byte effect) {
	if (effect == 2) { // Interrupt Saturn's reminiscence to get directions to the trailer.
		for (uint i = 8; i <= 9; ++i) {
			say(81, i);
			say(91, i, false, 1);
		}
	} else if (effect == 3) { // Saturn knows the trailer, unlocking the route on the overview.
		_finale.trailerFound = 1;
	}
}

void Scene8300::finishCustomConversation() {
	_saturnState = kReturning;
	_saturnStep = 90;
	_saturnTick = _now;
}

} // End of namespace Runaway
} // End of namespace Hollywood
