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
#include "mohawk/myst_saveload.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/myst.h"

#include "common/events.h"

#include "gui/message.h"

namespace Mohawk {

// NOTE: Credits Start Card is 10000

MystScriptParser_Myst::MystScriptParser_Myst(MohawkEngine_Myst *vm) : MystScriptParser(vm) {
	setupOpcodes();

	// Card ID preinitialized by the engine for use by opcode 18
	// when linking back to Myst in the library
	_savedCardId = 4329;

	_libraryBookcaseChanged = false;
	_dockVaultState = 0;
	_cabinMatchState = 2;
	_matchBurning = false;
	_tree = 0;
	_treeAlcove = 0;
	_treeStopped = false;
	_treeMinPosition = 0;
	_treeLastMoveTime = _vm->_system->getMillis();
}

MystScriptParser_Myst::~MystScriptParser_Myst() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &MystScriptParser_Myst::x, #x))

void MystScriptParser_Myst::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, NOP);
	OPCODE(101, o_libraryBookPageTurnLeft);
	OPCODE(102, o_libraryBookPageTurnRight);
	OPCODE(103, o_fireplaceToggleButton);
	OPCODE(104, o_fireplaceRotation);
	OPCODE(105, opcode_105);
	OPCODE(106, o_towerRotationStart);
	OPCODE(107, NOP);
	OPCODE(108, o_towerRotationEnd);
	OPCODE(109, opcode_109);
	OPCODE(113, o_dockVaultOpen);
	OPCODE(114, o_dockVaultClose);
	OPCODE(115, o_bookGivePage);
	OPCODE(116, o_clockWheelsExecute);
	OPCODE(117, opcode_117);
	OPCODE(118, opcode_118);
	OPCODE(119, opcode_119);
	OPCODE(120, o_generatorButtonPressed);
	OPCODE(121, o_cabinSafeChangeDigit);
	OPCODE(122, o_cabinSafeHandleStartMove);
	OPCODE(123, o_cabinSafeHandleMove);
	OPCODE(124, o_cabinSafeHandleEndMove);
	OPCODE(128, o_treePressureReleaseStart);
	OPCODE(129, opcode_129);
	OPCODE(130, opcode_130);
	OPCODE(131, opcode_131);
	OPCODE(132, opcode_132);
	OPCODE(133, opcode_133);
	OPCODE(134, opcode_134);
	OPCODE(135, opcode_135);
	OPCODE(136, opcode_136);
	OPCODE(137, opcode_137);
	OPCODE(141, o_circuitBreakerStartMove);
	OPCODE(142, o_circuitBreakerMove);
	OPCODE(143, o_circuitBreakerEndMove);
	OPCODE(146, o_boilerIncreasePressureStart);
	OPCODE(147, o_boilerLightPilot);
	OPCODE(148, NOP);
	OPCODE(149, o_boilerIncreasePressureStop);
	OPCODE(150, o_boilerDecreasePressureStart);
	OPCODE(151, o_boilerDecreasePressureStop);
	OPCODE(152, NOP);
	OPCODE(153, o_basementIncreasePressureStart);
	OPCODE(154, o_basementIncreasePressureStop);
	OPCODE(155, o_basementDecreasePressureStart);
	OPCODE(156, o_basementDecreasePressureStop);
	OPCODE(157, o_rocketPianoMove);
	OPCODE(158, o_rocketSoundSliderStartMove);
	OPCODE(159, o_rocketSoundSliderMove);
	OPCODE(160, o_rocketSoundSliderEndMove);
	OPCODE(161, o_rocketPianoStart);
	OPCODE(162, o_rocketPianoStop);
	OPCODE(163, o_rocketLeverStartMove);
	OPCODE(164, o_rocketOpenBook);
	OPCODE(165, o_rocketLeverMove);
	OPCODE(166, o_rocketLeverEndMove);
	OPCODE(167, NOP);
	OPCODE(168, o_treePressureReleaseStop);
	OPCODE(169, o_cabinLeave);
	OPCODE(170, opcode_170);
	OPCODE(171, opcode_171);
	OPCODE(172, opcode_172);
	OPCODE(173, opcode_173);
	OPCODE(174, opcode_174);
	OPCODE(175, opcode_175);
	OPCODE(176, opcode_176);
	OPCODE(177, opcode_177);
	OPCODE(180, o_libraryCombinationBookStop);
	OPCODE(181, NOP);
	OPCODE(182, o_cabinMatchLight);
	OPCODE(183, opcode_183);
	OPCODE(184, opcode_184);
	OPCODE(185, NOP);
	OPCODE(186, o_clockMinuteWheelStartTurn);
	OPCODE(187, NOP);
	OPCODE(188, o_clockWheelEndTurn);
	OPCODE(189, o_clockHourWheelStartTurn);
	OPCODE(190, o_libraryCombinationBookStartRight);
	OPCODE(191, o_libraryCombinationBookStartLeft);
	OPCODE(192, opcode_192);
	OPCODE(194, opcode_194);
	OPCODE(195, opcode_195);
	OPCODE(196, opcode_196);
	OPCODE(197, opcode_197);
	OPCODE(198, o_dockVaultForceClose);
	OPCODE(199, opcode_199);

	// "Init" Opcodes
	OPCODE(200, o_libraryBook_init);
	OPCODE(201, opcode_201);
	OPCODE(202, o_towerRotationMap_init);
	OPCODE(203, o_forechamberDoor_init);
	OPCODE(204, opcode_204);
	OPCODE(205, opcode_205);
	OPCODE(206, opcode_206);
	OPCODE(208, opcode_208);
	OPCODE(209, o_libraryBookcaseTransform_init);
	OPCODE(210, o_generatorControlRoom_init);
	OPCODE(211, o_fireplace_init);
	OPCODE(212, opcode_212);
	OPCODE(213, opcode_213);
	OPCODE(214, opcode_214);
	OPCODE(215, opcode_215);
	OPCODE(216, o_treeCard_init);
	OPCODE(217, o_treeEntry_init);
	OPCODE(218, opcode_218);
	OPCODE(219, o_rocketSliders_init);
	OPCODE(220, o_rocketLinkVideo_init);
	OPCODE(221, o_greenBook_init);
	OPCODE(222, opcode_222);

	// "Exit" Opcodes
	OPCODE(300, opcode_300);
	OPCODE(301, opcode_301);
	OPCODE(302, opcode_302);
	OPCODE(303, opcode_303);
	OPCODE(304, o_treeCard_exit);
	OPCODE(305, o_treeEntry_exit);
	OPCODE(306, opcode_306);
	OPCODE(307, opcode_307);
	OPCODE(308, opcode_308);
	OPCODE(309, opcode_309);
	OPCODE(312, opcode_312);
}

#undef OPCODE

void MystScriptParser_Myst::disablePersistentScripts() {
	opcode_201_disable();
	opcode_205_disable();

	_libraryBookcaseMoving = false;
	_generatorControlRoomRunning = false;
	_libraryCombinationBookPagesTurning = false;
	_clockTurningWheel = 0;
	_towerRotationMapRunning = false;
	_boilerPressureIncreasing = false;
	_boilerPressureDecreasing = false;
	_basementPressureIncreasing = false;
	_basementPressureDecreasing = false;

	opcode_212_disable();
}

void MystScriptParser_Myst::runPersistentScripts() {
	opcode_201_run();
	opcode_205_run();
	opcode_212_run();

	if (_towerRotationMapRunning)
		towerRotationMap_run();

	if (_generatorControlRoomRunning)
		generatorControlRoom_run();

	if (_libraryCombinationBookPagesTurning)
		libraryCombinationBook_run();

	if (_libraryBookcaseMoving)
		libraryBookcaseTransform_run();

	if (_clockTurningWheel)
		clockWheel_run();

	if (_matchBurning)
		matchBurn_run();

	if (_boilerPressureIncreasing)
		boilerPressureIncrease_run();

	if (_boilerPressureDecreasing)
		boilerPressureDecrease_run();

	if (_basementPressureIncreasing)
		basementPressureIncrease_run();

	if (_basementPressureDecreasing)
		basementPressureDecrease_run();

	if (!_treeStopped)
		tree_run();
}

