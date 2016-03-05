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

#include "titanic/core/link_item.h"

namespace Titanic {

void CLinkItemSub::clear() {
	_field0 = 0;
	_field4 = 0;
	_field8 = 0;
	_fieldC = 0;
}

/*------------------------------------------------------------------------*/

CLinkItem::CLinkItem() : CNamedItem() {
	_field24 = -1;
	_field28 = -1;
	_field2C = -1;
	_field30 = 0;
	_field34 = 1;
	_name = "Link";
}

void CLinkItem::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(2, indent);
	file->writeQuotedLine("L", indent);
	file->writeNumberLine(_field34, indent + 1);
	file->writeNumberLine(_field30, indent + 1);
	file->writeNumberLine(_field24, indent + 1);
	file->writeNumberLine(_field28, indent + 1);
	file->writeNumberLine(_field2C, indent + 1);

	file->writeQuotedLine("Hotspot", indent + 1);
	file->writeNumberLine(_sub._field0, indent + 2);
	file->writeNumberLine(_sub._field4, indent + 2);
	file->writeNumberLine(_sub._field8, indent + 2);
	file->writeNumberLine(_sub._fieldC, indent + 2);

	CNamedItem::save(file, indent);
}

void CLinkItem::load(SimpleFile *file) {
	int val = file->readNumber();
	file->readBuffer();

	switch (val) {
	case 2:
		_field34 = file->readNumber();
		// Deliberate fall-through

	case 1:
		_field30 = file->readNumber();
		// Deliberate fall-through

	case 0:
		_field24 = file->readNumber();
		_field28 = file->readNumber();
		_field2C = file->readNumber();

		file->readBuffer();
		_sub._field0 = file->readNumber();
		_sub._field4 = file->readNumber();
		_sub._field8 = file->readNumber();
		_sub._fieldC = file->readNumber();
		break;

	default:
		break;
	}

	CNamedItem::load(file);

	if (val < 2) {
		switch (_field30) {
		case 2:
			_field34 = 2;
			break;
		case 3:
			_field34 = 3;
			break;
		case 5:
			_field34 = 7;
			break;
		default:
			_field34 = 4;
			break;
		}
	}
}

} // End of namespace Titanic
