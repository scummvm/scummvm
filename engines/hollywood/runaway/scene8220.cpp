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
#include "hollywood/runaway/scene8220.h"
#include "hollywood/runaway/scene_registry.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{188, 599, 5, {301, 302}, {0, 3}, {188, 599}, 7}, // 1: Path.
	{778, 512, 3, {303, 304}, {0, 1}, {778, 512}, 6}, // 2: Saloon.
	{125, 441, 7, {305, 306}, {0, 1}, {79, 399}, 4}, // 3: Hotel.
	{388, 284, 7, {307, 308}, {0, 1}, {388, 284}, 4}, // 4: Sheriff's office.
	{574, 242, 7, {309, 310}, {0, 1}, {526, 216}, 4}, // 5: Liquor store.
	{647, 198, 0, {311, 312}, {0, 3}, {646, 195}, 4}, // 6: Bank.
	{722, 192, 0, {313, 314}, {0, 1}, {722, 192}, 5}, // 7: Locomotive.
	{293, 350, 7, {315, 316}, {1, 0}, {285, 340}}, // 8: Wagon.
	{450, 345, 7, {318, 319}, {0, 1}, {404, 302}, 3}, // 9: Oscar.
	{456, 271, 1, {320, 321}, {0, 0}, {472, 254}}, // 10: Car.
	{722, 192, 0, {322, 323}, {0, 3}, {722, 192}, 5} // 11: North side of town.
};

const RunawayResponse kResponses[] = {
	{8220, 301, 1, 0},  // Examine the railroad tracks.
	{8220, 303, 2, 0},  // Examine the saloon.
	{8220, 305, 3, 0},  // Examine Sushi's hotel.
	{8220, 307, 4, 0},  // Examine the sheriff's office.
	{8220, 309, 5, 0},  // Examine the boarded-up liquor store.
	{8220, 310, 6, 0},  // Try to enter the liquor store.
	{8220, 311, 7, 0},  // Examine the ruined bank.
	{8220, 313, 8, 0},  // Examine the derailed locomotive.
	{8220, 315, 9, 2},  // Examine the wagon after cutting its strap.
	{8220, 316, 10, 1}, // Refuse to take the wagon.
	{8220, 318, 13, 0}, // Examine Oscar, the new sheriff.
	{8220, 320, 15, 0}, // Consider using the gangsters' car.
	{8220, 321, 16, 0}, // It is not time to leave yet.
	{8220, 322, 1, 1}   // Look toward the cemetery.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8220 = {
	8220, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {95, 90, 70}, {75, 70, 50}, {55, 50, 30}}, Common::Point(188, 599),
	kResponses, ARRAYSIZE(kResponses), nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene8220::Scene8220(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game), _oscar(false, 6) {}

bool Scene8220::loadCustomResources() {
	return _oscarAnimation.load(_archive, 7, 107) && _brianAnimation.load(_archive, 8, 33) &&
		_carAnimation.load(_archive, 9, 161);
}

void Scene8220::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		// The hotel balcony has a separate entrance and walk map.
		if (_maps[i] < 2 || _maps[i] > 4) {
			_maps[i] = 0;
		}
		byte &id = _maps[256 + i];
		if ((id == 9 || id == 10) && !_finale.oscarAway) {
			id = 4;
		}
	}
}

void Scene8220::prepareCustomGameplayLoop() {
	_oscar.resume(_now);
	_carTick = _brianTick = _now;
}

void Scene8220::resumeAmbientAudio(bool entering) {
	resumeFinaleOutdoorAudio(false, 36, 51);
	if (entering && _entryScene == 8220 && (!_finale.streetVisited || !_random.getRandomNumber(5))) {
		_musicCue = 20;
		_audio.playMusic(_musicCue, false);
		_finale.streetVisited = 1;
	} else if (entering && _entryScene == 8223) {
		_musicCue = 12 + _random.getRandomNumber(4);
		_audio.playMusic(_musicCue, false);
	}
	_audio.setMusicVolume(255);
}

