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

#include "titanic/pet_control/pet_real_life.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/pet_control/pet_load.h"
#include "titanic/pet_control/pet_save.h"
#include "titanic/pet_control/pet_sound.h"
#include "titanic/pet_control/pet_quit.h"

namespace Titanic {

bool CPetRealLife::setup(CPetControl *petControl) {
	if (petControl && setupControl(petControl))
		return reset();
	return false;
}

bool CPetRealLife::reset() {
	_glyphs.reset();
	uint col = getColor(0);
	_text.setColor(col);
	_text.setLineColor(0, col);

	return true;
}

void CPetRealLife::draw(CScreenManager *screenManager) {
	_petControl->drawSquares(screenManager, 4);
	_glyphs.draw(screenManager);
	_text.draw(screenManager);
}

bool CPetRealLife::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return _glyphs.MouseButtonDownMsg(msg->_mousePos);
}

bool CPetRealLife::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	return _glyphs.MouseDragStartMsg(msg);
}

bool CPetRealLife::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	return _glyphs.MouseDragMoveMsg(msg);
}

bool CPetRealLife::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	return _glyphs.MouseDragEndMsg(msg);
}

bool CPetRealLife::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	return _glyphs.MouseButtonUpMsg(msg->_mousePos);
}

bool CPetRealLife::KeyCharMsg(CKeyCharMsg *msg) {
	return _glyphs.KeyCharMsg(msg->_key);
}

bool CPetRealLife::VirtualKeyCharMsg(CVirtualKeyCharMsg *msg) {
	return _glyphs.VirtualKeyCharMsg(msg);
}

void CPetRealLife::postLoad() {
	reset();
}

bool CPetRealLife::isValid(CPetControl *petControl) {
	setupControl(petControl);
	return true;
}

void CPetRealLife::enter(PetArea oldArea) {
	_glyphs.enterHighlighted();
}

void CPetRealLife::leave() {
	_glyphs.leaveHighlighted();
}

bool CPetRealLife::setupControl(CPetControl *petControl) {
	if (petControl) {
		_petControl = petControl;
		_glyphs.setup(4, this);
		_glyphs.setFlags(6);

		addButton(new CPetLoad());
		addButton(new CPetSave());
		addButton(new CPetSound());
		addButton(new CPetQuit());

		Rect textRect(0, 0, 276, 30);
		textRect.moveTo(32, 436);
		_text.setBounds(textRect);
		_text.setHasBorder(false);
		_text.setup();
	}

	return true;
}

void CPetRealLife::addButton(CPetGlyph *glyph) {
	if (glyph) {
		if (glyph->setup(_petControl, &_glyphs))
			_glyphs.push_back(glyph);
	}
}

void CPetRealLife::syncSoundSettings() {
	for (CPetGlyphs::iterator i = _glyphs.begin(); i != _glyphs.end(); ++i) {
		CPetSound *sound = dynamic_cast<CPetSound *>(*i);
		if (sound)
			sound->setSliders();
	}
}

} // End of namespace Titanic
