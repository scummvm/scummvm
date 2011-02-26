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
#include "mohawk/myst_stacks/mechanical.h"

#include "gui/message.h"

namespace Mohawk {
namespace MystStacks {

Mechanical::Mechanical(MohawkEngine_Myst *vm) :
		MystScriptParser(vm), _state(vm->_gameState->_mechanical) {
	setupOpcodes();

	_mystStaircaseState = false;
	_fortressPosition = 0;
}

Mechanical::~Mechanical() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &Mechanical::x, #x))

void Mechanical::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, o_throneEnablePassage);
	OPCODE(104, o_snakeBoxTrigger);
	OPCODE(105, o_fortressStaircaseMovie);
	OPCODE(106, o_elevatorRotationStart);
	OPCODE(107, o_elevatorRotationMove);
	OPCODE(108, o_elevatorRotationStop);
	OPCODE(121, o_elevatorWindowMovie);
	OPCODE(122, opcode_122);
	OPCODE(123, o_elevatorTopMovie);
	OPCODE(124, opcode_124);
	OPCODE(125, o_mystStaircaseMovie);
	OPCODE(126, opcode_126);
	OPCODE(127, o_crystalEnterYellow);
	OPCODE(128, o_crystalLeaveYellow);
	OPCODE(129, o_crystalEnterGreen);
	OPCODE(130, o_crystalLeaveGreen);
	OPCODE(131, o_crystalEnterRed);
	OPCODE(132, o_crystalLeaveRed);

	// "Init" Opcodes
	OPCODE(200, o_throne_init);
	OPCODE(201, o_fortressStaircase_init);
	OPCODE(202, opcode_202);
	OPCODE(203, o_snakeBox_init);
	OPCODE(204, o_elevatorRotation_init);
	OPCODE(205, opcode_205);
	OPCODE(206, opcode_206);
	OPCODE(209, opcode_209);

	// "Exit" Opcodes
	OPCODE(300, opcode_300);
}

#undef OPCODE

void Mechanical::disablePersistentScripts() {
	opcode_202_disable();
	opcode_205_disable();
	opcode_206_disable();
	opcode_209_disable();
}

void Mechanical::runPersistentScripts() {
	opcode_202_run();

	if (_elevatorRotationLeverMoving)
		elevatorRotation_run();

	opcode_205_run();
	opcode_206_run();
	opcode_209_run();
}

uint16 Mechanical::getVar(uint16 var) {
	switch(var) {
	case 0: // Sirrus's Secret Panel State
		return _state.sirrusPanelState;
	case 1: // Achenar's Secret Panel State
		return _state.achenarPanelState;
	case 2: // Achenar's Secret Room Crate Lid Open and Blue Page Present
		if (_state.achenarCrateOpened) {
			if (_globals.bluePagesInBook & 4 || _globals.heldPage == 3)
				return 2;
			else
				return 3;
		} else {
			return _globals.bluePagesInBook & 4 || _globals.heldPage == 3;
		}
	case 3: // Achenar's Secret Room Crate State
		return _state.achenarCrateOpened;
	case 4: // Myst Book Room Staircase State
		return _mystStaircaseState;
	case 5: // Fortress Position
		return _fortressPosition;
	case 6: // Fortress Position - Big Cog Visible Through Doorway
		return _fortressPosition == 0;
	case 7: // Fortress Elevator Open
		if (_state.elevatorRotation == 4)
			return 1; // Open
		else
			return 0; // Closed
	case 10: // Fortress Staircase State
		return _state.staircaseState;
	case 11: // Fortress Elevator Rotation Position
		return _state.elevatorRotation;
	case 12: // Fortress Elevator Rotation Cog Position
		return 5 - (uint16)(_elevatorRotationGearPosition + 0.5) % 6;
	case 14: // Elevator going down when at top
		return _elevatorGoingDown; // TODO add too late value (2)
	case 15: // Code Lock Execute Button Script
		if (_mystStaircaseState)
			return 0;
		else if (_state.codeShape[0] == 2 && _state.codeShape[1] == 8
				&& _state.codeShape[2] == 5 && _state.codeShape[3] == 1)
			return 1;
		else
			return 2;
	case 16: // Code Lock Shape #1 - Left
	case 17: // Code Lock Shape #2
	case 18: // Code Lock Shape #3
	case 19: // Code Lock Shape #4 - Right
		return _state.codeShape[var - 16];
	case 20: // Crystal Lit Flag - Yellow
		return _crystalLit == 3;
	case 21: // Crystal Lit Flag - Green
		return _crystalLit == 1;
	case 22: // Crystal Lit Flag - Red
		return _crystalLit == 2;
	case 102: // Red page
		return !(_globals.redPagesInBook & 4) && (_globals.heldPage != 9);
	case 103: // Blue page
		return !(_globals.bluePagesInBook & 4) && (_globals.heldPage != 3);
	default:
		return MystScriptParser::getVar(var);
	}
}

