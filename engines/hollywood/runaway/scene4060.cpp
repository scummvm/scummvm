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
#include "hollywood/runaway/scene4060.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const byte kCoffeeCupFrames[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 26, 27,
	28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48,
};

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{-25, 319, 7, {301, 302}, {0, 1}, {68, 371}, 7}, // 1: Stairs.
	{218, 379, 7, {321, 322}, {1, 1}, {180, 245}}, // 2: Coffee machines.
	{607, 323, 7, {311, 312}, {0, 1}, {606, 322}, 4}, // 3: Clive's office door.
	{696, 319, 7, {332, 333}, {0, 0}, {676, 164}}, // 4: Fire extinguisher.
	{696, 319, 7, {313, 314}, {1, 1}, {668, 280}}, // 5: Wastepaper basket.
	{873, 311, 7, {315, 316}, {0, 1}, {872, 310}, 4}, // 6: Door to the restoration laboratory.
	{1118, 309, 7, {317, 318}, {1, 1}, {1094, 299}}, // 7: Security door.
	{1153, 297, 7, {319, 320}, {1, 1}, {1152, 296}}, // 8: Panel.
	{1220, 280, 7, {301, 303}, {0, 1}, {1279, 290}, 7}, // 9: Stairs.
	{1473, 312, 7, {324, 325}, {0, 1}, {1440, 188}}, // 10: Telephone.
	{1596, 351, 1, {326, 327}, {0, 0}, {1694, 186}}, // 11: Display case.
	{140, 504, 7, {304, 305}, {0, 1}, {206, 618}, 4}, // 12: Stairs.
	{1085, 411, 7, {304, 306}, {0, 1}, {1140, 446}, 4}, // 13: Stairs.
	{921, 813, 7, {307, 308}, {0, 1}, {840, 730}, 3}, // 14: Maintenance worker.
	{1588, 506, 7, {309, 310}, {0, 1}, {1610, 450}, 5}, // 15: Mayan exhibition.
	{711, 797, 7, {328, 329}, {1, 0}, {670, 726}}, // 16: Plaque.
	{1060, 730, 7, {330, 331}, {1, 0}, {984, 638}}, // 17: Plaque.
};

const RunawayResponse kResponses[] = {
	{kMuseumHall, 301, 1, 0}, // Look at the stairs from the upper floor.
	{kMuseumHall, 304, 2, 0}, // Look at the stairs from the lower floor.
	{kMuseumHall, 309, 4, 0}, // Look at the Mayan exhibition entrance.
	{kMuseumHall, 311, 5, 0}, // Look at Clive's office door.
	{kMuseumHall, 314, 7, 0}, // Take the empty coffee package.
	{kMuseumHall, 315, 8, 0}, // Look at the laboratory door.
	{kMuseumHall, 317, 9, 0}, // Look at the security door.
	{kMuseumHall, 318, 13, 0}, // Enter the analysis laboratory.
	{kMuseumHall, 319, 10, 0}, // Look at the keypad.
	{kMuseumHall, 321, 11, 0}, // Look at the coffee machine.
	{kMuseumHall, 323, 13, 0}, // Try the locked security door.
	{kMuseumHall, 324, 14, 0}, // Look at the telephone.
	{kMuseumHall, 326, 16, 0}, // Look at the display case.
	{kMuseumHall, 327, 17, 0}, // Open the display case.
	{kMuseumHall, 328, 18, 0}, // Read the left plaque.
	{kMuseumHall, 329, 19, 0}, // Use the left plaque.
	{kMuseumHall, 330, 20, 0}, // Read the right plaque.
	{kMuseumHall, 331, 19, 0}, // Use the right plaque.
	{kMuseumHall, 332, 21, 0}, // Look at the fire extinguisher.
	{kMuseumHall, 333, 22, 0}, // Take the fire extinguisher.
	{kMuseumHall, 336, 25, 0}, // Show the crucifix to Willy.
	{kMuseumHall, 339, 29, 0}, // Show the Mayan mask to Willy.
	{kMuseumHall, 340, 30, 0}, // Show the ruby to Willy.
	{kMuseumHall, 341, 31, 0}, // Show Willy his business card.
	{kMuseumHall, 343, 34, 0}, // Give the cup of coffee to Willy.
	{kMuseumHall, 344, 35, 0}, // Show the strange key to Willy.
};

