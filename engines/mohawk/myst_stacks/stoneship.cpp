/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "mohawk/cursors.h"
#include "mohawk/myst.h"
#include "mohawk/graphics.h"
#include "mohawk/myst_areas.h"
#include "mohawk/myst_state.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/stoneship.h"

#include "gui/message.h"

namespace Mohawk {

MystScriptParser_Stoneship::MystScriptParser_Stoneship(MohawkEngine_Myst *vm) :
		MystScriptParser(vm), _state(vm->_gameState->_stoneship) {
	setupOpcodes();

	_state.generatorDepletionTime = 0;
	_state.generatorDuration = 0;
	_cabinMystBookPresent = 0;
	_siriusDrawerDrugsOpen = 0;
	_chestDrawersOpen = 0;
	_chestAchenarBottomDrawerClosed = 1;

	// Drop key
	if (_state.trapdoorKeyState == 1)
		_state.trapdoorKeyState = 2;

	// Power is not available when loading
//	if (_state.sideDoorOpened)
//		_state.generatorPowerAvailable = 2;
//	else
//		_state.generatorPowerAvailable = 0;
}

MystScriptParser_Stoneship::~MystScriptParser_Stoneship() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &MystScriptParser_Stoneship::x, #x))

void MystScriptParser_Stoneship::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, o_pumpTurnOff);
	OPCODE(101, o_brotherDoorOpen);
	OPCODE(102, o_cabinBookMovie);
	OPCODE(103, o_drawerOpenSirius);
	OPCODE(104, o_drawerClose);
	OPCODE(108, o_generatorStart);
	OPCODE(109, NOP);
	OPCODE(110, o_generatorStop);
	OPCODE(111, o_drawerOpenAchenar);
	OPCODE(112, o_hologramPlayback);
	OPCODE(113, o_hologramSelectionStart);
	OPCODE(114, o_hologramSelectionMove);
	OPCODE(115, o_hologramSelectionStop);
	OPCODE(116, o_compassButton);
	OPCODE(117, o_chestValveVideos);
	OPCODE(118, o_chestDropKey);
	OPCODE(119, o_trapLockOpen);
	OPCODE(120, o_sideDoorsMovies);
	OPCODE(121, o_cloudOrbEnter);
	OPCODE(122, o_cloudOrbLeave);
	OPCODE(125, o_drawerCloseOpened);

	// "Init" Opcodes
	OPCODE(200, o_hologramDisplay_init);
	OPCODE(201, o_hologramSelection_init);
	OPCODE(202, opcode_202);
	OPCODE(203, opcode_203);
	OPCODE(204, opcode_204);
	OPCODE(205, opcode_205);
	OPCODE(206, opcode_206);
	OPCODE(207, o_chest_init);
	OPCODE(208, opcode_208);
	OPCODE(209, o_achenarDrawers_init);
	OPCODE(210, o_cloudOrb_init);

	// "Exit" Opcodes
	OPCODE(300, opcode_300);
}

#undef OPCODE

void MystScriptParser_Stoneship::disablePersistentScripts() {
	_batteryCharging = false;
}

void MystScriptParser_Stoneship::runPersistentScripts() {
	if (_batteryCharging)
		chargeBattery_run();
}

