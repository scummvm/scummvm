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

#include "titanic/core/background.h"

namespace Titanic {

CBackground::CBackground() : CGameObject(), _fieldBC(0), _fieldC0(0), _fieldDC(0) {
}

void CBackground::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeNumberLine(_fieldC0, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_fieldDC, indent);

	CGameObject::save(file, indent);
}

void CBackground::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_fieldC0 = file->readNumber();
	_string1 = file->readString();
	_string2 = file->readString();
	_fieldDC = file->readNumber();

	CGameObject::load(file);
}

bool CBackground::handleMessage(CStatusChangeMsg &msg) {
	setVisible(true);
	if (_fieldDC) {
		fn1(_fieldBC, _fieldC0, 16);
	} else {
		fn1(_fieldBC, _fieldC0, 0);
	}
	return true;
}

bool CBackground::handleMessage(CSetFrameMsg &msg) {
	loadFrame(msg._frameNumber);
	return true;
}

bool CBackground::handleMessage(CVisibleMsg &msg) {
	setVisible(msg._visible);
	return true;
}

} // End of namespace Titanic
