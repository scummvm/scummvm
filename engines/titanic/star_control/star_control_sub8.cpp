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

#include "titanic/star_control/star_control_sub8.h"
#include "titanic/star_control/star_control_sub7.h"
#include "titanic/star_control/star_control_sub12.h"
#include "titanic/star_control/star_field.h"

namespace Titanic {

CStarControlSub8::CStarControlSub8() : _field8(-1) {
#if 0
	_field4(0), _field8(-1), _fieldC(-1)
#endif
}

bool MouseButtonDown(const Common::Point &pt) {
	// TODO
	return true;
}

int CStarControlSub8::findStar(const Common::Point &pt) {
	// TODO
	return -1;
}

void CStarControlSub8::selectStar(int index, CVideoSurface *surface,
		CStarField *starField, CStarControlSub7 *sub7) {
	// TODO
}

void CStarControlSub8::fn1(CStarField *starField, CSurfaceArea *surfaceArea, CStarControlSub12 *sub12) {
	// TODO
}

void CStarControlSub8::fn2(CVideoSurface *surface, CStarField *starField, CStarControlSub7 *sub7) {
	// TODO
}

void CStarControlSub8::fn3() {
	if (_field8 < 3)
		++_field8;
}

FPoint CStarControlSub8::getPosition() const {
	return (_fieldC >= 0 && _fieldC <= 2) ? _data[_fieldC]._position : FPoint();
}

void CStarControlSub8::draw(CSurfaceArea *surfaceArea) {
	// TODO
}

} // End of namespace Titanic
