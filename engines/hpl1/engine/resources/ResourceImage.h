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

#ifndef HPL_RESOURCE_IMAGE_H
#define HPL_RESOURCE_IMAGE_H

#include "hpl1/engine/resources/ResourceBase.h"

#include "common/array.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"

namespace hpl {

class cFrameTexture;
class cFrameBitmap;
class iTexture;

class cResourceImage : public iResourceBase {
	friend class cImageManager;

public:
	cResourceImage(tString asName, cFrameTexture *apFrameTex,
				   cFrameBitmap *apFrameBmp,
				   cRect2l aRect,
				   cVector2l avSrcSize, int alHandle);

	bool reload();
	void unload();
	void destroy();

	// Image specific
	int GetHeight() const { return mRect.h; }
	int GetWidth() const { return mRect.w; }
	cVector2l GetSize() const { return cVector2l(mRect.w, mRect.h); }
	cVector2l GetPosition() const { return cVector2l(mRect.x, mRect.y); }

	int GetSourceWidth() const { return mvSourceSize.x; }
	int GetSourceHeight() const { return mvSourceSize.y; }

	iTexture *GetTexture() const;

	cFrameTexture *GetFrameTexture() const { return mpFrameTexture; }
	cFrameBitmap *GetFrameBitmap() const { return mpFrameBitmap; }

	tVertexVec GetVertexVecCopy(const cVector2f &avPos, const cVector2f &avSize);
	const tVertexVec &GetVertexVec() { return mvVtx; }

private:
	~cResourceImage();

	cFrameTexture *mpFrameTexture;
	cFrameBitmap *mpFrameBitmap;

	cVector2l mvSourceSize;
	cRect2l mRect;
	tVertexVec mvVtx;

	int mlHandle;
};

typedef Common::Array<cResourceImage *> tResourceImageVec;
typedef tResourceImageVec::iterator tResourceImageVecIt;

} // namespace hpl

#endif // HPL_RESOURCE_IMAGE_H
