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

#ifndef HPL_SDL_TEXTURE_H
#define HPL_SDL_TEXTURE_H

#include "common/scummsys.h"
#include "hpl1/engine/graphics/Texture.h"
#include "hpl1/engine/graphics/bitmap2D.h"
#include "hpl1/engine/impl/LowLevelGraphicsSDL.h"

#ifdef USE_OPENGL

namespace hpl {

class cSDLTexture : public iTexture {
public:
	cSDLTexture(const tString &asName, Graphics::PixelFormat *apPxlFmt, iLowLevelGraphics *apLowLevelGraphics,
				eTextureType aType, bool abUseMipMaps, eTextureTarget aTarget,
				bool abCompress = false);
	~cSDLTexture();

	bool CreateFromBitmap(Bitmap2D *pBmp);

	bool CreateAnimFromBitmapVec(tBitmap2DVec *avBitmaps);

	bool CreateCubeFromBitmapVec(tBitmap2DVec *avBitmaps);
	bool Create(unsigned int alWidth, unsigned int alHeight, cColor aCol);

	bool CreateFromArray(unsigned char *apPixelData, int alChannels, const cVector3l &avSize);

	void SetPixels2D(int alLevel, const cVector2l &avOffset, const cVector2l &avSize,
					 eColorDataFormat aDataFormat, void *apPixelData);

	float GetGamma() { return 0; }
	void SetGamma(float afGamma) {}
	int GetHandle() { return (int)mvTextureHandles[0]; }

	void SetFilter(eTextureFilter aFilter);
	void SetAnisotropyDegree(float afX);

	void SetWrapS(eTextureWrap aMode);
	void SetWrapT(eTextureWrap aMode);
	void SetWrapR(eTextureWrap aMode);

	void Update(float afTimeStep);

	bool HasAnimation();
	void NextFrame();
	void PrevFrame();
	float GetT();
	float GetTimeCount();
	void SetTimeCount(float afX);
	int GetCurrentLowlevelHandle();

	/// SDL / OGL Specific ///////////

	unsigned int GetTextureHandle();
	// cPBuffer *GetPBuffer() { return mpPBuffer; }

private:
	bool CreateFromBitmapToHandle(Bitmap2D *pBmp, int alHandleIdx);

	GLenum InitCreation(int alHandleIdx);
	void PostCreation(GLenum aGLTarget);

	GLenum GetGLWrap(eTextureWrap aMode);

	tUIntVec mvTextureHandles;
	bool mbContainsData;
	cLowLevelGraphicsSDL *mpGfxSDL;

	float mfTimeCount;
	int mlTextureIndex;
	float mfTimeDir;
};

} // namespace hpl

#endif // USE_OPENGL
#endif // HPL_SDL_TEXTURE_H