void Scene8220::advanceCustomGameplayLoop(uint32 delta) {
	if (!_departing) {
		updateFinaleOutdoorAudio();
		_audio.setMusicVolume(255);
	}
	if (_finale.oscarAway) {
		_oscar.update(_now, _random, _audio);
	}
}

void Scene8220::runCustomEntrySequence() {
	struct Entry {
		int16 x, y, targetX, targetY;
		byte facing;
	};
	static const Entry entries[] = {
		{188, 599, 275, 530, 1}, // Arrive from the town overview.
		{778, 512, 686, 474, 7}, // Leave the saloon.
		{147, 460, 209, 527, 3}, // Leave the hotel.
		{420, 297, 470, 330, 3}, // Leave the sheriff's office.
		{647, 197, 652, 209, 3}, // Leave the bank.
		{718, 194, 718, 194, 5}  // Leave the locomotive.
	};

	if (_entryScene == 8229) {
		// Oscar arrives in the gangsters' car while Brian watches from the street.
		_finale.oscarAway = 0;
		updateMaps();
		_arrivalPhase = kWaitingForCar;
		_roomAction = 420;
		_musicCue = 38;
		_audio.playMusic(_musicCue, false);
		_audio.setMusicVolume(255);
		_audio.playEffect(67, 178);
		return;
	}

	const Entry &entry = entries[_entryScene - 8220];
	_position = Common::Point(entry.x, entry.y);
	_facing = entry.facing;
	_walkPath.reset(_position);
	if (_entryScene == 8222) {
		enterFromHotel();
		return;
	}
	walkTo(Common::Point(entry.targetX, entry.targetY), entry.facing, true);
	if (_entryScene == 8220 && _finale.seanceComplete && !_finale.sushiVisited) {
		deferAction(403);
	}
}

void Scene8220::enterFromHotel() {
	if (_finale.departure == 1) {
		if (findRunawayRoom(9020)) {
			_finale.departure = 2;
			_departing = true;
			_audio.stopMusic();
			_audio.stopEffect();
			_position = Common::Point(209, 527);
			_walkPath.reset(_position);
			walkTo(Common::Point(450, 345), 7, false, true);
			deferAction(410);
			return;
		}
		leaveRoom(9020);
	} else if (_finale.voiceEditing == 1 || _finale.voicePlanPending) {
		const uint16 destination = _finale.voiceEditing == 1 ? 8383 : 8382;
		if (findRunawayRoom(destination)) {
			if (_finale.voiceEditing == 1) {
				_finale.voiceEditing = 2;
			} else {
				_finale.voicePlanPending = 0;
			}
			_position = Common::Point(209, 527);
			_walkPath.reset(_position);
			say(0, 20);
			walkTo(Common::Point(125, 441), 7, false, true);
			leaveRoom(destination);
			return;
		}
		leaveRoom(destination);
	}

	if (_finale.oscarAway && !_finale.sheriffThanked) {
		walkTo(Common::Point(450, 345), 7, false, true);
		deferAction(319);
	} else {
		walkTo(Common::Point(209, 527), 3, true);
	}
}

bool Scene8220::updateCustomSequence() {
	if (_arrivalPhase != kNoArrival) {
		updateCarArrival();
		return _arrivalPhase != kNoArrival;
	}
	if (_waitingForOscar) {
		if (_oscar.frame(false, 0) != 105) {
			return true;
		}
		_waitingForOscar = false;
		_oscarTalking = true;
		_oscar.setTalking(true);
	}
	return false;
}

