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

#include "titanic/carry/photograph.h"

namespace Titanic {

int CPhotograph::_v1;

CPhotograph::CPhotograph() : CCarry(), _field12C(0), _field130(0) {
}

void CPhotograph::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field12C, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_field130, indent);

	CCarry::save(file, indent);
}

void CPhotograph::load(SimpleFile *file) {
	file->readNumber();
	_field12C = file->readNumber();
	_v1 = file->readNumber();
	_field130 = file->readNumber();

	CCarry::load(file);
}

} // End of namespace Titanic
