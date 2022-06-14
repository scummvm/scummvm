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

#include "hpl1/engine/graphics/GfxObject.h"

#include "hpl1/engine/graphics/Material.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGfxObject::cGfxObject(iMaterial *apMat, const tString &asFile, bool abIsImage) {
	mpMat = apMat;

	msSourceFile = asFile;

	mbIsImage = abIsImage;

	if (mbIsImage) {
		mvVtx = apMat->GetImage(eMaterialTexture_Diffuse)->GetVertexVecCopy(0, -1);
	} else {
		mvVtx.push_back(cVertex(cVector3f(0, 0, 0), cVector2f(0, 0), cColor(1, 1)));
		mvVtx.push_back(cVertex(cVector3f(1, 0, 0), cVector2f(1, 0), cColor(1, 1)));
		mvVtx.push_back(cVertex(cVector3f(1, 1, 0), cVector2f(1, 1), cColor(1, 1)));
		mvVtx.push_back(cVertex(cVector3f(0, 1, 0), cVector2f(0, 1), cColor(1, 1)));
	}
}

//-----------------------------------------------------------------------

cGfxObject::~cGfxObject() {
	hplDelete(mpMat);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

} // namespace hpl
