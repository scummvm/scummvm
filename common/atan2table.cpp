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

#include "common/scummsys.h"
#include "common/atan2table.h"
#include "common/math.h"
#include "common/util.h"

namespace Common {

Atan2Table::Atan2Table(int nPoints) {
	assert((nPoints >= 4) && (nPoints <= 1e8));
	// Create the table values as floats, but compute them using doubles.
	_nPoints = nPoints;
	_table = new float[_nPoints];
	_MPI_2 = M_PI / 2.0f;	
	double slope;
	int n;
	for (n=0; n < _nPoints; n++) {
		slope = -1.0 + 2.0/(_nPoints-1)*n;
		_table[n] = atan(slope);
	}
}

Atan2Table::~Atan2Table() {
	delete[] _table;
}

float Atan2Table::angle(float y,float x) {
	float slope;
	float angle;
	bool xpos  = x >= 0;
	int ypos   = y >= 0;
	bool ygx   = y * y > x * x; // Is magnitude of y greater than magnitude of x?

	slope = ygx ? x/y: y/x; // choose slope so that its magnitude is less than 1.

	// Map [-1,1] to [0, _nPoints - 1]
	// Table has zero index as -1 slope atan value and last value as +1 slope
	// If round is used to determine tableIndex then the error will be lower on average
	// for a given _nPoints, but it will also run slower.
	int tableIndex = (slope + 1.0f) * (_nPoints - 1) / 2;

	tableIndex = CLIP<int>(tableIndex, 0, _nPoints - 1);

	angle = _table[tableIndex];

	// Now we correct the angle calculated based on which part of the unit circle it is in.
	// This is done by going through each quarter quadrant doing the positive and negative angles 
	// that are the same distance from the +x axis at the same time.

	if (xpos && !ygx)       // In [-pi/4,+pi/4], the most common case
		return angle;
	else if (ygx)           // In [-pi/2,-pi/4), (pi/4,pi/2], [-3/4pi,-pi/2) or (pi/2, 3/4pi]
		return ypos ? _MPI_2 - angle: -_MPI_2 - angle;
	else if (!xpos && !ygx) // In [-pi,-3/4pi) or (3/4pi,pi]
		return ypos ? M_PI + angle: -M_PI + angle;
	else
		return 0.0f; // I don't think this can be reached
}

// same as angle() but return angle is in degrees
float Atan2Table::angleD(float y,float x) {
	return rad2deg<float>(angle(y, x));
}

// compare atan2 radian result
float Atan2Table::error(float y, float x) {
	return angle(y, x) - atan2(y, x);
}

} // End of namespace Common