uint16 MystScriptParser_Myst::getVar(uint16 var) {
	MystVariables::Globals &globals = _vm->_saveLoad->_v->globals;
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	switch(var) {
	case 0: // Myst Library Bookcase Closed
		return myst.libraryBookcaseDoor;
	case 1:
		if (globals.ending != 4) {
			return myst.libraryBookcaseDoor != 1;
		} else if (myst.libraryBookcaseDoor == 1) {
			return 2;
		} else {
			return 3;
		}
	case 2: // Marker Switch Near Cabin
		return myst.cabinMarkerSwitch;
	case 3: // Marker Switch Near Clock Tower
		return myst.clockTowerMarkerSwitch;
	case 4: // Marker Switch on Dock
		return myst.dockMarkerSwitch;
	case 5: // Marker Switch Near Ship Pool
		return myst.poolMarkerSwitch;
	case 6: // Marker Switch Near Cogs
		return myst.gearsMarkerSwitch;
	case 7: // Marker Switch Near Generator Room
		return myst.generatorMarkerSwitch;
	case 8: // Marker Switch Near Stellar Observatory
		return myst.observatoryMarkerSwitch;
	case 9: // Marker Switch Near Rocket Ship
		return myst.rocketshipMarkerSwitch;
	case 11: // Cabin Door Open State
		return _cabinDoorOpened;
	case 12: // Clock tower gears bridge
		return myst.clockTowerBridgeOpen;
	case 13: // Tower in right position
		return myst.towerRotationAngle == 271
				|| myst.towerRotationAngle == 83
				|| myst.towerRotationAngle == 129
				|| myst.towerRotationAngle == 152;
	case 14: // Tower Solution (Key) Plaque
		switch (myst.towerRotationAngle) {
		case 271:
			return 1;
		case 83:
			return 2;
		case 129:
			return 3;
		case 152:
			return 4;
		default:
			return 0;
		}
	case 15: // Tower Window (Book) View
		switch (myst.towerRotationAngle) {
		case 271:
			return 1;
		case 83:
			if (myst.gearsOpen)
				return 6;
			else
				return 2;
		case 129:
			if (myst.shipState)
				return 5;
			else
				return 3;
		case 152:
			return 4;
		default:
			return 0;
		}
	case 16: // Tower Window (Book) View From Ladder Top
		if (myst.towerRotationAngle != 271
						&& myst.towerRotationAngle != 83
						&& myst.towerRotationAngle != 129) {
			if (myst.towerRotationAngle == 152)
				return 2;
			else
				return 0;
		} else {
			return 1;
		}

	case 23: // Fireplace Pattern Correct
		return _fireplaceLines[0] == 195
				&& _fireplaceLines[1] == 107
				&& _fireplaceLines[2] == 163
				&& _fireplaceLines[3] == 147
				&& _fireplaceLines[4] == 204
				&& _fireplaceLines[5] == 250;
	case 24: // Fireplace Blue Page Present
		if (globals.ending != 4) {
			return !(globals.bluePagesInBook & 32) && (globals.heldPage != 6);
		} else {
			return 0;
		}
	case 25: // Fireplace Red Page Present
		if (globals.ending != 4) {
			return !(globals.redPagesInBook & 32) && (globals.heldPage != 12);
		} else {
			return 0;
		}
	case 26: // Courtyard Image Box - Cross
	case 27: // Courtyard Image Box - Leaf
	case 28: // Courtyard Image Box - Arrow
	case 29: // Courtyard Image Box - Eye
	case 30: // Courtyard Image Box - Snake
	case 31: // Courtyard Image Box - Spider
	case 32: // Courtyard Image Box - Anchor
	case 33: // Courtyard Image Box - Ostrich
		if (!_tempVar) {
			return 0;
		} else if (myst.courtyardImageBoxes & (0x01 << (var - 26))) {
			return 2;
		} else {
			return 1;
		}
	case 37: // Clock Tower Control Wheels Position
		return 3 * ((myst.clockTowerMinutePosition / 5) % 3) + myst.clockTowerHourPosition % 3;
	case 41: // Dock Marker Switch Vault State
		return _dockVaultState;
	case 43: // Clock Tower Time
		return myst.clockTowerHourPosition * 12 + myst.clockTowerMinutePosition / 5;
	case 44: // Rocket ship power state
		if (myst.generatorBreakers || myst.generatorVoltage == 0)
			return 0;
		else if (myst.generatorVoltage != 59)
			return 1;
		else
			return 2;
	case 46:
		return bookCountPages(100);
	case 47:
		return bookCountPages(101);
	case 49: // Generator running
		return myst.generatorVoltage > 0;
	case 52: // Generator Switch #1
		return (myst.generatorButtons & 1) != 0;
	case 53: // Generator Switch #2
		return (myst.generatorButtons & 2) != 0;
	case 54: // Generator Switch #3
		return (myst.generatorButtons & 4) != 0;
	case 55: // Generator Switch #4
		return (myst.generatorButtons & 8) != 0;
	case 56: // Generator Switch #5
		return (myst.generatorButtons & 16) != 0;
	case 57: // Generator Switch #6
		return (myst.generatorButtons & 32) != 0;
	case 58: // Generator Switch #7
		return (myst.generatorButtons & 64) != 0;
	case 59: // Generator Switch #8
		return (myst.generatorButtons & 128) != 0;
	case 60: // Generator Switch #9
		return (myst.generatorButtons & 256) != 0;
	case 61: // Generator Switch #10
		return (myst.generatorButtons & 512) != 0;
	case 62: // Generator Power Dial Left LED Digit
		return _generatorVoltage / 10;
	case 63: // Generator Power Dial Right LED Digit
		return _generatorVoltage % 10;
	case 64: // Generator Power To Spaceship Dial Left LED Digit
		if (myst.generatorVoltage > 59 || myst.generatorBreakers)
			return 0;
		else
			return myst.generatorVoltage / 10;
	case 65: // Generator Power To Spaceship Dial Right LED Digit
		if (myst.generatorVoltage > 59 || myst.generatorBreakers)
			return 0;
		else
			return myst.generatorVoltage % 10;
	case 66: // Generators lights on
		return 0;
	case 67: // Cabin Safe Lock Number #1 - Left
		return myst.cabinSafeCombination / 100;
	case 68: // Cabin Safe Lock Number #2
		return (myst.cabinSafeCombination / 10) % 10;
	case 69: // Cabin Safe Lock Number #3 - Right
		return myst.cabinSafeCombination % 10;
	case 70: // Cabin Safe Matchbox State
		return _cabinMatchState;
	case 72: // Channelwood tree position
		return myst.treePosition;
	case 93: // Breaker nearest Generator Room Blown
		return myst.generatorBreakers == 1;
	case 94: // Breaker nearest Rocket Ship Blown
		return myst.generatorBreakers == 2;
	case 95: // Going out of tree destination selection
		if (myst.treePosition == 0)
			return 0;
		else if (myst.treePosition == 4 || myst.treePosition == 5)
			return 1;
		else
			return 2;
	case 96: // Generator Power Dial Needle Position
		return myst.generatorVoltage / 4;
	case 97: // Generator Power To Spaceship Dial Needle Position
		if (myst.generatorVoltage > 59 || myst.generatorBreakers)
			return 0;
		else
			return myst.generatorVoltage / 4;
	case 98: // Cabin Boiler Pilot Light Lit
		return myst.cabinPilotLightLit;
	case 99: // Cabin Boiler Gas Valve Position
		return myst.cabinValvePosition % 6;
	case 102: // Red page
		if (globals.ending != 4) {
			return !(globals.redPagesInBook & 1) && (globals.heldPage != 7);
		} else {
			return 0;
		}
	case 103: // Blue page
		if (globals.ending != 4) {
			return !(globals.bluePagesInBook & 1) && (globals.heldPage != 1);
		} else {
			return 0;
		}
	case 300: // Rocket Ship Music Puzzle Slider State
		return 1;
	case 302: // Green Book Opened Before Flag
		return myst.greenBookOpenedBefore;
	case 304: // Tower Rotation Map Initialized
		return _towerRotationMapInitialized;
	case 305: // Cabin Boiler Lit
		return myst.cabinPilotLightLit == 1 && myst.cabinValvePosition > 0;
	case 306: // Cabin Boiler Steam Sound Control
		if (myst.cabinPilotLightLit == 1) {
			if (myst.cabinValvePosition <= 0)
				return 26;
			else
				return 27;
		} else {
			return myst.cabinValvePosition;
		}
	case 307: // Cabin Boiler Fully Pressurised
		return myst.cabinPilotLightLit == 1 && myst.cabinValvePosition > 12;
	default:
		return MystScriptParser::getVar(var);
	}
}

void MystScriptParser_Myst::toggleVar(uint16 var) {
	MystVariables::Globals &globals = _vm->_saveLoad->_v->globals;
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	switch(var) {
	case 2: // Marker Switch Near Cabin
		myst.cabinMarkerSwitch = (myst.cabinMarkerSwitch + 1) % 2;
		break;
	case 3: // Marker Switch Near Clock Tower
		myst.clockTowerMarkerSwitch = (myst.clockTowerMarkerSwitch + 1) % 2;
		break;
	case 4: // Marker Switch on Dock
		myst.dockMarkerSwitch = (myst.dockMarkerSwitch + 1) % 2;
		break;
	case 5: // Marker Switch Near Ship Pool
		myst.poolMarkerSwitch = (myst.poolMarkerSwitch + 1) % 2;
		break;
	case 6: // Marker Switch Near Cogs
		myst.gearsMarkerSwitch = (myst.gearsMarkerSwitch + 1) % 2;
		break;
	case 7: // Marker Switch Near Generator Room
		myst.generatorMarkerSwitch = (myst.generatorMarkerSwitch + 1) % 2;
		break;
	case 8: // Marker Switch Near Stellar Observatory
		myst.observatoryMarkerSwitch = (myst.observatoryMarkerSwitch + 1) % 2;
		break;
	case 9: // Marker Switch Near Rocket Ship
		myst.rocketshipMarkerSwitch = (myst.rocketshipMarkerSwitch + 1) % 2;
		break;
	case 24: // Fireplace Blue Page
		if (globals.ending != 4 && !(globals.bluePagesInBook & 32)) {
			if (globals.heldPage == 6)
				globals.heldPage = 0;
			else {
				globals.heldPage = 6;
			}
		}
		break;
	case 25: // Fireplace Red page
		if (globals.ending != 4 && !(globals.redPagesInBook & 32)) {
			if (globals.heldPage == 12)
				globals.heldPage = 0;
			else {
				globals.heldPage = 12;
			}
		}
		break;
	case 41: // Vault white page
		if (globals.ending != 4) {
			if (_dockVaultState == 1) {
				_dockVaultState = 2;
				globals.heldPage = 0;
			} else if (_dockVaultState == 2) {
				_dockVaultState = 1;
				globals.heldPage = 13;
			}
		}
		break;
	case 102: // Red page
		if (globals.ending != 4 && !(globals.redPagesInBook & 1)) {
			if (globals.heldPage == 7)
				globals.heldPage = 0;
			else {
				globals.heldPage = 7;
			}
		}
		break;
	case 103: // Blue page
		if (globals.ending != 4 && !(globals.bluePagesInBook & 1)) {
			if (globals.heldPage == 1)
				globals.heldPage = 0;
			else {
				globals.heldPage = 1;
			}
		}
		break;
	default:
		MystScriptParser::toggleVar(var);
		break;
	}
}

