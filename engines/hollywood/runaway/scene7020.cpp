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
#include "hollywood/runaway/scene7020.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{0, 0, 0, {301, 302}, {0, 0}, {1052, 627}, 3, true}, // 1: Gina.
	{1500, 540, 0, {303, 304}, {0, 1}, {1554, 490}}, // 2: Statue.
	{1500, 540, 0, {305, 306}, {0, 3}, {1554, 490}, 4}, // 3: Cave.
	{1132, 659, 7, {307, 308}, {0, 0}, {830, 674}}, // 4: Rope.
	{609, 291, 0, {1, 309}, {0, 0}, {103, 507}, 6}, // 5: Lower part of the village.
	{0, 0, 0, {1, 1}, {0, 0}, {0, 0}}, // 6: Piece of rope.
	{609, 291, 0, {1, 309}, {0, 0}, {0, 0}, 7}, // 7: Lower part of the village.
	{128, 501, 0, {310, 311}, {0, 1}, {103, 507}, 5}, // 8: Door.
	{0, 0, 0, {312, 313}, {-1, -1}, {0, 0}, 4}, // 9: Door.
	{0, 0, 0, {312, 314}, {-1, -1}, {186, 428}, 4}, // 10: Door.
	{0, 0, 0, {312, 315}, {-1, -1}, {227, 421}, 4}, // 11: Door.
	{0, 0, 0, {312, 316}, {-1, 0}, {0, 0}, 5}, // 12: Door.
	{0, 0, 0, {312, 317}, {-1, -1}, {274, 304}, 4}, // 13: Door.
	{0, 0, 0, {312, 318}, {-1, -1}, {300, 307}, 4}, // 14: Door.
	{0, 0, 0, {312, 319}, {-1, -1}, {323, 311}, 4}, // 15: Door.
	{0, 0, 0, {320, 321}, {0, -1}, {360, 293}, 5}, // 16: Door at the back.
	{0, 0, 0, {312, 322}, {-1, -1}, {424, 280}, 5}, // 17: Door.
	{0, 0, 0, {323, 324}, {0, 0}, {1590, 536}}, // 18: Kiva.
	{0, 785, 0, {325, 326}, {0, 1}, {0, 745}, 7} // 19: Unexplored area.
};

const RunawayResponse kResponses[] = {
	{7020, 301, 1, 0},  // Check on Gina.
	{7020, 302, 2, 0},  // Let Gina rest.
	{7020, 304, 4, 0},  // Touch the stone statue.
	{7020, 305, 5, 0},  // Examine the mine entrance.
	{7020, 307, 6, 0},  // Inspect the climbing rope.
	{7020, 308, 7, 0},  // Refuse to climb back up the rope.
	{7020, 310, 8, 0},  // Examine the better-preserved houses.
	{7020, 312, 9, 0},  // Notice the damaged paths between houses.
	{7020, 320, 10, 0}, // Examine the connecting doorways.
	{7020, 323, 11, 0}, // Examine the kiva's ladder.
	{7020, 324, 12, 0}, // Refuse to climb down the unsafe ladder.
	{7020, 325, 13, 0}, // Look along the canyon.
	{7020, 328, 15, 0}, // Reject the unsplit timber as a splint.
	{7020, 330, 17, 0}  // Reject unsuitable material for the splint.
};

const RunawayItemAction kItemActions[] = {
	{7020, 1, 1, 31, false}, {7020, 2, 1, 15, false},
	{7020, 3, 1, 15, false}, {7020, 4, 1, 15, false},
	{7020, 5, 1, 31, false}, {7020, 6, 1, 31, false},
	{7020, 7, 1, 31, false}, {7020, 8, 1, 15, false},
	{7020, 9, 1, 31, false}, {7020, 10, 1, 328, false},
	{7020, 11, 1, 15, false}, {7020, 11, 4, 327, false},
	{7020, 12, 1, 331, false}, {7020, 13, 1, 329, false},
	{7020, 14, 1, 31, false}, {7020, 15, 1, 15, false},
	{7020, 16, 1, 31, false}
};

