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
}

MystScriptParser_Stoneship::~MystScriptParser_Stoneship() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &MystScriptParser_Stoneship::x, #x))

void MystScriptParser_Stoneship::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, o_pumpTurnOff);
	OPCODE(101, opcode_101);
	OPCODE(102, opcode_102);
	OPCODE(103, opcode_103);
	OPCODE(104, opcode_104);
	OPCODE(108, o_generatorStart);
	OPCODE(109, NOP);
	OPCODE(110, o_generatorStop);
	OPCODE(111, opcode_111);
	OPCODE(112, opcode_112);
	OPCODE(116, opcode_116);
	OPCODE(117, o_chestValveVideos);
	OPCODE(118, o_chestDropKey);
	OPCODE(119, o_trapLockOpen);
	OPCODE(120, opcode_120);
	OPCODE(125, opcode_125);

	// "Init" Opcodes
	OPCODE(200, opcode_200);
	OPCODE(201, opcode_201);
	OPCODE(202, opcode_202);
	OPCODE(203, opcode_203);
	OPCODE(204, opcode_204);
	OPCODE(205, opcode_205);
	OPCODE(206, opcode_206);
	OPCODE(207, o_chest_init);
	OPCODE(208, opcode_208);
	OPCODE(209, opcode_209);
	OPCODE(210, opcode_210);

	// "Exit" Opcodes
	OPCODE(300, opcode_300);
}

#undef OPCODE

void MystScriptParser_Stoneship::disablePersistentScripts() {
	_batteryCharging = false;

	opcode_200_disable();
	opcode_201_disable();
	opcode_209_disable();
}