const RunawayItemAction kItemActions[] = {
	{kMuseumHall, 19, 2, 346, false}, // Use the coffee package with the coffee machine.
	{kMuseumHall, 0, 5, 19, false}, // Use an item with the wastepaper basket.
	{kMuseumHall, 9, 8, 337}, // Coat the keypad with varnish.
	{kMuseumHall, 10, 8, 338}, // Dust the keypad with talcum powder.
	{kMuseumHall, 1, 14, 11, false}, // Use wallet with maintenance worker.
	{kMuseumHall, 2, 14, 11, false}, // Use car keys with maintenance worker.
	{kMuseumHall, 3, 14, 11, false}, // Show Willy the receipt (condom in the demo).
	{kMuseumHall, 4, 14, 11, false}, // Use piece of amber with maintenance worker.
	{kMuseumHall, 5, 14, 334, false}, // Show the tape recorder to Willy.
	{kMuseumHall, 6, 14, 335, false}, // Show the battery to Willy.
	{kMuseumHall, 7, 14, 336, false}, // Show the crucifix to Willy.
	{kMuseumHall, 8, 14, 11, false}, // Use strange mayan object with maintenance worker.
	{kMuseumHall, 9, 14, 11, false}, // Use colorless varnish with maintenance worker.
	{kMuseumHall, 10, 14, 11, false}, // Use talcum powder with maintenance worker.
	{kMuseumHall, 11, 14, 339, false}, // Show the Mayan mask to Willy.
	{kMuseumHall, 12, 14, 340, false}, // Show the ruby to Willy.
	{kMuseumHall, 13, 14, 341, false}, // Show Willy his business card.
	{kMuseumHall, 14, 14, 342}, // Give the bowl of coffee to Willy.
	{kMuseumHall, 15, 14, 343, false}, // Give the cup of coffee to Willy.
	{kMuseumHall, 16, 14, 11, false}, // Use pair of books with maintenance worker.
	{kMuseumHall, 17, 14, 344, false}, // Show the strange key to Willy.
	{kMuseumHall, 18, 14, 11, false}, // Use ladle with maintenance worker.
	{kMuseumHall, 19, 14, 345}, // Give the filled coffee package to Willy.
};

const RunawayExit kExits[] = {
	{kMuseumHall, 302, kMuseumHall, 533, 848, false, false}, // Go downstairs on the left.
	{kMuseumHall, 303, kMuseumHall, 1169, 643, false, false}, // Go downstairs on the right.
	{kMuseumHall, 305, kMuseumHall, 69, 372, true, false}, // Go upstairs on the left.
	{kMuseumHall, 306, kMuseumHall, 1290, 300, true, false}, // Go upstairs on the right.
	{kMuseumHall, 310, kMuseumExhibition, 881, 392, false, false}, // Enter the Mayan exhibition.
	{kMuseumHall, 312, kMuseumOffice, 934, 563, true, true}, // Enter Clive's office.
	{kMuseumHall, 316, kMuseumLaboratory, 382, 529, true, true}, // Enter the restoration laboratory.
	{kMuseumHall, 320, kMuseumKeypad, 0, 0, true, false}, // Examine the keypad close up.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene4060 = {
	kMuseumHall, 1760, 1200, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {40, 40, 40}, {30, 30, 30}, {60, 60, 60}, {30, 30, 30}, {70, 70, 70}, {20, 20, 25}, {50, 50, 50}, {60, 60, 70}, {70, 70, 80}},
	Common::Point(873, 311),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	kExits, ARRAYSIZE(kExits)
};