typedef RunawaySequence S;
const S::Command kGinaStory[] = {
	{S::kOpen, 2}, {S::kMusic, 17, 255, 1}, {S::kEffect, 30, 255},
	{S::kSpeech, 0, 5, 5}, {S::kPlay, 0, 240}, {S::kWaitSpeech},
	{S::kSpeech, 0, 6, 6}, {S::kPlay, 241, 693}, {S::kWaitSpeech},
	{S::kSpeech, 0, 7, 7}, {S::kPlay, 694, 870}, {S::kWaitSpeech},
	{S::kMusic, 11, 255}, {S::kEnd}
};

const S::Command kDeparture[] = {
	{S::kOpen, 4}, {S::kMusic, 16, 255}, {S::kStill, 0},
	{S::kTalk, 16, 12, 1}, {S::kPlay, 12, 77}, {S::kEnd}
};

const S::Command kDepartureGog[] = {
	{S::kOpen, 4}, {S::kMusic, 16, 255}, {S::kStill, 0},
	{S::kTalk, 16, 12, 1}, {S::kPlay, 0, 65}, {S::kEnd}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene7020 = {
	7020, 1640, 800, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {80, 70, 70}, {70, 60, 60}, {80, 70, 70}, {90, 80, 80}}, Common::Point(1119, 706),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene7020::Scene7020(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene7020::loadCustomResources() {
	return _ginaArrival.load(_archive, 4, 1) && _gina.load(_archive, 7, 151) && _rope.load(_archive, 8, 32);
}

bool Scene7020::applyCustomBackgroundPatches() {
	return applyPatch(_hopi.rope == 1 ? 13 : 14);
}

void Scene7020::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (_hopi.villageLevel) {
			_maps[i] = 0;
		} else if (_maps[i] >= 9 && _maps[i] <= 12) {
			_maps[i] = 0;
		}
		switch (_hopi.villageLevel) {
		case 0:
			if ((id >= 9 && id <= 17) || id == 5 || id == 7) {
				id = 0;
			}
			break;
		case 1:
			if (id < 9 || id > 11) {
				id = 0;
			}
			break;
		case 2:
			if (id < 13 || id > 15) {
				id = 0;
			}
			break;
		case 3:
			if (id < 16 || id > 17) {
				id = 0;
			}
			break;
		case 4:
			if (id >= 1 && id <= 3) {
				id = 7;
			} else if (id > 7) {
				id = 5;
			}
			break;
		default:
			break;
		}
		if (id == 6 || (id == 4 && _hopi.rope)) {
			id = 0;
		}
	}
}

void Scene7020::prepareCustomGameplayLoop() {
	_ginaTick = _now;
	if (_hopi.legSplinted && !_gina.load(_archive, 12, 151)) {
		error("Cannot load Runaway Gina animation");
	}
}

void Scene7020::resumeAmbientAudio(bool entering) {
	_audio.playEffect(12, 230, 1, true);
}

void Scene7020::runCustomEntrySequence() {
	if (_entryScene == 7021) { // Return from the cave beneath the statue.
		_hopi.ginaFound = 1;
		enterLevel(0, Common::Point(1500, 540), 5);
		walkTo(Common::Point(1432, 584), 5, true);
	} else if (_entryScene == 7022) { // Return from the unexplored end of the canyon.
		_hopi.ginaFound = 1;
		enterLevel(0, Common::Point(0, 785), 3);
		walkTo(Common::Point(434, 661), 3);
	} else if (!_hopi.ginaFound) {
		_arrivalStage = 1;
		_camera = Common::Point(600, 0);
		beginRoomAnimation(400, 5, 55);
	}
}

void Scene7020::enterLevel(byte level, Common::Point position, byte facing) {
	_hopi.villageLevel = level;
	_position = position;
	_walkPath.reset(position);
	_facing = facing;
	updateMaps();
	updateCamera();
}

