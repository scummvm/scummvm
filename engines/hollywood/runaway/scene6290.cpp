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
#include "hollywood/runaway/scene6290.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

// 0x4e5bf0 keeps the running motorcycle at a fixed point in the room.
const Common::Point kMotorcycleSoundPosition(1081, 200);

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{0, 537, 5, {301, 302}, {0, 3}, {-100, 537}, 7}, // 1: Crater.
	{1739, 442, 3, {303, 304}, {0, 3}, {1839, 442}, 5}, // 2: Trail.
	{440, 556, 7, {305, 306}, {0, 1}, {479, 468}}, // 3: Panel of buttons.
	{607, 549, 7, {307, 308}, {0, 0}, {612, 506}}, // 4: Switch.
	{1133, 434, 0, {309, 310}, {0, 0}, {1068, 422}}, // 5: Motorcycle.
	{1396, 293, 3, {311, 312}, {0, 0}, {1485, 336}}, // 6: Camping tent.
	{0, 0, 7, {313, 314}, {0, 0}, {201, 442}}, // 7: Lighting panel.
	{440, 556, 7, {315, 316}, {0, 1}, {293, 508}, 3}, // 8: Joshua.
	{0, 0, 7, {317, 318}, {0, 0}, {1068, 422}}, // 9: Strap.
	{0, 0, 7, {319, 320}, {0, 0}, {1095, 395}}, // 10: Wrench.
	{0, 0, 0, {321, 322}, {0, 0}, {547, 500}}, // 11: Machine.
	{0, 0, 0, {1, 1}, {0, 0}, {0, 0}},
	{0, 0, 7, {330, 331}, {0, 0}, {669, 500}}, // 13: Rotor.
	{0, 0, 0, {1, 1}, {0, 0}, {0, 0}}
};

const RunawayResponse kResponses[] = {
	{6290, 301, 1, 0},   // Look down into the crater.
	{6290, 303, 2, 0},   // Look at the trail back to town.
	{6290, 305, 3, 0},   // Examine the musical keyboard.
	{6290, 307, 5, 0},   // Look at the machine's switch.
	{6290, 311, 9, 0},   // Look at Joshua's tent.
	{6290, 314, 12, 0},  // Leave the lighting panel alone.
	{6290, 318, 15, 0},  // Leave the delivered strap for Joshua.
	{6290, 319, 16, 0},  // Examine the delivered wrench.
	{6290, 320, 15, 0},  // Leave the delivered wrench for Joshua.
	{6290, 322, 18, 0},  // Refuse to move the machine.
	{6290, 326, 23, 0},  // Do not give Joshua the gasoline.
	{6290, 327, 24, 0},  // Do not pour concentrated gasoline into the motorcycle.
	{6290, 330, 26, 0},  // Identify the rotor's purpose.
	{6290, 331, 27, 0},  // Turning the rotor by hand would not help.
	{6290, 332, 28, 0}   // Let Joshua use the wrench.
};

