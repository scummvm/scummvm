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

#include "common/config-manager.h"
#include "common/util.h"
#include "hollywood/runaway/scene4050.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const byte kRecordSusan[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 29, 28, 27, 26, 25, 26,
	27, 28, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
};

const byte kGrindCoffee[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 26, 27, 28, 29, 30, 31, 32, 33, 34, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 26, 27, 28, 29, 30, 31, 32, 33, 34, 26, 27, 28, 29, 30, 31, 32, 33,
	34, 26, 27, 28, 29, 30, 31, 32, 33, 34, 26, 27, 28, 29, 30, 31, 32, 33, 34, 26,
	27, 28, 29, 30, 31, 32, 33, 34, 26, 27, 28, 29, 30, 31, 32, 33, 34, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13,
	12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
};

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{227, 503, 7, {301, 339}, {0, 1}, {192, 484}, 4}, // 1: Door.
	{311, 493, 7, {302, 303}, {0, 0}, {291, 470}}, // 2: Shelves.
	{382, 529, 1, {304, 305}, {0, 1}, {490, 508}, 3}, // 3: Susan.
	{395, 529, 1, {306, 307}, {0, 0}, {406, 493}}, // 4: Electric lathe.
	{460, 543, 1, {308, 309}, {0, 0}, {527, 501}}, // 5: Microscope.
	{637, 570, 1, {310, 311}, {0, 0}, {692, 530}}, // 6: Water dispenser.
	{703, 575, 1, {312, 313}, {1, 0}, {747, 534}}, // 7: Wastepaper basket.
	{844, 505, 1, {314, 315}, {0, 0}, {862, 481}}, // 8: Tank.
	{884, 580, 1, {316, 317}, {0, 0}, {919, 531}}, // 9: Machine.
	{992, 556, 1, {318, 319}, {1, 0}, {1020, 519}}, // 10: Laser.
	{1327, 521, 1, {320, 321}, {0, 0}, {1376, 495}}, // 11: Shelves.
	{1277, 514, 7, {322, 323}, {1, 1}, {1245, 489}}, // 12: Briefcase.
	{1211, 510, 7, {324, 325}, {0, 0}, {1100, 443}}, // 13: Skeleton.
	{824, 531, 7, {337, 338}, {1, 1}, {794, 503}}, // 14: Strange brush.
};

const RunawayResponse kResponses[] = {
	{kMuseumLaboratory, 301, 1, 0}, // Look at the laboratory door.
	{kMuseumLaboratory, 306, 6, 0}, // Look at the electric lathe.
	{kMuseumLaboratory, 307, 7, 0}, // Use the electric lathe.
	{kMuseumLaboratory, 308, 8, 0}, // Look at the microscope.
	{kMuseumLaboratory, 309, 9, 0}, // Use the microscope.
	{kMuseumLaboratory, 310, 10, 0}, // Look at the water dispenser.
	{kMuseumLaboratory, 311, 11, 0}, // Use the water dispenser.
	{kMuseumLaboratory, 314, 12, 0}, // Look at the tank.
	{kMuseumLaboratory, 316, 13, 0}, // Look at the machine.
	{kMuseumLaboratory, 319, 15, 0}, // Use the laser.
	{kMuseumLaboratory, 320, 16, 0}, // Look at the right shelves.
	{kMuseumLaboratory, 321, 17, 0}, // Take objects from the right shelves.
	{kMuseumLaboratory, 322, 18, 0}, // Look inside the briefcase.
	{kMuseumLaboratory, 324, 20, 0}, // Look at the skeleton.
	{kMuseumLaboratory, 325, 21, 0}, // Use the skeleton.
	{kMuseumLaboratory, 326, 22, 2}, // Leave the crucifix on the restoration shelves.
	{kMuseumLaboratory, 332, 27, 0}, // Use an item with the electric lathe.
	{kMuseumLaboratory, 334, 29, 0}, // Use an item with the microscope.
	{kMuseumLaboratory, 337, 31, 0}, // Look at the talcum powder brush.
};

