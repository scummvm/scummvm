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

#include "hpl1/engine/scene/Camera2D.h"

#include "hpl1/engine/graphics/LowLevelGraphics.h"


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cCamera2D::cCamera2D(unsigned int alW,unsigned int alH)
	{
		mvPosition = cVector3f(0,0,0);
		mfAngle =0;
		mvClipArea.x = alW;
		mvClipArea.y = alH;

		mfZMin = -100;
		mfZMax = -100;
		mfFOV = 60;
	}

	//-----------------------------------------------------------------------

	cCamera2D::~cCamera2D()
	{
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cVector3f cCamera2D::GetEyePosition()
	{
		cVector3f vEyePos = GetPosition();
		vEyePos.z = 100;
		return vEyePos;
	}

	//-----------------------------------------------------------------------

	void cCamera2D::GetClipRect(cRect2f &aRect)
	{
		float fDiv = 1;
		if(mvPosition.z<0){
			fDiv = 1 - std::abs(mvPosition.z/mfZMin);
			if(fDiv<=0)fDiv=0.0001f;
		}
		else
			fDiv = 1 + std::abs(mvPosition.z/mfZMax);

		//Transform these depending on z
		float lW = ((float) mvClipArea.x)*fDiv;
		float lH = ((float) mvClipArea.y)*fDiv;

		float lX = mvPosition.x - lW/2;
		float lY = mvPosition.y - lH/2;

		aRect.x = lX;
		aRect.y = lY;
		aRect.w = lW;
		aRect.h = lH;
	}

	//-----------------------------------------------------------------------

	void cCamera2D::SetModelViewMatrix(iLowLevelGraphics* apLowLevel)
	{
		apLowLevel->SetIdentityMatrix(eMatrix_ModelView);

		cRect2f ClipRect;
		GetClipRect(ClipRect);
		cVector2f vVirtSize = apLowLevel->GetVirtualSize();
		cVector3f vPos(-(ClipRect.x),-(ClipRect.y),0);
		cVector3f vScale(vVirtSize.x/ClipRect.w, vVirtSize.y/ClipRect.h, 1 );

		apLowLevel->ScaleMatrix(eMatrix_ModelView,vScale);
		apLowLevel->TranslateMatrix(eMatrix_ModelView,vPos);
	}

	void cCamera2D::SetProjectionMatrix(iLowLevelGraphics* apLowLevel)
	{
		apLowLevel->SetOrthoProjection(apLowLevel->GetVirtualSize(),-1000,1000);
	}

	//-----------------------------------------------------------------------

}