namespace Runaway {

Scene4060::Scene4060(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

byte Scene4060::actionCursor(byte id) const {
	if (id == 7) {
		return _museum.keypad == 4 ? 4 : 0;
	}
	return RunawayRoomScene::actionCursor(id);
}

const char *Scene4060::actionCaptionVariant(byte id, bool look, byte item) const {
	if (!look && !item) {
		if (id == 1 || id == 9 || id == 12 || id == 13) {
			return _upperFloor ? "downstairs" : "upstairs";
		}
		if (id == 5 && !_coffeeTaken && _museum.needsPackage) {
			return "coffee";
		}
		if (id == 7 && _museum.keypad == 4) {
			return "unlocked";
		}
	}
	return nullptr;
}

bool Scene4060::loadCustomResources() {
	return _willyAnimation.load(_archive, 8, 22);
}

void Scene4060::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		// Both floors share the image and mask, but have separate walk zones and hotspots.
		const byte zone = _maps[i];
		if (_upperFloor ? zone < 5 || zone > 6 : zone > 4) {
			_maps[i] = 0;
		}
		if (museumChapter() && _museum.willyInLab && id == 14) {
			id = 0;
		}
		if (_upperFloor ? id > 11 : id < 12) {
			id = 0;
		}
	}
}

void Scene4060::runCustomEntrySequence() {
	if (_entryScene == 4010) {
		startMuseumFilm(kArrivalFilm);
		return;
	}
	if (_entryScene != 4063 && _entryScene != 4064) {
		_museum.willyInLab = 0;
		updateMaps();
	} else if (_entryScene == 4064 && _museum.willyInLab) {
		changeFloor(false, Common::Point(921, 813));
		if (_museum.willyInLab == 1) {
			walkTo(Common::Point(1029, 842), 7, true);
			say(38, 1);
			deferMuseumAction(460);
		} else {
			say(36, 3);
			deferMuseumAction(462);
		}
	}
	if (_entryScene == 4063) {
		if (_museum.keypad == 1) {
			say(26, 2);
		} else if (_museum.keypad == 4) {
			finishCustomAnimation(470);
		}
	} else if (_entryScene == 4062) {
		beginRoomAnimationRange(473, 4, 13, 12, 0, false);
		_audio.playEffectAt(32, Common::Point(1138, 200), 204);
	} else if (_entryScene == 4061 && _museum.susan == 2) {
		say(40);
		deferMuseumAction(316);
	}
}

void Scene4060::advanceCustomGameplayLoop(uint32 delta) {
	_willyFrame = (_now / 80) % 22;
}

void Scene4060::updateActor(uint elapsed, int scrollDirection, bool inventoryOpen) {
	RunawayRoomScene::updateActor(elapsed, scrollDirection, inventoryOpen);
	if (!inventoryOpen && !_upperFloor && (!_roomAction || _roomAction < 461 || _roomAction > 466)) {
		_camera.y = CLIP<int>(_position.y - 400, 200, 600);
	}
}

void Scene4060::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (!museumChapter() || !_museum.willyInLab) {
		_willyAnimation.draw(viewport, museumChapter() ? _willyFrame : 0, _camera.x, -_camera.y);
	}
}

