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
#include "hollywood/runaway/scene4100.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const byte kPressButtonFrames[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

const byte kFreezeBatteryFrames[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	39, 40, 41, 42, 43, 44, 45, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
	46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
	56, 57, 58, 59, 60, 61, 62, 63, 64, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
	16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
};

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{141, 468, 7, {301, 302}, {0, 1}, {43, 471}, 4}, // 1: Door.
	{196, 451, 1, {303, 304}, {0, 0}, {220, 420}}, // 2: First aid kit.
	{310, 457, 0, {305, 306}, {1, 1}, {337, 425}}, // 3: Glass display case.
	{204, 448, 1, {307, 308}, {1, 1}, {270, 421}}, // 4: Button.
	{310, 457, 0, {309, 310}, {1, 0}, {327, 425}}, // 5: Mayan mask.
	{906, 470, 0, {311, 312}, {1, 1}, {912, 400}}, // 6: Scanner.
	{1352, 431, 1, {315, 316}, {0, 0}, {1400, 395}}, // 7: Tank.
	{111, 446, 1, {313, 314}, {1, 1}, {136, 421}}, // 8: Golden object.
	{539, 439, 1, {317, 318}, {0, 0}, {576, 400}}, // 9: Device.
	{1329, 409, 1, {328, 329}, {1, 1}, {1381, 381}}, // 10: Ladle.
};

const RunawayResponse kResponses[] = {
	{kMuseumSecurity, 301, 1, 0}, // Look at the security door.
	{kMuseumSecurity, 303, 2, 0}, // Look at the first aid kit.
	{kMuseumSecurity, 304, 3, 0}, // Open the first aid kit.
	{kMuseumSecurity, 307, 6, 0}, // Look at the chamber button.
	{kMuseumSecurity, 312, 11, 0}, // Use the scanner.
	{kMuseumSecurity, 316, 15, 0}, // Use the liquid nitrogen tank.
	{kMuseumSecurity, 318, 17, 0}, // Use the electron microscope.
	{kMuseumSecurity, 319, 18, 0}, // Put an item in the thermal chamber.
	{kMuseumSecurity, 322, 21, 0}, // Use an item with the scanner.
	{kMuseumSecurity, 323, 11, 0}, // Use the crucifix with the scanner.
	{kMuseumSecurity, 324, 22, 0}, // Put an item in the liquid nitrogen.
	{kMuseumSecurity, 326, 24, 0}, // Use an item with the electron microscope.
	{kMuseumSecurity, 327, 25, 0}, // Examine the crucifix with the electron microscope.
	{kMuseumSecurity, 328, 26, 0}, // Look at the ladle.
};

const RunawayItemAction kItemActions[] = {
	{kMuseumSecurity, 5, 3, 321, false}, // Play the recording to the voice lock.
	{kMuseumSecurity, 11, 3, 320, false}, // Put the Mayan mask back in the chamber.
	{kMuseumSecurity, 0, 3, 319, false}, // Put an item in the thermal chamber.
	{kMuseumSecurity, 5, 4, 321}, // Play the recording to the voice lock.
	{kMuseumSecurity, 7, 6, 323, false}, // Use the crucifix with the scanner.
	{kMuseumSecurity, 0, 6, 322, false}, // Use an item with the scanner.
	{kMuseumSecurity, 6, 7, 330, false}, // Put the battery directly in the liquid nitrogen.
	{kMuseumSecurity, 18, 7, 325}, // Freeze the battery in the ladle.
	{kMuseumSecurity, 0, 7, 324, false}, // Put an item in the liquid nitrogen.
	{kMuseumSecurity, 7, 9, 327, false}, // Examine the crucifix with the electron microscope.
	{kMuseumSecurity, 0, 9, 326, false}, // Use an item with the electron microscope.
};

const RunawayExit kExits[] = {
	{kMuseumSecurity, 302, kMuseumHall, 1118, 309, true, false} // Return to the exhibition hall.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene4100 = {
	kMuseumSecurity, 1536, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {40, 40, 60}, {30, 30, 50}, {20, 20, 40}},
	Common::Point(141, 468),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	kExits, ARRAYSIZE(kExits)
};