void Scene7020::startRoute(const RouteStep *route, uint count, uint16 completion) {
	_route = route;
	_routeCount = count;
	_routeIndex = 0;
	_routeCompletion = completion;
	advanceRoute();
}

void Scene7020::advanceRoute() {
	while (_routeIndex < _routeCount) {
		const RouteStep &step = _route[_routeIndex++];
		const Common::Point point(step.x, step.y);
		if (step.level >= 0) {
			enterLevel(step.level, point, step.facing);
		} else {
			walkTo(point, step.facing, true);
			deferAction(490);
			return;
		}
	}
	_route = nullptr;
	if (_routeCompletion) {
		const uint16 action = _routeCompletion;
		_routeCompletion = 0;
		execute(action);
	}
}

void Scene7020::returnToGround() {
	static const RouteStep route[] = {
		{609, 291, -1, 6}, {128, 501, 0, 5}, {85, 523, -1, 5}
	};
	startRoute(route, ARRAYSIZE(route), _hopi.rope == 1 ? 428 : 0);
}

float Scene7020::actorHeight() const {
	switch (_hopi.villageLevel) {
	case 1:
		return 50;
	case 2:
		return 25;
	case 3:
		return 57;
	case 4:
		return _position.y * 3.137254953f - 737.84314f;
	default:
		if (_position.x < 592 && _position.y <= 522) {
			return 55;
		}
		const float height = _position.x < 592 ? _position.y * 1.698019743f - 831.366333f :
			_position.x < 1240 ? _position.y * 0.887755096f - 243.908157f :
			_position.y * 1.70370376f - 806.851868f;
		return MIN<float>(455, height);
	}
}

void Scene7020::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[3] = 510;
	params.depthY[4] = 800;
	params.depthY[5] = 703;
	switch (_hopi.villageLevel) {
	case 1:
		params.depthY[1] = 600;
		params.depthY[3] = _position.x < 200 ? 434 : 440;
		break;
	case 2:
		params.depthY[1] = 600;
		params.depthY[3] = _position.x < 290 ? 314 : 317;
		break;
	case 3:
		params.depthY[1] = 800;
		params.depthY[3] = 314;
		params.depthY[4] = 0;
		break;
	case 4:
		params.depthY[1] = 800;
		params.depthY[2] = 800;
		params.depthY[3] = 292;
		break;
	default:
		break;
	}
}

bool Scene7020::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = MIN<int>(target.x, 1500);
	return false;
}

void Scene7020::findWalkPath(Common::Point target) {
	findGameplayWalkPath(target);
}

void Scene7020::updateGina() {
	if (_arrivalStage || _now - _ginaTick < 80) {
		return;
	}
	_ginaTick = _now;
	switch (_ginaPose) {
	case kMoving:
		if (_ginaFrame < 50) {
			++_ginaFrame;
		} else {
			_ginaPose = kSettling;
			_ginaFrame = 51;
		}
		break;
	case kSettling:
		if (_ginaFrame < 66) {
			++_ginaFrame;
		} else {
			_ginaPose = kResting;
			_ginaFrame = 67;
		}
		break;
	case kResting:
		if (_ginaFrame < 100) {
			++_ginaFrame;
		} else {
			_ginaFrame = 67;
			if (!_random.getRandomNumber(79)) {
				_ginaPose = kTurning;
			}
		}
		break;
	case kTurning:
		if (_ginaFrame > 50) {
			--_ginaFrame;
		} else {
			_ginaPose = kMoving;
			_ginaFrame = 0;
		}
		break;
	case kSittingUp:
		if (_ginaFrame < 138) {
			++_ginaFrame;
		} else {
			_ginaPose = kTalking;
		}
		break;
	case kLyingDown:
		if (_ginaFrame > 102) {
			--_ginaFrame;
		} else {
			_ginaPose = kSettling;
			_ginaFrame = 51;
		}
		break;
	case kTalking:
		break;
	}
}

