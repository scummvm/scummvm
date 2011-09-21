/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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
 */

#include "pegasus/menu.h"
#include "pegasus/pegasus.h"

namespace Pegasus {

GameMenu::GameMenu(const uint32 id) : IDObject(id), InputHandler((InputHandler *)((PegasusEngine *)g_engine)) {
	_previousHandler = 0;
	_lastCommand = kMenuCmdNoCommand;
}

void GameMenu::becomeCurrentHandler() {
	_previousHandler = InputHandler::setInputHandler(this);
}

void GameMenu::restorePreviousHandler() {
	InputHandler::setInputHandler(_previousHandler);
}

enum {
	kMainMenuStartDemo = 0,
	kMainMenuCreditsDemo,
	kMainMenuQuitDemo,
	kFirstSelectionDemo = kMainMenuStartDemo,
	kLastSelectionDemo = kMainMenuQuitDemo,

	kMainMenuOverview = 0,
	kMainMenuStart,
	kMainMenuRestore,
	kMainMenuDifficulty,
	kMainMenuCredits,
	kMainMenuQuit,
	kFirstSelection = kMainMenuOverview,
	kLastSelection = kMainMenuQuit
};

static const tCoordType kStartLeftDemo = 44;
static const tCoordType kStartTopDemo = 336;

static const tCoordType kStartSelectLeftDemo = 40;
static const tCoordType kStartSelectTopDemo = 331;

static const tCoordType kCreditsLeftDemo = 44;
static const tCoordType kCreditsTopDemo = 372;

static const tCoordType kCreditsSelectLeftDemo = 40;
static const tCoordType kCreditsSelectTopDemo = 367;

static const tCoordType kMainMenuQuitLeftDemo = 32;
static const tCoordType kMainMenuQuitTopDemo = 412;

static const tCoordType kMainMenuQuitSelectLeftDemo = 28;
static const tCoordType kMainMenuQuitSelectTopDemo = 408;

static const tCoordType kOverviewLeft = 200;
static const tCoordType kOverviewTop = 208;

static const tCoordType kOverviewSelectLeft = 152;
static const tCoordType kOverviewSelectTop = 204;

static const tCoordType kStartLeft = 212;
static const tCoordType kStartTop = 256;

static const tCoordType kStartSelectLeft = 152;
static const tCoordType kStartSelectTop = 252;

static const tCoordType kRestoreLeft = 212;
static const tCoordType kRestoreTop = 296;

static const tCoordType kRestoreSelectLeft = 152;
static const tCoordType kRestoreSelectTop = 292;

static const tCoordType kDifficultyLeft = 320;
static const tCoordType kDifficultyTop = 340;

static const tCoordType kDifficultySelectLeft = 152;
static const tCoordType kDifficultySelectTop = 336;

static const tCoordType kCreditsLeft = 212;
static const tCoordType kCreditsTop = 388;

static const tCoordType kCreditsSelectLeft = 152;
static const tCoordType kCreditsSelectTop = 384;

static const tCoordType kMainMenuQuitLeft = 212;
static const tCoordType kMainMenuQuitTop = 428;

static const tCoordType kMainMenuQuitSelectLeft = 152;
static const tCoordType kMainMenuQuitSelectTop = 424;

//	Never set the current input handler to the MainMenu.
MainMenu::MainMenu() : GameMenu(kMainMenuID), _menuBackground(0), _overviewButton(0),
		_restoreButton(0), _adventureButton(0), _walkthroughButton(0), _startButton(0),
		_creditsButton(0), _quitButton(0), _largeSelect(0), _smallSelect(0) {

	bool isDemo = ((PegasusEngine *)g_engine)->isDemo();

	if (isDemo)
		_menuBackground.initFromPICTFile("Images/Demo/DemoMenu.pict");
	else
		_menuBackground.initFromPICTFile("Images/Main Menu/MainMenu.mac");
	_menuBackground.setDisplayOrder(0);
	_menuBackground.startDisplaying();
	_menuBackground.show();

	if (!isDemo) {
		_overviewButton.initFromPICTFile("Images/Main Menu/pbOvervi.pict");
		_overviewButton.setDisplayOrder(1);
		_overviewButton.moveElementTo(kOverviewLeft, kOverviewTop);
		_overviewButton.startDisplaying();

		_restoreButton.initFromPICTFile("Images/Main Menu/pbRestor.pict");
		_restoreButton.setDisplayOrder(1);
		_restoreButton.moveElementTo(kRestoreLeft, kRestoreTop);
		_restoreButton.startDisplaying();

		_adventureButton.initFromPICTFile("Images/Main Menu/BtnAdv.pict");
		_adventureButton.setDisplayOrder(1);
		_adventureButton.moveElementTo(kDifficultyLeft, kDifficultyTop);
		_adventureButton.startDisplaying();

		_walkthroughButton.initFromPICTFile("Images/Main Menu/BtnWlk.pict");
		_walkthroughButton.setDisplayOrder(1);
		_walkthroughButton.moveElementTo(kDifficultyLeft, kDifficultyTop);
		_walkthroughButton.startDisplaying();
	}

	if (isDemo)
		_startButton.initFromPICTFile("Images/Demo/Start.pict");
	else
		_startButton.initFromPICTFile("Images/Main Menu/pbStart.pict");
	_startButton.setDisplayOrder(1);
	_startButton.moveElementTo(isDemo ? kStartLeftDemo : kStartLeft, isDemo ? kStartTopDemo : kStartTop);
	_startButton.startDisplaying();

	if (isDemo)
		_creditsButton.initFromPICTFile("Images/Demo/Credits.pict");
	else
		_creditsButton.initFromPICTFile("Images/Main Menu/pbCredit.pict");
	_creditsButton.setDisplayOrder(1);
	_creditsButton.moveElementTo(isDemo ? kCreditsLeftDemo : kCreditsLeft, isDemo ? kCreditsTopDemo : kCreditsTop);
	_creditsButton.startDisplaying();

	if (isDemo)
		_quitButton.initFromPICTFile("Images/Demo/Quit.pict");
	else
		_quitButton.initFromPICTFile("Images/Main Menu/pbQuit.pict");
	_quitButton.setDisplayOrder(1);
	_quitButton.moveElementTo(isDemo ? kMainMenuQuitLeftDemo : kMainMenuQuitLeft, isDemo ? kMainMenuQuitTopDemo : kMainMenuQuitTop);
	_quitButton.startDisplaying();

	if (isDemo)
		_largeSelect.initFromPICTFile("Images/Demo/SelectL.pict", true);
	else
		_largeSelect.initFromPICTFile("Images/Main Menu/SelectL.pict", true);
	_largeSelect.setDisplayOrder(1);
	_largeSelect.startDisplaying();

	if (isDemo)
		_smallSelect.initFromPICTFile("Images/Demo/SelectS.pict", true);
	else
		_smallSelect.initFromPICTFile("Images/Main Menu/SelectS.pict", true);
	_smallSelect.setDisplayOrder(1);
	_smallSelect.startDisplaying();

	_menuSelection = isDemo ? kFirstSelectionDemo : kFirstSelection;

	_adventureMode = true;

	//_menuLoop.attachFader(&_menuFader);
	_menuLoop.initFromAIFFFile("Sounds/Main Menu.aiff");

	updateDisplay();
}

MainMenu::~MainMenu() {
	if (_menuLoop.isPlaying())
		stopMainMenuLoop();
}

void MainMenu::startMainMenuLoop() {
	FaderMoveSpec spec;

	_menuLoop.loopSound();
	spec.makeTwoKnotFaderSpec(30, 0, 0, 30, 255);
	//_menuFader.startFaderSync(spec);
}

void MainMenu::stopMainMenuLoop() {
	FaderMoveSpec spec;
	
	spec.makeTwoKnotFaderSpec(30, 0, 255, 30, 0);
	//_menuFader.startFaderSync(spec);
	_menuLoop.stopSound();
}

void MainMenu::handleInput(const Input &input, const Hotspot *cursorSpot) {
	PegasusEngine *vm = (PegasusEngine *)g_engine;
	bool isDemo = vm->isDemo();

	if (input.upButtonDown()) {
		if (_menuSelection > (isDemo ? kFirstSelectionDemo : kFirstSelection)) {
			_menuSelection--;
			updateDisplay();
		}
	} else if (input.downButtonDown()) {
		if (_menuSelection < (isDemo ? kLastSelectionDemo : kLastSelection)) {
			_menuSelection++;
			updateDisplay();
		}
	} else if (!isDemo && (input.leftButtonDown() || input.rightButtonDown())) {
		if (_menuSelection == kMainMenuDifficulty) {
			_adventureMode = !_adventureMode;
			updateDisplay();
		}
	} else if (JMPPPInput::isMenuButtonPressInput(input)) {
		if (isDemo) {
			switch (_menuSelection) {
			case kMainMenuCreditsDemo:
				_creditsButton.show();
				vm->delayShell(kMenuButtonHiliteTime, kMenuButtonHiliteScale);
				_creditsButton.hide();
				setLastCommand(kMenuCmdCredits);
				break;
			case kMainMenuStartDemo:
				_startButton.show();
				vm->delayShell(kMenuButtonHiliteTime, kMenuButtonHiliteScale);
				_startButton.hide();
				setLastCommand(kMenuCmdStartAdventure);
				break;
			case kMainMenuQuitDemo:
				_quitButton.show();
				vm->delayShell(kMenuButtonHiliteTime, kMenuButtonHiliteScale);
				_quitButton.hide();
				setLastCommand(kMenuCmdQuit);
				break;
			}
		} else {
			switch (_menuSelection) {
			case kMainMenuOverview:
				_overviewButton.show();
				vm->delayShell(kMenuButtonHiliteTime, kMenuButtonHiliteScale);
				_overviewButton.hide();
				setLastCommand(kMenuCmdOverview);
				break;
			case kMainMenuRestore:
				_restoreButton.show();
				vm->delayShell(kMenuButtonHiliteTime, kMenuButtonHiliteScale);
				_restoreButton.hide();
				setLastCommand(kMenuCmdRestore);
				break;
			case kMainMenuCredits:
				_creditsButton.show();
				vm->delayShell(kMenuButtonHiliteTime, kMenuButtonHiliteScale);
				_creditsButton.hide();
				setLastCommand(kMenuCmdCredits);
				break;
			case kMainMenuStart:
				_startButton.show();
				vm->delayShell(kMenuButtonHiliteTime, kMenuButtonHiliteScale);
				_startButton.hide();
				if (_adventureMode)
					setLastCommand(kMenuCmdStartAdventure);
				else
					setLastCommand(kMenuCmdStartWalkthrough);
				break;
			case kMainMenuDifficulty:
				_adventureMode = !_adventureMode;
				updateDisplay();
				break;
			case kMainMenuQuit:
				_quitButton.show();
				vm->delayShell(kMenuButtonHiliteTime, kMenuButtonHiliteScale);
				_quitButton.hide();
				setLastCommand(kMenuCmdQuit);
				break;
			}
		}
	}

	InputHandler::handleInput(input, cursorSpot);
}

void MainMenu::updateDisplay() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	if (vm->isDemo()) {
		switch (_menuSelection) {
		case kMainMenuStartDemo:
			_smallSelect.moveElementTo(kStartSelectLeftDemo, kStartSelectTopDemo);
			_smallSelect.show();
			_largeSelect.hide();
			break;
		case kMainMenuCreditsDemo:
			_smallSelect.moveElementTo(kCreditsSelectLeftDemo, kCreditsSelectTopDemo);
			_smallSelect.show();
			_largeSelect.hide();
			break;
		case kMainMenuQuitDemo:
			_largeSelect.moveElementTo(kMainMenuQuitSelectLeftDemo, kMainMenuQuitSelectTopDemo);
			_largeSelect.show();
			_smallSelect.hide();
			break;
		}
	} else {
		switch (_menuSelection) {
		case kMainMenuOverview:
			_largeSelect.moveElementTo(kOverviewSelectLeft, kOverviewSelectTop);
			_largeSelect.show();
			_smallSelect.hide();
			break;
		case kMainMenuRestore:
			_smallSelect.moveElementTo(kRestoreSelectLeft, kRestoreSelectTop);
			_smallSelect.show();
			_largeSelect.hide();
			break;
		case kMainMenuDifficulty:
			if (_adventureMode) {
				_adventureButton.show();
				_walkthroughButton.hide();
			} else {
				_walkthroughButton.show();
				_adventureButton.hide();
			}

			_largeSelect.moveElementTo(kDifficultySelectLeft, kDifficultySelectTop);
			_largeSelect.show();
			_smallSelect.hide();
			break;
		case kMainMenuStart:
			_smallSelect.moveElementTo(kStartSelectLeft, kStartSelectTop);
			_smallSelect.show();
			_largeSelect.hide();
			break;
		case kMainMenuCredits:
			_smallSelect.moveElementTo(kCreditsSelectLeft, kCreditsSelectTop);
			_smallSelect.show();
			_largeSelect.hide();
			break;
		case kMainMenuQuit:
			_smallSelect.moveElementTo(kMainMenuQuitSelectLeft, kMainMenuQuitSelectTop);
			_smallSelect.show();
			_largeSelect.hide();
			break;
		}

		vm->resetIntroTimer();
	}
}

} // End of namespace Pegasus
