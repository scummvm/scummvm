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

#include "titanic/game/head_slot.h"

namespace Titanic {

int CHeadSlot::_v1;

CHeadSlot::CHeadSlot() : CGameObject(), _string1("NotWorking"), _string2("NULL"),
	_fieldBC(0), _fieldD8(0), _fieldDC(27), _fieldE0(56),
	_fieldE4(82), _fieldE8(112), _fieldEC(0) {
}

void CHeadSlot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_fieldD8, indent);
	file->writeNumberLine(_fieldDC, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_fieldE8, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_fieldEC, indent);

	CGameObject::save(file, indent);
}

void CHeadSlot::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_string1 = file->readString();
	_string2 = file->readString();
	_fieldD8 = file->readNumber();
	_fieldDC = file->readNumber();
	_fieldE0 = file->readNumber();
	_fieldE4 = file->readNumber();
	_fieldE8 = file->readNumber();
	_v1 = file->readNumber();
	_fieldEC = file->readNumber();

	CGameObject::load(file);
}

} // End of namespace Titanic
