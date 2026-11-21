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
#include "common/textconsole.h"
#include "common/util.h"
#include "hollywood/runaway/scene8380.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{866, 268, 0, {301, 302}, {0, 3}, {798, 250}, 7}, // 1: Stairs.
	{1023, 318, 0, {303, 304}, {0, 1}, {1123, 300}, 6}, // 2: Other side.
	{520, 410, 7, {305, 306}, {0, 1}, {442, 338}, 3}, // 3: Sushi.
	{553, 363, 0, {307, 308}, {0, 0}, {616, 346}}, // 4: Table.
	{0, 0, 0, {319, 321}, {0, 0}, {641, 214}}, // 5: Machines.
	{0, 0, 0, {311, 312}, {0, 0}, {46, 258}}, // 6: Satellite antenna.
	{0, 0, 0, {313, 314}, {0, 0}, {194, 247}}, // 7: Musical equipment.
	{0, 0, 0, {315, 316}, {0, 0}, {760, 393}}, // 8: Robot.
	{0, 0, 0, {317, 318}, {0, 0}, {508, 200}}, // 9: Fireplace.
	{0, 0, 0, {309, 310}, {0, 0}, {398, 321}}, // 10: Sushi's computer.
	{0, 0, 0, {320, 321}, {0, 0}, {305, 220}}, // 11: Machines.
	{0, 0, 0, {322, 323}, {0, 0}, {929, 385}} // 12: Electric guitar.
};

const RunawayResponse kResponses[] = {
	{8380, 301, 1, 0},  // Look toward the stairs.
	{8380, 303, 2, 0},  // Examine the narrow walkway.
	{8380, 307, 4, 0},  // Examine the equipment on the table.
	{8380, 308, 5, 0},  // Leave Sushi's equipment alone.
	{8380, 310, 7, 0},  // Ask Sushi instead of using her computer.
	{8380, 312, 9, 0},  // Avoid disturbing the antenna's alignment.
	{8380, 314, 11, 0}, // Leave the musical equipment alone.
	{8380, 316, 13, 0}, // Leave the robot in place.
	{8380, 317, 14, 0}, // Examine the unlit fireplace.
	{8380, 318, 15, 0}, // It is too hot to light a fire.
	{8380, 319, 16, 0}, // Examine the radio equipment.
	{8380, 320, 17, 0}, // Examine the database machines.
	{8380, 321, 18, 0}, // Avoid breaking the machines.
	{8380, 323, 20, 0}  // Brian cannot play the guitar.
};

const RunawayItemAction kItemActions[] = {
	{8380, 13, 3, 326}, // Give Sushi the gangsters' belongings.
	{8380, 14, 3, 324}, // Ask Sushi to investigate the bank brochure.
	{8380, 16, 3, 325}, // Hand over the recorded voices.
	{8380, 9, 7, 31, false}, // Refuse to damage the musical equipment with the tomahawk.
	{8380, 9, 8, 31, false}  // Refuse to damage the robot with the tomahawk.
};

// RESOURCE.H38 shares a body bank between typing, turning and the microphone.
const byte kSushiFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 10, 10, 10, 10,
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
	56, 57, 58, 58, 58, 58, 58, 58, 58, 58, 58, 59, 60, 61, 62, 63, 58, 59, 60, 61,
	62, 63, 58, 58, 58, 58, 58, 58, 58, 58, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
	75, 76, 77, 78, 79, 10, 10, 10, 10, 10, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,
	91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
	111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130,
	131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 96, 95, 94, 93,
	92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 10, 10, 10, 10, 10, 10, 10, 10,
	147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166,
	166, 167, 168, 169, 170, 171, 166, 167, 168, 169, 170, 171, 166, 166, 166, 166, 166, 166, 166, 166,
	165, 164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 10
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8380 = {
	8380, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {60, 60, 60}, {40, 40, 40}}, Common::Point(866, 268),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene8380::Scene8380(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene8380::loadCustomResources() {
	static const uint frames[] = {48, 48, 6, 4, 11, 48, 11, 11, 11, 4, 4, 4, 6, 6, 6, 6};
	for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
		if (!_equipment[i].load(_archive, 9 + i, frames[i])) {
			return false;
		}
	}
	_gog = Common::File::exists("DATAVC00.000");
	return _sushi.load(_archive, 4, 173) && _antenna.load(_archive, 5, 97) &&
		_monitor.load(_archive, 25, 953) && loadForeground(0, 6) && loadForeground(1, 7);
}

