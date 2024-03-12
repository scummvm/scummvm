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

#ifndef HPL_BACKGROUND_IMAGE_H
#define HPL_BACKGROUND_IMAGE_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/math/MathTypes.h"
#include "common/stablemap.h"

namespace hpl {

class cBackgroundImage {
public:
	cBackgroundImage(iMaterial *apMat, const cVector3f &avPos, bool abTile,
					 const cVector2f &avSize, const cVector2f &avPosPercent, const cVector2f &avVel);
	~cBackgroundImage();

	void Draw(const cRect2f &aCollideRect, iLowLevelGraphics *apLowLevelGraphics);

	void Update();

private:
	iMaterial *mpMaterial;

	tVertexVec mvVtx;

	cVector3f mvPos;
	bool mbTile;
	cVector2f mvSize;
	cVector2f mvPosPercent;
	cVector2f mvVel;
};

typedef Common::StableMap<float, cBackgroundImage *> tBackgroundImageMap;
typedef tBackgroundImageMap::iterator tBackgroundImageMapIt;

} // namespace hpl

#endif // HPL_BACKGROUND_IMAGE_H
