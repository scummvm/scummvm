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

#include "hpl1/engine/impl/texture_tgl.h"
#include "common/str.h"
#include "graphics/pixelformat.h"
#include "hpl1/engine/graphics/bitmap2D.h"

#include "hpl1/debug.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

static void getSettings(Bitmap2D *apSrc, int &alChannels, TGLint &internalFormat, TGLenum &format) {
	alChannels = apSrc->getNumChannels();
	tString sType = cString::ToLowerCase(apSrc->getType());
	const Common::String bmpFormat = apSrc->format().toString();

	if (alChannels == 4) {
		internalFormat = TGL_RGBA;
		if (bmpFormat.contains("BGRA")) {
			format = TGL_BGRA;
		} else {
			format = TGL_RGBA;
		}
	}
	if (alChannels == 3) {
		internalFormat = TGL_RGB;
		if (bmpFormat.contains("BGR")) {
			format = TGL_BGR;
		} else {
			format = TGL_RGB;
		}
	}
	if (alChannels == 1) {
		format = TGL_RED;
		internalFormat = TGL_RED;
	}
}

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

TGLTexture::TGLTexture(const tString &asName, Graphics::PixelFormat *apPxlFmt, iLowLevelGraphics *apLowLevelGraphics,
					   eTextureType aType, bool abUseMipMaps, eTextureTarget aTarget,
					   bool abCompress)
	: iTexture(asName, "OGL", apPxlFmt, apLowLevelGraphics, aType, abUseMipMaps, aTarget, abCompress) {
	mbContainsData = false;

	if (aType == eTextureType_RenderTarget)
		error("texture render target not supported");
	if (mTarget != eTextureTarget_2D)
		error("TGLTexture only supports 2D textures");

	mpGfxSDL = static_cast<LowLevelGraphicsTGL *>(mpLowLevelGraphics);

	mlTextureIndex = 0;
	mfTimeCount = 0;

	mfTimeDir = 1;
}

