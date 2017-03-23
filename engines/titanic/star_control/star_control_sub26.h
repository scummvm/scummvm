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

#ifndef TITANIC_STAR_CONTROL_SUB26_H
#define TITANIC_STAR_CONTROL_SUB26_H

#include "titanic/star_control/dmatrix.h"

namespace Titanic {

class CStarControlSub26 {
	struct Sub {
		double _v1;
		double _v2;
		double _v3;

		Sub() : _v1(0.0), _v2(0.0), _v3(0.0) {}
	};
private:
	double fn2(const CStarControlSub26 *src);
	const CStarControlSub26 *fn3(const CStarControlSub26 *src, double val);
public:
	double _field0;
	Sub _sub;
public:
	CStarControlSub26() : _field0(1.0) {}

	/**
	 * Sets the field values
	 */
	void setup(double val1, double val2, double val3, double val4);

	/**
	 * Copies from another instance
	 */
	void copyFrom(const CStarControlSub26 *src);

	double fn1() const;
	void fn4(const DMatrix &m);
	void fn5(double val, CStarControlSub26 *s1, CStarControlSub26 *s2);
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB26_H */
