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
#include "mohawk/myst_stacks/channelwood.h"

#include "gui/message.h"

namespace Mohawk {

MystScriptParser_Channelwood::MystScriptParser_Channelwood(MohawkEngine_Myst *vm) :
		MystScriptParser(vm), _state(vm->_gameState->_channelwood) {
	setupOpcodes();
}

MystScriptParser_Channelwood::~MystScriptParser_Channelwood() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &MystScriptParser_Channelwood::x, #x))

void MystScriptParser_Channelwood::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, o_bridgeToggle);
	OPCODE(101, o_pipeExtend);
	OPCODE(102, opcode_102);
	OPCODE(104, o_waterTankValveOpen);
	OPCODE(105, o_leverStartMove);
	OPCODE(106, o_leverEndMove);
	OPCODE(107, o_leverMoveFail);
	OPCODE(108, o_leverMove);
	OPCODE(109, o_stairsDoorToggle);
	OPCODE(110, o_valveHandleMove1);
	OPCODE(111, o_valveHandleMoveStart1);
	OPCODE(112, o_valveHandleMoveStop);
	OPCODE(113, o_valveHandleMove2);
	OPCODE(114, o_valveHandleMoveStart2);
	OPCODE(115, o_valveHandleMove3);
	OPCODE(116, o_valveHandleMoveStart3);
	OPCODE(117, opcode_117);
	OPCODE(118, o_drawerOpen);
	OPCODE(119, opcode_119);
	OPCODE(122, o_waterTankValveClose);
	OPCODE(123, o_executeMouseUp);
	OPCODE(124, o_leverEndMoveWithSound);
	OPCODE(127, o_elevatorMovies);
	OPCODE(128, o_leverEndMoveResumeBackground);
	OPCODE(129, o_soundReplace);

	// "Init" Opcodes
	OPCODE(201, o_lever_init);
	OPCODE(202, o_pipeValve_init);
	OPCODE(203, o_drawer_init);

	// "Exit" Opcodes
	OPCODE(300, opcode_300);
}

#undef OPCODE

void MystScriptParser_Channelwood::disablePersistentScripts() {

}

void MystScriptParser_Channelwood::runPersistentScripts() {

}

