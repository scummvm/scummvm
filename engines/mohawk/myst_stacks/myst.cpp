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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "mohawk/cursors.h"
#include "mohawk/myst.h"
#include "mohawk/myst_areas.h"
#include "mohawk/myst_card.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/myst_state.h"
#include "mohawk/myst_sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/myst.h"

#include "common/events.h"
#include "common/math.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace Mohawk {
namespace MystStacks {

Myst::Myst(MohawkEngine_Myst *vm, MystStack stackId) :
		MystScriptParser(vm, stackId),
		_state(_vm->_gameState->_myst),
		_towerRotationCenter(Common::Point(383, 124)) {
	setupOpcodes();

	// Card ID preinitialized by the engine for use by opcode 18
	// when linking back to Myst in the library
	_savedCardId = 4329;

	_towerRotationBlinkLabel = false;
	_towerRotationBlinkLabelCount = 0;
	_towerRotationSpeed = 0;
	_towerRotationMapInitialized = 0;
	_towerRotationMapRunning = false;
	_towerRotationMapClicked = false;
	_towerRotationMapTower = nullptr;
	_towerRotationMapLabel = nullptr;
	_towerRotationOverSpot = false;

	_libraryBookcaseChanged = false;
	_libraryBookcaseMoving = false;
	_libraryBookcaseMovie = nullptr;
	_libraryBookcaseSoundId = 0;

	_libraryBookPagesTurning = false;
	_libraryBookNumPages = 0;
	_libraryBookBaseImage = 0;
	_libraryBookSound1 = 0;
	_libraryBookSound2 = 0;

	_libraryCombinationBookPagesTurning = false;

	for (uint i = 0; i < ARRAYSIZE(_fireplaceLines); i++) {
		_fireplaceLines[i] = 0;
	}

	_dockVaultState = 0;

	_cabinDoorOpened = 0;
	_cabinHandleDown = 0;
	_cabinMatchState = 2;
	_cabinGaugeMovieEnabled = false;

	_boilerPressureIncreasing = false;
	_boilerPressureDecreasing = false;
	_basementPressureIncreasing = false;
	_basementPressureDecreasing = false;

	_matchBurning = false;
	_matchGoOutCnt = 0;
	_matchGoOutTime = 0;

	_tree = nullptr;
	_treeAlcove = nullptr;
	_treeStopped = false;
	_treeMinPosition = 0;
	_treeMinAccessiblePosition = 0;
	_treeMaxAccessiblePosition = 0;

	_imagerRunning = false;
	_imagerRedButton = nullptr;
	_imagerMovie = nullptr;
	_imagerValidationRunning = false;
	_imagerValidationCard = 0;
	_imagerValidationStep = 0;
	for (uint i = 0; i < ARRAYSIZE(_imagerSound); i++) {
		_imagerSound[i] = 0;
	}

	_butterfliesMoviePlayed = false;
	_state.treeLastMoveTime = _vm->getTotalPlayTime();

	_rocketPianoSound = 0;
	_rocketSlider1 = nullptr;
	_rocketSlider2 = nullptr;
	_rocketSlider3 = nullptr;
	_rocketSlider4 = nullptr;
	_rocketSlider5 = nullptr;
	_rocketSliderSound = 0;
	_rocketLeverPosition = 0;

	_generatorControlRoomRunning = false;
	_generatorVoltage = _state.generatorVoltage;

	_observatoryRunning = false;
	_observatoryMonthChanging = false;
	_observatoryDayChanging = false;
	_observatoryYearChanging = false;
	_observatoryTimeChanging = false;
	_observatoryVisualizer = nullptr;
	_observatoryGoButton = nullptr;
	_observatoryCurrentSlider = nullptr;
	_observatoryDaySlider = nullptr;
	_observatoryMonthSlider = nullptr;
	_observatoryYearSlider = nullptr;
	_observatoryTimeSlider = nullptr;
	_observatoryLastTime = 0;
	_observatoryNotInitialized = true;
	_observatoryIncrement = 0;

	_greenBookRunning = false;

	_gullsFlying1 = false;
	_gullsFlying2 = false;
	_gullsFlying3 = false;
	_gullsNextTime = 0;

	_courtyardBoxSound = 0;

	_clockTurningWheel = 0;
	_clockWeightPosition = 0;
	_clockMiddleGearMovedAlone = false;
	_clockLeverPulled = false;
	for (uint i = 0; i < ARRAYSIZE(_clockGearsPositions); i++) {
		_clockGearsPositions[i] = 0;
	}
}

Myst::~Myst() {
}

void Myst::setupOpcodes() {
	// "Stack-Specific" Opcodes
	REGISTER_OPCODE(100, Myst, NOP);
	REGISTER_OPCODE(101, Myst, o_libraryBookPageTurnLeft);
	REGISTER_OPCODE(102, Myst, o_libraryBookPageTurnRight);
	REGISTER_OPCODE(103, Myst, o_fireplaceToggleButton);
	REGISTER_OPCODE(104, Myst, o_fireplaceRotation);
	REGISTER_OPCODE(105, Myst, o_courtyardBoxesCheckSolution);
	REGISTER_OPCODE(106, Myst, o_towerRotationStart);
	REGISTER_OPCODE(107, Myst, NOP);
	REGISTER_OPCODE(108, Myst, o_towerRotationEnd);
	REGISTER_OPCODE(109, Myst, o_imagerChangeSelection);
	REGISTER_OPCODE(113, Myst, o_dockVaultOpen);
	REGISTER_OPCODE(114, Myst, o_dockVaultClose);
	REGISTER_OPCODE(115, Myst, o_bookGivePage);
	REGISTER_OPCODE(116, Myst, o_clockWheelsExecute);
	REGISTER_OPCODE(117, Myst, o_imagerPlayButton);
	REGISTER_OPCODE(118, Myst, o_imagerEraseButton);
	REGISTER_OPCODE(119, Myst, o_towerElevatorAnimation);
	REGISTER_OPCODE(120, Myst, o_generatorButtonPressed);
	REGISTER_OPCODE(121, Myst, o_cabinSafeChangeDigit);
	REGISTER_OPCODE(122, Myst, o_cabinSafeHandleStartMove);
	REGISTER_OPCODE(123, Myst, o_cabinSafeHandleMove);
	REGISTER_OPCODE(124, Myst, o_cabinSafeHandleEndMove);
	REGISTER_OPCODE(126, Myst, o_clockLeverStartMove);
	REGISTER_OPCODE(127, Myst, o_clockLeverEndMove);
	REGISTER_OPCODE(128, Myst, o_treePressureReleaseStart);
	if (!observatoryIsDDMMYYYY2400()) {
		REGISTER_OPCODE(129, Myst, o_observatoryMonthChangeStartIncrease);
		REGISTER_OPCODE(130, Myst, o_observatoryMonthChangeStartDecrease);
		REGISTER_OPCODE(131, Myst, o_observatoryDayChangeStartIncrease);
		REGISTER_OPCODE(132, Myst, o_observatoryDayChangeStartDecrease);
	} else {
		REGISTER_OPCODE(129, Myst, o_observatoryDayChangeStartIncrease);
		REGISTER_OPCODE(130, Myst, o_observatoryDayChangeStartDecrease);
		REGISTER_OPCODE(131, Myst, o_observatoryMonthChangeStartIncrease);
		REGISTER_OPCODE(132, Myst, o_observatoryMonthChangeStartDecrease);
	}
	REGISTER_OPCODE(133, Myst, o_observatoryGoButton);
	REGISTER_OPCODE(134, Myst, o_observatoryMonthSliderMove);
	REGISTER_OPCODE(135, Myst, o_observatoryDaySliderMove);
	REGISTER_OPCODE(136, Myst, o_observatoryYearSliderMove);
	REGISTER_OPCODE(137, Myst, o_observatoryTimeSliderMove);
	REGISTER_OPCODE(138, Myst, o_clockResetLeverStartMove);
	REGISTER_OPCODE(139, Myst, o_clockResetLeverMove);
	REGISTER_OPCODE(140, Myst, o_clockResetLeverEndMove);
	REGISTER_OPCODE(141, Myst, o_circuitBreakerStartMove);
	REGISTER_OPCODE(142, Myst, o_circuitBreakerMove);
	REGISTER_OPCODE(143, Myst, o_circuitBreakerEndMove);
	REGISTER_OPCODE(144, Myst, o_clockLeverMoveLeft);
	REGISTER_OPCODE(145, Myst, o_clockLeverMoveRight);
	REGISTER_OPCODE(146, Myst, o_boilerIncreasePressureStart);
	REGISTER_OPCODE(147, Myst, o_boilerLightPilot);
	REGISTER_OPCODE(148, Myst, NOP);
	REGISTER_OPCODE(149, Myst, o_boilerIncreasePressureStop);
	REGISTER_OPCODE(150, Myst, o_boilerDecreasePressureStart);
	REGISTER_OPCODE(151, Myst, o_boilerDecreasePressureStop);
	REGISTER_OPCODE(152, Myst, NOP);
	REGISTER_OPCODE(153, Myst, o_basementIncreasePressureStart);
	REGISTER_OPCODE(154, Myst, o_basementIncreasePressureStop);
	REGISTER_OPCODE(155, Myst, o_basementDecreasePressureStart);
	REGISTER_OPCODE(156, Myst, o_basementDecreasePressureStop);
	REGISTER_OPCODE(157, Myst, o_rocketPianoMove);
	REGISTER_OPCODE(158, Myst, o_rocketSoundSliderStartMove);
	REGISTER_OPCODE(159, Myst, o_rocketSoundSliderMove);
	REGISTER_OPCODE(160, Myst, o_rocketSoundSliderEndMove);
	REGISTER_OPCODE(161, Myst, o_rocketPianoStart);
	REGISTER_OPCODE(162, Myst, o_rocketPianoStop);
	REGISTER_OPCODE(163, Myst, o_rocketLeverStartMove);
	REGISTER_OPCODE(164, Myst, o_rocketOpenBook);
	REGISTER_OPCODE(165, Myst, o_rocketLeverMove);
	REGISTER_OPCODE(166, Myst, o_rocketLeverEndMove);
	REGISTER_OPCODE(167, Myst, NOP);
	REGISTER_OPCODE(168, Myst, o_treePressureReleaseStop);
	REGISTER_OPCODE(169, Myst, o_cabinLeave);
	REGISTER_OPCODE(170, Myst, o_observatoryMonthSliderStartMove);
	REGISTER_OPCODE(171, Myst, o_observatoryMonthSliderEndMove);
	REGISTER_OPCODE(172, Myst, o_observatoryDaySliderStartMove);
	REGISTER_OPCODE(173, Myst, o_observatoryDaySliderEndMove);
	REGISTER_OPCODE(174, Myst, o_observatoryYearSliderStartMove);
	REGISTER_OPCODE(175, Myst, o_observatoryYearSliderEndMove);
	REGISTER_OPCODE(176, Myst, o_observatoryTimeSliderStartMove);
	REGISTER_OPCODE(177, Myst, o_observatoryTimeSliderEndMove);
	REGISTER_OPCODE(178, Myst, o_libraryBookPageTurnStartLeft);
	REGISTER_OPCODE(179, Myst, o_libraryBookPageTurnStartRight);
	REGISTER_OPCODE(180, Myst, o_libraryCombinationBookStop);
	REGISTER_OPCODE(181, Myst, NOP);
	REGISTER_OPCODE(182, Myst, o_cabinMatchLight);
	REGISTER_OPCODE(183, Myst, o_courtyardBoxEnter);
	REGISTER_OPCODE(184, Myst, o_courtyardBoxLeave);
	REGISTER_OPCODE(185, Myst, NOP);
	REGISTER_OPCODE(186, Myst, o_clockMinuteWheelStartTurn);
	REGISTER_OPCODE(187, Myst, NOP);
	REGISTER_OPCODE(188, Myst, o_clockWheelEndTurn);
	REGISTER_OPCODE(189, Myst, o_clockHourWheelStartTurn);
	REGISTER_OPCODE(190, Myst, o_libraryCombinationBookStartRight);
	REGISTER_OPCODE(191, Myst, o_libraryCombinationBookStartLeft);
	REGISTER_OPCODE(192, Myst, o_observatoryTimeChangeStartIncrease);
	REGISTER_OPCODE(193, Myst, NOP);
	REGISTER_OPCODE(194, Myst, o_observatoryChangeSettingStop);
	REGISTER_OPCODE(195, Myst, o_observatoryTimeChangeStartDecrease);
	REGISTER_OPCODE(196, Myst, o_observatoryYearChangeStartIncrease);
	REGISTER_OPCODE(197, Myst, o_observatoryYearChangeStartDecrease);
	REGISTER_OPCODE(198, Myst, o_dockVaultForceClose);
	REGISTER_OPCODE(199, Myst, o_imagerEraseStop);

	// "Init" Opcodes
	REGISTER_OPCODE(200, Myst, o_libraryBook_init);
	REGISTER_OPCODE(201, Myst, o_courtyardBox_init);
	REGISTER_OPCODE(202, Myst, o_towerRotationMap_init);
	REGISTER_OPCODE(203, Myst, o_forechamberDoor_init);
	REGISTER_OPCODE(204, Myst, o_shipAccess_init);
	REGISTER_OPCODE(205, Myst, NOP);
	REGISTER_OPCODE(206, Myst, o_butterflies_init);
	REGISTER_OPCODE(208, Myst, o_imager_init);
	REGISTER_OPCODE(209, Myst, o_libraryBookcaseTransform_init);
	REGISTER_OPCODE(210, Myst, o_generatorControlRoom_init);
	REGISTER_OPCODE(211, Myst, o_fireplace_init);
	REGISTER_OPCODE(212, Myst, o_clockGears_init);
	REGISTER_OPCODE(213, Myst, o_gulls1_init);
	REGISTER_OPCODE(214, Myst, o_observatory_init);
	REGISTER_OPCODE(215, Myst, o_gulls2_init);
	REGISTER_OPCODE(216, Myst, o_treeCard_init);
	REGISTER_OPCODE(217, Myst, o_treeEntry_init);
	REGISTER_OPCODE(218, Myst, o_boilerMovies_init);
	REGISTER_OPCODE(219, Myst, o_rocketSliders_init);
	REGISTER_OPCODE(220, Myst, o_rocketLinkVideo_init);
	REGISTER_OPCODE(221, Myst, o_greenBook_init);
	REGISTER_OPCODE(222, Myst, o_gulls3_init);

	// "Exit" Opcodes
	REGISTER_OPCODE(300, Myst, o_bookAddSpecialPage_exit);
	REGISTER_OPCODE(301, Myst, NOP);
	REGISTER_OPCODE(302, Myst, NOP);
	REGISTER_OPCODE(303, Myst, NOP);
	REGISTER_OPCODE(304, Myst, o_treeCard_exit);
	REGISTER_OPCODE(305, Myst, o_treeEntry_exit);
	REGISTER_OPCODE(306, Myst, o_boiler_exit);
	REGISTER_OPCODE(307, Myst, o_generatorControlRoom_exit);
	REGISTER_OPCODE(308, Myst, o_rocketSliders_exit);
	REGISTER_OPCODE(309, Myst, NOP);
	REGISTER_OPCODE(312, Myst, NOP);
}

void Myst::disablePersistentScripts() {
	_libraryBookcaseMoving = false;
	_generatorControlRoomRunning = false;
	_libraryBookPagesTurning = false;
	_libraryCombinationBookPagesTurning = false;
	_clockTurningWheel = 0;
	_towerRotationMapRunning = false;
	_boilerPressureIncreasing = false;
	_boilerPressureDecreasing = false;
	_basementPressureIncreasing = false;
	_basementPressureDecreasing = false;
	_imagerValidationRunning = false;
	_imagerRunning = false;
	_observatoryRunning = false;
	_observatoryMonthChanging = false;
	_observatoryDayChanging = false;
	_observatoryYearChanging = false;
	_observatoryTimeChanging = false;
	_greenBookRunning = false;
	_clockLeverPulled = false;
	_gullsFlying1 = false;
	_gullsFlying2 = false;
	_gullsFlying3 = false;
}

void Myst::runPersistentScripts() {
	if (_towerRotationMapRunning)
		towerRotationMap_run();

	if (_generatorControlRoomRunning)
		generatorControlRoom_run();

	if (_libraryCombinationBookPagesTurning)
		libraryCombinationBook_run();

	if (_libraryBookPagesTurning)
		libraryBook_run();

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

	if (_imagerValidationRunning)
		imagerValidation_run();

	if (_imagerRunning)
		imager_run();

	if (_observatoryRunning)
		observatory_run();

	if (_observatoryMonthChanging)
		observatoryMonthChange_run();

	if (_observatoryDayChanging)
		observatoryDayChange_run();

	if (_observatoryYearChanging)
		observatoryYearChange_run();

	if (_observatoryTimeChanging)
		observatoryTimeChange_run();

	if (_greenBookRunning)
		greenBook_run();

	if (_clockLeverPulled)
		clockGears_run();

	if (_gullsFlying1)
		gullsFly1_run();

	if (_gullsFlying2)
		gullsFly2_run();

	if (_gullsFlying3)
		gullsFly3_run();
}

uint16 Myst::getVar(uint16 var) {
	switch(var) {
	case 0: // Myst Library Bookcase Closed
		return _state.libraryBookcaseDoor;
	case 1:
		if (_globals.ending != kBooksDestroyed)
			return _state.libraryBookcaseDoor != 1;
		else if (_state.libraryBookcaseDoor == 1)
			return 2;
		else
			return 3;
	case 2: // Marker Switch Near Cabin
		return _state.cabinMarkerSwitch;
	case 3: // Marker Switch Near Clock Tower
		return _state.clockTowerMarkerSwitch;
	case 4: // Marker Switch on Dock
		return _state.dockMarkerSwitch;
	case 5: // Marker Switch Near Ship Pool
		return _state.poolMarkerSwitch;
	case 6: // Marker Switch Near Cogs
		return _state.gearsMarkerSwitch;
	case 7: // Marker Switch Near Generator Room
		return _state.generatorMarkerSwitch;
	case 8: // Marker Switch Near Stellar Observatory
		return _state.observatoryMarkerSwitch;
	case 9: // Marker Switch Near Rocket Ship
		return _state.rocketshipMarkerSwitch;
	case 10: // Ship Floating State
		return _state.shipFloating;
	case 11: // Cabin Door Open State
		return _cabinDoorOpened;
	case 12: // Clock tower gears bridge
		return _state.clockTowerBridgeOpen;
	case 13: // Tower in right position
		return _state.towerRotationAngle == 271
				|| _state.towerRotationAngle == 83
				|| _state.towerRotationAngle == 129
				|| _state.towerRotationAngle == 152;
	case 14: // Tower Solution (Key) Plaque
		switch (_state.towerRotationAngle) {
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
		switch (_state.towerRotationAngle) {
		case 271:
			return 1;
		case 83:
			if (_state.gearsOpen)
				return 6;
			else
				return 2;
		case 129:
			if (_state.shipFloating)
				return 5;
			else
				return 3;
		case 152:
			return 4;
		default:
			return 0;
		}
	case 16: // Tower Window (Book) View From Ladder Top
		if (_state.towerRotationAngle != 271 && _state.towerRotationAngle != 83	&& _state.towerRotationAngle != 129) {
			if (_state.towerRotationAngle == 152)
				return 2;
			else
				return 0;
		} else
			return 1;
	case 23: // Fireplace Pattern Correct
		return _fireplaceLines[0] == 195
				&& _fireplaceLines[1] == 107
				&& _fireplaceLines[2] == 163
				&& _fireplaceLines[3] == 147
				&& _fireplaceLines[4] == 204
				&& _fireplaceLines[5] == 250;
	case 24: // Fireplace Blue Page Present
		if (_globals.ending != kBooksDestroyed)
			return !(_globals.bluePagesInBook & 32) && (_globals.heldPage != kBlueFirePlacePage);
		else
			return 0;
	case 25: // Fireplace Red Page Present
		if (_globals.ending != kBooksDestroyed)
			return !(_globals.redPagesInBook & 32) && (_globals.heldPage != kRedFirePlacePage);
		else
			return 0;
	case 26: // Courtyard Image Box - Cross
	case 27: // Courtyard Image Box - Leaf
	case 28: // Courtyard Image Box - Arrow
	case 29: // Courtyard Image Box - Eye
	case 30: // Courtyard Image Box - Snake
	case 31: // Courtyard Image Box - Spider
	case 32: // Courtyard Image Box - Anchor
	case 33: // Courtyard Image Box - Ostrich
		if (!_tempVar)
			return 0;
		else if (_state.courtyardImageBoxes & (0x01 << (var - 26)))
			return 2;
		else
			return 1;
	case 34: // Sound Control In Dock forechamber
		if (_state.imagerActive) {
			if (_state.imagerSelection == 40 && !_state.imagerMountainErased)
				return 1;
			else if (_state.imagerSelection == 67 && !_state.imagerWaterErased)
				return 2;
			else
				return 0;
		}

		return 0;
	case 35: // Dock Forechamber Imager Control Left Digit
		if (_state.imagerSelection > 9)
			return _state.imagerSelection / 10 - 1;
		else
			return 9;
	case 36: // Dock Forechamber Imager Control Right Digit
		return (10 + _state.imagerSelection - 1) % 10;
	case 37: // Clock Tower Control Wheels Position
		return 3 * ((_state.clockTowerMinutePosition / 5) % 3) + _state.clockTowerHourPosition % 3;
	case 40: // Gears Open State
		return _state.gearsOpen;
	case 41: // Dock Marker Switch Vault State
		return _dockVaultState;
	case 43: // Clock Tower Time
		return _state.clockTowerHourPosition * 12 + _state.clockTowerMinutePosition / 5;
	case 44: // Rocket ship power state
		if (_state.generatorBreakers || _state.generatorVoltage == 0)
			return 0;
		else if (_state.generatorVoltage != 59)
			return 1;
		else
			return 2;
	case 45: // Dock Vault Imager Active On Water
		return _state.imagerActive && _state.imagerSelection == 67 && !_state.imagerWaterErased;
	case 46:
		return bookCountPages(100);
	case 47:
		return bookCountPages(101);
	case 48:
		if (_state.dockMarkerSwitch && !_state.shipFloating)
			return 1;
		else if (!_state.dockMarkerSwitch && _state.shipFloating)
			return 2;
		else
			return 0;
	case 49: // Generator running
		return _state.generatorVoltage > 0;
	case 51: // Forechamber Imager Movie Control
		if (_state.imagerSelection == 40 && !_state.imagerMountainErased)
			return 1;
		else if (_state.imagerSelection == 67 && !_state.imagerWaterErased)
			return 2;
		else if (_state.imagerSelection == 8 && !_state.imagerAtrusErased)
			return 3;
		else if (_state.imagerSelection == 47 && !_state.imagerMarkerErased)
			return 4;
		else
			return 0;
	case 52: // Generator Switch #1
	case 53: // Generator Switch #2
	case 54: // Generator Switch #3
	case 55: // Generator Switch #4
	case 56: // Generator Switch #5
	case 57: // Generator Switch #6
	case 58: // Generator Switch #7
	case 59: // Generator Switch #8
	case 60: // Generator Switch #9
	case 61: // Generator Switch #10
		return (_state.generatorButtons & (1 << (var - 52))) != 0;
	case 62: // Generator Power Dial Left LED Digit
		return _generatorVoltage / 10;
	case 63: // Generator Power Dial Right LED Digit
		return _generatorVoltage % 10;
	case 64: // Generator Power To Spaceship Dial Left LED Digit
		if (_state.generatorVoltage > 59 || _state.generatorBreakers)
			return 0;
		else
			return _state.generatorVoltage / 10;
	case 65: // Generator Power To Spaceship Dial Right LED Digit
		if (_state.generatorVoltage > 59 || _state.generatorBreakers)
			return 0;
		else
			return _state.generatorVoltage % 10;
	case 66: // Generators lights on
		return 0;
	case 67: // Cabin Safe Lock Number #1 - Left
		return _state.cabinSafeCombination / 100;
	case 68: // Cabin Safe Lock Number #2
		return (_state.cabinSafeCombination / 10) % 10;
	case 69: // Cabin Safe Lock Number #3 - Right
		return _state.cabinSafeCombination % 10;
	case 70: // Cabin Safe Matchbox State
		return _cabinMatchState;
	case 71: // Stellar Observatory Lights
		return _state.observatoryLights;
	case 72: // Channelwood tree position
		return _state.treePosition;
	case 73: // Stellar Observatory Date - Month
		return _state.observatoryMonthSetting;
	case 74: // Stellar Observatory Date - Day #1 (Left)
		if (_state.observatoryDaySetting / 10 == 0)
			return 10;
		else
			return _state.observatoryDaySetting / 10;
	case 75: // Stellar Observatory Date - Day #2 (Right)
		return _state.observatoryDaySetting % 10;
	case 76: // Stellar Observatory Date - Year #1 (Left)
		if (_state.observatoryYearSetting >= 1000)
			return (_state.observatoryYearSetting / 1000) % 10;
		else
			return 10;
	case 77: // Stellar Observatory Date - Year #2
		if (_state.observatoryYearSetting >= 100)
			return (_state.observatoryYearSetting / 100) % 10;
		else
			return 10;
	case 78: // Stellar Observatory Date - Year #3
		if (_state.observatoryYearSetting >= 10)
			return (_state.observatoryYearSetting / 10) % 10;
		else
			return 10;
	case 79: // Stellar Observatory Date - Year #4 (Right)
		return (_state.observatoryYearSetting / 1) % 10;
	case 80: // Stellar Observatory Hour #1 - Left ( Hour digits can be 10 (Blank), or 0-2)
		uint32 observatoryLeftMinutes;
		if (!observatoryIsDDMMYYYY2400()) {
			// 12 Hour Format
			observatoryLeftMinutes = _state.observatoryTimeSetting % (12 * 60);
			if (observatoryLeftMinutes > 59 && observatoryLeftMinutes < (10 * 60))
				return 10;
			else
				return 1;
		} else {
			// 24 Hour Format
			observatoryLeftMinutes = _state.observatoryTimeSetting;
			if (observatoryLeftMinutes < (10 * 60))
				return 0;
			else if (observatoryLeftMinutes < (20 * 60))
				return 1;
			else
				return 2;
		}
	case 81: // Stellar Observatory Hour #2 - Right
		uint32 observatoryRightMinutes,observatoryRightHour;
		if (!observatoryIsDDMMYYYY2400()) {
			// 12 Hour Format
			observatoryRightMinutes = _state.observatoryTimeSetting % (12 * 60);
			observatoryRightHour = observatoryRightMinutes / 60;
			if (observatoryRightHour % 12 == 0)
				return 2;
			else
				return observatoryRightHour % 10;
		} else {
			// 24 Hour Format
			observatoryRightMinutes = _state.observatoryTimeSetting;
			observatoryRightHour = observatoryRightMinutes / 60;
			return observatoryRightHour % 10;
		}
	case 82: // Stellar Observatory Minutes #1 - Left
		return (_state.observatoryTimeSetting % 60) / 10;
	case 83: // Stellar Observatory Minutes #2 - Right
		return (_state.observatoryTimeSetting % 60) % 10;
	case 88: // Stellar Observatory AM/PM
		if (_state.observatoryTimeSetting < (12 * 60))
			return 0; // AM
		else
			return 1; // PM
	case 89:
	case 90:
	case 91:
	case 92: // Stellar observatory sliders state
		return 1;
	case 93: // Breaker nearest Generator Room Blown
		return _state.generatorBreakers == 1;
	case 94: // Breaker nearest Rocket Ship Blown
		return _state.generatorBreakers == 2;
	case 95: // Going out of tree destination selection
		if (_state.treePosition == 0)
			return 0;
		else if (_state.treePosition == 4 || _state.treePosition == 5)
			return 1;
		else
			return 2;
	case 96: // Generator Power Dial Needle Position
		return _state.generatorVoltage / 4;
	case 97: // Generator Power To Spaceship Dial Needle Position
		if (_state.generatorVoltage > 59 || _state.generatorBreakers)
			return 0;
		else
			return _state.generatorVoltage / 4;
	case 98: // Cabin Boiler Pilot Light Lit
		return _state.cabinPilotLightLit;
	case 99: // Cabin Boiler Gas Valve Position
		return _state.cabinValvePosition % 6;
	case 102: // Red page
		if (_globals.ending != kBooksDestroyed)
			return !(_globals.redPagesInBook & 1) && (_globals.heldPage != kRedLibraryPage);
		else
			return 0;
	case 103: // Blue page
		if (_globals.ending != kBooksDestroyed)
			return !(_globals.bluePagesInBook & 1) && (_globals.heldPage != kBlueLibraryPage);
		else
			return 0;
	case 300: // Rocket Ship Music Puzzle Slider State
		return 1;
	case 302: // Green Book Opened Before Flag
		return _state.greenBookOpenedBefore;
	case 303: // Library Bookcase status changed
		return _libraryBookcaseChanged;
	case 304: // Tower Rotation Map Initialized
		return _towerRotationMapInitialized;
	case 305: // Cabin Boiler Lit
		return _state.cabinPilotLightLit == 1 && _state.cabinValvePosition > 0;
	case 306: // Cabin Boiler Steam Sound Control
		if (_state.cabinPilotLightLit == 1) {
			if (_state.cabinValvePosition <= 0)
				return 26;
			else
				return 27;
		}

		return _state.cabinValvePosition;
	case 307: // Cabin Boiler Fully Pressurized
		return _state.cabinPilotLightLit == 1 && _state.cabinValvePosition > 12;
	case 308: // Cabin handle position
		return _cabinHandleDown;
	default:
		return MystScriptParser::getVar(var);
	}
}

void Myst::toggleVar(uint16 var) {
	switch(var) {
	case 2: // Marker Switch Near Cabin
		_state.cabinMarkerSwitch = (_state.cabinMarkerSwitch + 1) % 2;
		break;
	case 3: // Marker Switch Near Clock Tower
		_state.clockTowerMarkerSwitch = (_state.clockTowerMarkerSwitch + 1) % 2;
		break;
	case 4: // Marker Switch on Dock
		_state.dockMarkerSwitch = (_state.dockMarkerSwitch + 1) % 2;
		break;
	case 5: // Marker Switch Near Ship Pool
		_state.poolMarkerSwitch = (_state.poolMarkerSwitch + 1) % 2;
		break;
	case 6: // Marker Switch Near Cogs
		_state.gearsMarkerSwitch = (_state.gearsMarkerSwitch + 1) % 2;
		break;
	case 7: // Marker Switch Near Generator Room
		_state.generatorMarkerSwitch = (_state.generatorMarkerSwitch + 1) % 2;
		break;
	case 8: // Marker Switch Near Stellar Observatory
		_state.observatoryMarkerSwitch = (_state.observatoryMarkerSwitch + 1) % 2;
		break;
	case 9: // Marker Switch Near Rocket Ship
		_state.rocketshipMarkerSwitch = (_state.rocketshipMarkerSwitch + 1) % 2;
		break;
	case 24: // Fireplace Blue Page
		if (_globals.ending != kBooksDestroyed && !(_globals.bluePagesInBook & 32)) {
			if (_globals.heldPage == kBlueFirePlacePage)
				_globals.heldPage = kNoPage;
			else
				_globals.heldPage = kBlueFirePlacePage;
		}
		break;
	case 25: // Fireplace Red page
		if (_globals.ending != kBooksDestroyed && !(_globals.redPagesInBook & 32)) {
			if (_globals.heldPage == kRedFirePlacePage)
				_globals.heldPage = kNoPage;
			else
				_globals.heldPage = kRedFirePlacePage;
		}
		break;
	case 26: // Courtyard Image Box - Cross
	case 27: // Courtyard Image Box - Leaf
	case 28: // Courtyard Image Box - Arrow
	case 29: // Courtyard Image Box - Eye
	case 30: // Courtyard Image Box - Snake
	case 31: // Courtyard Image Box - Spider
	case 32: // Courtyard Image Box - Anchor
	case 33: // Courtyard Image Box - Ostrich
		{
			uint16 mask = 0x01 << (var - 26);
			if (_state.courtyardImageBoxes & mask)
				_state.courtyardImageBoxes &= ~mask;
			else
				_state.courtyardImageBoxes |= mask;
		}
		break;
	case 41: // Vault white page
		if (_globals.ending != kBooksDestroyed) {
			if (_dockVaultState == 1) {
				_dockVaultState = 2;
				_globals.heldPage = kNoPage;
			} else if (_dockVaultState == 2) {
				_dockVaultState = 1;
				_globals.heldPage = kWhitePage;
			}
		}
		break;
	case 102: // Red page
		if (_globals.ending != kBooksDestroyed && !(_globals.redPagesInBook & 1)) {
			if (_globals.heldPage == kRedLibraryPage)
				_globals.heldPage = kNoPage;
			else
				_globals.heldPage = kRedLibraryPage;
		}
		break;
	case 103: // Blue page
		if (_globals.ending != kBooksDestroyed && !(_globals.bluePagesInBook & 1)) {
			if (_globals.heldPage == kBlueLibraryPage)
				_globals.heldPage = kNoPage;
			else
				_globals.heldPage = kBlueLibraryPage;
		}
		break;
	default:
		MystScriptParser::toggleVar(var);
		break;
	}
}

bool Myst::setVarValue(uint16 var, uint16 value) {
	bool refresh = false;

	switch (var) {
	case 0: // Myst Library Bookcase Closed
		if (_state.libraryBookcaseDoor != value) {
			_state.libraryBookcaseDoor = value;
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
	case 71: // Stellar Observatory Lights
		_state.observatoryLights = value;
		break;
	case 89:
	case 90:
	case 91:
	case 92:
	case 300: // Set slider value
		break; // Do nothing
	case 302: // Green Book Opened Before Flag
		_state.greenBookOpenedBefore = value;
		break;
	case 303: // Library Bookcase status changed
		_libraryBookcaseChanged = value;
		break;
	case 304: // Myst Library Image Present on Tower Rotation Map
		_towerRotationMapInitialized = value;
		break;
	case 308: // Cabin handle position
		_cabinHandleDown = value;
		break;
	case 309: // Tree stopped
		_treeStopped = value;
		break;
	case 310: // Imager validation step
		_imagerValidationStep = value;
		break;
	default:
		refresh = MystScriptParser::setVarValue(var, value);
		break;
	}

	return refresh;
}

uint16 Myst::bookCountPages(uint16 var) {
	uint16 pages = 0;
	uint16 cnt = 0;

	// Select book according to var
	if (var == 100)
		pages = _globals.redPagesInBook;
	else if (var == 101)
		pages = _globals.bluePagesInBook;

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

void Myst::o_libraryBookPageTurnLeft(uint16 var, const ArgumentsArray &args) {
	libraryBookPageTurnLeft();
}

void Myst::libraryBookPageTurnLeft() {
	if (_libraryBookPage - 1 >= 0) {
		_libraryBookPage--;

		Common::Rect rect = Common::Rect(0, 0, 544, 333);
		_vm->_gfx->copyImageToScreen(_libraryBookBaseImage + _libraryBookPage, rect);

		if (_vm->_rnd->getRandomBit())
			_vm->_sound->playEffect(_libraryBookSound1);
		else
			_vm->_sound->playEffect(_libraryBookSound2);
	}
}

void Myst::o_libraryBookPageTurnRight(uint16 var, const ArgumentsArray &args) {
	libraryBookPageTurnRight();
}

void Myst::libraryBookPageTurnRight() {
	if (_libraryBookPage + 1 < _libraryBookNumPages) {
		_libraryBookPage++;

		Common::Rect rect = Common::Rect(0, 0, 544, 333);
		_vm->_gfx->copyImageToScreen(_libraryBookBaseImage + _libraryBookPage, rect);

		if (_vm->_rnd->getRandomBit())
			_vm->_sound->playEffect(_libraryBookSound1);
		else
			_vm->_sound->playEffect(_libraryBookSound2);
	}
}

void Myst::o_fireplaceToggleButton(uint16 var, const ArgumentsArray &args) {
	// Used on Myst Card 4162 (Fireplace Grid)
	uint16 bitmask = args[0];
	uint16 line = _fireplaceLines[var - 17];
	Common::Rect buttonRect = getInvokingResource<MystArea>()->getRect();

	// The animations are too slow when playing each animation image at 60fps.
	// Only play every second image.

	if (line & bitmask) {
		// Unset button
		for (uint i = 4795; i >= 4779; i -= 2) {
			_vm->_gfx->copyImageToScreen(i, buttonRect);
			_vm->doFrame();
		}
		_vm->_gfx->copyBackBufferToScreen(buttonRect);
		_fireplaceLines[var - 17] &= ~bitmask;
	} else {
		// Set button
		for (uint i = 4779; i <= 4795; i += 2) {
			_vm->_gfx->copyImageToScreen(i, buttonRect);
			_vm->doFrame();
		}
		_fireplaceLines[var - 17] |= bitmask;
	}
}

void Myst::o_fireplaceRotation(uint16 var, const ArgumentsArray &args) {
	// Used on Myst Card 4162 and 4166 (Fireplace Puzzle Rotation Movies)
	uint16 movieNum = args[0];

	if (movieNum)
		_vm->playMovieBlocking("fpout", kMystStack, 167, 4);
	else
		_vm->playMovieBlocking("fpin", kMystStack, 167, 4);
}

void Myst::o_courtyardBoxesCheckSolution(uint16 var, const ArgumentsArray &args) {
	uint16 soundId = args[0];

	// Change ship state if the boxes are correctly enabled
	if (_state.courtyardImageBoxes == 50 && !_state.shipFloating) {
		_vm->_cursor->hideCursor();
		_state.shipFloating = 1;
		_vm->playSoundBlocking(soundId);
		_vm->_cursor->showCursor();
	} else if (_state.courtyardImageBoxes != 50 && _state.shipFloating) {
		_vm->_cursor->hideCursor();
		_state.shipFloating = 0;
		_vm->playSoundBlocking(soundId);
		_vm->_cursor->showCursor();
	}
}

void Myst::o_towerRotationStart(uint16 var, const ArgumentsArray &args) {
	_towerRotationBlinkLabel = false;
	_towerRotationMapClicked = true;
	_towerRotationSpeed = 0;

	_vm->_cursor->setCursor(700);

	Common::Point end = towerRotationMapComputeCoords(_state.towerRotationAngle);
	towerRotationMapComputeAngle();
	towerRotationMapDrawLine(end, true);

	_vm->_sound->playEffect(5378, true);
}

void Myst::o_towerRotationEnd(uint16 var, const ArgumentsArray &args) {
	_towerRotationMapClicked = false;

	// Set angle value to expected value
	if (_state.towerRotationAngle >= 265
			&& _state.towerRotationAngle <= 277
			&& _state.rocketshipMarkerSwitch) {
		_state.towerRotationAngle = 271;
	} else if (_state.towerRotationAngle >= 77
			&& _state.towerRotationAngle <= 89
			&& _state.gearsMarkerSwitch) {
		_state.towerRotationAngle = 83;
	} else if (_state.towerRotationAngle >= 123
			&& _state.towerRotationAngle <= 135
			&& _state.dockMarkerSwitch) {
		_state.towerRotationAngle = 129;
	} else if (_state.towerRotationAngle >= 146
			&& _state.towerRotationAngle <= 158
			&& _state.cabinMarkerSwitch) {
		_state.towerRotationAngle = 152;
	}

	_vm->_sound->playEffect(6378);

	_towerRotationBlinkLabel = true;
	_towerRotationBlinkLabelCount = 0;
}

void Myst::o_imagerChangeSelection(uint16 var, const ArgumentsArray &args) {
	if (_imagerValidationStep != 10) {
		_imagerValidationStep = 0;

		int16 signedValue = args[0];
		uint16 d1 = (_state.imagerSelection / 10) % 10;
		uint16 d2 = _state.imagerSelection % 10;

		if (var == 35 && signedValue > 0 && d1 < 9)
			d1++;
		else if (var == 35 && signedValue < 0 && d1 > 0)
			d1--;
		else if (var == 36 && signedValue > 0 && d2 < 9)
			d2++;
		else if (var == 36 && signedValue < 0 && d2 > 0)
			d2--;

		_state.imagerSelection = 10 * d1 + d2;
		_state.imagerActive = 0;

		_vm->getCard()->redrawArea(var);
	}
}

void Myst::o_dockVaultOpen(uint16 var, const ArgumentsArray &args) {
	// Used on Myst 4143 (Dock near Marker Switch)
	uint16 soundId = args[0];
	uint16 delay = args[1];
	uint16 directionalUpdateDataSize = args[2];

	if ((_state.cabinMarkerSwitch == 1) &&
		(_state.clockTowerMarkerSwitch == 1) &&
		(_state.dockMarkerSwitch == 0) &&
		(_state.gearsMarkerSwitch == 1) &&
		(_state.generatorMarkerSwitch == 1) &&
		(_state.observatoryMarkerSwitch == 1) &&
		(_state.poolMarkerSwitch == 1) &&
		(_state.rocketshipMarkerSwitch == 1)) {
		if (_globals.heldPage != kWhitePage && _globals.ending != kBooksDestroyed)
			_dockVaultState = 2;
		else
			_dockVaultState = 1;

		_vm->_sound->playEffect(soundId);
		_vm->getCard()->redrawArea(41, false);
		animatedUpdate(ArgumentsArray(args.begin() + 3, directionalUpdateDataSize), delay);
	}
}

void Myst::o_dockVaultClose(uint16 var, const ArgumentsArray &args) {
	// Used on Myst 4143 (Dock near Marker Switch)
	uint16 soundId = args[0];
	uint16 delay = args[1];
	uint16 directionalUpdateDataSize = args[2];

	if ((_state.cabinMarkerSwitch == 1) &&
		(_state.clockTowerMarkerSwitch == 1) &&
		(_state.dockMarkerSwitch == 1) &&
		(_state.gearsMarkerSwitch == 1) &&
		(_state.generatorMarkerSwitch == 1) &&
		(_state.observatoryMarkerSwitch == 1) &&
		(_state.poolMarkerSwitch == 1) &&
		(_state.rocketshipMarkerSwitch == 1)) {
		if (_dockVaultState == 1 || _dockVaultState == 2)
			_dockVaultState = 0;

		_vm->_sound->playEffect(soundId);
		_vm->getCard()->redrawArea(41, false);
		animatedUpdate(ArgumentsArray(args.begin() + 3, directionalUpdateDataSize), delay);
	}
}

void Myst::o_bookGivePage(uint16 var, const ArgumentsArray &args) {
	uint16 cardIdLose = args[0];
	uint16 cardIdBookCover = args[1];
	uint16 soundIdAddPage = args[2];

	debugC(kDebugScript, "Card Id (Lose): %d", cardIdLose);
	debugC(kDebugScript, "Card Id (Book Cover): %d", cardIdBookCover);
	debugC(kDebugScript, "SoundId (Add Page): %d", soundIdAddPage);

	uint16 bookVar = 101;
	uint16 mask = 0;

	switch (_globals.heldPage) {
	case kNoPage:
	case kWhitePage:
		_vm->changeToCard(cardIdBookCover, kTransitionDissolve);
		return;
	case kRedLibraryPage:
		bookVar = 100;
		// fallthrough
	case kBlueLibraryPage:
		mask = 1;
		break;
	case kRedSeleniticPage:
		bookVar = 100;
		// fallthrough
	case kBlueSeleniticPage:
		mask = 2;
		break;
	case kRedMechanicalPage:
		bookVar = 100;
		// fallthrough
	case kBlueMechanicalPage:
		mask = 4;
		break;
	case kRedStoneshipPage:
		bookVar = 100;
		// fallthrough
	case kBlueStoneshipPage:
		mask = 8;
		break;
	case kRedChannelwoodPage:
		bookVar = 100;
		// fallthrough
	case kBlueChannelwoodPage:
		mask = 16;
		break;
	case kRedFirePlacePage:
		bookVar = 100;
		// fallthrough
	case kBlueFirePlacePage:
		mask = 32;
		break;
	default:
		break;
	}

	// Wrong book
	if (bookVar != var) {
		_vm->changeToCard(cardIdBookCover, kTransitionDissolve);
		return;
	}

	_vm->_cursor->hideCursor();
	_vm->playSoundBlocking(soundIdAddPage);
	_vm->setMainCursor(kDefaultMystCursor);

	// Add page to book
	if (var == 100)
		_globals.redPagesInBook |= mask;
	else
		_globals.bluePagesInBook |= mask;

	// Remove page from hand
	_globals.heldPage = kNoPage;

	_vm->_cursor->showCursor();

	if (mask == 32) {
		// You lose!
		if (var == 100)
			_globals.currentAge = kSirrusEnding;
		else
			_globals.currentAge = kAchenarEnding;

		_vm->changeToCard(cardIdLose, kTransitionDissolve);
	} else {
		_vm->changeToCard(cardIdBookCover, kTransitionDissolve);
	}
}

void Myst::o_clockWheelsExecute(uint16 var, const ArgumentsArray &args) {
	// Used on Card 4006 (Clock Tower Time Controls)
	uint16 soundId = args[0];

	// Correct time is 2:40
	bool correctTime = _state.clockTowerHourPosition == 2
						&& _state.clockTowerMinutePosition == 40;

	if (!_state.clockTowerBridgeOpen && correctTime) {
		_vm->_sound->playEffect(soundId);
		_vm->wait(500);

		// Gears rise up
		VideoEntryPtr gears = _vm->playMovie("gears", kMystStack);
		gears->moveTo(305, 33);
		gears->setBounds(Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 650, 600));

		_vm->waitUntilMovieEnds(gears);

		_state.clockTowerBridgeOpen = 1;
		_vm->getCard()->redrawArea(12);
	} else if (_state.clockTowerBridgeOpen && !correctTime) {
		_vm->_sound->playEffect(soundId);
		_vm->wait(500);

		// Gears sink down
		VideoEntryPtr gears = _vm->playMovie("gears", kMystStack);
		gears->moveTo(305, 33);
		gears->setBounds(Audio::Timestamp(0, 700, 600), Audio::Timestamp(0, 1300, 600));

		_vm->waitUntilMovieEnds(gears);

		_state.clockTowerBridgeOpen = 0;
		_vm->getCard()->redrawArea(12);
	}
}

void Myst::o_imagerPlayButton(uint16 var, const ArgumentsArray &args) {
	uint16 video = getVar(51);

	// Press button
	_vm->_sound->playEffect(4698);

	Common::Rect src = Common::Rect(0, 0, 32, 75);
	Common::Rect dest = Common::Rect(261, 257, 293, 332);
	_vm->_gfx->copyImageSectionToScreen(4699, src, dest);

	_vm->wait(200);

	_vm->_gfx->copyBackBufferToScreen(dest);
	_vm->doFrame();

	_vm->_cursor->hideCursor();


	// Play selected video
	if (!_state.imagerActive && video != 3)
		_vm->_sound->playEffect(args[0]);

	switch (video) {
	case 0: // Nothing
	case 3: // Atrus
	case 4: // Marker
		_imagerMovie->playMovie();
		break;
	case 1: // Mountain
		if (_state.imagerActive) {
			// Mountains disappearing
			Common::String file = "vltmntn";
			VideoEntryPtr mountain = _vm->playMovie(file, kMystStack);
			mountain->moveTo(159, 96);
			mountain->setBounds(Audio::Timestamp(0, 11180, 600), Audio::Timestamp(0, 16800, 600));

			_state.imagerActive = 0;
		} else {
			// Mountains appearing
			Common::String file = "vltmntn";
			VideoEntryPtr mountain = _vm->playMovie(file, kMystStack);
			mountain->moveTo(159, 96);
			mountain->setBounds(Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 11180, 600));

			_state.imagerActive = 1;
		}
		break;
	case 2: // Water
		_imagerMovie->setBlocking(false);

		if (_state.imagerActive) {
			_vm->_sound->playEffect(args[1]);

			// Water disappearing
			VideoEntryPtr water = _imagerMovie->playMovie();
			water->setBounds(Audio::Timestamp(0, 4204, 600), Audio::Timestamp(0, 6040, 600));
			water->setLooping(false);

			_state.imagerActive = 0;
		} else {
			// Water appearing
			VideoEntryPtr water = _imagerMovie->playMovie();
			water->setBounds(Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 1814, 600));
			_vm->waitUntilMovieEnds(water);

			// Water looping
			water = _imagerMovie->playMovie();
			water->setBounds(Audio::Timestamp(0, 1814, 600), Audio::Timestamp(0, 4204, 600));
			water->setLooping(true);

			_state.imagerActive = 1;
		}
		break;
	default:
		break;
	}

	_vm->_cursor->showCursor();
}

void Myst::o_imagerEraseButton(uint16 var, const ArgumentsArray &args) {
	_imagerRedButton = static_cast<MystAreaImageSwitch *>(getInvokingResource<MystArea>()->_parent);
	for (uint i = 0; i < 4; i++)
		_imagerSound[i] = args[i];
	_imagerValidationCard = args[4];

	if (_imagerValidationStep == 0) {
		// Validation script is not running, run it
		_startTime = _vm->getTotalPlayTime() + 100;
		_imagerValidationRunning = true;
		return;
	} else if (_imagerValidationStep < 7) {
		// Too early
		_vm->playSoundBlocking(_imagerSound[2]);
		_imagerValidationStep = 0;
		return;
	} else if (_imagerValidationStep < 11) {
		_vm->playSoundBlocking(_imagerSound[3]);

		// Erase selected video from imager
		switch (_state.imagerSelection) {
		case 8:
			_state.imagerAtrusErased = 1;
			break;
		case 40:
			_state.imagerMountainErased = 1;
			break;
		case 47:
			_state.imagerMarkerErased = 1;
			break;
		case 67:
			_state.imagerWaterErased = 1;
			break;
		default:
			break;
		}

		_state.imagerActive = 0;
		_imagerValidationStep = 0;
		return;
	} else if (_imagerValidationStep == 11) {
		// Too late
		_imagerValidationStep = 0;
		return;
	}
}

void Myst::imagerValidation_run() {
	uint32 time = _vm->getTotalPlayTime();

	if (time > _startTime) {
		_imagerRedButton->drawConditionalDataToScreen(1);

		if (_imagerValidationStep < 6)
			_vm->_sound->playEffect(_imagerSound[0]);
		else if (_imagerValidationStep < 10)
			_vm->_sound->playEffect(_imagerSound[1]);
		else if (_imagerValidationStep == 10)
			_vm->_sound->playEffect(_imagerSound[2]);

		_imagerValidationStep++;

		_vm->wait(50);

		_imagerRedButton->drawConditionalDataToScreen(0);

		if (_imagerValidationStep == 11) {
			_imagerValidationStep = 0;
			_vm->changeToCard(_imagerValidationCard, kTransitionBottomToTop);
		} else {
			_startTime = time + 100;
		}
	}
}

void Myst::o_towerElevatorAnimation(uint16 var, const ArgumentsArray &args) {
	_treeStopped = true;

	_vm->_cursor->hideCursor();
	_vm->_sound->stopEffect();
	_vm->_sound->pauseBackground();

	switch (args[0]) {
	case 0:
		_vm->playMovieBlocking("libdown", kMystStack, 216, 78);
		break;
	case 1:
		_vm->playMovieBlocking("libup", kMystStack, 216, 78);
		break;
	default:
		break;
	}

	_vm->_sound->resumeBackground();
	_vm->_cursor->showCursor();
	_treeStopped = false;
}

void Myst::o_generatorButtonPressed(uint16 var, const ArgumentsArray &args) {
	MystArea *button = getInvokingResource<MystArea>()->_parent;

	generatorRedrawRocket();

	_generatorVoltage = _state.generatorVoltage;

	uint16 mask = 0;
	uint16 value = 0;
	generatorButtonValue(button, mask, value);

	// Button pressed
	if (_state.generatorButtons & mask) {
		_state.generatorButtons &= ~mask;
		_state.generatorVoltage -= value;

		if (_state.generatorVoltage)
			_vm->_sound->playEffect(8297);
		else {
			_vm->_sound->playEffect(9297);
			_vm->_sound->stopBackground();
		}
	} else {
		if (_generatorVoltage)
			_vm->_sound->playEffect(6297);
		else {
			_vm->_sound->playBackground(4297);
			_vm->_sound->playEffect(7297);
		}

		_state.generatorButtons |= mask;
		_state.generatorVoltage += value;
	}

	// Redraw button
	_vm->getCard()->redrawArea(button->getImageSwitchVar());

	// Blow breaker
	if (_state.generatorVoltage > 59)
		_state.generatorBreakers = _vm->_rnd->getRandomNumberRng(1, 2);
}

void Myst::generatorRedrawRocket() {
	_vm->getCard()->redrawArea(64);
	_vm->getCard()->redrawArea(65);
	_vm->getCard()->redrawArea(97);
}

void Myst::generatorButtonValue(MystArea *button, uint16 &mask, uint16 &value) {
	switch (button->getImageSwitchVar()) {
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
	default:
		break;
	}
}

void Myst::o_cabinSafeChangeDigit(uint16 var, const ArgumentsArray &args) {
	uint16 d1 = _state.cabinSafeCombination / 100;
	uint16 d2 = (_state.cabinSafeCombination / 10) % 10;
	uint16 d3 = _state.cabinSafeCombination % 10;

	if (var == 67)
		d1 = (d1 + 1) % 10;
	else if (var == 68)
		d2 = (d2 + 1) % 10;
	else
		d3 = (d3 + 1) % 10;

	_state.cabinSafeCombination = 100 * d1 + 10 * d2 + d3;

	_vm->getCard()->redrawArea(var);
}

void Myst::o_cabinSafeHandleStartMove(uint16 var, const ArgumentsArray &args) {
	// Used on Card 4100
	MystVideoInfo *handle = getInvokingResource<MystVideoInfo>();
	handle->drawFrame(0);
	_vm->_cursor->setCursor(700);
	_tempVar = 0;
}

void Myst::o_cabinSafeHandleMove(uint16 var, const ArgumentsArray &args) {
	// Used on Card 4100
	MystVideoInfo *handle = getInvokingResource<MystVideoInfo>();

	if (handle->pullLeverV()) {
		// Sound not played yet
		if (_tempVar == 0) {
			uint16 soundId = handle->getList2(0);
			if (soundId)
				_vm->_sound->playEffect(soundId);
		}
		// Combination is right
		if (_state.cabinSafeCombination == 724) {
			uint16 soundId = handle->getList2(1);
			if (soundId)
				_vm->_sound->playEffect(soundId);

			_vm->changeToCard(4103, kNoTransition);

			Common::Rect screenRect = Common::Rect(544, 333);
			_vm->_gfx->runTransition(kTransitionLeftToRight, screenRect, 2, 5);
		}
		_tempVar = 1;
	} else {
		_tempVar = 0;
	}
}

void Myst::o_cabinSafeHandleEndMove(uint16 var, const ArgumentsArray &args) {
	// Used on Card 4100
	MystVideoInfo *handle = getInvokingResource<MystVideoInfo>();
	handle->drawFrame(0);
	_vm->refreshCursor();
}

void Myst::o_observatoryMonthChangeStartIncrease(uint16 var, const ArgumentsArray &args) {
	observatoryMonthChangeStart(true);
}

void Myst::o_observatoryMonthChangeStartDecrease(uint16 var, const ArgumentsArray &args) {
	observatoryMonthChangeStart(false);
}

void Myst::observatoryMonthChangeStart(bool increase) {
	_vm->_sound->pauseBackground();

	if (increase) {
		// Increase
		if (observatoryIsDDMMYYYY2400())
			_vm->_gfx->copyImageSectionToScreen(11098, Common::Rect(36, 0, 48, 9), Common::Rect(351, 70, 363, 79));
		else
			_vm->_gfx->copyImageSectionToScreen(11098, Common::Rect(0, 0, 12, 9), Common::Rect(315, 70, 327, 79));

		_observatoryIncrement = -1;
	} else {
		// Decrease
		if (observatoryIsDDMMYYYY2400())
			_vm->_gfx->copyImageSectionToScreen(11097, Common::Rect(36, 0, 48, 9), Common::Rect(351, 204, 363, 213));
		else
			_vm->_gfx->copyImageSectionToScreen(11097, Common::Rect(0, 0, 12, 9), Common::Rect(315, 204, 327, 213));

		_observatoryIncrement = 1;
	}

	// Highlight slider
	_observatoryMonthSlider->drawConditionalDataToScreen(2);
	_observatoryCurrentSlider = _observatoryMonthSlider;

	// First increment
	observatoryIncrementMonth(_observatoryIncrement);

	// Start persistent script
	_startTime = _vm->getTotalPlayTime();
	_observatoryMonthChanging = true;
}

void Myst::observatoryIncrementMonth(int16 increment) {
	int16 newMonth = _state.observatoryMonthSetting + increment;

	if (newMonth >= 0 && newMonth <= 11) {
		_state.observatoryMonthSetting = newMonth;

		// Redraw digits
		_vm->getCard()->redrawArea(73);

		// Update slider
		_observatoryMonthSlider->setPosition(94 + 94 * _state.observatoryMonthSetting / 11);
		_observatoryMonthSlider->restoreBackground();
		_observatoryMonthSlider->drawConditionalDataToScreen(2);
		_state.observatoryMonthSlider = _observatoryMonthSlider->_pos.y;
	}

	_vm->_sound->playEffect(8500);
	_vm->wait(20);
}

void Myst::observatoryMonthChange_run() {
	if (_startTime + 500 < _vm->getTotalPlayTime())
		observatoryIncrementMonth(_observatoryIncrement);
}

void Myst::o_observatoryDayChangeStartIncrease(uint16 var, const ArgumentsArray &args) {
	observatoryDayChangeStart(true);
}

void Myst::o_observatoryDayChangeStartDecrease(uint16 var, const ArgumentsArray &args) {
	observatoryDayChangeStart(false);
}

void Myst::observatoryDayChangeStart(bool increase) {
	_vm->_sound->pauseBackground();

	if (increase) {
		// Increase
		if (observatoryIsDDMMYYYY2400())
			_vm->_gfx->copyImageSectionToScreen(11098, Common::Rect(0, 0, 12, 9), Common::Rect(315, 70, 327, 79));
		else
			_vm->_gfx->copyImageSectionToScreen(11098, Common::Rect(36, 0, 48, 9), Common::Rect(351, 70, 363, 79));

		_observatoryIncrement = -1;
	} else {
		// Decrease
		if (observatoryIsDDMMYYYY2400())
			_vm->_gfx->copyImageSectionToScreen(11097, Common::Rect(0, 0, 12, 9), Common::Rect(315, 204, 327, 213));
		else
			_vm->_gfx->copyImageSectionToScreen(11097, Common::Rect(36, 0, 48, 9), Common::Rect(351, 204, 363, 213));

		_observatoryIncrement = 1;
	}

	// Highlight slider
	_observatoryDaySlider->drawConditionalDataToScreen(2);
	_observatoryCurrentSlider = _observatoryDaySlider;

	// First increment
	observatoryIncrementDay(_observatoryIncrement);

	// Start persistent script
	_startTime = _vm->getTotalPlayTime();
	_observatoryDayChanging = true;
}

void Myst::observatoryIncrementDay(int16 increment) {
	int16 newDay = _state.observatoryDaySetting + increment;

	if (newDay >= 1 && newDay <= 31) {
		_state.observatoryDaySetting = newDay;

		// Redraw digits
		_vm->getCard()->redrawArea(75);
		_vm->getCard()->redrawArea(74);

		// Update slider
		// WORKAROUND: Have the day setting increment at 315/100 rather than x3 so that the slider
		// will reach the bottom spot on day 31st. Only relevant when using the down button and
		// not dragging the slider. Fixes Trac#10572. The original engine incremented it with x3 
		// and has this bug, but it is less noticeable.
		_observatoryDaySlider->setPosition(91 + (_state.observatoryDaySetting * 315) / 100 );
		_observatoryDaySlider->restoreBackground();
		_observatoryDaySlider->drawConditionalDataToScreen(2);
		_state.observatoryDaySlider = _observatoryDaySlider->_pos.y;
	}

	_vm->_sound->playEffect(8500);
	_vm->wait(20);
}

void Myst::observatoryDayChange_run() {
	if (_startTime + 500 < _vm->getTotalPlayTime())
		observatoryIncrementDay(_observatoryIncrement);
}

void Myst::o_observatoryYearChangeStartIncrease(uint16 var, const ArgumentsArray &args) {
	observatoryYearChangeStart(true);
}

void Myst::o_observatoryYearChangeStartDecrease(uint16 var, const ArgumentsArray &args) {
	observatoryYearChangeStart(false);
}

void Myst::observatoryYearChangeStart(bool increase) {
	_vm->_sound->pauseBackground();

	if (increase) {
		// Increase
		_vm->_gfx->copyImageSectionToScreen(11098, Common::Rect(72, 0, 84, 9), Common::Rect(387, 70, 399, 79));
		_observatoryIncrement = -1;
	} else {
		// Decrease
		_vm->_gfx->copyImageSectionToScreen(11097, Common::Rect(72, 0, 84, 9), Common::Rect(387, 204, 399, 213));
		_observatoryIncrement = 1;
	}

	// Highlight slider
	_observatoryYearSlider->drawConditionalDataToScreen(2);
	_observatoryCurrentSlider = _observatoryYearSlider;

	// First increment
	observatoryIncrementYear(_observatoryIncrement);

	// Start persistent script
	_startTime = _vm->getTotalPlayTime();
	_observatoryYearChanging = true;
}

void Myst::observatoryIncrementYear(int16 increment) {
	int16 newYear = _state.observatoryYearSetting + increment;

	if (newYear >= 0 && newYear <= 9999) {
		_state.observatoryYearSetting = newYear;

		// Redraw digits
		_vm->getCard()->redrawArea(79);
		_vm->getCard()->redrawArea(78);
		_vm->getCard()->redrawArea(77);
		_vm->getCard()->redrawArea(76);

		// Update slider
		_observatoryYearSlider->setPosition(94 + 94 * _state.observatoryYearSetting / 9999);
		_observatoryYearSlider->restoreBackground();
		_observatoryYearSlider->drawConditionalDataToScreen(2);
		_state.observatoryYearSlider = _observatoryYearSlider->_pos.y;
	}

	_vm->_sound->playEffect(8500);
	_vm->wait(20);
}

void Myst::observatoryYearChange_run() {
	if (_startTime + 500 < _vm->getTotalPlayTime())
		observatoryIncrementYear(_observatoryIncrement);
}

void Myst::o_observatoryTimeChangeStartIncrease(uint16 var, const ArgumentsArray &args) {
	observatoryTimeChangeStart(true);
}

void Myst::o_observatoryTimeChangeStartDecrease(uint16 var, const ArgumentsArray &args) {
	observatoryTimeChangeStart(false);
}

void Myst::observatoryTimeChangeStart(bool increase) {
	_vm->_sound->pauseBackground();

	if (increase) {
		// Increase
		_vm->_gfx->copyImageSectionToScreen(11098, Common::Rect(109, 0, 121, 9), Common::Rect(424, 70, 436, 79));
		_observatoryIncrement = -1;
	} else {
		// Decrease
		_vm->_gfx->copyImageSectionToScreen(11097, Common::Rect(109, 0, 121, 9), Common::Rect(424, 204, 436, 213));
		_observatoryIncrement = 1;
	}

	// Highlight slider
	_observatoryTimeSlider->drawConditionalDataToScreen(2);
	_observatoryCurrentSlider = _observatoryTimeSlider;

	// First increment
	observatoryIncrementTime(_observatoryIncrement);

	// Start persistent script
	_startTime = _vm->getTotalPlayTime();
	_observatoryTimeChanging = true;
}

void Myst::observatoryIncrementTime(int16 increment) {
	int16 newTime = _state.observatoryTimeSetting + increment;

	if (newTime >= 0 && newTime <= 1439) {
		_state.observatoryTimeSetting = newTime;

		// Redraw digits
		_vm->getCard()->redrawArea(80);
		_vm->getCard()->redrawArea(81);
		_vm->getCard()->redrawArea(82);
		_vm->getCard()->redrawArea(83);

		// Draw AM/PM
		if (!observatoryIsDDMMYYYY2400()) {
			_vm->getCard()->redrawArea(88);
		}

		// Update slider
		_observatoryTimeSlider->setPosition(94 + 94 * _state.observatoryTimeSetting / 1439);
		_observatoryTimeSlider->restoreBackground();
		_observatoryTimeSlider->drawConditionalDataToScreen(2);
		_state.observatoryTimeSlider = _observatoryTimeSlider->_pos.y;
	}

	_vm->_sound->playEffect(8500);
	_vm->wait(20);
}

void Myst::observatoryTimeChange_run() {
	if (_startTime + 500 < _vm->getTotalPlayTime())
		observatoryIncrementTime(_observatoryIncrement);
}

void Myst::o_observatoryGoButton(uint16 var, const ArgumentsArray &args) {
	// Setting not at target
	if (_state.observatoryDayTarget != _state.observatoryDaySetting
			|| _state.observatoryMonthTarget != _state.observatoryMonthSetting
			|| _state.observatoryYearTarget != _state.observatoryYearSetting
			|| _state.observatoryTimeTarget != _state.observatoryTimeSetting) {
		uint16 soundId = args[0];
		_vm->_sound->playEffect(soundId);

		int16 distance = _state.observatoryYearTarget - _state.observatoryYearSetting;
		uint32 end = _vm->getTotalPlayTime() + 32 * ABS(distance) / 50 + 800;

		while (end > _vm->getTotalPlayTime()) {
			_vm->wait(50);

			observatoryUpdateVisualizer(_vm->_rnd->getRandomNumber(406), _vm->_rnd->getRandomNumber(406));

			_vm->redrawResource(_observatoryVisualizer);
		}

		_vm->_sound->resumeBackground();

		// Redraw visualizer
		observatorySetTargetToSetting();
		_vm->redrawResource(_observatoryVisualizer);

		// Redraw button
		_tempVar = 0;
		_vm->getCard()->redrawArea(105);
	}
}

void Myst::o_observatoryMonthSliderMove(uint16 var, const ArgumentsArray &args) {
	observatoryUpdateMonth();
}

void Myst::o_observatoryDaySliderMove(uint16 var, const ArgumentsArray &args) {
	observatoryUpdateDay();
}

void Myst::o_observatoryYearSliderMove(uint16 var, const ArgumentsArray &args) {
	observatoryUpdateYear();
}

void Myst::o_observatoryTimeSliderMove(uint16 var, const ArgumentsArray &args) {
	observatoryUpdateTime();
}

void Myst::o_circuitBreakerStartMove(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *breaker = getInvokingResource<MystVideoInfo>();
	breaker->drawFrame(0);
	_vm->_cursor->setCursor(700);
	_tempVar = 0;
}

void Myst::o_circuitBreakerMove(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *breaker = getInvokingResource<MystVideoInfo>();
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();

	int16 maxStep = breaker->getStepsV() - 1;
	int16 step = ((mouse.y - 80) * breaker->getStepsV()) / 65;
	step = CLIP<uint16>(step, 0, maxStep);

	breaker->drawFrame(step);

	if (_tempVar != step) {
		_tempVar = step;

		// Breaker switched
		if (step == maxStep) {
			// Choose breaker
			if (breaker->getImageSwitchVar() == 93) {
				// Voltage is still too high or not broken
				if (_state.generatorVoltage > 59 || _state.generatorBreakers != 1) {
					uint16 soundId = breaker->getList2(1);
					if (soundId)
						_vm->_sound->playEffect(soundId);
				} else {
					uint16 soundId = breaker->getList2(0);
					if (soundId)
						_vm->_sound->playEffect(soundId);

					// Reset breaker state
					_state.generatorBreakers = 0;
				}
			} else {
				// Voltage is still too high or not broken
				if (_state.generatorVoltage > 59 || _state.generatorBreakers != 2) {
					uint16 soundId = breaker->getList2(1);
					if (soundId)
						_vm->_sound->playEffect(soundId);
				} else {
					uint16 soundId = breaker->getList2(0);
					if (soundId)
						_vm->_sound->playEffect(soundId);

					// Reset breaker state
					_state.generatorBreakers = 0;
				}
			}
		}
	}
}

void Myst::o_circuitBreakerEndMove(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *breaker = getInvokingResource<MystVideoInfo>();
	_vm->getCard()->redrawArea(breaker->getImageSwitchVar());
	_vm->refreshCursor();
}

void Myst::o_boilerIncreasePressureStart(uint16 var, const ArgumentsArray &args) {
	_treeStopped = true;
	if (_state.cabinValvePosition < 25)
		_vm->_sound->stopBackground();

	_boilerPressureIncreasing = true;
}

void Myst::o_boilerLightPilot(uint16 var, const ArgumentsArray &args) {
	// Match is lit
	if (_cabinMatchState == 1) {
		_state.cabinPilotLightLit = 1;
		_vm->getCard()->redrawArea(98);

		boilerFireUpdate(false);

		// Put out match
		_matchGoOutTime = _vm->getTotalPlayTime();

		if (_state.cabinValvePosition > 0)
			_vm->_sound->playBackground(8098, 49152);

		if (_state.cabinValvePosition > 12) {
			// Compute the speed of the gauge to synchronize it with the next tree move
			uint32 delay = treeNextMoveDelay(_state.cabinValvePosition);
			Common::Rational rate = boilerComputeGaugeRate(_state.cabinValvePosition, delay);
			boilerResetGauge(rate);

			_state.treeLastMoveTime = _vm->getTotalPlayTime();
		}
	}
}

Common::Rational Myst::boilerComputeGaugeRate(uint16 pressure, uint32 delay) {
	Common::Rational rate = Common::Rational(2088, delay);
	if (pressure < 12)
		return -rate;
	else
		return rate;
}

void Myst::boilerResetGauge(const Common::Rational &rate) {
	if (!_cabinGaugeMovie || _cabinGaugeMovie->endOfVideo()) {
		if (_vm->getCard()->getId() == 4098) {
			_cabinGaugeMovie = _vm->playMovie("cabingau", kMystStack);
			_cabinGaugeMovie->moveTo(243, 96);
		} else {
			_cabinGaugeMovie = _vm->playMovie("cabcgfar", kMystStack);
			_cabinGaugeMovie->moveTo(254, 136);
		}
	}

	Audio::Timestamp goTo;
	if (rate > 0)
		goTo = Audio::Timestamp(0, 0, 600);
	else
		goTo = _cabinGaugeMovie->getDuration();

	_cabinGaugeMovie->seek(goTo);
	_cabinGaugeMovie->setRate(rate);
}

void Myst::o_boilerIncreasePressureStop(uint16 var, const ArgumentsArray &args) {
	_treeStopped = false;
	_boilerPressureIncreasing = false;
	_state.treeLastMoveTime = _vm->getTotalPlayTime();

	while (_vm->_sound->isEffectPlaying()) {
		_vm->doFrame();
	}

	if (_state.cabinPilotLightLit == 1) {
		if (_state.cabinValvePosition > 0)
			_vm->_sound->playBackground(8098, 49152);

		if (_cabinGaugeMovie && !_cabinGaugeMovie->endOfVideo()) {
			uint16 delay = treeNextMoveDelay(_state.cabinValvePosition);
			Common::Rational rate = boilerComputeGaugeRate(_state.cabinValvePosition, delay);
			_cabinGaugeMovie->setRate(rate);
		}

	} else if (_state.cabinValvePosition > 0)
		_vm->_sound->playBackground(4098, _state.cabinValvePosition << 10);
}

void Myst::boilerPressureIncrease_run() {
	// Allow increasing pressure if sound has stopped
	if (!_vm->_sound->isEffectPlaying() && _state.cabinValvePosition < 25) {
		_state.cabinValvePosition++;
		if (_state.cabinValvePosition == 1) {
			// Set fire to high
			boilerFireUpdate(false);

			// Draw fire
			_vm->getCard()->redrawArea(305);
		} else if (_state.cabinValvePosition == 25) {
			if (_state.cabinPilotLightLit == 1)
				_vm->_sound->playBackground(8098, 49152);
			else
				_vm->_sound->playBackground(4098, 25600);
		}

		// Pressure increasing sound
		_vm->_sound->playEffect(5098);

		// Redraw wheel
		_vm->getCard()->redrawArea(99);
	}
}

void Myst::boilerPressureDecrease_run() {
	// Allow decreasing pressure if sound has stopped
	if (!_vm->_sound->isEffectPlaying() && _state.cabinValvePosition > 0) {
		_state.cabinValvePosition--;
		if (_state.cabinValvePosition == 0) {
			// Set fire to low
			boilerFireUpdate(false);

			// Draw fire
			_vm->getCard()->redrawArea(305);
		}

		// Pressure increasing sound
		_vm->_sound->playEffect(5098);

		// Redraw wheel
		_vm->getCard()->redrawArea(99);
	}
}

void Myst::o_boilerDecreasePressureStart(uint16 var, const ArgumentsArray &args) {
	_treeStopped = true;
	_vm->_sound->stopBackground();

	_boilerPressureDecreasing = true;
}

void Myst::o_boilerDecreasePressureStop(uint16 var, const ArgumentsArray &args) {
	_treeStopped = false;
	_boilerPressureDecreasing = false;
	_state.treeLastMoveTime = _vm->getTotalPlayTime();

	while (_vm->_sound->isEffectPlaying()) {
		_vm->doFrame();
	}

	if (_state.cabinPilotLightLit == 1) {
		if (_state.cabinValvePosition > 0)
			_vm->_sound->playBackground(8098, 49152);

		if (_cabinGaugeMovie && !_cabinGaugeMovie->endOfVideo()) {
			uint16 delay = treeNextMoveDelay(_state.cabinValvePosition);
			Common::Rational rate = boilerComputeGaugeRate(_state.cabinValvePosition, delay);
			_cabinGaugeMovie->setRate(rate);
		}

	} else {
		if (_state.cabinValvePosition > 0)
			_vm->_sound->playBackground(4098, _state.cabinValvePosition << 10);
	}
}

void Myst::o_basementIncreasePressureStart(uint16 var, const ArgumentsArray &args) {
	_treeStopped = true;
	_basementPressureIncreasing = true;
}

void Myst::o_basementIncreasePressureStop(uint16 var, const ArgumentsArray &args) {
	_treeStopped = false;
	_basementPressureIncreasing = false;
	_state.treeLastMoveTime = _vm->getTotalPlayTime();
}

void Myst::basementPressureIncrease_run() {
	// Allow increasing pressure if sound has stopped
	if (!_vm->_sound->isEffectPlaying() && _state.cabinValvePosition < 25) {
		_state.cabinValvePosition++;

		// Pressure increasing sound
		_vm->_sound->playEffect(4642);

		// Redraw wheel
		_vm->getCard()->redrawArea(99);
	}
}

void Myst::basementPressureDecrease_run() {
	// Allow decreasing pressure if sound has stopped
	if (!_vm->_sound->isEffectPlaying() && _state.cabinValvePosition > 0) {
		_state.cabinValvePosition--;

		// Pressure decreasing sound
		_vm->_sound->playEffect(4642);

		// Redraw wheel
		_vm->getCard()->redrawArea(99);
	}
}

void Myst::o_basementDecreasePressureStart(uint16 var, const ArgumentsArray &args) {
	_treeStopped = true;
	_basementPressureDecreasing = true;
}

void Myst::o_basementDecreasePressureStop(uint16 var, const ArgumentsArray &args) {
	_treeStopped = false;
	_basementPressureDecreasing = false;
	_state.treeLastMoveTime = _vm->getTotalPlayTime();
}

void Myst::tree_run() {
	uint16 pressure;
	if (_state.cabinPilotLightLit)
		pressure = _state.cabinValvePosition;
	else
		pressure = 0;

	// 12 means tree is balanced
	if (pressure != 12) {
		bool goingDown = true;
		if (pressure >= 12)
			goingDown = false;

		// Tree is within bounds
		if ((_state.treePosition < 12 && !goingDown)
				|| (_state.treePosition > _treeMinPosition && goingDown)) {
			uint16 delay = treeNextMoveDelay(pressure);
			uint32 time = _vm->getTotalPlayTime();
			if (delay < time - _state.treeLastMoveTime) {

				// Tree movement
				if (goingDown) {
					_state.treePosition--;
					_vm->_sound->playEffect(2);
				} else {
					_state.treePosition++;
					_vm->_sound->playEffect(1);
				}

				// Stop background music if going up from book room
				if (_vm->getCard()->getId() == 4630) {
					if (_state.treePosition > 0)
						_vm->_sound->stopBackground();
					else
						_vm->_sound->playBackground(4630, 24576);
				}

				// Redraw tree
				_vm->getCard()->redrawArea(72);

				// Check if alcove is accessible
				treeSetAlcoveAccessible();

				if (_cabinGaugeMovieEnabled) {
					Common::Rational rate = boilerComputeGaugeRate(pressure, delay);
					boilerResetGauge(rate);
				}

				_state.treeLastMoveTime = time;
			}
		}
	}
}

void Myst::treeSetAlcoveAccessible() {
	if (_treeAlcove) {
		// Make alcove accessible if the tree is in the correct position
		_treeAlcove->setEnabled(_state.treePosition >= _treeMinAccessiblePosition
					&& _state.treePosition <= _treeMaxAccessiblePosition);
	}
}

uint32 Myst::treeNextMoveDelay(uint16 pressure) {
	if (pressure >= 12)
		return 25000 * (13 - (pressure - 12)) / 12 + 3000;
	else
		return 25000 * pressure / 13 + 3000;
}

void Myst::o_rocketSoundSliderStartMove(uint16 var, const ArgumentsArray &args) {
	_rocketSliderSound = 0;
	_vm->_cursor->setCursor(700);
	_vm->_sound->pauseBackground();
	rocketSliderMove();
}

void Myst::o_rocketSoundSliderMove(uint16 var, const ArgumentsArray &args) {
	rocketSliderMove();
}

void Myst::o_rocketSoundSliderEndMove(uint16 var, const ArgumentsArray &args) {
	_vm->refreshCursor();

	if (_state.generatorVoltage == 59 && !_state.generatorBreakers && _rocketSliderSound)
		_vm->_sound->stopEffect();

	if (getInvokingResource<MystArea>() == _rocketSlider1)
		_state.rocketSliderPosition[0] = _rocketSlider1->_pos.y;
	else if (getInvokingResource<MystArea>() == _rocketSlider2)
		_state.rocketSliderPosition[1] = _rocketSlider2->_pos.y;
	else if (getInvokingResource<MystArea>() == _rocketSlider3)
		_state.rocketSliderPosition[2] = _rocketSlider3->_pos.y;
	else if (getInvokingResource<MystArea>() == _rocketSlider4)
		_state.rocketSliderPosition[3] = _rocketSlider4->_pos.y;
	else if (getInvokingResource<MystArea>() == _rocketSlider5)
		_state.rocketSliderPosition[4] = _rocketSlider5->_pos.y;

	_vm->_sound->resumeBackground();
}

void Myst::rocketSliderMove() {
	MystAreaSlider *slider = getInvokingResource<MystAreaSlider>();

	if (_state.generatorVoltage == 59 && !_state.generatorBreakers) {
		uint16 soundId = rocketSliderGetSound(slider->_pos.y);
		if (soundId != _rocketSliderSound) {
			_rocketSliderSound = soundId;
			_vm->_sound->playEffect(soundId, true);
		}
	}
}

uint16 Myst::rocketSliderGetSound(uint16 pos) {
	return (uint16)(9530 + (pos - 216) * 35.0 / 61.0);
}

void Myst::rocketCheckSolution() {
	_vm->_cursor->hideCursor();

	uint16 soundId;
	bool solved = true;

	soundId = rocketSliderGetSound(_rocketSlider1->_pos.y);
	_vm->_sound->playEffect(soundId);
	_rocketSlider1->drawConditionalDataToScreen(2);
	_vm->wait(250);
	if (soundId != 9558)
		solved = false;

	soundId = rocketSliderGetSound(_rocketSlider2->_pos.y);
	_vm->_sound->playEffect(soundId);
	_rocketSlider2->drawConditionalDataToScreen(2);
	_vm->wait(250);
	if (soundId != 9546)
		solved = false;

	soundId = rocketSliderGetSound(_rocketSlider3->_pos.y);
	_vm->_sound->playEffect(soundId);
	_rocketSlider3->drawConditionalDataToScreen(2);
	_vm->wait(250);
	if (soundId != 9543)
		solved = false;

	soundId = rocketSliderGetSound(_rocketSlider4->_pos.y);
	_vm->_sound->playEffect(soundId);
	_rocketSlider4->drawConditionalDataToScreen(2);
	_vm->wait(250);
	if (soundId != 9553)
		solved = false;

	soundId = rocketSliderGetSound(_rocketSlider5->_pos.y);
	_vm->_sound->playEffect(soundId);
	_rocketSlider5->drawConditionalDataToScreen(2);
	_vm->wait(250);
	if (soundId != 9560)
		solved = false;

	_vm->_sound->stopEffect();

	if (solved && !_rocketLinkBook) {
		// Reset lever position
		MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();
		lever->drawFrame(0);

		// Book appearing
		Common::String movieFile = "selenbok";
		_rocketLinkBook = _vm->playMovie(movieFile, kMystStack);
		_rocketLinkBook->moveTo(224, 41);
		_rocketLinkBook->setBounds(Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 660, 600));

		_vm->waitUntilMovieEnds(_rocketLinkBook);

		// Book looping closed
		_rocketLinkBook = _vm->playMovie(movieFile, kMystStack);
		_rocketLinkBook->moveTo(224, 41);
		_rocketLinkBook->setLooping(true);
		_rocketLinkBook->setBounds(Audio::Timestamp(0, 660, 600), Audio::Timestamp(0, 3500, 600));

		_tempVar = 1;
	}