namespace Runaway {

Scene4100::Scene4100(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

const char *Scene4100::actionCaptionVariant(byte id, bool look, byte item) const {
	if (!look && !item && id == 3) {
		return _museum.chamberOpen ? "open" : nullptr;
	}
	return nullptr;
}

bool Scene4100::loadCustomResources() {
	return !museumChapter() || (_ambientAnimation.load(_archive, 4, 121) && _overlayAnimation.load(_archive, 5, 121) &&
		_leftMachineAnimation.load(_archive, 12, 42) && _rightMachineAnimation.load(_archive, 20, 45) &&
		_computerAnimation.load(_archive, 9, 49) && _rejectionLight.load(_archive, 11, 7));
}

bool Scene4100::applyCustomBackgroundPatches() {
	if (!museumChapter()) {
		return true;
	}
	if (!applyPatch((_museum.chamberOpen ? 15 : 13) + (_museum.maskTaken ? 1 : 0))) {
		return false;
	}
	if (_museum.keyTaken && !applyPatch(6)) {
		return false;
	}
	if (_museum.ladleTaken && !applyPatch(8)) {
		return false;
	}
	return true;
}

void Scene4100::applyCustomHotspotState() {
	if (!museumChapter()) {
		return;
	}
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (id == 4 && (!_museum.buttonSeen || _museum.chamberOpen)) {
			id = 3;
		} else if (id == 5 && (!_museum.cabinetSeen || _museum.maskTaken)) {
			id = 3;
		} else if ((id == 8 && _museum.keyTaken) || (id == 10 && _museum.ladleTaken)) {
			id = 0;
		}
		if (_maps[i] == 4 || _maps[i] == 5) {
			_maps[i] = 0;
		}
	}
}

void Scene4100::runCustomEntrySequence() {
	if (_inventory.museum().restoredCrucifix) {
		walkTo(Common::Point(906, 470), 0, true);
		deferMuseumAction(409);
	} else {
		beginRoomAnimationRange(412, 21, 13, 12, 0, false);
		_audio.playEffect(32);
	}
}

void Scene4100::resumeAmbientAudio(bool entering) {
	if (entering || !_audio.musicPlaying()) {
		_audio.playMusic(11, false);
	}
}

void Scene4100::advanceCustomGameplayLoop(uint32 delta) {
	if (_now - _machineTick >= 80) {
		_machineTick = _now;
		_computerFrame = speaker() && !_cliveVisible ? _random.getRandomNumber(48) : 48;
		for (uint i = 0; i < 2; ++i) {
			if (_museum.chamberOpen) {
				_machinePosition[i] = MIN<uint>(i ? 405 : 512, _machinePosition[i] + 1);
			} else if (_machinePosition[i] > 5) {
				--_machinePosition[i];
			} else if (!_random.getRandomNumber(_machinePosition[i] ? 9 : 49)) {
				_machinePosition[i] = _machinePosition[i] ? 0 : i ? 5 : 4;
			}
		}
	}
	if (_museum.chamberTime) {
		_museum.chamberTime -= MIN<uint>(delta, _museum.chamberTime);
		if (!_museum.chamberTime && !_roomAction) {
			closeChamber();
		}
	}
	if (_museum.chamberOpen && !_museum.chamberTime && !_roomAction) {
		closeChamber();
	}
}

void Scene4100::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (!museumChapter()) {
		return;
	}
	const uint tick = (_now / 80 + 46) % 217;
	const uint frame = tick <= 106 ? 14 + tick : 216 - tick;
	_ambientAnimation.draw(viewport, frame, _camera.x, 0);
	_overlayAnimation.draw(viewport, frame, _camera.x, 0);
	static const byte durations[][45] = {
		{3, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12,
		 13, 13, 14, 14, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23},
		{3, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9,
		 10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16}
	};
	for (uint machine = 0; machine < 2; ++machine) {
		uint position = _machinePosition[machine], index = 0;
		const uint count = machine ? 45 : 42;
		while (index + 1 < count && position >= durations[machine][index]) {
			position -= durations[machine][index++];
		}
		(machine ? _rightMachineAnimation : _leftMachineAnimation).draw(viewport, index, _camera.x, 0);
	}
	if (_cliveVisible) {
		_cliveAnimation.draw(viewport, 17 + (speaker() ? speechFrame() : 0), _camera.x, 0);
	}
	_computerAnimation.draw(viewport, _computerFrame, _camera.x, 0);
	if (_computerReject) {
		_rejectionLight.draw(viewport, 6, _camera.x, 0);
	}
}