uint16 MystScriptParser_Channelwood::getVar(uint16 var) {
	switch(var) {
	case 1: // Water Pump Bridge Raised
		return _state.waterPumpBridgeState;
	case 2: // Lower Walkway to Upper Walkway Elevator Raised
		return _state.elevatorState;
	case 3: // Water Flowing To Lower Walkway To Upper Walkway Elevator
		return (_state.waterValveStates & 0xc0) == 0xc0;
	case 4: // Water Flowing To Book Room Elevator
		return ((_state.waterValveStates & 0xf8) == 0xb0 && _state.pipeState) ? 1 : 0;
	case 5: // Lower Walkway to Upper Walkway Spiral Stair Lower Door State
		return _state.stairsLowerDoorState;
	case 6: // Pipe Bridge Extended
		return _state.pipeState;
	case 7: // Water Flowing To Water Pump For Bridge
		return ((_state.waterValveStates & 0xe2) == 0x82 || (_state.waterValveStates & 0xf4) == 0xa0) ? 1 : 0;
	case 8: // Water Tank Valve
		return (_state.waterValveStates & 0x80) ? 1 : 0;
	case 9: // State of First Water Valve
		return (_state.waterValveStates & 0x40) ? 1 : 0;
	case 10: // State of Second Water Valve
		return (_state.waterValveStates & 0x20) ? 1 : 0;
	case 11: // State of Right Third Water Valve
		return (_state.waterValveStates & 0x10) ? 1 : 0;
	case 12: // State of Right-Right Fourth Water Valve
		return (_state.waterValveStates & 0x08) ? 1 : 0;
	case 13: // State of Right-Left Fourth Water Valve
		return (_state.waterValveStates & 0x04) ? 1 : 0;
	case 14: // State of Left Third Water Valve
		return (_state.waterValveStates & 0x02) ? 1 : 0;
	case 15: // Water Flowing To Lower Walkway To Upper Walkway Elevator
		return ((_state.waterValveStates & 0xf8) == 0xb8) ? 1 : 0;
	case 16: // Channelwood Lower Walkway to Upper Walkway Spiral Stair Upper Door State
		return _state.stairsUpperDoorState;
	case 17: // Achenar's Holoprojector Selection
		return _state.holoprojectorSelection;
	case 18: // Sirrus's Room Bed Drawer Open
		return _siriusDrawerState;
	case 19: // Sound - Water Tank Valve
		return (_state.waterValveStates & 0x80) ? 1 : 0;
	case 20: // Sound - First Water Valve Water Flowing To Left
		return ((_state.waterValveStates & 0xc0) == 0x80) ? 1 : 0;
	case 21: // Sound - Second Water Valve Water Flowing To Right
		return ((_state.waterValveStates & 0xe0) == 0xa0) ? 1 : 0;
	case 22: // Sound - Right Third Water Valve Water Flowing To Right
		return ((_state.waterValveStates & 0xf0) == 0xb0) ? 1 : 0;
	case 23: // Sound - Right Third Water Valve Water Flowing To Left
		return ((_state.waterValveStates & 0xf0) == 0xa0) ? 1 : 0;
	case 24: // Sound - Second Water Valve Water Flowing To Left
		return ((_state.waterValveStates & 0xe0) == 0x80) ? 1 : 0;
	case 25: // Sound - Right-Right Fourth Valve Water Flowing To Left (To Pipe Bridge)
		return ((_state.waterValveStates & 0xf8) == 0xb0) ? 1 : 0;
	case 26: // Sound - Right-Left Fourth Valve Water Flowing To Right (To Pipe Down Tree)
		return ((_state.waterValveStates & 0xf4) == 0xa4) ? 1 : 0;
	case 27: // Sound - Right-Left Fourth Valve Water Flowing To Left (To Pipe Fork)
		return ((_state.waterValveStates & 0xf4) == 0xa0) ? 1 : 0;
	case 28: // Sound - Left Third Water Valve Flowing To Right (To Pipe Fork)
		return ((_state.waterValveStates & 0xe2) == 0x82) ? 1 : 0;
	case 29: // Sound - Left Third Water Valve Flowing To Left (To Pipe In Water)
		return ((_state.waterValveStates & 0xe2) == 0x80) ? 1 : 0;
	case 30: // Door State
		return _doorOpened;
	case 32: // Sound - Water Flowing in Pipe to Book Room Elevator
		return ((_state.waterValveStates & 0xf8) == 0xb0 && _state.pipeState) ? 1 : 0;
	case 33: // Channelwood Lower Walkway to Upper Walkway Spiral Stair Upper Door State
		if (_state.stairsUpperDoorState) {
			if (_tempVar == 1)
				return 2;
			else
				return 1;
		} else {
			return 0;
		}
	case 102: // Sirrus's Desk Drawer / Red Page State
		if (_siriusDrawerState) {
			if(!(_globals.redPagesInBook & 16) && (_globals.heldPage != 11))
				return 2; // Drawer Open, Red Page Present
			else
				return 1; // Drawer Open, Red Page Taken
		} else {
			return 0; // Drawer Closed
		}
	case 103: // Blue Page Present
		return !(_globals.bluePagesInBook & 16) && (_globals.heldPage != 5);
	default:
		return MystScriptParser::getVar(var);
	}
}

void MystScriptParser_Channelwood::toggleVar(uint16 var) {
	switch(var) {
	case 1: // Water Pump Bridge Raised
		_state.waterPumpBridgeState ^= 1;
		break;
	case 6: // Pipe Bridge Extended
		_state.pipeState ^= 1;
		break;
	case 16: // Channelwood Lower Walkway to Upper Walkway Spiral Stair Upper Door State
		_state.stairsUpperDoorState ^= 1;
		break;
	case 102: // Red page
		if (!(_globals.redPagesInBook & 16)) {
			if (_globals.heldPage == 11)
				_globals.heldPage = 0;
			else
				_globals.heldPage = 11;
		}
		break;
	case 103: // Blue page
		if (!(_globals.bluePagesInBook & 16)) {
			if (_globals.heldPage == 5)
				_globals.heldPage = 0;
			else
				_globals.heldPage = 5;
		}
		break;
	default:
		MystScriptParser::toggleVar(var);
		break;
	}
}

bool MystScriptParser_Channelwood::setVarValue(uint16 var, uint16 value) {
	bool refresh = false;

	switch (var) {
	case 2: // Lower Walkway to Upper Walkway Elevator Raised
		if (_state.elevatorState != value) {
			_state.elevatorState = value;
			refresh = true;
		}
		break;
	case 5: // Lower Walkway to Upper Walkway Spiral Stair Lower Door State
		if (_state.stairsLowerDoorState != value) {
			_state.stairsLowerDoorState = value;
			refresh = true;
		}
		break;
	case 9:
		refresh = pipeChangeValve(value, 0x40);
		break;
	case 10:
		refresh = pipeChangeValve(value, 0x20);
		break;
	case 11:
		refresh = pipeChangeValve(value, 0x10);
		break;
	case 12:
		refresh = pipeChangeValve(value, 0x08);
		break;
	case 13:
		refresh = pipeChangeValve(value, 0x04);
		break;
	case 14:
		refresh = pipeChangeValve(value, 0x02);
		break;
	case 18: // Sirrus's Room Bed Drawer Open
		if (_siriusDrawerState != value) {
			_siriusDrawerState = value;
			refresh = true;
		}
		break;
	case 30: // Door opened
		_doorOpened = value;
		break;
	default:
		refresh = MystScriptParser::setVarValue(var, value);
		break;
	}

	return refresh;
}

