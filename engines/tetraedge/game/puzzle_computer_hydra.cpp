/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/puzzle_computer_hydra.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/amerzone_game.h"
#include "tetraedge/game/game.h"
#include "tetraedge/te/te_sound_manager.h"

namespace Tetraedge {

PuzzleComputerHydra::PuzzleComputerHydra() : _checklistStep(0), _axisNo(0) {
	ARRAYCLEAR(_enteredCoord, 0);
	ARRAYCLEAR(_targetCoord, 0);
}

void PuzzleComputerHydra::enter() {
	_gui.load("GUI/PuzzleComputerHydra.lua");
	Application *app = g_engine->getApplication();
	app->frontLayout().addChild(_gui.layoutChecked("puzzleComputerHydra"));
	_exitTimer.alarmSignal().add(this, &PuzzleComputerHydra::onExitTimer);
	_exitTimer.start();
	_transitionTimer.start();
	initAll();
	hideScreens();
	enterChecklistScreen();
}

bool PuzzleComputerHydra::leave() {
	_exitTimer.alarmSignal().clear();
	_transitionTimer.alarmSignal().clear();
	_gui.unload();
	AmerzoneGame *game = dynamic_cast<AmerzoneGame *>(g_engine->getGame());
	assert(game);
	game->warpY()->setVisible(true, false);
	return false;
}

void PuzzleComputerHydra::setTargetCoordinates(int x, int y, int z) {
	_targetCoord[0] = x;
	_targetCoord[1] = y;
	_targetCoord[2] = z;
}

void PuzzleComputerHydra::clearChecklistScreen() {
	_gui.layoutChecked("eggText")->setVisible(false);
	_gui.layoutChecked("fuelText")->setVisible(false);
	_gui.layoutChecked("capText")->setVisible(false);
}

bool PuzzleComputerHydra::enterChecklistScreen() {
	_transitionTimer.alarmSignal().add(this, &PuzzleComputerHydra::onTransitionTimer);
	exitCoordinatesScreen();
	exitSelectMode();
	_checklistStep = 0;
	_gui.layoutChecked("checklist")->setVisible(true);
	_gui.layoutChecked("checklist")->setRatioMode(TeILayout::RATIO_MODE_NONE);
	_gui.spriteLayoutChecked("title")->load("2D/puzzles/Computer_Hydra/CHECKLIST.png");
	_gui.spriteLayoutChecked("infos")->setVisible(false);
	clearChecklistScreen();
	processCheckListScreen();
	return true;
}

bool PuzzleComputerHydra::enterCoordinatesScreen() {
	_transitionTimer.alarmSignal().remove(this, &PuzzleComputerHydra::enterCoordinatesScreen);
	exitChecklistScreen();
	_axisNo = 0;
	_enteredCoord[0] = -1;
	_enteredCoord[1] = -1;
	_enteredCoord[2] = -1;
	_gui.spriteLayoutChecked("coordinates")->setVisible(true);
	_gui.spriteLayoutChecked("coordinates")->setRatioMode(TeILayout::RATIO_MODE_NONE);
	_gui.spriteLayoutChecked("titleCoordinates")->load("2D/puzzles/Computer_Hydra/ENTERDETAILS.png");
	_gui.spriteLayoutChecked("title")->setVisible(false);
	_gui.spriteLayoutChecked("infos")->setVisible(false);
	_gui.spriteLayoutChecked("infosCoordinates")->setVisible(false);
	_gui.buttonLayoutChecked("button0")->setEnable(true);
	_gui.buttonLayoutChecked("button1")->setEnable(true);
	_gui.buttonLayoutChecked("button2")->setEnable(true);
	_gui.buttonLayoutChecked("button3")->setEnable(true);
	_gui.buttonLayoutChecked("button4")->setEnable(true);
	_gui.buttonLayoutChecked("button5")->setEnable(true);
	_gui.buttonLayoutChecked("button6")->setEnable(true);
	_gui.buttonLayoutChecked("button7")->setEnable(true);
	_gui.buttonLayoutChecked("button8")->setEnable(true);
	_gui.buttonLayoutChecked("button9")->setEnable(true);
	_gui.buttonLayoutChecked("cancel")->setEnable(true);
	_gui.buttonLayoutChecked("exit")->setEnable(true);
	_gui.spriteLayoutChecked("digit1")->setVisible(false);
	_gui.spriteLayoutChecked("digit2")->setVisible(false);
	_gui.spriteLayoutChecked("digit3")->setVisible(false);
	return true;
}

bool PuzzleComputerHydra::enterSelectMode() {
	_transitionTimer.alarmSignal().remove(this, &PuzzleComputerHydra::enterSelectMode);
	exitChecklistScreen();
	_gui.layoutChecked("modeSelect")->setVisible(true);
	_gui.layoutChecked("modeSelect")->setRatioMode(TeILayout::RATIO_MODE_NONE);
	_gui.spriteLayoutChecked("title")->load("2D/puzzles/Computer_Hydra/CHOOSEMODE.png");
	_gui.spriteLayoutChecked("infos")->setVisible(false);
	return true;
}

bool PuzzleComputerHydra::exitChecklistScreen() {
	_gui.layoutChecked("checklist")->setVisible(false);
	return true;
}

bool PuzzleComputerHydra::exitCoordinatesScreen() {
	_transitionTimer.alarmSignal().remove(this, &PuzzleComputerHydra::enterChecklistScreen);
	_gui.spriteLayoutChecked("title")->setVisible(true);
	_gui.spriteLayoutChecked("infos")->setVisible(false);
	_gui.buttonLayoutChecked("button0")->setEnable(false);
	_gui.buttonLayoutChecked("button1")->setEnable(false);
	_gui.buttonLayoutChecked("button2")->setEnable(false);
	_gui.buttonLayoutChecked("button3")->setEnable(false);
	_gui.buttonLayoutChecked("button4")->setEnable(false);
	_gui.buttonLayoutChecked("button5")->setEnable(false);
	_gui.buttonLayoutChecked("button6")->setEnable(false);
	_gui.buttonLayoutChecked("button7")->setEnable(false);
	_gui.buttonLayoutChecked("button8")->setEnable(false);
	_gui.buttonLayoutChecked("button9")->setEnable(false);
	_gui.buttonLayoutChecked("cancel")->setEnable(false);
	_gui.spriteLayoutChecked("digit1")->setVisible(false);
	_gui.spriteLayoutChecked("digit2")->setVisible(false);
	_gui.spriteLayoutChecked("digit3")->setVisible(false);
	return true;
}

bool PuzzleComputerHydra::exitSelectMode() {
	_gui.layoutChecked("modeSelect")->setVisible(false);
	return true;
}

void PuzzleComputerHydra::hideScreens() {
	_gui.layoutChecked("checklist")->setVisible(false);
	_gui.layoutChecked("coordinates")->setVisible(false);
	_gui.layoutChecked("modeSelect")->setVisible(false);

}

bool PuzzleComputerHydra::hideUnavailableModeMsg() {
	_gui.layoutChecked("infos")->setVisible(false);
	_transitionTimer.alarmSignal().remove(this, &PuzzleComputerHydra::hideUnavailableModeMsg);
	return true;
}

void PuzzleComputerHydra::initAll() {
	_gui.spriteLayoutChecked("screenBase")->setVisible(true);
	_gui.spriteLayoutChecked("screenBase")->setRatioMode(TeILayout::RATIO_MODE_NONE);
	_gui.spriteLayoutChecked("title")->setVisible(true);
	_gui.spriteLayoutChecked("infos")->setVisible(false);
	_gui.buttonLayoutChecked("confirmDestination")->setVisible(false);
	_gui.buttonLayoutChecked("confirmDestination")->setEnable(false);
	_gui.buttonLayoutChecked("button0")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButton0Clicked);
	_gui.buttonLayoutChecked("button1")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButton1Clicked);
	_gui.buttonLayoutChecked("button2")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButton2Clicked);
	_gui.buttonLayoutChecked("button3")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButton3Clicked);
	_gui.buttonLayoutChecked("button4")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButton4Clicked);
	_gui.buttonLayoutChecked("button5")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButton5Clicked);
	_gui.buttonLayoutChecked("button6")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButton6Clicked);
	_gui.buttonLayoutChecked("button7")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButton7Clicked);
	_gui.buttonLayoutChecked("button8")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButton8Clicked);
	_gui.buttonLayoutChecked("button9")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButton9Clicked);

	_gui.buttonLayoutChecked("cancel")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButtonCancelClicked);
	_gui.buttonLayoutChecked("exit")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onExitButton);

	_gui.buttonLayoutChecked("buttonBoat")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButtonBoatClicked);
	_gui.buttonLayoutChecked("buttonGrapple")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButtonGrappleClicked);
	_gui.buttonLayoutChecked("buttonHelicopter")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButtonHelicopterClicked);
	_gui.buttonLayoutChecked("buttonSubmarine")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButtonSubmarineClicked);
	_gui.buttonLayoutChecked("buttonSailboat")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButtonSailboatClicked);
	_gui.buttonLayoutChecked("buttonPlane")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onButtonPlaneClicked);
	_gui.buttonLayoutChecked("confirmDestination")->onMouseClickValidated().add(this, &PuzzleComputerHydra::onPuzzleCompleted);
}