uint16 MystScriptParser_Stoneship::getVar(uint16 var) {
	switch(var) {
	case 0: // Water Drained From Lighthouse / Right Button Of Pump
		return _state.pumpState == 4;
	case 1: // Water Drained From Tunnels To Brothers' Rooms / Middle Button Of Pump
		return _state.pumpState == 2;
	case 2: // Water Drained From Ship Cabin Tunnel / Left Button Of Pump
		return _state.pumpState == 1;
	case 3: // Lighthouse Chest Floating
		return _state.pumpState != 4 && !_state.chestValveState && !_state.chestWaterState;
	case 4: // Lighthouse State - Close Up
		if (_state.pumpState == 4) {
			return 1; // Drained
		} else {
			if (_state.chestValveState || _state.chestWaterState)
				return 0; // Flooded
			else
				return 2; // Flooded, Chest Floating
		}
	case 5: // Lighthouse Trapdoor State
		return _state.trapdoorState;
	case 6: // Chest valve state
		return _state.chestValveState;
	case 7: // Lighthouse Chest Unlocked
		return _state.chestOpenState;
	case 8: // Lighthouse Chest Key Position
		return _state.trapdoorKeyState;
	case 11: // Lighthouse Key State
		if (_state.chestOpenState) {
			if (_state.trapdoorKeyState == 1)
				return 1;
			else if (_state.trapdoorKeyState == 2)
				return 2;
			else
				return 3;
		} else {
			return 0;
		}
	case 12: // Trapdoor can be unlocked
		return _state.trapdoorKeyState == 1 && _state.trapdoorState == 2;
	case 13: // State Of Tunnels To Brothers' Rooms - Close Up
		if (_state.generatorPowerAvailable == 0) {
			if (_state.pumpState != 2)
				return 0; // Dark, Flooded
			else
				return 1; // Dark, Drained
		} else {
			if (_state.pumpState != 2)
				return 2; // Lit, Flooded
			else
				return 3; // Lit, Drained
		}
	case 14: // State Of Tunnels lights To Brothers' Rooms - Far
		return _state.generatorPowerAvailable;
	case 15: // Side Door in Tunnels To Brother's Rooms Open
		if (_state.generatorPowerAvailable == 1)
			return _state.sideDoorOpened;
		else
			return 0;
	case 16: // Ship Chamber Light State
		return _state.lightState;
	case 17: // Sirrus' Room Drawer with Drugs Open
		return _siriusDrawerDrugsOpen;
	case 18: // Brother Room Door Open
		return _brotherDoorOpen;
	case 19: // Brother Room Door State
		if (_brotherDoorOpen) {
			if (_state.lightState)
				return 2; // Open, Light On
			else
				return 1; // Open, Light Off
		} else {
			return 0; // Closed
		}
	case 20: // Ship Chamber Table/Book State
		return _cabinMystBookPresent;
	case 21: // Brothers Rooms' Chest Of Drawers Drawer State
		return _chestDrawersOpen;
	case 28: // Telescope Angle Position
		return 0;
	case 29: // Achenar's Room Rose/Skull Hologram Button Lit
		return _hologramTurnedOn;
	case 30: // Light State in Tunnel to Compass Rose Room
		if (_state.generatorPowerAvailable == 1) {
			if (_state.lightState)
				return 0;
			else
				return 1;
		} else {
			return 2;
		}
	case 31: // Lighthouse Lamp Room Battery Pack Indicator Light
		return batteryRemainingCharge() >= 10;
	case 32: // Lighthouse Lamp Room Battery Pack Meter Level
		return 0;
	case 33: // State of Side Door in Tunnels to Compass Rose Room
		if (_state.sideDoorOpened)
			return 2;
		else
			return _state.generatorPowerAvailable == 1;
	case 34: // Achenar's Room Drawer with Torn Note Closed
		return _chestAchenarBottomDrawerClosed;
	case 35: // Sirrus' Room Drawer #4 (Bottom) Open and Red Page State
		if (_chestDrawersOpen == 4)
			return getVar(102);
		else
			return 2;
	case 36: // Ship Chamber Door State
		if (_tempVar) {
			if (_state.lightState)
				return 2; // Open, Light On
			else
				return 1; // Open, Light Off
		} else {
			return 0; // Closed
		}
	case 102: // Red page
		return !(_globals.redPagesInBook & 8) && (_globals.heldPage != 10);
	case 103: // Blue page
		return !(_globals.bluePagesInBook & 8) && (_globals.heldPage != 4);
	default:
		return MystScriptParser::getVar(var);
	}
}

