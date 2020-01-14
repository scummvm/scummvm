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

#include "titanic/star_control/const_boundaries.h"
#include "titanic/star_control/camera.h"
#include "titanic/star_control/star_field.h"
#include "titanic/star_control/surface_area.h"
#include "titanic/support/files_manager.h"
#include "titanic/titanic.h"

#include "common/math.h"

namespace Titanic {

#define ARRAY_COUNT 876

CConstBoundaries::CConstBoundaries() {
}

bool CConstBoundaries::initialize() {
	double y, z, ra, dec, phi, theta;

	// Get a reference to the starfield points resource
	Common::SeekableReadStream *stream = g_vm->_filesManager->getResource("STARFIELD/POINTS");
	assert(stream && stream->size() == (12 * ARRAY_COUNT));

	_data.resize(ARRAY_COUNT);
	for (int idx = 0; idx < ARRAY_COUNT; ++idx) {
		CBoundaryVector &entry = _data[idx];

		// Get the next set of values
		entry._isDrawn = (idx == 0) ? 0 : stream->readUint32LE() != 0;
		y = stream->readSint32LE();
		z = stream->readSint32LE();

		ra = y * 360.0F / 24000.0F;
		dec = z / 100.0F;

		// Work the polar coordinates
		phi = Common::deg2rad<double>(ra);
		theta = Common::deg2rad<double>(dec);

		entry._x = UNIVERSE_SCALE * cos(theta) * cos(phi);
		entry._y = UNIVERSE_SCALE * cos(theta) * sin(phi);
		entry._z = UNIVERSE_SCALE * sin(theta);
	}

	delete stream;
	return true;
}

void CConstBoundaries::draw(CSurfaceArea *surface, CCamera *camera) {
	if (_data.empty())
		return;

	// get the current camera transform.
	FPose pose = camera->getPose();
	float threshold = camera->getFrontClip();
	float centerX = (float)surface->_width / 2.0f;
	float centerY = (float)surface->_height / 2.0f;

	FVector ec0, ec1, wc;
	FVector sc0, sc1;

	// Get the starting point
	wc = _data[0];
	ec0._x = wc._x * pose._row1._x + wc._y * pose._row2._x + wc._z * pose._row3._x + pose._vector._x;
	ec0._y = wc._x * pose._row1._y + wc._y * pose._row2._y + wc._z * pose._row3._y + pose._vector._y;
	ec0._z = wc._x * pose._row1._z + wc._y * pose._row2._z + wc._z * pose._row3._z + pose._vector._z;

	// Set the drawing mode and color
	surface->_pixel = 0xff0000;
	uint oldPixel = surface->_pixel;
	surface->setColorFromPixel();
	SurfaceAreaMode oldMode = surface->setMode(SA_SOLID);

	// Iterate through each remaining point
	for (uint idx = 1; idx < _data.size(); ++idx) {
		// Process the next point
		wc = _data[idx];
		ec1._x = wc._x * pose._row1._x + wc._y * pose._row2._x + wc._z * pose._row3._x + pose._vector._x;
		ec1._y = wc._x * pose._row1._y + wc._y * pose._row2._y + wc._z * pose._row3._y + pose._vector._y;
		ec1._z = wc._x * pose._row1._z + wc._y * pose._row2._z + wc._z * pose._row3._z + pose._vector._z;

		// Is this connected to the previous point?
		if (_data[idx]._isDrawn) {
			if (ec0._z > threshold && ec1._z > threshold) {
				// Render the line
				sc0 = camera->getRelativePos(2, ec0);
				sc1 = camera->getRelativePos(2, ec1);
				surface->drawLine(FPoint(sc0._x + centerX, sc0._y + centerY),
					FPoint(sc1._x + centerX, sc1._y + centerY));
			}
		}

		ec0 = ec1;
	}

	// Reset back to previous
	surface->_pixel = oldPixel;
	surface->setColorFromPixel();
	surface->setMode(oldMode);
}

} // End of namespace Titanic