bool MystScriptParser_Myst::setVarValue(uint16 var, uint16 value) {
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;
	bool refresh = false;

	switch (var) {
	case 0: // Myst Library Bookcase Closed
		if (myst.libraryBookcaseDoor != value) {
			myst.libraryBookcaseDoor = value;
			_tempVar = 0;
			refresh = true;
		}
		break;
	case 11: // Cabin Door Open State
		if (_cabinDoorOpened != value) {
			_cabinDoorOpened = value;
			refresh = true;
		}
		break;
	case 70: // Cabin Safe Matchbox State
		if (_cabinMatchState != value) {
			_cabinMatchState = value;
			refresh = true;
		}
		break;
	case 302: // Green Book Opened Before Flag
		myst.greenBookOpenedBefore = value;
		break;
	case 303: // Library Bookcase status changed
		_libraryBookcaseChanged = value;
		break;
	case 304: // Myst Library Image Present on Tower Rotation Map
		_towerRotationMapInitialized = value;
		break;
	default:
		refresh = MystScriptParser::setVarValue(var, value);
		break;
	}

	return refresh;
}

uint16 MystScriptParser_Myst::bookCountPages(uint16 var) {
	MystVariables::Globals &globals = _vm->_saveLoad->_v->globals;

	uint16 pages = 0;
	uint16 cnt = 0;

	// Select book according to var
	if (var == 100)
		pages = globals.redPagesInBook;
	else if (var == 101)
		pages = globals.bluePagesInBook;

	// Special page present
	if (pages & 64)
		return 6;

	// Count pages
	if (pages & 1)
		cnt++;

	if (pages & 2)
		cnt++;

	if (pages & 4)
		cnt++;

	if (pages & 8)
		cnt++;

	if (pages & 16)
		cnt++;

	return cnt;
}

void MystScriptParser_Myst::o_libraryBookPageTurnLeft(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Turn book page left", op);

	if (_libraryBookPage - 1 >= 0) {
		_libraryBookPage--;

		Common::Rect rect = Common::Rect(0, 0, 544, 333);
		_vm->_gfx->copyImageToScreen(_libraryBookBaseImage + _libraryBookPage, rect);

		if (_vm->_rnd->getRandomBit())
			_vm->_sound->playSound(_libraryBookSound1);
		else
			_vm->_sound->playSound(_libraryBookSound2);

		_vm->_system->updateScreen();
	}
}

void MystScriptParser_Myst::o_libraryBookPageTurnRight(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Turn book page right", op);

	if (_libraryBookPage + 1 < _libraryBookNumPages) {
		_libraryBookPage++;

		Common::Rect rect = Common::Rect(0, 0, 544, 333);
		_vm->_gfx->copyImageToScreen(_libraryBookBaseImage + _libraryBookPage, rect);

		if (_vm->_rnd->getRandomBit())
			_vm->_sound->playSound(_libraryBookSound1);
		else
			_vm->_sound->playSound(_libraryBookSound2);

		_vm->_system->updateScreen();
	}
}

void MystScriptParser_Myst::o_fireplaceToggleButton(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Myst Card 4162 (Fireplace Grid)
	debugC(kDebugScript, "Opcode %d: Fireplace grid toggle button", op);

	uint16 bitmask = argv[0];
	uint16 line = _fireplaceLines[var - 17];

	debugC(kDebugScript, "\tvar: %d", var);
	debugC(kDebugScript, "\tbitmask: 0x%02X", bitmask);

	if (line & bitmask) {
		// Unset button
		for (uint i = 4795; i >= 4779; i--) {
			_vm->_gfx->copyImageToScreen(i, _invokingResource->getRect());
			_vm->_system->updateScreen();
			_vm->_system->delayMillis(1);
		}
		_fireplaceLines[var - 17] &= ~bitmask;
	} else {
		// Set button
		for (uint i = 4779; i <= 4795; i++) {
			_vm->_gfx->copyImageToScreen(i, _invokingResource->getRect());
			_vm->_system->updateScreen();
			_vm->_system->delayMillis(1);
		}
		_fireplaceLines[var - 17] |= bitmask;
	}
}

void MystScriptParser_Myst::o_fireplaceRotation(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Myst Card 4162 and 4166 (Fireplace Puzzle Rotation Movies)
	uint16 movieNum = argv[0];
	debugC(kDebugScript, "Opcode %d: Play Fireplace Puzzle Rotation Movies", op);
	debugC(kDebugScript, "\tmovieNum: %d", movieNum);

	if (movieNum)
		_vm->_video->playMovie(_vm->wrapMovieFilename("fpout", kMystStack), 167, 4);
	else
		_vm->_video->playMovie(_vm->wrapMovieFilename("fpin", kMystStack), 167, 4);
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

void MystScriptParser_Myst::o_towerRotationStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	_towerRotationBlinkLabel = false;
	_towerRotationMapClicked = true;
	_towerRotationSpeed = 0;

	_vm->_cursor->setCursor(700);

	const Common::Point center = Common::Point(383, 124);
	Common::Point end = towerRotationMapComputeCoords(center, myst.towerRotationAngle);
	towerRotationMapComputeAngle();
	towerRotationMapDrawLine(center, end);

	_vm->_sound->replaceSound(5378, Audio::Mixer::kMaxChannelVolume, true);
}

