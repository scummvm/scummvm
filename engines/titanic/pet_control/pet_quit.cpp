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
	tempRect.moveTo(32, 407);
	_text.setBounds(tempRect);
	_text.resize(3);
	_text.setHasBorder(true);
	_text.setup();

	Rect elementRect(0, 0, 496, 388);
	elementRect.moveTo(496, 388);
	_element.setBounds(elementRect);
	
	return true;
}

bool CPetQuit::reset() {
	CPetControl *pet = getPetControl();
	if (!pet)
		return false;

	setName("PetExit", pet);

	uint col = getPetSection()->getColor(0);	
	_text.setText("Are you sure you want to quit?");
	_text.setColor(0, col);

	_btnYes.reset("PetQuitOut", pet, MODE_UNSELECTED);
	_btnYes.reset("PetQuitIn", pet, MODE_SELECTED);

	return true;
}

void CPetQuit::draw2(CScreenManager *screenManager) {
	_text.draw(screenManager);
	_btnYes.draw(screenManager);
}

bool CPetQuit::checkHighlight(const Point &pt) {
	return !_btnYes.highlightBounds(pt);
}

bool CPetQuit::mouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CPetControl *pet = getPetControl();
	if (_btnYes.MouseButtonDownMsg(msg) && pet) {
		CGameManager *gameManager = pet->getGameManager();
		if (gameManager)
			gameManager->_gameState._quitGame = true;
		return true;
	} else {
		return false;
	}
}

void CPetQuit::getTooltip(CPetText *text) {
	text->setText("Quit the game.");
}

} // End of namespace Titanic