void Scene7020::advanceCustomGameplayLoop(uint32 delta) {
	updateOutdoorMusic();
	updateGina();
	if ((_roomAction == 410 || _roomAction == 435) && !_actionFrames.empty()) {
		_actionFrames[0] = 11 + (speaking() && !speaker() ? speechFrame() % 6 : 0);
	}
}

void Scene7020::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (_arrivalStage == 1) {
		_ginaArrival.draw(viewport, 0, _camera.x, -_camera.y);
	} else if (!_arrivalStage && _roomAction != 432) {
		const uint frame = _ginaPose == kTalking ? 138 + (speaker() == 1 ? speechFrame() : 0) : _ginaFrame;
		_gina.draw(viewport, frame, _camera.x, -_camera.y);
	}
	if (!_arrivalStage && _position.y > 500 && _roomAction != 427) {
		_rope.draw(viewport, _hopi.rope ? 31 : 1, _camera.x, -_camera.y);
	}
}

void Scene7020::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (!_arrivalStage && _position.y <= 500 && _roomAction != 427) {
		_rope.draw(viewport, _hopi.rope ? 31 : 1, _camera.x, -_camera.y);
	}
}

void Scene7020::handleAnimationFrameHook() {
	if (_roomAction == 400) {
		if (_actionStep == 16) {
			_audio.playEffect(19);
		}
		if (_actionStep > 30) {
			_camera.y = MIN<uint>(200, (_actionStep - 30) * 8);
		}
	} else if (_roomAction == 401) {
		_camera.y = MIN<int>(200, _camera.y + 8);
		if (_actionStep == _actionFrames.size() - 1) {
			_actionLoopSpeech = false;
		}
	} else if (_roomAction == 427 && _actionStep == 51) {
		_audio.playEffect(21);
	} else if (_roomAction == 432) {
		if (_actionStep == _actionFrames.size() - 1) {
			_actionLoopSpeech = false;
		}
		if (_actionStep == 22) {
			_audio.playEffect(26);
		} else if (_actionStep == 52) {
			say(16, 2, false, 1);
		}
	}
}

void Scene7020::finishCustomFilm() {
	if (_hopi.legSplinted) {
		leaveRoom(8092);
		return;
	}
	_hopi.ginaFound = 1;
	_arrivalStage = 0;
	enterLevel(0, Common::Point(1132, 659), 7);
	resumeAmbientAudio(false);
	_ginaPose = kTalking;
	_ginaFrame = 138;
	for (byte variation = 8; variation <= 16; ++variation) {
		say(0, variation, false, (variation & 1) ? 0 : 1);
	}
	holdRoomAnimation(410, 15, 18, 11);
}

void Scene7020::startSplintAnimation() {
	Common::Array<byte> brian, gina;
	brian.push_back(0);
	for (uint i = 0; i <= 32; ++i) {
		brian.push_back(i);
	}
	for (uint repeat = 0; repeat < 4; ++repeat) {
		for (uint i = 21; i <= 32; ++i) {
			brian.push_back(i);
		}
	}
	for (uint i = 33; i <= 42; ++i) {
		brian.push_back(i);
		if (i == 39) {
			brian.push_back(i);
		}
	}
	for (uint i = 0; i < brian.size(); ++i) {
		gina.push_back(i < 34 ? 0 : i < 59 ? i - 33 : i < 69 ? 25 : i < 81 ? 26 : MIN<uint>(33, i - 54));
	}
	beginRoomAnimation(432, 10, 43, brian.data(), brian.size());
	setRoomAnimationOverlay(11, 34, gina.data(), gina.size());
	_actionHoldSpeech = true;
	_actionLoopSpeech = true;
}

