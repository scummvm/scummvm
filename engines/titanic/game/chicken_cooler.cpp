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

#include "titanic/game/chicken_cooler.h"
#include "titanic/carry/chicken.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CChickenCooler, CGameObject)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

void CChickenCooler::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeNumberLine(_fieldC0, indent);

	CGameObject::save(file, indent);
}

void CChickenCooler::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_fieldC0 = file->readNumber();

	CGameObject::load(file);
}

bool CChickenCooler::EnterRoomMsg(CEnterRoomMsg *msg) {
	if (_fieldC0) {
		CGameObject *obj = getMailManFirstObject();
		if (obj) {
			// WORKAROUND: Redundant loop for chicken in originalhere
		} else {
			getNextMail(nullptr);
			if (CChicken::_v1 > _fieldBC)
				CChicken::_v1 = _fieldBC;
		}
	}

	return true;
}

bool CChickenCooler::EnterViewMsg(CEnterViewMsg *msg) {
	if (!_fieldC0) {
		for (CGameObject *obj = getMailManFirstObject(); obj;
				obj = getNextMail(obj)) {
			if (obj->isEquals("Chicken"))
				return true;
		}

		if (CChicken::_v1 > _fieldBC)
			CChicken::_v1 = _fieldBC;
	}

	return true;
}

} // End of namespace Titanic