bool Scene8380::applyCustomBackgroundPatches() {
	if (_recorderOnDesk || _finale.voiceEditing == 1) {
		return applyPatch(29);
	}
	if (_finale.bankResearch == 1) {
		return applyPatch(28);
	}
	return applyPatch(_finale.bagDelivered == 1 ? 27 : 26);
}

void Scene8380::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[i] == 6) {
			_maps[i] = 0;
		}
	}
}

void Scene8380::prepareCustomGameplayLoop() {
	_sushiTick = _now;
}

void Scene8380::resumeAmbientAudio(bool entering) {
	_audio.stopEffect(2);
	if (entering || !_audio.musicPlaying()) {
		_audio.playMusic(21);
	}
	_audio.setMusicVolume(255);
}

void Scene8380::runCustomEntrySequence() {
	if (_entryScene == 8381) {
		_position = Common::Point(975, 291);
		_facing = 5;
	} else if (_entryScene == 8382 || _entryScene == 8383 || _entryScene == 8385) {
		_position = Common::Point(520, 410);
		_facing = 7;
	} else {
		_position = Common::Point(866, 268);
		_facing = 3;
	}
	_walkPath.reset(_position);
	updateCamera();
	switch (_entryScene) {
	case 8380: // Meet Sushi, or tell her what happened during the séance.
		if (!_finale.sushiVisited || (_finale.seanceComplete && !_finale.sushiTruthTold)) {
			walkTo(Common::Point(520, 410), 7, false, true);
			turnSushi(_finale.sushiVisited ? 404 : 400);
		} else {
			walkTo(Common::Point(877, 271), 3, false, true);
		}
		break;
	case 8382: // Sushi has worked out the voice-impersonation plan.
		_recorderOnDesk = true;
		if (!rebuildBackground()) {
			error("Cannot show Runaway's recorder on Sushi's desk");
		}
		turnSushi(410);
		break;
	case 8383: // Return after Sushi has edited the recordings.
		turnSushi(420);
		break;
	case 8384: // Hear the results of Sushi's bank investigation.
		_finale.bankResearch = 3;
		walkTo(Common::Point(520, 410), 7, false, true);
		turnSushi(406);
		break;
	case 8385: // Tell Sushi that Oscar has jailed the gangsters.
		turnSushi(408);
		break;
	default:
		break;
	}
}

void Scene8380::turnSushi(uint16 action, bool microphone) {
	_sushiGoal = microphone ? kMicrophoneSpeech : kTalking;
	_waitingForSushi = true;
	if (_sushiState == kTalking && microphone) {
		returnToComputer();
	} else if (_sushiState == kMicrophoneSpeech && !microphone) {
		_sushiState = kLeaveMicrophone;
		_sushiStep = 260;
	}
	deferAction(action);
}

void Scene8380::returnToComputer() {
	_sushiState = kReturnToComputer;
	_sushiStep = 110;
}

