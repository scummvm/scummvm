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

#include "titanic/star_control/star_field.h"
#include "titanic/star_control/surface_area.h"

namespace Titanic {

CStarField::CStarField() : _val1(0), _val2(0), _val3(0), _val4(true),
	_val5(0), _val6(false) {
}

void CStarField::load(SimpleFile *file) {
	_sub7.load(file);
	_sub8.load(file);
	_val1 = file->readNumber();
	_val2 = file->readNumber();
	_val3 = file->readNumber();
	_val4 = file->readNumber();
	_val6 = file->readNumber();
}

void CStarField::save(SimpleFile *file, int indent) {
	_sub7.save(file, indent);
	_sub8.save(file, indent);
	file->writeNumberLine(_val1, indent);
	file->writeNumberLine(_val2, indent);
	file->writeNumberLine(_val3, indent);
	file->writeNumberLine(_val4, indent);
	file->writeNumberLine(_val6, indent);
}

bool CStarField::initDocument() {
	bool valid = setup() && _points1.initialize();
	if (valid)
		valid = _sub5.setup();
	if (valid)
		valid = _points1.initialize();
	if (valid)
		valid = _points2.initialize();

	return valid;
}

void CStarField::render(CVideoSurface *surface, CStarControlSub12 *sub12) {
	CSurfaceArea surfaceArea(surface);
	draw(&surfaceArea, sub12, &_sub5);


	// TODO
}

int CStarField::get1() const {
	return _val1;
}

void CStarField::set1(int val) {
	_val1 = val;
}

int CStarField::get2() const {
	return _val2;
}

void CStarField::set2(int val) {
	_val2 = val;
}

int CStarField::get54() const {
	return _sub5.get4();
}

void CStarField::set54(int val) {
	_sub5.set4(val);
}

int CStarField::get3() const {
	return _val3;
}

void CStarField::set3(int val) {
	_val3 = val;
}

void CStarField::toggle4() {
	_val4 = !_val4;
}

bool CStarField::set4(bool val) {
	bool oldVal = _val4;
	_val4 = val;
	return oldVal;
}

int CStarField::get88() const {
	return _sub8._field8;
}

int CStarField::get5() const {
	return _val5;
}

void CStarField::update6() {
	_val6 = _sub8._field8 == 2;
}

int CStarField::get6() const {
	return _val6;
}

} // End of namespace Titanic