void MystScriptParser_Myst::o_towerRotationEnd(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	_towerRotationMapClicked = false;

	// Set angle value to expected value
	if (myst.towerRotationAngle >= 265
			&& myst.towerRotationAngle <= 277
			&& myst.rocketshipMarkerSwitch) {
		myst.towerRotationAngle = 271;
	} else if (myst.towerRotationAngle >= 77
			&& myst.towerRotationAngle <= 89
			&& myst.gearsMarkerSwitch) {
		myst.towerRotationAngle = 83;
	} else if (myst.towerRotationAngle >= 123
			&& myst.towerRotationAngle <= 135
			&& myst.dockMarkerSwitch) {
		myst.towerRotationAngle = 129;
	} else if (myst.towerRotationAngle >= 146
			&& myst.towerRotationAngle <= 158
			&& myst.cabinMarkerSwitch) {
		myst.towerRotationAngle = 152;
	}

	_vm->_sound->replaceSound(6378);

	_towerRotationBlinkLabel = true;
	_towerRotationBlinkLabelCount = 0;
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

void MystScriptParser_Myst::o_dockVaultOpen(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Myst 4143 (Dock near Marker Switch)
	MystVariables::Globals &globals = _vm->_saveLoad->_v->globals;
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;
	uint16 soundId = argv[0];
	uint16 delay = argv[1];
	uint16 directionalUpdateDataSize = argv[2];

	debugC(kDebugScript, "Opcode %d: Vault Open Logic", op);
	debugC(kDebugScript, "\tsoundId: %d", soundId);
	debugC(kDebugScript, "\tdirectionalUpdateDataSize: %d", directionalUpdateDataSize);

	if ((myst.cabinMarkerSwitch == 1) &&
		(myst.clockTowerMarkerSwitch == 1) &&
		(myst.dockMarkerSwitch == 0) &&
		(myst.gearsMarkerSwitch == 1) &&
		(myst.generatorMarkerSwitch == 1) &&
		(myst.observatoryMarkerSwitch == 1) &&
		(myst.poolMarkerSwitch == 1) &&
		(myst.rocketshipMarkerSwitch == 1)) {
		if (globals.heldPage != 13 && globals.ending != 4)
			_dockVaultState = 2;
		else
			_dockVaultState = 1;

		_vm->_sound->playSound(soundId);
		_vm->redrawArea(41, false);
		animatedUpdate(directionalUpdateDataSize, &argv[3], delay);
	}
}

void MystScriptParser_Myst::o_dockVaultClose(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Myst 4143 (Dock near Marker Switch)
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;
	uint16 soundId = argv[0];
	uint16 delay = argv[1];
	uint16 directionalUpdateDataSize = argv[2];

	debugC(kDebugScript, "Opcode %d: Vault Close Logic", op);
	debugC(kDebugScript, "\tsoundId: %d", soundId);
	debugC(kDebugScript, "\tdirectionalUpdateDataSize: %d", directionalUpdateDataSize);

	if ((myst.cabinMarkerSwitch == 1) &&
		(myst.clockTowerMarkerSwitch == 1) &&
		(myst.dockMarkerSwitch == 1) &&
		(myst.gearsMarkerSwitch == 1) &&
		(myst.generatorMarkerSwitch == 1) &&
		(myst.observatoryMarkerSwitch == 1) &&
		(myst.poolMarkerSwitch == 1) &&
		(myst.rocketshipMarkerSwitch == 1)) {
		if (_dockVaultState == 1 || _dockVaultState == 2)
			_dockVaultState = 0;

		_vm->_sound->playSound(soundId);
		_vm->redrawArea(41, false);
		animatedUpdate(directionalUpdateDataSize, &argv[3], delay);
	}
}

void MystScriptParser_Myst::o_bookGivePage(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	MystVariables::Globals &globals = _vm->_saveLoad->_v->globals;

	uint16 cardIdLose = argv[0];
	uint16 cardIdBookCover = argv[1];
	uint16 soundIdAddPage = argv[2];

	debugC(kDebugScript, "Opcode %d: Red and Blue Book/Page Interaction", op);
	debugC(kDebugScript, "Var: %d", var);
	debugC(kDebugScript, "Card Id (Lose): %d", cardIdLose);
	debugC(kDebugScript, "Card Id (Book Cover): %d", cardIdBookCover);
	debugC(kDebugScript, "SoundId (Add Page): %d", soundIdAddPage);

	// No page or white page
	if (!globals.heldPage || globals.heldPage == 13) {
		_vm->changeToCard(cardIdBookCover, true);
		return;
	}

	uint16 bookVar = 101;
	uint16 mask = 0;

	switch (globals.heldPage) {
	case 7:
		bookVar = 100;
	case 1:
		mask = 1;
		break;
	case 8:
		bookVar = 100;
	case 2:
		mask = 2;
		break;
	case 9:
		bookVar = 100;
	case 3:
		mask = 4;
		break;
	case 10:
		bookVar = 100;
	case 4:
		mask = 8;
		break;
	case 11:
		bookVar = 100;
	case 5:
		mask = 16;
		break;
	case 12:
		bookVar = 100;
	case 6:
		mask = 32;
		break;
	}

	// Wrong book
	if (bookVar != var) {
		_vm->changeToCard(cardIdBookCover, true);
		return;
	}

	_vm->_cursor->hideCursor();
	_vm->_sound->playSoundBlocking(soundIdAddPage);
	_vm->setMainCursor(kDefaultMystCursor);

	// Add page to book
	if (var == 100)
		globals.redPagesInBook |= mask;
	else
		globals.bluePagesInBook |= mask;

	// Remove page from hand
	globals.heldPage = 0;

	_vm->_cursor->showCursor();

	if (mask == 32) {
		// You lose!
		if (var == 100)
			globals.currentAge = 9;
		else
			globals.currentAge = 10;

		_vm->changeToCard(cardIdLose, true);
	} else {
		_vm->changeToCard(cardIdBookCover, true);
	}
}

void MystScriptParser_Myst::o_clockWheelsExecute(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4006 (Clock Tower Time Controls)
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;
	uint16 soundId = argv[0];

	debugC(kDebugScript, "Opcode %d: Clock Tower Bridge Puzzle Execute Button", op);

	// Correct time is 2:40
	bool correctTime = myst.clockTowerHourPosition == 2
						&& myst.clockTowerMinutePosition == 40;

	if (!myst.clockTowerBridgeOpen && correctTime) {
		_vm->_sound->playSound(soundId);
		_vm->_system->delayMillis(500);

		// TODO: Play only 1st half of movie i.e. gears rise up, from 0 to 650
		_vm->_video->playMovie(_vm->wrapMovieFilename("gears", kMystStack), 305, 33);

		myst.clockTowerBridgeOpen = 1;
		_vm->redrawArea(12);
	} else if (myst.clockTowerBridgeOpen && !correctTime) {
		_vm->_sound->playSound(soundId);
		_vm->_system->delayMillis(500);

		// TODO: Play only 2nd half of movie i.e. gears sink down, from 700 to 1300
		_vm->_video->playMovie(_vm->wrapMovieFilename("gears", kMystStack), 305, 33);

		myst.clockTowerBridgeOpen = 0;
		_vm->redrawArea(12);
	}
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

void MystScriptParser_Myst::o_generatorButtonPressed(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Generator button pressed", op);

	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;
	MystResource *button = _invokingResource->_parent;

	generatorRedrawRocket();

	_generatorVoltage = myst.generatorVoltage;

	uint16 mask = 0;
	uint16 value = 0;
	generatorButtonValue(button, mask, value);

	// Button pressed
	if (myst.generatorButtons & mask) {
		myst.generatorButtons &= ~mask;
		myst.generatorVoltage -= value;

		if (myst.generatorVoltage)
			_vm->_sound->playSound(8297);
		else
			_vm->_sound->playSound(9297);
	} else {
		if (_generatorVoltage)
			_vm->_sound->playSound(6297);
		else
			_vm->_sound->playSound(7297); //TODO: Replace with play sound and replace background 4297

		myst.generatorButtons |= mask;
		myst.generatorVoltage += value;
	}

	// Redraw button
	_vm->redrawArea(button->getType8Var());

	// Blow breaker
	if (myst.generatorVoltage > 59)
		myst.generatorBreakers = _vm->_rnd->getRandomNumberRng(1, 2);
}

void MystScriptParser_Myst::generatorRedrawRocket() {
	_vm->redrawArea(64);
	_vm->redrawArea(65);
	_vm->redrawArea(97);
}

void MystScriptParser_Myst::generatorButtonValue(MystResource *button, uint16 &mask, uint16 &value) {
	switch (button->getType8Var()) {
	case 52: // Generator Switch #1
		mask = 1;
		value = 10;
		break;
	case 53: // Generator Switch #2
		mask = 2;
		value = 7;
		break;
	case 54: // Generator Switch #3
		mask = 4;
		value = 8;
		break;
	case 55: // Generator Switch #4
		mask = 8;
		value = 16;
		break;
	case 56: // Generator Switch #5
		mask = 16;
		value = 5;
		break;
	case 57: // Generator Switch #6
		mask = 32;
		value = 1;
		break;
	case 58: // Generator Switch #7
		mask = 64;
		value = 2;
		break;
	case 59: // Generator Switch #8
		mask = 128;
		value = 22;
		break;
	case 60: // Generator Switch #9
		mask = 256;
		value = 19;
		break;
	case 61: // Generator Switch #10
		mask = 512;
		value = 9;
		break;
	}
}

void MystScriptParser_Myst::o_cabinSafeChangeDigit(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Cabin safe change digit", op);

	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	uint16 d1 = myst.cabinSafeCombination / 100;
	uint16 d2 = (myst.cabinSafeCombination / 10) % 10;
	uint16 d3 = myst.cabinSafeCombination % 10;

	if (var == 67)
		d1 = (d1 + 1) % 10;
	else if (var == 68)
		d2 = (d2 + 1) % 10;
	else
		d3 = (d3 + 1) % 10;

	myst.cabinSafeCombination = 100 * d1 + 10 * d2 + d3;

	_vm->redrawArea(var);
}

void MystScriptParser_Myst::o_cabinSafeHandleStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Cabin safe handle start move", op);

	// Used on Card 4100
	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	handle->drawFrame(0);
	_vm->_cursor->setCursor(700);
	_tempVar = 0;
}

void MystScriptParser_Myst::o_cabinSafeHandleMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Cabin safe handle move", op);

	// Used on Card 4100
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;
	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);

	// Make the handle follow the mouse
	int16 maxStep = handle->getStepsV() - 1;
    Common::Rect rect = handle->getRect();
    int16 step = ((_vm->_mouse.y - rect.top) * handle->getStepsV()) / rect.height();
	step = CLIP<uint16>(step, 0, maxStep);

	handle->drawFrame(step);

	if (step == maxStep) {
		// Sound not played yet
		if (_tempVar == 0) {
			uint16 soundId = handle->getList2(0);
			if (soundId)
				_vm->_sound->playSound(soundId);
		}
		// Combination is right
		if (myst.cabinSafeCombination == 724) {
			uint16 soundId = handle->getList2(1);
			if (soundId)
				_vm->_sound->playSound(soundId);

			_vm->changeToCard(4103, false);

			Common::Rect screenRect = Common::Rect(544, 333);
			_vm->_gfx->runTransition(0, screenRect, 2, 5);
		}
		_tempVar = 1;
	} else {
		_tempVar = 0;
	}
}

void MystScriptParser_Myst::o_cabinSafeHandleEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Cabin safe handle end move", op);

	// Used on Card 4100
	MystResourceType12 *handle = static_cast<MystResourceType12 *>(_invokingResource);
	handle->drawFrame(0);
	_vm->checkCursorHints();
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

