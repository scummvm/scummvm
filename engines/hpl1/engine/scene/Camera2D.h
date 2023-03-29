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

#ifndef HPL_CAMERA2D_H
#define HPL_CAMERA2D_H

#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/scene/Camera.h"

namespace hpl {

class iLowLevelGraphics;

class cCamera2D : public iCamera {
public:
	cCamera2D(unsigned int alW, unsigned int alH);
	~cCamera2D();

	const cVector3f &GetPosition() const { return mvPosition; }
	cVector3f GetEyePosition();
	void SetPosition(cVector3f avPos) { mvPosition = avPos; }

	void SetXY(cVector2f avPos) {
		mvPosition.x = avPos.x;
		mvPosition.y = avPos.y;
	}
	void SetZ(float afZ) { mvPosition.z = afZ; }
	void MoveXY(cVector2f avAdd) {
		mvPosition.x += avAdd.x;
		mvPosition.y += avAdd.y;
	}
	void MoveZ(float afZ) { mvPosition.z += afZ; }

	void GetClipRect(cRect2f &aRect);

	void SetModelViewMatrix(iLowLevelGraphics *apLowLevel);
	void SetProjectionMatrix(iLowLevelGraphics *apLowLevel);

	eCameraType GetType() { return eCameraType_2D; }

private:
	cVector3f mvPosition;
	float mfAngle;
	float mfZMin;
	float mfZMax;
	float mfFOV;

	cVector2l mvClipArea;
};

} // namespace hpl

#endif // HPL_CAMERA2D_H
