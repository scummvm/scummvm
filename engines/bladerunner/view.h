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

#ifndef BLADERUNNER_VIEW_H
#define BLADERUNNER_VIEW_H

#include "matrix.h"

namespace Common {
class ReadStream;
}

namespace BladeRunner {

class View {
public:
	float     _fovX;
	Matrix4x3 _frameViewMatrix;
	Matrix4x3 _sliceViewMatrix;
	uint32    _frame;

	Vector3   _cameraPosition;
	Vector3   _viewportPosition;

	bool readVqa(Common::ReadStream *stream);
	Vector3 calculateScreenPosition(Vector3 worldPosition);

private:
	void setFovX(float fovX);
	void calculateSliceViewMatrix();
	void calculateCameraPosition();
};

} // End of namespace BladeRunner

#endif
