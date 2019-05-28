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

#include "titanic/star_control/star_points2.h"
#include "titanic/star_control/star_camera.h"
#include "titanic/star_control/surface_area.h"
#include "titanic/support/files_manager.h"
#include "titanic/titanic.h"

#include "common/math.h"

namespace Titanic {

#define ARRAY_COUNT 80

bool CStarPoints2::initialize() {
	// Get a reference to the starfield points resource
	Common::SeekableReadStream *stream = g_vm->_filesManager->getResource("STARFIELD/POINTS2");

	_data.resize(ARRAY_COUNT);
	for (int rootCtr = 0; rootCtr < ARRAY_COUNT; ++rootCtr) {
		// Get the number of sub-entries for this entry
		int count = stream->readUint32LE();
		double v1, v2;

		// Read in the sub-entries
		RootEntry &rootEntry = _data[rootCtr];
		rootEntry.resize(count);
		for (int idx = 0; idx < count; ++idx) {
			CStarPointEntry &se = rootEntry[idx];
			FVector *vectors[2] = { &se._v1, &se._v2 };

			for (int fctr = 0; fctr < 2; ++fctr) {
				v1 = stream->readSint32LE();
				v2 = stream->readSint32LE();
				v1 *= Common::deg2rad<double>(0.015);
				v2 *= Common::deg2rad<double>(0.01);

				vectors[fctr]->_x = cos(v1) * 3000000.0 * cos(v2);
				vectors[fctr]->_y = sin(v1) * 3000000.0 * cos(v2);
				vectors[fctr]->_z = sin(v2) * 3000000.0;
			}
		}
	}

	delete stream;
	return true;
}

void CStarPoints2::draw(CSurfaceArea *surface, CStarCamera *camera) {
	if (_data.empty())
		return;

	FPose pose = camera->getPose();
	double threshold = camera->getThreshold();
	FVector vector1, vector2, vector3, vector4;
	double vWidth2 = (double)surface->_width * 0.5;
	double vHeight2 = (double)surface->_height * 0.5;
	FRect r;

	surface->_pixel = 0xffff00;
	uint oldPixel = surface->_pixel;
	surface->setColorFromPixel();
	SurfaceAreaMode oldMode = surface->setMode(SA_SOLID);

	for (uint rootCtr = 0; rootCtr < _data.size(); ++rootCtr) {
		const RootEntry &re = _data[rootCtr];
		if (!re._field0 || re.empty())
			continue;

		for (uint idx = 0; idx < re.size(); ++idx) {
			const CStarPointEntry &se = re[idx];
			vector1._z = pose._row2._z * se._v1._y + pose._row3._z * se._v1._z
				+ pose._row1._z * se._v1._x + pose._vector._z;
			vector1._x = pose._row2._x * se._v1._y + pose._row3._x * se._v1._z
				+ pose._row1._x * se._v1._x + pose._vector._x;
			vector1._y = pose._row2._y * se._v1._y + pose._row3._y * se._v1._z
				+ pose._row1._y * se._v1._x + pose._vector._y;
			vector3._z = pose._row2._z * se._v2._y + pose._row2._x * se._v2._z
				+ pose._row1._z * se._v2._x + pose._vector._y;
			vector3._x = pose._row3._z * se._v2._y + pose._row3._x * se._v2._z
				+ pose._row1._x * se._v2._x + pose._vector._y;
			vector3._y = pose._row2._y * se._v2._y + pose._row3._y * se._v2._z
				+ pose._row1._y * se._v2._x + pose._vector._y;

			if (vector1._z > threshold && vector3._z > threshold) {
				vector2 = camera->getRelativePos(2, vector1);
				vector4 = camera->getRelativePos(2, vector3);

				r.bottom = vector4._y + vHeight2;
				r.right = vector4._x + vWidth2;
				r.top = vector2._y + vHeight2;
				r.left = vector2._x + vWidth2;
				surface->drawLine(r);
			}
		}
	}

	surface->_pixel = oldPixel;
	surface->setColorFromPixel();
	surface->setMode(oldMode);
}

} // End of namespace Titanic