void MystScriptParser_Stoneship::runPersistentScripts() {
	if (_batteryCharging)
		chargeBattery_run();

	opcode_200_run();
	opcode_201_run();
	opcode_209_run();
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
	case 16: // Ship Chamber Light State
		return _state.lightState;
//case 20: // Ship Chamber Table/Book State
//	return 0;
//	return 1;
	case 28: // Telescope Angle Position
		return 0;
	case 30: // Light State in Tunnel to Compass Rose Room
		if (_state.generatorPowerAvailable == 1) {
			if (_state.lightState)
				return 0;
			else
				return 1;
		} else {
			return 2;
		}
	case 31:
		return batteryRemainingCharge() >= 10;
	case 32: // Lighthouse Lamp Room Battery Pack Meter Level
		return 0;
//case 36: // Ship Chamber Door State
//	return 0; // Closed
//	return 1; // Open, Light Off
//	return 2; // Open, Light On
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

void MystScriptParser_Stoneship::opcode_101(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 6) {
		// Used by Door Buttons to Brothers' Rooms
		// Cards 2294, 2255
		Common::Rect u0_rect = Common::Rect(argv[0], argv[1], argv[2], argv[3]);
		uint16 u1 = argv[3];
		uint16 u2 = argv[2];

		debugC(kDebugScript, "Opcode %d: Unknown", op);
		debugC(kDebugScript, "u0_rect.left: %d", u0_rect.left);
		debugC(kDebugScript, "u0_rect.top: %d", u0_rect.top);
		debugC(kDebugScript, "u0_rect.right: %d", u0_rect.right);
		debugC(kDebugScript, "u0_rect.bottom: %d", u0_rect.bottom);
		debugC(kDebugScript, "u1: %d", u1);
		debugC(kDebugScript, "u2: %d", u2);

		// TODO: Fill in logic...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_102(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 2) {
		debugC(kDebugScript, "Opcode %d: Play Book Room Movie", op);

		uint16 startTime = argv[0];
		uint16 endTime = argv[1];

		debugC(kDebugScript, "\tstartTime: %d", startTime);
		debugC(kDebugScript, "\tendTime: %d", endTime);

		warning("TODO: Opcode %d Movie Time Index %d to %d", op, startTime, endTime);
		// TODO: Need version of playMovie blocking which allows selection
		//       of start and finish points.
		_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("bkroom", kStoneshipStack), 159, 99);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_103(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		// Used on Card 2197 (Sirrus' Room Drawers)
		debugC(kDebugScript, "Opcode %d: Unknown", op);

		uint16 u0 = argv[0];

		debugC(kDebugScript, "\tu0: %d", u0);
		// TODO: Fill in Logic...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_104(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Card 2004 (Achenar's Room Drawers)
	// Used for Closeup of Torn Note?
	if (argc == 1) {
		debugC(kDebugScript, "Opcode %d: Unknown Function", op);

		uint16 u0 = argv[0];
		debugC(kDebugScript, "\tu0: %d", u0);

		// TODO: Fill in Function...
		// Does u0 correspond to a resource Id? Enable? Disable?
		// Similar to Opcode 111 (Stoneship Version).. But does this also
		// draw closeup image of note / change to closeup card?
	} else
		unknown(op, var, argc, argv);
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

void MystScriptParser_Stoneship::opcode_111(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		// Used for Card 2004 (Achenar's Room Drawers)
		// Used by Drawers Hotspots...

		debugC(kDebugScript, "Opcode %d: Unknown Function", op);

		uint16 u0 = argv[0];
		debugC(kDebugScript, "\tu0: %d", u0);

		// TODO: Fill in Function...
		// Does u0 correspond to a resource Id? Enable? Disable?
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_112(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Card 2013 (Achenar's Rose-Skull Hologram)
	if (argc == 3) {
		debugC(kDebugScript, "Opcode %d: Rose-Skull Hologram Playback", op);

		uint16 varValue = _vm->_varStore->getVar(var);

		debugC(kDebugScript, "\tVar: %d = %d", var, varValue);

		uint16 startPoint = argv[0];
		uint16 endPoint = argv[1];
		uint16 u0 = argv[2];

		debugC(kDebugScript, "\tstartPoint: %d", startPoint);
		debugC(kDebugScript, "\tendPoint: %d", endPoint);
		debugC(kDebugScript, "\tu0: %d", u0);

		// TODO: Fill in Function...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_116(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		// Used on Card 2111 (Compass Rose)
		// Called when Button Clicked.
		uint16 correctButton = argv[0];

		if (correctButton) {
			// Correct Button -> Light On Logic
			// TODO: Deal with if main power on?
			_vm->_varStore->setVar(16, 1);
			_vm->_varStore->setVar(30, 0);
		} else {
			// Wrong Button -> Power Failure Logic
			// TODO: Fill in Alarm
			_vm->_varStore->setVar(16, 0);
			_vm->_varStore->setVar(30, 2);
			_vm->_varStore->setVar(33, 0);
		}
	} else
		unknown(op, var, argc, argv);
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

void MystScriptParser_Stoneship::opcode_120(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		// Used for Cards 2285, 2289, 2247, 2251 (Side Doors in Tunnels Down To Brothers Rooms)
		uint16 movieId = argv[0];

		debugC(kDebugScript, "Opcode %d: Play Side Door Movies", op);
		debugC(kDebugScript, "\tmovieId: %d", movieId);

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
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_125(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 1) {
		// Used on Card 2197 (Sirrus' Room Drawers)
		debugC(kDebugScript, "Opcode %d: Unknown uses Var %d", op, var);

		uint16 u0 = argv[0];

		debugC(kDebugScript, "\tu0: %d", u0);
		// TODO: Fill in Logic...
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode200Parameters;

void MystScriptParser_Stoneship::opcode_200_run() {
	// Used for Card 2013 (Achenar's Rose-Skull Hologram)

	// TODO: Implement Function...
}

void MystScriptParser_Stoneship::opcode_200_disable() {
	g_opcode200Parameters.enabled = false;
}

void MystScriptParser_Stoneship::opcode_200(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 2013 (Achenar's Rose-Skull Hologram)
	if (argc == 0)
		g_opcode200Parameters.enabled = true;
	else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode201Parameters;

void MystScriptParser_Stoneship::opcode_201_run() {
	// Used for Card 2013 (Achenar's Rose-Skull Hologram)

	// TODO: Fill in Function...
}

void MystScriptParser_Stoneship::opcode_201_disable() {
	g_opcode201Parameters.enabled = false;
}

void MystScriptParser_Stoneship::opcode_201(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 2013 (Achenar's Rose-Skull Hologram)
	if (argc == 0)
		g_opcode201Parameters.enabled = true;
	else
		unknown(op, var, argc, argv);
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

static struct {
	uint16 u0[5];
	uint16 u1[5];
	uint16 stateVar;

	bool enabled;
} g_opcode209Parameters;

void MystScriptParser_Stoneship::opcode_209_run(void) {
	// Used for Card 2004 (Achenar's Room Drawers)

	// TODO: Implement Function...
	// Swap Open Drawers?
}

void MystScriptParser_Stoneship::opcode_209_disable(void) {
	g_opcode209Parameters.enabled = false;
}

void MystScriptParser_Stoneship::opcode_209(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 2004 (Achenar's Room Drawers)
	if (argc == 11) {
		g_opcode209Parameters.u0[0] = argv[0];
		g_opcode209Parameters.u0[1] = argv[1];
		g_opcode209Parameters.u0[2] = argv[2];
		g_opcode209Parameters.u0[3] = argv[3];
		g_opcode209Parameters.u0[4] = argv[4];

		g_opcode209Parameters.u1[0] = argv[5];
		g_opcode209Parameters.u1[1] = argv[6];
		g_opcode209Parameters.u1[2] = argv[7];
		g_opcode209Parameters.u1[3] = argv[8];
		g_opcode209Parameters.u1[4] = argv[9];

		g_opcode209Parameters.stateVar = argv[10];

		g_opcode209Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_210(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used in Cards 2205 and 2207 (Cloud Orbs in Sirrus' Room)
	if (argc == 2) {
		uint16 soundId = argv[0];
		uint16 soundIdStopping = argv[1];

		// TODO: Work Out Function i.e. control Var etc.
		if (false) {
			_vm->_sound->replaceSoundMyst(soundId);
			_vm->_sound->replaceSoundMyst(soundIdStopping);
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_300(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Card 2218 (Telescope view)
	varUnusedCheck(op, var);
	// TODO: Fill in Logic. Clearing Variable for View?
}

} // End of namespace Mohawk