	_rocketSlider1->drawConditionalDataToScreen(1);
	_rocketSlider2->drawConditionalDataToScreen(1);
	_rocketSlider3->drawConditionalDataToScreen(1);
	_rocketSlider4->drawConditionalDataToScreen(1);
	_rocketSlider5->drawConditionalDataToScreen(1);

	_vm->_cursor->showCursor();
}

void Myst::o_rocketPianoStart(uint16 var, const ArgumentsArray &args) {
	MystAreaDrag *key = getInvokingResource<MystAreaDrag>();

	// What the hell??
	Common::Rect src = key->getSubImage(1).rect;
	Common::Rect rect = key->getSubImage(0).rect;
	Common::Rect dest = rect;
	dest.top = 332 - rect.bottom;
	dest.bottom = 332 - rect.top;

	// Draw pressed piano key
	_vm->_gfx->copyImageSectionToScreen(key->getSubImage(1).wdib, src, dest);

	// Play note
	_rocketPianoSound = 0;
	if (_state.generatorVoltage == 59 && !_state.generatorBreakers) {
		_vm->_sound->pauseBackground();
		_rocketPianoSound = key->getList1(0);
		_vm->_sound->playEffect(_rocketPianoSound, true);
	}
}

void Myst::o_rocketPianoMove(uint16 var, const ArgumentsArray &args) {
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();
	Common::Rect piano = Common::Rect(85, 123, 460, 270);

	// Unpress previous key
	MystAreaDrag *key = getInvokingResource<MystAreaDrag>();

	Common::Rect src = key->getSubImage(0).rect;
	Common::Rect dest = src;
	dest.top = 332 - src.bottom;
	dest.bottom = 332 - src.top;

	// Draw unpressed piano key
	_vm->_gfx->copyImageSectionToScreen(key->getSubImage(0).wdib, src, dest);

	if (piano.contains(mouse)) {
		MystArea *resource = _vm->getCard()->forceUpdateClickedResource(mouse);
		if (resource && resource->hasType(kMystAreaDrag)) {
			// Press new key
			key = static_cast<MystAreaDrag *>(resource);
			src = key->getSubImage(1).rect;
			Common::Rect rect = key->getSubImage(0).rect;
			dest = rect;
			dest.top = 332 - rect.bottom;
			dest.bottom = 332 - rect.top;

			// Draw pressed piano key
			_vm->_gfx->copyImageSectionToScreen(key->getSubImage(1).wdib, src, dest);

			// Play note
			if (_state.generatorVoltage == 59 && !_state.generatorBreakers) {
				uint16 soundId = key->getList1(0);
				if (soundId != _rocketPianoSound) {
					_rocketPianoSound = soundId;
					_vm->_sound->playEffect(soundId, true);
				}
			}
		} else {
			// Not pressing a key anymore
			_vm->_sound->stopEffect();
			_vm->_sound->resumeBackground();
		}
	}
}