bool MystScriptParser_Channelwood::pipeChangeValve(bool open, uint16 mask) {
	if (open) {
		if (!(_state.waterValveStates & mask)) {
			_state.waterValveStates |= mask;
			return true;
		}
	} else {
		if (_state.waterValveStates & mask) {
			_state.waterValveStates &= ~mask;
			return true;
		}
	}

	return false;
}

void MystScriptParser_Channelwood::o_bridgeToggle(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Bridge rise / skink video", op);

	VideoHandle bridge = _vm->_video->playMovie(_vm->wrapMovieFilename("bridge", kChannelwoodStack), 292, 203);

	// Toggle bridge state
	if (_state.waterPumpBridgeState)
		_vm->_video->setVideoBounds(bridge, Graphics::VideoTimestamp(3050, 600), Graphics::VideoTimestamp(6100, 600));
	else
		_vm->_video->setVideoBounds(bridge, Graphics::VideoTimestamp(0, 600), Graphics::VideoTimestamp(3050, 600));

	_vm->_video->waitUntilMovieEnds(bridge);
}

void MystScriptParser_Channelwood::o_pipeExtend(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Play Pipe Movie and Sound", op);

	uint16 soundId = argv[0];
	debugC(kDebugScript, "\tsoundId: %d", soundId);

	_vm->_sound->replaceSoundMyst(soundId);
	VideoHandle pipe = _vm->_video->playMovie(_vm->wrapMovieFilename("pipebrid", kChannelwoodStack), 267, 170);

	// Toggle pipe state
	if (_state.pipeState)
		_vm->_video->setVideoBounds(pipe, Graphics::VideoTimestamp(3040, 600), Graphics::VideoTimestamp(6080, 600));
	else
		_vm->_video->setVideoBounds(pipe, Graphics::VideoTimestamp(0, 600), Graphics::VideoTimestamp(3040, 600));

	_vm->_video->waitUntilMovieEnds(pipe);
	_vm->_sound->resumeBackgroundMyst();
}

void MystScriptParser_Channelwood::opcode_102(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 2 || argc == 3) {
		debugC(kDebugScript, "Opcode %d: Draw Full Screen Image, Optional Delay and Change Card", op);

		uint16 imageId = argv[0];
		uint16 cardId = argv[1];
		uint16 delay = 0;

		if (argc == 3)
			delay = argv[2]; // TODO: Not sure about purpose of this parameter...

		debugC(kDebugScript, "\timageId: %d", imageId);
		debugC(kDebugScript, "\tcardId: %d", cardId);
		debugC(kDebugScript, "\tdelay: %d", delay);

		_vm->_gfx->copyImageToScreen(imageId, Common::Rect(0, 0, 544, 333));
		_vm->_system->delayMillis(delay / 100);
		_vm->changeToCard(cardId, true);
	} else
		unknown(op, var, argc, argv);
}


void MystScriptParser_Channelwood::o_waterTankValveOpen(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Do Water Tank Valve Open Animation", op);
	Common::Rect rect = _invokingResource->getRect();

	for (uint i = 0; i < 2; i++)
		for (uint16 imageId = 3601; imageId >= 3595; imageId--) {
			_vm->_gfx->copyImageToScreen(imageId, rect);
			_vm->_system->updateScreen();
			_vm->_system->delayMillis(5);
		}

	pipeChangeValve(true, 0x80);
}

void MystScriptParser_Channelwood::o_leverStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generic lever start move", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);
	lever->drawFrame(0);
	_vm->_cursor->setCursor(700);
	_leverPulled = false;
}

void MystScriptParser_Channelwood::o_leverMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generic lever move", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	// Make the handle follow the mouse
	int16 maxStep = lever->getStepsV() - 1;
	Common::Rect rect = lever->getRect();
	int16 step = ((_vm->_mouse.y - rect.top) * lever->getStepsV()) / rect.height();
	step = CLIP<int16>(step, 0, maxStep);

	lever->drawFrame(step);

	if (step == maxStep) {
		if (!_leverPulled) {
			_leverPulled = true;
			_leverAction->handleMouseUp(_vm->_system->getEventManager()->getMousePos());
		}
	} else {
		_leverPulled = false;
	}
}

