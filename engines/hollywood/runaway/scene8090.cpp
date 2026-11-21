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

#include "common/textconsole.h"
#include "common/util.h"
#include "hollywood/runaway/presentation.h"
#include "hollywood/runaway/scene8090.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{910, 599, 3, {301, 302}, {0, 3}, {1023, 599}, 6}, // 1: Oil well area.
	{696, 510, 0, {303, 304}, {0, 0}, {668, 390}, 4}, // 2: Entrance to the house.
	{846, 509, 0, {305, 306}, {0, 0}, {837, 455}}, // 3: Sign.
	{676, 510, 7, {307, 308}, {0, 1}, {517, 463}, 3}, // 4: Oscar.
	{244, 416, 7, {309, 310}, {0, 0}, {216, 394}}, // 5: Ladder.
	{696, 510, 0, {312, 313}, {0, 1}, {668, 390}, 5}, // 6: Entrance to the house.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{50, 599, 5, {326, 327}, {0, 3}, {0, 599}, 7} // 8: Path.
};

const RunawayResponse kResponses[] = {
	{8090, 301, 13, 0}, // Look toward the oil well.
	{8090, 303, 2, 0},  // Examine the doorway.
	{8090, 305, 4, 0},  // Read Mama Dorita's sign.
	{8090, 309, 7, 0},  // Examine the ladder.
	{8090, 326, 1, 0}   // Look along the path.
};

const RunawayItemAction kItemActions[] = {
	{8090, 12, 4, 325} // Offer Oscar the sheriff's badge.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8090 = {
	8090, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {100, 95, 90}, {80, 75, 70}}, Common::Point(838, 551),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene8090::Scene8090(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game), _oscar(true, 6) {}

const char *Scene8090::actionCaptionVariant(byte id, bool look, byte item) const {
	if (!look && !item && (id == 2 || id == 6) && _finale.oscarAway) {
		return "oscar_away";
	}
	return nullptr;
}

bool Scene8090::loadCustomResources() {
	if (_entryScene == 8092 && !_carryingGina.load(_archive, 10, 7)) {
		return false;
	}
	return _oscarAnimation.load(_archive, 7, 113) && loadForeground(0, 4);
}

bool Scene8090::applyCustomBackgroundPatches() {
	return applyPatch(_curtainOpen ? 5 : 6);
}

void Scene8090::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (id == 7) {
			id = _finale.oscarAway ? 2 : 4;
		} else if (id == 4 && _finale.oscarAway) {
			id = 0;
		} else if (id == 6 && !_finale.oscarAway) {
			id = 2;
		}
	}
}

void Scene8090::prepareCustomGameplayLoop() {
	_oscar.resume(_now);
}

void Scene8090::runCustomEntrySequence() {
	switch (_entryScene) {
	case 8091: // Come back out after speaking with Mama Dorita or visiting Gina.
		_position = Common::Point(696, 510);
		_walkPath.reset(_position);
		_visitingGina = _finale.ginaVisitPending;
		_finale.ginaVisitPending = 0;
		returnFromHouse();
		break;
	case 8092: // Bring the injured Gina to Mama Dorita after leaving the village.
		_arrivalPhase = kArrivalWait;
		_arrivalStarted = _now;
		break;
	case 8093: // Oscar returns armed, ready to confront the gangsters.
		_position = Common::Point(676, 510);
		_walkPath.reset(_position);
		_facing = 7;
		_animatesOscar = true;
		beginRoomAnimationRange(430, 14, 53, 0, 13, false);
		break;
	case 8094: // Follow Oscar's plan back to Sushi.
		_position = Common::Point(676, 510);
		_walkPath.reset(_position);
		_facing = 3;
		say(20, 23);
		deferAction(434);
		break;
	default:
		walkTo(Common::Point(837, 550), 7);
		break;
	}
}

void Scene8090::resumeAmbientAudio(bool entering) {
	if (_entryScene == 8092) {
		// This arrival still uses chapter five's voices and effects.
		_audio.setChapter(5);
		_audio.stopMusic();
		_audio.stopEffect();
		return;
	}
	resumeFinaleOutdoorAudio(entering, 40, 26);
}

void Scene8090::advanceCustomGameplayLoop(uint32 delta) {
	if (_entryScene == 8092) {
		return;
	}
	updateFinaleOutdoorAudio(102);
	if (!_finale.oscarAway && !_animatesOscar && !_oscarPose) {
		_oscar.update(_now, _random, _audio);
	}
}

