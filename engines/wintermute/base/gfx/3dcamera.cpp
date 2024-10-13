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
	_position = Math::Vector3d(0.0f, 0.0f, 0.0f);
	_target = Math::Vector3d(0.0f, 0.0f, 0.0f);
	_bank = 0.0f;
	_fov = _origFov = degToRad(45.0f);
	_nearClipPlane = _farClipPlane = -1.0f;
}

//////////////////////////////////////////////////////////////////////////
Camera3D::~Camera3D() {
}

//////////////////////////////////////////////////////////////////////////
bool Camera3D::getViewMatrix(Math::Matrix4 *viewMatrix) {
	Math::Vector3d up = Math::Vector3d(0.0f, 1.0f, 0.0f);

	if (_bank != 0) {
		Math::Matrix4 rotZ;
		rotZ.buildAroundZ(Math::Angle(_bank).getRadians());
		rotZ.transform(&up, false);
	}

	*viewMatrix = Math::makeLookAtMatrix(_position, _target, up);

	return true;
}

//////////////////////////////////////////////////////////////////////////
void Camera3D::setupPos(Math::Vector3d pos, Math::Vector3d target, float bank) {
	_position = pos;
	_target = target;
	_bank = bank;
}

//////////////////////////////////////////////////////////////////////////
void Camera3D::rotateView(float x, float y, float z) {
	Math::Vector3d vVector; // Vector for the position/view.

	// Get our view vector (The direciton we are facing)
	vVector = _target - _position; // This gets the direction of the view

	// Rotate the view along the desired axis
	if (x) {
		// Rotate the view vector up or down, then add it to our position
		_target.z() = (float)(_position.z() + sin(x) * vVector.y() + cos(x) * vVector.z());
		_target.y() = (float)(_position.y() + cos(x) * vVector.y() - sin(x) * vVector.z());
	}
	if (y) {
		// Rotate the view vector right or left, then add it to our position
		_target.z() = (float)(_position.z() + sin(y) * vVector.x() + cos(y) * vVector.z());
		_target.x() = (float)(_position.x() + cos(y) * vVector.x() - sin(y) * vVector.z());
	}
	if (z) {
		// Rotate the view vector diagnally right or diagnally down, then add it to our position
		_target.x() = (float)(_position.x() + sin(z) * vVector.y() + cos(z) * vVector.x());
		_target.y() = (float)(_position.y() + cos(z) * vVector.y() - sin(z) * vVector.x());
	}
}

//////////////////////////////////////////////////////////////////////////
void Camera3D::move(float speed) {
	Math::Vector3d vector; // Init a vector for our view

	// Get our view vector (The direciton we are facing)
	vector = _target - _position; // This gets the direction of the view

	_position.x() += vector.x() * speed; // Add our acceleration to our position's X
	_position.z() += vector.z() * speed; // Add our acceleration to our position's Z
	_target.x() += vector.x() * speed;   // Add our acceleration to our view's X
	_target.z() += vector.z() * speed;   // Add our acceleration to our view's Z
}

bool Camera3D::loadFrom3DS(Common::MemoryReadStream &fileStream) {
	uint32 wholeChunkSize = fileStream.readUint32LE();
	int32 end = fileStream.pos() + wholeChunkSize - 6;

	_position.x() = fileStream.readFloatLE();
	_position.z() = -fileStream.readFloatLE();
	_position.y() = fileStream.readFloatLE();

	_target.x() = fileStream.readFloatLE();
	_target.z() = -fileStream.readFloatLE();
	_target.y() = fileStream.readFloatLE();

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