void MystScriptParser_Stoneship::toggleVar(uint16 var) {
	switch(var) {
	case 0: // Water Drained From Lighthouse / Right Button Of Pump
		if (_state.pumpState == 4)
			_state.pumpState = 0;
		else
			_state.pumpState = 4;
		break;
	case 1: // Water Drained From Tunnels To Brothers' Rooms / Middle Button Of Pump
		if (_state.pumpState == 2)
			_state.pumpState = 0;
		else
			_state.pumpState = 2;
		break;
	case 2: // Water Drained From Ship Cabin Tunnel / Left Button Of Pump
		if (_state.pumpState == 1)
			_state.pumpState = 0;
		else
			_state.pumpState = 1;
		break;
	case 6: // Chest valve state
		_state.chestValveState = (_state.chestValveState + 1) % 2;
		break;
	case 8:  // Lighthouse Chest Key Position
		if (_state.trapdoorKeyState) {
			if (_state.trapdoorKeyState == 1)
				_state.trapdoorKeyState = 2;
			else
				_state.trapdoorKeyState = 1;
		}
		break;
	case 10: // Chest water state
		_state.chestWaterState = 0;
		break;
	case 11:
		if (_state.chestOpenState)
			_state.trapdoorKeyState = _state.trapdoorKeyState != 1;
		break;
	case 20: // Ship Chamber Table/Book State
		_cabinMystBookPresent = (_cabinMystBookPresent + 1) % 2;
		break;
	case 29: // Achenar's Room Rose/Skull Hologram Button Lit
		_hologramTurnedOn = (_hologramTurnedOn + 1) % 2;
		break;
	case 102: // Red page
		if (!(_globals.redPagesInBook & 8)) {
			if (_globals.heldPage == 10)
				_globals.heldPage = 0;
			else
				_globals.heldPage = 10;
		}
		break;
	case 103: // Blue page
		if (!(_globals.bluePagesInBook & 8)) {
			if (_globals.heldPage == 4)
				_globals.heldPage = 0;
			else
				_globals.heldPage = 4;
		}
		break;
	default:
		MystScriptParser::toggleVar(var);
		break;
	}
}

bool MystScriptParser_Stoneship::setVarValue(uint16 var, uint16 value) {
	bool refresh = false;

	switch (var) {
	case 5: // Lighthouse Trapdoor State
		_state.trapdoorState = value;
		break;
	case 7:
		if (_state.chestOpenState != value)
			_state.chestOpenState = value;
		break;
	case 8: // Lighthouse Chest Key Position
		_state.trapdoorKeyState = value;
		break;
	case 15: // Side Door in Tunnels To Brother's Rooms Open
		if (_state.sideDoorOpened != value) {
			if (!value && _state.generatorPowerAvailable == 2)
				_state.generatorPowerAvailable = 0;
			_state.sideDoorOpened = value;
			refresh = true;
		}
		break;
	case 17: // Sirrus' Room Drawer with Drugs Open
		if (_siriusDrawerDrugsOpen != value) {
			_siriusDrawerDrugsOpen = value;
			refresh = true;
		}
		break;
	case 18: // Brother Room Door Open
		if (_brotherDoorOpen != value) {
			_brotherDoorOpen = value;
			refresh = true;
		}
		break;
	case 21: // Brothers Rooms' Chest Of Drawers Drawer State
		if (_chestDrawersOpen != value) {
			_chestDrawersOpen = value;
			refresh = true;
		}
		break;
	case 29: // Achenar's Room Rose/Skull Hologram Button Lit
		_hologramTurnedOn = value;
		break;
	case 34: // Achenar's Room Drawer with Torn Note Closed
		_chestAchenarBottomDrawerClosed = value;
		break;
	default:
		refresh = MystScriptParser::setVarValue(var, value);
		break;
	}

	return refresh;
}

void MystScriptParser_Stoneship::o_pumpTurnOff(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Turn off previous pump selection", op);

	if (_state.pumpState) {
		uint16 buttonVar = 0;

		switch (_state.pumpState) {
		case 1:
			buttonVar = 2;
			break;
		case 2:
			buttonVar = 1;
			break;
		case 4:
			buttonVar = 0;
			break;
		default:
			warning("Incorrect pump state");
		}

		for (uint i = 0; i < _vm->_resources.size(); i++) {
			MystResource *resource = _vm->_resources[i];
			if (resource->type == kMystConditionalImage && resource->getType8Var() == buttonVar) {
				static_cast<MystResourceType8 *>(resource)->drawConditionalDataToScreen(0, true);
				break;
			}
		}
	}
}

void MystScriptParser_Stoneship::o_brotherDoorOpen(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Open brother door", op);

	_brotherDoorOpen = 1;
	_vm->redrawArea(19, 0);
	animatedUpdate(argc, argv, 5);
}

