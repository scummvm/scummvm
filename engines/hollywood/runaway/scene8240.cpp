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
#include "hollywood/runaway/scene8240.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{0, 453, 5, {301, 302}, {0, 3}, {0, 453}, 7}, // 1: Door.
	{782, 408, 7, {303, 304}, {0, 1}, {625, 391}, 4}, // 2: Stairs.
	{395, 402, 7, {305, 306}, {0, 3}, {346, 260}, 3}, // 3: Rutger.
	{1286, 435, 0, {307, 308}, {0, 0}, {1286, 405}}, // 4: Watering can.
	{742, 332, 5, {309, 310}, {0, 0}, {821, 340}}, // 5: Gardening shed.
	{610, 467, 1, {311, 312}, {0, 0}, {669, 431}}, // 6: Door knob.
	{1393, 427, 0, {313, 314}, {0, 0}, {1393, 386}}, // 7: Plant.
	{955, 396, 1, {315, 316}, {0, 0}, {1009, 366}} // 8: Bonsai tree.
};

const RunawayResponse kResponses[] = {
	{8240, 301, 1, 0},  // Look out into town.
	{8240, 303, 2, 1},  // Recognize the stairs to Kevin's room.
	{8240, 307, 5, 0},  // Examine the watering can.
	{8240, 308, 6, 0},  // Leave the watering to Rutger.
	{8240, 309, 7, 3},  // Examine the gardening shed.
	{8240, 312, 10, 0}, // The door knob is carved into the column.
	{8240, 315, 13, 0}, // Examine the bonsai.
	{8240, 316, 14, 0}, // Leave the bonsai where it belongs.
	{8240, 317, 15, 0}  // Refuse to water the plants with an inventory item.
};

const RunawayItemAction kItemActions[] = {
	{8240, 9, 3, 318, false},  // Offer Rutger the Hopi axe-pipe.
	{8240, 10, 3, 319, false}, // Give Rutger the yawaskel seeds.
	{8240, 16, 7, 317, false}, {8240, 16, 8, 317, false},
	{8240, 24, 7, 317, false}, {8240, 24, 8, 317, false}
};

const byte kRutgerFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 0, 0, 0,
	17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 32, 32, 32, 32,
	33, 34, 35, 36, 35, 34, 34, 34, 34, 34, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47
};

// Rutger approaches, speaks at step 50, then hands Brian the cup at step 69.
const byte kBrewFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 46, 46, 46, 46, 47, 48, 49, 50, 51, 46, 47, 48, 49,
	50, 51, 46, 46, 46, 46, 46, 46, 46, 46, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
	63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82,
	83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
	103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
	123, 124, 125, 126, 127, 128, 129, 130, 131, 132
};

const byte kDrinkFrames[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
	27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 28, 29, 30, 31, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene8240 = {
	8240, 1728, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {70, 70, 70}, {50, 50, 50}, {40, 40, 40}}, Common::Point(45, 427),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene8240::Scene8240(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	Scene6240(audio, inventory, strings, game) {}

bool Scene8240::loadCustomResources() {
	_brew = _entryScene == 8243;
	return Scene6240::loadCustomResources() && _rutger.load(_archive, _brew ? 12 : 10, _brew ? 152 : 48);
}

bool Scene8240::applyCustomBackgroundPatches() {
	return applyPatch(9) && (!_brew || applyPatch(13));
}

void Scene8240::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[i] >= 6 && _maps[i] <= 8) {
			_maps[i] = 0;
		}
	}
}

void Scene8240::prepareCustomGameplayLoop() {
	_rutgerTick = _now;
}

void Scene8240::resumeAmbientAudio(bool entering) {
	_audio.stopEffect(2);
	if (entering || !_audio.musicPlaying()) {
		_audio.playMusic(19);
	}
}

void Scene8240::runExitSideEffectsAfterLoop() {
	_audio.stopEffect(0);
}