const RunawayItemAction kItemActions[] = {
	{6290, 10, 5, 332, false}, {6290, 10, 8, 324},
	{6290, 11, 5, 323, false}, {6290, 11, 8, 325}, {6290, 11, 13, 323, false},
	{6290, 12, 5, 327, false}, {6290, 12, 8, 326, false},
	{6290, 24, 5, 328}, {6290, 24, 8, 326, false},
	{6290, 25, 5, 329, false}, {6290, 26, 13, 333, false}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6290 = {
	6290, 1740, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {80, 70, 70}, {70, 60, 60}, {60, 50, 50}}, Common::Point(1739, 442),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene6290::Scene6290(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

const char *Scene6290::actionCaptionVariant(byte id, bool look, byte item) const {
	if (!look && !item) {
		if (id == 4 && _town.joshuaGone) {
			return "raised";
		}
		if (id == 5 && _town.machinePowered && !_town.joshuaGone) {
			return "running";
		}
	}
	return nullptr;
}

bool Scene6290::loadCustomResources() {
	return _joshuaAnimation.load(_archive, 10, 131) && _machineAnimation.load(_archive, 5, 52) &&
		_lightsAnimation.load(_archive, 6, 43) && _machineAmbientAnimation.load(_archive, 7, 6) &&
		_machineOverlayAnimation.load(_archive, 8, 31) && _machineIndicator.load(_archive, 12, 2) &&
		loadForeground(0, 4) && loadForeground(1, 11);
}

bool Scene6290::applyCustomBackgroundPatches() {
	return (_town.strapDelivered != 1 || applyPatch(13)) && (_town.wrenchDelivered != 1 || applyPatch(14));
}

void Scene6290::prepareCustomGameplayLoop() {
	_joshuaTick = _now;
}

void Scene6290::resumeAmbientAudio(bool entering) {
	if (_town.machinePowered && !_town.joshuaGone) {
		_audio.playEffectAt(78, kMotorcycleSoundPosition, 178, 1, true);
	}
	if (entering || !_audio.musicPlaying()) {
		_audio.playMusic(_town.joshuaGone ? 30 : 19);
	}
}

void Scene6290::runCustomEntrySequence() {
	if (_entryScene == 6290) {
		_position = Common::Point(0, 537);
		_walkPath.reset(_position);
		walkTo(Common::Point(440, 556), 7);
		if (_town.joshuaGone) {
			say(0, 31);
		}
	} else {
		walkTo(Common::Point(1363, 484), 5);
	}
	if (!_town.joshuaMet) {
		deferTownAction(400);
	} else if ((_town.strapDelivered == 1 && !_town.wrenchNeeded) || _town.strapDelivered == 2) {
		deferTownAction(420);
	}
}

void Scene6290::advanceCustomGameplayLoop(uint32 delta) {
	if (_keyboard && _pressedNote) {
		if (!_noteSoundPlayed && _now - _noteStarted >= 18 * 80) {
			_audio.playMusicEffect(30 + _pressedNote);
			_noteSoundPlayed = true;
		}
		if (_now - _noteStarted >= 27 * 80) {
			_pressedNote = 0;
			if (_noteCount == ARRAYSIZE(_notes)) {
				leaveKeyboard(true);
			}
		}
	}
	if (_town.joshuaGone || _joshuaDown || _joshuaAnimating || _joshuaChunk != 10) {
		return;
	}
	if (drawingRoomAnimation() || _now - _joshuaTick < 80) {
		return;
	}
	_joshuaTick = _now;
	if (_joshuaTalking) {
		if (_joshuaFrame > 6 || speaker()) {
			_joshuaFrame = 6;
		} else if (_joshuaFrame < 6) {
			++_joshuaFrame;
		}
	} else if (_joshuaFrame > 11) {
		if (_joshuaFrame == 99 || _joshuaFrame == 130) {
			_joshuaFrame = 0;
		} else {
			++_joshuaFrame;
		}
	} else if (_joshuaFrame) {
		--_joshuaFrame;
	} else if (!_random.getRandomNumber(49)) {
		_joshuaFrame = _random.getRandomBit() ? 12 : 100;
	}
}

void Scene6290::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[i] == 3 && !_town.joshuaGone) {
			_maps[i] = 0; // Joshua occupies the approach to the machine.
		}
		switch (_originalMaps[256 + i]) {
		case 1: // The crater floor becomes accessible after Joshua's departure.
			_maps[256 + i] = _town.joshuaGone ? 1 : 0;
			break;
		case 3: // The keyboard becomes usable once the motorcycle powers it.
			_maps[256 + i] = _town.machinePowered ? 3 : 11;
			break;
		case 4:
			_maps[256 + i] = _town.joshuaGone ? 4 : 11;
			break;
		case 14:
			_maps[256 + i] = _town.machinePowered && !_town.joshuaGone ? 4 : 11;
			break;
		case 8:
			_maps[256 + i] = _town.joshuaGone ? 0 : 8;
			break;
		case 9:
			_maps[256 + i] = _town.strapDelivered == 1 ? 9 : 5;
			break;
		case 10:
			_maps[256 + i] = _town.wrenchDelivered == 1 ? 10 : 0;
			break;
		case 12:
			_maps[256 + i] = _town.joshuaGone ? 7 : 8;
			break;
		case 13:
			_maps[256 + i] = _town.rotorKnown ? 13 : 11;
			break;
		default:
			break;
		}
	}
}

float Scene6290::actorHeight() const {
	if (_position.x < 1713 && _position.y < 356) {
		return _position.y * 1.75757575f - 259.69696f;
	}
	return (_position.x * -0.0231607631f + 425.276581f +
		_position.y * 0.440251559f + 191.289307f) / 2;
}

void Scene6290::findWalkPath(Common::Point target) {
	walkTown(target);
}