void MystScriptParser_Myst::o_circuitBreakerStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Circuit breaker start move", op);

	MystResourceType12 *breaker = static_cast<MystResourceType12 *>(_invokingResource);
	breaker->drawFrame(0);
	_vm->_cursor->setCursor(700);
	_tempVar = 0;
}

void MystScriptParser_Myst::o_circuitBreakerMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Circuit breaker move", op);

	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;
	MystResourceType12 *breaker = static_cast<MystResourceType12 *>(_invokingResource);

	int16 maxStep = breaker->getStepsV() - 1;
	int16 step = ((_vm->_mouse.y - 80) * breaker->getStepsV()) / 65;
	step = CLIP<uint16>(step, 0, maxStep);

	breaker->drawFrame(step);

	if (_tempVar != step) {
		_tempVar = step;

		// Breaker switched
		if (step == maxStep) {

			// Choose breaker
			if (breaker->getType8Var() == 93) {

				// Voltage is still too high or not broken
				if (myst.generatorVoltage > 59 || myst.generatorBreakers != 1) {
					uint16 soundId = breaker->getList2(1);
					if (soundId)
						_vm->_sound->playSound(soundId);
				} else {
					uint16 soundId = breaker->getList2(0);
					if (soundId)
						_vm->_sound->playSound(soundId);

					// Reset breaker state
					myst.generatorBreakers = 0;
				}
			} else {
				// Voltage is still too high or not broken
				if (myst.generatorVoltage > 59 || myst.generatorBreakers != 2) {
					uint16 soundId = breaker->getList2(1);
					if (soundId)
						_vm->_sound->playSound(soundId);
				} else {
					uint16 soundId = breaker->getList2(0);
					if (soundId)
						_vm->_sound->playSound(soundId);

					// Reset breaker state
					myst.generatorBreakers = 0;
				}
			}
		}
	}
}

void MystScriptParser_Myst::o_circuitBreakerEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Circuit breaker end move", op);

	MystResourceType12 *breaker = static_cast<MystResourceType12 *>(_invokingResource);
	_vm->redrawArea(breaker->getType8Var());
	_vm->checkCursorHints();
}

void MystScriptParser_Myst::o_boilerIncreasePressureStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Boiler increase pressure start", op);
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	_treeStopped = true;
	if (myst.cabinValvePosition < 25)
		_vm->_sound->stopBackground();

	_boilerPressureIncreasing = true;
}

void MystScriptParser_Myst::o_boilerLightPilot(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Boiler light pilot", op);
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	// Match is lit
	if (_cabinMatchState == 1) {
		myst.cabinPilotLightLit = 1;
		_vm->redrawArea(98);

		// Put out match
		_matchGoOutTime = _vm->_system->getMillis();

		if (myst.cabinValvePosition > 0)
			_vm->_sound->replaceBackground(8098, 49152);

		if (myst.cabinValvePosition > 12)
			_treeLastMoveTime = _vm->_system->getMillis();

		// TODO: Complete. Play movies
	}
}

void MystScriptParser_Myst::o_boilerIncreasePressureStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Boiler increase pressure stop", op);
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	_treeStopped = false;
	_boilerPressureIncreasing = false;
	_treeLastMoveTime = _vm->_system->getMillis();

	if (myst.cabinPilotLightLit == 1) {
		if (myst.cabinValvePosition > 0)
			_vm->_sound->replaceBackground(8098, 49152);

		// TODO: Play movies
	} else {
		if (myst.cabinValvePosition > 0)
			_vm->_sound->replaceBackground(4098, myst.cabinValvePosition << 10);
	}
}

void MystScriptParser_Myst::boilerPressureIncrease_run() {
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	// Allow increasing pressure if sound has stopped
	if (!_vm->_sound->isPlaying(5098) && myst.cabinValvePosition < 25) {
		myst.cabinValvePosition++;
		if (myst.cabinValvePosition == 1) {
			// TODO: Play fire movie

			// Draw fire
			_vm->redrawArea(305);
		} else if (myst.cabinValvePosition == 25) {
			if (myst.cabinPilotLightLit == 1)
				_vm->_sound->replaceBackground(8098, 49152);
			else
				_vm->_sound->replaceBackground(4098, 25600);
		}

		// Pressure increasing sound
		_vm->_sound->playSound(5098);

		// Redraw wheel
		_vm->redrawArea(99);
	}
}

void MystScriptParser_Myst::boilerPressureDecrease_run() {
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	// Allow decreasing pressure if sound has stopped
	if (!_vm->_sound->isPlaying(5098) && myst.cabinValvePosition > 0) {
		myst.cabinValvePosition--;
		if (myst.cabinValvePosition == 0) {
			// TODO: Play fire movie

			// Draw fire
			_vm->redrawArea(305);
		}

		// Pressure increasing sound
		_vm->_sound->playSound(5098);

		// Redraw wheel
		_vm->redrawArea(99);
	}
}

void MystScriptParser_Myst::o_boilerDecreasePressureStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Boiler decrease pressure start", op);

	_treeStopped = true;
	_vm->_sound->stopBackground();

	_boilerPressureDecreasing = true;
}

void MystScriptParser_Myst::o_boilerDecreasePressureStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Boiler decrease pressure stop", op);
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	_treeStopped = false;
	_boilerPressureDecreasing = false;
	_treeLastMoveTime = _vm->_system->getMillis();

	if (myst.cabinPilotLightLit == 1) {
		if (myst.cabinValvePosition > 0)
			_vm->_sound->replaceBackground(8098, 49152);

		// TODO: Play movies
	} else {
		if (myst.cabinValvePosition > 0)
			_vm->_sound->replaceBackground(4098, myst.cabinValvePosition << 10);
	}
}

void MystScriptParser_Myst::o_basementIncreasePressureStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Basement increase pressure start", op);

	_treeStopped = true;
	_basementPressureIncreasing = true;
}

void MystScriptParser_Myst::o_basementIncreasePressureStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Basement increase pressure stop", op);

	_treeStopped = false;
	_basementPressureIncreasing = false;
	_treeLastMoveTime = _vm->_system->getMillis();
}

void MystScriptParser_Myst::basementPressureIncrease_run() {
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	// Allow increasing pressure if sound has stopped
	if (!_vm->_sound->isPlaying(4642) && myst.cabinValvePosition < 25) {
		myst.cabinValvePosition++;

		// Pressure increasing sound
		_vm->_sound->playSound(4642);

		// Redraw wheel
		_vm->redrawArea(99);
	}
}

void MystScriptParser_Myst::basementPressureDecrease_run() {
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	// Allow decreasing pressure if sound has stopped
	if (!_vm->_sound->isPlaying(4642) && myst.cabinValvePosition > 0) {
		myst.cabinValvePosition--;

		// Pressure decreasing sound
		_vm->_sound->playSound(4642);

		// Redraw wheel
		_vm->redrawArea(99);
	}
}

void MystScriptParser_Myst::o_basementDecreasePressureStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Basement decrease pressure start", op);

	_treeStopped = true;
	_basementPressureDecreasing = true;
}

void MystScriptParser_Myst::o_basementDecreasePressureStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Basement decrease pressure stop", op);

	_treeStopped = false;
	_basementPressureDecreasing = false;
	_treeLastMoveTime = _vm->_system->getMillis();
}

void MystScriptParser_Myst::tree_run() {
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	uint16 pressure;
	if (myst.cabinPilotLightLit)
		pressure = myst.cabinValvePosition;
	else
		pressure = 0;

	// 12 means tree is balanced
	if (pressure != 12) {
		bool goingDown = true;
		if (pressure >= 12)
			goingDown = false;

		// Tree is within bounds
		if ((myst.treePosition < 12 && !goingDown)
				|| (myst.treePosition > _treeMinPosition && goingDown)) {
			uint16 delay = treeNextMoveDelay(pressure);
			uint32 time = _vm->_system->getMillis();
			if (delay < time - _treeLastMoveTime) {

				// Tree movement
				if (goingDown) {
					myst.treePosition--;
					_vm->_sound->playSound(2);
				} else {
					myst.treePosition++;
					_vm->_sound->playSound(1);
				}

				// Stop background music if going up from book room
				if (_vm->getCurCard() == 4630 && myst.treePosition > 0) {
					_vm->_sound->stopBackground();
				} else {
					_vm->_sound->replaceBackground(4630, 24576);
				}

				// Redraw tree
				_vm->redrawArea(72);

				// Check if alcove is accessible
				treeSetAlcoveAccessible();

				_treeLastMoveTime = time;
			}
		}
	}
}

void MystScriptParser_Myst::treeSetAlcoveAccessible() {
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	if (_treeAlcove) {
		// Make alcove accessible if the tree is in the correct position
		_treeAlcove->setEnabled(myst.treePosition >= _treeMinAccessiblePosition
					&& myst.treePosition <= _treeMaxAccessiblePosition);
	}
}

uint32 MystScriptParser_Myst::treeNextMoveDelay(uint16 pressure) {
	if (pressure >= 12)
		return 25000 * (13 - (pressure - 12)) / 12 + 3000;
	else
		return 25000 * pressure / 13 + 3000;
}