void Scene8220::updateCarArrival() {
	if (_arrivalPhase == kWaitingForCar) {
		if (_now - _carTick >= 2000) {
			_carTick = _now;
			_arrivalPhase = kWatchingCar;
			say(12);
		}
		return;
	}

	while (_carFrame < 58 && _now - _carTick >= 80) {
		_carTick += 80;
		++_carFrame;
	}
	if (_arrivalPhase == kWatchingCar && !speaking()) {
		_arrivalPhase = kTurningToCar;
		_brianTick = _now;
	}
	if (_arrivalPhase == kTurningToCar) {
		while (_brianStep < 21 && _now - _brianTick >= 60) {
			_brianTick += 60;
			++_brianStep;
		}
		if (_brianStep == 21) {
			_arrivalPhase = kCommentingOnCar;
			say(12, 1);
		}
	} else if (_arrivalPhase == kCommentingOnCar && _carFrame == 58 && !speaking()) {
		_arrivalPhase = kNoArrival;
		_finale.oscarAway = 1;
		_finale.gangsterBag = 1;
		_position = Common::Point(209, 527);
		_facing = 3;
		_walkPath.reset(_position);
		updateMaps();
		_oscar.startIdle();
	}
}

void Scene8220::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (_arrivalPhase != kNoArrival) {
		_carAnimation.draw(viewport, _carFrame, _camera.x, -_camera.y);
		uint frame;
		if (_arrivalPhase == kWaitingForCar || _arrivalPhase == kWatchingCar) {
			frame = speaking() ? speechFrame() : 0;
		} else {
			frame = 12 + (_brianStep ? _brianStep - 1 : 0);
		}
		_brianAnimation.draw(viewport, frame, _camera.x, -_camera.y);
	} else if (_finale.oscarAway) {
		// This wide view has no separate talking frames for Oscar.
		_oscarAnimation.draw(viewport, _oscarTalking ? 105 : _oscar.frame(false, 0), _camera.x, -_camera.y);
		if (!drawingRoomAnimation()) {
			_carAnimation.draw(viewport, 58, _camera.x, -_camera.y);
		}
	}
}

bool Scene8220::drawCustomComposite(Graphics::ManagedSurface &viewport) {
	if (!_farewell) {
		return false;
	}
	viewport.clear();
	return true;
}

bool Scene8220::shouldDrawActor() const {
	return _arrivalPhase == kNoArrival;
}

float Scene8220::actorHeight() const {
	return _position.y * 0.194835678f - 11.706573f;
}

void Scene8220::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 263;
	params.depthY[2] = 195;
	params.depthY[3] = 490;
	params.depthY[4] = 396;
	params.depthY[5] = 296;
	params.depthY[6] = 247;
	params.depthY[7] = 599;
	params.depthY[8] = 322;
	params.depthY[9] = 599;
}

void Scene8220::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

bool Scene8220::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = CLIP<int>(target.x, 114, 722);
	return false;
}

void Scene8220::leaveStreet(uint16 destination, bool advanceResearch) {
	if (!findRunawayRoom(destination)) {
		leaveRoom(destination);
		return;
	}
	if (advanceResearch && _finale.bankResearch == 1) {
		_finale.bankResearch = 2;
	}
	if (destination == 8350 && _musicCue == 20) {
		_musicCue = 12 + _random.getRandomNumber(4);
		_audio.playMusic(_musicCue, false);
		_audio.setMusicVolume(255);
	}
	leaveRoom(destination);
}