Common::U32String Scene6290::hotspotName(byte id) const {
	if (_keyboard) {
		return actionCaption(id, kVerbUse, 0);
	}
	if (id == 11 && _town.machineKnown) {
		return _strings.hotspotName(_room, 14);
	}
	return RunawayRoomScene::hotspotName(id);
}

void Scene6290::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_position.x > 1380) {
		drawForeground(viewport, 0);
	}
	if (_position.y < 356) {
		drawForeground(viewport, 1);
	}
}

void Scene6290::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (!_town.joshuaGone && !_joshuaDown && !(_joshuaAnimating && drawingRoomAnimation())) {
		_joshuaAnimation.draw(viewport, _joshuaTalking && speaker() ? 6 + speechFrame() % 6 : _joshuaFrame, _camera.x, 0);
	}
	const bool running = _town.machinePowered && !_town.joshuaGone;
	_machineAnimation.draw(viewport, running ? 4 + (_now / 80) % 46 : _town.strapDelivered >= 2 ? 1 : 0, _camera.x, 0);
	_machineOverlayAnimation.draw(viewport, running ? 6 + (_now / 80) % 24 : 0, _camera.x, 0);
	_machineAmbientAnimation.draw(viewport, running ? (_now / 80) % 4 : 0, _camera.x, 0);
	_machineIndicator.draw(viewport, running, _camera.x, 0);
	if (_keyboard) {
		_keyboardPlatform.draw(viewport, 1, _camera.x, 0);
		uint brian = 0, button = 0, light = 0;
		if (_pressedNote) {
			const uint note = _pressedNote - 1;
			const uint step = MIN<uint>(26, (_now - _noteStarted) / 80);
			if (step < 10) {
				brian = step;
			} else if (step < 18) {
				brian = note ? 27 + (note - 1) * 14 + MIN<uint>(6, step - 10) : MIN<uint>(17, step);
			} else {
				brian = note ? 34 + (note - 1) * 14 + MIN<uint>(6, step - 18) : step;
			}
			if (step >= 15 && step < 18) {
				button = note * 4 + step - 14;
			}
			if (step >= 18 && step < 24) {
				light = note * 6 + step - 17;
			}
		}
		_lightsAnimation.draw(viewport, light, _camera.x, 0);
		_keyboardBrian.draw(viewport, brian, _camera.x, 0);
		_keyboardButtons.draw(viewport, button, _camera.x, 0);
	}
}

void Scene6290::setJoshuaAnimation(uint chunk, uint frames) {
	if (_joshuaChunk != chunk && !_joshuaAnimation.load(_archive, chunk, frames)) {
		error("Cannot load Runaway Joshua animation %u", chunk);
	}
	_joshuaChunk = chunk;
	_joshuaAnimating = false;
	_joshuaFrame = 0;
}

bool Scene6290::canSave() const {
	return !_keyboard && RunawayRoomScene::canSave();
}

bool Scene6290::busy() const {
	return _pressedNote || RunawayRoomScene::busy();
}

bool Scene6290::shouldDrawActor() const {
	return !_keyboard;
}

bool Scene6290::shouldUpdateActor() const {
	return !_keyboard;
}

byte Scene6290::hotspotAt(Common::Point mouse) const {
	if (!_keyboard) {
		return RunawayRoomScene::hotspotAt(mouse);
	}
	if (mouse.x < 0 || mouse.x >= kRunawayScreenWidth || mouse.y < kRunawayViewportTop ||
			mouse.y >= kRunawayViewportTop + kRunawayViewportHeight) {
		return 0;
	}
	const int x = mouse.x + _camera.x, y = mouse.y - kRunawayViewportTop + _camera.y;
	if (x < 0 || x >= _background.w || y < 0 || y >= _background.h) {
		return 0;
	}
	const byte id = _keyboardMaps[256 + _keyboardMask[y * _background.w + x]];
	for (uint i = 0; i < _noteCount; ++i) {
		if (id == 20 + _notes[i]) {
			return 29; // The note is labelled but cannot be pressed again.
		}
	}
	return id;
}

Common::U32String Scene6290::actionCaption(byte id, RunawayVerb verb, byte item) const {
	if (_keyboard) {
		return _strings.sceneActionCaption(_room, id, 0, false);
	}
	return RunawayRoomScene::actionCaption(id, verb, item);
}

