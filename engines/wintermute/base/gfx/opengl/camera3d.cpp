/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */


#include "camera3d.h"
#include "math/angle.h"
#include "math/quat.h"
#include "../../../math/math_util.h"
#include "math/glmath.h"
#include "loader3ds.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
Camera3D::Camera3D(BaseGame* inGame): BaseNamedObject(inGame) {
	_position = Math::Vector3d(0,0,0);
	_target = Math::Vector3d(0,0,0);
	_bank = 0.0f;
	_fov = _originalFOV = Math::Angle(45.0f).getRadians();
	_nearClipPlane = _farClipPlane = -1.0f;
}


//////////////////////////////////////////////////////////////////////////
Camera3D::~Camera3D() {

}

bool Camera3D::loadFrom3DS(byte **buffer) {
	uint32 whole_chunk_size = *reinterpret_cast<uint32 *>(*buffer);
	byte *end = *buffer + whole_chunk_size - 2;
	*buffer += 4; // chunk size

	_position.x() = *reinterpret_cast<float *>(*buffer);
	*buffer += 4;
	_position.z() = *reinterpret_cast<float *>(*buffer);
	*buffer += 4;
	_position.y() = *reinterpret_cast<float *>(*buffer);
	*buffer += 4;

	_target.x() = *reinterpret_cast<float *>(*buffer);
	*buffer += 4;
	_target.z() = *reinterpret_cast<float *>(*buffer);
	*buffer += 4;
	_target.y() = *reinterpret_cast<float *>(*buffer);
	*buffer += 4;

	_bank = *reinterpret_cast<float *>(*buffer);
	*buffer += 4;

	float lens = *reinterpret_cast<float *>(*buffer);
	*buffer += 4;

	if (lens > 0.0f) {
		_fov = Math::Angle(1900.0f / lens).getRadians();
	} else {
		_fov = Math::Angle(45.0f).getRadians();
	}

	// this is overkill here, simplify it later
	while (*buffer < end) {
		uint16 chunk_id = *reinterpret_cast<uint16 *>(*buffer);

		switch (chunk_id) {
		case 0x4720:
			*buffer += *reinterpret_cast<uint16 *>(*buffer + 2);
		default:
			break;
		}
	}

	return true;
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
void Camera3D::rotateView(float X, float Y, float Z) {
	Math::Vector3d vVector;						// Vector for the position/view.

	// Get our view vector (The direciton we are facing)
	vVector = _target - _position;		// This gets the direction of the view
	
	// Rotate the view along the desired axis
	if (X) {
		// Rotate the view vector up or down, then add it to our position
		_target.z() = (float)(_position.z() + sin(X)*vVector.y() + cos(X)*vVector.z());
		_target.y() = (float)(_position.y() + cos(X)*vVector.y() - sin(X)*vVector.z());
	}
	if (Y) {
		// Rotate the view vector right or left, then add it to our position
		_target.z() = (float)(_position.z() + sin(Y)*vVector.x() + cos(Y)*vVector.z());
		_target.x() = (float)(_position.x() + cos(Y)*vVector.x() - sin(Y)*vVector.z());
	}
	if (Z) {
		// Rotate the view vector diagnally right or diagnally down, then add it to our position
		_target.x() = (float)(_position.x() + sin(Z)*vVector.y() + cos(Z)*vVector.x());
		_target.y() = (float)(_position.y() + cos(Z)*vVector.y() - sin(Z)*vVector.x());
	}

}


//////////////////////////////////////////////////////////////////////////
void Camera3D::move(float speed) {
	Math::Vector3d vector;						// Init a vector for our view

	// Get our view vector (The direciton we are facing)
	vector = _target - _position;		// This gets the direction of the view
	
	_position.x() += vector.x() * speed;		// Add our acceleration to our position's X
	_position.z() += vector.z() * speed;		// Add our acceleration to our position's Z
	_target.x() += vector.x() * speed;			// Add our acceleration to our view's X
	_target.z() += vector.z() * speed;			// Add our acceleration to our view's Z
}

}