RunawayRoomScene::ActionResult Scene4100::dispatchCustomSceneAction(uint16 action) {
	if (!museumChapter()) {
		return kUnhandled;
	}
	switch (action) {
	case 302: // Open the security door to return to the hall.
		beginRoomAnimation(450, 21, 13, nullptr, 0, false);
		_audio.playEffectAt(32, Common::Point(43, 200));
		return kHandled;
	case 305: // Look inside the thermal chamber.
		say(4, !_museum.cabinetSeen ? 0 : _museum.maskTaken ? 2 : 1);
		_museum.cabinetSeen = 1;
		return kBackgroundChanged;
	case 306: // Try to open the thermal chamber.
		say(5, _museum.buttonSeen);
		_museum.buttonSeen = 1;
		return kBackgroundChanged;
	case 308: // Press the chamber button and try voice identification.
		beginRoomAnimation(308, 29, 12, kPressButtonFrames, ARRAYSIZE(kPressButtonFrames));
		return kHandled;
	case 309: // Look at the Mayan mask.
		say(8, _museum.chamberOpen != 0);
		return kHandled;
	case 310: // Take the Mayan mask.
		if (!_museum.chamberOpen) {
			say(9);
		} else {
			say(9, 1);
			_museum.maskTaken = 1;
			_inventory.add(11);
			say(9, 2);
			deferMuseumAction(432);
		}
		return kBackgroundChanged;
	case 311: // Look at the scanner.
		say(10, _museum.scannerSeen);
		_museum.scannerSeen = 1;
		return kHandled;
	case 313: // Look at the strange key.
		say(12, _museum.keySeen);
		_museum.keySeen = 1;
		return kHandled;
	case 314: // Take the strange key.
		if (!_museum.keySeen) {
			say(12);
		}
		_museum.keySeen = _museum.keyTaken = 1;
		_inventory.add(17);
		say(13);
		say(13, 1);
		say(13, 2);
		return kBackgroundChanged;
	case 315: // Look at the liquid nitrogen tank.
		say(14, _museum.nitrogenSeen);
		_museum.nitrogenSeen = 1;
		return kHandled;
	case 317: // Look at the electron microscope.
		say(16, _museum.microscopeSeen);
		_museum.microscopeSeen = 1;
		return kHandled;
	case 320: // Put the Mayan mask back in the chamber.
		say(19, _inventory.museum().rubyRemoved);
		return kHandled;
	case 321: // Play the recording to the voice lock.
		if (!_museum.voiceLockSeen) {
			say(2, 0, true);
		} else if (!_inventory.museum().voiceRecorded) {
			say(20);
		} else if (_museum.chamberOpen) {
			say(1, 2, true);
		} else if (_inventory.museum().recorderBattery == 1 || _inventory.museum().recorderBattery == 2) {
			say(20, _inventory.museum().recorderBattery == 1 ? 2 : 1);
		} else {
			const byte battery = _inventory.museum().recorderBattery;
			say(20, battery == 0 ? 3 : battery == 3 ? 7 : 9);
			walkTo(Common::Point(204, 448), 1, true);
			beginRoomAnimation(340, 29, 12, kPressButtonFrames, ARRAYSIZE(kPressButtonFrames));
		}
		return kHandled;
	case 325: // Freeze the battery in the ladle.
		if (!_inventory.museum().batteryInLadle) {
			say(23);
		} else {
			say(23, 1);
			beginRoomAnimation(325, 23, 69, kFreezeBatteryFrames, ARRAYSIZE(kFreezeBatteryFrames));
			byte tank[ARRAYSIZE(kFreezeBatteryFrames)];
			for (uint i = 0; i < ARRAYSIZE(tank); ++i) {
				tank[i] = MIN<uint>(44, i > 17 ? i - 17 : 0);
			}
			setRoomAnimationOverlay(25, 45, tank, ARRAYSIZE(tank));
		}
		return kHandled;
	case 329: // Take the ladle.
		_museum.ladleTaken = 1;
		_inventory.add(18);
		say(113, 0, true);
		return kBackgroundChanged;
	case 330: // Put the battery directly in the liquid nitrogen.
		say(27, _inventory.museum().frozenBattery);
		return kHandled;
	default:
		return kUnhandled;
	}
}

