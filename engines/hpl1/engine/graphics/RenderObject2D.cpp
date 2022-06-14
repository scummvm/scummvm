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

#include "hpl1/engine/graphics/RenderObject2D.h"

#include "hpl1/engine/graphics/Material.h"


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cRenderObject2D::cRenderObject2D(iMaterial* apMaterial, tVertexVec* apVtxVec,tUIntVec* apIdxVec,
							ePrimitiveType aType,float afZ,cRect2f& aRect,
							cMatrixf *apMtx, cVector3f *avTransform)
	{
		mpMaterial = apMaterial;
		mpVtxVec = apVtxVec;
		mpIdxVec = apIdxVec;
		mType = aType;
		mpMtx = apMtx;
		mpTransform = avTransform;
		mfZ = afZ;
		mRect = aRect;
		mpCustomRenderer = NULL;
	}

	cRenderObject2D::cRenderObject2D(iMaterial* apMaterial, iRenderObject2DRenderer* apRenderer,
		ePrimitiveType aType,float afZ)
	{
		mpMaterial = apMaterial;
		mpVtxVec = NULL;
		mpIdxVec = NULL;
		mType = aType;
		mpMtx = NULL;
		mpTransform = NULL;
		mfZ = afZ;
		mpCustomRenderer = apRenderer;
	}


	//-----------------------------------------------------------------------

	cRenderObject2D::~cRenderObject2D()
	{
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	//-----------------------------------------------------------------------
}