bool Scene8090::updateCustomSequence() {
	switch (_arrivalPhase) {
	case kArrivalWait:
		if (_now - _arrivalStarted >= 2500) {
			say(0, 20);
			say(0, 21, false, 1);
			_arrivalPhase = kArrivalSpeech;
		}
		return true;
	case kArrivalSpeech: {
		if (speaking()) {
			return true;
		}
		byte frames[34] = {};
		for (uint i = 1; i < ARRAYSIZE(frames); ++i) {
			frames[i] = i - 1;
		}
		_curtainOpen = true;
		if (!rebuildBackground()) {
			error("Cannot restore Runaway house entrance");
		}
		beginRoomAnimation(440, 11, 33, frames, ARRAYSIZE(frames), true, 80);
		_arrivalPhase = kArrivalCurtain;
		break;
	}
	case kArrivalFadeOut:
		if (_now - _arrivalStarted < 500) {
			return true;
		}
		leaveRoom(8100);
		break;
	default:
		break;
	}
	return false;
}

bool Scene8090::drawCustomComposite(Graphics::ManagedSurface &viewport) {
	if (_entryScene != 8092) {
		return false;
	}
	if (drawingRoomAnimation()) {
		_actionAnimation.draw(viewport, _actionFrames[_actionStep], 0, 0);
	} else if (_arrivalPhase == kArrivalFadeOut) {
		_actionAnimation.draw(viewport, 32, 0, 0);
	} else {
		_oscarAnimation.draw(viewport, 107 + (speaker() ? speechFrame() % 6 : 0), 0, 0);
	}
	_carryingGina.draw(viewport, speaking() && !speaker() ? speechFrame() % 6 : 0, 0, 0);
	if (_arrivalPhase == kArrivalWait) {
		dimRunawayViewport(_screen, MIN<uint>(500, _now - _arrivalStarted), 500);
	} else if (_arrivalPhase == kArrivalFadeOut) {
		dimRunawayViewport(_screen, 500 - MIN<uint>(500, _now - _arrivalStarted), 500);
	}
	return true;
}

void Scene8090::handleAnimationFrameHook() {
	if (_roomAction == 402 && _actionStep == 9) {
		_audio.playFootstep(1, _random.getRandomBit(), 230, 696);
	} else if (_roomAction == 403 && (_actionStep == 5 || _actionStep == 9)) {
		_audio.playFootstep(2, _random.getRandomBit(), 230, 696);
	}
	if (_roomAction == 440 && _actionStep == 14) {
		_audio.playEffect(27, 230);
	}
}

void Scene8090::skipVideo() {
	if (_entryScene == 8092) {
		_speech.stop();
		_audio.stopEffect();
		_arrivalPhase = kNoArrival;
		_roomAction = 0;
		_actionFrames.clear();
		_transition.scene = 8100;
	} else {
		RunawayRoomScene::skipVideo();
	}
}

void Scene8090::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	drawOscar(viewport, false);
}

void Scene8090::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	drawOscar(viewport, true);
	drawForeground(viewport, 0);
}

bool Scene8090::roomAnimationInForeground() const {
	return _position.y < 460;
}

void Scene8090::drawOscar(Graphics::ManagedSurface &viewport, bool foreground) {
	if (_finale.oscarAway || (_position.y < 460) != foreground || (_animatesOscar && drawingRoomAnimation())) {
		return;
	}
	if (_oscarPose) {
		const uint frame = speaker() && speechFrame() ? _oscarMouth + speechFrame() % 6 : _oscarPose - 1;
		_oscarPoseAnimation.draw(viewport, frame, _camera.x, -_camera.y);
	} else {
		_oscarAnimation.draw(viewport, _oscar.frame(speaker(), speechFrame()), _camera.x, -_camera.y);
	}
}

float Scene8090::actorHeight() const {
	return _position.y < 510 ? _position.y * 0.840707958f - 38.7610626f :
		_position.y * 0.730337083f + 17.5280895f;
}

byte Scene8090::initialActorFacing() const {
	return 7;
}

void Scene8090::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

int8 Scene8090::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (id == 2 && use && !item && _finale.oscarAway) {
		return true;
	}
	return approach;
}

