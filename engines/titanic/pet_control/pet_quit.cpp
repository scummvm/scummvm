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

#include "titanic/pet_control/pet_quit.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/pet_control/pet_real_life.h"
#include "titanic/support/rect.h"
#include "titanic/game_manager.h"

namespace Titanic {

bool CPetQuit::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CPetGlyph::setup(petControl, owner);

	Rect tempRect(0, 0, 280, 16);
	tempRect.moveTo(322, 407);
	_text.setBounds(tempRect);
	_text.resize(3);
	_text.setHasBorder(false);
	_text.setup();

	Rect btnRect(0, 0, 68, 52);
	btnRect.moveTo(496, 388);
	_btnYes.setBounds(btnRect);

	return true;
}

bool CPetQuit::reset() {
	CPetControl *pet = getPetControl();
	if (!pet)
		return false;

	setName("PetExit", pet);

	uint col = getPetSection()->getColor(0);
	_text.setText(SURE_YOU_WANT_TO_QUIT);
	_text.setLineColor(0, col);

	_btnYes.reset("PetQuitOut", pet, MODE_UNSELECTED);
	_btnYes.reset("PetQuitIn", pet, MODE_SELECTED);

	return true;
}

void CPetQuit::draw2(CScreenManager *screenManager) {
	_text.draw(screenManager);
	_btnYes.draw(screenManager);
}

bool CPetQuit::MouseButtonDownMsg(const Point &pt) {
	return !_btnYes.MouseButtonDownMsg(pt);
}

bool CPetQuit::MouseButtonUpMsg(const Point &pt) {
	CPetControl *pet = getPetControl();
	if (_btnYes.MouseButtonUpMsg(pt) && pet) {
		CGameManager *gameManager = pet->getGameManager();
		if (gameManager)
			gameManager->_gameState._quitGame = true;
		return true;
	} else {
		return false;
	}
}

void CPetQuit::getTooltip(CTextControl *text) {
	text->setText(QUIT_THE_GAME);
}

} // End of namespace Titanic