void MystScriptParser_Stoneship::o_cabinBookMovie(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Play Book Room Movie", op);

	uint16 startTime = argv[0];
	uint16 endTime = argv[1];

	VideoHandle book = _vm->_video->playMovie(_vm->wrapMovieFilename("bkroom", kStoneshipStack), 159, 99);
	_vm->_video->setVideoBounds(book, Video::VideoTimestamp(startTime, 600), Video::VideoTimestamp(endTime, 600));
	_vm->_video->waitUntilMovieEnds(book);
}

void MystScriptParser_Stoneship::o_drawerOpenSirius(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Open drawer", op);

	MystResourceType8 *drawer = static_cast<MystResourceType8 *>(_vm->_resources[argv[0]]);

	if (drawer->getType8Var() == 35) {
		drawer->drawConditionalDataToScreen(getVar(102), 0);
	} else {
		drawer->drawConditionalDataToScreen(0, 0);
	}

	uint16 transition = 5;
	if (argc == 2 && argv[1])
		transition = 11;

	_vm->_gfx->runTransition(transition, drawer->getRect(), 25, 5);
}

void MystScriptParser_Stoneship::o_drawerClose(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Close drawer", op);
	drawerClose(argv[0]);
}

void MystScriptParser_Stoneship::o_generatorStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generator start", op);

	MystResourceType11 *handle = static_cast<MystResourceType11 *>(_invokingResource);

	uint16 soundId = handle->getList1(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);

	if (_state.generatorDuration)
		_state.generatorDuration -= _vm->_system->getMillis() - _state.generatorDepletionTime;

	// Start charging the battery
	_batteryCharging = true;
	_batteryNextTime = _vm->_system->getMillis() + 1000;

	// Start handle movie
	MystResourceType6 *movie = static_cast<MystResourceType6 *>(handle->getSubResource(0));
	movie->playMovie();

	soundId = handle->getList2(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId, Audio::Mixer::kMaxChannelVolume, true);
}

void MystScriptParser_Stoneship::o_generatorStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generator stop", op);

	_batteryCharging = false;

	if (_state.generatorDuration) {
		// Clip battery power
		if (_state.generatorDuration > 600000)
			_state.generatorDuration = 600000;

		// Start depleting power
		_state.generatorDepletionTime = _vm->_system->getMillis() + _state.generatorDuration;
		_state.generatorPowerAvailable = 1;
		_batteryDepleting = true;
	}

	// Pause handle movie
	MystResourceType11 *handle = static_cast<MystResourceType11 *>(_invokingResource);
	MystResourceType6 *movie = static_cast<MystResourceType6 *>(handle->getSubResource(0));
	movie->pauseMovie(true);

	uint16 soundId = handle->getList3(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);
}

void MystScriptParser_Stoneship::chargeBattery_run() {
	uint32 time = _vm->_system->getMillis();

	if (time > _batteryNextTime) {
		_batteryNextTime = time + 1000;
		_state.generatorDuration += 30000;
	}
}

uint16 MystScriptParser_Stoneship::batteryRemainingCharge() {
	if (_state.generatorDepletionTime) {
		return (_state.generatorDepletionTime - _vm->_system->getMillis()) / 7500;
	} else {
		return 0;
	}
}

void MystScriptParser_Stoneship::o_drawerOpenAchenar(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Open drawer", op);

	MystResourceType8 *drawer = static_cast<MystResourceType8 *>(_vm->_resources[argv[0]]);
	drawer->drawConditionalDataToScreen(0, 0);
	_vm->_gfx->runTransition(5, drawer->getRect(), 25, 5);
}

void MystScriptParser_Stoneship::o_hologramPlayback(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Card 2013 (Achenar's Rose-Skull Hologram)
	debugC(kDebugScript, "Opcode %d: Rose-Skull Hologram Playback", op);

	uint16 startPoint = argv[0];
	uint16 endPoint = argv[1];
	// uint16 direction = argv[2];

	_hologramDisplay->setBlocking(false);
	VideoHandle displayMovie = _hologramDisplay->playMovie();

	if (_hologramTurnedOn) {
		if (_hologramDisplayPos)
			endPoint = _hologramDisplayPos;
		_vm->_video->setVideoBounds(displayMovie, Video::VideoTimestamp(startPoint, 600), Video::VideoTimestamp(endPoint, 600));
	} else {
		_vm->_video->setVideoBounds(displayMovie, Video::VideoTimestamp(startPoint, 600), Video::VideoTimestamp(endPoint, 600));
	}

	_vm->_video->delayUntilMovieEnds(displayMovie);
}

void MystScriptParser_Stoneship::o_hologramSelectionStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Hologram start move", op);
	//_vm->_cursor->setCursor(0);
}