RunawayRoomScene::ActionResult Scene8090::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Return to the oil well.
		leaveRoom(8131);
		break;
	case 304: // Ask permission to enter, or visit Gina while Oscar is away.
		if (_finale.oscarAway) {
			enterHouse(true);
		} else {
			_facing = 4;
			say(3, 1);
		}
		break;
	case 306: // Leave Mama Dorita's sign alone.
		say(5, _finale.oscarAway);
		break;
	case 307: { // Look at Oscar.
		const byte variation = _random.getRandomNumber(2);
		if (variation == 2) {
			_facing = 4;
		}
		say(6, variation);
		break;
	}
	case 308: // Talk to Oscar.
		startOscarConversation();
		break;
	case 310: // Refuse to climb the ladder.
		say(8, _finale.oscarAway);
		break;
	case 312: // The second doorway shares the entrance description.
		break;
	case 313: // Go inside to speak with Mama Dorita.
		enterHouse(false);
		break;
	case 320: // Visit Gina.
		enterHouse(true);
		break;
	case 325: // Recruit Oscar as Douglasville's sheriff.
		if (_finale.nunsHabit < 2) {
			_facing = 4;
			say(20, 24);
			break;
		}
		_inventory.remove(12);
		_inventory.select(0);
		_oscar.setTalking(true);
		_audio.stopEffect(1);
		for (uint i = 0; i <= 4; ++i) {
			say(20, i, false, i % 2);
		}
		deferAction(420);
		break;
	case 327: // Return to the town overview.
		leaveRoom(8350);
		break;
	case 400: // Oscar resumes chewing after the conversation.
		_oscar.startIdle();
		break;
	case 401: // Oscar opens the curtain for Brian.
		_animatesOscar = true;
		_audio.stopEffect(1);
		beginRoomAnimationRange(401, 11, 33, 0, 32, false);
		break;
	case 1401: // Keep the curtain open while Brian goes inside.
		_curtainOpen = true;
		setOscarPose(11, 33, 32);
		_animatesOscar = false;
		deferAction(402);
		return kBackgroundChanged;
	case 402: // Brian steps through the doorway.
		beginRoomAnimationRange(402, 8, 15, 0, 14);
		break;
	case 1402: // Switch to the interior, or return after a brief visit to Gina.
		if (_houseDestination) {
			leaveRoom(_houseDestination);
		} else {
			returnFromHouse();
		}
		break;
	case 1403: // Close the curtain and restore the outdoor view.
		_curtainOpen = false;
		_animatesOscar = false;
		_oscarPose = 0;
		_oscar.startIdle();
		_position = Common::Point(696, 510);
		_walkPath.reset(_position);
		_facing = 4;
		if (_visitingGina) {
			describeGinaVisit();
			_visitingGina = false;
		}
		return kBackgroundChanged;
	case 420: // Brian hands Oscar the badge.
		beginRoomAnimationRange(420, 12, 36, 0, 35);
		break;
	case 1420: // Oscar accepts the badge and raises his right hand.
		_animatesOscar = true;
		beginRoomAnimationRange(421, 13, 100, 0, 60, false);
		break;
	case 1421: // Swear Oscar in as sheriff.
		_animatesOscar = false;
		setOscarPose(13, 100, 60);
		_oscarMouth = 60;
		for (uint i = 5; i <= 11; ++i) {
			say(20, i, false, i == 5 || i == 8 || i == 10);
		}
		deferAction(422);
		break;
	case 422: // Oscar lowers his hand after taking the oath.
		_animatesOscar = true;
		beginRoomAnimationRange(422, 13, 100, 67, 94, false);
		break;
	case 1422: // Explain how the new sheriff can deal with the gangsters.
		_animatesOscar = false;
		setOscarPose(13, 100, 0);
		_oscarMouth = 94;
		for (uint i = 12; i <= 19; ++i) {
			say(20, i, false, i <= 16 && !(i % 2));
		}
		deferAction(423);
		break;
	case 423: // Return to Sushi to continue the plan.
		_oscarPose = 0;
		_oscar.startIdle();
		leaveRoom(8373);
		break;
	case 1430: // Oscar shows Brian his rifle.
		_animatesOscar = false;
		setOscarPose(14, 53, 13);
		_oscarMouth = 13;
		say(20, 20, false, 1);
		say(20, 21);
		say(20, 22, false, 1);
		deferAction(431);
		break;
	case 431: // Oscar leaves to deal with the gangsters.
		_animatesOscar = true;
		beginRoomAnimationRange(431, 14, 53, 20, 52, false);
		break;
	case 1431: // Follow Oscar toward the oil well.
		_animatesOscar = false;
		_oscarPose = 0;
		_finale.gangsters = 2;
		leaveRoom(8132);
		break;
	case 434: // Meet Sushi after Oscar's instructions.
		leaveRoom(8374);
		break;
	case 1440: // Fade out once Oscar opens the curtain for Gina.
		_arrivalPhase = kArrivalFadeOut;
		_arrivalStarted = _now;
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

