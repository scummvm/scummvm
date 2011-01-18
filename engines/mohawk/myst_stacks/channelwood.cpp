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
	OPCODE(101, opcode_101);
	OPCODE(102, opcode_102);
	OPCODE(104, o_waterTankValveOpen);
	OPCODE(110, o_valveHandleMove1);
	OPCODE(111, o_valveHandleMoveStart1);
	OPCODE(112, o_valveHandleMoveStop);
	OPCODE(113, o_valveHandleMove2);
	OPCODE(114, o_valveHandleMoveStart2);
	OPCODE(115, o_valveHandleMove3);
	OPCODE(116, o_valveHandleMoveStart3);
	OPCODE(117, opcode_117);
	OPCODE(118, opcode_118);
	OPCODE(119, opcode_119);
	OPCODE(122, o_waterTankValveClose);
	OPCODE(127, opcode_127);
	OPCODE(129, opcode_129);

	// "Init" Opcodes
	OPCODE(201, opcode_201);
	OPCODE(202, o_pipeValve_init);
	OPCODE(203, opcode_203);

	// "Exit" Opcodes
	OPCODE(300, opcode_300);
}

#undef OPCODE

void MystScriptParser_Channelwood::disablePersistentScripts() {
	opcode_203_disable();
}

void MystScriptParser_Channelwood::runPersistentScripts() {
	opcode_203_run();
}

uint16 MystScriptParser_Channelwood::getVar(uint16 var) {
	switch(var) {
	case 1: // Water Pump Bridge Raised
		return _state.waterPumpBridgeState;
	case 2: // Lower Walkway to Upper Walkway Elevator Raised
		return _state.elevatorState;
	case 3: // Water Flowing To Lower Walkway To Upper Walkway Elevator
		return (_state.waterValveStates & 0xc0) ? 1 : 0;
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
//	case 18: // Sirrus's Room Bed Drawer Open
//		return 0;
//		return 1;
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
//	case 30: // Temple Door State
//		return 0;
//		return 1;
	case 32: // Sound - Water Flowing in Pipe to Book Room Elevator
		return ((_state.waterValveStates & 0xf8) == 0xb0 && _state.pipeState) ? 1 : 0;
//	case 102: // Sirrus's Desk Drawer / Red Page State
//		return 0; // Drawer Closed
//		return 1; // Drawer Open, Red Page Taken
//		return 2; // Drawer Open, Red Page Present
//	case 103: // Blue Page Present
//		return 0; // Blue Page Taken
//		return 1; // Blue Page Present
	default:
		return MystScriptParser::getVar(var);
	}
}

void MystScriptParser_Channelwood::toggleVar(uint16 var) {
	switch(var) {
	case 6: // Pipe Bridge Extended
		_state.pipeState ^= 1;
	default:
		MystScriptParser::toggleVar(var);
		break;
	}
}

