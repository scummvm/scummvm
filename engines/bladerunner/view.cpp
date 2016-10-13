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

#include "bladerunner/view.h"

#include "common/debug.h"
#include "common/stream.h"

namespace BladeRunner {

bool View::read(Common::ReadStream *stream) {
	_frame = stream->readUint32LE();

	float d[12];
	for (int i = 0; i != 12; ++i)
		d[i] = stream->readFloatLE();

	_frameViewMatrix = Matrix4x3(d);

	float fovX = stream->readFloatLE();

	setFovX(fovX);
	calculateSliceViewMatrix();
	calculateCameraPosition();

	return true;
}

void View::setFovX(float fovX) {
	_fovX = fovX;

	_viewportHalfWidth = 320.0f;
	_viewportHalfHeight = 240.0f;

	_viewportDistance = 320.0f / tanf(_fovX / 2.0f);
}

void View::calculateSliceViewMatrix() {
	Matrix4x3 m = _frameViewMatrix;

	m = m * rotationMatrixX(float(M_PI) / 2.0f);

	Matrix4x3 a(-1.0f,  0.0f, 0.0f, 0.0f,
	             0.0f, -1.0f, 0.0f, 0.0f,
	             0.0f,  0.0f, 1.0f, 0.0f);

	m = a * m;

	_sliceViewMatrix = m;
}

void View::calculateCameraPosition() {
	Matrix4x3 invertedMatrix = invertMatrix(_sliceViewMatrix);

	_cameraPosition.x = invertedMatrix(0, 3);
	_cameraPosition.y = invertedMatrix(1, 3);
	_cameraPosition.z = invertedMatrix(2, 3);
}

Vector3 View::calculateScreenPosition(Vector3 worldPosition) {
	Vector3 viewPosition = _frameViewMatrix * worldPosition;
	return Vector3(
		this->_viewportHalfWidth - viewPosition.x / viewPosition.z * _viewportDistance,
		this->_viewportHalfHeight - viewPosition.y / viewPosition.z * _viewportDistance,
		viewPosition.z
	);
}

} // End of namespace BladeRunner
