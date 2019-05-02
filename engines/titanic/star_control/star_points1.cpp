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
#include "titanic/star_control/star_camera.h"
#include "titanic/star_control/surface_area.h"
#include "titanic/support/files_manager.h"
#include "titanic/titanic.h"

#include "common/math.h"

namespace Titanic {

#define ARRAY_COUNT 876

CStarPoints1::CStarPoints1() {
}

bool CStarPoints1::initialize() {
	// Get a reference to the starfield points resource
	Common::SeekableReadStream *stream = g_vm->_filesManager->getResource("STARFIELD/POINTS");
	assert(stream && stream->size() == (12 * ARRAY_COUNT));

	_data.resize(ARRAY_COUNT);
	for (int idx = 0; idx < ARRAY_COUNT; ++idx) {
		CStarPointEntry &entry = _data[idx];

		// Get the next set of values
		double v1 = stream->readSint32LE();
		double v2 = stream->readSint32LE();
		entry._flag = stream->readUint32LE() != 0;

		v1 *= Common::deg2rad<double>(0.015);
		v2 *= Common::deg2rad<double>(0.0099999998);

		entry._x = cos(v2) * 3000000.0 * cos(v1);
		entry._y = sin(v1) * 3000000.0 * cos(v2);
		entry._z = sin(v2) * 3000000.0;
	}

	delete stream;
	return true;
}

void CStarPoints1::draw(CSurfaceArea *surface, CStarCamera *camera) {
	if (_data.empty())
		return;

	FPose pose = camera->getPose();
	double threshold = camera->getThreshold();
	FVector vector1, vector2, vector3, vector4;
	FVector vTemp = _data[0];
	double vWidth2 = (double)surface->_width * 0.5;
	double vHeight2 = (double)surface->_height * 0.5;
	FRect r;

	surface->_pixel = 0xff0000;
	uint oldPixel = surface->_pixel;
	surface->setColorFromPixel();
	SurfaceAreaMode oldMode = surface->setMode(SA_SOLID);

	vector1._z = vTemp._x * pose._row1._z + vTemp._y * pose._row2._z + vTemp._z * pose._row3._z + pose._vector._z;
	vector1._x = vTemp._x * pose._row1._x + vTemp._y * pose._row2._x + vTemp._z * pose._row3._x + pose._vector._x;
	vector1._y = vTemp._x * pose._row1._y + vTemp._y * pose._row2._y + vTemp._z * pose._row3._y + pose._vector._y; 

	for (uint idx = 1; idx < _data.size(); ++idx) {
		const FVector &sv = _data[idx];
		bool flag = _data[idx - 1]._flag;
		vTemp = sv;

		vector3._x = vTemp._x * pose._row1._x + vTemp._y * pose._row2._x + vTemp._z * pose._row3._x * pose._vector._x;
		vector3._y = vTemp._x * pose._row1._y + vTemp._y * pose._row2._y + vTemp._z * pose._row3._y * pose._vector._y;
		vector3._z = vTemp._x * pose._row1._z + vTemp._y * pose._row2._z + vTemp._z * pose._row3._z + pose._vector._z;

		if (flag && vector1._z > threshold && vector3._z > threshold) {
			vector2 = camera->getRelativePos(2, vector1);
			vector4 = camera->getRelativePos(2, vector3);

			r.bottom = vector4._y + vHeight2;
			r.right = vector4._x + vWidth2;
			r.top = vector2._y + vHeight2;
			r.left = vector2._x + vWidth2;
			surface->drawLine(r);
		}

		vector1 = vector3;
	}

	surface->_pixel = oldPixel;
	surface->setColorFromPixel();
	surface->setMode(oldMode);
}

} // End of namespace Titanic
