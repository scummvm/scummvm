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
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hpl1/engine/scene/Light2DPoint.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLight2DPoint::cLight2DPoint(tString asName) : iLight2D(asName) {
	UpdateBoundingBox();
}

//-----------------------------------------------------------------------

cLight2DPoint::~cLight2DPoint() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

// TODO: Z might have to be some standard value...
// Has 100 right now, this should be in some global value..
int cLight2DPoint::Render(iLowLevelGraphics *apLowLevel, int alFirstIndex) {
	// make the center vetrex:
	cVector3f vTex(0);
	cVector3f vPos;
	cVector3f vLightPos = GetWorldPosition();
	vLightPos.z = 100;
	float fRadius = GetFarAttenuation();

	cColor Col = GetDiffuseColor() * mfIntensity;
	Col.a = 0;
	cVertex Vtx = cVertex(vLightPos, vTex, Col);
	apLowLevel->AddVertexToBatch(Vtx); // index 0!

	Col = cColor(0, 0);
	int idx = alFirstIndex + 1;
	for (float fAngle = 0; fAngle <= k2Pif; fAngle += k2Pif / 32) {
		vPos.x = vLightPos.x + fRadius * cos(fAngle); //*0.5;
		vPos.y = vLightPos.y + fRadius * sin(fAngle); //*0.5;
		vPos.z = 100;
		Vtx = cVertex(vPos, vTex, Col);
		apLowLevel->AddVertexToBatch(Vtx);
		if (idx > 0) {
			apLowLevel->AddIndexToBatch(alFirstIndex); // The center
			apLowLevel->AddIndexToBatch(idx);          // The current point
			apLowLevel->AddIndexToBatch(idx - 1);      // The previous  point
		}
		idx++;
	}
	apLowLevel->AddIndexToBatch(alFirstIndex);     // The center
	apLowLevel->AddIndexToBatch(alFirstIndex + 1); // The current point
	apLowLevel->AddIndexToBatch(idx - 1);          // The previous  point

	return idx;
}

//-----------------------------------------------------------------------

const cRect2f &cLight2DPoint::GetBoundingBox() {
	return mBoundingBox;
}

//-----------------------------------------------------------------------

bool cLight2DPoint::UpdateBoundingBox() {
	mBoundingBox = cRect2f(cVector2f(GetWorldPosition().x, GetWorldPosition().y) - mfFarAttenuation,
						   cVector2f(mfFarAttenuation * 2));

	return true;
}

//-----------------------------------------------------------------------
} // namespace hpl
