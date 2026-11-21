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
#include "hollywood/runaway/scene8400.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

// Clip the original (2662, 186) approach to the room's right edge.
const Common::Point kTrail(2591, 229);
const Common::Point kTrailEnd(2445, 318);
const Common::Point kDoor(1104, 377);

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{2591, 229, 0, {301, 302}, {0, 0}, {2662, 186}, 5}, // 1: Path to the trailer.
	{0, 0, 0, {303, 304}, {0, 0}, {1063, 351}}, // 2: Trailer.
	{1104, 377, 7, {305, 306}, {0, -1}, {1063, 351}}, // 3: Door.
	{1138, 373, 0, {308, 309}, {1, 1}, {1138, 357}}, // 4: Tray.
	{0, 0, 0, {310, 311}, {0, 0}, {0, 0}} // 5: Wooden bridge.
};

const RunawayResponse kResponses[] = {
	{8400, 301, 1, 0}, // Look at the path back to Douglasville.
	{8400, 303, 2, 0}, // Examine the trailer.
	{8400, 308, 6, 0}, // Examine the papers in the tray.
	{8400, 310, 8, 0}, // Examine the wooden bridge.
	{8400, 311, 9, 0}  // Leave the bridge alone.
};

const RunawayItemAction kItemActions[] = {
	{8400, 21, 3, 316, -1}, // Force the trailer door with the poker.
	{8400, 0, 3, 317, false}   // Refuse other tools for opening the door.
};

const byte kTryDoorFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
	9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
	9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
};

const byte kForceDoorFrames[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
	23, 24, 25, 26, 27, 28, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8400 = {
	8400, 2592, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {80, 70, 60}, {70, 60, 50}, {90, 80, 70}}, kTrail,
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene8400::Scene8400(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene8400::loadCustomResources() {
	return _door.load(_archive, 4, 3) && _scenery.load(_archive, 9, 123) &&
		_parkedCar.load(_archive, 10, 2) && _feodor.load(_archive, 11, 126) &&
		_gustav.load(_archive, 12, 99) && _arrivingCar.load(_archive, 13, 111) && _brian.load(_archive, 15, 39);
}

void Scene8400::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[i] < 8) {
			_maps[i] = 0;
		}
		if (_maps[256 + i] == 4 && !_finale.trailerDoor) {
			_maps[256 + i] = 2;
		}
	}
}

void Scene8400::prepareCustomGameplayLoop() {
	_doorFrame = _finale.trailerDoor ? 2 : 0;
	_carParked = _finale.nunsHabit == 2 && !_finale.oscarAway;
	_sceneryFrame = _random.getRandomNumber(122);
	_sceneryBackwards = _random.getRandomBit();
	_sceneryTick = _now;
}

void Scene8400::runCustomEntrySequence() {
	if (_entryScene == 8403) {
		_cinematic = true;
		setCameraX(1568);
		beginRoomAnimationRange(420, 15, 39, 0, 14);
	} else if (_entryScene == 8401 || _entryScene == 8402) {
		_position = kDoor;
		_walkPath.reset(_position);
		_facing = 3;
		updateCamera();
		if (_entryScene == 8402) {
			_carParked = false;
		}
		beginRoomAnimation(410, 6, 36);
	} else {
		_facing = 5;
		walkTo(kTrailEnd, 5, true);
		deferAction(400);
	}
}

void Scene8400::resumeAmbientAudio(bool entering) {
	resumeFinaleOutdoorAudio(entering, 36, 51);
}

void Scene8400::panTo(int x, uint step, uint16 action) {
	_cameraTarget = x;
	_panStep = step;
	_panTick = _now;
	deferAction(action);
}