void Mechanical::toggleVar(uint16 var) {
	switch(var) {
	case 0: // Sirrus's Secret Panel State
		_state.sirrusPanelState ^= 1;
	case 1: // Achenar's Secret Panel State
		_state.achenarPanelState ^= 1;
	case 3: // Achenar's Secret Room Crate State
		_state.achenarCrateOpened ^= 1;
	case 4: // Myst Book Room Staircase State
		_mystStaircaseState ^= 1;
	case 10: // Fortress Staircase State
		_state.staircaseState ^= 1;
	case 16: // Code Lock Shape #1 - Left
	case 17: // Code Lock Shape #2
	case 18: // Code Lock Shape #3
	case 19: // Code Lock Shape #4 - Right
		_state.codeShape[var - 16] = (_state.codeShape[var - 16] + 1) % 10;
		break;
	case 102: // Red page
		if (!(_globals.redPagesInBook & 4)) {
			if (_globals.heldPage == 9)
				_globals.heldPage = 0;
			else
				_globals.heldPage = 9;
		}
		break;
	case 103: // Blue page
		if (!(_globals.bluePagesInBook & 4)) {
			if (_globals.heldPage == 3)
				_globals.heldPage = 0;
			else
				_globals.heldPage = 3;
		}
		break;
	default:
		MystScriptParser::toggleVar(var);
		break;
	}
}

bool Mechanical::setVarValue(uint16 var, uint16 value) {
	bool refresh = false;

	switch (var) {
	case 14: // Elevator going down when at top
		_elevatorGoingDown = value;
		break;
	default:
		refresh = MystScriptParser::setVarValue(var, value);
		break;
	}

	return refresh;
}

void Mechanical::o_throneEnablePassage(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Enable throne passage", op);

	_vm->_resources[argv[0]]->setEnabled(getVar(var));
}

void Mechanical::o_snakeBoxTrigger(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Trigger Playing Of Snake Movie", op);

	// Used on Mechanical Card 6043 (Weapons Rack with Snake Box)
	_snakeBox->playMovie();
}

void Mechanical::o_fortressStaircaseMovie(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Play Stairs Movement Movie", op);

	VideoHandle staircase = _vm->_video->playMovie(_vm->wrapMovieFilename("hhstairs", kMechanicalStack), 174, 222);

	if (_state.staircaseState) {
		_vm->_video->setVideoBounds(staircase, Audio::Timestamp(0, 840, 600), Audio::Timestamp(0, 1680, 600));
	} else {
		_vm->_video->setVideoBounds(staircase, Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 840, 600));
	}

	_vm->_video->waitUntilMovieEnds(staircase);
}

void Mechanical::o_elevatorRotationStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Elevator rotation lever start", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);
	lever->drawFrame(0);

	_elevatorRotationLeverMoving = true;
	_elevatorRotationSpeed = 0;

	_vm->_sound->stopBackgroundMyst();

	_vm->_cursor->setCursor(700);
}

void Mechanical::o_elevatorRotationMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Elevator rotation lever move", op);

	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();
	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	// Make the handle follow the mouse
	int16 maxStep = lever->getNumFrames() - 1;
	Common::Rect rect = lever->getRect();
	int16 step = ((rect.bottom - mouse.y) * lever->getNumFrames()) / rect.height();
	step = CLIP<int16>(step, 0, maxStep);

	_elevatorRotationSpeed = step * 0.1;

	// Draw current frame
	lever->drawFrame(step);
}

