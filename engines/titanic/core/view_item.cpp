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

#include "titanic/core/view_item.h"

namespace Titanic {

CViewItem::CViewItem() : CNamedItem() {
	_field24 = 0;
	_field28 = 0.0;
	_viewNumber = 0;
	_field50 = 0;
	_field54 = 0;
	setData(0.0);
}

void CViewItem::setData(double v) {
	_field28 = v;
	_field50 = cos(_field28) * 30.0;
	_field54 = sin(_field28) * -30.0;
}

void CViewItem::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	_resourceKey.save(file, indent);
	file->writeQuotedLine("V", indent);
	file->writeFloatLine(_field28, indent + 1);
	file->writeNumberLine(_viewNumber, indent + 1);

	CNamedItem::save(file, indent);
}

void CViewItem::load(SimpleFile *file) {
	int val = file->readNumber();
	
	switch (val) {
	case 1:
		_resourceKey.load(file);
		// Deliberate fall-through

	default:
		file->readBuffer();
		setData(file->readFloat());
		_viewNumber = file->readNumber();
		break;
	}

	CNamedItem::load(file);
}

} // End of namespace Titanic
