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
#include "mohawk/myst_stacks/myst.h"

#include "gui/message.h"

namespace Mohawk {

// NOTE: Credits Start Card is 10000

MystScriptParser_Myst::MystScriptParser_Myst(MohawkEngine_Myst *vm) : MystScriptParser(vm) {
	setupOpcodes();

	// Card ID preinitialized by the engine for use by opcode 18
	// when linking back to Myst in the library
	if (_vm->getCurStack() == kMystStack)
		_savedCardId = 4329;
}

MystScriptParser_Myst::~MystScriptParser_Myst() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &MystScriptParser_Myst::x, #x))

void MystScriptParser_Myst::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(101, opcode_101);
	OPCODE(102, opcode_102);
	OPCODE(103, opcode_103);
	OPCODE(104, opcode_104);
	OPCODE(105, opcode_105);
	OPCODE(109, opcode_109);
	OPCODE(113, opcode_113);
	OPCODE(114, opcode_114);
	OPCODE(115, opcode_115);
	OPCODE(116, opcode_116);
	OPCODE(117, opcode_117);
	OPCODE(118, opcode_118);
	OPCODE(119, opcode_119);
	OPCODE(120, opcode_120);
	OPCODE(121, opcode_121);
	OPCODE(122, opcode_122);
	OPCODE(123, opcode_123);
	OPCODE(129, opcode_129);
	OPCODE(130, opcode_130);
	OPCODE(131, opcode_131);
	OPCODE(132, opcode_132);
	OPCODE(133, opcode_133);
	OPCODE(134, opcode_134);
	OPCODE(135, opcode_135);
	OPCODE(136, opcode_136);
	OPCODE(137, opcode_137);
	OPCODE(146, opcode_146);
	OPCODE(147, opcode_147);
	OPCODE(149, opcode_149);
	OPCODE(150, opcode_150);
	OPCODE(151, opcode_151);
	OPCODE(164, opcode_164);
	OPCODE(169, opcode_169);
	OPCODE(170, opcode_170);
	OPCODE(171, opcode_171);
	OPCODE(172, opcode_172);
	OPCODE(173, opcode_173);
	OPCODE(174, opcode_174);
	OPCODE(175, opcode_175);
	OPCODE(176, opcode_176);
	OPCODE(177, opcode_177);
	OPCODE(180, opcode_180);
	OPCODE(181, opcode_181);
	OPCODE(182, opcode_182);
	OPCODE(183, opcode_183);
	OPCODE(184, opcode_184);
	OPCODE(185, opcode_185);
	OPCODE(186, opcode_186);
	OPCODE(188, opcode_188);
	OPCODE(189, opcode_189);
	OPCODE(190, opcode_190);
	OPCODE(191, opcode_191);
	OPCODE(192, opcode_192);
	OPCODE(194, opcode_194);
	OPCODE(195, opcode_195);
	OPCODE(196, opcode_196);
	OPCODE(197, opcode_197);
	OPCODE(198, opcode_198);
	OPCODE(199, opcode_199);

	// "Init" Opcodes
	OPCODE(200, opcode_200);
	OPCODE(201, opcode_201);
	OPCODE(202, opcode_202);
	OPCODE(203, opcode_203);
	OPCODE(204, opcode_204);
	OPCODE(205, opcode_205);
	OPCODE(206, opcode_206);
	OPCODE(208, opcode_208);
	OPCODE(209, opcode_209);
	OPCODE(210, opcode_210);
	OPCODE(211, opcode_211);
	OPCODE(212, opcode_212);
	OPCODE(213, opcode_213);
	OPCODE(214, opcode_214);
	OPCODE(215, opcode_215);
	OPCODE(216, opcode_216);
	OPCODE(217, opcode_217);
	OPCODE(218, opcode_218);
	OPCODE(219, opcode_219);
	OPCODE(220, opcode_220);
	OPCODE(221, opcode_221);
	OPCODE(222, opcode_222);

	// "Exit" Opcodes
	OPCODE(300, opcode_300);
	OPCODE(301, opcode_301);
	OPCODE(302, opcode_302);
	OPCODE(303, opcode_303);
	OPCODE(304, opcode_304);
	OPCODE(305, opcode_305);
	OPCODE(306, opcode_306);
	OPCODE(307, opcode_307);
	OPCODE(308, opcode_308);
	OPCODE(309, opcode_309);
	OPCODE(312, opcode_312);
}

