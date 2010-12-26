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
#include "mohawk/myst_stacks/mechanical.h"

#include "gui/message.h"

namespace Mohawk {

MystScriptParser_Mechanical::MystScriptParser_Mechanical(MohawkEngine_Myst *vm) :
		MystScriptParser(vm), _state(vm->_gameState->_mechanical) {
	setupOpcodes();
}

MystScriptParser_Mechanical::~MystScriptParser_Mechanical() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &MystScriptParser_Mechanical::x, #x))

void MystScriptParser_Mechanical::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(104, opcode_104);
	OPCODE(105, opcode_105);
	OPCODE(121, opcode_121);
	OPCODE(122, opcode_122);
	OPCODE(123, opcode_123);
	OPCODE(124, opcode_124);
	OPCODE(125, opcode_125);
	OPCODE(126, opcode_126);
	OPCODE(127, opcode_127);
	OPCODE(128, opcode_128);
	OPCODE(129, opcode_129);
	OPCODE(130, opcode_130);
	OPCODE(131, opcode_131);
	OPCODE(132, opcode_132);

	// "Init" Opcodes
	OPCODE(200, opcode_200);
	OPCODE(201, opcode_201);
	OPCODE(202, opcode_202);
	OPCODE(203, opcode_203);
	OPCODE(204, opcode_204);
	OPCODE(205, opcode_205);
	OPCODE(206, opcode_206);
	OPCODE(209, opcode_209);

	// "Exit" Opcodes
	OPCODE(300, opcode_300);
}

#undef OPCODE

void MystScriptParser_Mechanical::disablePersistentScripts() {
	opcode_200_disable();
	opcode_201_disable();
	opcode_202_disable();
	opcode_203_disable();
	opcode_204_disable();
	opcode_205_disable();
	opcode_206_disable();
	opcode_209_disable();
}

void MystScriptParser_Mechanical::runPersistentScripts() {
	opcode_200_run();
	opcode_201_run();
	opcode_202_run();
	opcode_203_run();
	opcode_204_run();
	opcode_205_run();
	opcode_206_run();
	opcode_209_run();
}

uint16 MystScriptParser_Mechanical::getVar(uint16 var) {
	switch(var) {
	case 0: // Sirrus's Secret Panel State
		return _state.sirrusPanelState;
	case 1: // Achenar's Secret Panel State
		return _state.achenarPanelState;
//	case 3: // Sirrus's Secret Room Crate State
//		return 0;
//		return 1;
//	case 4: // Myst Book Room Staircase State
//		return 0; // Staircase Up
//		return 1; // Staircase Down
//	case 5: // Fortress Position
//		return 0; // Island with Code Lock
//		return 1; // Island with Last Two Symbols of Code
//		return 2; // Island with First Two Symbols of Code
//		return 3; // No Island
//	case 6: // Fortress Position - Big Cog Visible Through Doorway
//		return 0;
//		return 1;
	case 7: // Fortress Elevator Open
		if (_state.elevatorRotation == 4)
			return 1; // Open
		else
			return 0; // Closed
	case 10: // Fortress Staircase State
		return _state.staircaseState;
	case 11: // Fortress Elevator Rotation Position
		return _state.elevatorRotation;
//	case 12: // Fortress Elevator Rotation Cog Position
//		return 0;
//		return 1;
//		return 2;
//		return 3;
//		return 4;
//		return 5;
//	case 15: // Code Lock Execute Button State(?)
//		return 0;
//		return 1;
//		return 2;
	case 16: // Code Lock Shape #1 - Left
	case 17: // Code Lock Shape #2
	case 18: // Code Lock Shape #3
	case 19: // Code Lock Shape #4 - Right
		return _state.codeShape[var - 16];
//	case 20: // Crystal Lit Flag - Yellow
//		return 0;
//		return 1;
//	case 21: // Crystal Lit Flag - Green
//		return 0;
//		return 1;
//	case 22: // Crystal Lit Flag - Red
//		return 0;
//		return 1;
//	case 102: // Red Page Present In Age
//		globals.heldPage and redPagesInBook?
//		return 0; // Page Not Present
//		return 1; // Page Present
//	case 103: // Blue Page Present In Age
//		globals.heldPage and bluePagesInBook?
//		return 0; // Page Not Present
//		return 1; // Page Present
	default:
		return MystScriptParser::getVar(var);
	}
}

void MystScriptParser_Mechanical::toggleVar(uint16 var) {
	switch(var) {
//	case 3: // Sirrus's Secret Room Crate State
//		temp ^= 1;
//	case 4: // Code Lock Puzzle Correct / Myst Book Room Staircase State
//		temp ^= 1;
	case 10: // Fortress Staircase State
		_state.staircaseState ^= 1;
	case 16: // Code Lock Shape #1 - Left
	case 17: // Code Lock Shape #2
	case 18: // Code Lock Shape #3
	case 19: // Code Lock Shape #4 - Right
		_state.codeShape[var - 16] = (_state.codeShape[var - 16] + 1) % 10;
		break;
//	case 102: // Red Page Grab/Release
//		globals.heldPage?
//	case 103: // Blue Page Grab/Release
//		globals.heldPage?
	default:
		MystScriptParser::toggleVar(var);
		break;
	}
}