TGLTexture::~TGLTexture() {
	for (unsigned int &mvTextureHandle : mvTextureHandles) {
		tglDeleteTextures(1, &mvTextureHandle);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool TGLTexture::CreateFromBitmap(Bitmap2D *pBmp) {
	// Generate handles
	if (mvTextureHandles.empty()) {
		mvTextureHandles.resize(1);
		tglGenTextures(1, &mvTextureHandles[0]);
	}

	return CreateFromBitmapToHandle(pBmp, 0);
}

//-----------------------------------------------------------------------

bool TGLTexture::CreateAnimFromBitmapVec(tBitmap2DVec *avBitmaps) {
	mvTextureHandles.resize(avBitmaps->size());

	for (size_t i = 0; i < mvTextureHandles.size(); ++i) {
		tglGenTextures(1, &mvTextureHandles[i]);
		if (CreateFromBitmapToHandle((*avBitmaps)[i], (int)i) == false) {
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------

bool TGLTexture::CreateCubeFromBitmapVec(tBitmap2DVec *avBitmaps) {
	HPL1_UNIMPLEMENTED(TGLTexture::CreateCubeFromBitmapVec);
}

//-----------------------------------------------------------------------

bool TGLTexture::Create(unsigned int alWidth, unsigned int alHeight, cColor aCol) {
	HPL1_UNIMPLEMENTED(TGLTexture::Create);
}

//-----------------------------------------------------------------------

static void generateMipmaps(eTextureTarget target) {
}

bool TGLTexture::CreateFromArray(unsigned char *apPixelData, int alChannels, const cVector3l &avSize) {
	if (mTarget != eTextureTarget_2D)
		error("non-2D textures are not supported in TGLTexture");

	if (mvTextureHandles.empty()) {
		mvTextureHandles.resize(1);
		tglGenTextures(1, &mvTextureHandles[0]);
	}

	TGLenum GLTarget = InitCreation(0);

	if (alChannels == 1 || alChannels == 2)
		error("TGLTexture only supports RGB and RGBA");

	TGLenum format = alChannels == 3 ? TGL_RGB : TGL_RGBA;

	_width = avSize.x;
	_height = avSize.y;
	_bpp = alChannels * 8;

	if (!cMath::IsPow2(_height) || !cMath::IsPow2(_width) || !cMath::IsPow2(avSize.z)) {
		Hpl1::logWarning(Hpl1::kDebugTextures, "texture '%s' does not have a pow2 size", msName.c_str());
	}

	tglTexImage2D(GLTarget, 0, format, _width, _height,
				  0, format, TGL_UNSIGNED_BYTE, apPixelData);

	if (mbUseMipMaps && mTarget != eTextureTarget_Rect && mTarget != eTextureTarget_3D)
		generateMipmaps(mTarget);

	PostCreation(GLTarget);

	return true;
}

//-----------------------------------------------------------------------

void TGLTexture::SetPixels2D(int alLevel, const cVector2l &avOffset, const cVector2l &avSize,
							 eColorDataFormat aDataFormat, void *apPixelData) {
	HPL1_UNIMPLEMENTED(TGLTexture::SetPixels2D);
}

//-----------------------------------------------------------------------

void TGLTexture::Update(float afTimeStep) {
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

bool TGLTexture::HasAnimation() {
	return mvTextureHandles.size() > 1;
}

void TGLTexture::NextFrame() {
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

void TGLTexture::PrevFrame() {
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

float TGLTexture::GetT() {
	return cMath::Modulus(mfTimeCount, 1.0f);
}

float TGLTexture::GetTimeCount() {
	return mfTimeCount;
}
void TGLTexture::SetTimeCount(float afX) {
	mfTimeCount = afX;
}
int TGLTexture::GetCurrentLowlevelHandle() {
	return GetTextureHandle();
}

//-----------------------------------------------------------------------

void TGLTexture::SetFilter(eTextureFilter aFilter) {
	if (mFilter == aFilter)
		return;

	mFilter = aFilter;
	if (mbContainsData) {
		TGLenum GLTarget = GetTGLTextureTargetEnum(mTarget);

		tglEnable(GLTarget);
		for (size_t i = 0; i < mvTextureHandles.size(); ++i) {
			tglBindTexture(GLTarget, mvTextureHandles[i]);

			if (mbUseMipMaps && mTarget != eTextureTarget_Rect) {
				if (mFilter == eTextureFilter_Bilinear)
					tglTexParameteri(GLTarget, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR_MIPMAP_NEAREST);
				else
					tglTexParameteri(GLTarget, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR_MIPMAP_LINEAR);
			} else {
				tglTexParameteri(GLTarget, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR);
			}
		}
		tglDisable(GLTarget);
	}
}

//-----------------------------------------------------------------------

void TGLTexture::SetAnisotropyDegree(float afX) {
}

//-----------------------------------------------------------------------

void TGLTexture::SetWrapS(eTextureWrap aMode) {
	if (mbContainsData) {
		TGLenum GLTarget = GetTGLTextureTargetEnum(mTarget);

		Hpl1::logInfo(Hpl1::kDebugTextures, "setting texture '%s' s wrap to %d\n", msName.c_str(), aMode);

		tglEnable(GLTarget);
		for (size_t i = 0; i < mvTextureHandles.size(); ++i) {
			tglBindTexture(GLTarget, mvTextureHandles[i]);

			tglTexParameteri(GLTarget, TGL_TEXTURE_WRAP_S, GetGLWrap(aMode));
		}
		tglDisable(GLTarget);
	}
}

//-----------------------------------------------------------------------

void TGLTexture::SetWrapT(eTextureWrap aMode) {
	if (mbContainsData) {
		TGLenum GLTarget = GetTGLTextureTargetEnum(mTarget);

		Hpl1::logInfo(Hpl1::kDebugTextures, "setting texture '%s' t wrap to %d\n", msName.c_str(), aMode);

		tglEnable(GLTarget);
		for (size_t i = 0; i < mvTextureHandles.size(); ++i) {
			tglBindTexture(GLTarget, mvTextureHandles[i]);

			tglTexParameteri(GLTarget, TGL_TEXTURE_WRAP_T, GetGLWrap(aMode));
		}
		tglDisable(GLTarget);
	}
}

//-----------------------------------------------------------------------

void TGLTexture::SetWrapR(eTextureWrap aMode) {
	HPL1_UNIMPLEMENTED(TGLTexture::SetWrapR);
}

//-----------------------------------------------------------------------

unsigned int TGLTexture::GetTextureHandle() {
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

bool TGLTexture::CreateFromBitmapToHandle(Bitmap2D *pBmp, int alHandleIdx) {
	TGLenum GLTarget = InitCreation(alHandleIdx);

	Bitmap2D *pBitmapSrc = pBmp;

	_width = pBitmapSrc->getWidth();
	_height = pBitmapSrc->getHeight();

	if ((!cMath::IsPow2(_height) || !cMath::IsPow2(_width)) && mTarget != eTextureTarget_Rect)
		Hpl1::logWarning(Hpl1::kDebugTextures, "texture '%s' does not have a pow2 size", msName.c_str());

	int lChannels = 0;
	TGLint internalFormat = 0;
	TGLenum format = 0;
	getSettings(pBitmapSrc, lChannels, internalFormat, format);

	_bpp = lChannels * 8;

	const unsigned char *pPixelSrc = (const unsigned char *)pBitmapSrc->getRawData();

	unsigned char *pNewSrc = nullptr;
	if (mlSizeLevel > 0 && (int)_width > mvMinLevelSize.x * 2) {
		// Log("OldSize: %d x %d ",mlWidth,mlHeight);

		int lOldW = _width;

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
	tglTexImage2D(GLTarget, 0, internalFormat, _width, _height,
				  0, format, TGL_UNSIGNED_BYTE, pPixelSrc);

	// if (tglGetError() != TGL_NO_ERROR)
	//	return false;

	if (mbUseMipMaps && mTarget != eTextureTarget_Rect)
		generateMipmaps(mTarget);

	PostCreation(GLTarget);
	if (mlSizeLevel > 0 && pNewSrc)
		hplDeleteArray(pNewSrc);

	return true;
}

//-----------------------------------------------------------------------

TGLenum TGLTexture::InitCreation(int alHandleIdx) {
	TGLenum GLTarget = GetTGLTextureTargetEnum(mTarget);

	tglEnable(GLTarget);
	tglBindTexture(GLTarget, mvTextureHandles[alHandleIdx]);

	return GLTarget;
}

//-----------------------------------------------------------------------

void TGLTexture::PostCreation(TGLenum aGLTarget) {
	if (mbUseMipMaps && mTarget != eTextureTarget_Rect) {
		if (mFilter == eTextureFilter_Bilinear)
			tglTexParameteri(aGLTarget, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR_MIPMAP_NEAREST);
		else
			tglTexParameteri(aGLTarget, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR_MIPMAP_LINEAR);
	} else {
		tglTexParameteri(aGLTarget, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR);
	}
	tglTexParameteri(aGLTarget, TGL_TEXTURE_MAG_FILTER, TGL_LINEAR);
	tglDisable(aGLTarget);

	mbContainsData = true;
}

//-----------------------------------------------------------------------

TGLenum TGLTexture::GetGLWrap(eTextureWrap aMode) {
	// FIXME: enable other modes when related bugs are fixed
	return TGL_REPEAT;
#if 0
	switch (aMode) {
	case eTextureWrap_Clamp:
		return TGL_CLAMP;
	case eTextureWrap_Repeat:
		return TGL_REPEAT;
	case eTextureWrap_ClampToEdge:
		return TGL_CLAMP_TO_EDGE;
	case eTextureWrap_ClampToBorder:
		return TGL_CLAMP_TO_EDGE; // TGL_CLAMP_TO_BORDER;
	default:
		break;
	}

	return TGL_REPEAT;
#endif
}

} // namespace hpl
  //-----------------------------------------------------------------------