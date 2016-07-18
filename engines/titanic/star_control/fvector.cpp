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

#include "titanic/star_control/fvector.h"
#include "titanic/star_control/star_control_sub6.h"
#include "common/algorithm.h"
#include "common/textconsole.h"

namespace Titanic {

void FVector::fn1(FVector *v) {
	v->_x = (ABS(_x - _y) < 0.00001 && ABS(_y - _z) < 0.00001 &&
		ABS(_x - _z) < 0.00001) ? -_x : _x;
	v->_y = _y;
	v->_z = _z;
}

void FVector::multiply(FVector *dest, const FVector *src) {
	dest->_x = (src->_z * _y) - (_z * src->_y);
	dest->_y = (src->_x * _z) - (_x * src->_z);
	dest->_z = (src->_y * _x) - (_y * src->_x);
}

void FVector::fn3() {
	double hyp = sqrt(_x * _x + _y * _y + _z * _z);
	assert(hyp);

	_x *= 1.0 / hyp;
	_y *= 1.0 / hyp;
	_z *= 1.0 / hyp;
}

double FVector::getDistance(const FVector *src) const {
	double xd = src->_x - _x;
	double yd = src->_y - _y;
	double zd = src->_z - _z;

	return sqrt(xd * xd + yd * yd + zd * zd);
}

void FVector::fn4(FVector *dest, const FVector *v1, const FVector *v2) {
	FVector tempVector(v1->_x + v2->_x, v1->_y + v2->_y, v1->_z + v2->_z);
	tempVector.fn3();

	*dest = tempVector;
}

void FVector::fn5(FVector *dest, const CStarControlSub6 *sub6) const {
	error("TODO: FVector::fn5");
}

} // End of namespace Titanic