bool PuzzleComputerHydra::processCheckListScreen() {
	Game *game = g_engine->getGame();
	TeSoundManager *sndMgr = g_engine->getSoundManager();

	switch (_checklistStep) {
	case 0: {
		if (game->luaContext().global("Egg_On_Board").toBoolean()) {
			_gui.spriteLayoutChecked("eggText")->load("2D/puzzles/Computer_Hydra/EGGOK.png");
			_checklistStep = 1;
			_transitionTimer.setAlarmIn(1000000);
			sndMgr->playFreeSound("Sounds/SFX/BipOrdi.ogg");
		} else {
			_gui.spriteLayoutChecked("eggText")->load("2D/puzzles/Computer_Hydra/BADEGG.png");
			_transitionTimer.setAlarmIn(2000000);
			sndMgr->playFreeSound("Sounds/SFX/N_CodeFaux.ogg");
		}
		_gui.spriteLayoutChecked("eggText")->setVisible(true);
		break;
	}
	case 1: {
		if (game->luaContext().global("Fuel_Tank_Full").toBoolean()) {
			_gui.spriteLayoutChecked("fuelText")->load("2D/puzzles/Computer_Hydra/FUELOK.png");
			sndMgr->playFreeSound("Sounds/SFX/BipOrdi.ogg");
		} else {
			_gui.spriteLayoutChecked("fuelText")->load("2D/puzzles/Computer_Hydra/BADFUEL.png");
			sndMgr->playFreeSound("Sounds/SFX/N_CodeFaux.ogg");
		}
		_gui.spriteLayoutChecked("fuelText")->setVisible(true);
		_checklistStep = 3;
		_transitionTimer.setAlarmIn(1000000);
		break;
	}
	case 3: {
		if (game->luaContext().global("Destination_Set").toBoolean()) {
			_gui.spriteLayoutChecked("capText")->load("2D/puzzles/Computer_Hydra/CAPOK.png");
			_transitionTimer.alarmSignal().remove(this, &PuzzleComputerHydra::onTransitionTimer);
			_transitionTimer.alarmSignal().add(this, &PuzzleComputerHydra::enterSelectMode);
			sndMgr->playFreeSound("Sounds/SFX/BipOrdi.ogg");
		} else {
			_gui.spriteLayoutChecked("capText")->load("2D/puzzles/Computer_Hydra/BADCAP.png");
			_transitionTimer.alarmSignal().remove(this, &PuzzleComputerHydra::onTransitionTimer);
			_transitionTimer.alarmSignal().add(this, &PuzzleComputerHydra::enterCoordinatesScreen);
			sndMgr->playFreeSound("Sounds/SFX/N_CodeFaux.ogg");
		}
		_transitionTimer.setAlarmIn(1000000);
		_gui.spriteLayoutChecked("capText")->setVisible(true);
		break;
	}
	default:
		break;
	}
	return false;
}