void Myst::o_rocketPianoStop(uint16 var, const ArgumentsArray &args) {
	MystAreaImageSwitch *key = getInvokingResource<MystAreaImageSwitch>();

	Common::Rect src = key->getSubImage(0).rect;
	Common::Rect dest = src;
	dest.top = 332 - src.bottom;
	dest.bottom = 332 - src.top;

	// Draw unpressed piano key
	_vm->_gfx->copyImageSectionToScreen(key->getSubImage(0).wdib, src, dest);

	_vm->_sound->stopEffect();
	_vm->_sound->resumeBackground();
}

void Myst::o_rocketLeverStartMove(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();

	_vm->_cursor->setCursor(700);
	_rocketLeverPosition = 0;
	lever->drawFrame(0);
}

void Myst::o_rocketOpenBook(uint16 var, const ArgumentsArray &args) {
	// Flyby movie
	_rocketLinkBook->setBounds(Audio::Timestamp(0, 3500, 600), Audio::Timestamp(0, 13100, 600));

	// Set linkable
	_tempVar = 2;
}

void Myst::o_rocketLeverMove(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();
	const Common::Point &mouse = _vm->_system->getEventManager()->getMousePos();

	// Make the lever follow the mouse
	int16 maxStep = lever->getStepsV() - 1;
	Common::Rect rect = lever->getRect();
	int16 step = ((mouse.y - rect.top) * lever->getStepsV()) / rect.height();
	step = CLIP<uint16>(step, 0, maxStep);

	lever->drawFrame(step);

	// If lever pulled
	if (step == maxStep && step != _rocketLeverPosition) {
		uint16 soundId = lever->getList2(0);

		if (soundId)
			_vm->playSoundBlocking(soundId);

		// If rocket correctly powered
		if (_state.generatorVoltage == 59 && !_state.generatorBreakers)
			rocketCheckSolution();
	}

	_rocketLeverPosition = step;
}