const RunawayItemAction kItemActions[] = {
	{kMuseumLaboratory, 7, 2, 326}, // Leave the crucifix on the restoration shelves.
	{kMuseumLaboratory, 0, 2, 15, false}, // Use an item with the shelves.
	{kMuseumLaboratory, 5, 3, 328}, // Record Susan's voice.
	{kMuseumLaboratory, 7, 3, 327}, // Give the crucifix to Susan.
	{kMuseumLaboratory, 8, 3, 329, false}, // Show the Mayan object to Susan.
	{kMuseumLaboratory, 11, 3, 330, false}, // Show the Mayan mask to Susan.
	{kMuseumLaboratory, 12, 3, 330, false}, // Show the ruby to Susan.
	{kMuseumLaboratory, 15, 3, 331}, // Give the coffee to Susan.
	{kMuseumLaboratory, 0, 3, 30, false}, // Use an item with Susan.
	{kMuseumLaboratory, 14, 4, 333}, // Grind the coffee beans with the lathe.
	{kMuseumLaboratory, 0, 4, 332, false}, // Use an item with the electric lathe.
	{kMuseumLaboratory, 0, 5, 334, false}, // Use an item with the microscope.
	{kMuseumLaboratory, 0, 7, 19, false}, // Use an item with the wastepaper basket.
	{kMuseumLaboratory, 12, 10, 336}, // Fit the ruby to the laser.
	{kMuseumLaboratory, 0, 10, 335, false}, // Use an item with the laser.
	{kMuseumLaboratory, 0, 11, 15, false}, // Use an item with the shelves.
};

const RunawayExit kExits[] = {
	{kMuseumLaboratory, 339, kMuseumHall, 873, 311, true, true}, // Leave the laboratory.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene4050 = {
	kMuseumLaboratory, 1552, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {60, 60, 70}, {70, 70, 80}, {60, 60, 70}},
	Common::Point(382, 529),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	kExits, ARRAYSIZE(kExits)
};

namespace Runaway {

Scene4050::Scene4050(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game), _susanSpeech(audio, _random, RunawaySpeech::kRecordedLipSync) {}

const char *Scene4050::actionCaptionVariant(byte id, bool look, byte item) const {
	if (item == 14 && id == 4) {
		return _inventory.museum().coffee < 2 ? "coffee" : "empty";
	}
	return nullptr;
}

bool Scene4050::loadCustomResources() {
	return _susanAnimation.load(_archive, 8, 75) && _susanLegsAnimation.load(_archive, 22, 1) &&
		(!museumChapter() || _susanBenchPatch.load(_archive, 12, 2)) &&
		loadForeground(0, 4) && loadForeground(1, 5) && loadForeground(2, 6) && loadForeground(3, 7);
}

bool Scene4050::applyCustomBackgroundPatches() {
	if (_talcTaken && !applyPatch(11)) {
		return false;
	}
	if (!museumChapter()) {
		return true;
	}
	if (_museum.restoration && !applyPatch(_museum.restoration == 1 ? 9 : 10)) {
		return false;
	}
	const int benchFrame = _museum.susan ? 0 : _susanBenchFrame;
	if (benchFrame >= 0) {
		_susanBenchPatch.draw(_background, benchFrame, 0, 0);
	}
	if (!_susanAnimation.load(_archive, _museum.susan == 1 ? 15 : _museum.susan == 3 ? 16 : 8,
			_museum.susan == 1 ? 170 : _museum.susan == 3 ? 2 : 75)) {
		return false;
	}
	_susanFrame = _museum.susan == 3 ? 1 : 0;
	_susanTalking = false;
	_susanWorkCycles = 0;
	_susanTick = _now;
	return true;
}

void Scene4050::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (_talcTaken && id == 14) {
			id = 0;
		}
		if (museumChapter() && _museum.susan == 1) {
			if (id == 15 || id == 16 || id == 17) {
				id = 3;
			} else if (id == 3 || id == 18 || id == 19) {
				id = 0;
			}
		} else {
			if (id == 15 || (id == 4 && (!museumChapter() || !_museum.susan)) || (id == 19 && (!museumChapter() || !_museum.susan))) {
				id = 0;
			} else if (id == 16) {
				id = 13;
			} else if (id == 17) {
				id = 10;
			} else if (id == 18 || id == 19) {
				id = 3;
			} else if (id == 3 && museumChapter() && _museum.susan >= 2) {
				id = 0;
			}
		}
	}
}

