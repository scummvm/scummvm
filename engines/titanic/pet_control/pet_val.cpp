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
#include "titanic/core/game_object.h"
#include "titanic/pet_control/pet_val.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

void CPetVal::loadObject(PetElementMode mode, const CString &name,
		CPetControl *petControl) {
	switch (mode) {
	case MODE_0:
		_object0 = petControl->getHiddenObject(name);
		break;
	case MODE_1:
		_object1 = petControl->getHiddenObject(name);
		break;
	case MODE_2:
		_object2 = petControl->getHiddenObject(name);
		break;
	default:
		break;
	}
}

void CPetVal::proc2() {
	error("TODO");
}

void CPetVal::draw(CScreenManager *screenManager) {
	draw(screenManager, Common::Point(_bounds.left, _bounds.top));
}

void CPetVal::draw(CScreenManager *screenManager, const Common::Point &destPos) {
	CGameObject *obj = getObject();
	if (!obj)
		obj = _object0;

	if (obj)
		obj->draw(screenManager, destPos);
}

void CPetVal::getBounds(Rect *rect) {
	if (rect) {
		CGameObject *obj = getObject();
		if (!obj)
			obj = _object0;

		if (obj && obj->getSurface45())
			*rect = _bounds;
		else
			rect->clear();
	}
}

CGameObject *CPetVal::getObject() const {
	switch (_mode) {
	case MODE_0:
		return _object0;
	case MODE_1:
		return _object1;
	case MODE_2:
		return _object2;
	default:
		return nullptr;
	}
}

} // End of namespace Titanic
