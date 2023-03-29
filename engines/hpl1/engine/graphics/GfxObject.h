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

#ifndef HPL_GFX_OBJECT_H
#define HPL_GFX_OBJECT_H

#include "common/array.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"

namespace hpl {

class iMaterial;

class cGfxObject {
public:
	cGfxObject(iMaterial *apMat, const tString &asFile, bool abIsImage);
	~cGfxObject();

	iMaterial *GetMaterial() const { return mpMat; }
	cVertex *GetVtxPtr(int alNum) { return &mvVtx[alNum]; }
	tVertexVec *GetVertexVec() { return &mvVtx; }

	const tString &GetSourceFile() { return msSourceFile; }

	bool IsImage() { return mbIsImage; }

private:
	tVertexVec mvVtx;
	iMaterial *mpMat;
	// float mfZ;
	bool mbIsImage;

	tString msSourceFile;
};

typedef Common::Array<cGfxObject> tGfxObjectVec;
typedef tGfxObjectVec::iterator tGfxObjectVecIt;

} // namespace hpl

#endif // HPL_GFX_OBJECT_H