bool PuzzleComputerHydra::registerNewDigit(int digit) {
	if (_axisNo >= 3)
		return false;
	_enteredCoord[_axisNo] = digit;
	_axisNo++;
	const Common::String dname = Common::String::format("digit%d", _axisNo);
	const Common::String dimg = Common::String::format("2D/puzzles/Computer_Hydra/%d.png", digit);
	_gui.spriteLayoutChecked(dname)->load(dimg);
	_gui.spriteLayoutChecked(dname)->setVisible(true);
	if (_axisNo == 3) {
		if (_enteredCoord[0] == _targetCoord[0] && _enteredCoord[1] == _targetCoord[1]
				&& _enteredCoord[2] == _targetCoord[2]) {
			// Correct!
			Game *game = g_engine->getGame();
			game->luaContext().setGlobal("Destination_Set", true);
			_gui.spriteLayoutChecked("infosCoordinates")->load("2D/puzzles/Computer_Hydra/CAPOK.png");
			_transitionTimer.alarmSignal().add(this, &PuzzleComputerHydra::enterChecklistScreen);
			_transitionTimer.setAlarmIn(1000000);
			g_engine->getSoundManager()->playFreeSound("Sounds/SFX/BipOrdi.ogg");
		} else {
			// Incorrect.
			_gui.spriteLayoutChecked("infosCoordinates")->load("2D/puzzles/Computer_Hydra/BADCAP.png");
			g_engine->getSoundManager()->playFreeSound("Sounds/SFX/N_CodeFaux.ogg");
		}
		_gui.spriteLayoutChecked("infosCoordinates")->setVisible(true);
	}
	return true;
}

