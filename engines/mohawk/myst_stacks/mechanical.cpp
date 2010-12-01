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
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/mechanical.h"

#include "gui/message.h"

namespace Mohawk {

#define OPCODE(op, x) { op, &MystScriptParser::x, #x }
#define SPECIFIC_OPCODE(op, x) { op, (OpcodeProcMyst) &MystScriptParser_Mechanical::x, #x }

MystScriptParser_Mechanical::MystScriptParser_Mechanical(MohawkEngine_Myst *vm) : MystScriptParser(vm) {
	setupOpcodes();
}

MystScriptParser_Mechanical::~MystScriptParser_Mechanical() {
}

void MystScriptParser_Mechanical::setupOpcodes() {
	// "invalid" opcodes do not exist or have not been observed
	// "unknown" opcodes exist, but their meaning is unknown

	static const MystOpcode myst_opcodes[] = {
		// "Standard" Opcodes
		OPCODE(0, o_toggleVar),
		OPCODE(1, o_setVar),
		OPCODE(2, o_changeCardSwitch),
		OPCODE(3, o_takePage),
		OPCODE(4, o_redrawCard),
		// TODO: Opcode 5 Not Present
		OPCODE(6, o_goToDest),
		OPCODE(7, o_goToDest),
		OPCODE(8, o_goToDest),
		OPCODE(9, o_triggerMovie),
		OPCODE(10, o_toggleVarNoRedraw),
		// TODO: Opcode 10 to 11 Not Present
		OPCODE(12, o_changeCardSwitch),
		OPCODE(13, o_changeCardSwitch),
		OPCODE(14, o_drawAreaState),
		OPCODE(15, o_redrawAreaForVar),
		OPCODE(16, o_changeCardDirectional),
		OPCODE(17, o_changeCardPush),
		OPCODE(18, o_changeCardPop),
		OPCODE(19, o_enableAreas),
		OPCODE(20, o_disableAreas),
		OPCODE(21, o_directionalUpdate),
		OPCODE(22, o_goToDest),
		OPCODE(23, o_toggleAreasActivation),
		OPCODE(24, o_playSound),
		// TODO: Opcode 25 Not Present
		OPCODE(26, o_stopSoundBackground),
		OPCODE(27, o_playSoundBlocking),
		OPCODE(28, o_restoreDefaultRect),
		OPCODE(29, o_blitRect),
		OPCODE(30, o_changeSound),
		OPCODE(31, o_soundPlaySwitch),
		OPCODE(32, o_soundResumeBackground),
		OPCODE(33, o_blitRect),
		OPCODE(34, o_changeCard),
		OPCODE(35, o_drawImageChangeCard),
		OPCODE(36, o_changeMainCursor),
		OPCODE(37, o_hideCursor),
		OPCODE(38, o_showCursor),
		OPCODE(39, o_delay),
		OPCODE(40, o_changeStack),
		OPCODE(41, o_changeCardPlaySoundDirectional),
		OPCODE(42, o_directionalUpdatePlaySound),
		OPCODE(43, o_saveMainCursor),
		OPCODE(44, o_restoreMainCursor),
		// TODO: Opcode 45 Not Present
		OPCODE(46, o_soundWaitStop),
		// TODO: Opcodes 47 to 99 Not Present

		// "Stack-Specific" Opcodes
		SPECIFIC_OPCODE(104, opcode_104),
		SPECIFIC_OPCODE(105, opcode_105),
		SPECIFIC_OPCODE(121, opcode_121),
		SPECIFIC_OPCODE(122, opcode_122),
		SPECIFIC_OPCODE(123, opcode_123),
		SPECIFIC_OPCODE(124, opcode_124),
		SPECIFIC_OPCODE(125, opcode_125),
		SPECIFIC_OPCODE(126, opcode_126),
		SPECIFIC_OPCODE(127, opcode_127),
		SPECIFIC_OPCODE(128, opcode_128),
		SPECIFIC_OPCODE(129, opcode_129),
		SPECIFIC_OPCODE(130, opcode_130),
		SPECIFIC_OPCODE(131, opcode_131),
		SPECIFIC_OPCODE(132, opcode_132),

		// "Init" Opcodes
		SPECIFIC_OPCODE(200, opcode_200),
		SPECIFIC_OPCODE(201, opcode_201),
		SPECIFIC_OPCODE(202, opcode_202),
		SPECIFIC_OPCODE(203, opcode_203),
		SPECIFIC_OPCODE(204, opcode_204),
		SPECIFIC_OPCODE(205, opcode_205),
		SPECIFIC_OPCODE(206, opcode_206),
		SPECIFIC_OPCODE(209, opcode_209),

		// "Exit" Opcodes
		SPECIFIC_OPCODE(300, opcode_300),

		OPCODE(0xFFFF, NOP)
	};

	_opcodes = myst_opcodes;
	_opcodeCount = ARRAYSIZE(myst_opcodes);
}

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