void Scene6290::click(Common::Point mouse, RunawayVerb verb, byte item) {
	if (!_keyboard) {
		RunawayRoomScene::click(mouse, verb, item);
		return;
	}
	if (busy() || _pressedNote) {
		return;
	}
	const byte id = hotspotAt(mouse);
	if (id == 27) { // Leave the musical keyboard without completing an attempt.
		leaveKeyboard(false);
	} else if (id >= 20 && id <= 26 && _noteCount < ARRAYSIZE(_notes)) {
		_pressedNote = id - 19;
		_notes[_noteCount++] = id - 20;
		_noteStarted = _now;
		_noteSoundPlayed = false;
	}
}

void Scene6290::startKeyboard() {
	if (_keyboardMask.empty() &&
			(!_keyboardBrian.load(_archive, 20, 109) || !_keyboardButtons.load(_archive, 21, 29) ||
			!_keyboardPlatform.load(_archive, 9, 2) ||
			!loadRunawayMask(_archive, _background.w, _background.h, _keyboardMask, _keyboardMaps, 22))) {
		error("Cannot load Runaway musical keyboard");
	}
	_keyboard = true;
	_joshuaDown = true;
	_noteCount = _pressedNote = 0;
	_inventory.select(0);
}

void Scene6290::leaveKeyboard(bool review) {
	_keyboard = false;
	_pressedNote = 0;
	_reviewingMelody = review;
	static const byte combination[] = {0, 4, 2, 6, 5};
	if (review && !memcmp(_notes, combination, sizeof(_notes))) {
		reviewMelody();
		return;
	}
	if (review && !_town.melodyTried) {
		say(30, 1);
	}
	_joshuaAnimating = true;
	beginRoomAnimation(451, 26, 38, nullptr, 0, false);
}

void Scene6290::reviewMelody() {
	static const byte combination[] = {0, 4, 2, 6, 5};
	uint correctNotes = 0, correctPositions = 0;
	for (uint i = 0; i < ARRAYSIZE(_notes); ++i) {
		if (_notes[i] == combination[i]) {
			++correctPositions;
		}
		for (uint j = 0; j < ARRAYSIZE(combination); ++j) {
			if (_notes[i] == combination[j]) {
				++correctNotes;
				break;
			}
		}
	}
	if (correctPositions == 5) {
		_joshuaDown = true;
		say(30, 20);
		deferTownAction(460);
		return;
	}
	_town.melodyTried = 1;
	_joshuaTalking = true;
	say(30, correctNotes - 1, false, 1);
	const byte positionCue = correctPositions < 3 ? 5 + correctPositions : correctPositions == 3 ?
		(correctNotes == 3 ? 9 : 8) : (correctNotes == 5 ? 10 : 11);
	say(30, positionCue, false, 1);
	static const byte filmCombination[] = {5, 6, 4, 0, 1};
	if (!memcmp(_notes, filmCombination, sizeof(_notes))) {
		say(30, 19, false, 1);
	}
	say(30, correctNotes + correctPositions < 5 ? 12 : correctNotes + correctPositions < 8 ? 13 : 14, false, 1);
	startConversation(9);
	_dialogue[0][0] = {1, 0, 0, 15, 17, 1, 0};
	_dialogue[0][1] = {1, 0, 0, 16, 18, 2, 0};
}

uint16 Scene6290::dialoguePlayerCue() const {
	return _conversation == 9 ? 30 : 80;
}

uint16 Scene6290::dialogueResponseCue() const {
	return _conversation == 9 ? 30 : 90;
}

void Scene6290::finishCustomConversation() {
	_joshuaTalking = false;
}

int8 Scene6290::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (use && !item) {
		if (id == 3) {
			return !_town.joshuaGone;
		}
		if (id == 6) {
			return _town.joshuaGone != 0;
		}
	}
	return approach;
}

