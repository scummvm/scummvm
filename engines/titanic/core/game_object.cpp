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

#include "titanic/core/game_object.h"
#include "titanic/core/resource_key.h"

namespace Titanic {

CGameObject::CGameObject(): CNamedItem() {
	_bounds = Common::Rect(0, 0, 15, 15);
	_field34 = 0;
	_field38 = 0;
	_field3C = 0;
	_field40 = 0;
	_field44 = 0xF0;
	_field48 = 0xF0;
	_field4C = 0xFF;
	_field50 = 0;
	_field54 = 0;
	_field58 = 0;
	_field5C = 1;
	_field60 = 0;
	_cursorId = 1;
	_field78 = 0;
	_field8C = -1;
	_field90 = 0;
	_field94 = 0;
	_field98 = 0;
	_field9C = 0;
	_fieldA0 = 0;
	_fieldA4 = 0;
	_fieldA8 = nullptr;
	_fieldB8 = 0;
}

void CGameObject::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(7, indent);
	error("TODO: CGameObject::save");

	CNamedItem::save(file, indent);
}

void CGameObject::load(SimpleFile *file) {
	int val = file->readNumber();
	CResourceKey resourceKey;

	switch (val) {
	case 7:
		_clipList2.load(file);
		_field8C = file->readNumber();
		// Deliberate fall-through

	case 6:
		val = _cursorId = file->readNumber();
		// Deliberate fall-through

	case 5:
		_clipList1.load(file);
		// Deliberate fall-through

	case 4:
		_field60 = file->readNumber();
		// Deliberate fall-through

	case 3:
		_field40 = file->readNumber();
		// Deliberate fall-through

	case 2:
		_string = file->readString();
		// Deliberate fall-through

	case 1:
		_bounds = file->readRect();
		_field34 = file->readFloat();
		_field38 = file->readFloat();
		_field3C = file->readFloat();
		_field44 = file->readNumber();
		_field48 = file->readNumber();
		_field4C = file->readNumber();
		_fieldB8 = file->readNumber();
		_field5C = file->readNumber();
		_field50 = file->readNumber();
		_field54 = file->readNumber();
		_field58 = file->readNumber();

		resourceKey.load(file);		
		_fieldA8 = nullptr;
		val = file->readNumber();
		if (val) {
			_string = resourceKey.getString();
		}
		break;

	default:
		break;
	}

	CNamedItem::load(file);
}

void CGameObject::fn2() {
	error("TODO");
}

bool CGameObject::checkPoint(const Common::Point &pt, int v0, int v1) {
	warning("TODO: CGameObject::checkPoint");
	return false;
}

} // End of namespace Titanic