void Scene4100::finishCustomAnimation(uint16 action) {
	switch (action) {
	case 450: // Keep the security door open while Brian leaves.
		_actionAnimation.draw(_background, 12, 0, 0);
		walkTo(Common::Point(0, 465), 0xff, true);
		deferMuseumAction(451);
		break;
	case 451: // Close the door from the hall side.
		leaveMuseumRoom(4062, Common::Point(1095, 300), 3);
		break;
	case 340: // The voice lock asks for identification.
		say(90, 0, false, 1);
		deferMuseumAction(341);
		break;
	case 341: // Raise the tape recorder.
		beginRoomAnimationRange(342, 27, 21, 0, 20);
		break;
	case 342: // Play Susan's recorded name.
		say(91, _inventory.museum().recorderBattery ? 1 : 0, false, 2);
		holdRoomAnimation(343, 27, 21, 20);
		break;
	case 343: // Lower the tape recorder.
		beginRoomAnimationRange(344, 27, 21, 20, 0);
		break;
	case 344: // The voice lock accepts or rejects the recording.
		_computerReject = !_inventory.museum().recorderBattery;
		say(90, _computerReject ? 1 : 2, false, 1);
		deferMuseumAction(321);
		break;
	case 321: // Remove the dead battery or unlock the chamber.
		_computerReject = false;
		if (!_inventory.museum().recorderBattery) {
			say(20, 4);
			say(20, 5);
			_inventory.museum().recorderBattery = 2;
			_inventory.add(6);
			say(20, 6);
		} else {
			beginRoomAnimation(345, 10, 7, nullptr, 0, false);
		}
		break;
	case 345: // Open the unlocked thermal chamber.
		beginRoomAnimation(431, _museum.maskTaken ? 33 : 32, 36, nullptr, 0, false);
		_audio.playEffect(33, 191);
		break;
	case 325: // Retrieve the frozen battery from the ladle.
		_inventory.museum().batteryInLadle = 0;
		_inventory.museum().frozenBattery = 1;
		_inventory.add(6);
		say(23, 2);
		say(23, 3);
		break;
	case 308: // Try identifying Brian to the voice lock.
		startComputerConversation();
		break;
	case 409: // Approach the scanner with the restored crucifix.
		say(28, 1);
		beginRoomAnimation(410, 26, 46);
		break;
	case 410: // Show the scanner film.
		startMuseumFilm(kScannerFilm);
		break;
	case 411: // Show the betrayal film.
		startMuseumFilm(kBetrayalFilm);
		break;
	case 412: // Introduce the analysis laboratory.
		if (!_museum.labVisited) {
			_museum.labVisited = 1;
			say(0);
		}
		break;
	case 413: // React to the scan results.
		say(28);
		deferMuseumAction(414);
		break;
	case 414: // Brian turns toward Clive.
		beginRoomAnimationRange(415, 24, 28, 0, 4);
		break;
	case 415: { // Clive enters the laboratory.
		_camera.x = 0;
		beginRoomAnimationRange(416, 22, 60, 0, 16);
		const byte brian[] = {4};
		setRoomAnimationOverlay(24, 28, brian, ARRAYSIZE(brian));
		break;
	}
	case 416: // Clive calls Brian over.
		if (!_cliveAnimation.load(_archive, 22, 60)) {
			error("Cannot load Runaway Clive animation");
		}
		_cliveVisible = true;
		say(92, 0, false, 1);
		holdRoomAnimation(417, 24, 28, 4);
		break;
	case 417: // Brian turns to listen.
		beginRoomAnimationRange(418, 24, 28, 5, 9);
		break;
	case 418: // Clive warns about Gina.
		say(92, 1, false, 1);
		holdRoomAnimation(419, 24, 28, 9);
		break;
	case 419: { // Clive leaves the laboratory.
		_cliveVisible = false;
		beginRoomAnimationRange(420, 22, 60, 44, 59);
		const byte brian[] = {9};
		setRoomAnimationOverlay(24, 28, brian, ARRAYSIZE(brian));
		break;
	}
	case 420: // Brian follows Clive.
		beginRoomAnimationRange(411, 24, 28, 10, 27);
		break;
	case 432: // Close the chamber after taking the mask.
		closeChamber();
		break;
	case 430: // Finish closing the thermal chamber.
		_museum.chamberOpen = 0;
		_museum.chamberTime = 0;
		refreshMuseum();
		break;
	case 431: // Start the chamber closing timer.
		if (_inventory.museum().recorderBattery == 3) {
			say(20, 8);
		}
		_inventory.museum().recorderBattery = 4;
		_museum.buttonSeen = _museum.chamberOpen = 1;
		_museum.chamberTime = 30000;
		refreshMuseum();
		break;
	}
}