void Scene8240::runCustomEntrySequence() {
	switch (_entryScene) {
	case 8241: // Return down the stairs from Saturn's workshop.
		_position = Common::Point(782, 408);
		_walkPath.reset(_position);
		walkTo(Common::Point(783, 408), 3, false, true);
		break;
	case 8242: // Return from talking to Rutger; inspect the branch if he mentioned yawaskel.
		_position = Common::Point(395, 402);
		_walkPath.reset(_position);
		_facing = 3;
		if (_finale.rutgerPlantHint) {
			walkTo(Common::Point(676, 531), 4);
			deferAction(430);
		} else {
			walkTo(Common::Point(386, 427), 3);
		}
		break;
	case 8243: // Collect Rutger's finished brew.
		walkTo(Common::Point(88, 403), 3);
		deferAction(410);
		break;
	default:
		walkTo(Common::Point(88, 403), 1);
		if (!_finale.saloonVisited) {
			_finale.saloonVisited = 1;
			deferAction(400);
		}
		break;
	}
}

void Scene8240::advanceCustomGameplayLoop(uint32 delta) {
	while (_now - _rutgerTick >= 80) {
		_rutgerTick += 80;
		if (_brew) {
			if (_rutgerApproaching) {
				if (_rutgerStep < 46) {
					++_rutgerStep;
				} else {
					_rutgerStep = 50;
					_rutgerApproaching = false;
				}
			} else if (_rutgerStep >= 131 && _rutgerStep < 149) {
				++_rutgerStep;
			}
			continue;
		}
		switch (_rutgerState) {
		case kDrumming:
			if (_rutgerStep == 16) {
				_rutgerStep = 0;
				if (!_random.getRandomNumber(19)) {
					_rutgerState = kStartScratching;
					_rutgerStep = 20;
				}
			} else {
				++_rutgerStep;
				if (_rutgerStep == 1 || _rutgerStep == 8 || _rutgerStep == 12) {
					_audio.playEffect(23, 153);
				} else if (_rutgerStep == 4 || _rutgerStep == 10 || _rutgerStep == 16) {
					_audio.playEffect(24, 153);
				}
			}
			break;
		case kStartScratching:
			if (_rutgerStep < 35) {
				++_rutgerStep;
			} else {
				_rutgerStep = 40;
				_rutgerState = kScratching;
			}
			break;
		case kScratching:
			if (_rutgerStep <= 44) {
				_rutgerStep += 2;
			} else if (!_random.getRandomNumber(2)) {
				_rutgerStep = 41;
			} else {
				_rutgerStep = 51;
				_rutgerState = kStopScratching;
			}
			break;
		case kStopScratching:
			if (_rutgerStep < 64) {
				++_rutgerStep;
			} else {
				_rutgerStep = 0;
				_rutgerState = kDrumming;
			}
			break;
		}
	}
}

bool Scene8240::updateCustomSequence() {
	return _rutgerApproaching;
}

void Scene8240::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	uint step = _rutgerStep;
	if (drawingRoomAnimation() && _roomAction == 414) {
		step = 69 + _actionStep;
	} else if (_brew && step == 50 && speaker()) {
		step += speechFrame() % 6;
	}
	_rutger.draw(viewport, _brew ? kBrewFrames[step] : kRutgerFrames[step], _camera.x, -_camera.y);
	drawAmbientAnimations(viewport, &_plantAnimation, 1);
}

Common::U32String Scene8240::hotspotName(byte id) const {
	if (id == 7 && _town.carnivorousPlantSeen) {
		return _strings.hotspotName(_room, 9);
	}
	return RunawayRoomScene::hotspotName(id);
}

