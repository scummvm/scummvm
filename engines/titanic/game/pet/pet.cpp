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

#include "titanic/game/pet/pet.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPET, CGameObject)
	ON_MESSAGE(ShowTextMsg)
END_MESSAGE_MAP()

CPET::CPET() : CGameObject(), _fieldBC(0), _fieldC0(3),
	_fieldC4(0), _fieldC8(0), _fieldD8(0), _fieldDC(0) {
}

void CPET::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeNumberLine(_fieldC0, indent);
	file->writeNumberLine(_fieldC4, indent);
	file->writeNumberLine(_fieldC8, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeNumberLine(_fieldD8, indent);
	file->writeNumberLine(_fieldDC, indent);

	CGameObject::save(file, indent);
}

void CPET::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_fieldC0 = file->readNumber();
	_fieldC4 = file->readNumber();
	_fieldC8 = file->readNumber();
	_string1 = file->readString();
	_fieldD8 = file->readNumber();
	_fieldDC = file->readNumber();

	CGameObject::load(file);
}

bool CPET::ShowTextMsg(CShowTextMsg *msg) {
	CPetControl *pet = getPetControl();
	if (pet)
		pet->petDisplayMessage(1, msg->_message);
	return true;
}

} // End of namespace Titanic
