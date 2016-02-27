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

#include "titanic/carry/carry.h"

namespace Titanic {

CCarry::CCarry() : CGameObject(), _fieldDC(0), _fieldE0(1),
	_field100(0), _field104(0), _field108(0), _field10C(0),
	_field110(0), _field120(0), _field124(0), _field128(0),
	_string1("None"),
	_string2("NULL"),
	_string3("That doesn't seem to do anything."),
	_string4("It doesn't seem to want this.") {
}

void CCarry::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string1, indent);
	file->writePoint(_pos1, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_fieldDC, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeQuotedLine(_string3, indent);
	file->writeQuotedLine(_string4, indent);
	file->writePoint(_pos2, indent);
	file->writeNumberLine(_field104, indent);
	file->writeNumberLine(_field108, indent);
	file->writeNumberLine(_field10C, indent);
	file->writeNumberLine(_field110, indent);
	file->writeQuotedLine(_string5, indent);
	file->writeNumberLine(_field120, indent);
	file->writeNumberLine(_field124, indent);
	file->writeNumberLine(_field128, indent);

	CGameObject::save(file, indent);
}

void CCarry::load(SimpleFile *file) {
	file->readNumber();
	_string1 = file->readString();
	_pos1 = file->readPoint();
	_string2 = file->readString();
	_fieldDC = file->readNumber();
	_fieldE0 = file->readNumber();
	_string3 = file->readString();
	_string4 = file->readString();
	_pos2 = file->readPoint();
	_field104 = file->readNumber();
	_field108 = file->readNumber();
	_field10C = file->readNumber();
	_field110 = file->readNumber();
	_string5 = file->readString();
	_field120 = file->readNumber();
	_field124 = file->readNumber();
	_field128 = file->readNumber();

	CGameObject::load(file);
}

} // End of namespace Titanic