void Scene8400::advanceCustomGameplayLoop(uint32 delta) {
	updateFinaleOutdoorAudio();
	while (_now - _sceneryTick >= 80) {
		_sceneryTick += 80;
		if (!_sceneryFrame) {
			_sceneryBackwards = false;
		} else if (_sceneryFrame == 122) {
			_sceneryBackwards = true;
		}
		_sceneryFrame += _sceneryBackwards ? -1 : 1;
	}
	while (_cameraTarget >= 0 && _now - _panTick >= 10) {
		_panTick += 10;
		setCameraX(_camera.x + CLIP<int>(_cameraTarget - _camera.x, -(int)_panStep, _panStep));
		if (_camera.x == _cameraTarget) {
			_cameraTarget = -1;
		}
	}
	while ((_carMoving || _gangstersEntering) && _now - _castTick >= 80) {
		_castTick += 80;
		if (_carMoving && ++_carFrame == 110) {
			_carMoving = false;
			_carParked = true;
		}
		if (_gangstersEntering) {
			_gustavFrame = MIN<uint>(31, _gustavFrame + 1);
			_feodorFrame = MIN<uint>(20, _feodorFrame + 1);
			_gangstersEntering = _gustavFrame < 31;
		}
	}
}

bool Scene8400::updateCustomSequence() {
	return _cameraTarget >= 0 || _carMoving || _gangstersEntering;
}

void Scene8400::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	_door.draw(viewport, _doorFrame, _camera.x, 0);
	_scenery.draw(viewport, _sceneryFrame, _camera.x, 0);
	_parkedCar.draw(viewport, _carParked ? 0 : 1, _camera.x, 0);
}

void Scene8400::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_carMoving) {
		_arrivingCar.draw(viewport, _carFrame, _camera.x, 0);
	}
	if (_gangstersVisible) {
		_gustav.draw(viewport, _gustavFrame + (!_gangstersEntering && speaker() == 1 ? speechFrame() : 0), _camera.x, 0);
		_feodor.draw(viewport, _feodorFrame + (!_gangstersEntering && speaker() == 2 ? speechFrame() : 0), _camera.x, 0);
	}
	if (_brianPose >= 0 && !drawingRoomAnimation()) {
		const uint mouth = _brianPose == 14 && speaking() && !speaker() ? speechFrame() : 0;
		_brian.draw(viewport, _brianPose + mouth, _camera.x, 0);
	}
}

float Scene8400::actorHeight() const {
	if (_position.x < 1014) {
		return _position.x * -0.0769230798f + 276.923065f;
	}
	if (_position.x < 2474) {
		return _position.x * 0.0143575016f + 184.493896f;
	}
	return _position.y * 0.9550561905f - 130.505615f;
}

void Scene8400::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
	params.depthY[2] = walkZone() < 8 ? 599 : 0;
}

void Scene8400::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

bool Scene8400::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = CLIP<int>(target.x, 480, 2462);
	return false;
}

Common::U32String Scene8400::hotspotName(byte id) const {
	if (id == 3 && _finale.trailerDoor) {
		return _strings.hotspotName(_room, 8);
	}
	return RunawayRoomScene::hotspotName(id);
}

const char *Scene8400::actionCaptionVariant(byte id, bool look, byte item) const {
	if (!look && !item && id == 3 && _finale.trailerDoor) {
		return "open";
	}
	return nullptr;
}

void Scene8400::enterTrailer() {
	walkTo(kDoor, 7, false, true);
	beginRoomAnimation(307, 5, 37);
}

void Scene8400::withdraw(uint16 action) {
	static const byte frames[] = {14, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38};
	_brianPose = -1;
	beginRoomAnimation(action, 15, 39, frames, ARRAYSIZE(frames));
}