void Mechanical::o_elevatorRotationStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Elevator rotation lever stop", op);

	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();
	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	// Get current lever frame
	int16 maxStep = lever->getNumFrames() - 1;
	Common::Rect rect = lever->getRect();
	int16 step = ((rect.bottom - mouse.y) * lever->getNumFrames()) / rect.height();
	step = CLIP<int16>(step, 0, maxStep);

	// Release lever
	for (int i = step; i >= 0; i--) {
		lever->drawFrame(i);
		_vm->_system->delayMillis(10);
	}

	// Stop persistent script
	_elevatorRotationLeverMoving = false;

	float speed = _elevatorRotationSpeed * 10;

	if (speed > 0) {

		// Decrease speed
		while (speed > 2) {
			speed -= 0.5;

			_elevatorRotationGearPosition += speed * 0.1;

			if (_elevatorRotationGearPosition > 12)
				break;

			_vm->redrawArea(12);
			_vm->_system->delayMillis(100);
		}

		// Increment position
		_state.elevatorRotation = (_state.elevatorRotation + 1) % 10;

		_vm->_sound->replaceSoundMyst(_elevatorRotationSoundId);
		_vm->redrawArea(11);
	}

	_vm->checkCursorHints();
}

void Mechanical::o_elevatorWindowMovie(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 startTime = argv[0];
	uint16 endTime = argv[1];

	debugC(kDebugScript, "Opcode %d Movie Time Index %d to %d", op, startTime, endTime);

	VideoHandle window = _vm->_video->playMovie(_vm->wrapMovieFilename("ewindow", kMechanicalStack), 253, 0);
	_vm->_video->setVideoBounds(window, Audio::Timestamp(0, startTime, 600), Audio::Timestamp(0, endTime, 600));
	_vm->_video->waitUntilMovieEnds(window);
}

void Mechanical::opcode_122(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 0) {
		// Used on Card 6120 (Elevator)
		// Called when Exit Midde Button Pressed

		// TODO: hcelev? Movie of Elevator?
	} else
		unknown(op, var, argc, argv);
}

void Mechanical::o_elevatorTopMovie(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 startTime = argv[0];
	uint16 endTime = argv[1];

	debugC(kDebugScript, "Opcode %d Movie Time Index %d to %d", op, startTime, endTime);

	VideoHandle window = _vm->_video->playMovie(_vm->wrapMovieFilename("hcelev", kMechanicalStack), 206, 38);
	_vm->_video->setVideoBounds(window, Audio::Timestamp(0, startTime, 600), Audio::Timestamp(0, endTime, 600));
	_vm->_video->waitUntilMovieEnds(window);
}

void Mechanical::opcode_124(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used by Card 6156 (Fortress Rotation Controls)
		// Called when Red Exit Button Pressed to raise Elevator

		// TODO: Fill in Code...
	} else
		unknown(op, var, argc, argv);
}

void Mechanical::o_mystStaircaseMovie(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Myst book staircase video", op);

	_vm->_video->playMovieBlocking(_vm->wrapMovieFilename("sstairs", kMechanicalStack), 199, 108);
}

void Mechanical::opcode_126(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used by Card 6120 (Fortress Elevator)
		// Called when Red Exit Button Pressed to raise Elevator and
		// exit is clicked...

		// TODO: Fill in Code...
	} else
		unknown(op, var, argc, argv);
}

void Mechanical::o_crystalEnterYellow(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Crystal enter", op);

	_crystalLit = 3;
	_vm->redrawArea(20);
}

void Mechanical::o_crystalEnterGreen(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Crystal enter", op);

	_crystalLit = 1;
	_vm->redrawArea(21);
}

void Mechanical::o_crystalEnterRed(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Crystal enter", op);

	_crystalLit = 2;
	_vm->redrawArea(22);
}

void Mechanical::o_crystalLeaveYellow(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Crystal leave", op);

	_crystalLit = 0;
	_vm->redrawArea(20);
}

void Mechanical::o_crystalLeaveGreen(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Crystal leave", op);

	_crystalLit = 0;
	_vm->redrawArea(21);
}

void Mechanical::o_crystalLeaveRed(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Crystal leave", op);

	_crystalLit = 0;
	_vm->redrawArea(22);
}

void Mechanical::o_throne_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 6238 (Sirrus' Throne) and Card 6027 (Achenar's Throne)
	debugC(kDebugScript, "Opcode %d: Brother throne init", op);

	_invokingResource->setEnabled(getVar(var));
}

void Mechanical::o_fortressStaircase_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Staircase init", op);

	_vm->_resources[argv[0]]->setEnabled(!_state.staircaseState);
	_vm->_resources[argv[1]]->setEnabled(!_state.staircaseState);
	_vm->_resources[argv[2]]->setEnabled(_state.staircaseState);
}

