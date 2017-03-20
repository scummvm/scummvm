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

#include "titanic/star_control/dvector.h"
#include "common/algorithm.h"

namespace Titanic {

void DVector::normalize() {
	double hyp = sqrt(_x * _x + _y * _y + _z * _z);
	assert(hyp);

	_x *= 1.0 / hyp;
	_y *= 1.0 / hyp;
	_z *= 1.0 / hyp;
}

double DVector::getDistance(const DVector &src) {
	return sqrt((src._x - _x) * (src._x - _x) + (src._y - _y) * (src._y - _y) + (src._z - _z) * (src._z - _z));
}

DVector *DVector::fn1(DVector &dest, const DMatrix &m) {
	// TODO
	return nullptr;
}

void DVector::fn2(double val) {
	// TODO
}

void DVector::fn3(DVector &dest) {
	// TODO
}

const DMatrix *DVector::fn4(const DVector &v, DMatrix &m) {
	// TODO
	return nullptr;
}

void DVector::fn5(DMatrix &dest) {
	// TODO
}

} // End of namespace Titanic
