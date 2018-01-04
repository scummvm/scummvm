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

#include "titanic/star_control/matrix_transform.h"
#include "titanic/star_control/fpose.h"
#include "common/textconsole.h"

namespace Titanic {

void CMatrixTransform::setup(double val1, double x, double y, double z) {
	_field0 = val1;
	_vector._x = x;
	_vector._y = y;
	_vector._z = z;
}

void CMatrixTransform::copyFrom(const CMatrixTransform &src) {
	_field0 = src._field0;
	_vector = src._vector;
}

double CMatrixTransform::fn1() const {
	return _vector._x * _vector._x + _vector._y * _vector._y +
		_vector._z * _vector._z + _field0 * _field0;
}

double CMatrixTransform::fn2(const CMatrixTransform &src) {
	return _field0 * src._field0 + _vector._x * src._vector._x
		+ _vector._y * src._vector._y + _vector._z * src._vector._z;
}

CMatrixTransform CMatrixTransform::resize(double factor) const {
	CMatrixTransform dest;
	dest.setup(_field0 * factor, _vector._x * factor, _vector._y * factor,
		_vector._z * factor);
	return dest;
}

void CMatrixTransform::fn4(const FMatrix &m) {
	double total = m._row1._x + m._row3._z + m._row2._y + 1.0;

	if (total <= 0.00001) {
		total = m._row3._z;

		if (m._row1._x <= m._row3._z) {
			if (m._row2._y > total)
				total = m._row2._y;
		} else if (m._row1._x > total) {
			total = m._row1._x;
		}

		if (total == m._row1._x) {
			double val1 = sqrt(m._row1._x - -1.0 - m._row2._y - m._row3._z);
			double val2 = 0.5 / val1;
			_vector._x = val1 * 0.5;
			_field0 = (m._row2._z - m._row3._y) * val2;
			_vector._y = (m._row2._x + m._row1._y) * val2;
			_vector._z = (m._row3._x + m._row1._z) * val2;
		} else if (total == m._row2._y) {
			double val1 = sqrt(m._row2._y - -1.0 - m._row3._z - m._row1._x);
			double val2 = 0.5 / val1;
			_vector._y = val1 * 0.5;
			_field0 = (m._row3._x - m._row1._z) * val2;
			_vector._z = (m._row3._y + m._row2._z) * val2;
			_vector._x = (m._row2._x + m._row1._y) * val2;
		} else if (total == m._row3._z) {
			double val1 = sqrt(m._row3._z - -1.0 - m._row1._x - m._row2._y);
			double val2 = 0.5 / val1;
			_vector._z = val1 * 0.5;
			_field0 = (m._row1._y - m._row2._x) * val2;
			_vector._x = (m._row3._x + m._row1._z) * val2;
			_vector._y = (m._row3._y + m._row2._z) * val2;
		}
	} else {
		double val1 = 0.5 / sqrt(total);
		_field0 = sqrt(total) * 0.5;
		_vector._x = (m._row2._z - m._row3._y) * val1;
		_vector._y = (m._row3._x - m._row1._z) * val1;
		_vector._z = (m._row1._y - m._row2._x) * val1;
	}
}

CMatrixTransform CMatrixTransform::fn5(double percent, const CMatrixTransform &src) {
	CMatrixTransform sub1 = *this;
	CMatrixTransform sub2, sub4;
	CMatrixTransform dest;
	double val1 = sub1.fn2(src);

	if (val1 < 0.0) {
		val1 = -val1;
		sub2.setup(-sub1._field0, -sub1._vector._x, -sub1._vector._y, -sub1._vector._z);
		sub1 = sub2;
	}

	if (val1 + 1.0 <= 0.00001) {
		dest._vector._x = -sub1._vector._y;
		dest._vector._y = sub1._vector._x;
		dest._vector._z = -sub1._field0;
		dest._field0 = sub1._vector._z;

		double sin1 = sin(percent * M_PI);
		double sin2 = sin((0.5 - percent) * M_PI);
		dest._vector._x = sin1 * dest._vector._x + sub1._vector._x * sin2;
		dest._vector._y = sub1._vector._y * sin2 + sub1._vector._x * sin1;
		dest._vector._z = sin1 * -sub1._field0 + sub1._vector._z * sin2;
		return dest;
	}

	CMatrixTransform t1, t2;
	double val2;

	if (1.0 - val1 <= 0.00001) {
		val2 = 1.0 - percent;
		t1 = src.resize(percent);
	} else {
		double cosVal = acos(val1);
		double sinVal = sin(cosVal);
		val2 = sin((1.0 - percent) * cosVal) / sinVal;
		t1 = src.resize(sin(cosVal * percent) / sinVal);
	}

	t2 = sub1.resize(val2);
	dest.setup(t2._field0 + t1._field0, t1._vector._x + t2._vector._x,
		t1._vector._y + t2._vector._y, t1._vector._z + t2._vector._z);
	return dest;
}

} // End of namespace Titanic