bool PuzzleComputerHydra::showConfirmDestination() {
	exitSelectMode();
	_gui.buttonLayoutChecked("confirmDestination")->setVisible(true);
	_gui.buttonLayoutChecked("confirmDestination")->setEnable(true);
	_gui.spriteLayoutChecked("infos")->load("2D/puzzles/Computer_Hydra/CAPOK.png");
	_gui.spriteLayoutChecked("infos")->setVisible(true);
	return true;
}

bool PuzzleComputerHydra::showUnavailableModeMsg() {
	_gui.spriteLayoutChecked("infos")->load("2D/puzzles/Computer_Hydra/NONDISPO.png");
	_gui.spriteLayoutChecked("infos")->setVisible(true);
	_transitionTimer.alarmSignal().remove(this, &PuzzleComputerHydra::hideUnavailableModeMsg);
	_transitionTimer.alarmSignal().add(this, &PuzzleComputerHydra::hideUnavailableModeMsg);
	_transitionTimer.setAlarmIn(500000);
	return true;
}

bool PuzzleComputerHydra::onButton0Clicked() {
	return registerNewDigit(0);
}

bool PuzzleComputerHydra::onButton1Clicked() {
	return registerNewDigit(1);
}

bool PuzzleComputerHydra::onButton2Clicked() {
	return registerNewDigit(2);
}

bool PuzzleComputerHydra::onButton3Clicked() {
	return registerNewDigit(3);
}

bool PuzzleComputerHydra::onButton4Clicked() {
	return registerNewDigit(4);
}

bool PuzzleComputerHydra::onButton5Clicked() {
	return registerNewDigit(5);
}

bool PuzzleComputerHydra::onButton6Clicked() {
	return registerNewDigit(6);
}

bool PuzzleComputerHydra::onButton7Clicked() {
	return registerNewDigit(7);
}

bool PuzzleComputerHydra::onButton8Clicked() {
	return registerNewDigit(8);
}

bool PuzzleComputerHydra::onButton9Clicked() {
	return registerNewDigit(9);
}

bool PuzzleComputerHydra::onModeCheckButton(const Common::String &global) {
	Game *game = g_engine->getGame();
	TeSoundManager *sndMgr = g_engine->getSoundManager();
	bool available = game->luaContext().global(global).toBoolean();
	if (available) {
		showConfirmDestination();
		sndMgr->playFreeSound("Sounds/SFX/BipOrdi.ogg");
	} else {
		sndMgr->playFreeSound("Sounds/SFX/N_CodeFaux.ogg");
		showUnavailableModeMsg();
	}
	return true;
}

bool PuzzleComputerHydra::onButtonBoatClicked() {
	return onModeCheckButton("Bark_Mode_Available");
}

bool PuzzleComputerHydra::onButtonCancelClicked() {
	if (_axisNo <= 0)
		return true;
	const Common::String dname = Common::String::format("digit%d", _axisNo);
	_gui.spriteLayoutChecked(dname)->setVisible(false);
	_axisNo--;
	_enteredCoord[_axisNo] = -1;
	_gui.spriteLayoutChecked("infosCoordinates")->setVisible(false);
	return true;
}

bool PuzzleComputerHydra::onButtonGrappleClicked() {
	return onModeCheckButton("Grapple_Mode_Available");
}

bool PuzzleComputerHydra::onButtonHelicopterClicked() {
	return onModeCheckButton("Helicopter_Mode_Available");
}

bool PuzzleComputerHydra::onButtonSubmarineClicked() {
	return onModeCheckButton("Submarine_Mode_Available");
}

bool PuzzleComputerHydra::onButtonSailboatClicked() {
	return onModeCheckButton("Sailboat_Mode_Available");
}

bool PuzzleComputerHydra::onButtonPlaneClicked() {
	return onModeCheckButton("Plane_Mode_Available");
}

bool PuzzleComputerHydra::onExitButton() {
	leave();
	return true;
}

bool PuzzleComputerHydra::onExitTimer() {
	leave();
	return false;
}

bool PuzzleComputerHydra::onTransitionTimer() {
	processCheckListScreen();
	return false;
}

bool PuzzleComputerHydra::onPuzzleCompleted() {
	leave();
	g_engine->getGame()->luaScript().execute("OnComputerHydraPuzzleCompleted");
	return true;
}

} // end namespace Tetraedge