RunawayRoomScene::ActionResult Scene4060::dispatchCustomSceneAction(uint16 action) {
	if (museumChapter()) {
		switch (action) {
		case 307: // Look at Willy.
			say(3, _museum.willyMet != 0);
			return kHandled;
		case 308: // Talk to Willy.
			leaveMuseumRoom(4070, Common::Point(548, 580), 7);
			return kHandled;
		case 312: // Open Clive's office door.
		case 316: // Open the restoration laboratory door.
			// RESOURCE.001 bank 64: reach for the handle, facing northwest.
			beginActorAnimation(action, 64, 11, 6);
			return kHandled;
		case 313: // Look in the wastepaper basket.
			if (!_museum.basketSeen) {
				say(6);
				say(6, 1);
				_museum.basketSeen = 1;
			} else {
				say(_coffeeTaken ? 7 : 6, _coffeeTaken ? 0 : 2);
			}
			return kHandled;
		case 314: // Take the empty coffee package.
			if (!_museum.basketSeen || _coffeeTaken) {
				say(7);
			} else if (!_museum.needsPackage) {
				say(7, 1);
			} else {
				_coffeeTaken = true;
				say(7, 2);
				_inventory.add(19);
				say(114, 0, true);
			}
			return kHandled;
		case 317: // Look at the security door.
			say(9, _museum.analysisDoorSeen);
			_museum.analysisDoorSeen = 1;
			return kHandled;
		case 318: { // Enter the code to open the analysis laboratory.
			if (_museum.keypad != 4) {
				say(13);
			} else {
				static const byte frames[] = {
					0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
					15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17
				};
				beginRoomAnimation(470, 10, 19, frames, ARRAYSIZE(frames));
			}
			return kHandled;
		}
		case 320: // Examine the keypad close up.
			if (_museum.willyInLab == 1) {
				say(15, 5);
			} else {
				leaveMuseumRoom(4140, Common::Point(382, 529));
			}
			return kHandled;
		case 321: // Look at the coffee machine.
			if (!_museum.coffeeMachine && !_museum.coffeeSeen) {
				say(11, 1);
			}
			say(11, _museum.coffeeMachine ? 2 : _museum.coffeeSeen ? _random.getRandomBit() : 0);
			_museum.coffeeSeen = 1;
			return kHandled;
		case 322: // Use the coffee machine.
			_museum.coffeeSeen = 1;
			if (_museum.coffeeMachine != 1) {
				say(12, _museum.coffeeMachine ? 3 : 0);
			} else {
				say(12, 1);
				beginRoomAnimation(322, 9, 49, kCoffeeCupFrames, ARRAYSIZE(kCoffeeCupFrames));
			}
			return kHandled;
		case 325: // Call Willy.
			if (!_inventory.contains(13)) {
				say(15);
			} else if (_museum.willyInLab == 1) {
				if (!_museum.phoneUsed && !_museum.phonePlanExplained) {
					_museum.phonePlanExplained = 1;
					say(15, 1);
					say(15, 8);
				}
				say(15, 7);
			} else if (_museum.keypad > 1) {
				say(15, 6);
			} else if (_museum.phoneUsed && !_museum.keypad) {
				say(15, 3);
			} else {
				say(15, _museum.phoneUsed ? 2 : 1);
				beginRoomAnimation(325, 13, 27);
			}
			return kHandled;
		case 334: // Show the tape recorder to Willy.
			say(23, _inventory.museum().voiceRecorded);
			return kHandled;
		case 335: // Show the battery to Willy.
			say(24, _inventory.museum().frozenBattery);
			return kHandled;
		case 337: // Coat the keypad with varnish.
			if (_museum.willyInLab == 1) {
				say(28);
			} else if (!_museum.sawCode) {
				say(26);
			} else if (_museum.keypad) {
				say(26, 3);
			} else {
				say(26, 1);
				deferMuseumAction(337);
			}
			return kHandled;
		case 338: // Dust the keypad with talcum powder.
			if (_museum.willyInLab == 1) {
				say(28);
			} else if (!_museum.sawCode) {
				say(26);
			} else if (_museum.keypad < 2) {
				say(27, _museum.keypad);
			} else if (_museum.keypad > 2) {
				say(26, 3);
			} else {
				say(27, 2);
				deferMuseumAction(338);
			}
			return kHandled;
		case 342: // Give the bowl of coffee to Willy.
			if (_inventory.museum().coffee == 2) {
				say(32, 2);
			} else {
				leaveMuseumRoom(4072, Common::Point(548, 580), 7);
			}
			return kHandled;
		case 345: // Give the filled coffee package to Willy.
			if (!_inventory.museum().packageFilled) {
				say(36);
			} else {
				leaveMuseumRoom(4071, Common::Point(548, 580), 7);
			}
			return kHandled;
		case 346: // Use the coffee package with the coffee machine.
			say(37, _inventory.museum().packageFilled);
			return kHandled;
		default:
			return kUnhandled;
		}
	}

	if (action != 313) {
		return kUnhandled;
	}
	if (_coffeeTaken) {
		say(7);
		return kHandled;
	}
	_coffeeTaken = true;
	_inventory.add(19);
	say(6);
	say(6, 1);
	return kHandled;
}