void MystScriptParser_Myst::o_rocketSoundSliderStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket slider start move", op);

	_rocketSliderSound = 0;
	_vm->_cursor->setCursor(700);
	_vm->_sound->pauseBackground();
	rocketSliderMove();
}

void MystScriptParser_Myst::o_rocketSoundSliderMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket slider move", op);

	rocketSliderMove();
}

void MystScriptParser_Myst::o_rocketSoundSliderEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket slider end move", op);

	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	_vm->checkCursorHints();

	if (myst.generatorVoltage == 59 && !myst.generatorBreakers) {
		if (_rocketSliderSound)
			_vm->_sound->stopSound();
	}

	if (_invokingResource == _rocketSlider1) {
		myst.rocketSliderPosition[0] = _rocketSlider1->_pos.y;
	} else if (_invokingResource == _rocketSlider2) {
		myst.rocketSliderPosition[1] = _rocketSlider2->_pos.y;
	} else if (_invokingResource == _rocketSlider3) {
		myst.rocketSliderPosition[2] = _rocketSlider3->_pos.y;
	} else if (_invokingResource == _rocketSlider4) {
		myst.rocketSliderPosition[3] = _rocketSlider4->_pos.y;
	} else if (_invokingResource == _rocketSlider5) {
		myst.rocketSliderPosition[4] = _rocketSlider5->_pos.y;
	}

	_vm->_sound->resumeBackground();
}

void MystScriptParser_Myst::rocketSliderMove() {
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	MystResourceType10 *slider = static_cast<MystResourceType10 *>(_invokingResource);

	if (myst.generatorVoltage == 59 && !myst.generatorBreakers) {
		uint16 soundId = rocketSliderGetSound(slider->_pos.y);
		if (soundId != _rocketSliderSound) {
			_rocketSliderSound = soundId;
			_vm->_sound->replaceSound(soundId, Audio::Mixer::kMaxChannelVolume, true);
		}
	}
}

uint16 MystScriptParser_Myst::rocketSliderGetSound(uint16 pos) {
	return (uint16)(9530 + (pos - 216) * 35.0 * 0.01639344262295082);
}

void MystScriptParser_Myst::rocketCheckSolution() {
	_vm->_cursor->hideCursor();

	uint16 soundId;
	bool solved = true;

	soundId = rocketSliderGetSound(_rocketSlider1->_pos.y);
	_vm->_sound->replaceSound(soundId);
	_rocketSlider1->drawConditionalDataToScreen(2);
	_vm->_system->delayMillis(250);
	if (soundId != 9558)
		solved = false;

	soundId = rocketSliderGetSound(_rocketSlider2->_pos.y);
	_vm->_sound->replaceSound(soundId);
	_rocketSlider2->drawConditionalDataToScreen(2);
	_vm->_system->delayMillis(250);
	if (soundId != 9546)
		solved = false;

	soundId = rocketSliderGetSound(_rocketSlider3->_pos.y);
	_vm->_sound->replaceSound(soundId);
	_rocketSlider3->drawConditionalDataToScreen(2);
	_vm->_system->delayMillis(250);
	if (soundId != 9543)
		solved = false;

	soundId = rocketSliderGetSound(_rocketSlider4->_pos.y);
	_vm->_sound->replaceSound(soundId);
	_rocketSlider4->drawConditionalDataToScreen(2);
	_vm->_system->delayMillis(250);
	if (soundId != 9553)
		solved = false;

	soundId = rocketSliderGetSound(_rocketSlider5->_pos.y);
	_vm->_sound->replaceSound(soundId);
	_rocketSlider5->drawConditionalDataToScreen(2);
	_vm->_system->delayMillis(250);
	if (soundId != 9560)
		solved = false;

	_vm->_sound->stopSound();

	if (solved) {
		_vm->_video->playBackgroundMovie(_vm->wrapMovieFilename("selenbok", kMystStack), 224, 41, true);

		// TODO: Movie control
		// Play from 0 to 660
		// Then from 660 to 3500, looping

		_tempVar = 1;
	}

	_rocketSlider1->drawConditionalDataToScreen(1);
	_rocketSlider2->drawConditionalDataToScreen(1);
	_rocketSlider3->drawConditionalDataToScreen(1);
	_rocketSlider4->drawConditionalDataToScreen(1);
	_rocketSlider5->drawConditionalDataToScreen(1);

	_vm->_cursor->showCursor();
}

void MystScriptParser_Myst::o_rocketPianoStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket piano start move", op);

	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;
	MystResourceType11 *key = static_cast<MystResourceType11 *>(_invokingResource);

	// What the hell ??
	Common::Rect src = key->_subImages[1].rect;
	Common::Rect rect = key->_subImages[0].rect;
	Common::Rect dest = rect;
	dest.top = 332 - rect.bottom;
	dest.bottom = 332 - rect.top;

	// Draw pressed piano key
	_vm->_gfx->copyImageSectionToScreen(key->_subImages[1].wdib, src, dest);
	_vm->_system->updateScreen();

	// Play note
	if (myst.generatorVoltage == 59 && !myst.generatorBreakers) {
		uint16 soundId = key->getList1(0);
		_vm->_sound->replaceSound(soundId, Audio::Mixer::kMaxChannelVolume, true);
	}
}

void MystScriptParser_Myst::o_rocketPianoMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket piano move", op);

	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();
	Common::Rect piano = Common::Rect(85, 123, 460, 270);
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	// Unpress previous key
	MystResourceType11 *key = static_cast<MystResourceType11 *>(_invokingResource);

	Common::Rect src = key->_subImages[0].rect;
	Common::Rect dest = src;
	dest.top = 332 - src.bottom;
	dest.bottom = 332 - src.top;

	// Draw unpressed piano key
	_vm->_gfx->copyImageSectionToScreen(key->_subImages[0].wdib, src, dest);

	if (piano.contains(mouse)) {
		MystResource *resource = _vm->updateCurrentResource();
		if (resource && resource->type == kMystDragArea) {
			// Press new key
			key = static_cast<MystResourceType11 *>(resource);
			src = key->_subImages[1].rect;
			Common::Rect rect = key->_subImages[0].rect;
			dest = rect;
			dest.top = 332 - rect.bottom;
			dest.bottom = 332 - rect.top;

			// Draw pressed piano key
			_vm->_gfx->copyImageSectionToScreen(key->_subImages[1].wdib, src, dest);

			// Play note
			if (myst.generatorVoltage == 59 && !myst.generatorBreakers) {
				uint16 soundId = key->getList1(0);
				_vm->_sound->replaceSound(soundId, Audio::Mixer::kMaxChannelVolume, true);
			}
		} else {
			// Not pressing a key anymore
			_vm->_sound->stopSound();
			_vm->_sound->resumeBackground();
		}
	}

	_vm->_system->updateScreen();
}

void MystScriptParser_Myst::o_rocketPianoStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket piano end move", op);

	MystResourceType8 *key = static_cast<MystResourceType8 *>(_invokingResource);

	Common::Rect &src = key->_subImages[0].rect;
	Common::Rect dest = src;
	dest.top = 332 - src.bottom;
	dest.bottom = 332 - src.top;

	// Draw unpressed piano key
	_vm->_gfx->copyImageSectionToScreen(key->_subImages[0].wdib, src, dest);
	_vm->_system->updateScreen();

	_vm->_sound->stopSound();
	_vm->_sound->resumeBackground();
}

void MystScriptParser_Myst::o_rocketLeverStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket lever start move", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	_vm->_cursor->setCursor(700);
	_rocketLeverPosition = 0;
	lever->drawFrame(0);
}

void MystScriptParser_Myst::o_rocketOpenBook(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket open link book", op);

	// TODO: Update video playing
	// Play from 3500 to 13100, looping

	// Set linkable
	_tempVar = 2;
}

void MystScriptParser_Myst::o_rocketLeverMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket lever move", op);

	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;
	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	// Make the lever follow the mouse
	int16 maxStep = lever->getStepsV() - 1;
    Common::Rect rect = lever->getRect();
    int16 step = ((_vm->_mouse.y - rect.top) * lever->getStepsV()) / rect.height();
	step = CLIP<uint16>(step, 0, maxStep);

	lever->drawFrame(step);

	// If lever pulled
	if (step == maxStep && step != _rocketLeverPosition) {
		uint16 soundId = lever->getList2(0);
		if (soundId)
			_vm->_sound->playSound(soundId);

		// If rocket correctly powered
		if (myst.generatorVoltage == 59 && !myst.generatorBreakers) {
			rocketCheckSolution();
		}
	}

	_rocketLeverPosition = step;
}

void MystScriptParser_Myst::o_rocketLeverEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket lever end move", op);

	MystResourceType12 *lever = static_cast<MystResourceType12 *>(_invokingResource);

	_vm->checkCursorHints();
	_rocketLeverPosition = 0;
	lever->drawFrame(0);
}

void MystScriptParser_Myst::o_cabinLeave(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Leave cabin", op);

	// If match is lit, put out
	if (_cabinMatchState == 1) {
		_matchGoOutTime = _vm->_system->getMillis();
	} else if (_cabinMatchState == 0) {
		_vm->setMainCursor(_savedCursorId);
		_cabinMatchState = 2;
	}
}