void Scene4050::prepareCustomGameplayLoop() {
	_nextLaser = _now + (15 + _random.getRandomNumber(5)) * 1000;
}

void Scene4050::runCustomEntrySequence() {
	if (_museum.susan == 2) {
		_museum.susan = 3;
		_inventory.museum().restoredCrucifix = 1;
		say(32);
		say(32, 1);
		deferMuseumAction(405);
		refreshMuseum();
	} else if (!_museum.susanMet) {
		_museum.susanMet = 1;
		for (uint i = 0; i < 3; ++i) {
			say(80, i);
			say(90, i, false, 1);
		}
		startSusanConversation(true);
	}
}

void Scene4050::resumeAmbientAudio(bool entering) {
	if (entering || !_audio.musicPlaying()) {
		_audio.playMusic(13, false);
	}
}

void Scene4050::advanceCustomGameplayLoop(uint32 delta) {
	if (_susanSpeech.finished(_now) || busy() || _inventoryOpen || _museum.susan != 1) {
		stopAmbientSpeech();
	}
	const bool complaining = !_susanSpeech.subtitle().empty();
	if (_museum.susan >= 2) {
		_susanFrame = 1;
	} else if (speaker() || complaining) {
		_susanFrame = (_museum.susan == 1 ? 52 : 45) + (complaining ? _susanSpeech.frame(_now) : speechFrame());
		_susanTalking = true;
		_susanTick = _now;
	} else if (_susanTalking) {
		_susanTalking = false;
		_susanFrame = _museum.susan == 1 ? 79 : 72;
		_susanTick = _now;
	} else if (_now - _susanTick >= 80) {
		_susanTick = _now;
		advanceSusanIdle();
	}
	if (!_museum.susan && (int32)(_now - _nextLaser) >= 0) {
		if (!busy() && !_walkPath.active() && !_inventoryOpen && _position.x < 530) {
			beginRoomAnimation(450, 23, 95, nullptr, 0, false);
		}
		// A missed attempt is retried later, not as soon as Brian becomes idle.
		_nextLaser = _now + (25 + _random.getRandomNumber(5)) * 1000;
	}
}

void Scene4050::advanceSusanIdle() {
	if (_museum.susan == 1) {
		const bool idle = !busy() && !_walkPath.active() && !_inventoryOpen;
		if (_susanFrame == 33 || _susanFrame == 83 || _susanFrame == 101) {
			if (idle && _camera.x > 22 && !_random.getRandomNumber(7)) {
				_susanFrame = 34;
			} else {
				_susanFrame = _susanFrame == 101 || _random.getRandomBit() ? 0 : 84;
			}
		} else if (_susanFrame == 51) {
			_susanFrame = 79;
			if (idle) {
				const SceneSpeechCue cue = _text.stageCue(33, _random.getRandomNumber(3));
				if (cue.valid()) {
					_susanSpeech.start(Common::U32String(_text.largeTextRecord(cue.textRecordId), Common::kDos850), cue.voiceSampleId, false, _now);
					_susanFrame = 52;
					_susanTalking = true;
				}
			}
		} else {
			++_susanFrame;
		}
		return;
	}

	// Working, resting and returning to the mask are separate frame ranges.
	if (_susanFrame == 14 || _susanFrame == 39) {
		if (++_susanWorkCycles >= 10 + _random.getRandomNumber(4)) {
			_susanWorkCycles = 0;
			_susanFrame = 15;
		} else {
			_susanFrame = _random.getRandomBit() ? 29 : 0;
		}
	} else if (_susanFrame == 23) {
		if (!_random.getRandomNumber(9)) {
			++_susanFrame;
		}
	} else if (_susanFrame == 27 || _susanFrame == 74) {
		_susanFrame = _random.getRandomBit() ? 29 : 0;
	} else {
		++_susanFrame;
	}
}

