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
#include "mohawk/myst_scripts.h"

#include "gui/message.h"

namespace Mohawk {

const uint8 stack_map[8] = {
	kSeleniticStack,
	kStoneshipStack,
	kMystStack,
	kMechanicalStack,
	kChannelwoodStack,
	0x0f,
	kDniStack,
	kMystStack
};

const uint16 start_card[8] = {
	1282,
	2029,
	4396,
	6122,
	3137,
	0,
	5038,
	4134
};

// NOTE: Credits Start Card is 10000

#define OPCODE(op, x) { op, &MystScriptParser::x, #x }

MystScriptParser::MystScriptParser(MohawkEngine_Myst *vm) : _vm(vm) {
	setupOpcodes();
	_invokingResource = NULL;
}
	
MystScriptParser::~MystScriptParser() {
}
	
void MystScriptParser::setupOpcodes() {
	// "invalid" opcodes do not exist or have not been observed
	// "unknown" opcodes exist, but their meaning is unknown

	static const MystOpcode myst_opcodes[] = {
		// "Standard" Opcodes
		OPCODE(0, toggleBoolean),
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
		OPCODE(100, opcode_100),
		OPCODE(101, opcode_101),
		OPCODE(102, opcode_102),
		OPCODE(103, opcode_103),
		OPCODE(104, opcode_104),
		OPCODE(105, opcode_105),
		OPCODE(106, opcode_106),
		OPCODE(107, opcode_107),
		OPCODE(108, opcode_108),
		OPCODE(109, opcode_109),
		OPCODE(110, opcode_110),
		OPCODE(111, opcode_111),
		OPCODE(112, opcode_112),
		OPCODE(113, opcode_113),
		OPCODE(114, opcode_114),
		OPCODE(115, opcode_115),
		OPCODE(116, opcode_116),
		OPCODE(117, opcode_117),
		OPCODE(118, opcode_118),
		OPCODE(119, opcode_119),
		OPCODE(120, opcode_120),
		OPCODE(121, opcode_121),
		OPCODE(122, opcode_122),
		OPCODE(123, opcode_123),
		OPCODE(124, opcode_124),
		OPCODE(125, opcode_125),
		OPCODE(126, opcode_126),
		OPCODE(127, opcode_127),
		OPCODE(128, opcode_128),
		OPCODE(129, opcode_129),
		OPCODE(130, opcode_130),
		OPCODE(131, opcode_131),
		OPCODE(132, opcode_132),
		OPCODE(133, opcode_133),
		// TODO: Opcodes 134 to 146 Not Present
		OPCODE(147, opcode_147),
		// TODO: Opcodes 148 to 163 Not Present
		OPCODE(164, opcode_164),
		// TODO: Opcodes 165 to 168 Not Present
		OPCODE(169, opcode_169),
		// TODO: Opcodes 170 to 181 Not Present
		OPCODE(182, opcode_182),
		OPCODE(183, opcode_183),
		OPCODE(184, opcode_184),
		OPCODE(185, opcode_185),
		// TODO: Opcodes 186 to 195 Not Present
		OPCODE(196, opcode_196), // Demo only
		OPCODE(197, opcode_197), // Demo only
		OPCODE(198, opcode_198),
		OPCODE(199, opcode_199),
		
		// "Init" Opcodes
		OPCODE(200, opcode_200),
		OPCODE(201, opcode_201),
		OPCODE(202, opcode_202),
		OPCODE(203, opcode_203),
		OPCODE(204, opcode_204),
		OPCODE(205, opcode_205),
		OPCODE(206, opcode_206),
		OPCODE(207, opcode_207),
		OPCODE(208, opcode_208),
		OPCODE(209, opcode_209),
		OPCODE(210, opcode_210),
		OPCODE(211, opcode_211),
		OPCODE(212, opcode_212),
		OPCODE(213, opcode_213),
		OPCODE(214, opcode_214),
		OPCODE(215, opcode_215),
		OPCODE(216, opcode_216),
		OPCODE(217, opcode_217),
		OPCODE(218, opcode_218),
		OPCODE(219, opcode_219),
		OPCODE(220, opcode_220),
		OPCODE(221, opcode_221),
		OPCODE(222, opcode_222),
		// TODO: Opcodes 223 to 297 Not Present
		OPCODE(298, opcode_298), // Demo only
		OPCODE(299, opcode_299), // Demo only
		
		// "Exit" Opcodes
		OPCODE(300, opcode_300),
		OPCODE(301, opcode_301),
		OPCODE(302, opcode_302),
		OPCODE(303, opcode_303),
		OPCODE(304, opcode_304),
		OPCODE(305, opcode_305),
		OPCODE(306, opcode_306),
		OPCODE(307, opcode_307),
		OPCODE(308, opcode_308),
		OPCODE(309, opcode_309),
		// TODO: Opcodes 310 to 311 Not Present
		OPCODE(312, opcode_312),
		// TODO: Opcodes 313 and greater Not Present

		OPCODE(0xFFFF, NOP)
	};
	
	_opcodes = myst_opcodes;
	_opcodeCount = ARRAYSIZE(myst_opcodes);
}

void MystScriptParser::disableInitOpcodes() {
	opcode_200_disable();
	opcode_201_disable();
	opcode_202_disable();
	opcode_203_disable();
	opcode_204_disable();
	opcode_205_disable();
	opcode_206_disable();
	opcode_209_disable();
	opcode_210_disable();
	opcode_211_disable();
	opcode_212_disable();
}

void MystScriptParser::runPersistentOpcodes() {
	opcode_200_run();
	opcode_201_run();
	opcode_202_run();
	opcode_203_run();
	opcode_204_run();
	opcode_205_run();
	opcode_206_run();
	opcode_209_run();
	opcode_210_run();
	opcode_211_run();
	opcode_212_run();
}

void MystScriptParser::runScript(uint16 scriptCount, MystScriptEntry *scripts, MystResource *invokingResource) {
	_invokingResource = invokingResource;

	debugC(kDebugScript, "Script Count: %d", scriptCount);
	for (uint16 i = 0; i < scriptCount; i++) {
		debugC(kDebugScript, "\tOpcode %d: %d", i, scripts[i].opcode);
		runOpcode(scripts[i].opcode, scripts[i].var, scripts[i].numValues, scripts[i].values);
	}
}

void MystScriptParser::runOpcode(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	bool ranOpcode = false;
		
	for (uint16 i = 0; i < _opcodeCount; i++)
		if (_opcodes[i].op == op) {
			(this->*(_opcodes[i].proc)) (op, var, argc, argv);
			ranOpcode = true;
			break;
		}
		
	if (!ranOpcode)
		error ("Trying to run invalid opcode %d", op);
}

const char *MystScriptParser::getOpcodeDesc(uint16 op) {
	for (uint16 i = 0; i < _opcodeCount; i++)
		if (_opcodes[i].op == op)
			return _opcodes[i].desc;

	error("Unknown opcode %d", op);
	return "";
}

// NOTE: Check to be used on Opcodes where var is thought 
// not to be used. This emits a warning if var is nonzero.
// It is possible that the opcode does use var 0 in this case,
// but this will catch the majority of missed cases.
void MystScriptParser::varUnusedCheck(uint16 op, uint16 var) {
	if (var != 0)
		warning("Opcode %d: Unused Var %d", op, var);
}

void MystScriptParser::unknown(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// NOTE: printf used here instead of debug, so unknown opcodes are always reported...
	printf("Unimplemented opcode 0x%02x (%d)\n", op, op);
	printf("\tUses var %d\n", var);
	printf("\tArg count = %d\n", argc);
	if (argc)
		printf("\tArgs: ");
	for (uint16 i = 0; i < argc; i++) {
		if (i == argc - 1)
			printf("%d\n", argv[i]);
		else
			printf("%d, ", argv[i]);
	}
}

void MystScriptParser::NOP(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// NOTE: Don't check argc/argv here as they vary depending on NOP erased opcode
	debugC(kDebugScript, "NOP");
}

void MystScriptParser::toggleBoolean(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: toggleBoolean() var %d", op, var);
		// HACK: This Mech Card seems to be a special case... Are there others,
		// or a more general definition of this opcode?
		if (_vm->getCurStack() == kMechanicalStack && _vm->getCurCard() == 6267)
			_vm->_varStore->setVar(var, (_vm->_varStore->getVar(var) + 1) % 10);
		else
			_vm->_varStore->setVar(var, !_vm->_varStore->getVar(var));
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::setVar(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 1) {
		debugC(kDebugScript, "Opcode %d: setVar var %d = %d", op, var, argv[0]);

		_vm->_varStore->setVar(var, argv[0]);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::altDest(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 1) {
		// TODO: Work out any differences between opcode 2, 12 and 13..
		debugC(kDebugScript, "Opcode %d: altDest var %d: %d", op, var, _vm->_varStore->getVar(var));

		if (_vm->_varStore->getVar(var))
			_vm->changeToCard(argv[0]);
		else if (_invokingResource != NULL)
			_vm->changeToCard(_invokingResource->getDest());
		else
			warning("Missing invokingResource in altDest call");
	} else 
		unknown(op, var, argc, argv);
}

void MystScriptParser::takePage(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 1) {
		uint16 cursorId = argv[0];

		debugC(kDebugScript, "Opcode %d: takePage Var %d CursorId %d", op, var, cursorId);

		if (_vm->_varStore->getVar(var)) {
			_vm->setMainCursor(cursorId);

			_vm->_varStore->setVar(var, 0);

			// Return pages that are already held
			if (var == 102)
				_vm->_varStore->setVar(103, 1);

			if (var == 103)
				_vm->_varStore->setVar(102, 1);
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_4(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Exit Script of Mechanical Card 6044 (Fortress Rotation Simulator)

	if (argc == 0 && _vm->getCurStack() == kSeleniticStack && _vm->getCurCard() == 1275) {
		// TODO: Fixes Selenitic Card 1275, but not sure if this is correct for general case..
		// as it breaks Selenitic Card 1257, though this may be due to screen update. Also,
		// this may actually be a "Force Card Reload" or "VIEW conditional re-evaluation".. in
		// the general case, rather than this image blit...
		uint16 var_value = _vm->_varStore->getVar(var);
		if (var_value < _vm->_view.scriptResCount) {
			if (_vm->_view.scriptResources[var_value].type == 1) // TODO: Add Symbols for Types
				_vm->_gfx->copyImageToScreen(_vm->_view.scriptResources[var_value].id, Common::Rect(0, 0, 544, 333));
			else
				warning("Opcode %d: Script Resource %d Type Not Image", op, var_value);
		} else
			warning("Opcode %d: var %d value %d outside Script Resource Range %d", op, var, var_value, _vm->_view.scriptResCount);
	} else
		unknown(op, var, argc, argv);
}

// TODO: Work out difference between Opcode 6, 7 & 8...

void MystScriptParser::opcode_6(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used for Selenitic Card 1286 Resource #0
		// Used for Myst Card 4143 Resource #0 & #5
		debugC(kDebugScript, "Opcode %d: Change To Dest of Invoking Resource", op);

		if (_invokingResource != NULL)
			_vm->changeToCard(_invokingResource->getDest());
		else
			warning("Opcode %d: Missing invokingResource", op);
	} else 
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_7(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 0) {
		// Used for Selenitic Card 1244 Resource #3 Var = 5 (Sound Receiver Doors)
		// Used for Myst Card 4143 Resource #1 & #6
		debugC(kDebugScript, "Opcode %d: Change To Dest of Invoking Resource", op);
		debugC(kDebugScript, "\tVar: %d", var);
		// TODO: Var used (if non-zero?) in some way to control function...

		if (_invokingResource != NULL)
			_vm->changeToCard(_invokingResource->getDest());
		else
			warning("Opcode %d: Missing invokingResource", op);
	} else 
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_8(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 0) {
		// Used for Selenitic Card 1244 Resource #2 Var = 5 (Sound Receiver Doors)
		debugC(kDebugScript, "Opcode %d: Change To Dest of Invoking Resource", op);
		debugC(kDebugScript, "\tVar: %d", var);
		// TODO: Var used (if non-zero?) in some way to control function...

		if (_invokingResource != NULL)
			_vm->changeToCard(_invokingResource->getDest());
		else
			warning("Opcode %d: Missing invokingResource", op);
	} else 
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_9(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0 || argc == 1) {
		debugC(kDebugScript, "Opcode %d: Trigger Type 6 Resource Movie..", op);
		// TODO: Add Logic to do this...

		// Used on Stoneship Card 2138 with 1 argument of 66535 as well as with
		// no arguments. Seems logically consistent with play movie with optional
		// start point or time direction control?

		// This understanding of this opcode is based upon Stoneship Card 2197
		// i.e. Sirrus' Desk, but since this is a single case, we should find
		// more...
		if (!((_vm->getCurStack() == kStoneshipStack && _vm->getCurCard() == 2197) ||
		      (_vm->getCurStack() == kStoneshipStack && _vm->getCurCard() == 2138)))
			warning("TODO: Opcode 9 on this card - Check function is consistent");			
	} else 
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_14(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 1) {
		debugC(kDebugScript, "Opcode %d: Unknown, 1 Argument: %d", op, argv[0]);
		debugC(kDebugScript, "\tVar: %d", var);

		// TODO: Function Unknown...
		// Function looks like it changes the Var8 of the invoking resource to argument value..
		// Most calls seem to have var = 0, but used in Myst Card 4500 (Execute Button)
		// with Var 105..
	} else 
		unknown(op, var, argc, argv);
}

void MystScriptParser::dropPage(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: dropPage", op);
		debugC(kDebugScript, "\tvar: %d", var);

		// TODO: Need to check where this is used
		_vm->_varStore->setVar(var, 1);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_16(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used by Channelwood Card 3262 (In Elevator)
	if (argc == 2) {
		debugC(kDebugScript, "Opcode %d: Change Card? Conditional?", op);

		uint16 cardId = argv[0];
		uint16 u0 = argv[1];

		debugC(kDebugScript, "\tcardId: %d", cardId);
		debugC(kDebugScript, "\tu0: %d", u0);

		// TODO: Finish Implementation... 
		_vm->changeToCard(cardId);
	} else
		unknown(op, var, argc, argv);
}

// NOTE: Opcode 17 and 18 form a pair, where Opcode 17 jumps to a card,
// but with the current cardId stored.
// Opcode 18 then "pops" this stored CardId and returns to that card.

// TODO: The purpose of the optional argv[1] on Opcode 17 and argv[0]
// on Opcode 18 which are always 4, 5 or 6 is unknown.

static uint16 opcode_17_18_cardId = 0;

void MystScriptParser::opcode_17(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 2) {
		debugC(kDebugScript, "Opcode %d: Jump to Card Id, Storing Current Card Id", op);

		uint16 cardId = argv[0];
		debugC(kDebugScript, "\tJump to CardId: %d", cardId);

		uint16 u0 = argv[1]; // TODO
		debugC(kDebugScript, "\tu0: %d", u0);

		opcode_17_18_cardId = _vm->getCurCard();

		debugC(kDebugScript, "\tCurrent CardId: %d", opcode_17_18_cardId);

		_vm->changeToCard(cardId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_18(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		debugC(kDebugScript, "Opcode %d: Return To Stored Card Id", op);
		debugC(kDebugScript, "\tCardId: %d", opcode_17_18_cardId);

		uint16 u0 = argv[0];
		debugC(kDebugScript, "\tu0: %d", u0);

		_vm->changeToCard(opcode_17_18_cardId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::enableHotspots(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc > 0) {
		debugC(kDebugScript, "Opcode %d: Enable Hotspots", op);

		uint16 count = argv[0];

		if (argc != count + 1)
			unknown(op, var, argc, argv);
		else {
			for (uint16 i = 0; i < count; i++) {
				debugC(kDebugScript, "Enable hotspot index %d", argv[i + 1]);
				_vm->setResourceEnabled(argv[i + 1], true);
			}
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::disableHotspots(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc > 0) {
		debugC(kDebugScript, "Opcode %d: Disable Hotspots", op);

		uint16 count = argv[0];

		if (argc != count + 1)
			unknown(op, var, argc, argv);
		else {
			for (uint16 i = 0; i < count; i++) {
				debugC(kDebugScript, "Disable hotspot index %d", argv[i + 1]);
				if (argv[i + 1] == 0xFFFF) {
					if (_invokingResource != NULL)
						_invokingResource->setEnabled(false);
					else
						warning("Unknown Resource in disableHotspots script Opcode");
				} else
					_vm->setResourceEnabled(argv[i + 1], false);
			}
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_21(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 6) {
		// Used in Channelwood Card 3318 (Sirrus' Room Drawer)
		debugC(kDebugScript, "Opcode %d: Vertical Slide?", op);

		Common::Rect rect1 = Common::Rect(argv[0], argv[1], argv[2], argv[3]);
		uint16 u0 = argv[4];
		uint16 u1 = argv[5];
		
		debugC(kDebugScript, "\trect1.left: %d", rect1.left);
		debugC(kDebugScript, "\trect1.top: %d", rect1.top);
		debugC(kDebugScript, "\trect1.right: %d", rect1.right);
		debugC(kDebugScript, "\trect1.bottom: %d", rect1.bottom);
		
		debugC(kDebugScript, "\tu0: %d", u0);
		debugC(kDebugScript, "\tu1: %d", u1);

		// TODO: Complete Implementation...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_22(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		if (_invokingResource != NULL)
			_vm->changeToCard(_invokingResource->getDest());
		else
			warning("Missing invokingResource in opcode_22 call");
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_23(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 2 || argc == 4) {
		debugC(kDebugScript, "Opcode %d: Change Resource Enable States", op);

		// Used on Stoneship Card 2080 (Lit Ship Cabin Facing Myst Book Table)
		// Called when Table is clicked to extrude book.

		// Used on Mechanical Card 6159 (In Front of Staircase to Elevator Control)
		// Called when Button Pressed.

		for (byte i = 0; i < argc; i++) {
			debugC(kDebugScript, "\tResource %d Enable set to %d", i, argv[i]);
			switch (argv[i]) {
			case 0:
				_vm->setResourceEnabled(i, false);
				break;
			case 1:
				_vm->setResourceEnabled(i, true);
				break;
			default:
				warning("Opcode %d u%d non-boolean", op, i);
				_vm->setResourceEnabled(i, true);
				break;
			}
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::playSound(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		uint16 soundId = argv[0];

		debugC(kDebugScript, "Opcode %d: playSound", op);
		debugC(kDebugScript, "\tsoundId: %d", soundId);

		_vm->_sound->playSound(soundId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_26(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown...", op);

		// TODO: Work out function...
		if (_vm->getCurStack() == kSeleniticStack && _vm->getCurCard() == 1245) {
			debugC(kDebugScript, "TODO: Function Not Known... Used by Exit Hotspot Resource");
		} else if (_vm->getCurStack() == kStoneshipStack && _vm->getCurCard() == 2226) {
			debugC(kDebugScript, "TODO: Function Not Known... Used by Ship Cabin Door");
		} else if (_vm->getCurStack() == kStoneshipStack && _vm->getCurCard() == 2294) {
			debugC(kDebugScript, "TODO: Function Not Known... Used by Sirrus' Room Door");
		} else if (_vm->getCurStack() == kMechanicalStack && _vm->getCurCard() == 6327) {
			debugC(kDebugScript, "TODO: Function Not Known... Used by Elevator");
		} else if (_vm->getCurStack() == kDniStack && _vm->getCurCard() == 5014) {
			debugC(kDebugScript, "TODO: Function Not Known... Used by Atrus");
		} else
			unknown(op, var, argc, argv);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::playSoundBlocking(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		uint16 soundId = argv[0];

		debugC(kDebugScript, "Opcode %d: playSoundBlocking", op);
		debugC(kDebugScript, "\tsoundId: %d", soundId);

		Audio::SoundHandle *handle = _vm->_sound->playSound(soundId);

		while (_vm->_mixer->isSoundHandleActive(*handle))
			_vm->_system->delayMillis(10);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_28(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	Common::Rect rect;

	if (argc == 1 || argc == 4) {
		debugC(kDebugScript, "Opcode %d: Restore VIEW Default Image in Region", op);

		if (argc == 1) {
			// Used in Stoneship Card 2111 (Compass Rose)
			// Used in Mechanical Card 6267 (Code Lock)
			if (argv[0] == 0xFFFF) {
				rect = _invokingResource->getRect();
			} else
				unknown(op, var, argc, argv);
		} else if (argc == 4) {
			// Used in ... TODO: Fill in.
			rect = Common::Rect(argv[0], argv[1], argv[2], argv[3]);
		} else
			warning("Opcode %d: argc Error", op);

		debugC(kDebugScript, "\trect.left: %d", rect.left);
		debugC(kDebugScript, "\trect.top: %d", rect.top);
		debugC(kDebugScript, "\trect.right: %d", rect.right);
		debugC(kDebugScript, "\trect.bottom: %d", rect.bottom);

		// TODO: Need to fix VIEW logic so this doesn't need
		//       calculation at this level.
		uint16 imageToDraw = 0;
		if (_vm->_view.conditionalImageCount == 0)
			imageToDraw = _vm->_view.mainImage;
		else {
			for (uint16 i = 0; i < _vm->_view.conditionalImageCount; i++)
				if (_vm->_varStore->getVar(_vm->_view.conditionalImages[i].var) < _vm->_view.conditionalImages[i].numStates)
					imageToDraw = _vm->_view.conditionalImages[i].values[_vm->_varStore->getVar(_vm->_view.conditionalImages[i].var)];
		}
		_vm->_gfx->copyImageSectionToScreen(imageToDraw, rect, rect);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_29_33(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// TODO: Opcode 29 called on Mechanical Card 6178 causes a engine
	//       abort this is because imageId is 7158 (not valid), but the
	//       script resource gives this as 7178 (valid)...

	if (argc == 7) {
		uint16 imageId = argv[0];

		Common::Rect srcRect = Common::Rect(argv[1], argv[2], argv[3], argv[4]);

		Common::Rect dstRect = Common::Rect(argv[5], argv[6], 544, 333);

		if (dstRect.left == -1 || dstRect.top == -1) {
			// Interpreted as full screen
			dstRect.left = 0;
			dstRect.top = 0;
		}

		dstRect.right = dstRect.left + srcRect.width();
		dstRect.bottom = dstRect.top + srcRect.height();

		debugC(kDebugScript, "Opcode %d: Blit Image", op);
		debugC(kDebugScript, "\timageId: %d", imageId);
		debugC(kDebugScript, "\tsrcRect.left: %d", srcRect.left);
		debugC(kDebugScript, "\tsrcRect.top: %d", srcRect.top);
		debugC(kDebugScript, "\tsrcRect.right: %d", srcRect.right);
		debugC(kDebugScript, "\tsrcRect.bottom: %d", srcRect.bottom);
		debugC(kDebugScript, "\tdstRect.left: %d", dstRect.left);
		debugC(kDebugScript, "\tdstRect.top: %d", dstRect.top);
		debugC(kDebugScript, "\tdstRect.right: %d", dstRect.right);
		debugC(kDebugScript, "\tdstRect.bottom: %d", dstRect.bottom);

		_vm->_gfx->copyImageSectionToScreen(imageId, srcRect, dstRect);
	} else
		unknown(op, var, argc, argv);
}

// TODO: Implement common engine function for read and processing of sound blocks
//       for use by this opcode and VIEW sound block.
// TODO: Though the playSound and PlaySoundBlocking opcodes play sounds immediately,
//       this opcode changes the main background sound playing..
//       Current behaviour here and with VIEW sound block is not right as demonstrated
//       by Channelwood Card 3280 (Tank Valve) and water flow sound behaviour in pipe
//       on cards leading from shed...
void MystScriptParser::opcode_30(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	int16 *soundList = NULL;
	uint16 *soundListVolume = NULL;

	// Used on Stoneship Card 2080
	// Used on Channelwood Card 3225 with argc = 8 i.e. Conditional Sound List
	if (argc == 1 || argc == 2 || argc == 8) {
		debugC(kDebugScript, "Opcode %d: Process Sound Block", op);
		uint16 decodeIdx = 0;

		int16 soundAction = argv[decodeIdx++];
		uint16 soundVolume = 65535;
		if (soundAction == kMystSoundActionChangeVolume || soundAction > 0) {
			soundVolume = argv[decodeIdx++];
		} else if (soundAction == kMystSoundActionConditional) {
			debugC(kDebugScript, "Conditional sound list");
			uint16 condVar = argv[decodeIdx++];
			uint16 condVarValue = _vm->_varStore->getVar(condVar);
			uint16 condCount = argv[decodeIdx++];

			debugC(kDebugScript, "\tcondVar: %d = %d", condVar, condVarValue);
			debugC(kDebugScript, "\tcondCount: %d", condCount);
			
			soundList = new int16[condCount];
			soundListVolume = new uint16[condCount];

			if (condVarValue >= condCount)
				warning("Opcode %d: Conditional sound variable outside range",  op);
			else {
				for (uint16 i = 0; i < condCount; i++) {
					soundList[i] = argv[decodeIdx++];
					debugC(kDebugScript, "\t\tCondition %d: Action %d", i, soundList[i]);
					if (soundAction == kMystSoundActionChangeVolume || soundAction > 0) {
						soundListVolume[i] = argv[decodeIdx++];
					} else
						soundListVolume[i] = 65535;
					debugC(kDebugScript, "\t\tCondition %d: Volume %d", i, soundListVolume[i]);
				}

				soundAction = soundList[condVarValue];
				soundVolume = soundListVolume[condVarValue];
			}
		}

		// NOTE: Mixer only has 8-bit channel volume granularity, 
		// Myst uses 16-bit? Or is part of this balance?
		soundVolume = (byte)(soundVolume / 255);

		if (soundAction == kMystSoundActionContinue)
			debugC(kDebugScript, "Continue current sound");
		else if (soundAction == kMystSoundActionChangeVolume) {
			debugC(kDebugScript, "Continue current sound, change volume");
			debugC(kDebugScript, "\tVolume: %d", soundVolume);
			// TODO: Implement Volume Control..
		} else if (soundAction == kMystSoundActionStop) {
			debugC(kDebugScript, "Stop sound");
			_vm->_sound->stopSound();
		} else if (soundAction > 0) {
			debugC(kDebugScript, "Play new Sound, change volume");
			debugC(kDebugScript, "\tSound: %d", soundAction);
			debugC(kDebugScript, "\tVolume: %d", soundVolume);
			_vm->_sound->stopSound();
			// TODO: Need to keep sound handle and add function to change volume of
			// looped running sound for kMystSoundActionChangeVolume type
			_vm->_sound->playSound(soundAction, true, soundVolume);
		} else {
			debugC(kDebugScript, "Unknown");
			warning("Unknown sound control value in opcode %d", op);
		}
	} else
		unknown(op, var, argc, argv);

	delete[] soundList;
	soundList = NULL;
	delete[] soundListVolume;
	soundListVolume = NULL;
}

void MystScriptParser::opcode_31(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Channelwood Card 3505 (Walkway from Sirrus' Room)
	if (argc == 2) {
		debugC(kDebugScript, "Opcode %d: Boolean Choice of Play Sound", op);

		uint16 soundId0 = argv[0];
		uint16 soundId1 = argv[1];

		debugC(kDebugScript, "\tvar: %d", var);
		debugC(kDebugScript, "\tsoundId0: %d", soundId0);
		debugC(kDebugScript, "\tsoundId1: %d", soundId1);

		if (_vm->_varStore->getVar(var)) {
			if (soundId1)
				_vm->_sound->playSound(soundId1);
		} else {
			if (soundId0)
				_vm->_sound->playSound(soundId0);
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_32(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used on Channelwood Card 3503 (Door to Sirrus' Room)
	// Used on Myst Card 4188 (Door to Cabin)
	// Used on Myst Card 4363 (Red Book Open)
	// Used on Myst Card 4371 (Blue Book Open)
	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown...", op);
		// TODO: Implement function...
		// Set Resource 0 Enabled?
		// or Trigger Movie?
		// Set resource flag to Enabled?
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_34(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 2) {
		debugC(kDebugScript, "Opcode %d: Change Card (with Delay?)", op);

		uint16 cardId = argv[0];
		uint16 u0 = argv[1];

		debugC(kDebugScript, "\tTarget Card: %d", cardId);
		debugC(kDebugScript, "\tu0: %d", u0); // TODO: Delay?

		_vm->changeToCard(cardId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_35(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 3) {
		debugC(kDebugScript, "Opcode %d: Draw Full Screen Image, Delay then Change Card", op);

		uint16 imageId = argv[0];
		uint16 cardId = argv[1];
		uint16 delay = argv[2]; // TODO: Not sure about argv[2] being delay..

		debugC(kDebugScript, "\timageId: %d", imageId);
		debugC(kDebugScript, "\tcardId: %d", cardId);
		debugC(kDebugScript, "\tdelay: %d", delay);

		_vm->_gfx->copyImageToScreen(imageId, Common::Rect(0, 0, 544, 333));
		_vm->_system->updateScreen();
		_vm->_system->delayMillis(delay * 100);
		_vm->changeToCard(cardId);	
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::changeCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		debugC(kDebugScript, "Opcode %d: Change Cursor", op);
		debugC(kDebugScript, "Cursor: %d", argv[0]);
		
		// TODO: Not sure if this needs to change mainCursor or similar...
		_vm->_gfx->changeCursor(argv[0]);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::hideCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Hide Cursor", op);
		_vm->_gfx->hideCursor();
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::showCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Show Cursor", op);
		_vm->_gfx->showCursor();
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_39(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		// Used on Mechanical Card 6327 (Elevator)
		debugC(kDebugScript, "Opcode %d: Delay?", op);

		uint16 time = argv[0];

		debugC(kDebugScript, "\tTime: %d", time);

		// TODO: Fill in Function...
		// May actually be related to movie control.. not sure.
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::changeStack(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	Audio::SoundHandle *handle;
	varUnusedCheck(op, var);

	if (argc == 3) {
		debugC(kDebugScript, "Opcode %d: changeStack", op);

		uint16 targetStack = argv[0];
		uint16 soundIdLinkSrc = argv[1];
		uint16 soundIdLinkDst = argv[2];

		debugC(kDebugScript, "\tTarget Stack: %d", targetStack);
		debugC(kDebugScript, "\tSource Stack Link Sound: %d", soundIdLinkSrc);
		debugC(kDebugScript, "\tDestination Stack Link Sound: %d", soundIdLinkDst);

		_vm->_sound->stopSound();

		if (_vm->getFeatures() & GF_DEMO) {

			// The demo has linking sounds too for this, but it just sounds completely
			// wrong as you're not actually linking when using this opcode. The sounds are only
			// played for the full game linking.
			if (!_vm->_tweaksEnabled) {
				handle= _vm->_sound->playSound(soundIdLinkSrc);
				while (_vm->_mixer->isSoundHandleActive(*handle))
					_vm->_system->delayMillis(10);
			}

			// No need to have a table for just this data...
			if (targetStack == 1) {
				_vm->changeToStack(kDemoSlidesStack);
				_vm->changeToCard(1000);
			} else if (targetStack == 2) {
				_vm->changeToStack(kDemoPreviewStack);
				_vm->changeToCard(3000);
			}

			if (!_vm->_tweaksEnabled) {
				handle = _vm->_sound->playSound(soundIdLinkDst);
				while (_vm->_mixer->isSoundHandleActive(*handle))
					_vm->_system->delayMillis(10);
			}
		} else {
			handle = _vm->_sound->playSound(soundIdLinkSrc);
			while (_vm->_mixer->isSoundHandleActive(*handle))
				_vm->_system->delayMillis(10);

			// TODO: Play Flyby Entry Movie on Masterpiece Edition..? Only on Myst to Age Link?
			_vm->changeToStack(stack_map[targetStack]);
			_vm->changeToCard(start_card[targetStack]);

			handle = _vm->_sound->playSound(soundIdLinkDst);
			while (_vm->_mixer->isSoundHandleActive(*handle))
				_vm->_system->delayMillis(10);
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_41(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// TODO: Will need to stop immediate screen update on
	//       Opcode 29 etc. for this to work correctly..
	//       Also, script processing will have to block U/I
	//       events etc. for correct sequencing.

	if (argc == 10 || argc == 16) {
		uint16 cardId = argv[0];
		uint16 soundId = argv[1];
		uint16 u0 = argv[2];
		uint16 u1 = argv[3];
		Common::Rect region = Common::Rect(argv[4], argv[5], argv[6], argv[7]);
		uint16 updateDirection = argv[8];
		uint16 u2 = argv[9];

		Common::Rect region2;
		uint16 updateDirection2 = 0;
		uint16 u3 = 0;
		if (argc == 16) {
			region2 = Common::Rect(argv[10], argv[11], argv[12], argv[13]);
			updateDirection2 = argv[14];
			u3 = argv[15];
		}

		debugC(kDebugScript, "Opcode %d: Change Card, Play Sound and Directional Update Screen Region", op);
		debugC(kDebugScript, "\tCard Id: %d", cardId);
		debugC(kDebugScript, "\tSound Id: %d", soundId);
		debugC(kDebugScript, "\tu0: %d", u0);
		debugC(kDebugScript, "\tu1: %d", u1);
		debugC(kDebugScript, "\tregion.left: %d", region.left);
		debugC(kDebugScript, "\tregion.top: %d", region.top);
		debugC(kDebugScript, "\tregion.right: %d", region.right);
		debugC(kDebugScript, "\tregion.bottom: %d", region.bottom);
		debugCN(kDebugScript, "\tupdateDirection: %d = ", updateDirection);
		
		switch (updateDirection) {
		case 0:
			debugC(kDebugScript, "Left to Right");
			break;
		case 1:
			debugC(kDebugScript, "Right to Left");
			break;
		case 5:
			debugC(kDebugScript, "Top to Bottom");
			break;
		case 6:
			debugC(kDebugScript, "Bottom to Top");
			break;
		default:
			warning("Unknown Update Direction");
			break;
		}
		
		debugC(kDebugScript, "\tu2: %d", u2); // TODO: Speed / Delay of Update?

		// 10 Argument version Used in:
		// Selenitic Card 1243 (Sound Receiver Door)
		// Myst Card 4317 (Generator Room Door)

		if (argc == 16) {
			// 16 Argument version Used in:
			// Selenitic Card 1008 and 1010 (Mazerunner Door)

			debugC(kDebugScript, "\tregion2.left: %d", region2.left);
			debugC(kDebugScript, "\tregion2.top: %d", region2.top);
			debugC(kDebugScript, "\tregion2.right: %d", region2.right);
			debugC(kDebugScript, "\tregion2.bottom: %d", region2.bottom);
			debugCN(kDebugScript, "\tupdateDirection2: %d = ", updateDirection2);
			 
			switch (updateDirection2) {
			case 0:
				debugC(kDebugScript, "Left to Right");
				break;
			case 1:
				debugC(kDebugScript, "Right to Left");
				break;
			case 5:
				debugC(kDebugScript, "Top to Bottom");
				break;
			case 6:
				debugC(kDebugScript, "Bottom to Top");
				break;
			default:
				warning("Unknown Update Direction");
				break;
			}
			
			debugC(kDebugScript, "\tu3: %d", u3); // TODO: Speed / Delay of Update?
		}

		_vm->changeToCard(cardId);
		_vm->_sound->playSound(soundId);
		// TODO: Complete Implementation
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_42(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// TODO: Will need to stop immediate screen update on
	//       Opcode 29 etc. for this to work correctly..
	//       Also, script processing will have to block U/I
	//       events etc. for correct sequencing.

	if (argc == 9 || argc == 15) {
		uint16 soundId = argv[0];
		uint16 u0 = argv[1];
		uint16 u1 = argv[2];
		Common::Rect region = Common::Rect(argv[3], argv[4], argv[5], argv[6]);
		uint16 updateDirection = argv[7];
		uint16 u2 = argv[8];

		Common::Rect region2;
		uint16 updateDirection2 = 0;
		uint16 u3 = 0;
		if (argc == 15) {
			region2 = Common::Rect(argv[9], argv[10], argv[11], argv[12]);
			updateDirection2 = argv[13];
			u3 = argv[14];
		}

		debugC(kDebugScript, "Opcode %d: Play Sound and Directional Update Screen Region", op);
		debugC(kDebugScript, "\tsound: %d", soundId);
		debugC(kDebugScript, "\tu0: %d", u0);
		debugC(kDebugScript, "\tu1: %d", u1);
		debugC(kDebugScript, "\tregion.left: %d", region.left);
		debugC(kDebugScript, "\tregion.top: %d", region.top);
		debugC(kDebugScript, "\tregion.right: %d", region.right);
		debugC(kDebugScript, "\tregion.bottom: %d", region.bottom);
		debugCN(kDebugScript, "\tupdateDirection: %d = ", updateDirection);
		
		switch (updateDirection) {
		case 0:
			debugC(kDebugScript, "Left to Right");
			break;
		case 1:
			debugC(kDebugScript, "Right to Left");
			break;
		case 5:
			debugC(kDebugScript, "Top to Bottom");
			break;
		case 6:
			debugC(kDebugScript, "Bottom to Top");
			break;
		default:
			warning("Unknown Update Direction");
			break;
		}
		
		debugC(kDebugScript, "\tu2: %d", u2); // TODO: Speed / Delay of Update?

		// 9 Argument version Used in:
		// Myst Card 4730 (Stellar Observatory Door)
		// Myst Card 4184 (Cabin Door)

		if (argc == 15) {
			// 15 Argument version Used in:
			// Channelwood Card 3492 (Achenar's Room Door)

			debugC(kDebugScript, "\tregion2.left: %d", region2.left);
			debugC(kDebugScript, "\tregion2.top: %d", region2.top);
			debugC(kDebugScript, "\tregion2.right: %d", region2.right);
			debugC(kDebugScript, "\tregion2.bottom: %d", region2.bottom);
			debugCN(kDebugScript, "\tupdateDirection2: %d = ", updateDirection2);
			
			switch (updateDirection2) {
			case 0:
				debugC(kDebugScript, "Left to Right");
				break;
			case 1:
				debugC(kDebugScript, "Right to Left");
				break;
			case 5:
				debugC(kDebugScript, "Top to Bottom");
				break;
			case 6:
				debugC(kDebugScript, "Bottom to Top");
				break;
			default:
				warning("Unknown Update Direction");
				break;
			}
			
			debugC(kDebugScript, "\tu3: %d", u3); // TODO: Speed / Delay of Update?
		}

		_vm->_sound->playSound(soundId);
		// TODO: Complete Implementation
	} else
		unknown(op, var, argc, argv);
}

// TODO: Are Opcode 43 and 44 enable / disable paired commands?

void MystScriptParser::opcode_43(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown Function", op);

		// TODO: Function Unknown
		// Used on Stoneship Card 2154 (Bottom of Lighthouse)
		// Used on Stoneship Card 2138 (Lighthouse Floating Chest Closeup)
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_44(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown Function", op);

		// TODO: Function Unknown
		// Used on Stoneship Card 2154 (Bottom of Lighthouse)
		// Used on Stoneship Card 2138 (Lighthouse Floating Chest Closeup)
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_46(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used on Selenitic Card 1191 (Maze Runner)
		// Used on Mechanical Card 6267 (Code Lock)
		// Used when Button is pushed...
		debugC(kDebugScript, "Opcode %d: Conditional Code Jump?", op);
		// TODO: Function Unknown - Fill in...
		// Logic looks like this is some kind of Conditional Code
		// Jump Point.
		// The Logic for the Mechanical Code Lock Seems to be in this
		// opcode with it being present twice delimiting the start
		// of the incorrect code and correct code action blocks...
		// Not sure how a general case can be made for this..
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_100(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Hard coded SoundId valid only for Intro Stack.
	// Other stacks use Opcode 40, which takes SoundId values as arguments.
	const uint16 soundIdLinkSrc = 5;

	switch (_vm->getCurStack()) {
	case kIntroStack:
		debugC(kDebugScript, "Opcode %d: ChangeStack", op);
		debugC(kDebugScript, "\tvar: %d", var);

		// TODO: Merge with changeStack (Opcode 40) Implementation? 
		if (_vm->_varStore->getVar(var) == 5 || _vm->_varStore->getVar(var) > 7) {
			// TODO: Dead Book i.e. Released Sirrus/Achenar
		} else {
			// Play Linking Sound, blocking...
			_vm->_sound->stopSound();
			Audio::SoundHandle *handle = _vm->_sound->playSound(soundIdLinkSrc);
			while (_vm->_mixer->isSoundHandleActive(*handle))
				_vm->_system->delayMillis(10);

			// Play Flyby Entry Movie on Masterpiece Edition..
			if ((_vm->getFeatures() & GF_ME)) {
				switch (stack_map[_vm->_varStore->getVar(var)]) {
				case kSeleniticStack:
					_vm->_video->playMovieCentered(_vm->wrapMovieFilename("selenitic flyby", kMasterpieceOnly));
					break;
				case kStoneshipStack:
					_vm->_video->playMovieCentered(_vm->wrapMovieFilename("stoneship flyby", kMasterpieceOnly));
					break;
				// Myst Flyby Movie not used in Original Masterpiece Edition Engine
				case kMystStack:
					if (_vm->_tweaksEnabled)
						_vm->_video->playMovieCentered(_vm->wrapMovieFilename("myst flyby", kMasterpieceOnly));
					break;
				case kMechanicalStack:
					_vm->_video->playMovieCentered(_vm->wrapMovieFilename("mech age flyby", kMasterpieceOnly));
					break;
				case kChannelwoodStack:
					_vm->_video->playMovieCentered(_vm->wrapMovieFilename("channelwood flyby", kMasterpieceOnly));
					break;
				default:
					break;
				}
			}

			uint16 varValue = _vm->_varStore->getVar(var);
			_vm->changeToStack(stack_map[varValue]);
			_vm->changeToCard(start_card[varValue]);

			// TODO: No soundIdLinkDst for Opcode 100 link? Check Original.
		}
		break;
	case kStoneshipStack:
		// Used for Cards 2185 (Water Pump)
		varUnusedCheck(op, var);

		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Unknown Function", op);

			// TODO: Called when Water Pump Button is pressed? Animation?
		} else
			unknown(op, var, argc, argv);
		break;
	case kDniStack:
		// Used in Card 5022 (Rocks)
		varUnusedCheck(op, var);

		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Unknown Function", op);

			// TODO: Fill in Logic.
		} else
			unknown(op, var, argc, argv);
		break;
	case kCreditsStack:
	case kMakingOfStack:
		_vm->_system->quit();
		break;
	case kDemoSlidesStack:
		// TODO: Change to changeStack call?
		_vm->changeToStack(kDemoStack);
		_vm->changeToCard(2001);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_101(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kSeleniticStack:
		varUnusedCheck(op, var);

		if (argc == 0) {
			// Used on Card 1191 (Maze Runner)
			// Called when Red Warning Button is pushed

			debugC(kDebugScript, "Opcode %d: Repeat Buzzer Sound?", op);

			// TODO: Fill in logic...
			// Repeat buzzer sound
		} else
			unknown(op, var, argc, argv);
		break;
	case kStoneshipStack:
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
		break;
	case kDemoPreviewStack:
	case kMystStack:
		debugC(kDebugScript, "Opcode %d: Decrement Variable", op);
		if (argc == 0) {
			debugC(kDebugScript, "\tvar: %d", var);
			uint16 varValue = _vm->_varStore->getVar(var);
			// Logic to prevent decrement to negative
			if (varValue != 0)
				_vm->_varStore->setVar(var, varValue - 1);
		} else
			unknown(op, var, argc, argv);
		break;
	case kChannelwoodStack:
		varUnusedCheck(op, var);

		if (argc == 1) {
			debugC(kDebugScript, "Opcode %d: Play Pipe Movie and Sound", op);

			uint16 soundId = argv[0];
			debugC(kDebugScript, "\tsoundId: %d", soundId);

			_vm->_sound->playSound(soundId);

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
		break;
	case kDniStack:
		// Used in Card 5014 (Atrus)
		// Hotspot Resource Used to hand Page to Atrus...
		varUnusedCheck(op, var);
		// TODO: Fill in Logic.
		break;
	case kDemoStack:
		varUnusedCheck(op, var);

		// Used on Card 2000, 2002 and 2003
		// Triggered by Click
		if (argc == 0) {
			// TODO: Fill in Logic.. Fade in?
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_102(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kStoneshipStack:
		varUnusedCheck(op, var);

		if (argc == 2) {
			debugC(kDebugScript, "Opcode %d: Play Book Room Movie", op);

			uint16 startTime = argv[0];
			uint16 endTime = argv[1];

			debugC(kDebugScript, "\tstartTime: %d", startTime);
			debugC(kDebugScript, "\tendTime: %d", endTime);

			printf("TODO: Opcode %d Movie Time Index %d to %d\n", op, startTime, endTime);
			// TODO: Need version of playMovie blocking which allows selection
			//       of start and finish points.
			_vm->_video->playMovie(_vm->wrapMovieFilename("bkroom", kStoneshipStack), 159, 99);
		} else
			unknown(op, var, argc, argv);
		break;
	case kDemoPreviewStack:
	case kMystStack:
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Increment Variable", op);
			debugC(kDebugScript, "\tvar: %d", var);

			// AFAIK no logic to put ceiling on increment at least in this opcode
			_vm->_varStore->setVar(var, _vm->_varStore->getVar(var) + 1);
		} else
			unknown(op, var, argc, argv);
		break;
	case kChannelwoodStack:
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
			_vm->changeToCard(cardId);
		} else
			unknown(op, var, argc, argv);
		break;
	case kDemoStack:
		varUnusedCheck(op, var);

		// Used on Card 2002 and 2003
		// Triggered by Click
		if (argc == 0) {
			// TODO: Fill in Logic.. Fade out?
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_103(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kStoneshipStack:
		varUnusedCheck(op, var);

		if (argc == 1) {
			// Used on Card 2197 (Sirrus' Room Drawers)
			debugC(kDebugScript, "Opcode %d: Unknown", op);

			uint16 u0 = argv[0];

			debugC(kDebugScript, "\tu0: %d", u0);
			// TODO: Fill in Logic...
		} else
			unknown(op, var, argc, argv);
		break;
	case kDemoPreviewStack:
	case kMystStack:
		// Used on Myst Card 4162 (Fireplace Grid)
		if (argc == 1) {
			debugC(kDebugScript, "Opcode %d: Toggle Variable with Bitmask", op);

			uint16 bitmask = argv[0];
			uint16 varValue = _vm->_varStore->getVar(var);

			debugC(kDebugScript, "\tvar: %d", var);
			debugC(kDebugScript, "\tbitmask: 0x%02X", bitmask);

			if (varValue & bitmask)
				_vm->_varStore->setVar(var, varValue & ~bitmask);
			else
				_vm->_varStore->setVar(var, varValue | bitmask);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_104(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kStoneshipStack:
		varUnusedCheck(op, var);

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
		break;
	case kMystStack:
		varUnusedCheck(op, var);

		// Used on Myst Card 4162 and 4166 (Fireplace Puzzle Rotation Movies)
		if (argc == 1) {
			debugC(kDebugScript, "Opcode %d: Play Fireplace Puzzle Rotation Movies", op);

			uint16 movieNum = argv[0];
			debugC(kDebugScript, "\tmovieNum: %d", movieNum);

			if (movieNum == 0)
				_vm->_video->playMovie(_vm->wrapMovieFilename("fpin", kMystStack), 167, 5);
			if (movieNum == 1)
				_vm->_video->playMovie(_vm->wrapMovieFilename("fpout", kMystStack), 167, 5);
		} else
			unknown(op, var, argc, argv);
		break;
	case kMechanicalStack:
		varUnusedCheck(op, var);

		// Used on Mechanical Card 6043 (Weapons Rack with Snake Box)
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Trigger Playing Of Snake Movie", op);

			// TODO: Trigger Type 6 To Play Snake Movie.. Resource #3 on card.
		} else
			unknown(op, var, argc, argv);
		break;
	case kChannelwoodStack:
		varUnusedCheck(op, var);

		// Used on Channelwood Card 3280
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Do Water Tank Valve Open Animation", op);
			Common::Rect rect = _invokingResource->getRect();

			// TODO: Need to load the image ids from Script Resources structure of VIEW
			for (uint16 imageId = 3595; imageId <= 3601; imageId++) { 
				_vm->_gfx->copyImageToScreen(imageId, rect);
				_vm->_system->delayMillis(50);
			}

			// TODO: Is 8 gotten from var7 of calling hotspot, rather than hardcoded?
			_vm->_varStore->setVar(8, 1);
			_vm->_varStore->setVar(19, 1);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_105(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kSeleniticStack:
		if (argc == 1) {
			uint16 soundId = argv[0];

			debugC(kDebugScript, "Opcode %d: Sound Receiver Water Button", op);
			debugC(kDebugScript, "\tvar: %d", var);

			// TODO: Complete Function including Var Change?
			_vm->_sound->playSound(soundId);
		} else
			unknown(op, var, argc, argv);
		break;
	case kMystStack:
		if (argc == 1) {
			varUnusedCheck(op, var);

			uint16 soundId = argv[0];
			uint16 boxValue = 0;
			Audio::SoundHandle *handle;

			debugC(kDebugScript, "Opcode %d: Ship Puzzle Logic", op);
			debugC(kDebugScript, "\tsoundId: %d", soundId);

			// Logic for Myst Ship Box Puzzle Solution
			for (byte i = 0; i < 8; i++)
				boxValue |= _vm->_varStore->getVar(i + 26) ? (1 << i) : 0;

			uint16 var10 = _vm->_varStore->getVar(10);

			if (boxValue == 0x32 && var10 == 0) {
				handle = _vm->_sound->playSound(soundId);

				while (_vm->_mixer->isSoundHandleActive(*handle))
					_vm->_system->delayMillis(10);

				_vm->_varStore->setVar(10, 1);
			} else if (boxValue != 0x32 && var10 == 1) {
				handle = _vm->_sound->playSound(soundId);

				while (_vm->_mixer->isSoundHandleActive(*handle))
					_vm->_system->delayMillis(10);

				_vm->_varStore->setVar(10, 0);
			}
		} else
			unknown(op, var, argc, argv);
		break;
	case kMechanicalStack:
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
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_106(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kSeleniticStack:
		if (argc == 1) {
			uint16 soundId = argv[0];

			debugC(kDebugScript, "Opcode %d: Sound Receiver Volcanic Crack Button", op);
			debugC(kDebugScript, "\tvar: %d", var);

			// TODO: Complete Function including Var Change?
			_vm->_sound->playSound(soundId);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_107(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kSeleniticStack:
		if (argc == 1) {
			uint16 soundId = argv[0];

			debugC(kDebugScript, "Opcode %d: Sound Receiver Clock Button", op);
			debugC(kDebugScript, "\tvar: %d", var);

			// TODO: Complete Function including Var Change?
			_vm->_sound->playSound(soundId);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_108(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kSeleniticStack:
		if (argc == 1) {
			uint16 soundId = argv[0];

			debugC(kDebugScript, "Opcode %d: Sound Receiver Crystal Rocks Button", op);
			debugC(kDebugScript, "\tvar: %d", var);

			// TODO: Complete Function including Var Change?
			_vm->_sound->playSound(soundId);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_109(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kSeleniticStack:
		if (argc == 1) {
			uint16 soundId = argv[0];

			debugC(kDebugScript, "Opcode %d: Sound Receiver Wind Button", op);
			debugC(kDebugScript, "\tvar: %d", var);

			// TODO: Complete Function including Var Change?
			_vm->_sound->playSound(soundId);
		} else
			unknown(op, var, argc, argv);
		break;
	case kMystStack:
		if (argc == 1) {
			int16 signedValue = argv[0];

			debugC(kDebugScript, "Opcode %d: Add Signed Value to Var", op);
			debugC(kDebugScript, "\tVar: %d", var);
			debugC(kDebugScript, "\tsignedValue: %d", signedValue);

			_vm->_varStore->setVar(var, _vm->_varStore->getVar(var) + signedValue);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_110(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kSeleniticStack:
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
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_111(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kSeleniticStack:
		if (argc == 0) {
			// Used on Card 1245 (Sound Receiver)
			// Used by Source Selection Buttons...

			debugC(kDebugScript, "Opcode %d: Unknown", op);
			// TODO: Fill in Function...
		} else
			unknown(op, var, argc, argv);
		break;
	case kStoneshipStack:
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
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_112(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kStoneshipStack:
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
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_113(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used on Myst 4143 (Dock near Marker Switch)
		if (argc == 9) {
			uint16 soundId = argv[0];
				
			uint16 u0 = argv[1];
			uint16 u1 = argv[2];

			Common::Rect rect = Common::Rect(argv[3], argv[4], argv[5], argv[6]);

			uint16 updateDirection = argv[7];
			uint16 u2 = argv[8];
				
			debugC(kDebugScript, "Opcode %d: Vault Open Logic", op);
			debugC(kDebugScript, "\tsoundId: %d", soundId);
			debugC(kDebugScript, "\tu0: %d", u0);
			debugC(kDebugScript, "\tu1: %d", u1);

			debugC(kDebugScript, "\trect.left: %d", rect.left);
			debugC(kDebugScript, "\trect.top: %d", rect.top);
			debugC(kDebugScript, "\trect.right: %d", rect.right);
			debugC(kDebugScript, "\trect.bottom: %d", rect.bottom);
			debugC(kDebugScript, "\trect updateDirection: %d", updateDirection);
			debugC(kDebugScript, "\tu2: %d", u2);

			if ((_vm->_varStore->getVar(2) == 1) &&
			   (_vm->_varStore->getVar(3) == 1) &&
			   (_vm->_varStore->getVar(4) == 0) &&
			   (_vm->_varStore->getVar(5) == 1) &&
			   (_vm->_varStore->getVar(6) == 1) &&
			   (_vm->_varStore->getVar(7) == 1) &&
			   (_vm->_varStore->getVar(8) == 1) &&
			   (_vm->_varStore->getVar(9) == 1)) {
				// TODO: Implement correct function...
				// Blit Image in Left to Right Vertical stripes i.e. transistion
				// like door opening
				_vm->_sound->playSound(soundId);
				// TODO: Set 41 to 1 if page already present in hand.
				_vm->_varStore->setVar(41, 2);
			}
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_114(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used on Myst 4143 (Dock near Marker Switch)
		if (argc == 9) {
			uint16 soundId = argv[0];
				
			uint16 u0 = argv[1];
			uint16 u1 = argv[2];

			Common::Rect rect = Common::Rect(argv[3], argv[4], argv[5], argv[6]);

			uint16 updateDirection = argv[7];
			uint16 u2 = argv[8];
			
			debugC(kDebugScript, "Opcode %d: Vault Close Logic", op);
			debugC(kDebugScript, "\tsoundId: %d", soundId);
			debugC(kDebugScript, "\tu0: %d", u0);
			debugC(kDebugScript, "\tu1: %d", u1);

			debugC(kDebugScript, "\trect.left: %d", rect.left);
			debugC(kDebugScript, "\trect.top: %d", rect.top);
			debugC(kDebugScript, "\trect.right: %d", rect.right);
			debugC(kDebugScript, "\trect.bottom: %d", rect.bottom);
			debugC(kDebugScript, "\tupdateDirection: %d", updateDirection);
			debugC(kDebugScript, "\tu2: %d", u2);

			if ((_vm->_varStore->getVar(2) == 1) &&
			   (_vm->_varStore->getVar(3) == 1) &&
			   (_vm->_varStore->getVar(4) == 1) &&
			   (_vm->_varStore->getVar(5) == 1) &&
			   (_vm->_varStore->getVar(6) == 1) &&
			   (_vm->_varStore->getVar(7) == 1) &&
			   (_vm->_varStore->getVar(8) == 1) &&
			   (_vm->_varStore->getVar(9) == 1)) {
				// TODO: Implement correct function...
				// Blit Image in Right to Left Vertical stripes i.e. transistion
				// like door closing
				_vm->_sound->playSound(soundId);
				_vm->_varStore->setVar(41, 0);
			}
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_115(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kSeleniticStack:
		varUnusedCheck(op, var);

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
		break;
	case kDemoPreviewStack:
	case kMystStack:
		if (argc == 3) {
			uint16 cardIdLose = argv[0];
			uint16 cardIdBookCover = argv[1];
			uint16 soundIdAddPage = argv[2];

			debugC(kDebugScript, "Opcode %d: Red and Blue Book/Page Interaction", op);
			debugC(kDebugScript, "Var: %d", var);
			debugC(kDebugScript, "Card Id (Lose): %d", cardIdLose);
			debugC(kDebugScript, "Card Id (Book Cover): %d", cardIdBookCover);
			debugC(kDebugScript, "SoundId (Add Page): %d", soundIdAddPage);

			// TODO: if holding page for this book, play SoundIdAddPage
			if (false) { // TODO: Should be access to mainCursor...
				_vm->_sound->playSound(soundIdAddPage);
				// TODO: Code for updating variables based on adding page
			}

			// TODO: Add Tweak to improve original logic by denying
			//       lose until all red / blue pages collected, rather
			//       than allowing shortcut based on 1 fireplace page?

			// If holding last page for this book i.e. var 24/25
			// Then trigger Trapped in Book Losing Ending
			if ((var == 100 && !_vm->_varStore->getVar(25)) ||
			   (var == 101 && !_vm->_varStore->getVar(24))) {
				// TODO: Clear mainCursor back to nominal..
				_vm->changeToCard(cardIdLose);
			} else
				_vm->changeToCard(cardIdBookCover);
	
			// TODO: Is this logic here?
			//       i.e. If was holding page, wait then auto open and play book...
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		varUnusedCheck(op, var);
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_116(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kStoneshipStack:
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
		break;
	case kMystStack:
		if (argc == 1) {
			// Used on Card 4006 (Clock Tower Time Controls)
			uint16 soundId = argv[0];

			debugC(kDebugScript, "Opcode %d: Clock Tower Bridge Puzzle Execute Button", op);

			uint16 bridgeState = _vm->_varStore->getVar(12);
			uint16 currentTime = _vm->_varStore->getVar(43);

			const uint16 correctTime = 32; // 2:40 i.e. From 12 Noon in 5 min increments

			if (!bridgeState && currentTime == correctTime) {
				_vm->_sound->playSound(soundId);

				// TODO: Play only 1st half of movie i.e. gears rise up
				_vm->_video->playMovie(_vm->wrapMovieFilename("gears", kMystStack), 305, 36);
					
				bridgeState = 1;
				_vm->_varStore->setVar(12, bridgeState);
			} else if (bridgeState && currentTime != correctTime) {
				_vm->_sound->playSound(soundId);

				// TODO: Play only 2nd half of movie i.e. gears sink down
				_vm->_video->playMovie(_vm->wrapMovieFilename("gears", kMystStack), 305, 36);

				bridgeState = 0;
				_vm->_varStore->setVar(12, bridgeState);
			}
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_117(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kStoneshipStack:
		varUnusedCheck(op, var);

		if (argc == 0) {
			// Used on Card 2132 (Chest at Bottom of Lighthouse)
			// Called when Valve Hotspot Clicked.
			// TODO: Fill in Function to play right section of movie
			//       based on valve state and water in chest..
			_vm->_video->playMovie(_vm->wrapMovieFilename("ligspig", kStoneshipStack), 97, 267);
		} else
			unknown(op, var, argc, argv);
		break;
	case kMystStack:
		if (argc == 2) {
			// Used by Myst Imager Control Button
			uint16 varValue = _vm->_varStore->getVar(var);

			if (varValue)
				_vm->_sound->playSound(argv[1]);
			else
				_vm->_sound->playSound(argv[0]);

			_vm->_varStore->setVar(var, !varValue);
			// TODO: Change Var 45 "Dock Forechamber Imager Water Effect Enabled" here?
		} else
			unknown(op, var, argc, argv);
		break;
	case kChannelwoodStack:
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
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_118(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kStoneshipStack:
		if (argc == 0) {
			// Used on Card 2126 (Lighthouse Looking Along Plank)
			// Called when Exit Resource is clicked

			// TODO: Implement Function...
			// If holding Key to Lamp Room Trapdoor, drop to bottom of
			// Lighthouse...
		} else
			unknown(op, var, argc, argv);
		break;
	case kMystStack:
		varUnusedCheck(op, var);

		if (argc == 5) {
			// Used by Card 4709 (Myst Imager Control Panel Red Button)

			debugC(kDebugScript, "Opcode %d: Imager Change Value", op);

			uint16 soundIdBeepLo = argv[0];
			uint16 soundIdBeepHi = argv[1];
			uint16 soundIdBwapp = argv[2];
			uint16 soundIdBeepTune = argv[3]; // 5 tones..
			uint16 soundIdPanelSlam = argv[4];

			debugC(kDebugScript, "\tsoundIdBeepLo: %d", soundIdBeepLo);
			debugC(kDebugScript, "\tsoundIdBeepHi: %d", soundIdBeepHi);
			debugC(kDebugScript, "\tsoundIdBwapp: %d", soundIdBwapp);
			debugC(kDebugScript, "\tsoundIdBeepTune: %d", soundIdBeepTune);
			debugC(kDebugScript, "\tsoundIdPanelSlam: %d", soundIdPanelSlam);

			_vm->_sound->playSound(soundIdBeepLo);

			// TODO: Complete Logic...
		} else
			unknown(op, var, argc, argv);
		break;
	case kChannelwoodStack:
		varUnusedCheck(op, var);

		if (argc == 0) {
			// Used on Card 3318 (Sirrus' Room Nightstand Drawer)
			// Triggered when clicked on drawer
			// TODO: Implement function...
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_119(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kStoneshipStack:
		if (argc == 0) {
			// Used on Card 2143 (Lighthouse Trapdoor)
			// Called when Lock Hotspot Clicked while holding key.
			_vm->_video->playMovie(_vm->wrapMovieFilename("openloc", kStoneshipStack), 187, 72);
		} else
			unknown(op, var, argc, argv);
		break;
	case kMystStack:
		if (argc == 1) {
			// Used on Card 4383 and 4451 (Tower Elevator)
			switch (argv[0]) {
			case 0:
				_vm->_video->playMovie(_vm->wrapMovieFilename("libdown", kMystStack), 216, 78);
				break;
			case 1:
				_vm->_video->playMovie(_vm->wrapMovieFilename("libup", kMystStack), 214, 75);
				break;
			default:
				break;
			}
		} else
			unknown(op, var, argc, argv);
		break;
	case kChannelwoodStack:
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
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_120(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	MystResource *_top;

	switch (_vm->getCurStack()) {
	case kStoneshipStack:
		if (argc == 1) {
			// Used for Cards 2285, 2289, 2247, 2251 (Side Doors in Tunnels Down To Brothers Rooms)
			uint16 movieId = argv[0];

			debugC(kDebugScript, "Opcode %d: Play Side Door Movies", op);
			debugC(kDebugScript, "\tmovieId: %d", movieId);

			switch (movieId) {
			case 0:
				// Card 2251
				_vm->_video->playMovie(_vm->wrapMovieFilename("tunaup", kStoneshipStack), 149, 161);
				break;
			case 1:
				// Card 2247
				_vm->_video->playMovie(_vm->wrapMovieFilename("tunadown", kStoneshipStack), 218, 150);
				break;
			case 2:
				// Card 2289
				_vm->_video->playMovie(_vm->wrapMovieFilename("tuncup", kStoneshipStack), 259, 161);
				break;
			case 3:
				// Card 2285
				_vm->_video->playMovie(_vm->wrapMovieFilename("tuncdown", kStoneshipStack), 166, 150);
				break;
			default:
				warning("Opcode 120 MovieId Out Of Range");
				break;
			}
		} else
			unknown(op, var, argc, argv);
		break;
	case kMystStack:
		// Used for Card 4297 (Generator Puzzle Buttons)
		debugC(kDebugScript, "Opcode %d: Toggle Var8 of Invoking Resource", op);
		_top = _invokingResource;
			
		while(_top->_parent != NULL)
			_top = _top->_parent;

		if (argc == 0) {
			uint16 var8 = _top->getType8Var();
			if (var8 != 0xFFFF)
				_vm->_varStore->setVar(var8, !_vm->_varStore->getVar(var8));
			else
				warning("Opcode 120: No invoking Resource Var 8 found!");
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_121(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used on Card 4100 (Cabin Safe Buttons)
		// Correct Solution (724) -> Var 67=2, 68=7, 69=5 
		// Jump to Card 4103 when solution correct and handle pulled...
		if (argc == 0) {
			uint16 varValue = _vm->_varStore->getVar(var);
			if (varValue == 0)
				varValue = 9;
			else
				varValue--;
			_vm->_varStore->setVar(var, varValue);
		} else
			unknown(op, var, argc, argv);
		break;
	case kMechanicalStack:
		varUnusedCheck(op, var);

		if (argc == 2) {
			uint16 startTime = argv[0];
			uint16 endTime = argv[1];

			printf("TODO: Opcode %d Movie Time Index %d to %d\n", op, startTime, endTime);
			// TODO: Need version of playMovie blocking which allows selection
			//       of start and finish points.
			_vm->_video->playMovie(_vm->wrapMovieFilename("ewindow", kMechanicalStack), 253, 0);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		varUnusedCheck(op, var);

		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_122(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kChannelwoodStack:
		// Used on Channelwood Card 3280
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Do Water Tank Valve Close Animation", op);
			Common::Rect rect = _invokingResource->getRect();

			// TODO: Need to load the image ids from Script Resources structure of VIEW
			for (uint16 imageId = 3601; imageId >= 3595; imageId--) {
				_vm->_gfx->copyImageToScreen(imageId, rect);
				_vm->_system->delayMillis(50);
			}

			// TODO: Is 8 gotten from var7 of calling hotspot, rather than hard-coded?
			_vm->_varStore->setVar(8, 0);
			_vm->_varStore->setVar(19, 0);
		} else
			unknown(op, var, argc, argv);
		break;
	case kMechanicalStack:
		if (argc == 0) {
			// Used on Card 6120 (Elevator)
			// Called when Exit Midde Button Pressed

			// TODO: hcelev? Movie of Elevator?
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_123(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMechanicalStack:
		if (argc == 2) {
			// Used on Card 6154
			uint16 start_time = argv[0];
			uint16 end_time = argv[1];

			printf("TODO: Opcode %d Movie Time Index %d to %d\n", op, start_time, end_time);
			// TODO: Need version of playMovie blocking which allows selection
			//       of start and finish points.
			// TODO: Not 100% sure about movie position
			_vm->_video->playMovie(_vm->wrapMovieFilename("hcelev", kMechanicalStack), 205, 40);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_124(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMechanicalStack:
		if (argc == 0) {
			// Used by Card 6156 (Fortress Rotation Controls)
			// Called when Red Exit Button Pressed to raise Elevator

			// TODO: Fill in Code...
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_125(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kStoneshipStack:
		if (argc == 1) {
			// Used on Card 2197 (Sirrus' Room Drawers)
			debugC(kDebugScript, "Opcode %d: Unknown uses Var %d", op, var);

			uint16 u0 = argv[0];

			debugC(kDebugScript, "\tu0: %d", u0);
			// TODO: Fill in Logic...
		} else
			unknown(op, var, argc, argv);
		break;
	case kMechanicalStack:
		if (argc == 0) {
			// Used on Card 6267 (Code Lock)
			// Called by Red Execute Button...
			debugC(kDebugScript, "Opcode %d: Code Lock Execute...", op);

			// TODO: Fill in Logic For Code Lock...
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_126(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMechanicalStack:
		if (argc == 0) {
			// Used by Card 6120 (Fortress Elevator)
			// Called when Red Exit Button Pressed to raise Elevator and
			// exit is clicked...

			// TODO: Fill in Code...
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_127(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kChannelwoodStack:
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
		break;
	case kMechanicalStack:
		if (argc == 0) {
			// Used for Mech Card 6226 (3 Crystals)
			_vm->_varStore->setVar(20, 1);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_128(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMechanicalStack:
		if (argc == 0) {
			// Used for Mech Card 6226 (3 Crystals)
			_vm->_varStore->setVar(20, 0);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_129(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kChannelwoodStack:
		if (argc == 1) {
			// Used by Card 3262 (Elevator)
			debugC(kDebugScript, "Opcode %d: Unknown...", op);

			uint16 cardId = argv[0];

			debugC(kDebugScript, "\tcardId: %d", cardId);

			// TODO: Fill in Code...
			_vm->changeToCard(cardId);
		} else
			unknown(op, var, argc, argv);
		break;
	case kMechanicalStack:
		if (argc == 0) {
			// Used for Mech Card 6226 (3 Crystals)
			_vm->_varStore->setVar(21, 1);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_130(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMechanicalStack:
		if (argc == 0) {
			// Used for Mech Card 6226 (3 Crystals)
			_vm->_varStore->setVar(21, 0);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_131(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMechanicalStack:
		if (argc == 0) {
			// Used for Mech Card 6226 (3 Crystals)
			_vm->_varStore->setVar(22, 1);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_132(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMechanicalStack:
		if (argc == 0) {
			// Used for Mech Card 6226 (3 Crystals)
			_vm->_varStore->setVar(22, 0);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_133(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used on Card 4500 (Stellar Observatory Controls)
		if (argc == 1) {
			// Called by Telescope Slew Button
			uint16 soundId = argv[0];

			// TODO: Function to change variables controlling telescope view
			//       etc.
				
			// TODO: Sound seems to be stuck looping?
			_vm->_sound->playSound(soundId);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_147(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		if (argc == 0) {
			// TODO: Extra Logic to do this in INIT process watching cursor and var 98?
			_vm->_varStore->setVar(98, 0);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_164(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used on Card 4530 (Rocketship Music Slider Controls)
		// TODO: Finish Implementation...
		// Var 105 is used to set between 0 to 2 = No Function, Movie Playback and Linkable...
		// This is called when Var 105 = 1 i.e. this plays back Movie...
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_169(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used on Card 4099 (In Cabin, Looking Out Door)
		// TODO: Finish Implementation...
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_181(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		if (argc == 0) {
			// TODO: Logic for lighting the match
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_182(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		if (argc == 0) {
			// TODO: Logic for lighting the match
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_183(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		if (argc == 0) {
			// Used for Myst Cards 4257, 4260, 4263, 4266, 4269, 4272, 4275 and 4278 (Ship Puzzle Boxes)
			_vm->_varStore->setVar(105, 1);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_184(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		if (argc == 0) {
			// Used for Myst Cards 4257, 4260, 4263, 4266, 4269, 4272, 4275 and 4278 (Ship Puzzle Boxes)
			_vm->_varStore->setVar(105, 0);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_185(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		if (argc == 0) {
			// Used for Myst Card 4098 (Cabin Boiler Pilot Light)
			// TODO: Extra Logic to do this in INIT process watching cursor and var 98?
			_vm->_varStore->setVar(98, 1);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_196(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kDemoPreviewStack:
		// Used on Card ...
		// TODO: Finish Implementation...
		// Voice Over and Card Advance?
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_197(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kDemoPreviewStack:
		// Used on Card ...
		// TODO: Finish Implementation...
		// Voice Over and Card Advance?
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

// TODO: Merge with Opcode 42?
void MystScriptParser::opcode_198(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kDemoPreviewStack:
		if (argc == 0) {
			// Nuh-uh! No leaving the library in the demo!
			GUI::MessageDialog dialog("You can't leave the library in the demo.");
			dialog.runModal();
		} else
			unknown(op, var, argc, argv);
		break;
	case kMystStack:
		// Used on Card 4143 (Dock near Marker Switch, facing Cogs)
		if (argc == 9) {
			uint16 soundId = argv[0];
			uint16 u0 = argv[1];
			uint16 u1 = argv[2];
			Common::Rect rect = Common::Rect(argv[3], argv[4], argv[5], argv[6]);
			uint16 updateDirection = argv[7];
			uint16 u2 = argv[8];

			debugC(kDebugScript, "Opcode %d: Close Dock Marker Switch Vault", op);
			debugC(kDebugScript, "\tsoundId: %d", soundId);
			debugC(kDebugScript, "\tu0: %d", u0);
			debugC(kDebugScript, "\tu1: %d", u1);

			debugC(kDebugScript, "\trect.left: %d", rect.left);
			debugC(kDebugScript, "\trect.top: %d", rect.top);
			debugC(kDebugScript, "\trect.right: %d", rect.right);
			debugC(kDebugScript, "\trect.bottom: %d", rect.bottom);
			debugC(kDebugScript, "\tupdateDirection: %d", updateDirection);
			debugC(kDebugScript, "\tu2: %d", u2);

			Audio::SoundHandle *handle;
			if (_vm->_varStore->getVar(41) != 0) {
				handle = _vm->_sound->playSound(soundId);

				while (_vm->_mixer->isSoundHandleActive(*handle))
					_vm->_system->delayMillis(10);
				// TODO: Do Image Blit
			}
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_199(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Myst Imager Control Execute Button Logic", op);
				
			uint16 numericSelection = (_vm->_varStore->getVar(36) + 1) % 10;
			numericSelection += ((_vm->_varStore->getVar(35) + 1) % 10) * 10;

			debugC(kDebugScript, "\tImager Selection: %d", numericSelection);

			switch (numericSelection) {
			case 40:
				_vm->_varStore->setVar(51, 1); // Mountain
				break;
			case 67:
				_vm->_varStore->setVar(51, 2); // Water
				break;
			case 47:
				_vm->_varStore->setVar(51, 4); // Marker Switch
				break;
			case 8:
				_vm->_varStore->setVar(51, 3); // Atrus
				break;
			default:
				_vm->_varStore->setVar(51, 0); // Blank
				break;
			}

			// TODO: Fill in Logic
			//{  34, 2, "Dock Forechamber Imager State" }, // 0 to 2 = Off, Mountain, Water
			//{ 310, 0, "Dock Forechamber Imager Control Temp Value?" }
		} else
			unknown(op, var, argc, argv);
		break;
	case kDemoPreviewStack:
		// Used on Card ...
		// TODO: Finish Implementation...
		// Voice Over and Card Advance?
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
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

	uint16 var;
	uint16 imageCount;
	uint16 imageBaseId;
	uint16 soundDecrement;
	uint16 soundIncrement;

	// Myst Demo slideshow variables
	uint16 cardId;
	uint32 lastCardTime;
} g_opcode200Parameters;

void MystScriptParser::opcode_200_run() {
	static uint16 lastImageIndex = 0;
	uint16 curImageIndex;
	Common::Rect rect;

	if (g_opcode200Parameters.enabled) {
		switch (_vm->getCurStack()) {
		case kSeleniticStack:
			// Used on Card 1191 (Maze Runner)

			// TODO: Implementation Movie Function..
			if (false) {
				_vm->_video->playMovie(_vm->wrapMovieFilename(kHCMovPathSelenitic[0], kSeleniticStack), 201, 26);
			}
			break;
		case kStoneshipStack:
			// Used for Card 2013 (Achenar's Rose-Skull Hologram)

			// TODO: Implement Function...
			break;
		case kDemoPreviewStack:
		case kMystStack:
			curImageIndex = _vm->_varStore->getVar(g_opcode200Parameters.var);

			if (curImageIndex >= g_opcode200Parameters.imageCount) {
				curImageIndex = g_opcode200Parameters.imageCount - 1;
				_vm->_varStore->setVar(g_opcode200Parameters.var, curImageIndex);
			}

			// HACK: Think these images are centered on screen (when smaller than full screen),
			//       and since no _gfx call for image size, hack this to deal with this case for now...
			if (_vm->getCurCard() == 4059)
				rect = Common::Rect(157, 115, 544, 333);
			else
				rect = Common::Rect(0, 0, 544, 333);

			_vm->_gfx->copyImageToScreen(g_opcode200Parameters.imageBaseId + curImageIndex, rect);

			// TODO: Comparison with original engine shows that this simple solution
			//       may not be the correct one and the choice of which sound
			//       may be more complicated or even random..
			if (curImageIndex < lastImageIndex && g_opcode200Parameters.soundDecrement != 0)
				_vm->_sound->playSound(g_opcode200Parameters.soundDecrement);
			else if (curImageIndex > lastImageIndex && g_opcode200Parameters.soundIncrement != 0)
				_vm->_sound->playSound(g_opcode200Parameters.soundIncrement);

			lastImageIndex = curImageIndex;
			break;
		case kCreditsStack:
			curImageIndex = _vm->_varStore->getVar(g_opcode200Parameters.var);
			
			if (_vm->_system->getMillis() - g_opcode200Parameters.lastCardTime >= 7 * 1000) {
				// After the 6th image has shown, it's time to quit
				if (curImageIndex == 7)
					_vm->_system->quit();

				// Note: The modulus by 6 is because the 6th image is the one at imageBaseId
				_vm->_gfx->copyImageToScreen(g_opcode200Parameters.imageBaseId + curImageIndex % 6, Common::Rect(0, 0, 544, 333));

				_vm->_varStore->setVar(g_opcode200Parameters.var, curImageIndex + 1);
				g_opcode200Parameters.lastCardTime = _vm->_system->getMillis();
			}
			break;
		case kMechanicalStack:
			// Used on Card 6238 (Sirrus' Throne) and Card 6027 (Achenar's Throne)
			// g_opcode200Parameters.var == 0 for Achenar
			// g_opcode200Parameters.var == 1 for Sirrus

			// TODO: Fill in Function... 
			// Variable indicates that this is related to Secret Panel State
			break;
		case kDemoStack:
			// Used on Card 2000

			// TODO: Fill in Function...
			break;
		case kDemoSlidesStack:
			// Used on Cards...
			if (_vm->_system->getMillis() - g_opcode200Parameters.lastCardTime >= 2 * 1000)
				_vm->changeToCard(g_opcode200Parameters.cardId);
			break;
		}
	}
}

void MystScriptParser::opcode_200_disable() {
	g_opcode200Parameters.enabled = false;
	g_opcode200Parameters.var = 0;
	g_opcode200Parameters.imageCount = 0;
	g_opcode200Parameters.imageBaseId = 0;
	g_opcode200Parameters.soundDecrement = 0;
	g_opcode200Parameters.soundIncrement = 0;
}

void MystScriptParser::opcode_200(uint16 op, uint16 var, uint16 argc, uint16 *argv) {	
	switch (_vm->getCurStack()) {
	case kIntroStack:
		varUnusedCheck(op, var);

		// TODO: Play Intro Movies..
		// and then _vm->changeToCard(2);
		unknown(op, var, argc, argv);
		break;
	case kSeleniticStack:
		varUnusedCheck(op, var);

		// Used for Card 1191 (Maze Runner)
		if (argc == 0) {
			g_opcode200Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kStoneshipStack:
		varUnusedCheck(op, var);

		// Used for Card 2013 (Achenar's Rose-Skull Hologram)
		if (argc == 0) {
			g_opcode200Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kDemoPreviewStack:
	case kMystStack:
		if (argc == 4) {
			g_opcode200Parameters.var = var;
			g_opcode200Parameters.imageCount = argv[0];
			g_opcode200Parameters.imageBaseId = argv[1];
			g_opcode200Parameters.soundDecrement = argv[2];
			g_opcode200Parameters.soundIncrement = argv[3];
			g_opcode200Parameters.enabled = true;

			_vm->_varStore->setVar(var, 0);
		} else
			unknown(op, var, argc, argv);
		break;
	case kMechanicalStack:
		// Used on Card 6238 (Sirrus' Throne) and Card 6027 (Achenar's Throne)
		if (argc == 0) {
			g_opcode200Parameters.var = var;
			g_opcode200Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kDniStack:
		varUnusedCheck(op, var);
		// Used on Card 5014

		// TODO: Logic for Atrus Reactions and Movies
		if (false) {
			// Var 0 used for Atrus Gone (from across room) 0 = Present, 1 = Not Present
			// Var 1 used for Myst Book Status 0 = Not Usuable
			//                                 1 = Openable, but not linkable (Atrus Gone?)
			//                                 2 = Linkable
			// Var 2 used for Music Type 0 to 2..
			// Var 106 used for Atrus Static Image State 0 = Initial State
			//                                           1 = Holding Out Hand for Page
			//                                           2 = Gone, Book Open
			//                                           3 = Back #1
			//                                           4 = Back #2
			_vm->_video->playMovie(_vm->wrapMovieFilename("atr1nopg", kDniStack), 215, 77);
			_vm->_video->playMovie(_vm->wrapMovieFilename("atr1page", kDniStack), 215, 77);
			_vm->_video->playMovie(_vm->wrapMovieFilename("atrus2", kDniStack), 215, 77);
			_vm->_video->playMovie(_vm->wrapMovieFilename("atrwrite", kDniStack), 215, 77);
		}
		break;
	case kCreditsStack:
		if (argc == 0) {
			g_opcode200Parameters.var = var;
			// TODO: Pass ImageCount, rather than hardcoded in run process?
			g_opcode200Parameters.imageBaseId = _vm->getCurCard();
			g_opcode200Parameters.lastCardTime = _vm->_system->getMillis();
			g_opcode200Parameters.enabled = true;

			_vm->_varStore->setVar(var, 1);				
		} else
			unknown(op, var, argc, argv);
		break;
	case kDemoStack:
		// Used on Card 2000
		if (argc == 0) {
			g_opcode200Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kDemoSlidesStack:
		// Used on Cards...
		if (argc == 1) {
			g_opcode200Parameters.cardId = argv[0];
			g_opcode200Parameters.lastCardTime = _vm->_system->getMillis();
			g_opcode200Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

static struct {
	uint16 u0;
	uint16 u1;
	uint16 u2;

	uint16 lastVar105;
	uint16 soundId;

	bool enabled;
} g_opcode201Parameters;

void MystScriptParser::opcode_201_run() {
	uint16 var105;

	if (g_opcode201Parameters.enabled) {
		switch (_vm->getCurStack()) {
		case kSeleniticStack:
			// Used for Card 1191 (Maze Runner)

			// TODO: Fill in Function...
			break;
		case kStoneshipStack:
			// Used for Card 2013 (Achenar's Rose-Skull Hologram)

			// TODO: Fill in Function...
			break;
		case kMystStack:
			var105 = _vm->_varStore->getVar(105);
			if (var105 && !g_opcode201Parameters.lastVar105)
				_vm->_sound->playSound(g_opcode201Parameters.soundId);
			g_opcode201Parameters.lastVar105 = var105;
			break;
		case kMechanicalStack:
			// Used for Card 6159 (Facing Corridor to Fortress Elevator)

			// g_opcode201Parameters.u0
			// g_opcode201Parameters.u1
			// g_opcode201Parameters.u2

			// TODO: Fill in Function...
			break;
		case kDemoStack:
			// Used on Card 2001, 2002 and 2003

			// TODO: Fill in Function...
			break;
		}
	}
}

void MystScriptParser::opcode_201_disable() {
	g_opcode201Parameters.enabled = false;
	g_opcode201Parameters.soundId = 0;
	g_opcode201Parameters.lastVar105 = 0;
}

void MystScriptParser::opcode_201(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kIntroStack:
		_vm->_system->delayMillis(4 * 1000);
		_vm->_gfx->copyImageToScreen(4, Common::Rect(0, 0, 544, 333));
		break;
	case kSeleniticStack:
		// Used for Card 1191 (Maze Runner)

		if (argc == 0) {
			g_opcode201Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kStoneshipStack:
		varUnusedCheck(op, var);

		// Used for Card 2013 (Achenar's Rose-Skull Hologram)
		if (argc == 0) {
			g_opcode201Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kMystStack:
		// Used for Cards 4257, 4260, 4263, 4266, 4269, 4272, 4275 and 4278 (Ship Puzzle Boxes)
		if (argc == 1) {
			g_opcode201Parameters.soundId = argv[0];
			g_opcode201Parameters.lastVar105 = 0;
			g_opcode201Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kMechanicalStack:
		// Used for Card 6159 (Facing Corridor to Fortress Elevator)
		if (argc == 3) {
			g_opcode201Parameters.u0 = argv[0];
			g_opcode201Parameters.u1 = argv[1];
			g_opcode201Parameters.u2 = argv[2];

			g_opcode201Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kChannelwoodStack:
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
		break;
	case kDemoStack:
		// Used on Card 2001, 2002 and 2003
		if (argc == 0) {
			g_opcode201Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

static struct {
	bool enabled;
	uint16 var;
} g_opcode202Parameters;

void MystScriptParser::opcode_202_run(void) {
	if (g_opcode202Parameters.enabled) {
		switch (_vm->getCurStack()) {
		case kSeleniticStack:
			// Used for Card 1191 (Maze Runner)

			// TODO: Fill in function...
			break;
		case kDemoPreviewStack:
		case kMystStack:
			// Used for Card 4378 (Library Tower Rotation Map)
			// TODO: Fill in.. Code for Tower Rotation Angle etc..
			// Var 0, 3, 4, 5, 6, 7, 8, 9 used for Type 8 Image Display
			// Type 11 Hotspot for control..
			// Var 304 controls presence of Myst Library Image
			break;
		case kMechanicalStack:
			// Used for Card 6220 (Sirrus' Mechanical Bird)
			// TODO: Fill in Function
			break;
		case kChannelwoodStack:
			// Used for Cards 3328, 3691, 3731, 3809, 3846 etc. (Water Valves)

			// Code for Water Flow Logic
			// Var 8 = "Water Tank Valve State"
			// Controls
			// Var 19 = "Water Flowing to First Water Valve"
			// Code for this in Opcode 104 / 122

			// Var 19 = "Water Flowing to First Water Valve"
			// and
			// Var 9 = "First Water Valve State"
			// Controls
			// Var 20 = "Water Flowing to Second (L) Water Valve"
			// Var 3 = "Water Flowing (R) to Pump for Upper Walkway to Temple Elevator"
			uint16 var9 = _vm->_varStore->getVar(9);
			if (_vm->_varStore->getVar(19)) {
				_vm->_varStore->setVar(20, !var9);
				_vm->_varStore->setVar(3, var9);
			} else {
				// No water into Valve
				_vm->_varStore->setVar(20, 0);
				_vm->_varStore->setVar(3, 0);
			}

			// Var 20 = "Water Flowing to Second (L) Water Valve"
			// and
			// Var 10 = "Second (L) Water Valve State"
			// Controls
			// Var 24 = "Water Flowing to Third (L, L) Water Valve"
			// Var 21 = "Water Flowing to Third (L, R) Water Valve"
			uint16 var10 = _vm->_varStore->getVar(10);
			if (_vm->_varStore->getVar(20)) {
				_vm->_varStore->setVar(24, !var10);
				_vm->_varStore->setVar(21, var10);
			} else {
				// No water into Valve
				_vm->_varStore->setVar(24, 0);
				_vm->_varStore->setVar(21, 0);
			}

			// Var 21 = "Water Flowing to Third (L, R) Water Valve"
			// and
			// Var 11 = "Third (L, R) Water Valve State"
			// Controls
			// Var 23 = "Water Flowing to Fourth (L, R, L) Water Valve"
			// Var 22 = "Water Flowing to Fourth (L, R, R) Water Valve"
			uint16 var11 = _vm->_varStore->getVar(11);
			if (_vm->_varStore->getVar(21)) {
				_vm->_varStore->setVar(23, !var11);
				_vm->_varStore->setVar(22, var11);
			} else {
				// No water into Valve
				_vm->_varStore->setVar(23, 0);
				_vm->_varStore->setVar(22, 0);
			}

			// Var 24 = "Water Flowing to Third (L, L) Water Valve"
			// and
			// Var 14 = "Third (L, L) Water Valve State"
			// Controls
			// Var 29 = "Water Flowing to Pipe In Water (L, L, L)"
			// Var 28 = "Water Flowing to Join and Pump Bridge (L, L, R)"
			uint16 var14 = _vm->_varStore->getVar(14);
			if (_vm->_varStore->getVar(24)) {
				_vm->_varStore->setVar(29, !var14);
				_vm->_varStore->setVar(28, var14);
			} else {
				// No water into Valve
				_vm->_varStore->setVar(29, 0);
				_vm->_varStore->setVar(28, 0);
			}

			// Var 22 = "Water Flowing to Fourth (L, R, R) Water Valve"
			// and
			// Var 12 = "Fourth (L, R, R) Water Valve State"
			// Controls
			// Var 25 = "Water Flowing to Pipe Bridge (L, R, R, L)"
			// Var 15 = "Water Flowing (L, R, R, R) to Pump for Lower Walkway to Upper Walkway Elevator"
			uint16 var12 = _vm->_varStore->getVar(12);
			if (_vm->_varStore->getVar(22)) {
				_vm->_varStore->setVar(25, !var12);
				_vm->_varStore->setVar(15, var12);
			} else {
				// No water into Valve
				_vm->_varStore->setVar(25, 0);
				_vm->_varStore->setVar(15, 0);
			}

			// Var 23 = "Water Flowing to Fourth (L, R, L) Water Valve"
			// and
			// Var 13 = "Fourth (L, R, L) Water Valve State"
			// Controls
			// Var 27 = "Water Flowing to Join and Pump Bridge (L, R, L, L)"
			// Var 26 = "Water Flowing to Pipe At Entry Point (L, R, L, R)"
			uint16 var13 = _vm->_varStore->getVar(13);
			if (_vm->_varStore->getVar(23)) {
				_vm->_varStore->setVar(27, !var13);
				_vm->_varStore->setVar(26, var13);
			} else {
				// No water into Valve
				_vm->_varStore->setVar(27, 0);
				_vm->_varStore->setVar(26, 0);
			}

			// TODO: Not sure that original had OR logic for water flow at Join...
			// Var 27 = "Water Flowing to Join and Pump Bridge (L, R, L, L)"
			// Or
			// Var 28 = "Water Flowing to Join and Pump Bridge (L, L, R)"
			// Controls
			// Var 31 = "Water Flowing to Join (L, L, R)" // 0 to 2 = Stop Sound, Background, Background with Water Flow
			// Var 7 = "Bridge Pump Running"
			// TODO: Not sure about control of Var 31 which is tristate...
			if (_vm->_varStore->getVar(27) || _vm->_varStore->getVar(28)) {
				_vm->_varStore->setVar(31, 2); // Background with Water Flow
				_vm->_varStore->setVar(7, 1);
			} else {
				// No water into Valve
				_vm->_varStore->setVar(31, 1); // Background 
				_vm->_varStore->setVar(7, 0);
			}

			// TODO: Code for this shouldn't be here...
			//       Move to Opcodes called by Pipe Extension...
			// Var 25 = "Water Flowing to Pipe Bridge (L, R, R, L)"
			// and
			// Var 6 = "Pipe Bridge Extended"
			// Controls
			// Var 32 = "Water Flowing (L, R, R, L, Pipe) State" }, // 0 to 2 = Stop Sound, Background, Background with Water Flow
			// Var 4 = "Water Flowing (L, R, R, L, Pipe Extended) to Pump for Book Room Elevator"
			// TODO: Not sure about control of Var 32 which is tristate...
			if (_vm->_varStore->getVar(25) && _vm->_varStore->getVar(6)) {
				_vm->_varStore->setVar(32, 2); // Background with Water Flow
				_vm->_varStore->setVar(4, 1);
			} else {
				// No water into Valve
				_vm->_varStore->setVar(32, 1); // Background 
				_vm->_varStore->setVar(4, 0);
			}
			break;
		}
	}
}

void MystScriptParser::opcode_202_disable(void) {
	g_opcode202Parameters.enabled = false;
}

void MystScriptParser::opcode_202(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kSeleniticStack:
		varUnusedCheck(op, var);

		// Used for Card 1191 (Maze Runner)
		if (argc == 0) {
			g_opcode202Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kStoneshipStack:
		varUnusedCheck(op, var);

		// Used for Card 2160 (Lighthouse Battery Pack Closeup)
		// TODO: Implement Code...
		// Not Sure of Purpose - Update of Light / Discharge?
		unknown(op, var, argc, argv);
		break;
	case kDemoPreviewStack:
	case kMystStack:
		varUnusedCheck(op, var);

		// Used for Card 4378 (Library Tower Rotation Map)
		if (argc == 1) {
			// TODO: Figure Out argv[0] purpose.. number of image resources?
			g_opcode202Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kMechanicalStack:
		// Used for Card 6220 (Sirrus' Mechanical Bird)
		if (argc == 0) {
			g_opcode202Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kChannelwoodStack:
		// Used for Cards 3328, 3691, 3731, 3809, 3846 etc. (Water Valves)
		if (argc == 0) {
			g_opcode202Parameters.var = var;
			g_opcode202Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		varUnusedCheck(op, var);

		unknown(op, var, argc, argv);
		break;
	}
}

static struct {
	bool enabled;
} g_opcode203Parameters;

void MystScriptParser::opcode_203_run(void) {
	if (g_opcode203Parameters.enabled) {
		switch (_vm->getCurStack()) {
		case kSeleniticStack:
			// Used for Card 1245 (Sound Receiver)
			// TODO: Fill in Logic to Change Viewer Display etc.?
			break;
		case kMystStack:
			// Used for Card 4138 (Dock Forechamber Door)
			// TODO: Fill in Logic..
			break;
		case kMechanicalStack:
			// Used for Card 6043 (Weapons Rack with Snake Box)
			// TODO: Fill in Logic for Snake Box...
			break;
		case kChannelwoodStack:
			// Used for Card 3310 (Sirrus' Room Right Bed Drawer),
			// Card 3307 (Sirrus' Room Left Bed Drawer)
			// and Card 3318 (Sirrus' Room Nightstand Drawer)
			// TODO: Fill in Logic...
			break;
		}
	}
}

void MystScriptParser::opcode_203_disable(void) {
	g_opcode203Parameters.enabled = false;
}

void MystScriptParser::opcode_203(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	uint16 imageIdDarkDoorOpen = 0;
	uint16 imageIdDarkDoorClosed = 0;

	switch (_vm->getCurStack()) {
	case kSeleniticStack:
		// Used for Card 1245 (Sound Receiver)
		if (argc == 0) {
			g_opcode203Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kStoneshipStack:
		// Used for all/most Cards in Tunnels Down To Brothers Rooms

		// TODO: Duplicate or similar function to Opcode 203?
		if (argc == 2 || argc == 4) {
			debugC(kDebugScript, "Opcode %d: %d Arguments", op, argc);

			uint16 u0 = argv[0];
			if (argc == 4) {
				imageIdDarkDoorOpen = argv[1];
				imageIdDarkDoorClosed = argv[2];
			}
			uint16 soundIdAlarm = argv[argc - 1];

			debugC(kDebugScript, "\tu0: %d", u0);
			if (argc == 4) {
				debugC(kDebugScript, "\timageIdDarkDoorOpen: %d", imageIdDarkDoorOpen);
				debugC(kDebugScript, "\tsoundIdDarkDoorClosed: %d", imageIdDarkDoorClosed);
			}
			debugC(kDebugScript, "\tsoundIdAlarm: %d", soundIdAlarm);

			// TODO: Fill in Correct Function for Lights
		} else
			unknown(op, var, argc, argv);
		break;
	case kMystStack:
		// Used for Card 4138 (Dock Forechamber Door)
		if (argc == 0) {
			g_opcode203Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kMechanicalStack:
		// Used for Card 6043 (Weapons Rack with Snake Box)
		if (argc == 0) {
			g_opcode203Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kChannelwoodStack:
		// Used for Card 3310 (Sirrus' Room Right Bed Drawer),
		// Card 3307 (Sirrus' Room Left Bed Drawer)
		// and Card 3318 (Sirrus' Room Nightstand Drawer)
		if (argc == 0) {
			g_opcode203Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

static struct {
	bool enabled;
	uint16 soundId;
} g_opcode204Parameters;

void MystScriptParser::opcode_204_run(void) {
	if (g_opcode204Parameters.enabled) {
		switch (_vm->getCurStack()) {
		case kSeleniticStack:
			// Used for Card 1147 (Sound Code Lock)
			// TODO: Fill in code for Sound Lock...
			break;
		case kMystStack:
			// Used for Card 4134 and 4149 (Dock)
			// TODO: Fill in..
			break;
		case kMechanicalStack:
			// TODO: Fill in Logic.
			// Var 12 holds Large Cog Position in range 0 to 5
			// - For animation
			// Var 11 holds C position in range 0 to 9
			// - 4 for Correct Answer
			// C Movement Sound
			//_vm->_sound->playSound(g_opcode204Parameters.soundId);
			break;
		}
	}
}

void MystScriptParser::opcode_204_disable(void) {
	g_opcode204Parameters.enabled = false;
}

void MystScriptParser::opcode_204(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kSeleniticStack:
		// Used for Card 1147 (Sound Code Lock)
		if (argc == 0) {
			g_opcode204Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kStoneshipStack:
		// Used for Card 2160 (Lighthouse Battery Pack Closeup)
		if (argc == 0) {
			// TODO: Implement Code For Battery Meter Level
			// Overwrite _vm->_resources[1]->_subImages[0].rect.bottom 1 to 80
			// Add accessor functions for this...
		} else
			unknown(op, var, argc, argv);
		break;
	case kMystStack:
		// Used for Card 4134 and 4149 (Dock)
		// TODO: Fill in logic.. Unsure of exact function to trigger and location on screen..
		if (false) {
			// Card 4134
			_vm->_video->playMovie(_vm->wrapMovieFilename("birds1", kMystStack), 416, 0);
			
			// Card 4149
			_vm->_video->playMovie(_vm->wrapMovieFilename("birds2", kMystStack), 433, 0);
			
			// Unsure...
			_vm->_video->playMovie(_vm->wrapMovieFilename("birds3", kMystStack), 0, 0);
		}
		break;
	case kMechanicalStack:
		// Used for Card 6180 (Lower Elevator Puzzle)
		if (argc == 1) {
			g_opcode204Parameters.soundId = argv[0];
			g_opcode204Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

static struct {
	uint16 soundIdPosition[4];

	bool enabled;
} g_opcode205Parameters;

void MystScriptParser::opcode_205_run(void) {
	if (g_opcode205Parameters.enabled) {
		switch (_vm->getCurStack()) {
		case kSeleniticStack:
			// Used for Card 1191 (Maze Runner)
			// TODO: Fill in function...
			break;
		case kMystStack:
			// Used for Card 4532 (Rocketship Piano)
			// TODO: Fill in function...
			break;
		case kMechanicalStack:
			// Used for Card 6156 (Fortress Rotation Controls)
			// TODO: Fill in function...
			// g_opcode205Parameters.soundIdPosition[4]
			break;
		}
	}
}

void MystScriptParser::opcode_205_disable(void) {
	g_opcode205Parameters.enabled = false;
}

void MystScriptParser::opcode_205(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	uint16 imageIdDoorOpen = 0;
	uint16 imageIdDoorClosed = 0;

	switch (_vm->getCurStack()) {
	case kSeleniticStack:
		// Used for Card 1191 (Maze Runner)

		if (argc == 0) {
			g_opcode205Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kStoneshipStack:
		// Used on Cards 2322, 2285 (Tunnels Down To Brothers Rooms)

		// TODO: Duplicate or similar function to Opcode 203?
		if (argc == 2 || argc == 4) {
			debugC(kDebugScript, "Opcode %d: %d Arguments", op, argc);

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
		break;
	case kMystStack:
		// Used for Card 4532 (Rocketship Piano)

		if (argc == 0) {
			g_opcode205Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kMechanicalStack:
		// Used for Card 6156 (Fortress Rotation Controls)

		if (argc == 4) {
			g_opcode205Parameters.soundIdPosition[0] = argv[0];
			g_opcode205Parameters.soundIdPosition[1] = argv[1];
			g_opcode205Parameters.soundIdPosition[2] = argv[2];
			g_opcode205Parameters.soundIdPosition[3] = argv[3];

			g_opcode205Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

static struct {
	uint16 soundIdStart[2];
	uint16 soundIdPosition[4];

	bool enabled;
} g_opcode206Parameters;

void MystScriptParser::opcode_206_run(void) {
	if (g_opcode206Parameters.enabled) {
		switch (_vm->getCurStack()) {
		case kSeleniticStack:
			// Used for Card 1191 (Maze Runner)
			// TODO: Fill in function...
			break;
		case kMechanicalStack:
			// Used for Card 6044 (Fortress Rotation Simulator)

			// g_opcode206Parameters.soundIdStart[2]
			// g_opcode206Parameters.soundIdPosition[4]

			// TODO: Fill in function...
			break;
		}
	}
}

void MystScriptParser::opcode_206_disable(void) {
	g_opcode206Parameters.enabled = false;
}

void MystScriptParser::opcode_206(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kSeleniticStack:
		// Used for Card 1191 (Maze Runner)

		if (argc == 0) {
			g_opcode206Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kStoneshipStack:
		// Used for Cards 2272 and 2234 (Facing Out of Door)
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Unknown, %d Arguments", op, argc);
			// TODO: Function Unknown...
		} else
			unknown(op, var, argc, argv);
		break;
	case kMystStack:
		// Used for Card 4256 (Butterfly Movie Activation)
		// TODO: Implement Logic...
		break;
	case kMechanicalStack:
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
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_207(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kStoneshipStack:
		// Used for Card 2138 (Lighthouse Key/Chest Animation Logic)
		// TODO: Fill in function
		warning("TODO: Opcode 207 Lighthouse Key/Chest Animation Logic");
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_208(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kStoneshipStack:
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
		break;
	case kMystStack:
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Imager Function", op);
			debugC(kDebugScript, "Var: %d", var);

			// TODO: Fill in Correct Function
			if (false) {
				_vm->_video->playMovie(_vm->wrapMovieFilename("vltmntn", kMystStack), 159, 97);
			}
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

static struct {
	uint16 u0[5];
	uint16 u1[5];
	uint16 stateVar;

	uint16 soundId;

	bool enabled;
} g_opcode209Parameters;

void MystScriptParser::opcode_209_run(void) {
	static bool enabledLast;
	
	if (g_opcode209Parameters.enabled) {
		switch (_vm->getCurStack()) {
		case kStoneshipStack:
			// Used for Card 2004 (Achenar's Room Drawers)

			// TODO: Implement Function...
			// Swap Open Drawers?
			break;
		case kDemoPreviewStack:
		case kMystStack:
			// Used for Card 4334 and 4348 (Myst Library Bookcase Door)
			if (!enabledLast)
				// TODO: If Variable changed...
				_vm->_sound->playSound(g_opcode209Parameters.soundId);

				// TODO: Code to trigger Type 6 to play movie...
			break;
		case kMechanicalStack:
			// Used for Card 6044 (Fortress Rotation Simulator)

			// TODO: Implement Function For Secret Panel State as
			//       per Opcode 200 function (Mechanical)
			break;
		default:
			break;
		}
	}

	enabledLast = g_opcode209Parameters.enabled;
}

void MystScriptParser::opcode_209_disable(void) {
	g_opcode209Parameters.enabled = false;
}

void MystScriptParser::opcode_209(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kStoneshipStack:
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
		break;
	case kDemoPreviewStack:
	case kMystStack:
		// Used for Card 4334 and 4348 (Myst Library Bookcase Door)
		if (argc == 1) {
			g_opcode209Parameters.soundId = argv[0];
			g_opcode209Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	case kMechanicalStack:
		// Used for Card 6044 (Fortress Rotation Simulator)
		if (argc == 0) {
			g_opcode209Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

static struct {
	bool enabled;
} g_opcode210Parameters;

void MystScriptParser::opcode_210_run(void) {
	if (g_opcode210Parameters.enabled) {
		// Code for Generator Puzzle

		// Var 52 to 61 Hold Button State for 10 generators
		// Var 64, 65 - 2 8-Segments for Rocket Power Dial
		// Var 62, 63 - 2 8-Segments for Power Dial
		// Var 96, 97 - Needle for Power and Rocket Power Dials

		// Var 44 Holds State for Rocketship
		// 0 = No Power
		// 1 = Insufficient Power
		// 2 = Correct Power i.e. 59V

		// Var 93 Holds Breaker nearest Generator State
		// Var 94 Holds Breaker nearest Rocket Ship State
		// 0 = Closed 1 = Open

		const uint16 correctVoltage = 59;

		// Correct Solution is 4, 7, 8, 9 i.e. 16 + 2 + 22 + 19 = 59
		const uint16 genVoltages[10] = { 10, 7, 8, 16, 5, 1, 2, 22, 19, 9 };

		uint16 powerVoltage = 0;
		uint16 rocketPowerVoltage = 0;

		// Calculate Power Voltage from Generator Contributions
		for (byte i = 0; i < ARRAYSIZE(genVoltages); i++)
			if (_vm->_varStore->getVar(52 + i))
				powerVoltage += genVoltages[i];

		// Logic for Var 49 - Generator Running Sound Control
		if (powerVoltage == 0)
			_vm->_varStore->setVar(49, 0);
		else
			_vm->_varStore->setVar(49, 1);

		// TODO: Animation Code to Spin Up and Spin Down LED Dials?
		// Code For Power Dial Var 62 and 63
		_vm->_varStore->setVar(62, powerVoltage / 10);
		_vm->_varStore->setVar(63, powerVoltage % 10);
		// TODO: Var 96 - Power Needle Logic

		// Code For Breaker Logic
		if (_vm->_varStore->getVar(93) != 0 || _vm->_varStore->getVar(94) != 0)
			rocketPowerVoltage = 0;
		else {
			if (powerVoltage <= correctVoltage)
				rocketPowerVoltage = powerVoltage;
			else {
				// Blow Generator Room Breaker...
				_vm->_varStore->setVar(93, 1);
				// TODO: I think Logic For Blowing Other Breaker etc. 
				// is done in process on Breaker Cards.

				rocketPowerVoltage = 0;
			}
		}

		// TODO: Animation Code to Spin Up and Spin Down LED Dials?
		// Code For Rocket Power Dial
		_vm->_varStore->setVar(64, rocketPowerVoltage / 10);
		_vm->_varStore->setVar(65, rocketPowerVoltage % 10);
		// TODO: Var 97 - Rocket Power Needle Logic

		// Set Rocket Ship Power Based on Power Level
		if (rocketPowerVoltage == 0)
			_vm->_varStore->setVar(44, 0);
		else if (rocketPowerVoltage < correctVoltage)
			_vm->_varStore->setVar(44, 1);
		else if (rocketPowerVoltage == correctVoltage)
			_vm->_varStore->setVar(44, 2);
		else // Should Not Happen Case
			_vm->_varStore->setVar(44, 0);
	}
}

void MystScriptParser::opcode_210_disable(void) {
	g_opcode210Parameters.enabled = false;
}

void MystScriptParser::opcode_210(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kStoneshipStack:
		varUnusedCheck(op, var);

		// Used in Cards 2205 and 2207 (Cloud Orbs in Sirrus' Room)
		if (argc == 2) {
			uint16 soundId = argv[0];
			uint16 soundIdStopping = argv[1];

			// TODO: Work Out Function i.e. control Var etc.
			if (false) {
				_vm->_sound->playSound(soundId);
				_vm->_sound->playSound(soundIdStopping);
			}
		} else
			unknown(op, var, argc, argv);
		break;
	case kMystStack:
		// Used for Card 4297 (Generator Puzzle)
		if (argc == 2) {
			// TODO: Work Out 2 parameters meaning... 16, 17
			// Script Resources for Generator Spinup and Spindown Sounds?
			g_opcode210Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

static struct {
	bool enabled;
} g_opcode211Parameters;

void MystScriptParser::opcode_211_run(void) {
	const uint16 imageBaseId = 4779;
	const uint16 gridBaseLeft = 173;
	const uint16 gridBaseTop = 168;

	static uint16 lastGridState[6];
	uint16 gridState[6];
	uint16 image;

	if (g_opcode211Parameters.enabled) {
		// Grid uses Var 17 to 22 as bitfields (8 horizontal cells x 6 vertical)
		for (byte i = 0; i < 6; i++) {
			gridState[i] = _vm->_varStore->getVar(i + 17);

			if (gridState[i] != lastGridState[i]) {
				for (byte j = 0; j < 8; j++) {
					// TODO: Animation Code
					if ((gridState[i] >> (7 - j)) & 1)
						image = 16;
					else
						image = 0;

					_vm->_gfx->copyImageToScreen(imageBaseId + image, Common::Rect(gridBaseLeft + (j * 26), gridBaseTop + (i * 26), gridBaseLeft + ((j + 1) * 26), gridBaseTop + ((i + 1) * 26)));
				}
			}

			lastGridState[i] = gridState[i];
		}

		// Var 23 contains boolean for whether pattern matches correct book pattern i.e. Pattern 158
		if (gridState[0] == 0xc3 && gridState[1] == 0x6b && gridState[2] == 0xa3 &&
		    gridState[3] == 0x93 && gridState[4] == 0xcc && gridState[5] == 0xfa)
			_vm->_varStore->setVar(23, 1);
		else
			_vm->_varStore->setVar(23, 0);
	}
}

void MystScriptParser::opcode_211_disable(void) {
	g_opcode211Parameters.enabled = false;
}

void MystScriptParser::opcode_211(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kDemoPreviewStack:
	case kMystStack:
		// Used for Card 4059 (Fireplace Puzzle)
		if (argc == 0) {
			for (byte i = 0; i < 6; i++)
				_vm->_varStore->setVar(i + 17, 0);

			g_opcode211Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

static struct {
	bool enabled;
} g_opcode212Parameters;

void MystScriptParser::opcode_212_run(void) {
	if (g_opcode212Parameters.enabled) {
		// TODO: Implement Correct Code for Myst Clock Tower Cog Puzzle
		// Card 4113

		if (false) {
			// 3 videos to be played of Cog Movement
			// TODO: Not 100% sure of movie positions.
			_vm->_video->playMovie(_vm->wrapMovieFilename("cl1wg1", kMystStack), 220, 50);
			_vm->_video->playMovie(_vm->wrapMovieFilename("cl1wg2", kMystStack), 220, 80);
			_vm->_video->playMovie(_vm->wrapMovieFilename("cl1wg3", kMystStack), 220, 110);

			// 1 video of weight descent
			_vm->_video->playMovie(_vm->wrapMovieFilename("cl1wlfch", kMystStack), 123, 0);

			// Video of Cog Open on Success
			_vm->_video->playMovie(_vm->wrapMovieFilename("cl1wggat", kMystStack), 195, 225);
			// Var 40 set on success
			_vm->_varStore->setVar(40, 1);
		}
	}
}

void MystScriptParser::opcode_212_disable(void) {
	g_opcode212Parameters.enabled = false;
}

void MystScriptParser::opcode_212(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4113 (Clock Tower Cog Puzzle)
		if (argc == 0) {
			g_opcode212Parameters.enabled = true;
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_213(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4524 (Dockside Facing Towards Ship)
		if (argc == 0) {
			// TODO: Implement Code...
			// Code for Gull Videos?
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_214(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4500 (Stellar Observatory)
		if (argc == 5) {
			debugC(kDebugScript, "Opcode %d: Unknown...", op);

			uint16 u0 = argv[0];
			uint16 u1 = argv[1];
			uint16 u2 = argv[2];
			uint16 u3 = argv[3];
			uint16 u4 = argv[4];

			debugC(kDebugScript, "\tu0: %d", u0);
			debugC(kDebugScript, "\tu1: %d", u1);
			debugC(kDebugScript, "\tu2: %d", u2);
			debugC(kDebugScript, "\tu3: %d", u3);
			debugC(kDebugScript, "\tu4: %d", u4);
			// TODO: Complete Implementation...
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_215(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4134 (Dock Facing Marker Switch)
		// TODO: Fill in logic. Logic for Gull Videos?
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_216(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4571 (Channelwood Tree)
		if (argc == 0) {
			// TODO: Fill in logic for Tree Position From Far...
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_217(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4601 (Channelwood Tree)
		if (argc == 2) {
			// TODO: Fill in logic for Tree Position Close Up...
			// 2 arguments: 4, 4
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_218(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4097 (Cabin Boiler)
		// TODO: Fill in logic
		if (false) {
			_vm->_video->playMovie(_vm->wrapMovieFilename("cabfirfr", kMystStack), 254, 244);
			_vm->_video->playMovie(_vm->wrapMovieFilename("cabcgfar", kMystStack), 254, 138);
		}

		// Used for Card 4098 (Cabin Boiler)
		// TODO: Fill in logic
		if (false) {
			_vm->_video->playMovie(_vm->wrapMovieFilename("cabfire", kMystStack), 240, 279);
			_vm->_video->playMovie(_vm->wrapMovieFilename("cabingau", kMystStack), 243, 97);
		}
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_219(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4530 (Rocketship Music Puzzle)
		if (argc == 5) {
			debugC(kDebugScript, "Opcode %d: Unknown...", op);

			uint16 u0 = argv[0];
			uint16 u1 = argv[1];
			uint16 u2 = argv[2];
			uint16 u3 = argv[3];
			uint16 u4 = argv[4];

			debugC(kDebugScript, "\tu0: %d", u0);
			debugC(kDebugScript, "\tu1: %d", u1);
			debugC(kDebugScript, "\tu2: %d", u2);
			debugC(kDebugScript, "\tu3: %d", u3);
			debugC(kDebugScript, "\tu4: %d", u4);
			// TODO: Fill in logic...
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_220(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4530 (Rocketship Music Puzzle Video)
		// TODO: Fill in logic.
		if (false) {
			// loop?
			_vm->_video->playMovie(_vm->wrapMovieFilename("selenbok", kMystStack), 224, 41);
		}
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_221(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4168 (Green Book Movies)
		// Movie plays in resource #0 rect
		// TODO: Not sure if subsection is looped...
		if (!_vm->_varStore->getVar(302)) {
			// HACK: Stop Wind Sounds.. Think this is a problem at library entrance.
			_vm->_sound->stopSound();
			_vm->_video->playBackgroundMovie(_vm->wrapMovieFilename("atrusbk1", kMystStack), 314, 76);
			_vm->_varStore->setVar(302, 1);
		} else {
			// HACK: Stop Wind Sounds.. Think this is a problem at library entrance.
			_vm->_sound->stopSound();
			_vm->_video->playBackgroundMovie(_vm->wrapMovieFilename("atrusbk2", kMystStack), 314, 76);
		}
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_222(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4141 (Myst Dock Facing Sea)
		if (argc == 0) {
			// TODO: Logic for Gull Videos?
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_298(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kDemoPreviewStack:
		// Used for Card 3000 (Closed Myst Book)
		// TODO: Fill in logic.
		// Start Voice Over... which controls book opening
		_vm->_sound->playSound(3001);
			
		// then link to Myst - Trigger of Hotspot? then opcode 199/196/197 for voice over continue?
		// TODO: Sync Voice and Actions to Original
		// TODO: Flash Library Red
		// TODO: Move to run process based delay to prevent
		//       blocking...
		_vm->_system->delayMillis(20 * 1000);
		for (uint16 imageId = 3001; imageId <= 3012; imageId++) {
			_vm->_gfx->copyImageToScreen(imageId, Common::Rect(0, 0, 544, 333));
			_vm->_system->delayMillis(5 * 1000);
		}
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_299(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kDemoPreviewStack:
		// Used for Card 3002 (Myst Island Overview)
		// TODO: Fill in logic.
		// Zoom into Island?
		// On this card is a Type 8 controlled by Var 0, which
		// can change the Myst Library to Red..
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_300(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kIntroStack:
		varUnusedCheck(op, var);
		// TODO: StopSound?
		unknown(op, var, argc, argv);
		break;
	case kDemoPreviewStack:
	case kMystStack:
		// Used in Card 4371 (Blue Book) Var = 101 
		//     and Card 4363 (Red Book)  Var = 100
		// TODO: Fill in Logic
		debugC(kDebugScript, "Opcode %d: Book Exit Function...", op);
		debugC(kDebugScript, "Var: %d", var);
		break;
	case kStoneshipStack:
		// Used in Card 2218 (Telescope view)
		varUnusedCheck(op, var);
		// TODO: Fill in Logic. Clearing Variable for View?
		break;
	case kMechanicalStack:
		// Used in Card 6156 (Fortress Elevator View)
		varUnusedCheck(op, var);
		// TODO: Fill in Logic. Clearing Variable for View?
		break;
	case kChannelwoodStack:
		// Used in Card 3012 (Achenar's Holoprojector Control)
		varUnusedCheck(op, var);
		// TODO: Fill in Logic. Clearing Variable for View?
		break;
	case kDniStack:
		// Used in Card 5014 (Atrus Writing)
		varUnusedCheck(op, var);
		// TODO: Fill in Logic.
		break;
	case kDemoStack:
		// Used on Card 2000
		varUnusedCheck(op, var);

		// TODO: Fill in Function...
		break;
	default:
		varUnusedCheck(op, var);
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_301(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kDemoPreviewStack:
	case kMystStack:
		// Used in Card 4080 (Fireplace Book) and Other Myst Library Books
		// TODO: Fill in Logic. Clear Variable on Book exit.. or Copy from duplicate..
		_vm->_varStore->setVar(0, 1);
		break;
	default:
		varUnusedCheck(op, var);
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_302(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used in Card 4113 (Clock Tower Cog Puzzle)
		// TODO: Fill in Logic
		break;
	default:
		varUnusedCheck(op, var);
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_303(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4141 (Myst Dock Facing Sea)
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Clear Dock Forechamber Door Variable", op);
			_vm->_varStore->setVar(105, 0);
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_304(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4601 (Channelwood Tree)
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Unknown...", op);
			// TODO: Logic for clearing variable?
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_305(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4601 (Channelwood Tree)
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Unknown...", op);
			// TODO: Logic for clearing variable?
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_306(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4098 (Cabin Boiler Puzzle)
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Unknown...", op);
			// TODO: Logic for clearing variable?
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_307(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4299 (Generator Room Controls)
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Unknown...", op);
			// TODO: Logic for clearing variable?
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_308(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4530 (Rocketship Music Sliders)
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Unknown...", op);
			// TODO: Logic for clearing variable?
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_309(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4168 (Green D'ni Book Open), Red Book Open and Blue Book Open
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Unknown...", op);
			// TODO: Logic for clearing variable?
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

void MystScriptParser::opcode_312(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	switch (_vm->getCurStack()) {
	case kMystStack:
		// Used for Card 4698 (Dock Forechamber Imager)
		if (argc == 0) {
			debugC(kDebugScript, "Opcode %d: Unknown...", op);
			// TODO: Logic for clearing variable?
		} else
			unknown(op, var, argc, argv);
		break;
	default:
		unknown(op, var, argc, argv);
		break;
	}
}

} // End of namespace Mohawk