void MystScriptParser_Myst::o_treePressureReleaseStart(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Tree pressure release start", op);

	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	Common::Rect src = Common::Rect(0, 0, 49, 86);
	Common::Rect dest = Common::Rect(78, 46, 127, 132);
	_vm->_gfx->copyImageSectionToScreen(4631, src, dest);
	_vm->_system->updateScreen();

	_tempVar = myst.cabinValvePosition;

	if (myst.treePosition >= 4) {
		myst.cabinValvePosition = 0;
		_treeMinPosition = 4;
		_treeLastMoveTime = 0;
	}
}

void MystScriptParser_Myst::o_treePressureReleaseStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Tree pressure release stop", op);

	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	Common::Rect rect = Common::Rect(78, 46, 127, 132);
	_vm->_gfx->copyBackBufferToScreen(rect);
	_vm->_system->updateScreen();

	myst.cabinValvePosition = _tempVar;
	_treeMinPosition = 0;
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

void MystScriptParser_Myst::o_libraryCombinationBookStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Combiation book stop turning pages", op);
	_libraryCombinationBookPagesTurning = false;
}

void MystScriptParser_Myst::o_cabinMatchLight(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (!_cabinMatchState) {
		_vm->_sound->playSound(4103);

		// Match is lit
		_cabinMatchState = 1;
		_matchBurning = true;
		_matchGoOutCnt = 0;
		_vm->setMainCursor(kLitMatchCursor);

		// Match will stay lit for one minute
		_matchGoOutTime = _vm->_system->getMillis() + 60 * 1000;
	}
}

void MystScriptParser_Myst::matchBurn_run() {
	uint32 time = _vm->_system->getMillis();

	if (time > _matchGoOutTime) {
		_matchGoOutTime = time + 150;

		// Switch between lit match and dead match every 150 ms when match is dying
		if (_matchGoOutCnt % 2)
			_vm->setMainCursor(kLitMatchCursor);
		else
			_vm->setMainCursor(kDeadMatchCursor);

		_matchGoOutCnt++;

		// Match is dead
		if (_matchGoOutCnt >= 5) {
			_matchBurning = false;
			_vm->setMainCursor(_savedCursorId);

			_cabinMatchState = 2;
		}
	}
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

void MystScriptParser_Myst::o_clockMinuteWheelStartTurn(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4006
	debugC(kDebugScript, "Opcode %d: Minute wheel start turn", op);

	clockWheelStartTurn(2);
}

void MystScriptParser_Myst::o_clockWheelEndTurn(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4006
	debugC(kDebugScript, "Opcode %d: Wheel end turn", op);

	_clockTurningWheel = 0;
}

void MystScriptParser_Myst::o_clockHourWheelStartTurn(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Card 4006
	debugC(kDebugScript, "Opcode %d: Hour wheel start turn", op);

	clockWheelStartTurn(1);
}

void MystScriptParser_Myst::clockWheel_run() {
	// Turn wheel one step each second
	uint32 time = _vm->_system->getMillis();
	if (time > _startTime + 1000) {
		_startTime = time;

		if (_clockTurningWheel == 1) {
			clockWheelTurn(39);
		} else {
			clockWheelTurn(38);
		}
		_vm->redrawArea(37);
	}

}

void MystScriptParser_Myst::clockWheelStartTurn(uint16 wheel) {
	MystResourceType11 *resource = static_cast<MystResourceType11 *>(_invokingResource);
	uint16 soundId = resource->getList1(0);
	if (soundId)
		_vm->_sound->playSound(soundId);

	// Turn wheel one step
	if (wheel == 1) {
		clockWheelTurn(39);
	} else {
		clockWheelTurn(38);
	}
	_vm->redrawArea(37);

	// Continue turning wheel until mouse button is released
	_clockTurningWheel = wheel;
	_startTime = _vm->_system->getMillis();
}

void MystScriptParser_Myst::clockWheelTurn(uint16 var) {
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	if (var == 38) {
		// Hours
		myst.clockTowerHourPosition = (myst.clockTowerHourPosition + 1) % 12;
	} else {
		// Minutes
		myst.clockTowerMinutePosition = (myst.clockTowerMinutePosition + 5) % 60;
	}
}

void MystScriptParser_Myst::o_libraryCombinationBookStartRight(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Combination book start turning pages right", op);

	_tempVar = 0;
	libraryCombinationBookTurnRight();
	_startTime = _vm->_system->getMillis();
	_libraryCombinationBookPagesTurning = true;
}

void MystScriptParser_Myst::o_libraryCombinationBookStartLeft(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Combination book start turning pages left", op);

	_tempVar = 0;
	libraryCombinationBookTurnLeft();
	_startTime = _vm->_system->getMillis();
	_libraryCombinationBookPagesTurning = true;
}

void MystScriptParser_Myst::libraryCombinationBookTurnLeft() {
	// Turn page left
	if (_libraryBookPage - 1 >=  0) {
		_tempVar--;

		if (_tempVar >= -5) {
			_libraryBookPage--;
		} else {
			_libraryBookPage -= 5;
			_tempVar = -5;
		}

		_libraryBookPage = CLIP<int16>(_libraryBookPage, 0, _libraryBookNumPages - 1);

		Common::Rect rect = Common::Rect(157, 115, 544, 333);
		_vm->_gfx->copyImageToScreen(_libraryBookBaseImage + _libraryBookPage, rect);

		if (_vm->_rnd->getRandomBit())
			_vm->_sound->playSound(_libraryBookSound1);
		else
			_vm->_sound->playSound(_libraryBookSound2);

		_vm->_system->updateScreen();
	}
}

void MystScriptParser_Myst::libraryCombinationBookTurnRight() {
	// Turn page right
	if (_libraryBookPage + 1 < _libraryBookNumPages) {
		_tempVar++;

		if (_tempVar <= 5) {
			_libraryBookPage++;
		} else {
			_libraryBookPage += 5;
			_tempVar = 5;
		}

		_libraryBookPage = CLIP<uint16>(_libraryBookPage, 0, _libraryBookNumPages - 1);

		Common::Rect rect = Common::Rect(157, 115, 544, 333);
		_vm->_gfx->copyImageToScreen(_libraryBookBaseImage + _libraryBookPage, rect);

		if (_vm->_rnd->getRandomBit())
			_vm->_sound->playSound(_libraryBookSound1);
		else
			_vm->_sound->playSound(_libraryBookSound2);

		_vm->_system->updateScreen();
	}
}

void MystScriptParser_Myst::libraryCombinationBook_run() {
	uint32 time = _vm->_system->getMillis();
	if (time >= _startTime + 500) {
		if (_tempVar > 0) {
			libraryCombinationBookTurnRight();
			_startTime = time;
		} else if (_tempVar < 0) {
			libraryCombinationBookTurnLeft();
			_startTime = time;
		}
	}
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

void MystScriptParser_Myst::o_dockVaultForceClose(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Myst 4143 (Dock near Marker Switch)
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;
	uint16 soundId = argv[0];
	uint16 delay = argv[1];
	uint16 directionalUpdateDataSize = argv[2];

	debugC(kDebugScript, "Opcode %d: Vault Force Close", op);
	debugC(kDebugScript, "\tsoundId: %d", soundId);
	debugC(kDebugScript, "\tdirectionalUpdateDataSize: %d", directionalUpdateDataSize);

	if (_dockVaultState) {
		// Open switch
		myst.dockMarkerSwitch = 1;
		_vm->_sound->playSound(4143);
		_vm->redrawArea(4);

		// Close vault
		_dockVaultState = 0;
		_vm->_sound->playSound(soundId);
		_vm->redrawArea(41, false);
		animatedUpdate(directionalUpdateDataSize, &argv[3], delay);
	}
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

void MystScriptParser_Myst::o_libraryBook_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_libraryBookPage = 0;
	_libraryBookNumPages = argv[0];
	_libraryBookBaseImage = argv[1];
	_libraryBookSound1 = argv[2];
	_libraryBookSound2 = argv[3];
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

void MystScriptParser_Myst::towerRotationMap_run() {
	if (!_towerRotationMapInitialized) {
		_towerRotationMapInitialized = true;
		_vm->_sound->playSound(4378);

		towerRotationDrawBuildings();

		// Draw to screen
		_vm->_gfx->copyBackBufferToScreen(Common::Rect(106, 42, 459, 273));
		_vm->_system->updateScreen();
	}

	uint32 time = _vm->_system->getMillis();
	if (time > _startTime) {
		if (_towerRotationMapClicked) {
			towerRotationMapRotate();
			_startTime = time + 100;
		} else if (_towerRotationBlinkLabel
				&& _vm->_sound->isPlaying(6378)) {
			// Blink tower rotation label while sound is playing
			_towerRotationBlinkLabelCount = (_towerRotationBlinkLabelCount + 1) % 14;

			if (_towerRotationBlinkLabelCount == 7)
				_towerRotationMapLabel->drawConditionalDataToScreen(0);
			else if (_towerRotationBlinkLabelCount == 0)
				_towerRotationMapLabel->drawConditionalDataToScreen(1);

			_startTime = time + 100;
		} else {
			// Stop blinking label
			_towerRotationBlinkLabel = false;
			_towerRotationMapLabel->drawConditionalDataToScreen(0);

			// Blink tower
			_startTime = time + 500;
			_tempVar = (_tempVar + 1) % 2;
			_towerRotationMapTower->drawConditionalDataToScreen(_tempVar);
		}
	}
}

void MystScriptParser_Myst::o_towerRotationMap_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_towerRotationMapRunning = true;
	_towerRotationMapTower = static_cast<MystResourceType11 *>(_invokingResource);
	_towerRotationMapLabel = static_cast<MystResourceType8 *>(_vm->_resources[argv[0]]);
	_tempVar = 0;
	_startTime = 0;
	_towerRotationMapClicked = false;
}

void MystScriptParser_Myst::towerRotationDrawBuildings() {
	// Draw library
	_vm->redrawArea(304, false);

	// Draw other resources
	for (uint i = 1; i <= 10; i++) {
		MystResourceType8 *resource = static_cast<MystResourceType8 *>(_vm->_resources[i]);
		_vm->redrawResource(resource, false);
	}
}

uint16 MystScriptParser_Myst::towerRotationMapComputeAngle() {
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	_towerRotationSpeed++;
	if (_towerRotationSpeed >= 7)
		_towerRotationSpeed = 7;
	else
		_towerRotationSpeed++;

	myst.towerRotationAngle = (myst.towerRotationAngle + _towerRotationSpeed) % 360;
	uint16 angle = myst.towerRotationAngle;
	_towerRotationOverSpot = false;

	if (angle >= 265 && angle <= 277
			&& myst.rocketshipMarkerSwitch) {
		angle = 271;
		_towerRotationOverSpot = true;
		_towerRotationSpeed = 1;
	} else if (angle >= 77 && angle <= 89
			&& myst.gearsMarkerSwitch) {
		angle = 83;
		_towerRotationOverSpot = true;
		_towerRotationSpeed = 1;
	} else if (angle >= 123 && angle <= 135
			&& myst.dockMarkerSwitch) {
		angle = 129;
		_towerRotationOverSpot = true;
		_towerRotationSpeed = 1;
	} else if (angle >= 146 && angle <= 158
			&& myst.cabinMarkerSwitch) {
		angle = 152;
		_towerRotationOverSpot = true;
		_towerRotationSpeed = 1;
	}

	return angle;
}

Common::Point MystScriptParser_Myst::towerRotationMapComputeCoords(const Common::Point &center, uint16 angle) {
	Common::Point end;

	// Polar to rect coords
	double radians = angle * PI / 180.0;
	end.x = (int16)(center.x + cos(radians) * 310.0);
	end.y = (int16)(center.y + sin(radians) * 310.0);

	return end;
}

void MystScriptParser_Myst::towerRotationMapDrawLine(const Common::Point &center, const Common::Point &end) {
	Graphics::PixelFormat pf = _vm->_system->getScreenFormat();
	uint32 color = 0;

	if (!_towerRotationOverSpot)
		color = pf.RGBToColor(0xFF, 0xFF, 0xFF); // White
	else
		color = pf.RGBToColor(0xFF, 0, 0); // Red

	const Common::Rect rect = Common::Rect(106, 42, 459, 273);

	Common::Rect src;
	src.left = rect.left;
	src.top = 333 - rect.bottom;
	src.right = rect.right;
	src.bottom = 333 - rect.top;

	// Redraw background
	_vm->_gfx->copyImageSectionToBackBuffer(_vm->getCardBackgroundId(), src, rect);

	// Draw buildings
	towerRotationDrawBuildings();

	// Draw tower
	_towerRotationMapTower->drawConditionalDataToScreen(0, false);

	// Draw label
	_towerRotationMapLabel->drawConditionalDataToScreen(1, false);

	// Draw line
	_vm->_gfx->drawLine(center, end, color);
	_vm->_gfx->copyBackBufferToScreen(rect);
	_vm->_system->updateScreen();
}

void MystScriptParser_Myst::towerRotationMapRotate() {
	const Common::Point center = Common::Point(383, 124);
	uint16 angle = towerRotationMapComputeAngle();
	Common::Point end = towerRotationMapComputeCoords(center, angle);
	towerRotationMapDrawLine(center, end);
}

void MystScriptParser_Myst::o_forechamberDoor_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Card 4138 (Dock Forechamber Door)
	// Set forechamber door to closed
	_tempVar = 0;
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

void MystScriptParser_Myst::libraryBookcaseTransform_run(void) {
	if (_libraryBookcaseChanged) {
		_libraryBookcaseChanged = false;
		_libraryBookcaseMoving = false;

		// Play transform sound and video
		_vm->_sound->playSound(_libraryBookcaseSoundId);
		_libraryBookcaseMovie->playMovie();
	}
}

void MystScriptParser_Myst::o_libraryBookcaseTransform_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (_libraryBookcaseChanged) {
		MystResourceType7 *resource = static_cast<MystResourceType7 *>(_invokingResource);
		_libraryBookcaseMovie = static_cast<MystResourceType6 *>(resource->getSubResource(getVar(0)));
		_libraryBookcaseSoundId = argv[0];
		_libraryBookcaseMoving = true;
	}
}

void MystScriptParser_Myst::generatorControlRoom_run(void) {
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	if (_generatorVoltage == myst.generatorVoltage) {
		generatorRedrawRocket();
	} else {
		// Animate generator gauge		
		if (_generatorVoltage > myst.generatorVoltage)
			_generatorVoltage--;
		else
			_generatorVoltage++;

		// Redraw generator gauge
		_vm->redrawArea(62);
		_vm->redrawArea(63);
		_vm->redrawArea(96);
	}
}

void MystScriptParser_Myst::o_generatorControlRoom_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	debugC(kDebugScript, "Opcode %d: Generator control room init", op);

	_generatorVoltage = myst.generatorVoltage;
	_generatorControlRoomRunning = true;
}

