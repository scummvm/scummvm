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

#include "titanic/carry/chicken.h"

namespace Titanic {

int CChicken::_v1;

CChicken::CChicken() : CCarry(), _string6("None"),
		_field12C(1), _field13C(0), _field140(0) {
}

void CChicken::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field12C, indent);
	file->writeQuotedLine(_string6, indent);
	file->writeNumberLine(_field13C, indent);
	file->writeNumberLine(_field140, indent);

	CCarry::save(file, indent);
}

void CChicken::load(SimpleFile *file) {
	file->readNumber();
	_field12C = file->readNumber();
	_string6 = file->readString();
	_field13C = file->readNumber();
	_field140 = file->readNumber();

	CCarry::load(file);
}

} // End of namespace Titanic