void Scene4060::finishCustomAnimation(uint16 action) {
	switch (action) {
	case 312: // Enter Clive's office after opening the door.
		leaveMuseumRoom(4030, Common::Point(934, 563), 3, true);
		break;
	case 316: // Enter the restoration laboratory after opening the door.
		leaveMuseumRoom(4050, Common::Point(382, 529), 3, true);
		break;
	case 470: // Open the unlocked security door.
		beginRoomAnimation(471, 4, 13, nullptr, 0, false);
		_audio.playEffectAt(32, Common::Point(1138, 200), 204);
		break;
	case 471: // Keep the door open while Brian walks through it.
		_actionAnimation.draw(_background, 12, 0, 0);
		walkTo(Common::Point(980, 275), 7, true);
		deferMuseumAction(472);
		break;
	case 472: // Enter the analysis laboratory or show the gangsters arriving downstairs.
		if (_inventory.museum().restoredCrucifix) {
			byte frames[40];
			for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
				frames[i] = i < 20 ? 0 : i - 19;
			}
			_entranceCameraX = _camera.x;
			beginRoomAnimation(476, 11, 21, frames, ARRAYSIZE(frames), true, 10);
		} else {
			leaveMuseumRoom(4100, Common::Point(141, 468));
		}
		break;
	case 476: // Continue the museum escape after the gangsters enter.
		startMuseumFilm(kMuseumExitFilm);
		break;
	case 460: // Willy approaches the security door.
		_camera.y = 0;
		beginRoomAnimationRange(461, 16, 216, 0, 178, false);
		break;
	case 461: { // Open the security door for Willy.
		beginRoomAnimationRange(464, 4, 13, 0, 12, false);
		const byte willy[] = {178};
		setRoomAnimationOverlay(16, 216, willy, ARRAYSIZE(willy));
		_audio.playEffect(32, 204);
		break;
	}
	case 464: { // Willy enters the analysis laboratory.
		beginRoomAnimationRange(465, 16, 216, 179, 215, false);
		const byte door[] = {12};
		setRoomAnimationOverlay(4, 13, door, ARRAYSIZE(door));
		break;
	}
	case 465: // Close the security door behind Willy.
		beginRoomAnimationRange(466, 4, 13, 12, 0, false);
		_audio.playEffect(32, 204);
		break;
	case 466: // React to Willy entering the code.
		updateCamera();
		if (_museum.sawCode == 1) {
			say(38, 2);
		} else {
			walkTo(Common::Point(1474, 312), 5, true);
			deferMuseumAction(475);
		}
		updateMaps();
		break;
	case 475: // Come out of hiding after the telephone distraction.
		say(15, 5);
		break;
	case 462: // Follow Willy to the coffee machine.
		say(36, 4);
		walkTo(Common::Point(140, 504), 7, true);
		_camera = Common::Point(0, 0);
		beginRoomAnimationRange(463, 16, 216, 0, 150, false);
		break;
	case 463: // Finish refilling the coffee machine.
		_museum.willyInLab = 0;
		changeFloor(true, Common::Point(218, 379));
		_facing = 7;
		say(36, 5);
		break;
	case 322: // Collect the cup of coffee.
		_museum.coffeeMachine = 2;
		_inventory.add(15);
		say(12, 2);
		break;
	case 325: // Start the telephone conversation.
		startMuseumFilm(kPhoneFilm);
		break;
	case 474: // Hide after putting down the telephone.
		say(15, 4);
		walkTo(Common::Point(1440, 271), 7, true);
		deferMuseumAction(460);
		break;
	case 337: // Show the varnished keypad.
		_museum.keypad = 1;
		leaveMuseumRoom(4141, Common::Point(382, 529));
		break;
	case 338: // Show the fingerprints on the keypad.
		_museum.keypad = 3;
		leaveMuseumRoom(4142, Common::Point(382, 529));
		break;
	}
}

