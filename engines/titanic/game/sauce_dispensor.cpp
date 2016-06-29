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

#include "titanic/game/sauce_dispensor.h"

namespace Titanic {

CSauceDispensor::CSauceDispensor() : CBackground(),
		_fieldEC(0), _fieldF0(0), _field104(0), _field108(0) {
}

void CSauceDispensor::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string3, indent);
	file->writeNumberLine(_fieldEC, indent);
	file->writeNumberLine(_fieldF0, indent);
	file->writePoint(_pos1, indent);
	file->writePoint(_pos2, indent);
	file->writeNumberLine(_field104, indent);
	file->writeNumberLine(_field108, indent);

	CBackground::save(file, indent);
}

void CSauceDispensor::load(SimpleFile *file) {
	file->readNumber();
	_string3 = file->readString();
	_fieldEC = file->readNumber();
	_fieldF0 = file->readNumber();
	_pos1 = file->readPoint();
	_pos2 = file->readPoint();
	_field104 = file->readNumber();
	_field108 = file->readNumber();

	CBackground::load(file);
}

} // End of namespace Titanic
