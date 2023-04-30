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

#ifndef TETRAEDGE_GAME_PUZZLE_COMPUTER_HYDRA_H
#define TETRAEDGE_GAME_PUZZLE_COMPUTER_HYDRA_H

#include "tetraedge/te/te_3d_object2.h"
#include "tetraedge/te/te_lua_gui.h"
#include "tetraedge/te/te_timer.h"

namespace Tetraedge {

class PuzzleComputerHydra : public Te3DObject2 {
public:
	PuzzleComputerHydra();

	void enter();
	bool leave();
	void setTargetCoordinates(int x, int y, int z);

private:
	void clearChecklistScreen();
	bool enterChecklistScreen();
	bool enterCoordinatesScreen();
	bool enterSelectMode();
	bool exitChecklistScreen();
	bool exitCoordinatesScreen();
	bool exitSelectMode();
	void hideScreens();
	bool hideUnavailableModeMsg();
	void initAll();
	bool processCheckListScreen();
	bool registerNewDigit(int digit);
	bool showConfirmDestination();
	bool showUnavailableModeMsg();

	bool onButton0Clicked();
	bool onButton1Clicked();
	bool onButton2Clicked();
	bool onButton3Clicked();
	bool onButton4Clicked();
	bool onButton5Clicked();
	bool onButton6Clicked();
	bool onButton7Clicked();
	bool onButton8Clicked();
	bool onButton9Clicked();
	bool onButtonBoatClicked();
	bool onButtonCancelClicked();
	bool onButtonGrappleClicked();
	bool onButtonHelicopterClicked();
	bool onButtonSubmarineClicked();
	bool onButtonSailboatClicked();
	bool onButtonPlaneClicked();
	bool onExitButton();
	bool onExitTimer();
	bool onPuzzleCompleted();
	bool onTransitionTimer();
	bool onModeCheckButton(const Common::String &global);

	TeLuaGUI _gui;
	TeTimer _exitTimer;
	TeTimer _transitionTimer;
	int _axisNo;
	int _enteredCoord[3];
	int _targetCoord[3];
	int _checklistStep;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_PUZZLE_COMPUTER_HYDRA_H