RunawayRoomScene::ActionResult Scene6290::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Visit the crater floor after Joshua disappears.
		leaveTownRoom(6280);
		break;
	case 306: // Ask Joshua to listen while Brian tries the musical keyboard again.
		if (!_town.machinePowered || _town.joshuaGone) {
			say(4, 1);
		} else {
			_joshuaTalking = true;
			say(4);
			say(80, 8);
			say(90, 8, false, 1);
			deferTownAction(450);
		}
		break;
	case 308: // The switch stays on while Joshua works on the combination.
		say(6, !_town.machinePowered || _town.joshuaGone);
		break;
	case 312: // Search Joshua's tent once he has gone down into the crater.
		if (!_town.joshuaGone) {
			say(10);
		} else {
			say(10, _town.campingGearTaken ? 4 : 1);
			beginRoomAnimation(470, 24, 34);
		}
		break;
	case 1470: // Find the mining light and rope in the tent.
		if (!_town.campingGearTaken) {
			say(10, 2);
			_inventory.add(18);
			_inventory.add(19);
			_town.campingGearTaken = 1;
			say(10, 3);
		} else {
			say(10, 5);
		}
		holdRoomAnimation(471, 24, 34, 33);
		break;
	case 1471: // Withdraw from the tent with the collected equipment.
		beginRoomAnimationRange(472, 24, 34, 33, 0);
		break;
	case 1472: // Finish searching the tent.
		break;
	case 304: // Return to the overview; Joshua works while Brian is away.
		if (_town.strapDelivered == 1 && _town.wrenchDelivered == 1) {
			_town.strapDelivered = 2;
			_town.wrenchDelivered = 2;
		}
		_town.campLeft = 1;
		_audio.playMusic(12 + _random.getRandomNumber(4));
		leaveTownRoom(6350);
		break;
	case 309: // Examine the motorcycle before or after the repair.
		say(7, _town.joshuaGone ? 2 : _town.machinePowered ? 3 : _town.fuelNeeded);
		break;
	case 310: // Try to start the motorcycle, or explain why Brian cannot take it.
		if (_town.joshuaGone) {
			say(8, 2);
			deferTownAction(433);
		} else {
			say(8, _town.machinePowered ? 4 : _town.fuelNeeded);
		}
		break;
	case 433: // Brian admits that he does not know how to ride a motorcycle.
		_facing = 4;
		say(8, 3);
		break;
	case 315: // Look at Joshua.
		say(13, _random.getRandomNumber(2));
		break;
	case 313: // Describe the colored lights after trying the musical keyboard.
		say(11, _town.melodyTried);
		break;
	case 329: // The thirty-CC tube cannot fuel the motorcycle directly.
	case 333: // Do not pour the fifty-CC tube's concentrate into the rotor.
		if (action == 329 ? !_inventory.town().tube30 : !_inventory.town().tube50) {
			say(19, 0, true);
		} else {
			say(24);
		}
		break;
	case 316: // Talk to Joshua, or give him the wrench he requested.
		if (_town.strapDelivered == 1 && (!_town.wrenchNeeded || _town.wrenchDelivered)) {
			say(22);
		} else if (_town.strapDelivered == 1 && _town.wrenchNeeded && _inventory.contains(10)) {
			dispatchCustomSceneAction(324);
		} else {
			startJoshuaConversation(false);
		}
		break;
	case 317: // Look at the strap waiting to be fitted.
		say(14, _town.wrenchNeeded && !_inventory.contains(10) && !_town.wrenchDelivered);
		break;
	case 321: // Describe the machine and whether it is still running.
		say(17, _town.joshuaGone ? 3 : _town.machinePowered ? 2 : _town.machineKnown);
		break;
	case 323: // Work out how the strap could drive the rotor.
		if (!_town.rotorKnown) {
			say(19);
		} else if (_inventory.town().strap != 2) {
			say(19, _town.strapIdea ? 2 : 1);
			_town.strapIdea = 1;
			_inventory.town().strap = 1;
		} else {
			say(19, 3);
			deferTownAction(423);
		}
		break;
	case 423: // Brian decides to leave the assembly to Joshua.
		_facing = 4;
		say(19, 4);
		break;
	case 324: // Give Joshua the number ten wrench.
		_inventory.select(0);
		_joshuaTalking = true;
		say(20);
		say(20, 1, false, 1);
		say(20, 2);
		say(20, 3, false, 1);
		say(20, 4);
		deferTownAction(424);
		break;
	case 424: // Approach the motorcycle to put down the wrench.
		_joshuaTalking = false;
		walkTo(Common::Point(1133, 434), 7, false, true);
		deferTownAction(425);
		break;
	case 425: // Put the wrench beside the strap.
		_inventory.remove(10);
		_town.wrenchDelivered = 1;
		walkTo(Common::Point(512, 568), 5, false, true);
		deferTownAction(426);
		return kBackgroundChanged;
	case 426: // Comment after leaving the wrench for Joshua.
		say(20, 5);
		break;
	case 325: // Explain the drive belt idea and give Joshua the joined strap.
		if (!_town.rotorKnown) {
			say(25, 0, true);
		} else if (_inventory.town().strap != 2) {
			say(19, 2);
			_inventory.town().strap = 1;
		} else {
			_joshuaTalking = true;
			say(20);
			say(20, 1, false, 1);
			for (uint i = 0; i <= 6; ++i) {
				say(21, i, false, i % 2);
			}
			deferTownAction(430);
		}
		break;
	case 430: // Approach the motorcycle to put down the joined strap.
		_joshuaTalking = false;
		walkTo(Common::Point(1133, 434), 7, false, true);
		deferTownAction(431);
		break;
	case 431: // Put the strap beside the motorcycle.
		_inventory.select(0);
		_inventory.remove(11);
		_town.strapDelivered = 1;
		walkTo(Common::Point(512, 568), 5, false, true);
		deferTownAction(432);
		return kBackgroundChanged;
	case 432: // Comment after leaving the strap for Joshua.
		say(21, 7);
		break;
	case 328: // Only correctly diluted gasoline can run the motorcycle.
		if (!_inventory.town().bottleContents) {
			say(19, 1, true);
		} else if (_inventory.town().bottleContents == 1) {
			say(25);
		} else if (_town.machinePowered) {
			say(25, _town.joshuaGone ? 3 : 1);
		} else {
			say(25, 4);
			walkTo(Common::Point(1133, 434), 7, true);
			deferTownAction(440);
		}
		break;
	case 440: { // Fill the motorcycle's tank with the diluted gasoline.
		byte frames[120];
		for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
			frames[i] = i < 40 ? i : i < 80 ? 39 : 119 - i;
		}
		beginRoomAnimation(440, 28, 40, frames, ARRAYSIZE(frames));
		break;
	}
	case 1440: // Tell Joshua the motorcycle is ready.
		_inventory.town().bottleContents = 0;
		_inventory.select(0);
		say(25, 5);
		walkTo(Common::Point(440, 556), 7, true);
		deferTownAction(441);
		break;
	case 441: // Joshua explains how Brian should operate the switch.
		_joshuaTalking = true;
		say(20);
		say(20, 1, false, 1);
		for (uint i = 6; i <= 9; ++i) {
			say(25, i, false, i % 2);
		}
		deferTownAction(442);
		break;
	case 442: // Joshua walks to the motorcycle while Brian approaches the switch.
		_joshuaTalking = false;
		walkTo(Common::Point(607, 549), 7, true);
		_joshuaAnimating = true;
		beginRoomAnimation(442, 29, 113, nullptr, 0, false);
		break;
	case 1442: // Start the motorcycle and engage the machine.
		setJoshuaAnimation(29, 113);
		_joshuaFrame = 112;
		_audio.playEffect(77, 178, 1);
		beginRoomAnimation(443, 27, 27);
		break;
	case 1443: // The repaired communication machine is powered.
		_town.machinePowered = 1;
		_audio.playEffectAt(78, kMotorcycleSoundPosition, 178, 1, true);
		say(25, 10);
		_joshuaAnimating = true;
		beginRoomAnimationRange(444, 30, 189, 0, 150, false);
		return kBackgroundChanged;
	case 1444: // Joshua stops to call out to Brian on the way back.
		say(25, 11, false, 1);
		holdRoomAnimation(445, 30, 189, 150, false);
		break;
	case 1445: // Joshua returns to his place by the keyboard.
		beginRoomAnimationRange(446, 30, 189, 151, 188, false);
		break;
	case 1446: // Joshua explains the five-note combination and the rule against repeats.
		setJoshuaAnimation(10, 131);
		_joshuaTalking = true;
		for (uint i = 12; i <= 18; ++i) {
			say(25, i, false, !(i % 2));
		}
		deferTownAction(450);
		break;
	case 450: // Joshua goes down into the crater to listen for telepathic signals.
		_joshuaTalking = false;
		_joshuaAnimating = true;
		beginRoomAnimation(450, 25, 46, nullptr, 0, false);
		break;
	case 1450: // Brian takes his place at the keyboard.
		_joshuaDown = true;
		walkTo(Common::Point(419, 527), 1, true);
		say(30, 0, false, 1);
		deferTownAction(452);
		break;
	case 452: // Enable the seven musical keys and clear the previous attempt.
		startKeyboard();
		break;
	case 1451: // Joshua returns to review the melody, or agrees to stop for now.
		setJoshuaAnimation(10, 131);
		_joshuaDown = false;
		_position = Common::Point(440, 556);
		_walkPath.reset(_position);
		_facing = 7;
		if (_reviewingMelody) {
			reviewMelody();
		} else {
			_joshuaTalking = true;
			say(30, 16);
			say(30, 18, false, 1);
			deferTownAction(410);
		}
		break;
	case 460: // Approach the crater as the dimensional door opens.
		_audio.playMusic(23, false);
		walkTo(Common::Point(212, 471), 0, true);
		deferTownAction(461);
		break;
	case 461: // Joshua says farewell from the light in the crater.
		say(30, 21);
		say(30, 22, false, 1);
		say(30, 23);
		deferTownAction(462);
		break;
	case 462: { // Play the dimensional-door film.
		typedef RunawaySequence S;
		static const S::Command film[] = {
			{S::kOpen, 3}, {S::kPlay, 0, 265}, {S::kEnd}
		};
		_audio.stopEffect(1);
		_sequence.start(film, _text, _now, 'F');
		break;
	}
	case 463: // Follow Joshua into the crater after the light disappears.
		_town.joshuaGone = 1;
		_audio.playMusic(30);
		say(30, 24);
		leaveTownRoom(6280);
		break;
	case 400: // Call to Joshua from the trail.
		say(0);
		deferTownAction(401);
		break;
	case 401: // Approach Joshua for the first meeting.
		walkTo(Common::Point(440, 556), 7, false, true);
		deferTownAction(402);
		break;
	case 402: // Call to Joshua again after approaching him.
		say(0, 1);
		deferTownAction(403);
		break;
	case 403: // Joshua finishes receiving a telepathic message.
		_joshuaAnimating = true;
		beginRoomAnimationRange(403, 10, 131, 12, 99, false);
		break;
	case 1403: // Joshua turns to speak to Brian.
		beginRoomAnimationRange(404, 10, 131, 0, 6, false);
		break;
	case 1404: // Begin the first exchange with Joshua.
		_joshuaFrame = 6;
		_joshuaAnimating = false;
		_joshuaTalking = true;
		say(0, 4, false, 1);
		say(0, 5);
		deferTownAction(405);
		break;
	case 405: // Play Joshua's gesture during the introduction.
		_joshuaAnimating = true;
		beginRoomAnimationRange(405, 10, 131, 100, 130, false);
		break;
	case 1405: // Joshua introduces Trantor and the communication machine.
		_joshuaAnimating = false;
		for (uint i = 6; i <= 18; ++i) {
			say(0, i, false, !(i % 2));
		}
		_town.joshuaMet = 1;
		deferTownAction(406);
		break;
	case 406: // Start the introductory conversation with Joshua.
		startJoshuaConversation(true);
		break;
	case 410: // Joshua resumes his idle animation after a conversation.
		_joshuaTalking = false;
		break;
	case 420: // On returning, discover Joshua needs a wrench or fuel.
		say(0, _town.strapDelivered == 1 ? 19 : 26);
		deferTownAction(421);
		break;
	case 421: // Approach Joshua to ask about the repair.
		walkTo(Common::Point(440, 556), 7, false, true);
		deferTownAction(422);
		break;
	case 422: // Learn whether Joshua needs the wrench or gasoline.
		_joshuaTalking = true;
		say(20);
		say(20, 1, false, 1);
		if (_town.strapDelivered == 1) {
			_town.wrenchNeeded = 1;
			say(0, 20);
			say(0, 21, false, 1);
			say(0, 22);
			deferTownAction(427);
		} else {
			_town.strapDelivered = 3;
			_town.fuelNeeded = 1;
			say(0, 27);
			say(0, 28, false, 1);
			say(0, 29);
			say(0, 30, false, 1);
			deferTownAction(410);
		}
		break;
	case 427: // Brian comments on Joshua's request for a wrench.
		_facing = 4;
		say(0, 23);
		deferTownAction(428);
		break;
	case 428: // Finish discussing the wrench with Joshua.
		_facing = 7;
		say(0, 24);
		say(0, 25, false, 1);
		deferTownAction(410);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene6290::finishCustomFilm() {
	deferTownAction(463);
}

