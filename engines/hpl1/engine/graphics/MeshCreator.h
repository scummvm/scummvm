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

#ifndef HPL_MESHCREATOR_H
#define HPL_MESHCREATOR_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cMesh2D;
class iVertexBuffer;
class iLowLevelGraphics;
class cResources;
class cMesh;

class cMeshCreator {
public:
	cMeshCreator(iLowLevelGraphics *apLowLevelGraphics, cResources *apResources);
	~cMeshCreator();

	cMesh2D *Create2D(tString asName, cVector2f mvSize);

	cMesh *CreateBox(const tString &asName, cVector3f avSize, const tString &asMaterial);

	iVertexBuffer *CreateSkyBoxVertexBuffer(float afSize);
	iVertexBuffer *CreateBoxVertexBuffer(cVector3f avSize);

private:
	cVector3f GetBoxTex(int i, int x, int y, int z, cVector3f *vAdd);
	int GetBoxIdx(int i, int x, int y, int z);

	iLowLevelGraphics *mpLowLevelGraphics;
	cResources *mpResources;
};

} // namespace hpl

#endif // HPL_MESHCREATOR_H
