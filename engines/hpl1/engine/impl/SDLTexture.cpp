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

#include "hpl1/engine/impl/SDLTexture.h"
#include "hpl1/engine/impl/SDLBitmap2D.h"

#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSDLTexture::cSDLTexture(const tString &asName, iPixelFormat *apPxlFmt, iLowLevelGraphics *apLowLevelGraphics,
						 eTextureType aType, bool abUseMipMaps, eTextureTarget aTarget,
						 bool abCompress)
	: iTexture(asName, "OGL", apPxlFmt, apLowLevelGraphics, aType, abUseMipMaps, aTarget, abCompress) {
#if 0
  		mbContainsData = false;

		mpPBuffer = NULL;

		if(aType==eTextureType_RenderTarget)
		{
			mpPBuffer = hplNew( cPBuffer, (mpLowLevelGraphics,true) );
		}

		//Cubemap does not like mipmaps
		if(aTarget == eTextureTarget_CubeMap) mbUseMipMaps = false;

		mpGfxSDL = static_cast<cLowLevelGraphicsSDL*>(mpLowLevelGraphics);

		mlTextureIndex = 0;
		mfTimeCount =0;

		mfTimeDir = 1;

		mlBpp = 0;
#endif
}

cSDLTexture::~cSDLTexture() {
#if 0
  		if(mpPBuffer)hplDelete(mpPBuffer);

		for(size_t i=0; i<mvTextureHandles.size(); ++i)
		{
			glDeleteTextures(1,(GLuint *)&mvTextureHandles[i]);
		}
#endif
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cSDLTexture::CreateFromBitmap(iBitmap2D *pBmp) {
#if 0
  		//Generate handles
		if(mvTextureHandles.empty())
		{
			mvTextureHandles.resize(1);
			glGenTextures(1,(GLuint *)&mvTextureHandles[0]);
		}
		else
		{
			//Log("Delete + Generate!\n");
			//glDeleteTextures(1,(GLuint *)&mvTextureHandles[0]);
			//glGenTextures(1,(GLuint *)&mvTextureHandles[0]);
		}

		return CreateFromBitmapToHandle(pBmp,0);
#endif
	return false;
}

//-----------------------------------------------------------------------

bool cSDLTexture::CreateAnimFromBitmapVec(tBitmap2DVec *avBitmaps) {
#if 0
  		mvTextureHandles.resize(avBitmaps->size());

		for(size_t i=0; i< mvTextureHandles.size(); ++i)
		{
			glGenTextures(1,(GLuint *)&mvTextureHandles[i]);
			if(CreateFromBitmapToHandle((*avBitmaps)[i],(int)i)==false)
			{
				return false;
			}
		}

		return true;
#endif
	return false;
}

//-----------------------------------------------------------------------

bool cSDLTexture::CreateCubeFromBitmapVec(tBitmap2DVec *avBitmaps) {
#if 0
  		if(mType == eTextureType_RenderTarget || mTarget != eTextureTarget_CubeMap)
		{
			return false;
		}

		if(avBitmaps->size()<6){
			Error("Only %d bitmaps supplied for creation of cube map, 6 needed.",avBitmaps->size());
			return false;
		}

		//Generate handles
		if(mvTextureHandles.empty())
		{
			mvTextureHandles.resize(1);
			glGenTextures(1,(GLuint *)&mvTextureHandles[0]);
		}
		else
		{
			glDeleteTextures(1,(GLuint *)&mvTextureHandles[0]);
			glGenTextures(1,(GLuint *)&mvTextureHandles[0]);
		}

		GLenum GLTarget = InitCreation(0);

		//Create the cube map sides
		for(int i=0; i< 6; i++)
		{
			cSDLBitmap2D *pSrc = static_cast<cSDLBitmap2D*>((*avBitmaps)[i]);

			GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + i;

			int lChannels;
			GLenum format;
			GetSettings(pSrc, lChannels,format);

			glTexImage2D(target, 0, lChannels, pSrc->GetWidth(), pSrc->GetHeight(),
				0, format, GL_UNSIGNED_BYTE, pSrc->GetSurface()->pixels);

			//No mip maps for cubemap
			//if(mbUseMipMaps)
			//{
			//	int x = gluBuild2DMipmaps(target,4,pSrc->GetWidth(), pSrc->GetHeight(),
			//		GL_RGBA, GL_UNSIGNED_BYTE, pSrc->GetSurface()->pixels);
			//}

			mlWidth = pSrc->GetWidth();
			mlHeight = pSrc->GetHeight();
			mlBpp = lChannels * 8;

			if(!cMath::IsPow2(mlHeight) || !cMath::IsPow2(mlWidth))
			{
				Warning("Texture '%s' does not have a pow2 size!\n",msName.c_str());
			}
		}

		PostCreation(GLTarget);

		return true;
#endif
	return false;
}

//-----------------------------------------------------------------------

bool cSDLTexture::Create(unsigned int alWidth, unsigned int alHeight, cColor aCol) {
#if 0
  		//Generate handles
		mvTextureHandles.resize(1);
		glGenTextures(1,(GLuint *)&mvTextureHandles[0]);


		if(mType == eTextureType_RenderTarget)
		{
			if(!mpPBuffer->Init(alWidth,alHeight,aCol)){
				return false;
			}

			mlWidth = alWidth;
			mlHeight = alHeight;

			if((!cMath::IsPow2(mlHeight) || !cMath::IsPow2(mlWidth)) && mTarget != eTextureTarget_Rect)
			{
				Warning("Texture '%s' does not have a pow2 size!\n",msName.c_str());
			}

			GLenum GLTarget = mpGfxSDL->GetGLTextureTargetEnum(mTarget);

			glEnable(GLTarget);
			glBindTexture(GLTarget, mvTextureHandles[0]);
			if(mbUseMipMaps && mTarget != eTextureTarget_Rect){
				if(mFilter == eTextureFilter_Bilinear)
					glTexParameteri(GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				else
					glTexParameteri(GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			}
			else{
				glTexParameteri(GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			}
			glTexParameteri(GLTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GLTarget,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
			glTexParameteri(GLTarget,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

			glDisable(GLTarget);

			mbContainsData = true;
		}
		else
		{
			return false;
		}

		return true;
#endif
	return false;
}

//-----------------------------------------------------------------------

bool cSDLTexture::CreateFromArray(unsigned char *apPixelData, int alChannels, const cVector3l &avSize) {
#if 0
  		if(mvTextureHandles.empty())
		{
			mvTextureHandles.resize(1);
			glGenTextures(1,(GLuint *)&mvTextureHandles[0]);
		}

		GLenum GLTarget = InitCreation(0);

		int lChannels = alChannels;
		GLenum format =0;
		switch(lChannels)
		{
		case 1: format = GL_LUMINANCE; break;
		case 2: format = GL_LUMINANCE_ALPHA; break;
		case 3: format = GL_RGB; break;
		case 4: format = GL_RGBA; break;
		}

		mlWidth = avSize.x;
		mlHeight = avSize.y;
		mlDepth = avSize.z;
		mlBpp = lChannels * 8;

		if(!cMath::IsPow2(mlHeight) || !cMath::IsPow2(mlWidth) || !cMath::IsPow2(mlDepth))
		{
			Warning("Texture '%s' does not have a pow2 size!\n",msName.c_str());
		}

		if(mTarget == eTextureTarget_1D)
		{
			glTexImage1D(GLTarget, 0, lChannels, mlWidth,0,format,
							GL_UNSIGNED_BYTE, apPixelData);
		}
		else if(mTarget == eTextureTarget_2D)
		{
			glTexImage2D(GLTarget, 0, lChannels, mlWidth, mlHeight,
						0, format, GL_UNSIGNED_BYTE, apPixelData);
		}
		else if(mTarget == eTextureTarget_3D)
		{
			glTexImage3D(GLTarget, 0, lChannels, avSize.x, avSize.y,avSize.z,
				0, format, GL_UNSIGNED_BYTE, apPixelData);
		}

		if(mbUseMipMaps && mTarget != eTextureTarget_Rect && mTarget != eTextureTarget_3D)
		{
			if(mTarget == eTextureTarget_1D)
				gluBuild1DMipmaps(GLTarget,lChannels,mlWidth,
				format, GL_UNSIGNED_BYTE, apPixelData);
			else
				gluBuild2DMipmaps(GLTarget,lChannels,mlWidth, mlHeight,
				format, GL_UNSIGNED_BYTE, apPixelData);
		}

		PostCreation(GLTarget);

		return true;
#endif
	return false;
}

//-----------------------------------------------------------------------

void cSDLTexture::SetPixels2D(int alLevel, const cVector2l &avOffset, const cVector2l &avSize,
							  eColorDataFormat aDataFormat, void *apPixelData) {
#if 0
  		if(mTarget != eTextureTarget_2D && mTarget != eTextureTarget_Rect) return;

		glTexSubImage2D(TextureTargetToGL(mTarget),alLevel,avOffset.x,avOffset.y, avSize.x,avSize.y,
						ColorFormatToGL(aDataFormat),GL_UNSIGNED_BYTE,apPixelData);
#endif
}

//-----------------------------------------------------------------------

void cSDLTexture::Update(float afTimeStep) {
	if (mvTextureHandles.size() > 1) {
		float fMax = (float)(mvTextureHandles.size());
		mfTimeCount += afTimeStep * (1.0f / mfFrameTime) * mfTimeDir;

		if (mfTimeDir > 0) {
			if (mfTimeCount >= fMax) {
				if (mAnimMode == eTextureAnimMode_Loop) {
					mfTimeCount = 0;
				} else {
					mfTimeCount = fMax - 1.0f;
					mfTimeDir = -1.0f;
				}
			}
		} else {
			if (mfTimeCount < 0) {
				mfTimeCount = 1;
				mfTimeDir = 1.0f;
			}
		}
	}
}

//-----------------------------------------------------------------------

bool cSDLTexture::HasAnimation() {
	return mvTextureHandles.size() > 1;
}

void cSDLTexture::NextFrame() {
	mfTimeCount += mfTimeDir;

	if (mfTimeDir > 0) {
		float fMax = (float)(mvTextureHandles.size());
		if (mfTimeCount >= fMax) {
			if (mAnimMode == eTextureAnimMode_Loop) {
				mfTimeCount = 0;
			} else {
				mfTimeCount = fMax - 1.0f;
				mfTimeDir = -1.0f;
			}
		}
	} else {
		if (mfTimeCount < 0) {
			mfTimeCount = 1;
			mfTimeDir = 1.0f;
		}
	}
}

void cSDLTexture::PrevFrame() {
	mfTimeCount -= mfTimeDir;

	if (mfTimeDir < 0) {
		float fMax = (float)(mvTextureHandles.size());
		if (mfTimeCount >= fMax) {
			if (mAnimMode == eTextureAnimMode_Loop) {
				mfTimeCount = 0;
			} else {
				mfTimeCount = fMax - 1.0f;
				mfTimeDir = -1.0f;
			}
		}
	} else {
		if (mfTimeCount < 0) {
			mfTimeCount = 1;
			mfTimeDir = 1.0f;
		}
	}
}

float cSDLTexture::GetT() {
	return cMath::Modulus(mfTimeCount, 1.0f);
}

float cSDLTexture::GetTimeCount() {
	return mfTimeCount;
}
void cSDLTexture::SetTimeCount(float afX) {
	mfTimeCount = afX;
}
int cSDLTexture::GetCurrentLowlevelHandle() {
	return GetTextureHandle();
}

//-----------------------------------------------------------------------

void cSDLTexture::SetFilter(eTextureFilter aFilter) {
#if 0
  		if(mFilter == aFilter) return;

		mFilter = aFilter;
		if(mbContainsData)
		{
			GLenum GLTarget = mpGfxSDL->GetGLTextureTargetEnum(mTarget);

			glEnable(GLTarget);
			for(size_t i=0; i < mvTextureHandles.size(); ++i)
			{
				glBindTexture(GLTarget, mvTextureHandles[i]);

				if(mbUseMipMaps && mTarget != eTextureTarget_Rect){
					if(mFilter == eTextureFilter_Bilinear)
						glTexParameteri(GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
					else
						glTexParameteri(GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				}
				else{
					glTexParameteri(GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				}
			}

			glDisable(GLTarget);
		}
#endif
}

//-----------------------------------------------------------------------

void cSDLTexture::SetAnisotropyDegree(float afX) {
#if 0
  		if(!mpLowLevelGraphics->GetCaps(eGraphicCaps_AnisotropicFiltering)) return;
		if(afX < 1.0f) return;
		if(afX > (float) mpLowLevelGraphics->GetCaps(eGraphicCaps_MaxAnisotropicFiltering)) return;

		if(mfAnisotropyDegree == afX) return;

		mfAnisotropyDegree = afX;

		GLenum GLTarget = mpGfxSDL->GetGLTextureTargetEnum(mTarget);

		glEnable(GLTarget);
		for(size_t i=0; i < mvTextureHandles.size(); ++i)
		{
			glBindTexture(GLTarget, mvTextureHandles[i]);

			glTexParameterf(GLTarget,GL_TEXTURE_MAX_ANISOTROPY_EXT ,mfAnisotropyDegree);
		}

		glDisable(GLTarget);
#endif
}

//-----------------------------------------------------------------------

void cSDLTexture::SetWrapS(eTextureWrap aMode) {
#if 0
  		if(mbContainsData)
		{
			GLenum GLTarget = mpGfxSDL->GetGLTextureTargetEnum(mTarget);

			glEnable(GLTarget);
			for(size_t i=0; i < mvTextureHandles.size(); ++i)
			{
				glBindTexture(GLTarget, mvTextureHandles[i]);

				glTexParameteri(GLTarget,GL_TEXTURE_WRAP_S,GetGLWrap(aMode));
			}

			glDisable(GLTarget);
		}
#endif
}

//-----------------------------------------------------------------------

void cSDLTexture::SetWrapT(eTextureWrap aMode) {
#if 0
  		if(mbContainsData)
		{
			GLenum GLTarget = mpGfxSDL->GetGLTextureTargetEnum(mTarget);

			glEnable(GLTarget);
			for(size_t i=0; i < mvTextureHandles.size(); ++i)
			{
				glBindTexture(GLTarget, mvTextureHandles[i]);

				glTexParameteri(GLTarget,GL_TEXTURE_WRAP_T,GetGLWrap(aMode));
			}

			glDisable(GLTarget);
		}
#endif
}

//-----------------------------------------------------------------------

void cSDLTexture::SetWrapR(eTextureWrap aMode) {
#if 0
  		if(mbContainsData)
		{
			GLenum GLTarget = mpGfxSDL->GetGLTextureTargetEnum(mTarget);

			glEnable(GLTarget);
			for(size_t i=0; i < mvTextureHandles.size(); ++i)
			{
				glBindTexture(GLTarget, mvTextureHandles[i]);

				glTexParameteri(GLTarget,GL_TEXTURE_WRAP_R,GetGLWrap(aMode));
			}

			glDisable(GLTarget);
		}
#endif
}

//-----------------------------------------------------------------------

unsigned int cSDLTexture::GetTextureHandle() {
	if (mvTextureHandles.size() > 1) {
		int lFrame = (int)mfTimeCount;
		return mvTextureHandles[lFrame];
	} else {
		return mvTextureHandles[0];
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cSDLTexture::CreateFromBitmapToHandle(iBitmap2D *pBmp, int alHandleIdx) {
#if 0
  		if(mType == eTextureType_RenderTarget)
		{
			Error("Rendertarget cannot be created from bitmap\n");
			return false;
		}

		//For some reason checking for ARB texture is not working on radeon cards.
		/*if(mTarget == eTextureTarget_Rect && !GLEE_ARB_texture_rectangle)
		{
		Error("Rectangle texture target not supported\n");
		return false;
		}*/

		GLenum GLTarget =InitCreation(alHandleIdx);

		cSDLBitmap2D *pBitmapSrc = static_cast<cSDLBitmap2D*>(pBmp);

		mlWidth = pBitmapSrc->GetWidth();
		mlHeight = pBitmapSrc->GetHeight();

		if((!cMath::IsPow2(mlHeight) || !cMath::IsPow2(mlWidth)) && mTarget != eTextureTarget_Rect)
		{
			Warning("Texture '%s' does not have a pow2 size!\n",msName.c_str());
		}

		int lChannels =0;
		GLenum format =0;
		GetSettings(pBitmapSrc,lChannels,format);

		mlBpp = lChannels * 8;

		unsigned char *pPixelSrc = (unsigned char*)pBitmapSrc->GetSurface()->pixels;

		unsigned char *pNewSrc = NULL;
		if(mlSizeLevel>0 && (int)mlWidth > mvMinLevelSize.x*2)
		{
			//Log("OldSize: %d x %d ",mlWidth,mlHeight);

			int lOldW = mlWidth;
			int lOldH = mlHeight;

			int lSizeDiv = (int)pow((float)2,(int)mlSizeLevel);

			mlWidth /= lSizeDiv;
			mlHeight /= lSizeDiv;

			while(mlWidth < (unsigned int)mvMinLevelSize.x)
			{
				mlWidth*=2;
				mlHeight*=2;
				lSizeDiv/=2;
			}

			//Log("NewSize: %d x %d SizeDiv: %d\n",mlWidth,mlHeight,lSizeDiv);

			pNewSrc = hplNewArray( unsigned char, lChannels * mlWidth * mlHeight);

			int lWidthCount = mlWidth;
			int lHeightCount = mlHeight;
			int lOldAdd = lChannels*lSizeDiv;
			int lOldHeightAdd = lChannels*lOldW*(lSizeDiv-1);

			unsigned char *pOldPixel = pPixelSrc;
			unsigned char *pNewPixel = pNewSrc;

			while(lHeightCount)
			{
				memcpy(pNewPixel, pOldPixel,lChannels);

				pOldPixel += lOldAdd;
				pNewPixel += lChannels;

				lWidthCount--;
				if(!lWidthCount)
				{
					lWidthCount = mlWidth;
					lHeightCount--;
					pOldPixel += lOldHeightAdd;
				}
			}

			pPixelSrc = pNewSrc;
		}

		//Log("Loading %s  %d x %d\n",msName.c_str(), pSrc->GetWidth(), pSrc->GetHeight());
		//Log("Channels: %d Format: %x\n",lChannels, format);

		//Clear error flags
		while(glGetError()!=GL_NO_ERROR);

		if(mTarget == eTextureTarget_1D)
			glTexImage1D(GLTarget, 0, lChannels, mlWidth,0,format,
			GL_UNSIGNED_BYTE, pPixelSrc);
		else
			glTexImage2D(GLTarget, 0, lChannels, mlWidth, mlHeight,
			0, format, GL_UNSIGNED_BYTE, pPixelSrc);

		if(glGetError()!=GL_NO_ERROR) return false;

		if(mbUseMipMaps && mTarget != eTextureTarget_Rect)
		{
			if(mTarget == eTextureTarget_1D)
				gluBuild1DMipmaps(GLTarget,lChannels,mlWidth,
				format, GL_UNSIGNED_BYTE, pPixelSrc);
			else
				gluBuild2DMipmaps(GLTarget,lChannels,mlWidth, mlHeight,
				format, GL_UNSIGNED_BYTE, pPixelSrc);
		}

		PostCreation(GLTarget);

		if(mlSizeLevel>0 && pNewSrc)
		{
			hplDeleteArray(pNewSrc);
		}

		return true;
#endif
	return false;
}

//-----------------------------------------------------------------------

GLenum cSDLTexture::InitCreation(int alHandleIdx) {
#if 0
  		GLenum GLTarget = mpGfxSDL->GetGLTextureTargetEnum(mTarget);

		glEnable(GLTarget);
		glBindTexture(GLTarget, mvTextureHandles[alHandleIdx]);

		return GLTarget;
#endif
	return 0;
}

//-----------------------------------------------------------------------

void cSDLTexture::PostCreation(GLenum aGLTarget) {
#if 0
  		if(mbUseMipMaps && mTarget != eTextureTarget_Rect)
		{
			if(mFilter == eTextureFilter_Bilinear)
				glTexParameteri(aGLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			else
				glTexParameteri(aGLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
		else{
			glTexParameteri(aGLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		glTexParameteri(aGLTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(aGLTarget,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(aGLTarget,GL_TEXTURE_WRAP_T,GL_REPEAT);
		glTexParameteri(aGLTarget,GL_TEXTURE_WRAP_R,GL_REPEAT);

		glDisable(aGLTarget);

		mbContainsData = true;
#endif
}

//-----------------------------------------------------------------------

void cSDLTexture::GetSettings(cSDLBitmap2D *apSrc, int &alChannels, GLenum &aFormat) {
#if 0
  		SDL_Surface *pSurface =  apSrc->GetSurface();
		alChannels = pSurface->format->BytesPerPixel;
		aFormat = GL_RGBA;

		tString sType = cString::ToLowerCase(apSrc->msType);

		if(alChannels==4)
		{
			if(sType == "tga")
			{
				aFormat = GL_BGRA_EXT;
			}
			else
			{
				aFormat = GL_RGBA;
			}

		}
		if(alChannels==3)
		{
			if(sType == "tga")
			{
				aFormat = GL_BGR_EXT;
			}
			else
			{
				aFormat = GL_RGB;
			}
		}
		if(alChannels==1)
		{
			aFormat = GL_ALPHA;
		}
#endif
}

//-----------------------------------------------------------------------

GLenum cSDLTexture::GetGLWrap(eTextureWrap aMode) {
#if 0
  		switch(aMode)
		{
		case eTextureWrap_Clamp: return GL_CLAMP;
		case eTextureWrap_Repeat: return GL_REPEAT;
		case eTextureWrap_ClampToEdge: return GL_CLAMP_TO_EDGE;
		case eTextureWrap_ClampToBorder: return GL_CLAMP_TO_BORDER;
		}

		return GL_REPEAT;
#endif
	return 0;
}

//-----------------------------------------------------------------------

} // namespace hpl