void MystScriptParser_Stoneship::o_hologramSelectionMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Hologram move", op);

	MystResourceType11 *handle = static_cast<MystResourceType11 *>(_invokingResource);
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();

	if (handle->getRect().contains(mouse)) {
		int16 position = mouse.x - 143;
		position = CLIP<int16>(position, 0, 242);

		// Draw handle movie frame
		uint16 selectionPos = position * 1500 / 243;

		VideoHandle handleMovie = _hologramSelection->playMovie();
		_vm->_video->setVideoBounds(handleMovie, Video::VideoTimestamp(selectionPos, 600), Video::VideoTimestamp(selectionPos, 600));

		_hologramDisplayPos = position * 1450 / 243 + 350;

		// Draw display movie frame
		if (_hologramTurnedOn) {
			_hologramDisplay->setBlocking(false);
			VideoHandle displayMovie = _hologramDisplay->playMovie();
			_vm->_video->setVideoBounds(displayMovie, Video::VideoTimestamp(_hologramDisplayPos, 600), Video::VideoTimestamp(_hologramDisplayPos, 600));
		}
	}
}

void MystScriptParser_Stoneship::o_hologramSelectionStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Hologram stop move", op);
	_vm->checkCursorHints();
}

void MystScriptParser_Stoneship::o_compassButton(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Compass rose button pressed", op);
	// Used on Card 2111 (Compass Rose)
	// Called when Button Clicked.
	uint16 correctButton = argv[0];

	if (correctButton) {
		// Correct Button -> Light On Logic
		_state.lightState = 1;
	} else {
		// Wrong Button -> Power Failure Logic
		_state.generatorPowerAvailable = 2;
		_state.lightState = 0;
		_state.generatorDepletionTime = 0;
		_state.generatorDepletionTime = 0;

		_batteryDepleting = false;
	}

	o_redrawCard(op, var, argc, argv);
}

void MystScriptParser_Stoneship::o_chestValveVideos(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Chest valve videos", op);

	Common::String movie = _vm->wrapMovieFilename("ligspig", kStoneshipStack);

	_vm->_sound->playSound(2132);

	if (_state.chestValveState) {
		// Valve closing
		VideoHandle valve = _vm->_video->playMovie(movie, 97, 267);
		_vm->_video->setVideoBounds(valve, Video::VideoTimestamp(0, 600), Video::VideoTimestamp(350, 600));
		_vm->_video->waitUntilMovieEnds(valve);
	} else if (_state.chestWaterState) {
		// Valve opening, spilling water
		VideoHandle valve = _vm->_video->playMovie(movie, 97, 267);
		_vm->_video->setVideoBounds(valve, Video::VideoTimestamp(350, 600), Video::VideoTimestamp(650, 600));
		_vm->_video->waitUntilMovieEnds(valve);

		_vm->_sound->playSound(3132);

		for (uint i = 0; i < 25; i++) {
			valve = _vm->_video->playMovie(movie, 97, 267);
			_vm->_video->setVideoBounds(valve, Video::VideoTimestamp(650, 600), Video::VideoTimestamp(750, 600));
			_vm->_video->waitUntilMovieEnds(valve);
		}

		_vm->_sound->resumeBackgroundMyst();
	} else {
		// Valve opening
		// TODO: Play backwards
		VideoHandle valve = _vm->_video->playMovie(movie, 97, 267);
		_vm->_video->setVideoBounds(valve, Video::VideoTimestamp(0, 600), Video::VideoTimestamp(350, 600));
		_vm->_video->waitUntilMovieEnds(valve);
	}
}

void MystScriptParser_Stoneship::o_chestDropKey(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: drop chest key", op);

	// If holding Key to Lamp Room Trapdoor, drop to bottom of
	// Lighthouse...
	if (_state.trapdoorKeyState == 1) {
		_vm->setMainCursor(_savedCursorId);
		_state.trapdoorKeyState = 2;
	}
}