RunawayRoomScene::ActionResult Scene7020::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 1: // Looking down from the ledge does not start a speech.
		break;
	case 31: // Keep items that cannot help Gina.
		say(25, 0, true);
		break;
	case 303: // Examine the statue guarding the cave.
		say(3, _random.getRandomBit());
		break;
	case 306: // Enter the mine beside the statue.
		leaveRoom(7050);
		break;
	case 309: // Return from the rope ledge to the village floor.
		returnToGround();
		break;
	case 311: { // Enter the ground-floor doorway and emerge on the first roof.
		static const RouteStep route[] = {{160, 428, 1, 3}, {188, 436, -1, 3}};
		startRoute(route, ARRAYSIZE(route));
		break;
	}
	case 313: // The blocked doorway on the first roof does not lead anywhere.
	case 316: // The damaged second-floor doorway is also inaccessible.
		break;
	case 314: { // Return through the left door on the first roof.
		static const RouteStep route[] = {
			{188, 436, -1, 5}, {160, 428, -1, 7}, {128, 501, 0, 5}, {85, 523, -1, 5}
		};
		startRoute(route, ARRAYSIZE(route));
		break;
	}
	case 315: { // Reach the second roof through the right-hand doorway.
		static const RouteStep route[] = {
			{233, 440, -1, 3}, {206, 438, -1, 7}, {311, 318, 2, 3}, {325, 319, -1, 3}
		};
		startRoute(route, ARRAYSIZE(route));
		break;
	}
	case 317: { // Cross the second roof and climb to the third.
		static const RouteStep route[] = {
			{275, 315, -1, 5}, {262, 313, -1, 7}, {394, 300, 3, 5},
			{352, 308, -1, 5}, {391, 302, -1, 3}
		};
		startRoute(route, ARRAYSIZE(route));
		break;
	}
	case 318: { // The middle doorway leads back to ground level.
		static const RouteStep route[] = {
			{302, 317, -1, 5}, {288, 315, -1, 7}, {128, 501, 0, 5}, {85, 523, -1, 5}
		};
		startRoute(route, ARRAYSIZE(route));
		break;
	}
	case 319: { // Return from the second roof to the first.
		static const RouteStep route[] = {
			{325, 319, -1, 3}, {311, 318, -1, 7}, {206, 438, 1, 3}, {233, 440, -1, 3}
		};
		startRoute(route, ARRAYSIZE(route));
		break;
	}
	case 321: { // Take the rear doorway from the third roof to the second.
		static const RouteStep route[] = {
			{352, 308, -1, 5}, {394, 300, -1, 3}, {262, 313, 2, 3}, {275, 315, -1, 3}
		};
		startRoute(route, ARRAYSIZE(route));
		break;
	}
	case 322: { // Reach the high ledge beside the climbing rope.
		static const RouteStep route[] = {
			{448, 294, -1, 3}, {609, 291, 4, 3}, {817, 334, -1, 4}
		};
		startRoute(route, ARRAYSIZE(route));
		break;
	}
	case 326: // Explore the other end of the canyon.
		leaveRoom(7030);
		break;
	case 327: // Cut enough rope for Gina's splint from the high ledge.
		say(14, _hopi.villageLevel ? 1 : 0);
		if (_hopi.villageLevel) {
			deferAction(427);
		}
		break;
	case 329: // Check the materials and the crypt before splinting Gina's leg.
		if (!_inventory.contains(12)) {
			say(16);
		} else if (!_hopi.cryptVisited) {
			say(16, 13);
		} else {
			walkTo(Common::Point(978, 670), 0);
			deferAction(431);
		}
		break;
	case 331: // Rope also needs the two wooden splints.
		if (_inventory.contains(13)) {
			execute(329);
		} else {
			say(18);
		}
		break;
	case 1400: // Brian reaches the ground and calls to Gina.
		say(0);
		holdRoomAnimation(402, 5, 55, 54);
		break;
	case 1402: // Gina calls back as Brian runs over to her.
		_arrivalStage = 2;
		say(0, 1, false, 1);
		beginRoomAnimationRange(401, 6, 40, 0, 30);
		_actionHoldSpeech = true;
		_actionLoopSpeech = true;
		break;
	case 1401: // Brian is relieved to find Gina alive.
		say(0, 2);
		holdRoomAnimation(403, 6, 40, 30);
		break;
	case 1403: // Brian settles beside Gina.
		beginRoomAnimationRange(404, 6, 40, 31, 39);
		break;
	case 1404: // Ask how Gina reached the village.
		say(0, 3);
		say(0, 4, false, 1);
		holdRoomAnimation(405, 6, 40, 39);
		break;
	case 1405: // Gina describes her escape from the mine.
		_sequence.start(kGinaStory, _text, _now, 'G');
		break;
	case 1410: // Stand up after discussing the crypt with Gina.
		beginRoomAnimationRange(411, 15, 18, 11, 0);
		break;
	case 1411: // Step back while Gina lies down again.
		_ginaPose = kLyingDown;
		_ginaFrame = 138;
		walkTo(Common::Point(1119, 706), 4);
		deferAction(412);
		break;
	case 412: // Consider how to make a splint.
		say(0, 17);
		break;
	case 427: { // Cut the rope, animating its falling end separately.
		Common::Array<byte> brian, rope;
		brian.push_back(0);
		for (uint i = 0; i < 91; ++i) {
			brian.push_back(i);
		}
		for (uint i = 0; i < brian.size(); ++i) {
			rope.push_back(i < 54 ? 1 : MIN<uint>(31, i - 52));
		}
		beginRoomAnimation(427, 9, 91, brian.data(), brian.size());
		setRoomAnimationOverlay(8, 32, rope.data(), rope.size());
		break;
	}
	case 1427: // Leave the severed rope on the village floor.
		_hopi.rope = 1;
		say(14, 2);
		deferAction(309);
		return kBackgroundChanged;
	case 428: // Approach the fallen rope.
		walkTo(Common::Point(793, 738), 3);
		deferAction(429);
		break;
	case 429: // Examine the cut rope before collecting it.
		say(14, 3);
		deferAction(430);
		break;
	case 430: // Pick up the rope for the splint.
		_hopi.rope = 2;
		_inventory.add(12);
		say(14, 4);
		return kBackgroundChanged;
	case 431: // Tell Gina to keep still while Brian makes the splint.
		say(16, 1);
		deferAction(432);
		break;
	case 432: // Tie the wooden splints around Gina's leg.
		startSplintAnimation();
		break;
	case 1432: // Show Gina with the completed splint.
		_hopi.legSplinted = 1;
		if (!_gina.load(_archive, 12, 151)) {
			error("Cannot load Runaway Gina animation");
		}
		_ginaFrame = 102;
		_ginaPose = kSittingUp;
		say(16, 3);
		deferAction(433);
		break;
	case 433: // Approach Gina to discuss leaving the canyon.
		walkTo(Common::Point(1132, 659), 7);
		deferAction(434);
		break;
	case 434: // Kneel beside Gina.
		beginRoomAnimationRange(434, 15, 18, 0, 11);
		break;
	case 1434: // Discuss the crypt before the journey back.
		for (byte variation = 4; variation <= 10; ++variation) {
			say(16, variation, false, variation & 1);
		}
		holdRoomAnimation(435, 15, 18, 11);
		break;
	case 1435: // Stand up to help Gina.
		beginRoomAnimationRange(436, 15, 18, 11, 0);
		break;
	case 1436: // Tell Gina it is time to leave.
		say(16, 11);
		deferAction(437);
		break;
	case 437: // Help Gina leave the village.
		_sequence.start(Common::File::exists("DATAVG04.000") ? kDepartureGog : kDeparture, _text, _now, 'G');
		break;
	case 490: // Continue along the current doorway route.
		advanceRoute();
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

} // End of namespace Runaway
} // End of namespace Hollywood
