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
#include "common/util.h"

namespace BladeRunner {

bool View::readVqa(Common::ReadStream *stream) {
	_frame = stream->readUint32LE();

	float d[12];
	for (int i = 0; i != 12; ++i) {
		d[i] = stream->readFloatLE();
	}

	_frameViewMatrix = Matrix4x3(d);

	float fovX = stream->readFloatLE();

	setFovX(fovX);
	calculateSliceViewMatrix();
	calculateCameraPosition();

	return true;
}

void View::setFovX(float fovX) {
	_fovX = fovX;

	_viewportPosition.x = 320.0f;
	_viewportPosition.y = 240.0f;
	_viewportPosition.z = 320.0f / tan(_fovX / 2.0f);
}

void View::calculateSliceViewMatrix() {
	Matrix4x3 mRotation = rotationMatrixX(float(M_PI) / 2.0f);
	Matrix4x3 mInvert(-1.0f,  0.0f, 0.0f, 0.0f,
	                   0.0f, -1.0f, 0.0f, 0.0f,
	                   0.0f,  0.0f, 1.0f, 0.0f);
	_sliceViewMatrix = mInvert * (_frameViewMatrix * mRotation);
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
		_viewportPosition.x - ((viewPosition.x / ABS(viewPosition.z)) * ABS(_viewportPosition.z)),
		_viewportPosition.y - ((viewPosition.y / ABS(viewPosition.z)) * ABS(_viewportPosition.z)),
		viewPosition.z
	);
}

} // End of namespace BladeRunner
