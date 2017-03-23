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

#include "titanic/star_control/star_control_sub26.h"
#include "common/textconsole.h"

namespace Titanic {

void CStarControlSub26::setup(double val1, double val2, double val3, double val4) {
	_field0 = val1;
	_sub._v1 = val2;
	_sub._v2 = val3;
	_sub._v3 = val4;
}

void CStarControlSub26::copyFrom(const CStarControlSub26 *src) {
	_field0 = src->_field0;
	_sub = src->_sub;
}

double CStarControlSub26::fn1() const {
	return _sub._v1 * _sub._v1 + _sub._v2 * _sub._v2 +
		_sub._v3 * _sub._v3 + _field0 * _field0;
}

double CStarControlSub26::fn2(const CStarControlSub26 *src) {
	return _field0 * src->_field0 + _sub._v1 * src->_sub._v1
		+ _sub._v2 * src->_sub._v2 + _sub._v3 * src->_sub._v3;
}

const CStarControlSub26 *CStarControlSub26::fn3(const CStarControlSub26 *src, double val) {
	CStarControlSub26::setup(_field0 * val, _sub._v1 * val, _sub._v2 * val, _sub._v3 * val);
	return src;
}

void CStarControlSub26::fn4(const DMatrix &m) {
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
			_sub._v1 = val1 * 0.5;
			_field0 = (m._row2._z - m._row3._y) * val2;
			_sub._v2 = (m._row2._x + m._row1._y) * val2;
			_sub._v3 = (m._row3._x + m._row1._z) * val2;
		} else if (total == m._row2._y) {
			double val1 = sqrt(m._row2._y - -1.0 - m._row3._z - m._row1._x);
			double val2 = 0.5 / val1;
			_sub._v2 = val1 * 0.5;
			_field0 = (m._row3._x - m._row1._z) * val2;
			_sub._v3 = (m._row3._y + m._row2._z) * val2;
			_sub._v1 = (m._row2._x + m._row1._y) * val2;
		} else if (total == m._row3._z) {
			double val1 = sqrt(m._row3._z - -1.0 - m._row1._x - m._row2._y);
			double val2 = 0.5 / val1;
			_sub._v3 = val1 * 0.5;
			_field0 = (m._row1._y - m._row2._x) * val2;
			_sub._v1 = (m._row3._x + m._row1._z) * val2;
			_sub._v2 = (m._row3._y + m._row2._z) * val2;
		}
	} else {
		double val1 = 0.5 / sqrt(total);
		_field0 = sqrt(total) * 0.5;
		_sub._v1 = (m._row2._z - m._row3._y) * val1;
		_sub._v2 = (m._row3._x - m._row1._z) * val1;
		_sub._v3 = (m._row1._y - m._row2._x) * val1;
	}
}

void CStarControlSub26::fn5(double val, CStarControlSub26 *s1, CStarControlSub26 *s2) {
	CStarControlSub26 sub1 = *this;
	CStarControlSub26 sub2, sub3, sub4;
	double val1 = sub1.fn2(s1);

	if (val1 < 0.0) {
		val1 = -val1;
		sub2.setup(-sub1._field0, -sub1._sub._v1, -sub1._sub._v2, -sub1._sub._v3);
		sub1 = sub2;
	}

	if (val1 - -1.0 <= 0.00001) {
		double sval1 = sub1._sub._v1;
		double sval3 = sub1._sub._v3;
		double sval2 = -sub1._sub._v2;
		double sval0 = -sub1._field0;

		sub2._sub._v1 = sval2;
		sub2._sub._v2 = sval1;
		sub2._sub._v3 = sval0;
		sub2._field0 = sval3;

		double sinVal = sin(val * M_PI);
		double val2 = sinVal * sval2;
		double val3 = sin((0.5 - val) * M_PI);
		double val4 = sinVal * sval0;
		double val5 = sub1._sub._v3 * val3;

		sub2._sub._v1 = val2 + sub1._sub._v1 * val3;
		sub2._sub._v2 = sub1._sub._v2 * val3 + sub1._sub._v1 * sinVal;
		sub2._sub._v3 = val4 + val5;
	} else {
		double val2;
		const CStarControlSub26 *sp;

		if (1.0 - val1 <= 0.00001) {
			val2 = 1.0 - val;
			sp = s1->fn3(&sub3, val);
		} else {
			double cosVal = acos(val1);
			double sinVal = sin(cosVal);
			val2 = sin((1.0 - val) * cosVal) / sinVal;
			sp = s1->fn3(&sub3, sin(cosVal * val) / sinVal);
		}

		const CStarControlSub26 *sp2 = sub1.fn3(&sub4, val2);
		sub2.setup(sp2->_field0 + sp->_field0, sp->_sub._v1  + sp2->_sub._v1,
			sp->_sub._v2 + sp2->_sub._v2, sp->_sub._v3 + sp2->_sub._v3);
		*s2 = sub2;
	}
}

} // End of namespace Titanic