uint16 Scene8090::dialogueCue(byte choice, bool response) const {
	// Oscar's personal history uses the earlier conversation's text page.
	return (choice == 2 ? 81 : 82) + (response ? 10 : 0);
}

void Scene8090::handleCustomDialogueChoice(byte effect) {
	if (effect == 2) { // Ask to see Mama Dorita.
		_afterConversation = 313;
	} else if (effect == 3) { // Ask to see Gina.
		_afterConversation = 320;
	}
}

void Scene8090::startOscarConversation() {
	startConversation(1);
	_afterConversation = 400;
	_oscar.setTalking(true);
	_audio.stopEffect(1);
	say(82, 0);
	say(92, 0, false, 1);
	static const DialogueChoice topics[] = {
		{1, 0, 0, 1, 1, 2, 255}, {1, 0, 0, 2, 2, 3, 255},
		{1, 0, 3, 3, 3, 5, 255}, {0, 0, 3, 3, 3, 1, 255},
		{1, 0, 0, 4, 4, 4, 0}
	};
	memcpy(_dialogue[0], topics, sizeof(topics));
	_dialogue[0][3].enabled = _finale.mediumTests >= 3 && !_finale.trailerFound;
}

void Scene8090::enterHouse(bool visitGina) {
	_oscar.startIdle();
	_visitingGina = visitGina;
	_houseDestination = 8100;
	if (visitGina) {
		if (_finale.mediumTests < 3) {
			_houseDestination = 8420;
		} else if (!_finale.recordingPlan) {
			_houseDestination = 0;
		} else if (_finale.recordings == 0 || _finale.recordings == 2) {
			_houseDestination = 8422;
		} else {
			_facing = 4;
			say(41, _finale.recordings == 3);
			return;
		}
	}
	if (_houseDestination && !findRunawayRoom(_houseDestination)) {
		leaveRoom(_houseDestination);
		return;
	}
	walkTo(Common::Point(696, 510), 0);
	deferAction(_finale.oscarAway ? 402 : 401);
}

void Scene8090::returnFromHouse() {
	_curtainOpen = true;
	if (!rebuildBackground()) {
		error("Cannot restore Runaway house entrance");
	}
	_oscarPose = 0;
	_audio.stopEffect(1);
	_animatesOscar = !_finale.oscarAway;
	if (_finale.oscarAway) {
		beginRoomAnimationRange(403, 9, 16, 0, 15);
	} else {
		byte brian[33];
		for (uint i = 0; i < ARRAYSIZE(brian); ++i) {
			brian[i] = MIN<uint>(15, i);
		}
		beginRoomAnimationRange(403, 11, 33, 32, 0);
		setRoomAnimationOverlay(9, 16, brian, ARRAYSIZE(brian));
	}
}

void Scene8090::describeGinaVisit() {
	if (_finale.recordingPlan) {
		say(40, _finale.recordings < 2 ? 4 : 5);
		return;
	}
	byte variation = 0, bit = 0;
	if (_finale.moneyFound >= 3) {
		variation = _finale.nunsHabit >= 2 ? 2 : 1;
		bit = 1 << (variation - 1);
	} else if (_finale.trailerFound) {
		variation = 3;
		bit = 4;
	}
	if (_finale.ginaUpdates & bit) {
		variation = 0;
	}
	_finale.ginaUpdates |= bit;
	say(40, variation);
}

void Scene8090::setOscarPose(uint chunk, uint frames, byte pose) {
	if (!_oscarPoseAnimation.load(_archive, chunk, frames)) {
		error("Cannot load Runaway Oscar pose %u", chunk);
	}
	_oscarPose = pose + 1;
	_oscarMouth = pose;
}

} // End of namespace Runaway
} // End of namespace Hollywood
