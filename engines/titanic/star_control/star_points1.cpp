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

#include "titanic/star_control/star_points1.h"
#include "titanic/star_control/star_control_sub12.h"
#include "titanic/titanic.h"

namespace Titanic {

#define ARRAY_COUNT 876
const double FACTOR = 3.1415927 * 0.0055555557;

CStarPoints1::CStarPoints1() {
}

bool CStarPoints1::initialize() {
	// Get a reference to the starfield points resource
	Common::SeekableReadStream *stream = g_vm->_filesManager->getResource("STARFIELD/POINTS");
	assert(stream && stream->size() == (12 * ARRAY_COUNT));

	_data.resize(ARRAY_COUNT);
	for (int idx = 0; idx < ARRAY_COUNT; ++idx) {
		FVector &entry = _data[idx];

		// Get the next set of values
		double v1 = stream->readSint32LE();
		double v2 = stream->readSint32LE();
		stream->readUint32LE();

		v1 *= 0.015 * FACTOR;
		v2 *= 0.0099999998 * FACTOR;

		entry._x = cos(v2) * 3000000.0 * cos(v1);
		entry._y = sin(v1) * 3000000.0 * cos(v2);
		entry._z = sin(v2) * 3000000.0;
	}

	return true;
}

void CStarPoints1::draw(CSurfaceArea *surface, CStarControlSub12 *sub12) {
	if (_data.empty())
		return;

	/*CStarControlSub6 sub6 = */ sub12->proc23();
	sub12->proc25();
	/*
	FVector &v0 = _data[0];
	double vx = v0._x, vy = v0._y, vz = v0._z;

	| (vx*sub6._matrix.row1._z + vy*sub6._matrix.row2._z + vy) |
	| vz*sub6._matrix.row3._x |
	| surface->_width |
	| vy |
	| vx*sub6._matrix.row1._x |
	| vz |
	| vy*sub6._matrix.row2._x*sub6._matrix.row1._y*sub6._matrix.row3._z |
	| vz*sub6._matrix.row2._y |
	| vy*sub6._matrix.row2._z + vx*sub6._matrix.row1._z + vy*sub6._matrix.row2._z |
	| vx |
	*/

	// TODO
}

} // End of namespace Titanic
