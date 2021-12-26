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

#include "titanic/game/chicken_cooler.h"
#include "titanic/carry/chicken.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CChickenCooler, CGameObject)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

void CChickenCooler::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_newTemperature, indent);
	file->writeNumberLine(_triggerOnRoomEntry, indent);

	CGameObject::save(file, indent);
}

void CChickenCooler::load(SimpleFile *file) {
	file->readNumber();
	_newTemperature = file->readNumber();
	_triggerOnRoomEntry = file->readNumber();

	CGameObject::load(file);
}

bool CChickenCooler::EnterRoomMsg(CEnterRoomMsg *msg) {
	if (_triggerOnRoomEntry) {
		CGameObject *obj = getMailManFirstObject();
		if (!obj) {
			if (CChicken::_temperature > _newTemperature)
				CChicken::_temperature = _newTemperature;
		}
	}

	return true;
}

bool CChickenCooler::EnterViewMsg(CEnterViewMsg *msg) {
	if (!_triggerOnRoomEntry) {
		for (CGameObject *obj = getMailManFirstObject(); obj;
				obj = getNextMail(obj)) {
			if (obj->isEquals("Chicken"))
				return true;
		}

		if (CChicken::_temperature > _newTemperature)
			CChicken::_temperature = _newTemperature;
	}

	return true;
}

} // End of namespace Titanic