void Scene8400::handleAnimationFrameHook() {
	if (_roomAction == 306 && _actionStep == 15) {
		_audio.playEffect(50, 204);
	} else if (_roomAction == 316) {
		if (_actionStep == 20) {
			_audio.playEffect(52);
		} else if (_actionStep == 39) {
			_audio.playEffect(51, 255, 1);
		} else if (_actionStep == 41 || _actionStep == 42) {
			_doorFrame = _actionStep - 40;
		}
	} else if (_roomAction == 309 && _actionStep == 9) {
		_audio.playEffect(47, 127);
	} else if (_roomAction == 307) {
		if (_actionStep == 12 || _actionStep == 19 || _actionStep == 27 || _actionStep == 35) {
			_audio.playFootstep(5, _random.getRandomBit(), 112, 1065);
		}
	} else if (_roomAction == 410) {
		if (_actionStep == 7 || _actionStep == 17) {
			_audio.playFootstep(5, _random.getRandomBit(), 102, 1065);
		} else if (_actionStep == 25 || _actionStep == 33) {
			_audio.playFootstep(2, _random.getRandomBit(), 102, 1065);
		}
	} else if (_roomAction == 439) {
		const uint gustav = 43 + _actionStep;
		const uint feodor = _actionStep >= 5 ? 32 + _actionStep - 5 : 20;
		if (gustav == 53 || gustav == 84 || gustav == 87 || gustav == 94 || gustav == 98 ||
				feodor == 85 || feodor == 89 || feodor == 96 || feodor == 103 || feodor == 109 || feodor == 122) {
			_audio.playFootstep(5, _random.getRandomBit(), 191);
		}
	}
}

