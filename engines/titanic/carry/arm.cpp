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

#include "titanic/carry/arm.h"

namespace Titanic {

CArm::CArm() : CCarry(), _string6("Key"),
	_field138(0), _field13C(0), _field140(0), _field144(0),
	_field148(0), _field158(0), _field15C(220), _field160(208),
	_field164(409), _field168(350), _field16C(3), _field170(0) {
}

void CArm::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string6, indent);
	file->writeNumberLine(_field138, indent);
	file->writeNumberLine(_field13C, indent);
	file->writeNumberLine(_field140, indent);
	file->writeNumberLine(_field144, indent);
	file->writeNumberLine(_field148, indent);

	file->writeQuotedLine(_string7, indent);
	file->writeNumberLine(_field158, indent);
	file->writeNumberLine(_field15C, indent);
	file->writeNumberLine(_field160, indent);
	file->writeNumberLine(_field164, indent);
	file->writeNumberLine(_field168, indent);
	file->writeNumberLine(_field16C, indent);
	file->writeNumberLine(_field170, indent);

	CCarry::save(file, indent);
}

void CArm::load(SimpleFile *file) {
	file->readNumber();
	_string6 = file->readString();
	_field138 = file->readNumber();
	_field13C = file->readNumber();
	_field140 = file->readNumber();
	_field144 = file->readNumber();
	_field148 = file->readNumber();

	_string7 = file->readString();
	_field158 = file->readNumber();
	_field15C = file->readNumber();
	_field160 = file->readNumber();
	_field164 = file->readNumber();
	_field168 = file->readNumber();
	_field16C = file->readNumber();
	_field170 = file->readNumber();

	CCarry::load(file);
}

} // End of namespace Titanic