void Scene8380::advanceSushi() {
	switch (_sushiState) {
	case kSitDown:
		if (_sushiStep < 10) {
			++_sushiStep;
		} else {
			_sushiState = kTyping;
		}
		break;
	case kTyping:
		if (_sushiStep < 36) {
			if (++_sushiStep == 12) {
				_audio.playEffect(44, 178, 1);
			}
		} else if (_waitingForSushi) {
			_sushiState = _sushiGoal == kTalking ? kTurnToBrian : kUseMicrophone;
			_sushiStep = _sushiGoal == kTalking ? 60 : 220;
		} else if (!_random.getRandomNumber(4)) {
			const uint gesture = _random.getRandomNumber(2);
			_sushiState = gesture == 0 ? kLeanBack : gesture == 1 ? kTyping : kComputerGesture;
			_sushiStep = gesture == 0 ? 40 : gesture == 1 ? 11 : 130;
		}
		break;
	case kLeanBack:
		if (_sushiStep < 49) {
			++_sushiStep;
		} else {
			_sushiState = kResting;
		}
		break;
	case kResting:
		if (_waitingForSushi || !_random.getRandomNumber(49)) {
			_sushiState = kSitDown;
			_sushiStep = 0;
		}
		break;
	case kTurnToBrian:
		if (_sushiStep < 82) {
			++_sushiStep;
		} else {
			_sushiState = kTalking;
			_sushiStep = 90;
		}
		break;
	case kReturnToComputer:
		if (_sushiStep < 124) {
			++_sushiStep;
		} else {
			_sushiState = kTyping;
			_sushiStep = 10;
		}
		break;
	case kComputerGesture:
		if (_sushiStep < 213) {
			++_sushiStep;
		} else {
			_sushiState = kTyping;
			_sushiStep = 10;
		}
		break;
	case kUseMicrophone:
		if (_sushiStep < 239) {
			++_sushiStep;
		} else {
			_sushiState = kMicrophoneSpeech;
			_sushiStep = 240;
		}
		break;
	case kLeaveMicrophone:
		if (_sushiStep < 278) {
			++_sushiStep;
		} else {
			_sushiState = kTurnToBrian;
			_sushiStep = 60;
		}
		break;
	default:
		break;
	}
}

void Scene8380::advanceCustomGameplayLoop(uint32 delta) {
	while (_now - _sushiTick >= 80) {
		_sushiTick += 80;
		advanceSushi();
	}
	if (_phoneMoving) {
		const uint elapsed = _phone.frameAtTime(_now - _phoneStarted);
		if (!_phone.decodeToFrame(MIN(_phoneLast, _phoneFirst + elapsed))) {
			error("Cannot decode Runaway's phone call");
		}
		if (elapsed > _phoneLast - _phoneFirst) {
			_phoneMoving = false;
		}
	}
}

bool Scene8380::updateCustomSequence() {
	if (_waitingForSushi && _sushiState == _sushiGoal) {
		_waitingForSushi = false;
	}
	return _waitingForSushi || _phoneMoving;
}

void Scene8380::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	uint monitorFrame;
	if (_sushiState == kTalking || _sushiState == kTurnToBrian) {
		const uint step = (_now / 80) % 356;
		monitorFrame = 774 + (step < 178 ? step : 356 - step);
	} else if (_sushiState == kComputerGesture) {
		monitorFrame = 426 + (_now / 160) % 348;
	} else {
		monitorFrame = (_now / 80) % 146;
	}
	_monitor.draw(viewport, monitorFrame, 0, 0);
	drawAmbientAnimations(viewport, _equipment, ARRAYSIZE(_equipment));
	_antenna.draw(viewport, (_now / 80) % 97, 0, 0);
	const bool talking = (_sushiState == kTalking || _sushiState == kMicrophoneSpeech) && speaker() == 1;
	const uint step = _sushiStep + (talking ? speechFrame() : 0);
	_sushi.draw(viewport, kSushiFrames[step], 0, 0);
}

void Scene8380::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_position.y < 283) {
		drawForeground(viewport, 0);
	}
	if (_position.y < 376) {
		drawForeground(viewport, 1);
	}
	if (_phoneActive) {
		// The telephone appears over the lower left of the room, not full screen.
		// The GOG release crops eight pixels from the bottom and right edges.
		Graphics::ManagedSurface phone(viewport, Common::Rect(0, 312, _gog ? 280 : 288, _gog ? 592 : 600));
		const Common::Rect destination(288, 288);
		_phone.draw(phone, destination);
		if (!_phoneMoving && !_phonePortraits.draw(phone, destination, _phonePortrait,
				speaker() == 2 ? speechFrame() : 0)) {
			error("Cannot draw Runaway's phone portrait");
		}
	}
}