void MystScriptParser_Channelwood::o_leverMoveFail(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generic lever move", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	// Make the handle follow the mouse
	int16 maxStep = lever->getStepsV() - 1;
	Common::Rect rect = lever->getRect();
	int16 step = ((_vm->_mouse.y - rect.top) * lever->getStepsV()) / rect.height();
	step = CLIP<int16>(step, 0, maxStep);

	lever->drawFrame(step);

	if (step == maxStep) {
		if (!_leverPulled) {
			_leverPulled = true;
			uint16 soundId = lever->getList2(0);
			if (soundId)
				_vm->_sound->replaceSoundMyst(soundId);
		}
	} else {
		_leverPulled = false;
	}
}

void MystScriptParser_Channelwood::o_leverEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generic lever end move", op);

	// Get current lever frame
	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);
	int16 maxStep = lever->getStepsV() - 1;
	Common::Rect rect = lever->getRect();
	int16 step = ((_vm->_mouse.y - rect.top) * lever->getStepsV()) / rect.height();
	step = CLIP<int16>(step, 0, maxStep);

	// Release lever
	for (int i = step; i >= 0; i--) {
		lever->drawFrame(i);
		_vm->_system->delayMillis(10);
	}

	uint16 soundId = lever->getList3(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);

	_vm->checkCursorHints();
}

void MystScriptParser_Channelwood::o_leverEndMoveResumeBackground(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_vm->_sound->resumeBackgroundMyst();
	o_leverEndMove(op, var, argc, argv);
}

void MystScriptParser_Channelwood::o_leverEndMoveWithSound(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	o_leverEndMove(op, var, argc, argv);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);
	uint16 soundId = lever->getList3(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);
}

void MystScriptParser_Channelwood::o_stairsDoorToggle(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Play stairs door video", op);

	MystResourceType6 *movie = static_cast<MystResourceType6 *>(_invokingResource);

	if (_state.stairsUpperDoorState) {
		// TODO: Play backwards
		movie->playMovie();
	} else {
		movie->playMovie();
	}
}

void MystScriptParser_Channelwood::o_valveHandleMove1(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Valve handle move", op);

	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	if (handle->getRect().contains(_vm->_mouse)) {
		// Compute frame to draw
		_tempVar = (_vm->_mouse.x - 250) / 4;
		_tempVar = CLIP<int16>(_tempVar, 1, handle->getStepsH() - 2);

		// Draw frame
		handle->drawFrame(_tempVar);
	}
}

void MystScriptParser_Channelwood::o_valveHandleMoveStart1(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Valve handle move start", op);

	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	uint16 soundId = handle->getList1(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);
	_vm->_cursor->setCursor(700);

	o_valveHandleMove1(op, var, argc, argv);
}

void MystScriptParser_Channelwood::o_valveHandleMoveStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Valve handle move stop", op);

	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);

	// Update state with valve position
	if (_tempVar <= 5)
		setVarValue(_valveVar, 1);
	else
		setVarValue(_valveVar, 0);

	// Play release sound
	uint16 soundId = handle->getList3(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);

	// Redraw valve
	_vm->redrawArea(_valveVar);

	// Restore cursor
	_vm->checkCursorHints();
}

void MystScriptParser_Channelwood::o_valveHandleMove2(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Valve handle move", op);

	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	if (handle->getRect().contains(_vm->_mouse)) {
		// Compute frame to draw
		_tempVar = handle->getStepsH() - (_vm->_mouse.x - 234) / 4;
		_tempVar = CLIP<int16>(_tempVar, 1, handle->getStepsH() - 2);

		// Draw frame
		handle->drawFrame(_tempVar);
	}
}

void MystScriptParser_Channelwood::o_valveHandleMoveStart2(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Valve handle move start", op);

	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	uint16 soundId = handle->getList1(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);
	_vm->_cursor->setCursor(700);

	o_valveHandleMove2(op, var, argc, argv);
}

void MystScriptParser_Channelwood::o_valveHandleMove3(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Valve handle move", op);

	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	if (handle->getRect().contains(_vm->_mouse)) {
		// Compute frame to draw
		_tempVar = handle->getStepsH() - (_vm->_mouse.x - 250) / 4;
		_tempVar = CLIP<int16>(_tempVar, 1, handle->getStepsH() - 2);

		// Draw frame
		handle->drawFrame(_tempVar);
	}
}