void Scene6290::handleAnimationFrameHook() {
	static const AnimationFootstep descendingSteps[] = {
		{24, 200, 102}, {30, 150, 204}, {37, 100, 102}
	};
	static const AnimationFootstep returningSteps[] = {
		{3, 50, 102}, {9, 100, 204}, {17, 150, 102}, {22, 200, 204}
	};
	if (_roomAction == 450) {
		playAnimationFootsteps(_actionStep, 2, descendingSteps, ARRAYSIZE(descendingSteps));
	} else if (_roomAction == 451) {
		playAnimationFootsteps(_actionStep, 2, returningSteps, ARRAYSIZE(returningSteps));
	}
	if (_roomAction == 440) {
		if (_actionStep == 11 || _actionStep == 33 || _actionStep == 106) {
			_audio.playEffect(_actionStep == 11 ? 35 : _actionStep == 33 ? 36 : 37, 204);
		}
	} else if (_roomAction == 443 && _actionStep == 16) {
		_audio.playEffect(38);
	} else if (_roomAction == 470 && _actionStep == 11) {
		_audio.playEffect(39, 204);
	} else if (_roomAction == 472 && _actionStep == 11) {
		_audio.playEffect(40, 204);
	}
}

void Scene6290::startJoshuaConversation(bool introduction) {
	_conversation = 7;
	_dialogueDepth = _dialogueNode = _dialogueIntro = 0;
	_dialogueEnding = false;
	_choices.clear();
	_choiceIndices.clear();
	memset(_dialogue, 0, sizeof(_dialogue));
	_afterConversation = 410;
	_joshuaTalking = true;
	if (!introduction) {
		say(20);
		say(20, 1, false, 1);
	}

	static const DialogueChoice topics[][8] = {
		{
			{1, 0, 3, 0, 0, 1, 255}, {1, 0, 3, 1, 1, 1, 255},
			{0, 0, 1, 15, 15, 1, 255}, {0, 0, 1, 16, 15, 1, 255},
			{0, 1, 3, 6, 6, 5, 255}, {0, 0, 3, 7, 7, 1, 255},
			{0, 0, 0, 8, 8, 11, 255}, {1, 0, 0, 9, 9, 0, 0}
		},
		{
			{0, 0, 3, 2, 2, 6, 255}, {0, 0, 3, 3, 3, 1, 255},
			{0, 0, 1, 4, 4, 7, 255}, {0, 0, 1, 5, 5, 1, 255},
			{1, 0, 2, 17, 17, 0, 0}
		},
		{
			{0, 0, 3, 10, 10, 15, 255}, {0, 0, 3, 11, 11, 8, 255},
			{1, 0, 3, 12, 12, 1, 255}, {1, 0, 3, 13, 13, 1, 255},
			{1, 0, 2, 14, 14, 0, 0}
		}
	};
	for (uint i = 0; i < ARRAYSIZE(topics); ++i) {
		memcpy(_dialogue[i * 10], topics[i], sizeof(topics[i]));
	}
	_dialogue[0][6].enabled = _town.machinePowered && !_town.joshuaGone;
	_dialogue[0][2].enabled = !_town.campLeft;
	_dialogue[0][3].enabled = _town.campLeft;
	_dialogue[0][4].enabled = !_town.helmetKnown;
	_dialogue[0][5].enabled = _town.helmetKnown;
	_dialogue[10][0].enabled = !_town.dimensionalDoorKnown;
	_dialogue[10][1].enabled = _town.dimensionalDoorKnown;
	_dialogue[10][2].enabled = !_town.machineKnown;
	_dialogue[10][3].enabled = _town.machineKnown;
	_dialogue[20][0].enabled = !_town.batteryKnown;
	_dialogue[20][1].enabled = _town.batteryKnown;
}

void Scene6290::handleCustomDialogueChoice(byte effect) {
	if (_conversation == 9) {
		_afterConversation = effect == 1 ? 450 : 410;
		return;
	}
	switch (effect) {
	case 5: // Joshua explains his telepathic helmet.
		say(90, 7, false, 1);
		_town.helmetKnown = 1;
		break;
	case 6:
		_town.dimensionalDoorKnown = 1;
		break;
	case 7:
		_town.machineKnown = 1;
		break;
	case 8: // Brian can now suggest driving the rotor with the motorcycle.
		_town.rotorKnown = 1;
		updateMaps();
		break;
	case 11: // Resume the musical combination puzzle.
		_afterConversation = 450;
		break;
	case 15: // Asking about the broken battery reveals the request for help.
		_town.batteryKnown = 1;
		_dialogue[20][1].enabled = 1;
		break;
	default:
		break;
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
