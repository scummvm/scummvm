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

#include "titanic/game/pet/pet_sentinal.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPETSentinal, CGameObject)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

CPETSentinal::CPETSentinal() : CGameObject(), _elevatorNum(0),
		_wellEntry(0), _resetHighlight(0) {
}

void CPETSentinal::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_elevatorNum, indent);
	file->writeNumberLine(_wellEntry, indent);
	file->writeNumberLine(_resetHighlight, indent);
	CGameObject::save(file, indent);
}

void CPETSentinal::load(SimpleFile *file) {
	file->readNumber();
	_elevatorNum = file->readNumber();
	_wellEntry = file->readNumber();
	_resetHighlight = file->readNumber();
	CGameObject::load(file);
}

bool CPETSentinal::EnterViewMsg(CEnterViewMsg *msg) {
	CPetControl *pet = getPetControl();
	if (pet) {
		if (_elevatorNum != -1)
			pet->setRoomsElevatorNum(_elevatorNum);
		if (_wellEntry)
			pet->setRoomsWellEntry(_wellEntry);
		if (_resetHighlight)
			pet->resetRoomsHighlight();
	}

	return true;
}

} // End of namespace Titanic