RunawayRoomScene::ActionResult Scene8400::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 400: // Reach the end of the path on the first visit.
		if (!_finale.trailerVisited) {
			_finale.trailerVisited = 1;
			say(0);
		}
		break;
	case 302: // Walk back to the path toward Douglasville.
		walkTo(kTrailEnd, 1, false, true);
		deferAction(401);
		break;
	case 401: // Continue beyond the right edge of the scene.
		walkTo(kTrail, 1, true);
		leaveRoom(8350);
		break;
	case 304: // Describe whether the trailer is still inaccessible.
		say(3, _finale.trailerDoor ? 2 : _finale.trailerDoorTried);
		break;
	case 305: // Examine the door before or after forcing it open.
		say(4, _finale.trailerDoor ? 2 : _finale.trailerDoorTried);
		break;
	case 306: // Enter the trailer, or try the locked door.
		if (_finale.trailerDoor) {
			enterTrailer();
		} else {
			walkTo(Common::Point(1071, 383), 7, false, true);
			beginRoomAnimation(306, 7, 33, kTryDoorFrames, ARRAYSIZE(kTryDoorFrames));
		}
		break;
	case 1306: // Notice that the door cannot be opened by hand.
		_finale.trailerDoorTried = 1;
		say(5);
		break;
	case 307: // Step through the open door.
		enterTrailer();
		break;
	case 1307: // Finish entering the trailer.
		leaveRoom(8410);
		break;
	case 316: // Use the poker to force the locked door.
		if (_finale.trailerDoor) {
			say(2, 0, true);
		} else {
			walkTo(Common::Point(1040, 378), 7, false, true);
			deferAction(402);
		}
		break;
	case 402: // Announce the plan to pry the door open.
		say(10);
		beginRoomAnimation(316, 8, 47, kForceDoorFrames, ARRAYSIZE(kForceDoorFrames));
		break;
	case 1316: // Keep the door open and go inside.
		_audio.playMusic(38, false);
		_finale.trailerDoor = 1;
		_doorFrame = 2;
		updateMaps();
		_facing = 0;
		say(10, 1);
		deferAction(307);
		break;
	case 317: // Explain why another inventory item will not open the door.
		say(_finale.trailerDoor ? 2 : 11, 0, _finale.trailerDoor != 0);
		break;
	case 309: { // Search the tray for useful papers.
		byte frames[24];
		frames[0] = 0;
		for (uint i = 1; i < ARRAYSIZE(frames); ++i) {
			frames[i] = i - 1;
		}
		say(7, _finale.bankBrochureTaken ? 3 : 0);
		beginRoomAnimation(309, 14, 42, frames, ARRAYSIZE(frames));
		break;
	}
	case 1309: // Read the bank brochure, or find nothing else in the tray.
		say(7, _finale.bankBrochureTaken ? 4 : 1);
		holdRoomAnimation(403, 14, 42, 22);
		break;
	case 1403: // Put the brochure in the inventory.
		if (!_finale.bankBrochureTaken) {
			_finale.bankBrochureTaken = 1;
			_inventory.add(14);
			say(7, 2);
		}
		holdRoomAnimation(405, 14, 42, 22);
		break;
	case 1405: // Finish rummaging through the papers.
		beginRoomAnimationRange(404, 14, 42, 23, 41);
		break;
	case 1404: // Return to exploration after searching the tray.
		break;
	case 1410: // Leave the trailer after taking the habit, or return to exploration.
		if (_entryScene == 8402) {
			say(20);
			deferAction(430);
		}
		break;
	case 1420: // Peek from behind the rock on a later visit.
		_brianPose = 14;
		say(0, 1);
		deferAction(421);
		break;
	case 421: // Look toward the trailer while remaining hidden.
		panTo(552, 8, 422);
		break;
	case 422: { // Overhear the gangsters inside the trailer.
		const byte variation = _finale.trailerWarnings * 4 + 2;
		say(0, variation, false, 1);
		say(0, variation + 1, false, 2);
		say(0, variation + 2, false, 1);
		deferAction(423);
		break;
	}
	case 423: // Pan back to Brian's hiding place.
		panTo(1568, 8, 424);
		break;
	case 424: // Decide against approaching the gangsters.
		say(0, _finale.trailerWarnings * 4 + 5);
		_finale.trailerWarnings = _finale.trailerWarnings == 1 ? 2 : 1;
		deferAction(425);
		break;
	case 425: // Duck back behind the rock.
		withdraw(425);
		break;
	case 1425: // Return to the overview without entering the occupied trailer.
		leaveRoom(8350);
		break;
	case 430: // Hear something approaching after leaving the trailer.
		_audio.playMusic(41);
		_facing = 5;
		say(20, 1);
		deferAction(431);
		break;
	case 431: // Recognize the sound of the gangsters' car.
		_facing = 4;
		_audio.playEffect(66);
		say(20, 2);
		deferAction(432);
		break;
	case 432: // Hurry out of sight before the car arrives.
		walkTo(Common::Point(1740, 410), 7, false, true);
		deferAction(433);
		break;
	case 433: // Follow the car toward the trailer.
		_cinematic = true;
		_carMoving = true;
		_carFrame = 30;
		_castTick = _now;
		panTo(0, 4, 434);
		break;
	case 434: // Return to Brian as he watches from behind the rock.
		_brianPose = 14;
		panTo(1568, 8, 435);
		break;
	case 435: // Identify the two visitors.
		say(20, 3);
		deferAction(436);
		break;
	case 436: // Hide before the gangsters get out of the car.
		withdraw(436);
		break;
	case 1436: // Pan to Gustav and Feodor approaching the trailer.
		_gangstersVisible = true;
		_gangstersEntering = true;
		_gustavFrame = _feodorFrame = 0;
		_castTick = _now;
		panTo(552, 4, 438);
		break;
	case 438: // Listen to Gustav and Feodor discuss their search.
		for (uint variation = 4; variation <= 12; ++variation) {
			say(20, variation, false, variation % 2 ? 2 : 1);
		}
		deferAction(439);
		break;
	case 439: { // Watch both gangsters walk through the trailer door.
		byte gustav[99], feodor[99];
		for (uint i = 0; i < ARRAYSIZE(gustav); ++i) {
			gustav[i] = MIN<uint>(98, 43 + i);
			feodor[i] = i < 5 ? 20 : MIN<uint>(125, 32 + i - 5);
		}
		_gangstersVisible = false;
		beginRoomAnimation(439, 12, 99, gustav, ARRAYSIZE(gustav));
		setRoomAnimationOverlay(11, 126, feodor, ARRAYSIZE(feodor));
		break;
	}
	case 1439: // Pan back after the gangsters have gone inside.
		_brianPose = 0;
		panTo(1568, 8, 440);
		break;
	case 440: // Check that the coast is clear.
		beginRoomAnimationRange(440, 15, 39, 0, 14);
		break;
	case 1440: // Decide to return to Douglasville for help.
		_brianPose = 14;
		say(20, 13);
		deferAction(441);
		break;
	case 441: // Restore the outdoor music and leave the occupied trailer.
		_audio.playMusic(12 + _random.getRandomNumber(4), false);
		leaveRoom(8350);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