void Myst::o_rocketLeverEndMove(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();

	_vm->refreshCursor();
	_rocketLeverPosition = 0;
	lever->drawFrame(0);
}

void Myst::o_cabinLeave(uint16 var, const ArgumentsArray &args) {
	// If match is lit, put out
	if (_cabinMatchState == 1) {
		_matchGoOutTime = _vm->getTotalPlayTime();
	} else if (_cabinMatchState == 0) {
		_vm->setMainCursor(_savedCursorId);
		_cabinMatchState = 2;
	}
}

void Myst::o_treePressureReleaseStart(uint16 var, const ArgumentsArray &args) {
	Common::Rect src = Common::Rect(0, 0, 49, 86);
	Common::Rect dest = Common::Rect(78, 46, 127, 132);
	_vm->_gfx->copyImageSectionToScreen(4631, src, dest);

	_tempVar = _state.cabinValvePosition;

	if (_state.treePosition >= 4) {
		_state.cabinValvePosition = 0;
		_treeMinPosition = 4;
		_state.treeLastMoveTime = 0;
	}
}

void Myst::o_treePressureReleaseStop(uint16 var, const ArgumentsArray &args) {
	Common::Rect rect = Common::Rect(78, 46, 127, 132);
	_vm->_gfx->copyBackBufferToScreen(rect);

	_state.cabinValvePosition = _tempVar;
	_treeMinPosition = 0;
}

