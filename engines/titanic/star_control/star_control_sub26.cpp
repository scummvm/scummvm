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

double CStarControlSub26::fn1() const {
	return _sub._v1 * _sub._v1 + _sub._v2 * _sub._v2 +
		_sub._v3 * _sub._v3 + _field0 * _field0;
}

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

} // End of namespace Titanic
