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

#include "titanic/carry/phonograph_cylinder.h"

namespace Titanic {

CPhonographCylinder::CPhonographCylinder() : CCarry(),
	_field138(0), _field13C(0), _field140(0),  _field144(0), 
	_field148(0), _field14C(0), _field150(0), _field154(0), 
	_field158(0), _field15C(0), _field160(0), _field164(0), 
	_field168(0), _field16C(0), _field170(0), _field174(0), 
	_field178(0), _field17C(0), _field180(0), _field184(0) {
}

void CPhonographCylinder::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string6, indent);
	file->writeNumberLine(_field138, indent);
	file->writeNumberLine(_field13C, indent);
	file->writeNumberLine(_field140, indent);
	file->writeNumberLine(_field144, indent);
	file->writeNumberLine(_field148, indent);
	file->writeNumberLine(_field14C, indent);
	file->writeNumberLine(_field150, indent);
	file->writeNumberLine(_field154, indent);
	file->writeNumberLine(_field158, indent);
	file->writeNumberLine(_field15C, indent);
	file->writeNumberLine(_field160, indent);
	file->writeNumberLine(_field164, indent);
	file->writeNumberLine(_field168, indent);
	file->writeNumberLine(_field16C, indent);
	file->writeNumberLine(_field170, indent);
	file->writeNumberLine(_field174, indent);
	file->writeNumberLine(_field178, indent);
	file->writeNumberLine(_field17C, indent);
	file->writeNumberLine(_field180, indent);
	file->writeNumberLine(_field184, indent);

	CCarry::save(file, indent);
}

void CPhonographCylinder::load(SimpleFile *file) {
	file->readNumber();
	_string6 = file->readString();
	_field138 = file->readNumber();
	_field13C = file->readNumber();
	_field140 = file->readNumber();
	_field144 = file->readNumber();
	_field148 = file->readNumber();
	_field14C = file->readNumber();
	_field150 = file->readNumber();
	_field154 = file->readNumber();
	_field158 = file->readNumber();
	_field15C = file->readNumber();
	_field160 = file->readNumber();
	_field164 = file->readNumber();
	_field168 = file->readNumber();
	_field16C = file->readNumber();
	_field170 = file->readNumber();
	_field174 = file->readNumber();
	_field178 = file->readNumber();
	_field17C = file->readNumber();
	_field180 = file->readNumber();
	_field184 = file->readNumber();

	CCarry::load(file);
}

} // End of namespace Titanic
