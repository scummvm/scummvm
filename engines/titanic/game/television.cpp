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

#include "titanic/game/television.h"

namespace Titanic {

int CTelevision::_v1;
int CTelevision::_v2;
int CTelevision::_v3;
int CTelevision::_v4;
int CTelevision::_v5;
int CTelevision::_v6;

CTelevision::CTelevision() : CBackground(), _fieldE0(1),
	_fieldE4(7), _fieldE8(0), _fieldEC(0), _fieldF0(0) {
}

void CTelevision::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_v2, indent);
	file->writeNumberLine(_fieldE8, indent);
	file->writeNumberLine(_v3, indent);
	file->writeNumberLine(_fieldEC, indent);
	file->writeNumberLine(_v4, indent);
	file->writeNumberLine(_fieldF0, indent);
	file->writeNumberLine(_v5, indent);
	file->writeNumberLine(_v6, indent);

	CBackground::save(file, indent);
}

void CTelevision::load(SimpleFile *file) {
	file->readNumber();
	_fieldE0 = file->readNumber();
	_v1 = file->readNumber();
	_fieldE4 = file->readNumber();
	_v2 = file->readNumber();
	_fieldE8 = file->readNumber();
	_v3 = file->readNumber();
	_fieldEC = file->readNumber();
	_v4 = file->readNumber();
	_fieldF0 = file->readNumber();
	_v5 = file->readNumber();
	_v6 = file->readNumber();

	CBackground::load(file);
}

} // End of namespace Titanic
