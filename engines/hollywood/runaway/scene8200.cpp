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
#include "hollywood/runaway/scene8200.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1379, 491, 0, {301, 302}, {0, 3}, {1379, 491}, 5}, // 1: Street.
	{285, 501, 0, {303, 305}, {1, 0}, {258, 427}}, // 2: Cell.
	{1020, 483, 0, {315, 316}, {0, 1}, {1020, 480}}, // 3: Bag holding the thugs' stuff.
	{0, 0, 0, {0, 0}, {0, 0}, {536, 425}},
	{1020, 483, 0, {308, 309}, {0, 0}, {1020, 458}}, // 5: Desk.
	{1020, 483, 0, {310, 311}, {0, 0}, {1020, 458}}, // 6: Gun cabinet.
	{857, 490, 0, {312, 313}, {0, 0}, {857, 435}}, // 7: Bulletin board.
	{285, 501, 0, {306, 307}, {0, 0}, {271, 444}, 3, true} // 8: Gustav and Feodor.
};

const RunawayResponse kResponses[] = {
	{8200, 301, 1, 0},  // Look out toward the saloon.
	{8200, 306, 22, 0}, // Examine the captured gangsters.
	{8200, 307, 23, 0}, // Refuse to chat with Gustav and Feodor.
	{8200, 308, 6, 0},  // Examine the desk.
	{8200, 309, 7, 0},  // Search the desk.
	{8200, 310, 8, 0},  // Examine the gun cabinet.
	{8200, 311, 9, 0},  // Try to take the cabinet's chain.
	{8200, 312, 10, 0}, // Read the wanted posters.
	{8200, 313, 11, 0}  // Try to take the wanted posters.
};

const RunawayItemAction kItemActions[] = {
	{8200, 16, 8, 317, false} // Record the prisoners' voices.
};

// Animation order includes the mouth shapes for standing and leaning on the bars.
const byte kGustavFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 49, 49,
	49, 49, 49, 49, 49, 49, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
	73, 74, 75, 76, 77, 78, 89, 80, 81, 82, 83, 84, 84, 84, 84, 84,
	84, 84, 84, 84, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	84, 84, 84, 84, 84, 84, 84, 84, 83, 82, 81, 80, 79, 78, 77, 76,
	75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 49, 49, 49,
	49, 49, 49, 49, 49, 49, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107,
	106, 105, 104, 103, 102, 101, 100, 99, 98, 84, 48, 47, 46, 45, 44, 43,
	42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27,
	26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11,
	10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0
};

const byte kFeodorFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 35, 35, 35, 35, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 35, 35, 35, 35, 35, 35, 35, 35, 34, 33, 32, 31,
	30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15,
	14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0,
	0, 0, 0, 0, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
	60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75,
	76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
	92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107,
	108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 108, 108, 108, 108,
	108, 108, 108, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 98, 97,
	96, 95, 94, 94, 94, 94, 94, 94, 95, 96, 97, 98, 99, 100, 101, 102,
	103, 104, 105, 106, 107, 108, 108, 108, 108, 108, 108, 108, 107, 107, 106, 105,
	104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89,
	88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73,
	72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57,
	56, 55, 54, 53, 52, 51, 50, 49, 48, 35
};