static struct {
	bool enabled;
} g_opcode202Parameters;

void Mechanical::opcode_202_run() {
	// Used for Card 6220 (Sirrus' Mechanical Bird)
	// TODO: Fill in Function
}

void Mechanical::opcode_202_disable() {
	g_opcode202Parameters.enabled = false;
}

void Mechanical::opcode_202(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Card 6220 (Sirrus' Mechanical Bird)
	if (argc == 0)
		g_opcode202Parameters.enabled = true;
	else
		unknown(op, var, argc, argv);
}

void Mechanical::o_snakeBox_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Snake box init", op);

	_snakeBox = static_cast<MystResourceType6 *>(_invokingResource);
}

void Mechanical::elevatorRotation_run() {
	_vm->redrawArea(12);

	_elevatorRotationGearPosition += _elevatorRotationSpeed;

	if (_elevatorRotationGearPosition > 12) {
		uint16 position = (uint16)_elevatorRotationGearPosition;
		_elevatorRotationGearPosition = _elevatorRotationGearPosition - position + position % 6;

		_state.elevatorRotation = (_state.elevatorRotation + 1) % 10;

		_vm->_sound->replaceSoundMyst(_elevatorRotationSoundId);
		_vm->redrawArea(11);
		_vm->_system->delayMillis(100);
	}
}

void Mechanical::o_elevatorRotation_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Elevator rotation init", op);

	_elevatorRotationSoundId = argv[0];
	_elevatorRotationGearPosition = 0;
	_elevatorRotationLeverMoving = false;
}

static struct {
	uint16 soundIdPosition[4];

	bool enabled;
} g_opcode205Parameters;

void Mechanical::opcode_205_run() {
	// Used for Card 6156 (Fortress Rotation Controls)
	// TODO: Fill in function...
	// g_opcode205Parameters.soundIdPosition[4]
}

void Mechanical::opcode_205_disable() {
	g_opcode205Parameters.enabled = false;
}

void Mechanical::opcode_205(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 6156 (Fortress Rotation Controls)

	if (argc == 4) {
		g_opcode205Parameters.soundIdPosition[0] = argv[0];
		g_opcode205Parameters.soundIdPosition[1] = argv[1];
		g_opcode205Parameters.soundIdPosition[2] = argv[2];
		g_opcode205Parameters.soundIdPosition[3] = argv[3];

		g_opcode205Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	uint16 soundIdStart[2];
	uint16 soundIdPosition[4];

	bool enabled;
} g_opcode206Parameters;

void Mechanical::opcode_206_run() {
	if (g_opcode206Parameters.enabled) {
		// Used for Card 6044 (Fortress Rotation Simulator)

		// g_opcode206Parameters.soundIdStart[2]
		// g_opcode206Parameters.soundIdPosition[4]

		// TODO: Fill in function...
	}
}

void Mechanical::opcode_206_disable() {
	g_opcode206Parameters.enabled = false;
}

void Mechanical::opcode_206(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 6044 (Fortress Rotation Simulator)
	if (argc == 6) {
		g_opcode206Parameters.soundIdStart[0] = argv[0];
		g_opcode206Parameters.soundIdStart[1] = argv[1];
		g_opcode206Parameters.soundIdPosition[0] = argv[2];
		g_opcode206Parameters.soundIdPosition[1] = argv[3];
		g_opcode206Parameters.soundIdPosition[2] = argv[4];
		g_opcode206Parameters.soundIdPosition[3] = argv[5];

		g_opcode206Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}


static struct {
	bool enabled;
} g_opcode209Parameters;

void Mechanical::opcode_209_run() {
	// Used for Card 6044 (Fortress Rotation Simulator)

	// TODO: Implement Function For Secret Panel State as
	//       per Opcode 200 function (Mechanical)
}

void Mechanical::opcode_209_disable() {
	g_opcode209Parameters.enabled = false;
}

void Mechanical::opcode_209(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 6044 (Fortress Rotation Simulator)
	if (argc == 0)
		g_opcode209Parameters.enabled = true;
	else
		unknown(op, var, argc, argv);
}

void Mechanical::opcode_300(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Card 6156 (Fortress Elevator View)
	varUnusedCheck(op, var);
	// TODO: Fill in Logic. Clearing Variable for View?
}

} // End of namespace MystStacks
} // End of namespace Mohawk
