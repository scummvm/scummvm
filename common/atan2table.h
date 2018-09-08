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

#ifndef COMMON_ATAN2TABLE_H
#define COMMON_ATAN2TABLE_H

namespace Common {

class Atan2Table {
public:

	// Table below contains:
	// nPoints | Max degree error | % faster than atan2()
	// -----------------------------------------
	// 1e7 | 2.8e-5 | 25%
	// 1e6 | 1.3e-4 | 40%
	// 1e5 | 1.2e-3 | 65%
	// 1e4 | 1.2e-2 | 65%
	// 1e3 | 1.2e-1 | 65%
	// 1e2 | 1.15e0 | 65%

	/**
	 * Construct a float based atan2 table
	 *
	 * @param nPoints the number of slope points that the atan will be evaluated at (for domain [-1,1]).
	 * nPoints can be almost any positive integer.
	 */
	Atan2Table(int nPoints = 1e5); // In absense of a choice 1e5 is a good trade off between speed and accuracy
	~Atan2Table();

	/**
	 * Evaluate atan2 using this lookup table.
	 * This function returns the radian angle from the +x axis, 
	 * positive for counter-clockwise and negative for clockwise.
	 * The range, [-pi,+pi], and the return values are the same as atan2.
	 */
	float angle(float y,float x);

	/**
	 * Same as angle() but returned angle is in degrees
	 */
	float angleD(float y,float x);

	/**
	 * Compare atan2 and atan2 lookup.
	 * Comparison is in radians.
	 */
	float error(float y, float x);

private:
	int _nPoints;
	float *_table;
	float _MPI_2;
};

} // End of namespace Common

#endif // COMMON_ATAN2TABLE_H