#undef OPCODE

void MystScriptParser_Myst::disablePersistentScripts() {
	opcode_200_disable();
	opcode_201_disable();
	opcode_202_disable();
	opcode_203_disable();
	opcode_205_disable();
	opcode_209_disable();
	opcode_210_disable();
	opcode_211_disable();
	opcode_212_disable();
}

void MystScriptParser_Myst::runPersistentScripts() {
	opcode_200_run();
	opcode_201_run();
	opcode_202_run();
	opcode_203_run();
	opcode_205_run();
	opcode_209_run();
	opcode_210_run();
	opcode_211_run();
	opcode_212_run();
}

void MystScriptParser_Myst::opcode_101(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Decrement Variable", op);
	if (argc == 0) {
		debugC(kDebugScript, "\tvar: %d", var);
		uint16 varValue = _vm->_varStore->getVar(var);
		// Logic to prevent decrement to negative
		if (varValue != 0)
			_vm->_varStore->setVar(var, varValue - 1);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_102(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Increment Variable", op);
		debugC(kDebugScript, "\tvar: %d", var);

		// AFAIK no logic to put ceiling on increment at least in this opcode
		_vm->_varStore->setVar(var, _vm->_varStore->getVar(var) + 1);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_103(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
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
}

void MystScriptParser_Myst::opcode_104(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
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
}

void MystScriptParser_Myst::opcode_105(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
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
}

void MystScriptParser_Myst::opcode_109(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 1) {
		int16 signedValue = argv[0];

		debugC(kDebugScript, "Opcode %d: Add Signed Value to Var", op);
		debugC(kDebugScript, "\tVar: %d", var);
		debugC(kDebugScript, "\tsignedValue: %d", signedValue);

		_vm->_varStore->setVar(var, _vm->_varStore->getVar(var) + signedValue);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_113(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

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
}

void MystScriptParser_Myst::opcode_114(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

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
}

void MystScriptParser_Myst::opcode_115(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
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
			_vm->changeToCard(cardIdLose, true);
		} else
			_vm->changeToCard(cardIdBookCover, true);

		// TODO: Is this logic here?
		//       i.e. If was holding page, wait then auto open and play book...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_116(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

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
}

void MystScriptParser_Myst::opcode_117(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
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
}

void MystScriptParser_Myst::opcode_118(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
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
}

void MystScriptParser_Myst::opcode_119(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

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
}

void MystScriptParser_Myst::opcode_120(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4297 (Generator Puzzle Buttons)
	debugC(kDebugScript, "Opcode %d: Toggle Var8 of Invoking Resource", op);
	MystResource *_top = _invokingResource;

	while (_top->_parent != NULL)
		_top = _top->_parent;

	if (argc == 0) {
		uint16 var8 = _top->getType8Var();
		if (var8 != 0xFFFF)
			_vm->_varStore->setVar(var8, !_vm->_varStore->getVar(var8));
		else
			warning("Opcode 120: No invoking Resource Var 8 found");
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_121(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
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
}

void MystScriptParser_Myst::opcode_122(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4100
	// TODO: Mouse down on handle
}

void MystScriptParser_Myst::opcode_123(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4100
	// TODO: Mouse drag on handle
}

void MystScriptParser_Myst::opcode_129(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Month increase
}

void MystScriptParser_Myst::opcode_130(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Month decrease
}

void MystScriptParser_Myst::opcode_131(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Day increase
}

void MystScriptParser_Myst::opcode_132(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Day decrease
}

void MystScriptParser_Myst::opcode_133(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

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
}

void MystScriptParser_Myst::opcode_134(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Month slider movement
}

void MystScriptParser_Myst::opcode_135(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Day slider movement
}

void MystScriptParser_Myst::opcode_136(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Year slider movement
}

void MystScriptParser_Myst::opcode_137(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Time slider movement
}

void MystScriptParser_Myst::opcode_146(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4098
	// TODO: Boiler wheel clockwise mouse down
}

void MystScriptParser_Myst::opcode_147(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// TODO: Extra Logic to do this in INIT process watching cursor and var 98?
		_vm->_varStore->setVar(98, 0);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_149(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4098
	// TODO: Boiler wheel clockwise mouse up
}

void MystScriptParser_Myst::opcode_150(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4098
	// TODO: Boiler wheel counter-clockwise mouse down
}

void MystScriptParser_Myst::opcode_151(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4098
	// TODO: Boiler wheel counter-clockwise mouse up
}

void MystScriptParser_Myst::opcode_164(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used on Card 4530 (Rocketship Music Slider Controls)
	// TODO: Finish Implementation...
	// Var 105 is used to set between 0 to 2 = No Function, Movie Playback and Linkable...
	// This is called when Var 105 = 1 i.e. this plays back Movie...
}

void MystScriptParser_Myst::opcode_169(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used on Card 4099 (In Cabin, Looking Out Door)
	// TODO: Finish Implementation...
}

void MystScriptParser_Myst::opcode_170(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Month slider mouse down
}

void MystScriptParser_Myst::opcode_171(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Month slider mouse up
}

void MystScriptParser_Myst::opcode_172(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Day slider mouse down
}

void MystScriptParser_Myst::opcode_173(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Day slider mouse up
}

void MystScriptParser_Myst::opcode_174(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Year slider mouse down
}

void MystScriptParser_Myst::opcode_175(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Year slider mouse up
}

void MystScriptParser_Myst::opcode_176(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Time slider mouse down
}

void MystScriptParser_Myst::opcode_177(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Time slider mouse up
}

void MystScriptParser_Myst::opcode_180(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4059
	// TODO: Draw fireplace combination book page
}

void MystScriptParser_Myst::opcode_181(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// TODO: Logic for lighting the match
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_182(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// TODO: Logic for lighting the match
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_183(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used for Myst Cards 4257, 4260, 4263, 4266, 4269, 4272, 4275 and 4278 (Ship Puzzle Boxes)
		_vm->_varStore->setVar(105, 1);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_184(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used for Myst Cards 4257, 4260, 4263, 4266, 4269, 4272, 4275 and 4278 (Ship Puzzle Boxes)
		_vm->_varStore->setVar(105, 0);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_185(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used for Myst Card 4098 (Cabin Boiler Pilot Light)
		// TODO: Extra Logic to do this in INIT process watching cursor and var 98?
		_vm->_varStore->setVar(98, 1);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_186(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4006
	// TODO: Minute wheel turn
}

void MystScriptParser_Myst::opcode_188(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4006
	// TODO: Redraw time wheels?
}

void MystScriptParser_Myst::opcode_189(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4006
	// TODO: Hour wheel turn
}

void MystScriptParser_Myst::opcode_190(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4059
	// TODO: Increase fireplace combination book page
}

void MystScriptParser_Myst::opcode_191(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4059
	// TODO: Decrease fireplace combination book page
}

void MystScriptParser_Myst::opcode_192(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Time increase
}

void MystScriptParser_Myst::opcode_194(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Constellation drawing
}

void MystScriptParser_Myst::opcode_195(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Time decrease
}

void MystScriptParser_Myst::opcode_196(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Year increase
}

void MystScriptParser_Myst::opcode_197(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4500
	// TODO: Year decrease
}

// TODO: Merge with Opcode 42?
void MystScriptParser_Myst::opcode_198(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

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

		if (_vm->_varStore->getVar(41) != 0) {
			Audio::SoundHandle *handle = _vm->_sound->playSound(soundId);

			while (_vm->_mixer->isSoundHandleActive(*handle))
				_vm->_system->delayMillis(10);
			// TODO: Do Image Blit
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_199(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

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
}

static struct {
	bool enabled;

	uint16 var;
	uint16 imageCount;
	uint16 imageBaseId;
	uint16 soundDecrement;
	uint16 soundIncrement;
} g_opcode200Parameters;

void MystScriptParser_Myst::opcode_200_run() {
	static uint16 lastImageIndex = 0;

	if (g_opcode200Parameters.enabled) {
		uint16 curImageIndex = _vm->_varStore->getVar(g_opcode200Parameters.var);

		if (curImageIndex >= g_opcode200Parameters.imageCount) {
			curImageIndex = g_opcode200Parameters.imageCount - 1;
			_vm->_varStore->setVar(g_opcode200Parameters.var, curImageIndex);
		}

		Common::Rect rect;

		// HACK: Think these images are centered on screen (when smaller than full screen),
		//       and since no _gfx call for image size, hack this to deal with this case for now...
		if (_vm->getCurCard() == 4059)
			rect = Common::Rect(157, 115, 544, 333);
		else
			rect = Common::Rect(0, 0, 544, 333);

		if (curImageIndex != lastImageIndex)
			_vm->_gfx->copyImageToScreen(g_opcode200Parameters.imageBaseId + curImageIndex, rect);

		// TODO: Comparison with original engine shows that this simple solution
		//       may not be the correct one and the choice of which sound
		//       may be more complicated or even random..
		if (curImageIndex < lastImageIndex && g_opcode200Parameters.soundDecrement != 0)
			_vm->_sound->playSound(g_opcode200Parameters.soundDecrement);
		else if (curImageIndex > lastImageIndex && g_opcode200Parameters.soundIncrement != 0)
			_vm->_sound->playSound(g_opcode200Parameters.soundIncrement);

		lastImageIndex = curImageIndex;
	}
}

void MystScriptParser_Myst::opcode_200_disable() {
	g_opcode200Parameters.enabled = false;
	g_opcode200Parameters.var = 0;
	g_opcode200Parameters.imageCount = 0;
	g_opcode200Parameters.imageBaseId = 0;
	g_opcode200Parameters.soundDecrement = 0;
	g_opcode200Parameters.soundIncrement = 0;
}

void MystScriptParser_Myst::opcode_200(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
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
}

static struct {
	uint16 lastVar105;
	uint16 soundId;

	bool enabled;
} g_opcode201Parameters;

void MystScriptParser_Myst::opcode_201_run() {
	if (g_opcode201Parameters.enabled) {
		uint16 var105 = _vm->_varStore->getVar(105);

		if (var105 && !g_opcode201Parameters.lastVar105)
			_vm->_sound->playSound(g_opcode201Parameters.soundId);

		g_opcode201Parameters.lastVar105 = var105;
	}
}

void MystScriptParser_Myst::opcode_201_disable() {
	g_opcode201Parameters.enabled = false;
	g_opcode201Parameters.soundId = 0;
	g_opcode201Parameters.lastVar105 = 0;
}

void MystScriptParser_Myst::opcode_201(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Cards 4257, 4260, 4263, 4266, 4269, 4272, 4275 and 4278 (Ship Puzzle Boxes)
	if (argc == 1) {
		g_opcode201Parameters.soundId = argv[0];
		g_opcode201Parameters.lastVar105 = 0;
		g_opcode201Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
	uint16 var;
} g_opcode202Parameters;

void MystScriptParser_Myst::opcode_202_run(void) {
	// Used for Card 4378 (Library Tower Rotation Map)
	// TODO: Fill in.. Code for Tower Rotation Angle etc..
	// Var 0, 3, 4, 5, 6, 7, 8, 9 used for Type 8 Image Display
	// Type 11 Hotspot for control..
	// Var 304 controls presence of Myst Library Image
}

void MystScriptParser_Myst::opcode_202_disable(void) {
	g_opcode202Parameters.enabled = false;
}

void MystScriptParser_Myst::opcode_202(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4378 (Library Tower Rotation Map)
	if (argc == 1) {
		// TODO: Figure Out argv[0] purpose.. number of image resources?
		g_opcode202Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode203Parameters;

void MystScriptParser_Myst::opcode_203_run(void) {
	if (g_opcode203Parameters.enabled) {
		// Used for Card 4138 (Dock Forechamber Door)
		// TODO: Fill in Logic. Slide for Dock Forechamber Door?
		// Original has Left to Right Open Slide and Upon leaving card,
		// Right to left Slide before card change.
		//debugC(kDebugScript, "Opcode %d: Clear Dock Forechamber Door Variable", op);
		//_vm->_varStore->setVar(105, 0);
	}
}

void MystScriptParser_Myst::opcode_203_disable(void) {
	g_opcode203Parameters.enabled = false;
}

void MystScriptParser_Myst::opcode_203(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4138 (Dock Forechamber Door)
	// Set forechamber door to closed
	setVarValue(105, 0);
}

void MystScriptParser_Myst::opcode_204(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4134 and 4149 (Dock)
}

static struct {
	bool enabled;
} g_opcode205Parameters;

void MystScriptParser_Myst::opcode_205_run(void) {
	if (g_opcode205Parameters.enabled) {
		// Used for Card 4532 (Rocketship Piano)
		// TODO: Fill in function...
	}
}

void MystScriptParser_Myst::opcode_205_disable(void) {
	g_opcode205Parameters.enabled = false;
}

void MystScriptParser_Myst::opcode_205(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4532 (Rocketship Piano)

	if (argc == 0)
		g_opcode205Parameters.enabled = true;
	else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_206(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4256 (Butterfly Movie Activation)
	// TODO: Implement Logic...
}

void MystScriptParser_Myst::opcode_208(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Imager Function", op);
		debugC(kDebugScript, "Var: %d", var);

		// TODO: Fill in Correct Function
		if (false) {
			_vm->_video->playMovie(_vm->wrapMovieFilename("vltmntn", kMystStack), 159, 97);
		}
	} else
		unknown(op, var, argc, argv);
}

static struct {
	uint16 soundId;

	bool enabled;
} g_opcode209Parameters;

void MystScriptParser_Myst::opcode_209_run(void) {
	static bool enabledLast;

	if (g_opcode209Parameters.enabled) {
		// Used for Card 4334 and 4348 (Myst Library Bookcase Door)
		if (!enabledLast) {
			// TODO: If Variable changed...
			_vm->_sound->playSound(g_opcode209Parameters.soundId);
		}

		// TODO: Code to trigger Type 6 to play movie...
	}

	enabledLast = g_opcode209Parameters.enabled;
}

void MystScriptParser_Myst::opcode_209_disable(void) {
	g_opcode209Parameters.enabled = false;
}

void MystScriptParser_Myst::opcode_209(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4334 and 4348 (Myst Library Bookcase Door)
	if (argc == 1) {
		g_opcode209Parameters.soundId = argv[0];
		g_opcode209Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode210Parameters;

void MystScriptParser_Myst::opcode_210_run(void) {
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

void MystScriptParser_Myst::opcode_210_disable(void) {
	g_opcode210Parameters.enabled = false;
}

void MystScriptParser_Myst::opcode_210(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4297 (Generator Puzzle)
	if (argc == 2) {
		// TODO: Work Out 2 parameters meaning... 16, 17
		// Script Resources for Generator Spinup and Spindown Sounds?
		g_opcode210Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode211Parameters;

void MystScriptParser_Myst::opcode_211_run(void) {
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

		_vm->_gfx->updateScreen();
	}
}

void MystScriptParser_Myst::opcode_211_disable(void) {
	g_opcode211Parameters.enabled = false;
}

void MystScriptParser_Myst::opcode_211(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4059 (Fireplace Puzzle)
	if (argc == 0) {
		for (byte i = 0; i < 6; i++)
			_vm->_varStore->setVar(i + 17, 0);

		g_opcode211Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode212Parameters;

void MystScriptParser_Myst::opcode_212_run(void) {
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

void MystScriptParser_Myst::opcode_212_disable(void) {
	g_opcode212Parameters.enabled = false;
}

void MystScriptParser_Myst::opcode_212(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4113 (Clock Tower Cog Puzzle)
	if (argc == 0)
		g_opcode212Parameters.enabled = true;
	else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_213(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4524 (Dockside Facing Towards Ship)
	if (argc == 0) {
		// TODO: Implement Code...
		// Code for Gull Videos?
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_214(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

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
}

void MystScriptParser_Myst::opcode_215(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4134 (Dock Facing Marker Switch)
	// TODO: Fill in logic for Gull Videos.
	//       may be offset and overlap and need video update to all these
	//       to run in sequence with opcode215_run() process...
	if (false) {
		// All birds(x) videos are 120x48 and played in top right corner of card
		_vm->_video->playMovie(_vm->wrapMovieFilename("birds1", kMystStack), 544-120-1, 0);
		_vm->_video->playMovie(_vm->wrapMovieFilename("birds2", kMystStack), 544-120-1, 0);
		_vm->_video->playMovie(_vm->wrapMovieFilename("birds3", kMystStack), 544-120-1, 0);
	}
}

void MystScriptParser_Myst::opcode_216(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Cards 4571 (Channelwood Tree), 4586 (Channelwood Tree), 
	// 4615 (Channelwood Tree) and 4601 (Channelwood Tree)
	if (argc == 0) {
		// TODO: Fill in logic for Channelwood Tree Position i.e. Var 72 update // 0 to 12, 4 for Alcove
		// Based on Timer code and following variables :
		// 98  "Cabin Boiler Pilot Light Lit"
		// 99  "Cabin Boiler Gas Valve Position" }, // 0 to 5
		// 305 "Cabin Boiler Lit" },
		// 306 "Cabin Boiler Steam Sound Control" }, // 0 to 27
		// 307 "Cabin Boiler Needle Position i.e. Fully Pressurised" }, // 0 to 1

		// Note : Opcode 218 does boiler update code..
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_217(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4601 (Channelwood Tree)
	if (argc == 2) {
		// TODO: Fill in logic for Tree Position Close Up...
		// 2 arguments: 4, 4
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_218(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

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
}

void MystScriptParser_Myst::opcode_219(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

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
}

void MystScriptParser_Myst::opcode_220(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4530 (Rocketship Music Puzzle Video)
	// TODO: Fill in logic.
	if (false) {
		// loop?
		_vm->_video->playMovie(_vm->wrapMovieFilename("selenbok", kMystStack), 224, 41);
	}
}

void MystScriptParser_Myst::opcode_221(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

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
}

void MystScriptParser_Myst::opcode_222(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4141 (Myst Dock Facing Sea)
	if (argc == 0) {
		// TODO: Logic for Gull Videos?
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_300(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Card 4371 (Blue Book) Var = 101
	//     and Card 4363 (Red Book)  Var = 100
	debugC(kDebugScript, "Opcode %d: Book Exit Function...", op);
	debugC(kDebugScript, "Var: %d", var);
	// TODO: Fill in Logic
}

void MystScriptParser_Myst::opcode_301(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Cards 4047, 4059, 4060, 4068 and 4080 (Myst Library Books - Open)
	// TODO: Fill in Logic. Clear Variable on Book exit.. or Copy from duplicate..
	_vm->_varStore->setVar(0, 1);
}

void MystScriptParser_Myst::opcode_302(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Card 4113 (Clock Tower Cog Puzzle)
	// TODO: Fill in Logic
}

void MystScriptParser_Myst::opcode_303(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4134 (Dock Facing Marker Switch)
	// Used for Card 4141 (Myst Dock Facing Sea)
	// In the original engine, this opcode stopped Gull Movies if playing,
	// upon card change, but this behavior is now default in this engine.
}

void MystScriptParser_Myst::opcode_304(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4601 (Channelwood Tree)
	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown...", op);
		// TODO: Logic for clearing variable?
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_305(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4601 (Channelwood Tree)
	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown...", op);
		// TODO: Logic for clearing variable?
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_306(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4098 (Cabin Boiler Puzzle)
	// In the original engine, this opcode stopped the Boiler Fire and Meter Needle videos
	// if playing, upon card change, but this behavior is now default in this engine.
}

void MystScriptParser_Myst::opcode_307(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4297 (Generator Room Controls)
	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown...", op);
		// TODO: Logic for clearing variable?
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Myst::opcode_308(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4530 (Rocketship Music Sliders)
	// In the original engine, this opcode stopped the Selenitic Book Movie if playing,
	// upon card change, but this behavior is now default in this engine.
}

void MystScriptParser_Myst::opcode_309(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4168 (Green D'ni Book Open)
	// In the original engine, this opcode stopped the Green Book Atrus Movies if playing,
	// upon card change, but this behavior is now default in this engine.
}

void MystScriptParser_Myst::opcode_312(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 4698 (Dock Forechamber Imager)
	// In the original engine, this opcode stopped the Imager Movie if playing,
	// especially the hardcoded Topological Extrusion (Mountain) video,
	// upon card change, but this behavior is now default in this engine.
}

} // End of namespace Mohawk