void Scene4050::stopAmbientSpeech() {
	if (!_susanSpeech.subtitle().empty()) {
		_susanSpeech.stop();
	}
}

const Common::U32String &Scene4050::subtitle() const {
	if (!_sequence.active() && !speaking() && !_susanSpeech.subtitle().empty()) {
		return _susanSpeech.subtitle();
	}
	return RunawayRoomScene::subtitle();
}

void Scene4050::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	const bool movingSusan = museumChapter() && drawingRoomAnimation() &&
		(_roomAction == 450 || _roomAction == 451 || _roomAction == 452 || _roomAction == 331 || _roomAction == 454);
	if (!movingSusan) {
		_susanAnimation.draw(viewport, museumChapter() ? _susanFrame : 0, _camera.x, -_camera.y);
		// Only the seated working poses omit Susan's legs from their frames.
		if (!museumChapter() || !_museum.susan) {
			_susanLegsAnimation.draw(viewport, 0, _camera.x, -_camera.y);
		}
	}
}

void Scene4050::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_position.x > 1200) {
		drawForeground(viewport, 0);
	}
	if (_position.x > 789 && _position.x < 846 && _position.y < 541) {
		drawForeground(viewport, 1);
	}
	if (_position.x < 430) {
		drawForeground(viewport, 2);
	}
	if (_position.y < 501) {
		drawForeground(viewport, 3);
		if (museumChapter() && _museum.susan >= 2) {
			_susanAnimation.draw(viewport, _susanFrame, _camera.x, -_camera.y);
		}
	}
}

