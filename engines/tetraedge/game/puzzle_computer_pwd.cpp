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

#include "tetraedge/game/puzzle_computer_pwd.h"
#include "tetraedge/tetraedge.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/amerzone_game.h"

#include "tetraedge/te/te_sound_manager.h"

namespace Tetraedge {

static const int CORRECT_PWD[6] = {2, 8, 0, 6, 0, 4};

PuzzleComputerPwd::PuzzleComputerPwd() : _nDigits(0) {
	ARRAYCLEAR(_enteredPwd, 0);
}

void PuzzleComputerPwd::enter() {
	_gui.load("GUI/PuzzleComputerPwd.lua");
	g_engine->getApplication()->frontLayout().addChild(_gui.layoutChecked("puzzleComputerPassword"));
	_gui.spriteLayoutChecked("background")->setVisible(true);
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

	_gui.spriteLayoutChecked("star1")->setVisible(false);
	_gui.spriteLayoutChecked("star2")->setVisible(false);
	_gui.spriteLayoutChecked("star3")->setVisible(false);
	_gui.spriteLayoutChecked("star4")->setVisible(false);
	_gui.spriteLayoutChecked("star5")->setVisible(false);
	_gui.spriteLayoutChecked("star6")->setVisible(false);

	_gui.buttonLayoutChecked("button0")->onMouseClickValidated().add(this, &PuzzleComputerPwd::onButton0Clicked);
	_gui.buttonLayoutChecked("button1")->onMouseClickValidated().add(this, &PuzzleComputerPwd::onButton1Clicked);
	_gui.buttonLayoutChecked("button2")->onMouseClickValidated().add(this, &PuzzleComputerPwd::onButton2Clicked);
	_gui.buttonLayoutChecked("button3")->onMouseClickValidated().add(this, &PuzzleComputerPwd::onButton3Clicked);
	_gui.buttonLayoutChecked("button4")->onMouseClickValidated().add(this, &PuzzleComputerPwd::onButton4Clicked);
	_gui.buttonLayoutChecked("button5")->onMouseClickValidated().add(this, &PuzzleComputerPwd::onButton5Clicked);
	_gui.buttonLayoutChecked("button6")->onMouseClickValidated().add(this, &PuzzleComputerPwd::onButton6Clicked);
	_gui.buttonLayoutChecked("button7")->onMouseClickValidated().add(this, &PuzzleComputerPwd::onButton7Clicked);
	_gui.buttonLayoutChecked("button8")->onMouseClickValidated().add(this, &PuzzleComputerPwd::onButton8Clicked);
	_gui.buttonLayoutChecked("button9")->onMouseClickValidated().add(this, &PuzzleComputerPwd::onButton9Clicked);
	_gui.buttonLayoutChecked("cancel")->onMouseClickValidated().add(this, &PuzzleComputerPwd::onButtonCancelClicked);
	_gui.buttonLayoutChecked("exit")->onMouseClickValidated().add(this, &PuzzleComputerPwd::onExitButton);
}

bool PuzzleComputerPwd::leave() {
	resetPwd();
	_gui.unload();

	AmerzoneGame *game = dynamic_cast<AmerzoneGame *>(g_engine->getGame());
	assert(game);
	if (game->warpY()) {
		game->warpY()->setVisible(true, false);
	}
	return false;
}

bool PuzzleComputerPwd::onButton0Clicked() {
	return registerNewDigit(0);
}

bool PuzzleComputerPwd::onButton1Clicked() {
	return registerNewDigit(1);
}

bool PuzzleComputerPwd::onButton2Clicked() {
	return registerNewDigit(2);
}

bool PuzzleComputerPwd::onButton3Clicked() {
	return registerNewDigit(3);
}

bool PuzzleComputerPwd::onButton4Clicked() {
	return registerNewDigit(4);
}

bool PuzzleComputerPwd::onButton5Clicked() {
	return registerNewDigit(5);
}

bool PuzzleComputerPwd::onButton6Clicked() {
	return registerNewDigit(6);
}

bool PuzzleComputerPwd::onButton7Clicked() {
	return registerNewDigit(7);
}

bool PuzzleComputerPwd::onButton8Clicked() {
	return registerNewDigit(8);
}

bool PuzzleComputerPwd::onButton9Clicked() {
	return registerNewDigit(9);
}

bool PuzzleComputerPwd::onButtonCancelClicked() {
	if (_nDigits)
		_nDigits--;
	const Common::String sname = Common::String::format("star%d", _nDigits + 1);
	_gui.spriteLayoutChecked(sname)->setVisible(false);
	return false;
}

bool PuzzleComputerPwd::onExitButton() {
	leave();
	return false;
}

bool PuzzleComputerPwd::registerNewDigit(int digit) {
	if (_nDigits == 6)
		return false;
	_enteredPwd[_nDigits] = digit;
	_nDigits++;
	const Common::String sname = Common::String::format("star%d", _nDigits);
	_gui.spriteLayoutChecked(sname)->setVisible(true);
	if (_nDigits == 6) {
		bool match = true;
		for (uint i = 0; i < 6 && match; i++) {
			match &= (CORRECT_PWD[i] == _enteredPwd[i]);
		}

		TeSoundManager *sndMgr = g_engine->getSoundManager();
		if (match) {
			const Common::String snd = _gui.value("goodPassword").toString();
			sndMgr->playFreeSound(snd);
			leave();
			Game *game = g_engine->getGame();
			game->luaScript().execute("OnComputerPwdPuzzleAnswered");
			return true;
		} else {
			const Common::String snd = _gui.value("badPassword").toString();
			sndMgr->playFreeSound(snd);
			resetPwd();
		}
	}
	return false;
}

void PuzzleComputerPwd::resetPwd() {
	for (int i = 1; i < 7; i++) {
		const Common::String sname = Common::String::format("star%d", i);
		_gui.spriteLayoutChecked(sname)->setVisible(false);
	}
	_nDigits = 0;
}


} // end namespace Tetraedge
