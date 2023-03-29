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
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_CAMERA_H
#define HPL_CAMERA_H

#include "hpl1/engine/math/MathTypes.h"

namespace hpl {

class iLowLevelGraphics;

enum eCameraType {
	eCameraType_2D,
	eCameraType_3D,
	eCameraType_LastEnum
};

class iCamera {
public:
	virtual ~iCamera() = default;
	virtual void SetModelViewMatrix(iLowLevelGraphics *apLowLevel) = 0;
	virtual void SetProjectionMatrix(iLowLevelGraphics *apLowLevel) = 0;
	virtual cVector3f GetEyePosition() = 0;

	virtual eCameraType GetType() = 0;
};

} // namespace hpl

#endif // HPL_CAMERA_H