RunawayRoomScene::ActionResult Scene4050::dispatchCustomSceneAction(uint16 action) {
	if (museumChapter()) {
		switch (action) {
		case 302: // Look at the restoration shelves.
			say(2, !_museum.restoration ? 0 : _museum.restoration == 2 ? 3 : _museum.susan ? 2 : 1);
			_museum.shelvesSeen = 1;
			return kHandled;
		case 303: // Take objects from the restoration shelves.
			say(3, _museum.restoration == 1);
			return kHandled;
		case 304: // Look at Susan.
			say(4, _museum.susan == 3 ? 2 : _museum.susan == 1 ? 1 : _museum.restoration && _museum.leftLabOnce ? 3 : 0);
			return kHandled;
		case 305: // Talk to Susan.
			if (_museum.susan == 3) {
				say(5);
			} else if (_museum.susan == 1) {
				say(5, 1 + _random.getRandomNumber(3));
				say(5, 5 + _random.getRandomNumber(2), false, 1);
			} else {
				startSusanConversation(false);
			}
			return kHandled;
		case 312: // Look in the wastepaper basket.
			say(19, 1, true);
			return kHandled;
		case 313: // Use the wastepaper basket.
			say(4, 0, true);
			return kHandled;
		case 315: // Use the tank.
			say(7, 0, true);
			return kHandled;
		case 317: // Use the machine.
		case 335: // Use an item with the laser.
			say(14, _random.getRandomNumber(2), true);
			return kHandled;
		case 318: // Look at the laser.
			say(14, _museum.laserUpgraded ? 2 : _museum.laserSeen ? 1 : 0);
			return kHandled;
		case 322: // Look inside the briefcase.
			say(18, _museum.varnishSeen);
			_museum.varnishSeen = 1;
			return kHandled;
		case 326: // Leave the crucifix on the restoration shelves.
			if (_inventory.museum().restoredCrucifix) {
				say(22, 2);
			} else {
				if (!_museum.shelvesSeen) {
					say(2);
				}
				_museum.shelvesSeen = 1;
				say(22);
				beginRoomAnimation(326, 17, 43);
			}
			return kHandled;
		case 327: // Give the crucifix to Susan.
			if (_museum.susan == 3) {
				say(5);
			} else {
				say(_inventory.museum().restoredCrucifix ? 22 : 23, _inventory.museum().restoredCrucifix ? 2 : 0);
			}
			return kHandled;
		case 328: // Record Susan's voice.
			if (!_museum.voiceLockSeen) {
				say(1, 0, true);
			} else if (_museum.susan == 3) {
				say(5);
			} else if (_inventory.contains(6) || _inventory.museum().batteryInLadle) {
				say(24, 3);
			} else if (_inventory.museum().voiceRecorded) {
				say(24, 2);
			} else {
				beginRoomAnimation(328, 21, 56, kRecordSusan, 27);
			}
			return kHandled;
		case 329: // Show the Mayan object to Susan.
		case 330: // Show the mask or ruby to Susan.
			say(_museum.susan == 3 ? 5 : 25);
			return kHandled;
		case 331: // Give the coffee to Susan.
			if (_museum.susan != 1) {
				say(1, 0, true);
			} else {
				say(26);
				byte brian[25], susan[25];
				for (uint i = 0; i < ARRAYSIZE(brian); ++i) {
					brian[i] = i ? i - 1 : 0;
					susan[i] = i < 11 ? 102 + (i ? i - 1 : 0) : i < 20 ? 112 : 93 + i;
				}
				beginRoomAnimation(331, 20, 24, brian, ARRAYSIZE(brian));
				setRoomAnimationOverlay(15, 170, susan, ARRAYSIZE(susan));
			}
			return kHandled;
		case 332: // Use an item with the electric lathe.
			// Only solid objects get this reply in the full game; the demo uses it for every item.
			if (_pendingItem == 7 || _pendingItem == 8 || _pendingItem == 11 || _pendingItem == 12 || _pendingItem == 17) {
				say(27);
			} else {
				say(14, _random.getRandomNumber(2), true);
			}
			return kHandled;
		case 333: // Grind the coffee beans with the lathe.
			if (_inventory.museum().coffee == 2) {
				say(1, 2, true);
			} else if (_inventory.museum().coffee || !_museum.needsGroundCoffee) {
				say(1, 0, true);
			} else {
				say(28);
				beginRoomAnimation(333, 18, 35, kGrindCoffee, ARRAYSIZE(kGrindCoffee));
			}
			return kHandled;
		case 336: // Fit the ruby to the laser.
			if (!_museum.laserSeen) {
				say(1, 0, true);
			} else {
				say(30);
				beginRoomAnimation(336, 19, 13);
			}
			return kHandled;
		case 337: // Look at the talcum powder brush.
			say(31, _museum.talcSeen);
			_museum.talcSeen = 1;
			return kHandled;
		case 339: // Leave the laboratory.
			// Alternate actor bank for the northwest-facing door handle gesture.
			beginActorAnimation(339, 70, 11, 6);
			return kHandled;
		default:
			break;
		}
	}

	switch (action) {
	case 323: // Take the varnish from the briefcase.
		if (_varnishTaken) {
			say(19, 2);
			return kHandled;
		}
		if (museumChapter() && !_museum.varnishSeen) {
			say(18);
		}
		_museum.varnishSeen = 1;
		_varnishTaken = true;
		_inventory.add(9);
		say(19);
		say(19, 1);
		return kHandled;
	case 338: // Take the talcum powder.
		if (!museumChapter() || !_museum.talcSeen) {
			say(31);
		}
		_museum.talcSeen = 1;
		_talcTaken = true;
		_inventory.add(10);
		if (museumChapter()) {
			say(18, _random.getRandomNumber(2), true);
		}
		return kBackgroundChanged;
	default:
		break;
	}
	return kUnhandled;
}