// Inclusive ranges in the order tables, indexed by GangsterPose.
const uint16 kGangsterSteps[2][10][2] = {
	{{0, 0}, {1, 49}, {170, 218}, {50, 50}, {70, 91},
	 {100, 100}, {120, 141}, {150, 159}, {159, 159}, {160, 169}},
	{{0, 0}, {1, 35}, {60, 94}, {40, 40}, {100, 160},
	 {160, 160}, {220, 281}, {180, 194}, {194, 194}, {200, 214}}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8200 = {
	8200, 1380, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {85, 80, 80}, {65, 60, 65}, {55, 50, 55}, {45, 40, 45}, {55, 50, 55}},
	Common::Point(1227, 510),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene8200::Scene8200(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game), _tauntSpeech(audio, _random) {}

bool Scene8200::usesAlternateActor() const {
	return false;
}

bool Scene8200::loadCustomResources() {
	return _cellDoorAnimation.load(_archive, 6, 26) &&
		_gangsterAnimation[0].load(_archive, 8, 125) && _gangsterAnimation[1].load(_archive, 9, 125);
}

bool Scene8200::applyCustomBackgroundPatches() {
	return applyPatch(4) && applyPatch(_finale.gangsterBag == 1 || _finale.gangsterBag == 2 ? 10 : 11);
}

void Scene8200::applyCustomHotspotState() {
	const bool bagPresent = _finale.gangsterBag == 1 || _finale.gangsterBag == 2;
	for (uint i = 0; i < 256; ++i) {
		// Free walking stays outside the cell; its entrance uses a scripted walk.
		if (_maps[i] > 1) {
			_maps[i] = 0;
		}
		byte &id = _maps[256 + i];
		if (id == 8 && !_finale.oscarAway) {
			id = 2;
		} else if (id == 4) {
			id = bagPresent ? 3 : 6;
		} else if (id == 3 && !bagPresent) {
			id = 0;
		}
	}
}

void Scene8200::prepareCustomGameplayLoop() {
	_gangsterElapsed = 0;
	stopAmbientSpeech();
	if (!_gangstersInitialized) {
		for (uint i = 0; i < ARRAYSIZE(_gangsters); ++i) {
			setGangsterPose(i, _finale.gangstersMet && _random.getRandomBit() ? kStanding : kSeated);
		}
		_gangsters[_random.getRandomBit()].replyPending = true;
		_gangstersInitialized = true;
	}
}

void Scene8200::runCustomEntrySequence() {
	_facing = 5;
	if (_entryScene == 8201) {
		_position = Common::Point(258, 445);
		_walkPath.reset(_position);
		updateCamera();
		walkTo(Common::Point(211, 506), 5, true);
	} else if (_finale.oscarAway && !_finale.gangstersMet) {
		walkTo(Common::Point(814, 525), 7);
		deferAction(400);
	}
}

void Scene8200::resumeAmbientAudio(bool entering) {
	resumeFinaleJailAudio(entering);
}

void Scene8200::advanceCustomGameplayLoop(uint32 delta) {
	updateFinaleJailAudio();
	if (_tauntSpeaker && _tauntSpeech.finished(_now)) {
		++_tauntLine;
		if (_tauntLine < MAX<uint>(1, _tauntCue.continuationCount)) {
			showTauntLine();
		} else {
			stopAmbientSpeech();
		}
	}
	_gangsterElapsed += delta;
	while (_gangsterElapsed >= 80) {
		_gangsterElapsed -= 80;
		if (!_finale.oscarAway) {
			continue;
		}
		for (uint i = 0; i < ARRAYSIZE(_gangsters); ++i) {
			updateGangster(i);
		}
	}
}

void Scene8200::setGangsterPose(uint index, GangsterPose pose) {
	_gangsters[index].pose = pose;
	_gangsters[index].step = kGangsterSteps[index][pose][0];
}

void Scene8200::updateGangster(uint index) {
	Gangster &gangster = _gangsters[index];
	const bool summoned = _gangsterMovement == kComeToBars;
	const bool idle = _gangsterMovement == kIdle && !busy() && !_inventoryOpen;
	const uint16 first = kGangsterSteps[index][gangster.pose][0];
	const uint16 last = kGangsterSteps[index][gangster.pose][1];
	const uint16 oldStep = gangster.step;

	switch (gangster.pose) {
	case kSeated:
		if (summoned || !_random.getRandomNumber(34)) {
			setGangsterPose(index, kRising);
		}
		break;
	case kStanding:
	case kAtBars:
		if (summoned && gangster.pose == kStanding) {
			setGangsterPose(index, kApproaching);
		} else if (idle && _tauntSpeaker != index + 1) {
			if (gangster.step != first) {
				gangster.step = first;
			} else if (!_random.getRandomNumber(14)) {
				gangster.step = first + 6;
			} else if (gangster.pose == kStanding) {
				if (_random.getRandomNumber(4) < 2) {
					setGangsterPose(index, kApproaching);
				} else if (!_tauntSpeaker && gangster.replyPending) {
					startTaunt(index, 5);
					gangster.replyPending = false;
				}
			} else {
				const uint choice = _random.getRandomNumber(19);
				if (choice == 0 || choice == 2 || choice == 3) {
					setGangsterPose(index, kRetreating);
				} else if (choice == 1) {
					setGangsterPose(index, kTurningAside);
				} else if (choice == 4 && !_tauntSpeaker) {
					byte variation;
					do {
						variation = _random.getRandomNumber(4);
					} while (variation == gangster.lastTaunt);
					startTaunt(index, variation);
					gangster.lastTaunt = variation;
					gangster.replyPending = false;
					_gangsters[1 - index].replyPending = true;
				}
			}
		}
		break;
	case kAside:
		if (summoned || !_random.getRandomNumber(14)) {
			setGangsterPose(index, kReturning);
		}
		break;
	default:
		if (gangster.step < last) {
			++gangster.step;
		} else {
			switch (gangster.pose) {
			case kRising:
				setGangsterPose(index, summoned || (idle && _position.x <= 474) ? kApproaching : kStanding);
				break;
			case kSittingDown:
				setGangsterPose(index, kSeated);
				break;
			case kApproaching:
			case kReturning:
				setGangsterPose(index, kAtBars);
				break;
			case kRetreating:
				setGangsterPose(index, kStanding);
				break;
			case kTurningAside:
				setGangsterPose(index, summoned ? kReturning : kAside);
				break;
			default:
				break;
			}
		}
		break;
	}
	if (gangster.step != oldStep) {
		if (!index && (gangster.step == 9 || gangster.step == 82 || gangster.step == 128 || gangster.step == 209)) {
			_audio.playEffect(53, 178, 1);
		} else if (index && (gangster.step == 124 || gangster.step == 246)) {
			_audio.playEffect(54, 178, 1);
		}
	}
}

void Scene8200::moveGangsters(GangsterMovement movement) {
	stopAmbientSpeech();
	_gangsterMovement = movement;
	if (movement == kLeaveBars) {
		for (uint i = 0; i < ARRAYSIZE(_gangsters); ++i) {
			setGangsterPose(i, kRetreating);
		}
	}
}

bool Scene8200::updateCustomSequence() {
	if (_gangsterMovement == kIdle) {
		return false;
	}
	const GangsterPose destination = _gangsterMovement == kComeToBars ? kAtBars : kStanding;
	for (uint i = 0; i < ARRAYSIZE(_gangsters); ++i) {
		if (_gangsters[i].pose != destination) {
			return true;
		}
	}
	_gangsterMovement = kIdle;
	return false;
}

void Scene8200::startTaunt(uint index, byte variation) {
	_tauntCue = _text.stageCue(15 + index, variation);
	if (_tauntCue.valid()) {
		_tauntSpeaker = index + 1;
		_tauntLine = 0;
		showTauntLine();
	}
}

void Scene8200::showTauntLine() {
	const Common::U32String text(_text.largeTextRecord(_tauntCue.textRecordId + _tauntLine), Common::kDos850);
	const uint16 sample = _tauntCue.voiceSampleId ? _tauntCue.voiceSampleId + _tauntLine : 0;
	_tauntSpeech.start(text, sample, false, _now);
}

void Scene8200::stopAmbientSpeech() {
	if (_tauntSpeaker) {
		_tauntSpeech.stop();
		_tauntSpeaker = 0;
	}
}

const Common::U32String &Scene8200::subtitle() const {
	if (_tauntSpeaker) {
		return _tauntSpeech.subtitle();
	}
	return RunawayRoomScene::subtitle();
}

void Scene8200::sayInventory(const RunawayInventory::Reply &reply) {
	stopAmbientSpeech();
	RunawayRoomScene::sayInventory(reply);
}

void Scene8200::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (_finale.oscarAway) {
		for (uint i = 0; i < ARRAYSIZE(_gangsters); ++i) {
			const Gangster &gangster = _gangsters[i];
			uint step = gangster.step;
			const bool talking = speaker() == i + 1 || _tauntSpeaker == i + 1;
			if (talking && (gangster.pose == kStanding || gangster.pose == kAtBars)) {
				step = kGangsterSteps[i][gangster.pose][0] + (_tauntSpeaker ? _tauntSpeech.frame(_now) : speechFrame());
			}
			const byte frame = i ? kFeodorFrames[step] : kGustavFrames[step];
			_gangsterAnimation[i].draw(viewport, frame, _camera.x, -_camera.y);
		}
	}
	_cellDoorAnimation.draw(viewport, _finale.oscarAway ? 0 : 25, _camera.x, -_camera.y);
}