void Scene4060::handleAnimationFrameHook() {
	if (_roomAction == 476) {
		if (_actionStep <= 18) {
			setCameraX(_entranceCameraX * (18 - _actionStep) / 18);
			_camera.y = _actionStep * 32;
		}
		if (_actionStep == 18) {
			// The camera uses the 10 ms timer; the walking cels use 60 ms.
			_actionFrameDuration = 60;
			_audio.playMusic(21);
		}
		const byte frame = _actionFrames[_actionStep];
		if (frame == 1 || frame == 3 || frame == 8 || frame == 10 || frame == 14 || frame == 17) {
			_audio.playFootstep(1, _random.getRandomBit(), 191, 512);
		}
	}
	if (_roomAction == 461 && _actionStep == 156) {
		_audio.playEffect(29);
	} else if (_roomAction == 470 && _actionStep == 7) {
		_audio.playEffectAt(29, Common::Point(1138, 200));
	}
}

void Scene4060::finishCustomFilm() {
	switch (_museumFilm) {
	case kArrivalFilm:
		if (!_sequence.skipped()) {
			startMuseumFilm(kCliveFilm);
			return;
		}
		// Escape also skips the conversation following the museum arrival.
		// fall through
	case kCliveFilm:
		_entryScene = 4060;
		changeFloor(true, Common::Point(607, 323));
		_audio.playMusic(12, false);
		break;
	case kPhoneFilm:
		_museum.phoneUsed = 1;
		_museum.willyInLab = 1;
		_museum.sawCode = 2;
		if (_museum.keypad == 1) {
			_museum.keypad = 2;
		}
		beginRoomAnimationRange(474, 13, 27, 26, 0);
		break;
	case kMuseumExitFilm:
		leaveMuseumRoom(4100, Common::Point(141, 468));
		break;
	default:
		break;
	}
}

float Scene4060::actorHeight() const {
	const int x = _position.x, y = _position.y;
	const byte zone = walkZone();
	if (_upperFloor) {
		if (x < 69) {
			return x * 1.289855f + 140;
		}
		if (x < 1120) {
			return x * -0.04883463f + 241.64595f;
		}
		if (x < 1474) {
			return x * -0.01977401f + 209.127121f;
		}
		return x * 0.3125f - 280.3125f;
	}
	if (zone == 7 || zone == 8) {
		return y * 0.4370861f - 140.649f;
	}
	if (zone == 12 || zone == 13) {
		return y * 0.29007635f - 33.519085f;
	}
	return (y * 0.4347826f - 118.695656f + x * -0.08982036f + 258.263458f) / 2;
}

Common::U32String Scene4060::hotspotName(byte id) const {
	if (id == 14 && _museum.willyMet) {
		return _strings.text(RunawayStrings::kWilly);
	}
	return RunawayRoomScene::hotspotName(id);
}

void Scene4060::setupActorDepth(RunawayActor::RenderParams &params) const {
	const byte zone = walkZone();
	params.depthY[1] = 1199;
	params.depthY[2] = zone == 1 ? 1199 : 0;
	params.depthY[3] = zone == 13 || zone == 15 || zone == 16 || zone == 17 ? 1199 : 0;
	params.depthY[4] = zone == 19 || zone == 8 || zone == 7 ? 1199 : 0;
	params.depthY[5] = zone == 17 || zone == 18 ? 1199 : 0;
}

bool Scene4060::usesAlternateActor() const {
	return false;
}

int Scene4060::sceneViewportYOffset() const {
	return _upperFloor ? 0 : CLIP<int>(_position.y - 400, 200, _background.h - kRunawayViewportHeight);
}

} // End of namespace Runaway
} // End of namespace Hollywood
