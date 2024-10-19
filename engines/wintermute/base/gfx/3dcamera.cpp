/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/base/gfx/3dcamera.h"
#include "engines/wintermute/base/gfx/3dloader_3ds.h"
#include "engines/wintermute/math/math_util.h"

#include "math/angle.h"
#include "math/glmath.h"
#include "math/quat.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
Camera3D::Camera3D(BaseGame *inGame) : BaseNamedObject(inGame) {
	_position = DXVector3(0.0f, 0.0f, 0.0f);
	_target = DXVector3(0.0f, 0.0f, 0.0f);
	_bank = 0.0f;
	_fov = _origFov = degToRad(45.0f);
	_nearClipPlane = _farClipPlane = -1.0f;
}

//////////////////////////////////////////////////////////////////////////
Camera3D::~Camera3D() {
}

//////////////////////////////////////////////////////////////////////////
bool Camera3D::getViewMatrix(DXMatrix *viewMatrix) {
	DXVector3 up = DXVector3(0.0f, 1.0f, 0.0f);

	if (_bank != 0) {
		DXMatrix rot;
		DXMatrixRotationZ(&rot, degToRad(_bank));
		DXVec3TransformCoord(&up, &up, &rot);
	}

	DXMatrixLookAtRH(viewMatrix, &_position, &_target, &up);

	return true;
}

//////////////////////////////////////////////////////////////////////////
void Camera3D::setupPos(DXVector3 pos, DXVector3 target, float bank) {
	_position = pos;
	_target = target;
	_bank = bank;
}

//////////////////////////////////////////////////////////////////////////
void Camera3D::rotateView(float x, float y, float z) {
	DXVector3 vVector; // Vector for the position/view.

	// Get our view vector (The direciton we are facing)
	vVector = _target - _position; // This gets the direction of the view

	// Rotate the view along the desired axis
	if (x) {
		// Rotate the view vector up or down, then add it to our position
		_target._z = (float)(_position._z + sin(x) * vVector._y + cos(x) * vVector._z);
		_target._y = (float)(_position._y + cos(x) * vVector._y - sin(x) * vVector._z);
	}
	if (y) {
		// Rotate the view vector right or left, then add it to our position
		_target._z = (float)(_position._z + sin(y) * vVector._x + cos(y) * vVector._z);
		_target._x = (float)(_position._x + cos(y) * vVector._x - sin(y) * vVector._z);
	}
	if (z) {
		// Rotate the view vector diagnally right or diagnally down, then add it to our position
		_target._x = (float)(_position._x + sin(z) * vVector._y + cos(z) * vVector._x);
		_target._y = (float)(_position._y + cos(z) * vVector._y - sin(z) * vVector._x);
	}
}

//////////////////////////////////////////////////////////////////////////
void Camera3D::move(float speed) {
	DXVector3 vector; // Init a vector for our view

	// Get our view vector (The direciton we are facing)
	vector = _target - _position; // This gets the direction of the view

	_position._x += vector._x * speed; // Add our acceleration to our position's X
	_position._z += vector._z * speed; // Add our acceleration to our position's Z
	_target._x += vector._x * speed;   // Add our acceleration to our view's X
	_target._z += vector._z * speed;   // Add our acceleration to our view's Z
}

bool Camera3D::loadFrom3DS(Common::MemoryReadStream &fileStream) {
	uint32 wholeChunkSize = fileStream.readUint32LE();
	int32 end = fileStream.pos() + wholeChunkSize - 6;

	_position._x = fileStream.readFloatLE();
	_position._z = -fileStream.readFloatLE();
	_position._y = fileStream.readFloatLE();

	_target._x = fileStream.readFloatLE();
	_target._z = -fileStream.readFloatLE();
	_target._y = fileStream.readFloatLE();

	_bank = fileStream.readFloatLE();

	float lens = fileStream.readFloatLE();

	if (lens > 0.0f) {
		_fov = degToRad(1900.0f / lens);
	} else {
		_fov = degToRad(45.0f);
	}

	_origFov = _fov;

	// discard all subchunks
	while (fileStream.pos() < end) {
		fileStream.readUint16LE(); // chunk id
		uint32 chunkSize = fileStream.readUint32LE();

		fileStream.seek(chunkSize - 6, SEEK_CUR);
	}

	return true;
}

} // namespace Wintermute