void MystScriptParser_Channelwood::o_valveHandleMoveStart3(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Valve handle move start", op);

	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	uint16 soundId = handle->getList1(0);
	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);
	_vm->_cursor->setCursor(700);

	o_valveHandleMove3(op, var, argc, argv);
}

void MystScriptParser_Channelwood::opcode_117(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		// Used on Card 3012 (Temple Hologram Monitor)
		uint16 button = argv[0]; // 0 to 3
		_vm->_varStore->setVar(17, button);

		switch (button) {
		case 0:
			_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("monalgh", kChannelwoodStack), 227, 71);
			break;
		case 1:
			_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("monamth", kChannelwoodStack), 227, 71);
			break;
		case 2:
			_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("monasirs", kChannelwoodStack), 227, 71);
			break;
		case 3:
			_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("monsmsg", kChannelwoodStack), 227, 71);
			break;
		default:
			warning("Opcode %d Control Variable Out of Range", op);
			break;
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Channelwood::o_drawerOpen(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Open Sirius drawer", op);

	_siriusDrawerState = 1;
	_vm->redrawArea(18, false);
	_vm->redrawArea(102, false);
}

void MystScriptParser_Channelwood::opcode_119(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used on Card 3333 (Temple Hologram)
		// TODO: Not 100% sure about movie position...
		switch (_vm->_varStore->getVar(17)) {
		case 0:
			_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("holoalgh", kChannelwoodStack), 126, 74);
			break;
		case 1:
			_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("holoamth", kChannelwoodStack), 126, 74);
			break;
		case 2:
			_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("holoasir", kChannelwoodStack), 126, 74);
			break;
		case 3:
			_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("holosmsg", kChannelwoodStack), 126, 74);
			break;
		default:
			warning("Opcode %d Control Variable Out of Range", op);
			break;
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Channelwood::o_executeMouseUp(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Execute mouse up", op);

	MystResourceType5 *resource = static_cast<MystResourceType5 *>(_vm->_resources[argv[0]]);
	resource->handleMouseUp(_vm->_system->getEventManager()->getMousePos());
}

void MystScriptParser_Channelwood::o_waterTankValveClose(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Do Water Tank Valve Close Animation", op);
	Common::Rect rect = _invokingResource->getRect();

	for (uint i = 0; i < 2; i++)
		for (uint16 imageId = 3595; imageId <= 3601; imageId++) {
			_vm->_gfx->copyImageToScreen(imageId, rect);
			_vm->_system->updateScreen();
			_vm->_system->delayMillis(5);
		}

	pipeChangeValve(false, 0x80);
}

void MystScriptParser_Channelwood::o_elevatorMovies(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used by Card 3262 (Elevator)
	debugC(kDebugScript, "Opcode %d: Elevator movie", op);

	uint16 elevator = argv[0];
	uint16 direction = argv[1];

	Common::String movie;
	uint16 x;
	uint16 y;

	switch (elevator) {
	case 1:
		x = 214;
		y = 106;
		if (direction == 1)
			movie = _vm->wrapMovieFilename("welev1up", kChannelwoodStack);
		else
			movie = _vm->wrapMovieFilename("welev1dn", kChannelwoodStack);
		break;
	case 2:
		x = 215;
		y = 117;
		if (direction == 1)
			movie = _vm->wrapMovieFilename("welev2up", kChannelwoodStack);
		else
			movie = _vm->wrapMovieFilename("welev2dn", kChannelwoodStack);
		break;
	case 3:
		x = 213;
		y = 98;
		if (direction == 1)
			movie = _vm->wrapMovieFilename("welev3up", kChannelwoodStack);
		else
			movie = _vm->wrapMovieFilename("welev3dn", kChannelwoodStack);
		break;
	}

	_vm->_sound->pauseBackgroundMyst();
	_vm->_video->playMovieBlocking(movie, x, y);
	_vm->_sound->resumeBackgroundMyst();
}

void MystScriptParser_Channelwood::o_soundReplace(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Play sound if not already playing", op);

	uint16 soundId = argv[0];

	// TODO: If is foreground playing
	_vm->_sound->replaceSoundMyst(soundId);
}

void MystScriptParser_Channelwood::o_lever_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generic lever init", op);
	_leverAction = static_cast<MystResourceType5 *>(_invokingResource);
}

void MystScriptParser_Channelwood::o_pipeValve_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Water valve init", op);
	_valveVar = var;
}

void MystScriptParser_Channelwood::o_drawer_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_siriusDrawerState = 0;
}

void MystScriptParser_Channelwood::opcode_300(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Card 3012 (Achenar's Holoprojector Control)
	varUnusedCheck(op, var);
	// TODO: Fill in Logic. Clearing Variable for View?
}

} // End of namespace Mohawk