bool MystScriptParser_Channelwood::setVarValue(uint16 var, uint16 value) {
	bool refresh = false;

	switch (var) {
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
//	case 18: // Sirrus's Room Bed Drawer Open
//	temp ^= 1;
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

void MystScriptParser_Channelwood::opcode_101(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		debugC(kDebugScript, "Opcode %d: Play Pipe Movie and Sound", op);

		uint16 soundId = argv[0];
		debugC(kDebugScript, "\tsoundId: %d", soundId);

		_vm->_sound->replaceSound(soundId);

		// TODO: Get Movie Location from Invoking Resource Rect, rather than
		//       hardcoded 267, 170 ?

		// TODO: Need version of playMovie blocking which allows selection
		//       of start and finish points.
		if (!_vm->_varStore->getVar(6)) {
			// Play Pipe Extending i.e. 0 to 1/2 way through file
			_vm->_video->playMovie(_vm->wrapMovieFilename("pipebrid", kChannelwoodStack), 267, 170);
		} else {
			// Play Pipe Retracting i.e. 1/2 way to end of file
			_vm->_video->playMovie(_vm->wrapMovieFilename("pipebrid", kChannelwoodStack), 267, 170);
		}
	} else
		unknown(op, var, argc, argv);
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
		_vm->_sound->replaceSound(soundId);
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
		_vm->_sound->replaceSound(soundId);

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
		_vm->_sound->replaceSound(soundId);
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
		_vm->_sound->replaceSound(soundId);
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
			_vm->_video->playMovie(_vm->wrapMovieFilename("monalgh", kChannelwoodStack), 227, 71);
			break;
		case 1:
			_vm->_video->playMovie(_vm->wrapMovieFilename("monamth", kChannelwoodStack), 227, 71);
			break;
		case 2:
			_vm->_video->playMovie(_vm->wrapMovieFilename("monasirs", kChannelwoodStack), 227, 71);
			break;
		case 3:
			_vm->_video->playMovie(_vm->wrapMovieFilename("monsmsg", kChannelwoodStack), 227, 71);
			break;
		default:
			warning("Opcode %d Control Variable Out of Range", op);
			break;
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Channelwood::opcode_118(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used on Card 3318 (Sirrus' Room Nightstand Drawer)
		// Triggered when clicked on drawer
		// TODO: Implement function...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Channelwood::opcode_119(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used on Card 3333 (Temple Hologram)
		// TODO: Not 100% sure about movie position...
		switch (_vm->_varStore->getVar(17)) {
		case 0:
			_vm->_video->playMovie(_vm->wrapMovieFilename("holoalgh", kChannelwoodStack), 126, 74);
			break;
		case 1:
			_vm->_video->playMovie(_vm->wrapMovieFilename("holoamth", kChannelwoodStack), 126, 74);
			break;
		case 2:
			_vm->_video->playMovie(_vm->wrapMovieFilename("holoasir", kChannelwoodStack), 126, 74);
			break;
		case 3:
			_vm->_video->playMovie(_vm->wrapMovieFilename("holosmsg", kChannelwoodStack), 126, 74);
			break;
		default:
			warning("Opcode %d Control Variable Out of Range", op);
			break;
		}
	} else
		unknown(op, var, argc, argv);
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

void MystScriptParser_Channelwood::opcode_127(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 2) {
		// Used by Card 3262 (Elevator)
		debugC(kDebugScript, "Opcode %d: Unknown...", op);

		uint16 u0 = argv[0];
		uint16 u1 = argv[1];

		debugC(kDebugScript, "\tu0: %d", u0);
		debugC(kDebugScript, "\tu1: %d", u1);

		// TODO: Fill in Code...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Channelwood::opcode_129(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		// Used by Card 3262 (Elevator)
		debugC(kDebugScript, "Opcode %d: Unknown...", op);

		uint16 cardId = argv[0];

		debugC(kDebugScript, "\tcardId: %d", cardId);

		// TODO: Fill in Code...
		_vm->changeToCard(cardId, true);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Channelwood::opcode_201(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Card 3247 (Elevator #1 Movement), 3161 (Bridge Movement), 3259 (Elevator #3 Movement) and 3252 (Elevator #2 Movement)
	if (argc == 0) {
		// TODO: Fill in Function. Video Playback? Rect from invoking hotspot resource...
		if (false) {
			// Card 3161
			_vm->_video->playMovie(_vm->wrapMovieFilename("bridge", kChannelwoodStack), 292, 204);

			// Card 3247
			_vm->_video->playMovie(_vm->wrapMovieFilename("welev1dn", kChannelwoodStack), 214, 107);
			_vm->_video->playMovie(_vm->wrapMovieFilename("welev1up", kChannelwoodStack), 214, 107);

			// Card 3252
			_vm->_video->playMovie(_vm->wrapMovieFilename("welev2dn", kChannelwoodStack), 215, 118);
			_vm->_video->playMovie(_vm->wrapMovieFilename("welev2up", kChannelwoodStack), 215, 118);

			// Card 3259
			_vm->_video->playMovie(_vm->wrapMovieFilename("welev3dn", kChannelwoodStack), 213, 99);
			_vm->_video->playMovie(_vm->wrapMovieFilename("welev3up", kChannelwoodStack), 213, 99);
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Channelwood::o_pipeValve_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Water valve init", op);
	_valveVar = var;
}

static struct {
	bool enabled;
} g_opcode203Parameters;

void MystScriptParser_Channelwood::opcode_203_run(void) {
	if (g_opcode203Parameters.enabled) {
		// Used for Card 3310 (Sirrus' Room Right Bed Drawer),
		// Card 3307 (Sirrus' Room Left Bed Drawer)
		// and Card 3318 (Sirrus' Room Nightstand Drawer)
		// TODO: Fill in Logic...
	}
}

void MystScriptParser_Channelwood::opcode_203_disable(void) {
	g_opcode203Parameters.enabled = false;
}

void MystScriptParser_Channelwood::opcode_203(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 3310 (Sirrus' Room Right Bed Drawer),
	// Card 3307 (Sirrus' Room Left Bed Drawer)
	// and Card 3318 (Sirrus' Room Nightstand Drawer)
	if (argc == 0)
		g_opcode203Parameters.enabled = true;
	else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Channelwood::opcode_300(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Card 3012 (Achenar's Holoprojector Control)
	varUnusedCheck(op, var);
	// TODO: Fill in Logic. Clearing Variable for View?
}

} // End of namespace Mohawk
