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

#include "titanic/star_control/constellations.h"
#include "titanic/star_control/camera.h"
#include "titanic/star_control/star_field.h"
#include "titanic/star_control/surface_area.h"
#include "titanic/support/files_manager.h"
#include "titanic/titanic.h"

#include "common/math.h"

namespace Titanic {

#define TOTAL_CONSTELLATIONS 80

bool CConstellations::initialize() {
	double ra, dec, phi, theta;

	// Get a reference to the starfield points resource
	Common::SeekableReadStream *stream = g_vm->_filesManager->getResource("STARFIELD/POINTS2");

	_data.resize(TOTAL_CONSTELLATIONS);
	for (int rootCtr = 0; rootCtr < TOTAL_CONSTELLATIONS; ++rootCtr) {
		// Get the number of points in the constellation
		int count = stream->readUint32LE();

		// Read in the points
		Constellation &rootEntry = _data[rootCtr];
		rootEntry.resize(count);
		for (int idx = 0; idx < count; ++idx) {
			ConstellationLine &cl = rootEntry[idx];
			FVector *vectors[2] = { &cl._start, &cl._end};

			for (int fctr = 0; fctr < 2; ++fctr) {
				ra = (double)stream->readSint32LE() * 360.0f / 24000.0f;
				dec = (double)stream->readSint32LE() / 100.0f;

				// Work the polar coordinates
				phi = Common::deg2rad<double>(ra);
				theta = Common::deg2rad<double>(dec);

				vectors[fctr]->_x = UNIVERSE_SCALE * cos(theta) * cos(phi);
				vectors[fctr]->_y = UNIVERSE_SCALE * cos(theta) * sin(phi);
				vectors[fctr]->_z = UNIVERSE_SCALE * sin(theta);
			}
		}
	}

	delete stream;
	return true;
}

void CConstellations::draw(CSurfaceArea *surface, CCamera *camera) {
	if (_data.empty())
		return;

	FPose pose = camera->getPose();
	double threshold = camera->getFrontClip();
	double centerX = (double)surface->_width / 2.0F;
	double centerY = (double)surface->_height / 2.0F;
	FVector ec0, ec1;
	FVector sc0, sc1;

	// Set the drawing mode, saving the old mode
	surface->_pixel = 0xffff00;
	uint oldPixel = surface->_pixel;
	surface->setColorFromPixel();
	SurfaceAreaMode oldMode = surface->setMode(SA_SOLID);

	// Iterate through the constellations
	for (uint conCtr = 0; conCtr < _data.size(); ++conCtr) {
		const Constellation &con = _data[conCtr];
		if (con.empty())
			continue;

		for (uint idx = 0; idx < con.size(); ++idx) {
			const FVector &ps = con[idx]._start;
			ec0._x = ps._x * pose._row1._x + ps._y * pose._row2._x + ps._z * pose._row3._x + pose._vector._x;
			ec0._y = ps._x * pose._row1._y + ps._y * pose._row2._y + ps._z * pose._row3._y + pose._vector._y;
			ec0._z = ps._x * pose._row1._z + ps._y * pose._row2._z + ps._z * pose._row3._z + pose._vector._z;

			const FVector &pe = con[idx]._end;
			ec1._x = pe._x * pose._row1._x + pe._y * pose._row2._x + pe._z * pose._row3._x + pose._vector._x;
			ec1._y = pe._x * pose._row1._y + pe._y * pose._row2._y + pe._z * pose._row3._y + pose._vector._y;
			ec1._z = pe._x * pose._row1._z + pe._y * pose._row2._z + pe._z * pose._row3._z + pose._vector._z;

			// Draw if the constellation line is visible
			if (ec0._z > threshold && ec1._z > threshold) {
				sc0 = camera->getRelativePos(2, ec0);
				sc1 = camera->getRelativePos(2, ec1);
				surface->drawLine(Point(sc0._x + centerX, sc0._y + centerY),
					Point(sc1._x + centerX, sc1._y + centerY));
			}
		}
	}

	// Restore the old state
	surface->_pixel = oldPixel;
	surface->setColorFromPixel();
	surface->setMode(oldMode);
}

} // End of namespace Titanic