void Scene4100::handleAnimationFrameHook() {
	if ((_roomAction == 308 || _roomAction == 340) && _actionStep == 12) {
		_audio.playEffect(34, 230);
	}
	if ((_roomAction == 342 && _actionStep == 15) || (_roomAction == 344 && _actionStep == 4)) {
		_audio.playEffect(37, 204);
	}
	if (_roomAction == 325) {
		if (_actionStep == 12) {
			_audio.playEffect(42);
		} else if (_actionStep == 20) {
			_audio.playEffect(41, 255, 1);
		} else if (_actionStep == 105) {
			_audio.playEffect(43);
		}
	}
	if (_roomAction == 410 && _actionStep == 35) {
		_audio.playEffect(55);
	}
}

void Scene4100::finishCustomFilm() {
	switch (_museumFilm) {
	case kScannerFilm:
		if (!applyPatch(28)) {
			error("Cannot update Runaway scanner");
		}
		walkTo(Common::Point(915, 547), 4, true);
		deferMuseumAction(413);
		break;
	case kBetrayalFilm:
		_chapterComplete = true;
		break;
	default:
		break;
	}
}

void Scene4100::handleCustomDialogueChoice(byte effect) {
	_computerReject = true;
}

void Scene4100::finishCustomConversation() {
	_computerReject = false;
	say(7, _museum.ownVoiceTried);
	_museum.ownVoiceTried = 1;
}

void Scene4100::runExitSideEffectsAfterLoop() {
	_museum.chamberOpen = 0;
	_museum.chamberTime = 0;
}

float Scene4100::actorHeight() const {
	const int x = _position.x, y = _position.y;
	if (x > 1260) {
		return MIN(y, 545) * 1.2807018f - 242.982452f;
	}
	if (x > 645) {
		return MIN(y, 545) * 0.73333335f + 55.333332f;
	}
	return MIN(y, 545) * 0.9381443f - 56.288658f;
}

Common::U32String Scene4100::hotspotName(byte id) const {
	if (id == 3 && _museum.cabinetSeen) {
		return _strings.text(RunawayStrings::kThermalChamber);
	}
	if (id == 7 && _museum.nitrogenSeen) {
		return _strings.text(RunawayStrings::kNitrogenTank);
	}
	if (id == 8 && _museum.keySeen) {
		return _strings.text(RunawayStrings::kKey);
	}
	if (id == 9 && _museum.microscopeSeen) {
		return _strings.text(RunawayStrings::kElectronMicroscope);
	}
	return RunawayRoomScene::hotspotName(id);
}

void Scene4100::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
	params.depthY[2] = 477;
}

int8 Scene4100::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (museumChapter() && id == 6 && !use && !item) {
		return !_museum.scannerSeen;
	}
	// Native 0x469870 only approaches the mask while the chamber is open.
	if (id == 5 && use && !item) {
		return !museumChapter() || _museum.chamberOpen;
	}
	return approach;
}

Common::Point Scene4100::customizeSceneActionPosition(byte id, byte item, Common::Point position) const {
	if (museumChapter() && item == 5 && (id == 3 || id == 4)) {
		return Common::Point(204, 448);
	}
	return position;
}

byte Scene4100::customizeSceneActionFacing(byte id, byte facing) const {
	return !museumChapter() && (id == 3 || id == 5) ? 1 : facing;
}

void Scene4100::closeChamber() {
	_museum.chamberTime = 0;
	beginRoomAnimationRange(430, _museum.maskTaken ? 33 : 32, 36, 35, 0, false);
	_audio.playEffect(33, 191);
}

void Scene4100::startComputerConversation() {
	startConversation(3);
	for (uint i = 0; i < 5; ++i) {
		_dialogue[0][i] = {1, 0, 0, (byte)i, 1, 0, 0};
	}
	_dialogue[0][3].enabled = _inventory.museum().businessCardExamined;
	_museum.voiceLockSeen = 1;
	say(90, 0, false, 1);
}

} // End of namespace Runaway
} // End of namespace Hollywood