void Myst::o_observatoryMonthSliderStartMove(uint16 var, const ArgumentsArray &args) {
	_vm->_cursor->setCursor(700);
	_vm->_sound->pauseBackground();

	observatoryUpdateMonth();
}

void Myst::o_observatoryMonthSliderEndMove(uint16 var, const ArgumentsArray &args) {
	_vm->refreshCursor();
	_vm->_sound->resumeBackground();

	observatoryUpdateMonth();
}

void Myst::observatoryUpdateMonth() {
	int16 month = (_observatoryMonthSlider->_pos.y - 94) / 8;

	if (month != _state.observatoryMonthSetting) {
		_state.observatoryMonthSetting = month;
		_state.observatoryMonthSlider = _observatoryMonthSlider->_pos.y;
		_vm->_sound->playEffect(8500);
		_vm->wait(20);

		// Redraw digits
		_vm->getCard()->redrawArea(73);
	}
}

void Myst::o_observatoryDaySliderStartMove(uint16 var, const ArgumentsArray &args) {
	_vm->_cursor->setCursor(700);
	_vm->_sound->pauseBackground();

	observatoryUpdateDay();
}

void Myst::o_observatoryDaySliderEndMove(uint16 var, const ArgumentsArray &args) {
	_vm->refreshCursor();
	_vm->_sound->resumeBackground();

	observatoryUpdateDay();
}

