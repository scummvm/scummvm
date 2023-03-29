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

#include "hpl1/engine/impl/SDLTexture.h"
#include "common/str.h"
#include "graphics/pixelformat.h"
#include "hpl1/engine/graphics/bitmap2D.h"

#include "hpl1/debug.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/system/low_level_system.h"
#include "hpl1/opengl.h"

#ifdef USE_OPENGL

namespace hpl {

static void getSettings(Bitmap2D *apSrc, int &alChannels, GLint &internalFormat, GLenum &format) {
	alChannels = apSrc->getNumChannels();
	tString sType = cString::ToLowerCase(apSrc->getType());
	const Common::String bmpFormat = apSrc->format().toString();

	if (alChannels == 4) {
		internalFormat = GL_RGBA;
		if (bmpFormat.contains("BGRA")) {
			format = GL_BGRA;
		} else {
			format = GL_RGBA;
		}
	}
	if (alChannels == 3) {
		internalFormat = GL_RGB;
		if (bmpFormat.contains("BGR")) {
			format = GL_BGR;
		} else {
			format = GL_RGB;
		}
	}
	if (alChannels == 1) {
		format = GL_RED;
		internalFormat = GL_RED;
	}
}

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSDLTexture::cSDLTexture(const tString &asName, Graphics::PixelFormat *apPxlFmt, iLowLevelGraphics *apLowLevelGraphics,
						 eTextureType aType, bool abUseMipMaps, eTextureTarget aTarget,
						 bool abCompress)
	: iTexture(asName, "OGL", apPxlFmt, apLowLevelGraphics, aType, abUseMipMaps, aTarget, abCompress) {
	mbContainsData = false;

	if (aType == eTextureType_RenderTarget) {
		Hpl1::logError(Hpl1::kDebugTextures, "use of render target%s", ".");
		// mpPBuffer = hplNew( cPBuffer, (mpLowLevelGraphics,true) );
	}

	// Cubemap does not like mipmaps
	if (aTarget == eTextureTarget_CubeMap)
		mbUseMipMaps = false;

	mpGfxSDL = static_cast<cLowLevelGraphicsSDL *>(mpLowLevelGraphics);

	mlTextureIndex = 0;
	mfTimeCount = 0;

	mfTimeDir = 1;
}

cSDLTexture::~cSDLTexture() {
	for (unsigned int &mvTextureHandle : mvTextureHandles) {
		GL_CHECK(glDeleteTextures(1, (GLuint *)&mvTextureHandle));
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cSDLTexture::CreateFromBitmap(Bitmap2D *pBmp) {
	// Generate handles
	if (mvTextureHandles.empty()) {
		mvTextureHandles.resize(1);
		GL_CHECK(glGenTextures(1, &mvTextureHandles[0]));
	} else {
		// Log("Delete + Generate!\n");
		// glDeleteTextures(1,(GLuint *)&mvTextureHandles[0]);
		// glGenTextures(1,(GLuint *)&mvTextureHandles[0]);
	}

	return CreateFromBitmapToHandle(pBmp, 0);
}

//-----------------------------------------------------------------------

bool cSDLTexture::CreateAnimFromBitmapVec(tBitmap2DVec *avBitmaps) {
	mvTextureHandles.resize(avBitmaps->size());

	for (size_t i = 0; i < mvTextureHandles.size(); ++i) {
		glGenTextures(1, (GLuint *)&mvTextureHandles[i]);
		if (CreateFromBitmapToHandle((*avBitmaps)[i], (int)i) == false) {
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------

bool cSDLTexture::CreateCubeFromBitmapVec(tBitmap2DVec *avBitmaps) {
	// gl 1.3
	if (mType == eTextureType_RenderTarget || mTarget != eTextureTarget_CubeMap) {
		return false;
	}

	if (avBitmaps->size() < 6) {
		Hpl1::logError(Hpl1::kDebugTextures, "Only %d bitmaps supplied for creation of cube map, 6 needed.", avBitmaps->size());
		return false;
	}

	// Generate handles
	if (mvTextureHandles.empty()) {
		mvTextureHandles.resize(1);
		GL_CHECK(glGenTextures(1, &mvTextureHandles[0]));
	} else {
		GL_CHECK(glDeleteTextures(1, &mvTextureHandles[0]));
		GL_CHECK(glGenTextures(1, &mvTextureHandles[0]));
	}

	GLenum GLTarget = InitCreation(0);

	// Create the cube map sides
	for (int i = 0; i < 6; i++) {
		Bitmap2D *pSrc = static_cast<Bitmap2D *>((*avBitmaps)[i]);

		GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;

		int lChannels;
		GLenum format;
		GLint internalFormat;
		getSettings(pSrc, lChannels, internalFormat, format);

		glTexImage2D(target, 0, internalFormat, pSrc->getWidth(), pSrc->getHeight(),
					 0, format, GL_UNSIGNED_BYTE, pSrc->getRawData());

		// No mip maps for cubemap
		// if(mbUseMipMaps)
		//{
		//	int x = gluBuild2DMipmaps(target,4,pSrc->GetWidth(), pSrc->GetHeight(),
		//		GL_RGBA, GL_UNSIGNED_BYTE, pSrc->GetSurface()->pixels);
		// }

		_width = pSrc->getWidth();
		_height = pSrc->getHeight();
		_bpp = lChannels * 8;

		if (!cMath::IsPow2(_height) || !cMath::IsPow2(_width)) {
			Hpl1::logWarning(Hpl1::kDebugTextures, "Texture '%s' does not have a pow2 size", msName.c_str());
		}
	}

	PostCreation(GLTarget);

	return true;
}

//-----------------------------------------------------------------------

bool cSDLTexture::Create(unsigned int alWidth, unsigned int alHeight, cColor aCol) {
	error("call to unimplemented function SDLTexture::Create");
}

//-----------------------------------------------------------------------

static void generateMipmaps(eTextureTarget target) {
	// gl 1.4
	if (target == eTextureTarget_1D) {
		GL_CHECK(glGenerateMipmap(GL_TEXTURE_1D))
	} else {
		GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D))
	}
}

bool cSDLTexture::CreateFromArray(unsigned char *apPixelData, int alChannels, const cVector3l &avSize) {
	if (mvTextureHandles.empty()) {
		mvTextureHandles.resize(1);
		GL_CHECK(glGenTextures(1, (GLuint *)&mvTextureHandles[0]));
	}

	GLenum GLTarget = InitCreation(0);

	int lChannels = alChannels;
	GLenum format = 0;
	switch (lChannels) {
	case 1:
		format = GL_R;
		break;
	case 2:
		format = GL_RG;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	}

	_width = avSize.x;
	_height = avSize.y;
	_bpp = lChannels * 8;

	if (!cMath::IsPow2(_height) || !cMath::IsPow2(_width) || !cMath::IsPow2(avSize.z)) {
		Hpl1::logWarning(Hpl1::kDebugTextures, "Texture '%s' does not have a pow2 size", msName.c_str());
	}

	if (mTarget == eTextureTarget_1D) {
		GL_CHECK(glTexImage1D(GLTarget, 0, format, _width, 0, format,
							  GL_UNSIGNED_BYTE, apPixelData));
	} else if (mTarget == eTextureTarget_2D) {
		GL_CHECK(glTexImage2D(GLTarget, 0, format, _width, _height,
							  0, format, GL_UNSIGNED_BYTE, apPixelData));
	} else if (mTarget == eTextureTarget_3D) {
		GL_CHECK(glTexImage3D(GLTarget, 0, format, avSize.x, avSize.y, avSize.z,
							  0, format, GL_UNSIGNED_BYTE, apPixelData));
	}

	if (mbUseMipMaps && mTarget != eTextureTarget_Rect && mTarget != eTextureTarget_3D)
		generateMipmaps(mTarget);

	PostCreation(GLTarget);

	return true;
}

//-----------------------------------------------------------------------

void cSDLTexture::SetPixels2D(int alLevel, const cVector2l &avOffset, const cVector2l &avSize,
							  eColorDataFormat aDataFormat, void *apPixelData) {
	if (mTarget != eTextureTarget_2D && mTarget != eTextureTarget_Rect)
		return;

	GL_CHECK(glTexSubImage2D(TextureTargetToGL(mTarget), alLevel, avOffset.x, avOffset.y, avSize.x, avSize.y,
							 ColorFormatToGL(aDataFormat), GL_UNSIGNED_BYTE, apPixelData));
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
	if (mFilter == aFilter)
		return;

	mFilter = aFilter;
	if (mbContainsData) {
		GLenum GLTarget = mpGfxSDL->GetGLTextureTargetEnum(mTarget);

		GL_CHECK(glEnable(GLTarget));
		for (size_t i = 0; i < mvTextureHandles.size(); ++i) {
			glBindTexture(GLTarget, mvTextureHandles[i]);

			if (mbUseMipMaps && mTarget != eTextureTarget_Rect) {
				if (mFilter == eTextureFilter_Bilinear)
					glTexParameteri(GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				else
					glTexParameteri(GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			} else {
				glTexParameteri(GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			}
		}
		GL_CHECK_FN();
		GL_CHECK(glDisable(GLTarget));
	}
}

//-----------------------------------------------------------------------

void cSDLTexture::SetAnisotropyDegree(float afX) {
// gl 4.6
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

		glTexParameterf(GLTarget, GL_TEXTURE_MAX_ANISOTROPY ,mfAnisotropyDegree);
	}

	glDisable(GLTarget);
#endif
}

//-----------------------------------------------------------------------

void cSDLTexture::SetWrapS(eTextureWrap aMode) {
	if (mbContainsData) {
		GLenum GLTarget = mpGfxSDL->GetGLTextureTargetEnum(mTarget);

		Hpl1::logInfo(Hpl1::kDebugTextures, "setting texture '%s' s wrap to %d\n", msName.c_str(), aMode);

		GL_CHECK(glEnable(GLTarget));
		for (size_t i = 0; i < mvTextureHandles.size(); ++i) {
			glBindTexture(GLTarget, mvTextureHandles[i]);

			glTexParameteri(GLTarget, GL_TEXTURE_WRAP_S, GetGLWrap(aMode));
		}
		GL_CHECK_FN();
		GL_CHECK(glDisable(GLTarget));
	}
}

//-----------------------------------------------------------------------

void cSDLTexture::SetWrapT(eTextureWrap aMode) {
	if (mbContainsData) {
		GLenum GLTarget = mpGfxSDL->GetGLTextureTargetEnum(mTarget);

		Hpl1::logInfo(Hpl1::kDebugTextures, "setting texture '%s' t wrap to %d\n", msName.c_str(), aMode);

		GL_CHECK(glEnable(GLTarget));
		for (size_t i = 0; i < mvTextureHandles.size(); ++i) {
			glBindTexture(GLTarget, mvTextureHandles[i]);

			glTexParameteri(GLTarget, GL_TEXTURE_WRAP_T, GetGLWrap(aMode));
		}
		GL_CHECK(glDisable(GLTarget));
	}
}

//-----------------------------------------------------------------------

void cSDLTexture::SetWrapR(eTextureWrap aMode) {
	if (mbContainsData) {
		GLenum GLTarget = mpGfxSDL->GetGLTextureTargetEnum(mTarget);

		GL_CHECK(glEnable(GLTarget));
		glEnable(GLTarget);
		for (size_t i = 0; i < mvTextureHandles.size(); ++i) {
			glBindTexture(GLTarget, mvTextureHandles[i]);

			glTexParameteri(GLTarget, GL_TEXTURE_WRAP_R, GetGLWrap(aMode));
		}
		GL_CHECK(glDisable(GLTarget));

		glDisable(GLTarget);
	}
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

bool cSDLTexture::CreateFromBitmapToHandle(Bitmap2D *pBmp, int alHandleIdx) {
	if (mType == eTextureType_RenderTarget)
		error("trying to create a rendertarget in SDLTexture::CreateBitmapToHandle");

	// For some reason checking for ARB texture is not working on radeon cards.
	/*if(mTarget == eTextureTarget_Rect && !GLEE_ARB_texture_rectangle)
	{
	Error("Rectangle texture target not supported\n");
	return false;
	}*/

	GLenum GLTarget = InitCreation(alHandleIdx);

	Bitmap2D *pBitmapSrc = pBmp;

	_width = pBitmapSrc->getWidth();
	_height = pBitmapSrc->getHeight();

	if ((!cMath::IsPow2(_height) || !cMath::IsPow2(_width)) && mTarget != eTextureTarget_Rect)
		Hpl1::logWarning(Hpl1::kDebugTextures, "Texture '%s' does not have a pow2 size", msName.c_str());

	int lChannels = 0;
	GLint internalFormat = 0;
	GLenum format = 0;
	getSettings(pBitmapSrc, lChannels, internalFormat, format);

	_bpp = lChannels * 8;

	const unsigned char *pPixelSrc = (const unsigned char *)pBitmapSrc->getRawData();

	unsigned char *pNewSrc = nullptr;
	if (mlSizeLevel > 0 && (int)_width > mvMinLevelSize.x * 2) {
		// Log("OldSize: %d x %d ",mlWidth,mlHeight);

		int lOldW = _width;
		// int lOldH = _height;

		int lSizeDiv = (int)pow((float)2, (int)mlSizeLevel);

		_width /= lSizeDiv;
		_height /= lSizeDiv;

		while (_width < (unsigned int)mvMinLevelSize.x) {
			_width *= 2;
			_height *= 2;
			lSizeDiv /= 2;
		}

		// Log("NewSize: %d x %d SizeDiv: %d\n",mlWidth,mlHeight,lSizeDiv);

		pNewSrc = hplNewArray(unsigned char, lChannels *_width *_height);

		int lWidthCount = _width;
		int lHeightCount = _height;
		int lOldAdd = lChannels * lSizeDiv;
		int lOldHeightAdd = lChannels * lOldW * (lSizeDiv - 1);

		const unsigned char *pOldPixel = pPixelSrc;
		unsigned char *pNewPixel = pNewSrc;

		while (lHeightCount) {
			memcpy(pNewPixel, pOldPixel, lChannels);

			pOldPixel += lOldAdd;
			pNewPixel += lChannels;

			lWidthCount--;
			if (!lWidthCount) {
				lWidthCount = _width;
				lHeightCount--;
				pOldPixel += lOldHeightAdd;
			}
		}

		pPixelSrc = pNewSrc;
	}

	// Log("Loading %s  %d x %d\n",msName.c_str(), pSrc->GetWidth(), pSrc->GetHeight());
	// Log("Channels: %d Format: %x\n",lChannels, format);

	// Clear error flags
	GL_CHECK_FN();

	if (mTarget == eTextureTarget_1D)
		glTexImage1D(GLTarget, 0, internalFormat, _width, 0, format,
					 GL_UNSIGNED_BYTE, pPixelSrc);
	else
		glTexImage2D(GLTarget, 0, internalFormat, _width, _height,
					 0, format, GL_UNSIGNED_BYTE, pPixelSrc);

	if (glGetError() != GL_NO_ERROR)
		return false;

	if (mbUseMipMaps && mTarget != eTextureTarget_Rect)
		generateMipmaps(mTarget);

	PostCreation(GLTarget);
	if (mlSizeLevel > 0 && pNewSrc)
		hplDeleteArray(pNewSrc);

	return true;
}

//-----------------------------------------------------------------------

GLenum cSDLTexture::InitCreation(int alHandleIdx) {
	GLenum GLTarget = mpGfxSDL->GetGLTextureTargetEnum(mTarget);

	GL_CHECK(glEnable(GLTarget));
	GL_CHECK(glBindTexture(GLTarget, mvTextureHandles[alHandleIdx]));

	return GLTarget;
}

//-----------------------------------------------------------------------

void cSDLTexture::PostCreation(GLenum aGLTarget) {
	if (mbUseMipMaps && mTarget != eTextureTarget_Rect) {
		if (mFilter == eTextureFilter_Bilinear)
			glTexParameteri(aGLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		else
			glTexParameteri(aGLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} else {
		glTexParameteri(aGLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	GL_CHECK_FN();
	GL_CHECK(glTexParameteri(aGLTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	if (aGLTarget != GL_TEXTURE_RECTANGLE) {
		GL_CHECK(glTexParameteri(aGLTarget, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL_CHECK(glTexParameteri(aGLTarget, GL_TEXTURE_WRAP_T, GL_REPEAT));
		GL_CHECK(glTexParameteri(aGLTarget, GL_TEXTURE_WRAP_R, GL_REPEAT));
	}
	GL_CHECK(glDisable(aGLTarget));

	mbContainsData = true;
}

//-----------------------------------------------------------------------

GLenum cSDLTexture::GetGLWrap(eTextureWrap aMode) {
	switch (aMode) {
	case eTextureWrap_Clamp:
		return GL_CLAMP;
	case eTextureWrap_Repeat:
		return GL_REPEAT;
	case eTextureWrap_ClampToEdge:
		return GL_CLAMP_TO_EDGE;
	case eTextureWrap_ClampToBorder:
		return GL_CLAMP_TO_BORDER;
	default:
		break;
	}

	return GL_REPEAT;
}

//-----------------------------------------------------------------------

} // namespace hpl

#endif // USE_OPENGL