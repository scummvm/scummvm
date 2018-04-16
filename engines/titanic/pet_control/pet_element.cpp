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

#include "common/textconsole.h"
#include "titanic/pet_control/pet_element.h"
#include "titanic/core/game_object.h"

namespace Titanic {

CPetElement::CPetElement() : _mode(MODE_UNSELECTED) {}

Rect CPetElement::getBounds() const {
	return Rect();
}

bool CPetElement::MouseButtonDownMsg(const Point &pt) {
	bool result = _bounds.contains(pt);
	if (result)
		setMode(MODE_SELECTED);
	return result;
}

bool CPetElement::MouseButtonUpMsg(const Point &pt) {
	bool result = _bounds.contains(pt);
	if (result)
		setMode(MODE_UNSELECTED);
	return result;
}

bool CPetElement::MouseDoubleClickMsg(const Point &pt) const {
	return _bounds.contains(pt);
}

bool CPetElement::MouseMoveMsg(const Point &pt) {
	bool result = _bounds.contains(pt);
	if (result)
		setMode(MODE_FOCUSED);
	return result;
}

bool CPetElement::contains2(const Point &pt) const {
	return _bounds.contains(pt);
}

void CPetElement::playMovie(uint startFrame, uint endFrame) const {
	CGameObject *gameObject = getObject();

	if (gameObject)
		gameObject->playMovie(startFrame, endFrame, 0);
}

void CPetElement::changeStatus(int val) const {
	CGameObject *gameObject = getObject();

	if (gameObject)
		gameObject->playMovie(val);
}

bool CPetElement::hasActiveMovie() const {
	CGameObject *gameObject = getObject();
	return gameObject ? gameObject->hasActiveMovie() : false;
}

void CPetElement::loadFrame(int frameNumber) {
	CGameObject *gameObject = getObject();
	if (gameObject)
		gameObject->loadFrame(frameNumber);
}

int CPetElement::getMovieFrame() const {
	CGameObject *gameObject = getObject();
	return gameObject ? gameObject->getMovieFrame() : 0;
}

void CPetElement::setMode(PetElementMode newMode) {
	if (newMode >= MODE_UNSELECTED && newMode <= MODE_FOCUSED)
		changeMode(newMode);
}

void CPetElement::setSelected(bool flag) {
	if (flag)
		changeMode(MODE_SELECTED);
	else
		changeMode(MODE_UNSELECTED);
}

} // End of namespace Titanic
