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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/mohawk/myst_scripts.cpp $
 * $Id: myst_scripts.cpp 54107 2010-11-07 01:03:29Z fingolfin $
 *
 */

#include "mohawk/myst.h"
#include "mohawk/graphics.h"
#include "mohawk/myst_scripts_selenitic.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "gui/message.h"

namespace Mohawk {

#define OPCODE(op, x) { op, &MystScriptParser::x, #x }
#define SPECIFIC_OPCODE(op, x) { op, (OpcodeProcMyst) &MystScriptParser_Selenitic::x, #x }

MystScriptParser_Selenitic::MystScriptParser_Selenitic(MohawkEngine_Myst *vm) : MystScriptParser(vm) {
	setupOpcodes();
	_invokingResource = NULL;
}

MystScriptParser_Selenitic::~MystScriptParser_Selenitic() {
}

void MystScriptParser_Selenitic::setupOpcodes() {
	// "invalid" opcodes do not exist or have not been observed
	// "unknown" opcodes exist, but their meaning is unknown

	static const MystOpcode myst_opcodes[] = {
		// "Standard" Opcodes
		{ 0, &MystScriptParser::toggleBoolean, "toggleBoolean" },
		//OPCODE(0, toggleBoolean),
		OPCODE(1, setVar),
		OPCODE(2, altDest),
		OPCODE(3, takePage),
		OPCODE(4, opcode_4),
		// TODO: Opcode 5 Not Present
		OPCODE(6, opcode_6),
		OPCODE(7, opcode_7),
		OPCODE(8, opcode_8),
		OPCODE(9, opcode_9),
		// TODO: Opcode 10 to 11 Not Present
		OPCODE(12, altDest),
		OPCODE(13, altDest),
		OPCODE(14, opcode_14),
		OPCODE(15, dropPage),
		OPCODE(16, opcode_16),
		OPCODE(17, opcode_17),
		OPCODE(18, opcode_18),
		OPCODE(19, enableHotspots),
		OPCODE(20, disableHotspots),
		OPCODE(21, opcode_21),
		OPCODE(22, opcode_22),
		OPCODE(23, opcode_23),
		OPCODE(24, playSound),
		// TODO: Opcode 25 Not Present
		OPCODE(26, opcode_26),
		OPCODE(27, playSoundBlocking),
		OPCODE(28, opcode_28),
		OPCODE(29, opcode_29_33),
		OPCODE(30, opcode_30),
		OPCODE(31, opcode_31),
		OPCODE(32, opcode_32),
		OPCODE(33, opcode_29_33),
		OPCODE(34, opcode_34),
		OPCODE(35, opcode_35),
		OPCODE(36, changeCursor),
		OPCODE(37, hideCursor),
		OPCODE(38, showCursor),
		OPCODE(39, opcode_39),
		OPCODE(40, changeStack),
		OPCODE(41, opcode_41),
		OPCODE(42, opcode_42),
		OPCODE(43, opcode_43),
		OPCODE(44, opcode_44),
		// TODO: Opcode 45 Not Present
		OPCODE(46, opcode_46),
		// TODO: Opcodes 47 to 99 Not Present

		// "Stack-Specific" Opcodes
		SPECIFIC_OPCODE(101, opcode_101),
		SPECIFIC_OPCODE(105, opcode_105),
		SPECIFIC_OPCODE(106, opcode_106),
		SPECIFIC_OPCODE(107, opcode_107),
		SPECIFIC_OPCODE(108, opcode_108),
		SPECIFIC_OPCODE(109, opcode_109),
		SPECIFIC_OPCODE(110, opcode_110),
		SPECIFIC_OPCODE(111, opcode_111),
		SPECIFIC_OPCODE(115, opcode_115),

		// "Init" Opcodes
		SPECIFIC_OPCODE(200, opcode_200),
		SPECIFIC_OPCODE(201, opcode_201),
		SPECIFIC_OPCODE(202, opcode_202),
		SPECIFIC_OPCODE(203, opcode_203),
		SPECIFIC_OPCODE(204, opcode_204),
		SPECIFIC_OPCODE(205, opcode_205),
		SPECIFIC_OPCODE(206, opcode_206),

		OPCODE(0xFFFF, NOP)
	};

	_opcodes = myst_opcodes;
	_opcodeCount = ARRAYSIZE(myst_opcodes);
}

void MystScriptParser_Selenitic::disableInitOpcodes() {
	opcode_200_disable();
	opcode_201_disable();
	opcode_202_disable();
	opcode_203_disable();
	opcode_204_disable();
	opcode_205_disable();
	opcode_206_disable();
}

void MystScriptParser_Selenitic::runPersistentOpcodes() {
	opcode_200_run();
	opcode_201_run();
	opcode_202_run();
	opcode_203_run();
	opcode_204_run();
	opcode_205_run();
	opcode_206_run();
}

void MystScriptParser_Selenitic::opcode_101(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used on Card 1191 (Maze Runner)
		// Called when Red Warning Button is pushed

		debugC(kDebugScript, "Opcode %d: Repeat Buzzer Sound?", op);

		// TODO: Fill in logic...
		// Repeat buzzer sound
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Selenitic::opcode_105(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 1) {
		uint16 soundId = argv[0];

		debugC(kDebugScript, "Opcode %d: Sound Receiver Water Button", op);
		debugC(kDebugScript, "\tvar: %d", var);

		// TODO: Complete Function including Var Change?
		_vm->_sound->playSound(soundId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Selenitic::opcode_106(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 1) {
		uint16 soundId = argv[0];

		debugC(kDebugScript, "Opcode %d: Sound Receiver Volcanic Crack Button", op);
		debugC(kDebugScript, "\tvar: %d", var);

		// TODO: Complete Function including Var Change?
		_vm->_sound->playSound(soundId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Selenitic::opcode_107(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 1) {
		uint16 soundId = argv[0];

		debugC(kDebugScript, "Opcode %d: Sound Receiver Clock Button", op);
		debugC(kDebugScript, "\tvar: %d", var);

		// TODO: Complete Function including Var Change?
		_vm->_sound->playSound(soundId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Selenitic::opcode_108(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 1) {
		uint16 soundId = argv[0];

		debugC(kDebugScript, "Opcode %d: Sound Receiver Crystal Rocks Button", op);
		debugC(kDebugScript, "\tvar: %d", var);

		// TODO: Complete Function including Var Change?
		_vm->_sound->playSound(soundId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Selenitic::opcode_109(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 1) {
		uint16 soundId = argv[0];

		debugC(kDebugScript, "Opcode %d: Sound Receiver Wind Button", op);
		debugC(kDebugScript, "\tvar: %d", var);

		// TODO: Complete Function including Var Change?
		_vm->_sound->playSound(soundId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Selenitic::opcode_110(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 15) {
		// Used for Selenitic Maze Runner Exit Logic
		uint16 CardIdEntry = argv[0];
		uint16 CardIdExit = argv[1];
		uint16 u0 = argv[2];
		Common::Rect rect1 = Common::Rect(argv[3], argv[4], argv[5], argv[6]);
		uint16 rect1UpdateDirection = argv[7];
		uint16 u1 = argv[8];
		Common::Rect rect2 = Common::Rect(argv[9], argv[10], argv[11], argv[12]);
		uint16 rect2UpdateDirection = argv[13];
		uint16 u2 = argv[14];

		debugC(kDebugScript, "Opcode %d: Maze Runner Exit Logic and Door Open Animation", op);
		debugC(kDebugScript, "\tExit Card: %d", CardIdEntry);
		debugC(kDebugScript, "\tEntry Card: %d", CardIdExit);
		debugC(kDebugScript, "\tu0 (Exit Var?): %d", u0);

		debugC(kDebugScript, "\trect1.left: %d", rect1.left);
		debugC(kDebugScript, "\trect1.top: %d", rect1.top);
		debugC(kDebugScript, "\trect1.right: %d", rect1.right);
		debugC(kDebugScript, "\trect1.bottom: %d", rect1.bottom);
		debugC(kDebugScript, "\trect1 updateDirection: %d", rect1UpdateDirection);
		debugC(kDebugScript, "\tu1: %d", u1);

		debugC(kDebugScript, "\trect2.left: %d", rect2.left);
		debugC(kDebugScript, "\trect2.top: %d", rect2.top);
		debugC(kDebugScript, "\trect2.right: %d", rect2.right);
		debugC(kDebugScript, "\trect2.bottom: %d", rect2.bottom);
		debugC(kDebugScript, "\trect2 updateDirection: %d", rect2UpdateDirection);
		debugC(kDebugScript, "\tu2: %d", u2);

		// TODO: Finish Implementing Logic...
		// HACK: Bypass Higher Logic for now...
		_vm->changeToCard(argv[1]);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Selenitic::opcode_111(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used on Card 1245 (Sound Receiver)
		// Used by Source Selection Buttons...

		debugC(kDebugScript, "Opcode %d: Unknown", op);
		// TODO: Fill in Function...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Selenitic::opcode_115(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 11) {
		// Used for Selenitic Card 1147 (Musical Door Lock Button)
		uint16 imageIdClose = argv[0]; // TODO: Sound Id?
		uint16 imageIdOpen = argv[1]; // TODO: Sound Id?

		uint16 cardIdOpen = argv[2];

		uint16 u0 = argv[3];
		uint16 u1 = argv[4];

		Common::Rect rect = Common::Rect(argv[5], argv[6], argv[7], argv[8]);

		uint16 updateDirection = argv[9];
		uint16 u2 = argv[10];

		debugC(kDebugScript, "Music Door Lock Logic...");
		debugC(kDebugScript, "\timageId (Close): %d", imageIdClose);
		debugC(kDebugScript, "\timageId (Open): %d", imageIdOpen);
		debugC(kDebugScript, "\tcardId (Open): %d", cardIdOpen);
		debugC(kDebugScript, "\tu0: %d", u0);
		debugC(kDebugScript, "\tu1: %d", u1);

		debugC(kDebugScript, "\trect.left: %d", rect.left);
		debugC(kDebugScript, "\trect.top: %d", rect.top);
		debugC(kDebugScript, "\trect.right: %d", rect.right);
		debugC(kDebugScript, "\trect.bottom: %d", rect.bottom);
		debugC(kDebugScript, "\trect updateDirection: %d", updateDirection);
		debugC(kDebugScript, "\tu2: %d", u2);

		// TODO: Fix Logic...
		// HACK: Bypass Door Lock For Now
		_vm->changeToCard(cardIdOpen);
	} else
		unknown(op, var, argc, argv);
}

// Selenitic Stack Movies For Maze Runner (Card 1191)
static const char* kHCMovPathSelenitic[36] = {
	"backa1",
	"backe1",
	"backf0",
	"backf1",
	"backl0",
	"backl1",
	"backo0",
	"backo1",
	"backp0",
	"backp1",
	"backr0",
	"backr1",
	"backs0",
	"backs1",
	"forwa1",
	"forwe0",
	"forwf0",
	"forwf1",
	"forwl0",
	"forwl1",
	"forwo0",
	"forwo1",
	"forwp0",
	"forwp1",
	"forwr0",
	"forwr1",
	"forws0",
	"forws1",
	"left00",
	"left01",
	"left10",
	"left11",
	"right00",
	"right01",
	"right10",
	"right11"
};

static struct {
	bool enabled;
} g_opcode200Parameters;

void MystScriptParser_Selenitic::opcode_200_run() {
	if (g_opcode200Parameters.enabled) {
		// Used on Card 1191 (Maze Runner)

		// TODO: Implementation Movie Function..
		if (false) {
			_vm->_video->playMovie(_vm->wrapMovieFilename(kHCMovPathSelenitic[0], kSeleniticStack), 201, 26);
		}
	}
}

void MystScriptParser_Selenitic::opcode_200_disable() {
	g_opcode200Parameters.enabled = false;
}

void MystScriptParser_Selenitic::opcode_200(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 1191 (Maze Runner)
	if (argc == 0) {
		g_opcode200Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode201Parameters;

void MystScriptParser_Selenitic::opcode_201_run() {

	if (g_opcode201Parameters.enabled) {
		// Used for Card 1191 (Maze Runner)

		// TODO: Fill in Function...
	}
}

void MystScriptParser_Selenitic::opcode_201_disable() {
	g_opcode201Parameters.enabled = false;
}

void MystScriptParser_Selenitic::opcode_201(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 1191 (Maze Runner)

	if (argc == 0) {
		g_opcode201Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode202Parameters;

void MystScriptParser_Selenitic::opcode_202_run(void) {
	if (g_opcode202Parameters.enabled) {
		// Used for Card 1191 (Maze Runner)

		// TODO: Fill in function...
	}
}

void MystScriptParser_Selenitic::opcode_202_disable(void) {
	g_opcode202Parameters.enabled = false;
}

void MystScriptParser_Selenitic::opcode_202(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 1191 (Maze Runner)
	if (argc == 0) {
		g_opcode202Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode203Parameters;

void MystScriptParser_Selenitic::opcode_203_run(void) {
	if (g_opcode203Parameters.enabled) {
		// Used for Card 1245 (Sound Receiver)
		// TODO: Fill in Logic to Change Viewer Display etc.?
	}
}

void MystScriptParser_Selenitic::opcode_203_disable(void) {
	g_opcode203Parameters.enabled = false;
}

void MystScriptParser_Selenitic::opcode_203(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 1245 (Sound Receiver)
	if (argc == 0) {
		g_opcode203Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode204Parameters;

void MystScriptParser_Selenitic::opcode_204_run(void) {
	if (g_opcode204Parameters.enabled) {
		// Used for Card 1147 (Sound Code Lock)
		// TODO: Fill in code for Sound Lock...
	}
}

void MystScriptParser_Selenitic::opcode_204_disable(void) {
	g_opcode204Parameters.enabled = false;
}

void MystScriptParser_Selenitic::opcode_204(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 1147 (Sound Code Lock)
	if (argc == 0) {
		g_opcode204Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode205Parameters;

void MystScriptParser_Selenitic::opcode_205_run(void) {
	if (g_opcode205Parameters.enabled) {
		// Used for Card 1191 (Maze Runner)
		// TODO: Fill in function...
	}
}

void MystScriptParser_Selenitic::opcode_205_disable(void) {
	g_opcode205Parameters.enabled = false;
}

void MystScriptParser_Selenitic::opcode_205(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 1191 (Maze Runner)
	if (argc == 0) {
		g_opcode205Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode206Parameters;

void MystScriptParser_Selenitic::opcode_206_run(void) {
	if (g_opcode206Parameters.enabled) {
		// Used for Card 1191 (Maze Runner)
		// TODO: Fill in function...
	}
}

void MystScriptParser_Selenitic::opcode_206_disable(void) {
	g_opcode206Parameters.enabled = false;
}

void MystScriptParser_Selenitic::opcode_206(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 1191 (Maze Runner)

	if (argc == 0) {
		g_opcode206Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

} // End of namespace Mohawk