RunawayRoomScene::ActionResult Scene8220::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Leave town by the railroad tracks.
	case 323: // Leave town toward the cemetery.
		leaveStreet(8350, false);
		break;
	case 304: // Enter the saloon while Sushi continues her research.
		leaveStreet(8180, true);
		break;
	case 306: // Enter Sushi's hotel.
		leaveStreet(8370, false);
		break;
	case 308: // Enter the sheriff's office.
		leaveStreet(8200, true);
		break;
	case 312: // Enter the ruined bank.
		leaveStreet(8190, true);
		break;
	case 314: // Approach the locomotive.
		leaveStreet(8260, true);
		break;
	case 319: // Congratulate the new sheriff, or ask whether all is well.
		say(14, _finale.sheriffThanked ? 7 : 0);
		deferAction(400);
		break;
	case 400: // Let Oscar finish his idle gesture before answering.
		_waitingForOscar = true;
		deferAction(401);
		break;
	case 401: // Oscar describes the arrest and asks to be called sheriff.
		if (_finale.sheriffThanked) {
			say(14, 8, false, 1);
		} else {
			for (uint i = 1; i <= 6; ++i) {
				say(14, i, false, i % 2);
			}
		}
		deferAction(402);
		break;
	case 402: // Finish the conversation and resume Oscar's idle animation.
		_finale.sheriffThanked = 1;
		_oscarTalking = false;
		_oscar.startIdle();
		break;
	case 403: // Visit Sushi after the séance reveals the gangsters' plan.
		if (!findRunawayRoom(8380)) {
			leaveRoom(8380);
			break;
		}
		say(0, 10);
		deferAction(404);
		break;
	case 404: // Walk into the hotel to confide in Sushi.
		walkTo(Common::Point(125, 441), 7, false, true);
		leaveRoom(8380);
		break;
	case 410: // Ask Oscar for the prisoners' car before leaving Douglasville.
		_audio.playMusic(29, false);
		_audio.setMusicVolume(255);
		say(14, 9);
		deferAction(411);
		break;
	case 411: // Oscar stops his idle gesture to say goodbye.
		_waitingForOscar = true;
		deferAction(412);
		break;
	case 412: // Tell Oscar to follow Sushi's orders and say farewell.
		for (uint i = 10; i <= 14; ++i) {
			say(14, i, false, !(i % 2));
		}
		deferAction(413);
		break;
	case 413: // Walk over to the car after saying goodbye.
		_oscarTalking = false;
		_oscar.startIdle();
		walkTo(Common::Point(456, 271), 1, false, true);
		deferAction(414);
		break;
	case 414: { // Drive away while Brian's friends come out to wave.
		byte car[174], friends[174];
		for (uint i = 0; i < ARRAYSIZE(car); ++i) {
			car[i] = i < 30 ? 58 : i < 132 ? i + 29 : 0;
			friends[i] = i < 69 ? 0 : i - 68;
		}
		_audio.playEffect(69, 178);
		beginRoomAnimation(414, 9, 161, car, ARRAYSIZE(car));
		setRoomAnimationOverlay(10, 106, friends, ARRAYSIZE(friends));
		break;
	}
	case 1414: { // Show Sushi, Saturn and Rutger watching Brian leave.
		typedef RunawaySequence S;
		const bool gog = Common::File::exists("DATAVC00.000");
		_filmCommands.clear();
		_filmCommands.push_back({S::kOpen, 9});
		_filmCommands.push_back({S::kPlay, 0, 45});
		for (uint i = 0; i < 3; ++i) {
			// GOG replaces the CD's portrait ranges with three still frames.
			if (gog) {
				_filmCommands.push_back(S::Command(S::kStill, 45 + i));
			}
			_filmCommands.push_back(S::Command(S::kTalk, 40, i, 1 + i * 2));
		}
		_filmCommands.push_back(S::Command(S::kPlay, gog ? 48 : 63, gog ? 105 : 120));
		_filmCommands.push_back(S::Command(S::kEnd));
		_sequence.start(_filmCommands.data(), _text, _now, 'H');
		break;
	}
	case 415: // Continue to the final sequence after Rutger's last remark.
		_audio.stopMusic();
		_audio.stopEffect();
		leaveRoom(9020);
		break;
	case 420: // Oscar has parked and locked up the prisoners; return to Sushi.
		leaveRoom(8375);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene8220::finishCustomFilm() {
	_farewell = true;
	say(40, 3, false, 1);
	deferAction(415);
}

} // End of namespace Runaway
} // End of namespace Hollywood