void Scene4050::finishCustomAnimation(uint16 action) {
	switch (action) {
	case 339: // Return to the hall after opening the door.
		_museum.leftLabOnce = 1;
		leaveMuseumRoom(4061, Common::Point(873, 311), 3, true);
		break;
	case 326: // Leave the crucifix and collect the Mayan object.
		_inventory.remove(7);
		_inventory.add(8);
		_museum.restoration = 1;
		say(22, 1);
		break;
	case 328: // Ask Susan to say her name.
		say(24);
		holdRoomAnimation(428, 21, 56, 29);
		break;
	case 428: // Press the recorder button.
		beginRoomAnimation(438, 21, 56, kRecordSusan + 27, 8);
		break;
	case 438: // Record Susan's reply.
		say(24, 1, false, 1);
		holdRoomAnimation(448, 21, 56, 29);
		break;
	case 448: // Put the recorder away.
		beginRoomAnimation(458, 21, 56, kRecordSusan + 35, 29);
		break;
	case 458: // Remember that Susan's voice was recorded.
		_inventory.museum().voiceRecorded = 1;
		break;
	case 331: // Susan drinks the coffee.
		_inventory.remove(15);
		_audio.playEffect(48, 204);
		beginRoomAnimationRange(454, 15, 170, 117, 143, false);
		return;
	case 454: // Finish the exchange with Susan.
		say(26, 1);
		say(26, 2, false, 1);
		if (ConfMan.getBool("restored_content")) {
			say(26, 3);
		}
		deferMuseumAction(453);
		break;
	case 333: // Collect the ground coffee.
		_inventory.museum().coffee = 1;
		say(28, 1);
		break;
	case 336: // Show the ruby installation film.
		startMuseumFilm(kRubyFilm);
		return;
	case 400: // Susan demonstrates the laser after the introductory conversation.
		beginRoomAnimation(450, 23, 95, nullptr, 0, false);
		return;
	case 405: // Collect the restored crucifix.
		_inventory.add(7);
		say(32, 2);
		say(32, 3);
		break;
	case 456: // Finish installing the ruby and approach Susan.
		_inventory.remove(12);
		say(30, 1);
		walkTo(Common::Point(464, 544), 3, true);
		beginRoomAnimation(452, 23, 95, nullptr, 0, false);
		return;
	case 450: // Show the drilling film.
		startMuseumFilm(kDrillingFilm);
		return;
	case 451: // Resume the laboratory after drilling.
		_nextLaser = _now + (15 + _random.getRandomNumber(5)) * 1000;
		break;
	case 452: // Show the upgraded laser film.
		startMuseumFilm(kLaserFilm);
		return;
	case 453: // Show the mobsters film.
		startMuseumFilm(kMobstersFilm);
		return;
	default:
		return;
	}
	refreshMuseum();
}

void Scene4050::handleAnimationFrameHook() {
	// Susan's footsteps are timed to the outward and return animation cels.
	static const AnimationFootstep toLaser[] = {
		{18, 520}, {25, 550}, {29, 580}, {33, 610}, {35, 640}, {38, 670}, {42, 700},
		{45, 730}, {47, 760}, {51, 790}, {54, 820}, {56, 850}, {59, 880}, {62, 910},
		{65, 940}, {68, 970}, {72, 1000}, {75, 1030}, {77, 1060}
	};
	static const AnimationFootstep fromLaser[] = {
		{13, 1060}, {19, 1035}, {25, 1010}, {30, 985}, {33, 960}, {36, 935}, {39, 910},
		{42, 885}, {45, 860}, {48, 835}, {51, 810}, {54, 785}, {57, 760}, {60, 735},
		{63, 710}, {66, 685}, {67, 660}, {69, 635}, {72, 610}, {75, 585}, {78, 560},
		{81, 535}, {84, 510}
	};
	if (_roomAction == 450 || _roomAction == 452) {
		playAnimationFootsteps(_actionStep, 1, toLaser, ARRAYSIZE(toLaser));
	} else if (_roomAction == 451) {
		playAnimationFootsteps(_actionStep, 1, fromLaser, ARRAYSIZE(fromLaser));
	}
	if ((_roomAction == 328 || _roomAction == 428) && (_actionStep == 4 || _actionStep == 2)) {
		_audio.playEffect(37, 204);
	}
	// The original updates the bench as Susan leaves and returns.
	if ((_roomAction == 450 || _roomAction == 452) && _actionStep == 8) {
		_susanBenchFrame = 0;
		_susanBenchPatch.draw(_background, _susanBenchFrame, 0, 0);
	} else if (_roomAction == 451 && _actionStep == 90) {
		_susanBenchFrame = 1;
		_susanBenchPatch.draw(_background, _susanBenchFrame, 0, 0);
	}
}