void Myst::observatoryUpdateDay() {
	int16 day = (_observatoryDaySlider->_pos.y - 94) * 30 / 94 + 1;

	if (day != _state.observatoryDaySetting) {
		_state.observatoryDaySetting = day;
		_state.observatoryDaySlider = _observatoryDaySlider->_pos.y;
		_vm->_sound->playEffect(8500);
		_vm->wait(20);

		// Redraw digits
		_vm->getCard()->redrawArea(75);
		_vm->getCard()->redrawArea(74);
	}
}

void Myst::o_observatoryYearSliderStartMove(uint16 var, const ArgumentsArray &args) {
	_vm->_cursor->setCursor(700);
	_vm->_sound->pauseBackground();

	observatoryUpdateYear();
}

void Myst::o_observatoryYearSliderEndMove(uint16 var, const ArgumentsArray &args) {
	_vm->refreshCursor();
	_vm->_sound->resumeBackground();

	observatoryUpdateYear();
}

void Myst::observatoryUpdateYear() {
	int16 year = (_observatoryYearSlider->_pos.y - 94) * 9999 / 94;

	if (year != _state.observatoryYearSetting) {
		_state.observatoryYearSetting = year;
		_state.observatoryYearSlider = _observatoryYearSlider->_pos.y;
		_vm->_sound->playEffect(8500);
		_vm->wait(20);

		// Redraw digits
		_vm->getCard()->redrawArea(79);
		_vm->getCard()->redrawArea(78);
		_vm->getCard()->redrawArea(77);
		_vm->getCard()->redrawArea(76);
	}
}

void Myst::o_observatoryTimeSliderStartMove(uint16 var, const ArgumentsArray &args) {
	_vm->_cursor->setCursor(700);
	_vm->_sound->pauseBackground();

	observatoryUpdateTime();
}

void Myst::o_observatoryTimeSliderEndMove(uint16 var, const ArgumentsArray &args) {
	_vm->refreshCursor();
	_vm->_sound->resumeBackground();

	observatoryUpdateTime();
}

void Myst::observatoryUpdateTime() {
	int16 time = (_observatoryTimeSlider->_pos.y - 94) * 1439 / 94;

	if (time != _state.observatoryTimeSetting) {
		_state.observatoryTimeSetting = time;
		_state.observatoryTimeSlider = _observatoryTimeSlider->_pos.y;
		_vm->_sound->playEffect(8500);
		_vm->wait(20);

		// Redraw digits
		_vm->getCard()->redrawArea(80);
		_vm->getCard()->redrawArea(81);
		_vm->getCard()->redrawArea(82);
		_vm->getCard()->redrawArea(83);

		// Draw AM/PM
		if (!observatoryIsDDMMYYYY2400())
			_vm->getCard()->redrawArea(88);
	}
}

void Myst::o_libraryBookPageTurnStartLeft(uint16 var, const ArgumentsArray &args) {
	_tempVar = -1;
	libraryBookPageTurnLeft();
	_startTime = _vm->getTotalPlayTime();
	_libraryBookPagesTurning = true;
}

void Myst::o_libraryBookPageTurnStartRight(uint16 var, const ArgumentsArray &args) {
	_tempVar = 1;
	libraryBookPageTurnRight();
	_startTime = _vm->getTotalPlayTime();
	_libraryBookPagesTurning = true;
}

void Myst::libraryBook_run() {
	uint32 time = _vm->getTotalPlayTime();
	if (time >= _startTime + 500) {
		if (_tempVar > 0) {
			libraryBookPageTurnRight();
			_startTime = time;
		} else if (_tempVar < 0) {
			libraryBookPageTurnLeft();
			_startTime = time;
		}
	}
}

void Myst::o_libraryCombinationBookStop(uint16 var, const ArgumentsArray &args) {
	_libraryBookPagesTurning = false;
	_libraryCombinationBookPagesTurning = false;
}

void Myst::o_cabinMatchLight(uint16 var, const ArgumentsArray &args) {
	if (!_cabinMatchState) {
		_vm->_sound->playEffect(4103);

		// Match is lit
		_cabinMatchState = 1;
		_matchBurning = true;
		_matchGoOutCnt = 0;
		_vm->setMainCursor(kLitMatchCursor);

		// Match will stay lit for one minute
		_matchGoOutTime = _vm->getTotalPlayTime() + 60 * 1000;
	}
}

void Myst::matchBurn_run() {
	uint32 time = _vm->getTotalPlayTime();

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

void Myst::o_courtyardBoxEnter(uint16 var, const ArgumentsArray &args) {
	_tempVar = 1;
	_vm->_sound->playEffect(_courtyardBoxSound);
	_vm->getCard()->redrawArea(var);
}

void Myst::o_courtyardBoxLeave(uint16 var, const ArgumentsArray &args) {
	_tempVar = 0;
	_vm->getCard()->redrawArea(var);
}

void Myst::o_clockMinuteWheelStartTurn(uint16 var, const ArgumentsArray &args) {
	// Used on Card 4006
	clockWheelStartTurn(2);
}

void Myst::o_clockWheelEndTurn(uint16 var, const ArgumentsArray &args) {
	// Used on Card 4006
	_clockTurningWheel = 0;
}

void Myst::o_clockHourWheelStartTurn(uint16 var, const ArgumentsArray &args) {
	// Used on Card 4006
	clockWheelStartTurn(1);
}

void Myst::clockWheel_run() {
	// Turn wheel one step each second
	uint32 time = _vm->getTotalPlayTime();

	if (time > _startTime + 1000) {
		_startTime = time;

		if (_clockTurningWheel == 1)
			clockWheelTurn(39);
		else
			clockWheelTurn(38);

		_vm->getCard()->redrawArea(37);
	}
}

void Myst::clockWheelStartTurn(uint16 wheel) {
	MystAreaDrag *resource = getInvokingResource<MystAreaDrag>();
	uint16 soundId = resource->getList1(0);

	if (soundId)
		_vm->_sound->playEffect(soundId);

	// Turn wheel one step
	if (wheel == 1)
		clockWheelTurn(39);
	else
		clockWheelTurn(38);

	_vm->getCard()->redrawArea(37);

	// Continue turning wheel until mouse button is released
	_clockTurningWheel = wheel;
	_startTime = _vm->getTotalPlayTime();
}

void Myst::clockWheelTurn(uint16 var) {
	if (var == 38) {
		// Hours
		_state.clockTowerHourPosition = (_state.clockTowerHourPosition + 1) % 12;
	} else {
		// Minutes
		_state.clockTowerMinutePosition = (_state.clockTowerMinutePosition + 5) % 60;
	}
}

void Myst::o_libraryCombinationBookStartRight(uint16 var, const ArgumentsArray &args) {
	_tempVar = 0;
	libraryCombinationBookTurnRight();
	_startTime = _vm->getTotalPlayTime();
	_libraryCombinationBookPagesTurning = true;
}

void Myst::o_libraryCombinationBookStartLeft(uint16 var, const ArgumentsArray &args) {
	_tempVar = 0;
	libraryCombinationBookTurnLeft();
	_startTime = _vm->getTotalPlayTime();
	_libraryCombinationBookPagesTurning = true;
}

void Myst::libraryCombinationBookTurnLeft() {
	// Turn page left
	if (_libraryBookPage - 1 >= 0) {
		_tempVar--;

		if (_tempVar >= -5) {
			_libraryBookPage--;
		} else {
			_libraryBookPage -= 5;
			_tempVar = -5;
		}

		_libraryBookPage = CLIP<int16>(_libraryBookPage, 0, _libraryBookNumPages - 1);

		Common::Rect rect = Common::Rect(157, 113, 446, 220);
		_vm->_gfx->copyImageToScreen(_libraryBookBaseImage + _libraryBookPage, rect);

		if (_vm->_rnd->getRandomBit())
			_vm->_sound->playEffect(_libraryBookSound1);
		else
			_vm->_sound->playEffect(_libraryBookSound2);
	}
}

void Myst::libraryCombinationBookTurnRight() {
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

		Common::Rect rect = Common::Rect(157, 113, 446, 220);
		_vm->_gfx->copyImageToScreen(_libraryBookBaseImage + _libraryBookPage, rect);

		if (_vm->_rnd->getRandomBit())
			_vm->_sound->playEffect(_libraryBookSound1);
		else
			_vm->_sound->playEffect(_libraryBookSound2);
	}
}

