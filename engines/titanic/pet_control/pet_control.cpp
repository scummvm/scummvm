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

#include "titanic/pet_control/pet_control.h"

namespace Titanic {

void CPetControl::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(0, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeQuotedLine(_string2, indent);

	saveSubObjects(file, indent);
	CGameObject::save(file, indent);
}

void CPetControl::load(SimpleFile *file) {
	int val = file->readNumber();
	bool valid = isValid();
	
	if (!valid) {
		_fieldBC = file->readNumber();
		_string1 = file->readString();
		_string2 = file->readString();
		
		loadSubObjects(file, 0);
	}

	CGameObject::load(file);
}

void CPetControl::postLoad() {
	// TODO
}

bool CPetControl::isValid() const {
	return _sub1.isValid() && _sub2.isValid()
		&& _sub3.isValid() && _sub4.isValid()
		&& _sub5.isValid() && _sub6.isValid()
		&& _sub7.isValid() && _sub8.isValid();
}

void CPetControl::loadSubObjects(SimpleFile *file, int param) {
	_sub1.load(file, param);
	_sub2.load(file, param);
	_sub3.load(file, param);
	_sub4.load(file, param);
	_sub5.load(file, param);
	_sub6.load(file, param);
	_sub7.load(file, param);
	_sub8.load(file, param);
}

void CPetControl::saveSubObjects(SimpleFile *file, int indent) const {
	_sub1.save(file, indent);
	_sub2.save(file, indent);
	_sub3.save(file, indent);
	_sub4.save(file, indent);
	_sub5.save(file, indent);
	_sub6.save(file, indent);
	_sub7.save(file, indent);
	_sub8.save(file, indent);
}


} // End of namespace Titanic
