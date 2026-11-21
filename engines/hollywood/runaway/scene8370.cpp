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
#include "hollywood/runaway/scene8370.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{1443, 648, 1, {301, 302}, {0, 3}, {1374, 626}, 5}, // 1: Street.
	{775, 466, 0, {303, 304}, {0, -1}, {1006, 656}, 4}, // 2: Stairs.
	{0, 0, 0, {305, 306}, {0, 0}, {828, 695}}, // 3: Front desk.
	{423, 664, 0, {307, 308}, {0, 0}, {424, 653}}, // 4: Fireplace.
	{0, 0, 0, {309, 310}, {0, 0}, {424, 653}}, // 5: Rifles.
	{467, 678, 1, {311, 312}, {0, 1}, {504, 645}}, // 6: Poker.
	{757, 716, 0, {313, 314}, {0, 1}, {757, 716}}, // 7: Bell.
	{920, 345, 7, {315, 316}, {0, 1}, {0, 0}, 4}, // 8: Sushi's work area.
	{1124, 348, 1, {317, 318}, {0, 1}, {1138, 336}}, // 9: Rifle.
	{0, 0, 0, {319, 320}, {0, -1}, {965, 616}, 6}, // 10: Downstairs.
	{828, 710, 0, {321, 322}, {1, 0}, {828, 695}} // 11: Check-in book.
};

const RunawayResponse kResponses[] = {
	{8370, 301, 1, 0},   // Look out toward the saloon.
	{8370, 303, 2, 0},   // Examine the damaged staircase.
	{8370, 305, 3, 0},   // Look at the front desk.
	{8370, 306, 4, 0},   // Decline to become the concierge.
	{8370, 307, 5, 0},   // Notice the lit fireplace.
	{8370, 311, 9, 0},   // Examine the poker.
	{8370, 315, 13, 0},  // Watch Sushi at her computer.
	{8370, 318, 15, 0},  // Leave the rifle until Oscar needs it.
	{8370, 319, 16, 0},  // Notice the precarious upper floor.
	{8370, 321, 17, 0},  // Read the check-in book.
	{8370, 322, 18, 0},  // Leave the book on the desk.
	{8370, 324, 20, 0}   // The poker is not needed to tend the fire.
};

const RunawayItemAction kItemActions[] = {
	{8370, 19, 4, 323}, {8370, 21, 4, 324, false}
};

const byte kSushiFrames[] = {
	0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
	46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
	65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 55, 55, 55, 55,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 93, 93, 93, 93, 93, 93,
	92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8370 = {
	8370, 1444, 800, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {80, 80, 80}, {70, 70, 70}, {50, 50, 50}, {60, 60, 60}, {50, 50, 50}, {40, 40, 40}},
	Common::Point(1443, 648), kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene8370::Scene8370(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	Scene6370(audio, inventory, strings, game) {}

int Scene8370::sceneViewportYOffset() const {
	// Each floor has a fixed view, including when Brian enters on the stairs.
	return _upperFloor ? 0 : 200;
}

bool Scene8370::loadCustomResources() {
	return _sushi.load(_archive, 4, 94) && _fire.load(_archive, 5, 20) && _monitor.load(_archive, 6, 97);
}

bool Scene8370::applyCustomBackgroundPatches() {
	return (_finale.rifle != 2 || applyPatch(8)) && (!_finale.pokerTaken || applyPatch(9));
}

void Scene8370::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		const byte zone = _maps[i];
		byte &id = _maps[256 + i];
		if (_upperFloor) {
			if (zone < 11 || zone > 12) {
				_maps[i] = 0;
			}
			if ((id && id < 7) || id == 11) {
				id = 10;
			}
		} else {
			if (zone == 1 || zone == 4 || zone > 7) {
				_maps[i] = 0;
			}
			if (id >= 8 && id <= 10) {
				id = 0;
			}
		}
		if ((id == 9 && _finale.rifle == 2) || (id == 6 && _finale.pokerTaken)) {
			id = 0;
		}
	}
}

void Scene8370::prepareCustomGameplayLoop() {
	_sushiTick = _now;
}

void Scene8370::resumeAmbientAudio(bool entering) {
	_audio.playEffect(39, 77, 2, true);
	if (entering || !_audio.musicPlaying()) {
		_audio.playMusic(12 + _random.getRandomNumber(4), false);
	}
}