void Myst::libraryCombinationBook_run() {
	uint32 time = _vm->getTotalPlayTime();
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

void Myst::o_observatoryChangeSettingStop(uint16 var, const ArgumentsArray &args) {
	// Stop persistent scripts
	_observatoryMonthChanging = false;
	_observatoryDayChanging = false;
	_observatoryYearChanging = false;
	_observatoryTimeChanging = false;
	_observatoryIncrement = 0;

	// Restore button and slider
	_vm->_gfx->copyBackBufferToScreen(getInvokingResource<MystArea>()->getRect());
	if (_observatoryCurrentSlider) {
		_vm->redrawResource(_observatoryCurrentSlider);
		_observatoryCurrentSlider = nullptr;
	}
	_vm->_sound->resumeBackground();
}

void Myst::o_dockVaultForceClose(uint16 var, const ArgumentsArray &args) {
	// Used on Myst 4143 (Dock near Marker Switch)
	uint16 soundId = args[0];
	uint16 delay = args[1];
	uint16 directionalUpdateDataSize = args[2];

	if (_dockVaultState) {
		// Open switch
		_state.dockMarkerSwitch = 1;
		_vm->_sound->playEffect(4143);
		_vm->getCard()->redrawArea(4);

		// Close vault
		_dockVaultState = 0;
		_vm->_sound->playEffect(soundId);
		_vm->getCard()->redrawArea(41, false);
		animatedUpdate(ArgumentsArray(args.begin() + 3, directionalUpdateDataSize), delay);
	}
}

void Myst::o_imagerEraseStop(uint16 var, const ArgumentsArray &args) {
	_imagerValidationRunning = false;
}

void Myst::o_clockLeverStartMove(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();
	lever->drawFrame(0);
	_vm->_cursor->setCursor(700);
	_clockMiddleGearMovedAlone = false;
	_clockLeverPulled = false;
}

void Myst::o_clockLeverMoveLeft(uint16 var, const ArgumentsArray &args) {
	clockLeverMove(true);
}

void Myst::o_clockLeverMoveRight(uint16 var, const ArgumentsArray &args) {
	clockLeverMove(false);
}

void Myst::clockLeverMove(bool leftLever) {
	if (!_clockLeverPulled) {
		MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();

		// If lever pulled
		if (lever->pullLeverV()) {
			// Start videos for first step
			if (_clockWeightPosition < 2214) {
				_vm->_sound->playEffect(5113);
				clockGearForwardOneStep(1);

				// Left lever
				if (leftLever)
					clockGearForwardOneStep(2);
				else // Right lever
					clockGearForwardOneStep(0);

				clockWeightDownOneStep();
			}
			_clockLeverPulled = true;
		}
	}
}

void Myst::clockGearForwardOneStep(uint16 gear) {
	static const uint16 startTime[] = { 0, 324, 618 };
	static const uint16 endTime[] = { 324, 618, 950 };
	static const char *videos[] = { "cl1wg1", "cl1wg2", "cl1wg3" };
	static const uint16 x[] = { 224, 224, 224 };
	static const uint16 y[] = { 49, 82, 109 };

	// Increment value by one
	_clockGearsPositions[gear] = _clockGearsPositions[gear] % 3 + 1;

	// Set video bounds
	uint16 gearPosition = _clockGearsPositions[gear] - 1;
	_clockGearsVideos[gear] = _vm->playMovie(videos[gear], kMystStack);
	_clockGearsVideos[gear]->moveTo(x[gear], y[gear]);
	_clockGearsVideos[gear]->setBounds(
			Audio::Timestamp(0, startTime[gearPosition], 600),
			Audio::Timestamp(0, endTime[gearPosition], 600));
}

void Myst::clockWeightDownOneStep() {
	// The Myst ME version of this video is encoded faster than the original
	// The weight goes on the floor one step too early. Original ME engine also has this behavior.
	bool updateVideo = !_vm->isGameVariant(GF_ME) || _clockWeightPosition < (2214 - 246);

	// Set video bounds
	if (updateVideo) {
		_clockWeightVideo = _vm->playMovie("cl1wlfch", kMystStack);
		_clockWeightVideo->moveTo(124, 0);
		_clockWeightVideo->setBounds(
				Audio::Timestamp(0, _clockWeightPosition, 600),
				Audio::Timestamp(0, _clockWeightPosition + 246, 600));
	}

	// Increment value by one step
	_clockWeightPosition += 246;
}

void Myst::clockGears_run() {
	if (!_vm->_video->isVideoPlaying() && _clockWeightPosition < 2214) {
		_clockMiddleGearMovedAlone = true;
		_vm->_sound->playEffect(5113);
		clockGearForwardOneStep(1);
		clockWeightDownOneStep();
	}
}

void Myst::o_clockLeverEndMove(uint16 var, const ArgumentsArray &args) {
	static const char *videos[] = { "cl1wg1", "cl1wg2", "cl1wg3", "cl1wlfch" };

	_vm->_cursor->hideCursor();
	_clockLeverPulled = false;

	// Let movies stop playing
	for (uint i = 0; i < ARRAYSIZE(videos); i++) {
		VideoEntryPtr handle = _vm->findVideo(videos[i], kMystStack);
		if (handle)
			_vm->waitUntilMovieEnds(handle);
	}

	if (_clockMiddleGearMovedAlone)
		_vm->_sound->playEffect(8113);

	// Release lever
	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();
	lever->releaseLeverV();

	// Check if puzzle is solved
	clockGearsCheckSolution();

	_vm->_cursor->showCursor();
}

void Myst::clockGearsCheckSolution() {
	if (_clockGearsPositions[0] == 2
			&& _clockGearsPositions[1] == 2
			&& _clockGearsPositions[2] == 1
			&& !_state.gearsOpen) {

		// Make weight go down
		_vm->_sound->playEffect(9113);
		_clockWeightVideo = _vm->playMovie("cl1wlfch", kMystStack);
		_clockWeightVideo->moveTo(124, 0);
		_clockWeightVideo->setBounds(
				Audio::Timestamp(0, _clockWeightPosition, 600),
				Audio::Timestamp(0, 2214, 600));

		_vm->waitUntilMovieEnds(_clockWeightVideo);
		_clockWeightPosition = 2214;

		_vm->_sound->playEffect(6113);
		_vm->wait(1000);
		_vm->_sound->playEffect(7113);

		// Gear opening video
		_vm->playMovieBlocking("cl1wggat", kMystStack, 195, 225);
		_state.gearsOpen = 1;
		_vm->getCard()->redrawArea(40);

		_vm->_sound->playBackground(4113, 16384);
	}
}

void Myst::o_clockResetLeverStartMove(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();
	lever->drawFrame(0);
	_vm->_cursor->setCursor(700);
}

void Myst::o_clockResetLeverMove(uint16 var, const ArgumentsArray &args) {
	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();

	// If pulled
	if (lever->pullLeverV() && _clockWeightPosition != 0)
		clockReset();
}

void Myst::clockReset() {
	static const char *videos[] = { "cl1wg1", "cl1wg2", "cl1wg3", "cl1wlfch" };

	_vm->_cursor->hideCursor();

	_vm->_sound->stopBackground();
	_vm->_sound->playEffect(5113);

	// Play reset videos
	clockResetWeight();
	clockResetGear(0);
	clockResetGear(1);
	clockResetGear(2);

	// Let movies stop playing
	for (uint i = 0; i < ARRAYSIZE(videos); i++) {
		VideoEntryPtr handle = _vm->findVideo(videos[i], kMystStack);
		if (handle)
			_vm->waitUntilMovieEnds(handle);
	}

	_vm->_sound->playEffect(10113);

	// Close gear
	if (_state.gearsOpen) {
		_vm->_sound->playEffect(6113);
		_vm->wait(1000);
		_vm->_sound->playEffect(7113);

		// Gear closing movie
		VideoEntryPtr handle = _vm->playMovie("cl1wggat", kMystStack);
		handle->moveTo(195, 225);
		handle->seek(handle->getDuration());
		handle->setRate(-1);
		_vm->waitUntilMovieEnds(handle);

		// Redraw gear
		_state.gearsOpen = 0;
		_vm->getCard()->redrawArea(40);
	}

	_vm->_cursor->showCursor();
}

void Myst::clockResetWeight() {
	_vm->_sound->playEffect(9113);

	_clockWeightVideo = _vm->playMovie("cl1wlfch", kMystStack);
	_clockWeightVideo->moveTo(124, 0);

	// Play the movie backwards, weight going up
	_clockWeightVideo->seek(Audio::Timestamp(0, _clockWeightPosition, 600));
	_clockWeightVideo->setRate(-1);

	// Reset position
	_clockWeightPosition = 0;
}

void Myst::clockResetGear(uint16 gear) {
	static const uint16 time[] = { 324, 618, 950 };
	static const char *videos[] = { "cl1wg1", "cl1wg2", "cl1wg3" };
	static const uint16 x[] = { 224, 224, 224 };
	static const uint16 y[] = { 49, 82, 109 };

	// Set video bounds, gears going to 3
	uint16 gearPosition = _clockGearsPositions[gear] - 1;
	if (gearPosition != 2) {
		_clockGearsVideos[gear] = _vm->playMovie(videos[gear], kMystStack);
		_clockGearsVideos[gear]->moveTo(x[gear], y[gear]);
		_clockGearsVideos[gear]->setBounds(
				Audio::Timestamp(0, time[gearPosition], 600),
				Audio::Timestamp(0, time[2], 600));
	}

	// Reset gear position
	_clockGearsPositions[gear] = 3;
}

void Myst::o_clockResetLeverEndMove(uint16 var, const ArgumentsArray &args) {
	// Get current lever frame
	MystVideoInfo *lever = getInvokingResource<MystVideoInfo>();

	lever->releaseLeverV();

	_vm->refreshCursor();
}

void Myst::o_libraryBook_init(uint16 var, const ArgumentsArray &args) {
	_libraryBookPage = 0;
	_libraryBookNumPages = args[0];
	_libraryBookBaseImage = args[1];
	_libraryBookSound1 = args[2];
	_libraryBookSound2 = args[3];
}

void Myst::o_courtyardBox_init(uint16 var, const ArgumentsArray &args) {
	_courtyardBoxSound = args[0];
}

void Myst::towerRotationMap_run() {
	if (!_towerRotationMapInitialized) {
		_towerRotationMapInitialized = true;
		_vm->_sound->playEffect(4378);

		towerRotationDrawBuildings();

		// Draw to screen
		_vm->_gfx->copyBackBufferToScreen(Common::Rect(106, 42, 459, 273));
	}

	uint32 time = _vm->getTotalPlayTime();
	if (time > _startTime) {
		if (_towerRotationMapClicked) {
			towerRotationMapRotate();
			_startTime = time + 100;
		} else if (_towerRotationBlinkLabel
				&& _vm->_sound->isEffectPlaying()) {
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
			towerRotationMapRedraw();

			// Blink tower
			_startTime = time + 500;
			_tempVar = (_tempVar + 1) % 2;
			_towerRotationMapTower->drawConditionalDataToScreen(_tempVar);
		}
	}
}

void Myst::o_towerRotationMap_init(uint16 var, const ArgumentsArray &args) {
	_towerRotationMapRunning = true;
	_towerRotationMapTower = getInvokingResource<MystAreaImageSwitch>();
	_towerRotationMapLabel = _vm->getCard()->getResource<MystAreaImageSwitch>(args[0]);
	_tempVar = 0;
	_startTime = 0;
	_towerRotationMapClicked = false;
}

void Myst::towerRotationDrawBuildings() {
	// Draw library
	_vm->getCard()->redrawArea(304, false);

	// Draw other resources
	for (uint i = 1; i <= 10; i++) {
		MystAreaImageSwitch *resource = _vm->getCard()->getResource<MystAreaImageSwitch>(i);
		_vm->redrawResource(resource, false);
	}
}

uint16 Myst::towerRotationMapComputeAngle() {
	_towerRotationSpeed++;
	if (_towerRotationSpeed >= 7)
		_towerRotationSpeed = 7;
	else
		_towerRotationSpeed++;

	_state.towerRotationAngle = (_state.towerRotationAngle + _towerRotationSpeed) % 360;
	uint16 angle = _state.towerRotationAngle;
	_towerRotationOverSpot = false;

	if (angle >= 265 && angle <= 277
			&& _state.rocketshipMarkerSwitch) {
		angle = 271;
		_towerRotationOverSpot = true;
		_towerRotationSpeed = 1;
	} else if (angle >= 77 && angle <= 89
			&& _state.gearsMarkerSwitch) {
		angle = 83;
		_towerRotationOverSpot = true;
		_towerRotationSpeed = 1;
	} else if (angle >= 123 && angle <= 135
			&& _state.dockMarkerSwitch) {
		angle = 129;
		_towerRotationOverSpot = true;
		_towerRotationSpeed = 1;
	} else if (angle >= 146 && angle <= 158
			&& _state.cabinMarkerSwitch) {
		angle = 152;
		_towerRotationOverSpot = true;
		_towerRotationSpeed = 1;
	}

	return angle;
}

Common::Point Myst::towerRotationMapComputeCoords(uint16 angle) {
	Common::Point end;

	// Polar to rect coords
	float radians = Common::deg2rad<uint16,float>(angle);
	end.x = (int16)(_towerRotationCenter.x + cos(radians) * 310.0f);
	end.y = (int16)(_towerRotationCenter.y + sin(radians) * 310.0f);

	return end;
}

void Myst::towerRotationMapDrawLine(const Common::Point &end, bool rotationLabelVisible) {
	uint32 color;

	if (_vm->isGameVariant(GF_ME)) {
		Graphics::PixelFormat pf = _vm->_system->getScreenFormat();

		if (!_towerRotationOverSpot)
			color = pf.RGBToColor(0xFF, 0xFF, 0xFF); // White
		else
			color = pf.RGBToColor(0xFF, 0, 0); // Red
	} else {
		if (!_towerRotationOverSpot)
			color = 0xFF; // White
		else
			color = 0xF9; // Red
	}

	const Common::Rect rect = Common::Rect(106, 42, 459, 273);

	Common::Rect src;
	src.left = rect.left;
	src.top = 332 - rect.bottom;
	src.right = rect.right;
	src.bottom = 332 - rect.top;

	// Redraw background
	_vm->_gfx->copyImageSectionToBackBuffer(_vm->getCard()->getBackgroundImageId(), src, rect);

	// Draw buildings
	towerRotationDrawBuildings();

	// Draw tower
	_towerRotationMapTower->drawConditionalDataToScreen(0, false);

	// Draw label
	_towerRotationMapLabel->drawConditionalDataToScreen(rotationLabelVisible ? 1 : 0, false);

	// Draw line
	_vm->_gfx->drawLine(_towerRotationCenter, end, color);
	_vm->_gfx->copyBackBufferToScreen(rect);
}

void Myst::towerRotationMapRotate() {
	uint16 angle = towerRotationMapComputeAngle();
	Common::Point end = towerRotationMapComputeCoords(angle);
	towerRotationMapDrawLine(end, true);
}

void Myst::towerRotationMapRedraw() {
	Common::Point end = towerRotationMapComputeCoords(_state.towerRotationAngle);
	towerRotationMapDrawLine(end, false);
}

void Myst::o_forechamberDoor_init(uint16 var, const ArgumentsArray &args) {
	// Used for Card 4138 (Dock Forechamber Door)
	// Set forechamber door to closed
	_tempVar = 0;
}

void Myst::o_shipAccess_init(uint16 var, const ArgumentsArray &args) {
	// Enable acces to the ship
	if (_state.shipFloating) {
		getInvokingResource<MystArea>()->setEnabled(true);
	}
}

void Myst::o_butterflies_init(uint16 var, const ArgumentsArray &args) {
	// Used for Card 4256 (Butterfly Movie Activation)
	if (!_butterfliesMoviePlayed) {
		MystAreaVideo *butterflies = getInvokingResource<MystAreaVideo>();
		butterflies->playMovie();

		_butterfliesMoviePlayed = true;
	}
}

void Myst::o_imager_init(uint16 var, const ArgumentsArray &args) {
	MystAreaActionSwitch *select = getInvokingResource<MystAreaActionSwitch>();
	_imagerMovie = static_cast<MystAreaVideo *>(select->getSubResource(getVar(var)));
	_imagerRunning = true;
}

void Myst::imager_run() {
	_imagerRunning = false;

	if (_state.imagerActive && _state.imagerSelection == 67) {
		VideoEntryPtr water = _imagerMovie->playMovie();
		water->setBounds(Audio::Timestamp(0, 1814, 600), Audio::Timestamp(0, 4204, 600));
		water->setLooping(true);
	}
}

void Myst::libraryBookcaseTransform_run(void) {
	if (_libraryBookcaseChanged) {
		_libraryBookcaseChanged = false;
		_libraryBookcaseMoving = false;

		_vm->_cursor->hideCursor();

		// Play transform sound and video
		_vm->_sound->playEffect(_libraryBookcaseSoundId);
		_libraryBookcaseMovie->playMovie();

		if (_state.libraryBookcaseDoor) {
			_vm->_gfx->copyImageSectionToBackBuffer(11179, Common::Rect(0, 0, 106, 81), Common::Rect(0, 72, 106, 153));
			_vm->_gfx->runTransition(kTransitionBottomToTop, Common::Rect(0, 72, 106, 153), 5, 10);
			_vm->playSoundBlocking(7348);
			_vm->_sound->playBackground(4348, 16384);
		} else {
			_vm->_gfx->copyImageSectionToBackBuffer(11178, Common::Rect(0, 0, 107, 67), Common::Rect(437, 84, 544, 151));
			_vm->_gfx->copyBackBufferToScreen(Common::Rect(437, 84, 544, 151));
			_vm->playSoundBlocking(7348);
			_vm->_sound->playBackground(4334, 16384);
		}

		_vm->_cursor->showCursor();
	}
}

void Myst::o_libraryBookcaseTransform_init(uint16 var, const ArgumentsArray &args) {
	if (_libraryBookcaseChanged) {
		MystAreaActionSwitch *resource = getInvokingResource<MystAreaActionSwitch>();
		_libraryBookcaseMovie = static_cast<MystAreaVideo *>(resource->getSubResource(getVar(0)));
		_libraryBookcaseSoundId = args[0];
		_libraryBookcaseMoving = true;
	}
}

void Myst::generatorControlRoom_run(void) {
	if (_generatorVoltage == _state.generatorVoltage) {
		generatorRedrawRocket();
	} else {
		// Animate generator gauge
		if (_generatorVoltage > _state.generatorVoltage)
			_generatorVoltage--;
		else
			_generatorVoltage++;

		// Redraw generator gauge
		_vm->getCard()->redrawArea(62);
		_vm->getCard()->redrawArea(63);
		_vm->getCard()->redrawArea(96);
	}
}

void Myst::o_generatorControlRoom_init(uint16 var, const ArgumentsArray &args) {
	_generatorVoltage = _state.generatorVoltage;
	_generatorControlRoomRunning = true;
}

void Myst::o_fireplace_init(uint16 var, const ArgumentsArray &args) {
	// Clear fireplace grid
	for (uint i = 0; i < 6; i++)
		_fireplaceLines[i] = 0;
}

void Myst::o_clockGears_init(uint16 var, const ArgumentsArray &args) {
	// Used for Card 4113 (Clock Tower Cog Puzzle)
	// Set gears position
	if (_state.gearsOpen) {
		_clockGearsPositions[0] = 2;
		_clockGearsPositions[1] = 2;
		_clockGearsPositions[2] = 1;
		_clockWeightPosition = 2214;
	} else {
		_clockGearsPositions[0] = 3;
		_clockGearsPositions[1] = 3;
		_clockGearsPositions[2] = 3;
		_clockWeightPosition = 0;
	}
}

void Myst::o_gulls1_init(uint16 var, const ArgumentsArray &args) {
	if (!_state.shipFloating) {
		_gullsNextTime = _vm->getTotalPlayTime() + 2000;
		_gullsFlying1 = true;
	}
}

void Myst::gullsFly1_run() {
	static const char* gulls[] = { "birds1", "birds2", "birds3" };
	uint32 time = _vm->getTotalPlayTime();

	if (time > _gullsNextTime) {
		uint16 video = _vm->_rnd->getRandomNumber(3);
		if (video != 3) {
			uint16 x = 0;
			if (_vm->_rnd->getRandomBit())
				x = _vm->_rnd->getRandomNumber(110);
			else
				x = _vm->_rnd->getRandomNumber(160) + 260;

			VideoEntryPtr handle = _vm->playMovie(gulls[video], kMystStack);
			handle->moveTo(x, 0);

			_gullsNextTime = time + _vm->_rnd->getRandomNumber(16667) + 13334;
		}
	}
}

void Myst::o_observatory_init(uint16 var, const ArgumentsArray &args) {
	_tempVar = 0;
	_observatoryNotInitialized = true;
	_observatoryVisualizer = getInvokingResource<MystAreaImageSwitch>();
	_observatoryGoButton = _vm->getCard()->getResource<MystAreaImageSwitch>(args[0]);
	if (observatoryIsDDMMYYYY2400()) {
		_observatoryDaySlider = _vm->getCard()->getResource<MystAreaSlider>(args[1]);
		_observatoryMonthSlider = _vm->getCard()->getResource<MystAreaSlider>(args[2]);
	} else {
		_observatoryMonthSlider = _vm->getCard()->getResource<MystAreaSlider>(args[1]);
		_observatoryDaySlider = _vm->getCard()->getResource<MystAreaSlider>(args[2]);
	}
	_observatoryYearSlider = _vm->getCard()->getResource<MystAreaSlider>(args[3]);
	_observatoryTimeSlider = _vm->getCard()->getResource<MystAreaSlider>(args[4]);

	// Set date selection sliders position
	_observatoryDaySlider->setPosition(_state.observatoryDaySlider);
	_observatoryMonthSlider->setPosition(_state.observatoryMonthSlider);
	_observatoryYearSlider->setPosition(_state.observatoryYearSlider);
	_observatoryTimeSlider->setPosition(_state.observatoryTimeSlider);

	_observatoryLastTime = _vm->getTotalPlayTime();

	observatorySetTargetToSetting();

	_observatoryRunning = true;
}

bool Myst::observatoryIsDDMMYYYY2400() {
	// TODO: Auto-detect based on the month rect position
	return !_vm->isGameVariant(GF_ME) && (_vm->getLanguage() == Common::FR_FRA
			|| _vm->getLanguage() == Common::DE_DEU);
}

void Myst::observatoryUpdateVisualizer(uint16 x, uint16 y) {
	Common::Rect visu;
	visu.left = x;
	visu.right = visu.left + 105;
	visu.bottom = 512 - y;
	visu.top = visu.bottom - 106;

	_observatoryVisualizer->setSubImageRect(0, visu);
	_observatoryVisualizer->setSubImageRect(1, visu);
}

void Myst::observatorySetTargetToSetting() {
	uint32 visuX = _state.observatoryTimeSetting * 7 / 25;
	uint32 visuY = 250 * _state.observatoryYearSetting
			+ 65 * (_state.observatoryMonthSetting + 1)
			+ 20 * _state.observatoryDaySetting;

	observatoryUpdateVisualizer(visuX % 407, visuY % 407);

	_state.observatoryDayTarget = _state.observatoryDaySetting;
	_state.observatoryMonthTarget = _state.observatoryMonthSetting;
	_state.observatoryYearTarget = _state.observatoryYearSetting;
	_state.observatoryTimeTarget = _state.observatoryTimeSetting;
}

void Myst::observatory_run() {
	if (_observatoryNotInitialized) {
		_observatoryNotInitialized = false;

		_vm->_cursor->hideCursor();

		// Make sliders "initialize"
		if (observatoryIsDDMMYYYY2400()) {
			_vm->_sound->playEffect(8500);
			_observatoryDaySlider->drawConditionalDataToScreen(2);
			_vm->wait(200);
			_vm->redrawResource(_observatoryDaySlider);

			_vm->_sound->playEffect(8500);
			_observatoryMonthSlider->drawConditionalDataToScreen(2);
			_vm->wait(200);
			_vm->redrawResource(_observatoryMonthSlider);
		} else {
			_vm->_sound->playEffect(8500);
			_observatoryMonthSlider->drawConditionalDataToScreen(2);
			_vm->wait(200);
			_vm->redrawResource(_observatoryMonthSlider);

			_vm->_sound->playEffect(8500);
			_observatoryDaySlider->drawConditionalDataToScreen(2);
			_vm->wait(200);
			_vm->redrawResource(_observatoryDaySlider);
		}

		_vm->_sound->playEffect(8500);
		_observatoryYearSlider->drawConditionalDataToScreen(2);
		_vm->wait(200);
		_vm->redrawResource(_observatoryYearSlider);

		_vm->_sound->playEffect(8500);
		_observatoryTimeSlider->drawConditionalDataToScreen(2);
		_vm->wait(200);
		_vm->redrawResource(_observatoryTimeSlider);

		_vm->_cursor->showCursor();
	}

	// Setting not at target
	if (_state.observatoryDayTarget != _state.observatoryDaySetting
			|| _state.observatoryMonthTarget != _state.observatoryMonthSetting
			|| _state.observatoryYearTarget != _state.observatoryYearSetting
			|| _state.observatoryTimeTarget != _state.observatoryTimeSetting) {

		// Blink the go button
		uint32 time = _vm->getTotalPlayTime();
		if (time > _observatoryLastTime + 250) {
			_tempVar = (_tempVar + 1) % 2;
			_observatoryGoButton->drawConditionalDataToScreen(_tempVar);
			_observatoryLastTime = time;
		}
	}
}

void Myst::o_gulls2_init(uint16 var, const ArgumentsArray &args) {
	if (!_state.shipFloating) {
		_gullsNextTime = _vm->getTotalPlayTime() + 2000;
		_gullsFlying2 = true;
	}
}

void Myst::gullsFly2_run() {
	static const char* gulls[] = { "birds1", "birds2", "birds3" };
	uint32 time = _vm->getTotalPlayTime();

	if (time > _gullsNextTime) {
		uint16 video = _vm->_rnd->getRandomNumber(3);
		if (video != 3) {
			VideoEntryPtr handle = _vm->playMovie(gulls[video], kMystStack);
			handle->moveTo(424, 0);

			_gullsNextTime = time + _vm->_rnd->getRandomNumber(16667) + 13334;
		}
	}
}

void Myst::o_treeCard_init(uint16 var, const ArgumentsArray &args) {
	_tree = getInvokingResource<MystAreaImageSwitch>();
}

void Myst::o_treeEntry_init(uint16 var, const ArgumentsArray &args) {
	_treeAlcove = getInvokingResource<MystArea>();
	_treeMinAccessiblePosition = args[0];
	_treeMaxAccessiblePosition = args[1];

	treeSetAlcoveAccessible();
}

void Myst::o_boilerMovies_init(uint16 var, const ArgumentsArray &args) {
	boilerFireInit();
	boilerGaugeInit();
}

void Myst::boilerFireInit() {
	if (_vm->getCard()->getId() == 4098) {
		_cabinFireMovie = _vm->playMovie("cabfire", kMystStack);
		_cabinFireMovie->moveTo(240, 279);
		_cabinFireMovie->setLooping(true);
		_cabinFireMovie->pause(true);

		_vm->getCard()->redrawArea(305);
		boilerFireUpdate(true);
	} else {
		if (_state.cabinPilotLightLit == 1 && _state.cabinValvePosition >= 1) {
			_cabinFireMovie = _vm->playMovie("cabfirfr", kMystStack);
			_cabinFireMovie->moveTo(254, 244);
			_cabinFireMovie->setLooping(true);
		}
	}
}

void Myst::boilerFireUpdate(bool init) {
	uint position = _cabinFireMovie->getTime();

	if (_state.cabinPilotLightLit == 1) {
		if (_state.cabinValvePosition == 0) {
			if (position > (uint)Audio::Timestamp(0, 200, 600).msecs() || init) {
				_cabinFireMovie->setBounds(Audio::Timestamp(0, 0, 600), Audio::Timestamp(0, 100, 600));
				_cabinFireMovie->pause(false);
			}
		} else {
			if (position < (uint)Audio::Timestamp(0, 200, 600).msecs() || init) {
				_cabinFireMovie->setBounds(Audio::Timestamp(0, 201, 600), Audio::Timestamp(0, 1900, 600));
				_cabinFireMovie->pause(false);
			}
		}
	}
}

void Myst::boilerGaugeInit() {
	if (_vm->getCard()->getId() == 4098) {
		_cabinGaugeMovie = _vm->playMovie("cabingau", kMystStack);
		_cabinGaugeMovie->moveTo(243, 96);
	} else {
		_cabinGaugeMovie = _vm->playMovie("cabcgfar", kMystStack);
		_cabinGaugeMovie->moveTo(254, 136);
	}

	Audio::Timestamp frame;

	if (_state.cabinPilotLightLit == 1 && _state.cabinValvePosition > 12)
		frame = _cabinGaugeMovie->getDuration();
	else
		frame = Audio::Timestamp(0, 0, 600);

	_vm->_video->drawVideoFrame(_cabinGaugeMovie, frame);

	_cabinGaugeMovieEnabled = true;
}

void Myst::o_rocketSliders_init(uint16 var, const ArgumentsArray &args) {
	_rocketLinkBook.reset();
	_rocketSlider1 = _vm->getCard()->getResource<MystAreaSlider>(args[0]);
	_rocketSlider2 = _vm->getCard()->getResource<MystAreaSlider>(args[1]);
	_rocketSlider3 = _vm->getCard()->getResource<MystAreaSlider>(args[2]);
	_rocketSlider4 = _vm->getCard()->getResource<MystAreaSlider>(args[3]);
	_rocketSlider5 = _vm->getCard()->getResource<MystAreaSlider>(args[4]);

	// Initialize sliders position
	for (uint i = 0; i < 5; i++)
		if (!_state.rocketSliderPosition[i])
			_state.rocketSliderPosition[i] = 277;

	_rocketSlider1->setPosition(_state.rocketSliderPosition[0]);
	_rocketSlider2->setPosition(_state.rocketSliderPosition[1]);
	_rocketSlider3->setPosition(_state.rocketSliderPosition[2]);
	_rocketSlider4->setPosition(_state.rocketSliderPosition[3]);
	_rocketSlider5->setPosition(_state.rocketSliderPosition[4]);
}

void Myst::o_rocketLinkVideo_init(uint16 var, const ArgumentsArray &args) {
	_tempVar = 0;
}

void Myst::o_greenBook_init(uint16 var, const ArgumentsArray &args) {
	// Used for Card 4168 (Green Book Movies)
	_greenBookRunning = true;
	_tempVar = 1;
}

void Myst::greenBook_run() {
	uint loopStart = 0;
	uint loopEnd = 0;
	Common::String videoName;

	if (!_state.greenBookOpenedBefore) {
		loopStart = 113200;
		loopEnd = 116400;
		videoName = "atrusbk1";
	} else {
		loopStart = 8800;
		loopEnd = 9700;
		videoName = "atrusbk2";
	}

	if (_tempVar == 1) {
		_vm->_sound->stopEffect();
		_vm->_sound->pauseBackground();

		VideoEntryPtr book = _vm->playMovie(videoName, kMystStack);
		book->moveTo(314, 76);

		if (_globals.ending != kBooksDestroyed) {
			_tempVar = 2;
		} else {
			book->setBounds(Audio::Timestamp(0, loopStart, 600), Audio::Timestamp(0, loopEnd, 600));
			book->setLooping(true);
			_tempVar = 0;
		}
	} else if (_tempVar == 2 && !_vm->_video->isVideoPlaying()) {
		VideoEntryPtr book = _vm->playMovie(videoName, kMystStack);
		book->moveTo(314, 76);
		book->setBounds(Audio::Timestamp(0, loopStart, 600), Audio::Timestamp(0, loopEnd, 600));
		book->setLooping(true);

		_tempVar = 0;
	}
}

void Myst::o_gulls3_init(uint16 var, const ArgumentsArray &args) {
	if (!_state.shipFloating) {
		_gullsNextTime = _vm->getTotalPlayTime() + 2000;
		_gullsFlying3 = true;
	}
}

void Myst::gullsFly3_run() {
	static const char* gulls[] = { "birds1", "birds2", "birds3" };
	uint32 time = _vm->getTotalPlayTime();

	if (time > _gullsNextTime) {
		uint16 video = _vm->_rnd->getRandomNumber(3);
		if (video != 3) {
			uint16 x = _vm->_rnd->getRandomNumber(280) + 135;

			VideoEntryPtr handle = _vm->playMovie(gulls[video], kMystStack);
			handle->moveTo(x, 0);

			_gullsNextTime = time + _vm->_rnd->getRandomNumber(16667) + 13334;
		}
	}
}

void Myst::o_bookAddSpecialPage_exit(uint16 var, const ArgumentsArray &args) {
	uint16 numPages = bookCountPages(var);

	// Add special page
	if (numPages == 5) {
		if (var == 100)
			_globals.redPagesInBook |= 64;
		else
			_globals.bluePagesInBook |= 64;
	}
}

void Myst::o_treeCard_exit(uint16 var, const ArgumentsArray &args) {
	_tree = nullptr;
}

void Myst::o_treeEntry_exit(uint16 var, const ArgumentsArray &args) {
	_treeAlcove = nullptr;
}

void Myst::o_boiler_exit(uint16 var, const ArgumentsArray &args) {
	_cabinGaugeMovie = VideoEntryPtr();
	_cabinFireMovie = VideoEntryPtr();

	_cabinGaugeMovieEnabled = false;
}

void Myst::o_generatorControlRoom_exit(uint16 var, const ArgumentsArray &args) {
	_generatorVoltage = _state.generatorVoltage;
}

void Myst::o_rocketSliders_exit(uint16 var, const ArgumentsArray &args) {
	_rocketLinkBook.reset();
	_rocketSlider1 = nullptr;
	_rocketSlider2 = nullptr;
	_rocketSlider3 = nullptr;
	_rocketSlider4 = nullptr;
	_rocketSlider5 = nullptr;
}

} // End of namespace MystStacks
} // End of namespace Mohawk
