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

#ifndef WINTERMUTE_CAMERA3D_H
#define WINTERMUTE_CAMERA3D_H

#include "common/memstream.h"

#include "engines/wintermute/base/base_named_object.h"

#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Wintermute {

class Camera3D : public BaseNamedObject {
public:
	void move(float speed);
	void rotateView(float x, float y, float z);
	void setupPos(Math::Vector3d pos, Math::Vector3d target, float Bank = 0);
	bool getViewMatrix(Math::Matrix4 *viewMatrix);
	Camera3D(BaseGame *inGame);
	virtual ~Camera3D();

	Math::Vector3d _position;
	Math::Vector3d _target;
	float _bank;
	float _fov;
	float _originalFOV;
	float _nearClipPlane;
	float _farClipPlane;

	bool loadFrom3DS(Common::MemoryReadStream &fileStream);
};

} // namespace Wintermute

#endif