void MystScriptParser_Stoneship::o_trapLockOpen(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Trap lock open video", op);

	Common::String movie = _vm->wrapMovieFilename("openloc", kStoneshipStack);

	VideoHandle lock = _vm->_video->playMovie(movie, 187, 71);
	_vm->_video->setVideoBounds(lock, Video::VideoTimestamp(0, 600), Video::VideoTimestamp(750, 600));
	_vm->_video->waitUntilMovieEnds(lock);

	_vm->_sound->playSound(2143);

	lock = _vm->_video->playMovie(movie, 187, 71);
	_vm->_video->setVideoBounds(lock, Video::VideoTimestamp(750, 600), Video::VideoTimestamp(10000, 600));
	_vm->_video->waitUntilMovieEnds(lock);

	if (_state.pumpState != 4)
		_vm->_sound->playSound(4143);
}

void MystScriptParser_Stoneship::o_sideDoorsMovies(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Cards 2285, 2289, 2247, 2251 (Side Doors in Tunnels Down To Brothers Rooms)
	uint16 movieId = argv[0];

	debugC(kDebugScript, "Opcode %d: Play Side Door Movies", op);
	debugC(kDebugScript, "\tmovieId: %d", movieId);

	_vm->_cursor->hideCursor();
	_vm->_sound->pauseBackgroundMyst();

	switch (movieId) {
	case 0:
		// Card 2251
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("tunaup", kStoneshipStack), 149, 161);
		break;
	case 1:
		// Card 2247
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("tunadown", kStoneshipStack), 218, 150);
		break;
	case 2:
		// Card 2289
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("tuncup", kStoneshipStack), 259, 161);
		break;
	case 3:
		// Card 2285
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("tuncdown", kStoneshipStack), 166, 150);
		break;
	default:
		warning("Opcode 120 MovieId Out Of Range");
		break;
	}

	_vm->_sound->resumeBackgroundMyst();
	_vm->_cursor->showCursor();
}

void MystScriptParser_Stoneship::o_cloudOrbEnter(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Cloud orb enter", op);

	_vm->_sound->replaceSoundMyst(_cloudOrbSound, Audio::Mixer::kMaxChannelVolume, true);
	_cloudOrbMovie->playMovie();
}

void MystScriptParser_Stoneship::o_cloudOrbLeave(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Cloud orb leave", op);

	_cloudOrbMovie->pauseMovie(true);
	_vm->_sound->replaceSoundMyst(_cloudOrbStopSound);
	_vm->_gfx->runTransition(5, _invokingResource->getRect(), 4, 0);
}

void MystScriptParser_Stoneship::o_drawerCloseOpened(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Close open drawer", op);

	uint16 drawerOpen = getVar(var);
	if (drawerOpen)
		drawerClose(argv[0] + drawerOpen - 1);
}

void MystScriptParser_Stoneship::drawerClose(uint16 drawer) {
	_chestDrawersOpen = 0;
	_vm->drawCardBackground();
	_vm->drawResourceImages();

	MystResource *res = _vm->_resources[drawer];
	_vm->_gfx->runTransition(6, res->getRect(), 25, 5);
}

void MystScriptParser_Stoneship::o_hologramDisplay_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Hologram display init", op);
	_hologramDisplay = static_cast<MystResourceType6 *>(_invokingResource);

	_hologramDisplayPos = 0;
}

void MystScriptParser_Stoneship::o_hologramSelection_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Hologram selection init", op);
	_hologramSelection = static_cast<MystResourceType6 *>(_invokingResource);
}