bool MystScriptParser_Mechanical::setVarValue(uint16 var, uint16 value) {
	bool refresh = false;

	switch (var) {
	default:
		refresh = MystScriptParser::setVarValue(var, value);
		break;
	}

	return refresh;
}

void MystScriptParser_Mechanical::opcode_104(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used on Mechanical Card 6043 (Weapons Rack with Snake Box)
	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Trigger Playing Of Snake Movie", op);

		// TODO: Trigger Type 6 To Play Snake Movie.. Resource #3 on card.
	} else
		unknown(op, var, argc, argv);

}

void MystScriptParser_Mechanical::opcode_105(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Play Stairs Movement Movie", op);

		if (_vm->_varStore->getVar(10)) {
			// TODO: Play Movie from 0 to 1/2 way...
			_vm->_video->playMovie(_vm->wrapMovieFilename("hhstairs", kMechanicalStack), 174, 222);
		} else {
			// TODO: Play Movie from 1/2 way to end...
			_vm->_video->playMovie(_vm->wrapMovieFilename("hhstairs", kMechanicalStack), 174, 222);
		}
	}
}


void MystScriptParser_Mechanical::opcode_121(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 2) {
		uint16 startTime = argv[0];
		uint16 endTime = argv[1];

		warning("TODO: Opcode %d Movie Time Index %d to %d\n", op, startTime, endTime);
		// TODO: Need version of playMovie blocking which allows selection
		//       of start and finish points.
		_vm->_video->playMovie(_vm->wrapMovieFilename("ewindow", kMechanicalStack), 253, 0);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Mechanical::opcode_122(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 0) {
		// Used on Card 6120 (Elevator)
		// Called when Exit Midde Button Pressed

		// TODO: hcelev? Movie of Elevator?
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Mechanical::opcode_123(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 2) {
		// Used on Card 6154
		uint16 start_time = argv[0];
		uint16 end_time = argv[1];

		warning("TODO: Opcode %d Movie Time Index %d to %d\n", op, start_time, end_time);
		// TODO: Need version of playMovie blocking which allows selection
		//       of start and finish points.
		// TODO: Not 100% sure about movie position
		_vm->_video->playMovie(_vm->wrapMovieFilename("hcelev", kMechanicalStack), 205, 40);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Mechanical::opcode_124(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used by Card 6156 (Fortress Rotation Controls)
		// Called when Red Exit Button Pressed to raise Elevator

		// TODO: Fill in Code...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Mechanical::opcode_125(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 0) {
		// Used on Card 6267 (Code Lock)
		// Called by Red Execute Button...
		debugC(kDebugScript, "Opcode %d: Code Lock Execute...", op);

		// TODO: Fill in Logic For Code Lock...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Mechanical::opcode_126(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used by Card 6120 (Fortress Elevator)
		// Called when Red Exit Button Pressed to raise Elevator and
		// exit is clicked...

		// TODO: Fill in Code...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Mechanical::opcode_127(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used for Mech Card 6226 (3 Crystals)
		_vm->_varStore->setVar(20, 1);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Mechanical::opcode_128(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used for Mech Card 6226 (3 Crystals)
		_vm->_varStore->setVar(20, 0);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Mechanical::opcode_129(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used for Mech Card 6226 (3 Crystals)
		_vm->_varStore->setVar(21, 1);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Mechanical::opcode_130(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used for Mech Card 6226 (3 Crystals)
		_vm->_varStore->setVar(21, 0);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Mechanical::opcode_131(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used for Mech Card 6226 (3 Crystals)
		_vm->_varStore->setVar(22, 1);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Mechanical::opcode_132(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used for Mech Card 6226 (3 Crystals)
		_vm->_varStore->setVar(22, 0);
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
	uint16 var;
} g_opcode200Parameters;

void MystScriptParser_Mechanical::opcode_200_run() {
	// Used on Card 6238 (Sirrus' Throne) and Card 6027 (Achenar's Throne)
	// g_opcode200Parameters.var == 0 for Achenar
	// g_opcode200Parameters.var == 1 for Sirrus

	// TODO: Fill in Function...
	// Variable indicates that this is related to Secret Panel State
}

void MystScriptParser_Mechanical::opcode_200_disable() {
	g_opcode200Parameters.enabled = false;
	g_opcode200Parameters.var = 0;
}

void MystScriptParser_Mechanical::opcode_200(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 6238 (Sirrus' Throne) and Card 6027 (Achenar's Throne)
	if (argc == 0) {
		g_opcode200Parameters.var = var;
		g_opcode200Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	uint16 u0;
	uint16 u1;
	uint16 u2;

	bool enabled;
} g_opcode201Parameters;

void MystScriptParser_Mechanical::opcode_201_run() {
	// Used for Card 6159 (Facing Corridor to Fortress Elevator)

	// g_opcode201Parameters.u0
	// g_opcode201Parameters.u1
	// g_opcode201Parameters.u2

	// TODO: Fill in Function...
}

void MystScriptParser_Mechanical::opcode_201_disable() {
	g_opcode201Parameters.enabled = false;
	g_opcode201Parameters.u0 = 0;
	g_opcode201Parameters.u1 = 0;
	g_opcode201Parameters.u2 = 0;
}

void MystScriptParser_Mechanical::opcode_201(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 6159 (Facing Corridor to Fortress Elevator)
	if (argc == 3) {
		g_opcode201Parameters.u0 = argv[0];
		g_opcode201Parameters.u1 = argv[1];
		g_opcode201Parameters.u2 = argv[2];

		g_opcode201Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode202Parameters;

void MystScriptParser_Mechanical::opcode_202_run() {
	// Used for Card 6220 (Sirrus' Mechanical Bird)
	// TODO: Fill in Function
}

void MystScriptParser_Mechanical::opcode_202_disable() {
	g_opcode202Parameters.enabled = false;
}

void MystScriptParser_Mechanical::opcode_202(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Card 6220 (Sirrus' Mechanical Bird)
	if (argc == 0)
		g_opcode202Parameters.enabled = true;
	else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode203Parameters;

void MystScriptParser_Mechanical::opcode_203_run() {
	if (g_opcode203Parameters.enabled) {
		// Used for Card 6043 (Weapons Rack with Snake Box)
		// TODO: Fill in Logic for Snake Box...
	}
}

void MystScriptParser_Mechanical::opcode_203_disable() {
	g_opcode203Parameters.enabled = false;
}

void MystScriptParser_Mechanical::opcode_203(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 6043 (Weapons Rack with Snake Box)
	if (argc == 0)
		g_opcode203Parameters.enabled = true;
	else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
	uint16 soundId;
} g_opcode204Parameters;

void MystScriptParser_Mechanical::opcode_204_run() {
	if (g_opcode204Parameters.enabled) {
		// TODO: Fill in Logic.
		// Var 12 holds Large Cog Position in range 0 to 5
		// - For animation
		// Var 11 holds C position in range 0 to 9
		// - 4 for Correct Answer
		// C Movement Sound
		//_vm->_sound->playSound(g_opcode204Parameters.soundId);
	}
}

void MystScriptParser_Mechanical::opcode_204_disable() {
	g_opcode204Parameters.enabled = false;
}

void MystScriptParser_Mechanical::opcode_204(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 6180 (Lower Elevator Puzzle)
	if (argc == 1) {
		g_opcode204Parameters.soundId = argv[0];
		g_opcode204Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	uint16 soundIdPosition[4];

	bool enabled;
} g_opcode205Parameters;

void MystScriptParser_Mechanical::opcode_205_run() {
	// Used for Card 6156 (Fortress Rotation Controls)
	// TODO: Fill in function...
	// g_opcode205Parameters.soundIdPosition[4]
}

void MystScriptParser_Mechanical::opcode_205_disable() {
	g_opcode205Parameters.enabled = false;
}

void MystScriptParser_Mechanical::opcode_205(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
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

void MystScriptParser_Mechanical::opcode_206_run() {
	if (g_opcode206Parameters.enabled) {
		// Used for Card 6044 (Fortress Rotation Simulator)

		// g_opcode206Parameters.soundIdStart[2]
		// g_opcode206Parameters.soundIdPosition[4]

		// TODO: Fill in function...
	}
}

void MystScriptParser_Mechanical::opcode_206_disable() {
	g_opcode206Parameters.enabled = false;
}

void MystScriptParser_Mechanical::opcode_206(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
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

void MystScriptParser_Mechanical::opcode_209_run() {
	// Used for Card 6044 (Fortress Rotation Simulator)

	// TODO: Implement Function For Secret Panel State as
	//       per Opcode 200 function (Mechanical)
}

void MystScriptParser_Mechanical::opcode_209_disable() {
	g_opcode209Parameters.enabled = false;
}

void MystScriptParser_Mechanical::opcode_209(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 6044 (Fortress Rotation Simulator)
	if (argc == 0)
		g_opcode209Parameters.enabled = true;
	else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Mechanical::opcode_300(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Card 6156 (Fortress Elevator View)
	varUnusedCheck(op, var);
	// TODO: Fill in Logic. Clearing Variable for View?
}

} // End of namespace Mohawk