void Scene4050::finishCustomFilm() {
	switch (_museumFilm) {
	case kDrillingFilm:
		_museum.laserSeen = 1;
		beginRoomAnimation(451, 24, 101, nullptr, 0, false);
		break;
	case kRubyFilm:
		beginRoomAnimationRange(456, 19, 13, 4, 0);
		break;
	case kLaserFilm:
		_museum.susan = 1;
		_museum.laserUpgraded = 1;
		say(30, 2);
		say(33, 1, false, 1);
		refreshMuseum();
		break;
	case kMobstersFilm:
		_museum.susan = 2;
		_museum.restoration = 2;
		leaveMuseumRoom(4061, Common::Point(873, 311), 3, true);
		break;
	default:
		break;
	}
}

float Scene4050::actorHeight() const {
	const int x = _position.x, y = _position.y;
	const byte zone = walkZone();
	if (zone == 4) {
		return y * 1.3125f - 341.0625f;
	}
	if (zone == 3) {
		return x * 0.089147285f + 291.697662f + y - 578;
	}
	if (zone == 5) {
		return 291.697662f + 992 * 0.089147285f - (583 - y) * 0.5882353f - 1;
	}
	if (x < 396) {
		return 327;
	}
	if (x < 913) {
		return x * 0.089147285f + 291.697662f;
	}
	if (x < 1174) {
		return x * 0.09578544f + 285.643677f;
	}
	return MIN(y, 587) * 0.8181818f - 82.272728f;
}

Common::U32String Scene4050::hotspotName(byte id) const {
	if (id == 10 && _museum.laserUpgraded) {
		return _strings.text(RunawayStrings::kAlteredLaser);
	}
	if (id == 12 && _museum.varnishSeen) {
		return _strings.text(RunawayStrings::kVarnishBriefcase);
	}
	if (id == 14 && _museum.talcSeen) {
		return _strings.itemName(2, 10);
	}
	return RunawayRoomScene::hotspotName(id);
}

Common::Point Scene4050::customizeSceneActionPosition(byte id, byte item, Common::Point position) const {
	if (museumChapter() && id == 3 && _museum.susan == 1) {
		return Common::Point(912, 584);
	}
	return position;
}

byte Scene4050::customizeSceneActionFacing(byte id, byte facing) const {
	return museumChapter() && id == 3 && _museum.susan == 1 ? 1 : facing;
}

Common::Point Scene4050::sceneActionLookPosition(byte id) const {
	if (museumChapter() && id == 3) {
		return _museum.susan == 1 ? Common::Point(1063, 563) : Common::Point(490, 481);
	}
	return RunawayRoomScene::sceneActionLookPosition(id);
}

void Scene4050::startSusanConversation(bool introduction) {
	startConversation(1);
	static const DialogueChoice susan[][8] = {
		{{1,0,3,3,3,1,255}, {1,0,1,4,4,1,255}, {1,1,1,5,5,1,255}, {1,0,3,6,6,1,255}, {1,0,0,7,7,1,0}},
		{{1,0,3,8,8,2,0}, {1,0,3,9,9,2,0}, {1,0,3,10,10,2,0}},
		{{1,1,3,11,11,2,0}, {1,1,3,12,12,2,0}, {1,1,3,13,13,2,0}},
		{{1,0,2,14,14,1,255}}
	};
	memcpy(_dialogue[0], susan[0], sizeof(susan[0]));
	memcpy(_dialogue[10], susan[1], sizeof(susan[1]));
	memcpy(_dialogue[11], susan[2], sizeof(susan[2]));
	memcpy(_dialogue[20], susan[3], sizeof(susan[3]));
	if (ConfMan.getBool("restored_content")) {
		// Follow Susan's first mention of the laser, then return to her work topic.
		_dialogue[11][0].node = 0;
		_dialogue[11][0].transition = 1;
		_dialogue[20][0].node = 1;
	}
	if (introduction) {
		_afterConversation = 400;
	} else {
		say(80, 15);
		say(90, 15, false, 1);
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