float Scene8380::actorHeight() const {
	return (_position.x * 0.019550342f + 230.0f +
		_position.y * 1.476510048f - 311.275177f) / 2;
}

void Scene8380::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

bool Scene8380::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = CLIP<int>(target.x, 403, 929);
	return false;
}

void Scene8380::exchange(uint16 cue, byte first, byte last) {
	for (uint i = first; i <= last; ++i) {
		say(cue, i);
		say(cue + 10, i, false, 1);
	}
}

void Scene8380::deliverItem(uint16 action) {
	returnToComputer();
	walkTo(Common::Point(553, 363), 0, false, true);
	beginRoomAnimation(action, 8, 26);
}

void Scene8380::receiveRecordings() {
	exchange(83, 0, 0);
	if (_finale.recordings < 3) {
		exchange(83, 1, 1);
		say(83, 2);
		deferAction(402);
	} else {
		exchange(83, 3, 4);
		deferAction(442);
	}
}

RunawayRoomScene::ActionResult Scene8380::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Descend to the hotel lobby.
		leaveRoom(8371);
		break;
	case 304: // Cross to the walkway above the lobby.
		leaveRoom(8372);
		break;
	case 305: // Examine Sushi.
		say(3, _random.getRandomBit());
		break;
	case 306: // Turn Sushi toward Brian before starting a conversation.
		turnSushi(450);
		break;
	case 309: // Examine the computer and plasma monitor.
		say(6, _random.getRandomBit());
		break;
	case 311: // Examine the satellite antenna.
		say(8, _random.getRandomBit());
		break;
	case 313: // Examine the speakers and musical equipment.
		say(10, _random.getRandomBit());
		break;
	case 315: // Recognize the science-fiction robot.
		say(12, _random.getRandomBit());
		break;
	case 322: // Examine the electric guitar.
		say(19, _random.getRandomNumber(2));
		break;
	case 324: // Offer Sushi the bank brochure.
		turnSushi(440);
		break;
	case 325: // Offer Sushi the MP3 recorder.
		turnSushi(441);
		break;
	case 326: // Offer Sushi the bag from the jail.
		turnSushi(443);
		break;
	case 400: // Tell Sushi how Brian arrived in Douglasville with Gina.
		for (uint i = 0; i <= 9; ++i) {
			// After the abbreviated retelling, Brian continues with line seven.
			const byte person = i == 1 || i == 3 || i == 5 || i == 8;
			say(0, i, false, person);
		}
		_finale.sushiVisited = 1;
		deferAction(402);
		break;
	case 402: // Resume work after a scripted exchange.
		returnToComputer();
		break;
	case 404: // Explain Gina's confession and the search for Johnny's trailer.
		exchange(81, 0, 8);
		_finale.sushiTruthTold = 1;
		deferAction(402);
		break;
	case 406: // Explain the bank's fingerprint security and Johnny's twin sister.
		exchange(40, 3, 13);
		deferAction(402);
		break;
	case 408: // Report the arrest and discuss the remaining Sandretti threat.
		exchange(82, 12, 18);
		deferAction(402);
		break;
	case 410: // Explain the plan to impersonate Gustav on the telephone.
		exchange(80, 31, 37);
		deferAction(411);
		break;
	case 411: // Pick up Sushi's recorder from the table.
		deliverItem(412);
		break;
	case 1412: // Start the recording task and remove the recorder from the desk.
		_recorderOnDesk = false;
		_finale.recordingPlan = 1;
		_inventory.add(16);
		say(80, 38);
		return kBackgroundChanged;
	case 420: // Sushi demonstrates her completed voice filter.
		exchange(83, 5, 5);
		deferAction(451);
		break;
	case 421: // Say the test phrase through the microphone.
		say(93, 6, false, 1);
		deferAction(422);
		break;
	case 422: // Turn back to Brian after demonstrating Gustav's voice.
		turnSushi(423);
		break;
	case 423: // Explain the prepared call and Gina's recording.
		say(93, 7, false, 1);
		exchange(83, 8, 8);
		say(83, 9);
		deferAction(452);
		break;
	case 424: // Connect to the Sandrettis using their henchmen's caller ID.
		if (!_phone.load("DATAVH07.001") || !_phonePortraits.load('H', 7)) {
			error("Cannot load Runaway's Sandretti phone call");
		}
		_phoneActive = true;
		_phonePortrait = 0;
		beginPhoneRange(0, 27, 425);
		break;
	case 425: // Tell the boss that Gina and Brian have been caught.
		for (uint i = 10; i <= 12; ++i) {
			say(83, i, false, 2);
			say(93, i, false, 1);
		}
		deferAction(426);
		break;
	case 426: // The boss reacts to the supposed loss of the money.
		_phonePortrait = 1;
		beginPhoneRange(_gog ? 28 : 39, _gog ? 74 : 85, 427);
		break;
	case 427: // Invent the pursuit into the dynamite-filled mine.
		for (uint i = 13; i <= 15; ++i) {
			say(83, i, false, 2);
			say(93, i, false, 1);
		}
		deferAction(428);
		break;
	case 428: // The boss hears that the mine exploded.
		_phonePortrait = 2;
		beginPhoneRange(_gog ? 75 : 97, _gog ? 102 : 125, 429);
		break;
	case 429: // Play Gina's interruption and let the boss suspect a betrayal.
		say(83, 16, false, 2);
		say(93, 16, false, 1);
		say(83, 17, false, 3);
		say(93, 17, false, 2);
		deferAction(431);
		break;
	case 430: // Give Sushi an hour to prepare the recording plan.
		walkTo(Common::Point(520, 410), 7, false, true);
		leaveRoom(8222);
		break;
	case 431: // Disconnect after the boss threatens Gustav.
		beginPhoneRange(_gog ? 103 : 137, _gog ? 126 : 160, 432);
		break;
	case 432: // Return to the room after completing the call.
		_phoneActive = false;
		_phone.close();
		turnSushi(433);
		break;
	case 433: // Reveal the Swiss bank receipt and say farewell.
		say(93, 18, false, 1);
		exchange(83, 19, 25);
		deferAction(434);
		break;
	case 434: // Leave Douglasville after Sushi's final goodbye.
		_finale.departure = 1;
		returnToComputer();
		leaveRoom(8222);
		break;
	case 440: // Ask Sushi to research the bank described in the brochure.
		exchange(40, 0, 0);
		if (!_finale.trailerDiscussed) {
			exchange(80, 27, 27);
			_finale.trailerDiscussed = 1;
		}
		exchange(40, 1, 2);
		deferAction(444);
		break;
	case 441: // Check whether both required recordings are present.
		receiveRecordings();
		break;
	case 442: // Put the completed recordings on Sushi's desk.
		deliverItem(445);
		break;
	case 443: // Ask Sushi to investigate Gustav and Feodor's belongings.
		exchange(60, 0, 2);
		deferAction(446);
		break;
	case 444: // Leave the brochure on the desk for later investigation.
		deliverItem(447);
		break;
	case 1445: // Leave Sushi to edit the voices, then return for the call.
		_inventory.remove(16);
		_finale.voiceEditing = 1;
		walkTo(Common::Point(564, 415), 7, false, true);
		leaveRoom(8222);
		return kBackgroundChanged;
	case 446: // Set the evidence bag down on the desk.
		deliverItem(448);
		break;
	case 1447: // Begin the bank investigation and remove the brochure from inventory.
		_inventory.remove(14);
		_finale.bankResearch = 1;
		return kBackgroundChanged;
	case 1448: // Leave the gangsters' belongings for Sushi to examine.
		_inventory.remove(13);
		_finale.bagDelivered = 1;
		return kBackgroundChanged;
	case 450: // Select the conversation appropriate to the current puzzle stage.
		if (_finale.bankResearch == 3 && _finale.gangsters < 2) {
			if (_finale.nunsHabit < 2) {
				exchange(82, 0, 1);
			} else if (!_finale.gangstersDiscussed) {
				exchange(82, 0, 0);
				exchange(82, 2, 7);
				_finale.gangstersDiscussed = 1;
			} else {
				exchange(82, 8, 10);
				say(82, 11);
			}
			deferAction(402);
		} else if (_inventory.contains(16)) {
			receiveRecordings();
		} else {
			startSushiConversation();
		}
		break;
	case 451: // Turn to the microphone after explaining the voice filter.
		turnSushi(421, true);
		break;
	case 452: // Turn to the microphone after Brian agrees to make the call.
		turnSushi(424, true);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene8380::beginPhoneRange(uint first, uint last, uint16 nextAction) {
	if (!_phone.seekToFrame(first)) {
		error("Cannot seek to Runaway phone frame %u", first);
	}
	_phoneFirst = first;
	_phoneLast = last;
	_phoneStarted = _now;
	_phoneMoving = true;
	deferAction(nextAction);
}