float Scene8200::actorHeight() const {
	return _position.y > 501 ? _position.y * 0.618556678f + 84.4845352f :
		_position.y * 1.38297868f - 299.255310f;
}

void Scene8200::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
}

void Scene8200::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

Common::Point Scene8200::customizeSceneActionPosition(byte id, byte item, Common::Point position) const {
	return id == 2 && !_finale.oscarAway ? Common::Point(258, 445) : position;
}

byte Scene8200::customizeSceneActionFacing(byte id, byte facing) const {
	return id == 2 && !_finale.oscarAway ? 4 : facing;
}

const char *Scene8200::actionCaptionVariant(byte id, bool look, byte item) const {
	if (id == 2 && !look && !item && _finale.oscarAway) {
		return "locked";
	}
	return nullptr;
}

RunawayRoomScene::ActionResult Scene8200::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Return to the street.
		stopAmbientSpeech();
		leaveRoom(8223);
		break;
	case 303: // Examine the cell, now locked if Oscar has captured the gangsters.
		say(_finale.oscarAway ? 3 : 2);
		break;
	case 305: // Enter the open cell, or try its locked door.
		if (_finale.oscarAway) {
			say(3);
		} else {
			if (findRunawayRoom(8210)) {
				walkTo(Common::Point(258, 445), 0, true);
			}
			leaveRoom(8210);
		}
		break;
	case 315: // Examine the gangsters' belongings.
		say(20);
		if (_finale.gangsterBag == 1) {
			_finale.gangsterBag = 2;
		}
		break;
	case 316: // Decide to take the bag.
		say(21);
		deferAction(416);
		break;
	case 416: // Collect the bag from the cabinet.
		_facing = 4;
		_finale.gangsterBag = 3;
		_inventory.add(13);
		say(21, 1);
		return kBackgroundChanged;
	case 317: // Use the MP3 recorder on Gustav and Feodor.
		if (_finale.recordings & 2) {
			say(24, 4);
		} else {
			_inventory.select(0);
			say(24);
			deferAction(420);
		}
		break;
	case 400: // Greet the newly captured gangsters.
		say(0, 2);
		deferAction(401);
		break;
	case 401: // Bring both prisoners to the bars.
		moveGangsters(kComeToBars);
		deferAction(402);
		break;
	case 402: // Walk up to the prisoners.
		walkTo(Common::Point(285, 501), 0, false, true);
		deferAction(403);
		break;
	case 403: // Hear their threats and explain Oscar's role in their capture.
		say(0, 3, false, 1);
		say(0, 4, false, 2);
		say(0, 5);
		say(0, 6, false, 1);
		say(0, 7, false, 2);
		say(0, 8);
		say(0, 9, false, 1);
		say(0, 10, false, 2);
		say(0, 11);
		deferAction(404);
		break;
	case 404: // Step away from the angry prisoners.
		walkTo(Common::Point(901, 538), 4);
		deferAction(405);
		break;
	case 405: // Finish the first visit.
		say(0, 12);
		_finale.gangstersMet = 1;
		_gangsters[0].replyPending = true;
		_gangsters[1].replyPending = false;
		break;
	case 420: // Approach the bars with the recorder.
		walkTo(Common::Point(285, 501), 0, false, true);
		deferAction(421);
		break;
	case 421: // Take out and switch on the recorder.
		beginRoomAnimationRange(421, 7, 50, 0, 29);
		break;
	case 1421: // Attract their attention while holding the recorder.
		say(24, 1);
		holdRoomAnimation(422, 7, 50, 29);
		break;
	case 1422: // Wait for both gangsters to approach the bars.
		moveGangsters(kComeToBars);
		holdRoomAnimation(423, 7, 50, 29);
		break;
	case 1423: // Record Gustav and Feodor threatening Brian.
		say(15, 0, false, 1);
		say(16, 1, false, 2);
		say(15, 3, false, 1);
		say(16, 4, false, 2);
		holdRoomAnimation(424, 7, 50, 29);
		break;
	case 1424: // Keep recording as they step away from the bars.
		moveGangsters(kLeaveBars);
		holdRoomAnimation(425, 7, 50, 29);
		break;
	case 1425: // Record their final remarks.
		say(15, 5, false, 1);
		say(16, 5, false, 2);
		say(24, 2);
		holdRoomAnimation(426, 7, 50, 29);
		break;
	case 1426: // Switch off and put away the recorder.
		_audio.playEffect(49);
		beginRoomAnimationRange(427, 7, 50, 30, 49);
		break;
	case 1427: // Keep the voices and move away from the cell.
		_finale.recordings |= 2;
		walkTo(Common::Point(901, 538), 4);
		deferAction(428);
		break;
	case 428: // Confirm that the recording will be useful.
		say(24, 3);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene8200::handleAnimationFrameHook() {
	if (_roomAction == 421 && _actionStep == 27) {
		_audio.playEffect(48);
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