void MystScriptParser_Myst::o_fireplace_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Fireplace grid init", op);

	// Clear fireplace grid
	for (uint i = 0; i < 6; i++)
		_fireplaceLines[i] = 0;
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

void MystScriptParser_Myst::o_treeCard_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Enter tree card", op);

	_tree = static_cast<MystResourceType8 *>(_invokingResource);
}

void MystScriptParser_Myst::o_treeEntry_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Enter tree card with entry", op);

	_treeAlcove = static_cast<MystResourceType5 *>(_invokingResource);
	_treeMinAccessiblePosition = argv[0];
	_treeMaxAccessiblePosition = argv[1];

	treeSetAlcoveAccessible();
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

void MystScriptParser_Myst::o_rocketSliders_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket sliders init", op);

	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	_rocketSlider1 = static_cast<MystResourceType10 *>(_vm->_resources[argv[0]]);
	_rocketSlider2 = static_cast<MystResourceType10 *>(_vm->_resources[argv[1]]);
	_rocketSlider3 = static_cast<MystResourceType10 *>(_vm->_resources[argv[2]]);
	_rocketSlider4 = static_cast<MystResourceType10 *>(_vm->_resources[argv[3]]);
	_rocketSlider5 = static_cast<MystResourceType10 *>(_vm->_resources[argv[4]]);

	if (myst.rocketSliderPosition[0]) {
		_rocketSlider1->setPosition(myst.rocketSliderPosition[0]);
	}
	if (myst.rocketSliderPosition[1]) {
		_rocketSlider2->setPosition(myst.rocketSliderPosition[1]);
	}
	if (myst.rocketSliderPosition[2]) {
		_rocketSlider3->setPosition(myst.rocketSliderPosition[2]);
	}
	if (myst.rocketSliderPosition[3]) {
		_rocketSlider4->setPosition(myst.rocketSliderPosition[3]);
	}
	if (myst.rocketSliderPosition[4]) {
		_rocketSlider5->setPosition(myst.rocketSliderPosition[4]);
	}
}

void MystScriptParser_Myst::o_rocketLinkVideo_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Rocket link video init", op);
	_tempVar = 0;
}

void MystScriptParser_Myst::o_greenBook_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Card 4168 (Green Book Movies)
	debugC(kDebugScript, "Opcode %d: Green book init", op);

	MystVariables::Globals &globals = _vm->_saveLoad->_v->globals;
	MystVariables::Myst &myst = _vm->_saveLoad->_v->myst;

	uint loopStart = 0;
	uint loopEnd = 0;
	Common::String file;

	if (!myst.greenBookOpenedBefore) {
		loopStart = 113200;
		loopEnd = 116400;
		file = _vm->wrapMovieFilename("atrusbk1", kMystStack);
	} else {
		loopStart = 8800;
		loopEnd = 9700;
		file = _vm->wrapMovieFilename("atrusbk2", kMystStack);
	}

	_vm->_sound->stopSound();
	_vm->_sound->pauseBackground();

	if (globals.ending != 4) {
		_vm->_video->playBackgroundMovie(file, 314, 76);
	}

	// TODO: Movie play control
	// loop between loopStart and loopEnd
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

void MystScriptParser_Myst::o_treeCard_exit(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Exit tree card", op);

	_tree = 0;
}

void MystScriptParser_Myst::o_treeEntry_exit(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Exit tree card with entry", op);

	_treeAlcove = 0;
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