void Scene8370::runCustomEntrySequence() {
	static const Common::Point entries[] = {
		Common::Point(1443, 648), Common::Point(715, 437), Common::Point(1077, 349),
		Common::Point(1137, 348), Common::Point(1165, 345), Common::Point(1165, 345)
	};
	changeFloor(_entryScene >= 8372, entries[_entryScene - 8370]);
	switch (_entryScene) {
	case 8370: // Arrive from the street; Sushi may have news to share.
		_facing = 5;
		if (!_finale.hotelVisited || _finale.bankResearch == 2) {
			_upstairsScene = _finale.bankResearch == 2 ? 8384 : 8380;
			_sushiCalling = true;
			say(0, _upstairsScene == 8384 ? 2 : 0, false, 1);
			walkTo(Common::Point(1160, 701), 7, false, true);
			deferAction(400);
		} else {
			walkTo(Common::Point(1250, 716), 5, false, true);
		}
		break;
	case 8371: // Descend the stairs from Sushi's work area.
		walkTo(Common::Point(1150, 648), 2, false, true);
		break;
	case 8372: // Step onto the remaining upper-floor walkway.
		_facing = 3;
		break;
	case 8373: // Fetch Sushi's rifle for the new sheriff.
		_facing = 0;
		say(31, _finale.rifle == 0);
		deferAction(410);
		break;
	case 8374: // Watch for Oscar to return with his prisoners.
		_facing = 1;
		say(30, 0);
		beginRoomAnimation(411, 10, 13);
		break;
	case 8375: // Watch Oscar carry Gustav and Feodor into the jail.
		_facing = 1;
		_audio.playEffect(68);
		say(30, 1);
		beginRoomAnimationRange(412, 10, 13, 12, 0);
		break;
	default:
		break;
	}
}

void Scene8370::advanceCustomGameplayLoop(uint32 delta) {
	updateOutdoorMusic(5);
	if (_sushiCalling || _sushiAnimating) {
		_sushiTick = _now;
		return;
	}
	while (_now - _sushiTick >= 80) {
		_sushiTick += 80;
		switch (_sushiState) {
		case 1:
			if (_sushiStep < 95) {
				++_sushiStep;
			} else if (!_random.getRandomNumber(2)) {
				_sushiStep = 100;
				_sushiState = 2;
			} else {
				_sushiStep = 70;
			}
			break;
		case 2:
			if (_sushiStep < 113) {
				++_sushiStep;
			} else {
				_sushiState = 3;
			}
			break;
		case 3:
			if (!_random.getRandomNumber(19)) {
				_sushiStep = 120;
				_sushiState = 4;
			}
			break;
		case 4:
			if (_sushiStep < 132) {
				++_sushiStep;
			} else {
				_sushiStep = 70;
				_sushiState = 1;
			}
			break;
		default:
			break;
		}
	}
}

void Scene8370::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	_monitor.draw(viewport, (_now / 80) % 97, _camera.x, -_camera.y);
	_fire.draw(viewport, (_now / 80) % 20, _camera.x, -_camera.y);
	if (!_sushiAnimating || !drawingRoomAnimation()) {
		const uint frame = _sushiCalling ? (speaker() ? speechFrame() % 6 : 0) : kSushiFrames[_sushiStep];
		_sushi.draw(viewport, frame, _camera.x, -_camera.y);
	}
}

float Scene8370::actorHeight() const {
	if (!_upperFloor || _position.y >= 421) {
		return _position.y < 624 ? 175 : Scene6370::actorHeight();
	}
	if (_position.x >= 1172) {
		return _position.x * 0.56f - 556.32f;
	}
	return _position.y > 340 ? 100 : _position.y * -0.424528301f + 276.03775f;
}

void Scene8370::setupActorDepth(RunawayActor::RenderParams &params) const {
	Scene6370::setupActorDepth(params);
	params.depthY[3] = 425;
	params.depthY[7] = _upperFloor && _position.y >= 321 ? 800 : 0;
}

void Scene8370::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

bool Scene8370::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = _upperFloor ? CLIP<int>(target.x, 1077, 1297) : CLIP<int>(target.x, 275, 1306);
	return false;
}