void MystScriptParser_Stoneship::opcode_202(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 2160 (Lighthouse Battery Pack Closeup)
	// TODO: Implement Code...
	// Not Sure of Purpose - Update of Light / Discharge?
	unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_203(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for all/most Cards in Tunnels Down To Brothers Rooms

	// TODO: Duplicate or similar function to Opcode 203?
	if (argc == 2 || argc == 4) {
		debugC(kDebugScript, "Opcode %d: %d Arguments", op, argc);

		uint16 imageIdDarkDoorOpen = 0;
		uint16 imageIdDarkDoorClosed = 0;
		uint16 u0 = argv[0];
		uint16 soundIdAlarm = argv[argc - 1];

		if (argc == 4) {
			imageIdDarkDoorOpen = argv[1];
			imageIdDarkDoorClosed = argv[2];
		}

		debugC(kDebugScript, "\tu0: %d", u0);

		if (argc == 4) {
			debugC(kDebugScript, "\timageIdDarkDoorOpen: %d", imageIdDarkDoorOpen);
			debugC(kDebugScript, "\tsoundIdDarkDoorClosed: %d", imageIdDarkDoorClosed);
		}

		debugC(kDebugScript, "\tsoundIdAlarm: %d", soundIdAlarm);

		// TODO: Fill in Correct Function for Lights
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_204(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 2160 (Lighthouse Battery Pack Closeup)
	if (argc == 0) {
		// TODO: Implement Code For Battery Meter Level
		// Overwrite _vm->_resources[1]->_subImages[0].rect.bottom 1 to 80
		// Add accessor functions for this...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_205(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used on Cards 2322, 2285 (Tunnels Down To Brothers Rooms)

	// TODO: Duplicate or similar function to Opcode 203?
	if (argc == 2 || argc == 4) {
		debugC(kDebugScript, "Opcode %d: %d Arguments", op, argc);

		uint16 imageIdDoorOpen = 0;
		uint16 imageIdDoorClosed = 0;

		uint16 u0 = argv[0];
		if (argc == 4) {
			imageIdDoorOpen = argv[1];
			imageIdDoorClosed = argv[2];
		}

		uint16 soundIdAlarm = argv[argc - 1];

		debugC(kDebugScript, "\tu0: %d", u0);

		if (argc == 4) {
			debugC(kDebugScript, "\timageIdDoorOpen: %d", imageIdDoorOpen);
			debugC(kDebugScript, "\tsoundIdDoorClosed: %d", imageIdDoorClosed);
		}

		debugC(kDebugScript, "\tsoundIdAlarm: %d", soundIdAlarm);

		// TODO: Fill in Correct Function for Lights
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_206(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Cards 2272 and 2234 (Facing Out of Door)
	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown, %d Arguments", op, argc);
		// TODO: Function Unknown...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::o_chest_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Chest init", op);

	_state.chestOpenState = 0;
}

void MystScriptParser_Stoneship::opcode_208(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used in Card 2218 (Telescope view)
	if (argc == 3) {
		debugC(kDebugScript, "Opcode %d: Telescope View", op);
		uint16 imagePanorama = argv[0];
		uint16 imageLighthouseOff = argv[1];
		uint16 imageLighthouseOn = argv[2];

		debugC(kDebugScript, "Image (Panorama): %d", imagePanorama);
		debugC(kDebugScript, "Image (Lighthouse Off): %d", imageLighthouseOff);
		debugC(kDebugScript, "Image (Lighthouse On): %d", imageLighthouseOn);

		// TODO: Fill in Logic.
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::o_achenarDrawers_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Achenar's Room Drawers Init", op);

	// Used for Card 2004 (Achenar's Room Drawers)
	if (!_chestAchenarBottomDrawerClosed) {
		uint16 count1 = argv[0];
		for (uint16 i = 0; i < count1; i++) {
			debugC(kDebugScript, "Disable hotspot index %d", argv[i + 1]);
			_vm->setResourceEnabled(argv[i + 1], false);
		}
		uint16 count2 = argv[count1 + 1];
		for (uint16 i = 0; i < count2; i++) {
			debugC(kDebugScript, "Enable hotspot index %d", argv[i + count1 + 2]);
			_vm->setResourceEnabled(argv[i + count1 + 2], true);
		}
	}
}

void MystScriptParser_Stoneship::o_cloudOrb_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Cloud orb init", op);

	_cloudOrbMovie = static_cast<MystResourceType6 *>(_invokingResource);
	_cloudOrbSound = argv[0];
	_cloudOrbStopSound = argv[1];
}

void MystScriptParser_Stoneship::opcode_300(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Card 2218 (Telescope view)
	varUnusedCheck(op, var);
	// TODO: Fill in Logic. Clearing Variable for View?
}

} // End of namespace Mohawk
