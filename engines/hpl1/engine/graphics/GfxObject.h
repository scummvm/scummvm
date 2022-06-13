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
#ifndef HPL_GFX_OBJECT_H
#define HPL_GFX_OBJECT_H

#include <vector>
#include "hpl1/engine/graphics/GraphicsTypes.h"

namespace hpl {

	class iMaterial;

	class cGfxObject
	{
	public:
		cGfxObject(iMaterial* apMat,const tString& asFile, bool abIsImage);
		~cGfxObject();

		iMaterial* GetMaterial() const{ return mpMat;}
		cVertex* GetVtxPtr(int alNum){ return &mvVtx[alNum];}
		tVertexVec* GetVertexVec(){ return &mvVtx;}

		const tString& GetSourceFile(){ return msSourceFile;}

		bool IsImage(){ return mbIsImage;}

	private:
		tVertexVec mvVtx;
		iMaterial* mpMat;
		float mfZ;
		bool mbIsImage;

		tString msSourceFile;
	};

	typedef std::vector<cGfxObject> tGfxObjectVec;
	typedef tGfxObjectVec::iterator tGfxObjectVecIt;
};
#endif // HPL_GFX_OBJECT_H