RunawayRoomScene::ActionResult Scene8370::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Return to the street and give Sushi time to inspect the bag.
		if (_finale.bagDelivered == 1) {
			_finale.bagDelivered = 2;
		}
		leaveRoom(8222);
		break;
	case 304: // Climb the stairs to Sushi's work area.
		walkTo(Common::Point(775, 466), 6, false, true);
		leaveRoom(8380);
		break;
	case 308: // Refuse to warm up in the hot weather.
		say(6, _random.getRandomBit());
		break;
	case 309: // Examine the decorative rifles.
		say(7, _random.getRandomBit());
		break;
	case 310: // The decorative rifles cannot replace a real weapon.
		say(7, 1);
		say(8, _inventory.contains(11));
		break;
	case 312: // Take the poker from beside the fire.
		say(10, 0);
		deferAction(414);
		break;
	case 313: // Examine the bell before or after ringing it.
		say(11, _finale.bellRings != 0);
		break;
	case 314: // Ring the bell; repeated ringing annoys Sushi.
		_audio.playEffect(37);
		if (++_finale.bellRings == 5) {
			_finale.bellRings = 1;
			say(12, _random.getRandomBit(), false, 1);
		}
		break;
	case 316: // Enter Sushi's work area from the upper walkway.
		leaveRoom(8381);
		break;
	case 317: // Recognize the rifle Sushi used when Brian first arrived.
		say(14);
		if (!_finale.rifle) {
			_finale.rifle = 1;
		}
		break;
	case 320: // Cross to the stairs and descend to the lobby.
		walkTo(Common::Point(702, 340), 3, false, true);
		deferAction(415);
		break;
	case 323: // Heat the scalpel after discovering that it cannot cut the pods.
		if (!_inventory.finale().heatNeeded) {
			say(19, 0);
		} else if (_inventory.finale().podsOpened) {
			say(19, _finale.mediumTests >= 3 ? 6 : 5);
		} else {
			say(19, 1);
			beginRoomAnimation(416, 7, 38);
		}
		break;
	case 400: // Answer Sushi's invitation to come upstairs.
		say(0, _upstairsScene == 8384 ? 3 : 1);
		_sushiAnimating = true;
		beginRoomAnimationRange(401, 4, 94, 6, 55, false);
		break;
	case 1401: // Follow Sushi after she returns to her computer.
		_sushiCalling = _sushiAnimating = false;
		_finale.hotelVisited = 1;
		walkTo(Common::Point(775, 466), 6, false, true);
		leaveRoom(_upstairsScene);
		break;
	case 410: // Take the rifle straight back to Oscar.
		_finale.rifle = 2;
		leaveRoom(8093);
		return kBackgroundChanged;
	case 1411: // Switch to the street while waiting at the window.
		leaveRoom(8229);
		break;
	case 1412: // Go and tell Sushi that the gangsters have been captured.
		say(30, 2);
		walkTo(Common::Point(655, 238), 7, false, true);
		leaveRoom(8385);
		break;
	case 414: // Add the poker and remove it from the room.
		_finale.pokerTaken = 1;
		_inventory.add(21);
		say(10, 1);
		return kBackgroundChanged;
	case 415: // Resume walking downstairs after the stair transition.
		changeFloor(false, Common::Point(775, 466));
		walkTo(Common::Point(1150, 648), 3, false, true);
		break;
	case 1416: // Leave the scalpel in the fire for three seconds.
		say(19, 2);
		holdRoomAnimation(417, 7, 38, 37);
		break;
	case 1417: // Hold the pose while the blade heats up.
		_heatUntil = _now + 3000;
		holdRoomAnimation(418, 7, 38, 37);
		break;
	case 1418: // Retrieve the hot scalpel.
		say(19, 3);
		beginRoomAnimationRange(419, 7, 38, 37, 0);
		break;
	case 1419: // Return the heated blade to the inventory.
		_inventory.finale().scalpel = 1;
		say(19, 4);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

bool Scene8370::updateCustomSequence() {
	if (_heatUntil && _now < _heatUntil) {
		_actionStarted = _now;
		return true;
	}
	_heatUntil = 0;
	return false;
}

void Scene8370::handleAnimationFrameHook() {
	// Scene6370's greeting animation uses a different frame sequence and sound.
}

} // End of namespace Runaway
} // End of namespace Hollywood