RunawayRoomScene::ActionResult Scene8240::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Return to the saloon porch.
		leaveRoom(8181);
		break;
	case 304: // Climb the stairs to Saturn's workshop.
		beginRoomAnimation(304, 5, 19, nullptr, 0, true, 60);
		break;
	case 1304: // Enter the workshop after climbing the stairs.
		leaveRoom(8300);
		break;
	case 305: // Watch Rutger playing his bongos or scratching his foot.
		say(19, _rutgerState != kDrumming);
		break;
	case 306: // Talk to Rutger in the close-up view.
		leaveRoom(8230);
		break;
	case 310: // Search the gardening shed.
		walkTo(Common::Point(742, 332), 5, false, true);
		deferAction(420);
		break;
	case 420: // Find nothing useful in the shed.
		_audio.playEffect(18);
		say(8, 5);
		deferAction(421);
		break;
	case 421: // Step back out of the gardening shed.
		walkTo(Common::Point(855, 347), 3, false, true);
		break;
	case 311: // Recognize the artichoke carved into the column.
		say(9, _town.artichokeSeen);
		_town.artichokeSeen = 1;
		break;
	case 313: // Identify the carnivorous plant.
		say(11, _town.carnivorousPlantSeen);
		_town.carnivorousPlantSeen = 1;
		break;
	case 314: // Refuse to carry the carnivorous plant.
		if (!_town.carnivorousPlantSeen) {
			dispatchCustomSceneAction(313);
		}
		say(12, _random.getRandomBit());
		break;
	case 318: // Trade the tomahawk only after discussing the first medium test.
		if (_finale.mediumTests && _finale.rutgerTranceDiscussed) {
			_inventory.select(0);
			_inventory.remove(9);
			say(16, 1);
			deferAction(418);
		} else {
			say(16);
		}
		break;
	case 418: // Approach Rutger to smoke the axe-pipe.
		walkTo(Common::Point(395, 402), 7);
		leaveRoom(8231);
		break;
	case 319: // Offer Rutger the yawaskel seeds.
		_inventory.select(0);
		_inventory.remove(10);
		say(17);
		deferAction(419);
		break;
	case 419: // Approach Rutger for the seed handover.
		walkTo(Common::Point(395, 402), 7);
		leaveRoom(8232);
		break;
	case 400: // Recognize Rutger on the first visit.
		say(0, 1);
		deferAction(401);
		break;
	case 401: // Approach Rutger for the first conversation.
		walkTo(Common::Point(395, 402), 7);
		leaveRoom(8230);
		break;
	case 410: // Call Rutger from the saloon entrance.
		say(18);
		say(18, 1, false, 1);
		deferAction(411);
		break;
	case 411: // Approach the table where Rutger prepared the brew.
		walkTo(Common::Point(902, 369), 7);
		deferAction(412);
		break;
	case 412: // Ask whether the brew is ready.
		say(18, 2);
		deferAction(413);
		break;
	case 413: // Rutger brings the finished brew to Brian.
		_rutgerApproaching = true;
		_rutgerTick = _now;
		deferAction(414);
		break;
	case 414: // Discuss the brew before taking a drink.
		say(18, 3, false, 1);
		say(18, 4);
		say(18, 5, false, 1);
		beginRoomAnimation(414, 11, 32, kDrinkFrames, ARRAYSIZE(kDrinkFrames));
		break;
	case 1414: // React to the brew while Rutger puts away the cup.
		_rutgerStep = 131;
		_rutgerTick = _now;
		say(18, 6);
		deferAction(415);
		break;
	case 415: { // Cut to black as Brian leaves for his final medium test.
		typedef RunawaySequence S;
		static const S::Command commands[] = {
			{S::kDelay, 500}, {S::kSpeech, 18, 7, 7}, {S::kWaitSpeech}, {S::kEnd}
		};
		_film = kLeaveAfterBrew;
		_sequence.start(commands, _text, _now, 'H');
		break;
	}
	case 430: // Inspect the branch after hearing Rutger's description of yawaskel.
		_facing = 7;
		say(20);
		deferAction(431);
		break;
	case 431: { // Show the original close-up of the branch and its pods.
		typedef RunawaySequence S;
		static const S::Command commands[] = {
			{S::kImage, 151}, {S::kSpeech, 20, 1, 1}, {S::kWaitSpeech}, {S::kEnd}
		};
		_film = kBranchCloseup;
		_sequence.start(commands, _text, _now, 'H');
		break;
	}
	case 432: // Pull the pods off the branch.
		say(20, 2);
		deferAction(433);
		break;
	case 433: // Collect the pods and try opening them by hand.
		_audio.playEffect(38);
		_inventory.add(20);
		say(20, 3);
		deferAction(434);
		break;
	case 434: // The tough pods establish the need for the doctor's scalpel.
		_audio.playEffect(41);
		say(20, 4);
		_finale.scalpelNeeded = 1;
		_finale.rutgerPlantHint = 0;
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene8240::handleAnimationFrameHook() {
	Scene6240::handleAnimationFrameHook();
	if (_roomAction == 414 && _actionStep == 37) {
		_audio.playEffect(42, 230, 1);
	}
}

void Scene8240::finishCustomFilm() {
	_rutgerTick = _now;
	if (_film == kBranchCloseup) {
		deferAction(432);
	} else if (_film == kLeaveAfterBrew) {
		_finale.rutgerBrew = 2;
		leaveRoom(8103);
	}
	_film = kNoFilm;
}

} // End of namespace Runaway
} // End of namespace Hollywood
