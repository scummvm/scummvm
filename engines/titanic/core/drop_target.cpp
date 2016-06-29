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

#include "titanic/core/drop_target.h"

namespace Titanic {

CDropTarget::CDropTarget() : CGameObject(), _fieldC4(0),
	_fieldD4(0), _fieldE4(0), _fieldF4(0), _fieldF8(0),
	_fieldFC(0), _field10C(1), _field110(8), _field114(20) {
}

void CDropTarget::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writePoint(_pos1, indent);
	file->writeNumberLine(_fieldC4, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeNumberLine(_fieldD4, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeQuotedLine(_string3, indent);
	file->writeNumberLine(_fieldF4, indent);
	file->writeNumberLine(_fieldF8, indent);
	file->writeNumberLine(_fieldFC, indent);
	file->writeQuotedLine(_string4, indent);
	file->writeNumberLine(_field10C, indent);
	file->writeNumberLine(_field110, indent);
	file->writeNumberLine(_field114, indent);

	CGameObject::save(file, indent);
}

void CDropTarget::load(SimpleFile *file) {
	file->readNumber();
	_pos1 = file->readPoint();
	_fieldC4 = file->readNumber();
	_string1 = file->readString();
	_fieldD4 = file->readNumber();
	_string2 = file->readString();
	_fieldE4 = file->readNumber();
	_string3 = file->readString();
	_fieldF4 = file->readNumber();
	_fieldF8 = file->readNumber();
	_fieldFC = file->readNumber();
	_string4 = file->readString();
	_field10C = file->readNumber();
	_field110 = file->readNumber();
	_field114 = file->readNumber();

	CGameObject::load(file);
}

} // End of namespace Titanic