void Scene8380::startSushiConversation() {
	startConversation(5);
	exchange(80, 0, 0);
	static const DialogueChoice topics[] = {
		{1, 0, 3, 1, 1, 2, 255}, {0, 0, 3, 2, 2, 1, 255},
		{1, 0, 3, 3, 3, 1, 255}, {0, 0, 1, 4, 4, 4, 255},
		{0, 0, 3, 27, 27, 3, 255}, {0, 0, 3, 29, 29, 1, 255},
		{0, 0, 1, 30, 30, 0, 255}, {1, 0, 0, 5, 5, 0, 0}
	};
	memcpy(_dialogue[0], topics, sizeof(topics));
	_dialogue[0][0].enabled = !_finale.hackerKnown;
	_dialogue[0][1].enabled = _finale.hackerKnown;
	_dialogue[0][3].enabled = _finale.bagDelivered && !_finale.movieFanKnown;
	if (_finale.bagDelivered == 1) {
		_dialogue[0][3].response = 29;
		_dialogue[0][3].transition = 3;
		_dialogue[0][3].disable = 1;
	}
	_dialogue[0][4].enabled = !_finale.trailerDiscussed && _finale.trailerFound == 2;
	_dialogue[0][5].enabled = _finale.bankResearch == 1;
	_dialogue[0][6].enabled = _finale.movieFanKnown && !_finale.recordingPlan;
	// Flatten the original five topic depths into the shared dialogue's pages.
	for (uint i = 0; i < 6; ++i) {
		_dialogue[10][i] = {1, 0, 2, (byte)(13 + i), (byte)(13 + i), 0, 0};
		_dialogue[20][i] = {1, 0, 4, (byte)(19 + i), (byte)(19 + i), 0, 0};
	}
	_dialogue[10][4].transition = 1; // Woody Allen leads to the film choices.
	_dialogue[20][2].transition = 3;
	_dialogue[20][2].disable = 10; // Manhattan Murder Mystery inspires Sushi.
	_dialogue[21][0] = {1, 0, 0, 25, 25, 11, 255};
}

void Scene8380::handleCustomDialogueChoice(byte effect) {
	switch (effect) {
	case 2: // Learn that Sushi is the hacker Janie Guitar.
		exchange(80, 6, 10);
		_finale.hackerKnown = 1;
		break;
	case 3: // Tell Sushi that Brian found Johnny's trailer.
		exchange(80, 28, 28);
		_finale.trailerDiscussed = 1;
		break;
	case 4: // Learn that Gustav loves movies, then discuss favorite directors.
		exchange(80, 11, 12);
		_finale.movieFanKnown = 1;
		_dialogue[0][6].enabled = 1;
		break;
	case 10: // Ask Sushi to explain the idea inspired by the film.
		_dialogueDepth = 2;
		_dialogueNode = 1;
		break;
	case 11: // Agree to come back after Sushi organizes the plan.
		exchange(80, 26, 26);
		_finale.voicePlanPending = 1;
		_afterConversation = 430;
		break;
	default:
		break;
	}
}

void Scene8380::finishCustomConversation() {
	returnToComputer();
}

} // End of namespace Runaway
} // End of namespace Hollywood
