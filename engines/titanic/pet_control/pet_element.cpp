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

void CPetElement::getBounds(Rect *rect) {
	if (rect)
		*rect = Rect();
}

bool CPetElement::proc6(const Common::Point &pt) {
	bool result = _bounds.contains(pt);
	if (result)
		setMode(MODE_SELECTED);
	return result;
}

bool CPetElement::proc7(const Common::Point &pt) {
	bool result = _bounds.contains(pt);
	if (result)
		setMode(MODE_UNSELECTED);
	return result;
}

bool CPetElement::contains1(const Common::Point &pt) const {
	return _bounds.contains(pt);
}

int CPetElement::proc9(const Common::Point &pt) {
	bool result = _bounds.contains(pt);
	if (result)
		setMode(MODE_2);
	return result;
}

bool CPetElement::contains2(const Common::Point &pt) const {
	return _bounds.contains(pt);
}

void CPetElement::proc11(int val1, int val2) const {
	CGameObject *gameObject = getObject();

	if (gameObject)
		gameObject->fn1(val1, val2, 0);
}

void CPetElement::changeStatus(int newStatus) const {
	CGameObject *gameObject = getObject();

	if (gameObject)
		gameObject->changeStatus(newStatus);
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

int CPetElement::proc15() {
	CGameObject *gameObject = getObject();
	return gameObject ? gameObject->getMovie19() : 0;
}

void CPetElement::setMode(PetElementMode newMode) {
	if (newMode >= MODE_UNSELECTED && newMode <= MODE_2)
		changeMode(newMode);
}

} // End of namespace Titanic
