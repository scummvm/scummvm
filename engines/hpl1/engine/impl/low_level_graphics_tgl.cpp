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

#include "hpl1/engine/impl/low_level_graphics_tgl.h"

#include "hpl1/engine/graphics/bitmap2D.h"
#include "hpl1/engine/graphics/font_data.h"
#include "hpl1/engine/impl/vertex_buffer_tgl.h"
#include "hpl1/engine/system/low_level_system.h"
#include "hpl1/engine/impl/texture_tgl.h"
#include "hpl1/engine/impl/occlusion_query_tgl.h"

#include "common/algorithm.h"
#include "common/system.h"
#include "engines/util.h"
#include "hpl1/debug.h"
#include "graphics/tinygl/tinygl.h"
#include "hpl1/graphics.h"

namespace hpl {

TGLenum ColorFormatToTGL(eColorDataFormat format) {
	switch (format) {
	case eColorDataFormat_RGB:
		return TGL_RGB;
	case eColorDataFormat_RGBA:
		return TGL_RGBA;
	case eColorDataFormat_ALPHA:
		return TGL_ALPHA;
	case eColorDataFormat_BGR:
		return TGL_BGR;
	case eColorDataFormat_BGRA:
		return TGL_BGRA;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid color format (%d)\n", format);
	return 0;
}

TGLenum TextureTargetToTGL(eTextureTarget target) {
	switch (target) {
	case eTextureTarget_1D:
	case eTextureTarget_2D:
		return TGL_TEXTURE_2D;

	case eTextureTarget_Rect:
	case eTextureTarget_CubeMap:
	case eTextureTarget_3D:
	case eTextureTarget_LastEnum:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid texture target (%d)\n", target);
	return 0;
}

TGLenum GetGLBlendEnum(eBlendFunc type) {
	switch (type) {
	case eBlendFunc_Zero:
		return TGL_ZERO;
	case eBlendFunc_One:
		return TGL_ONE;
	case eBlendFunc_SrcColor:
		return TGL_SRC_COLOR;
	case eBlendFunc_OneMinusSrcColor:
		return TGL_ONE_MINUS_SRC_COLOR;
	case eBlendFunc_DestColor:
		return TGL_DST_COLOR;
	case eBlendFunc_OneMinusDestColor:
		return TGL_ONE_MINUS_DST_COLOR;
	case eBlendFunc_SrcAlpha:
		return TGL_SRC_ALPHA;
	case eBlendFunc_OneMinusSrcAlpha:
		return TGL_ONE_MINUS_SRC_ALPHA;
	case eBlendFunc_DestAlpha:
		return TGL_DST_ALPHA;
	case eBlendFunc_OneMinusDestAlpha:
		return TGL_ONE_MINUS_DST_ALPHA;
	case eBlendFunc_SrcAlphaSaturate:
		return TGL_SRC_ALPHA_SATURATE;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid blend op (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

TGLenum GetGLTextureParamEnum(eTextureParam type) {
	switch (type) {
	case eTextureParam_AlphaScale:
		return TGL_ALPHA_SCALE;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid texture parameter (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

TGLenum GetGLTextureOpEnum(eTextureOp type) {
	switch (type) {
	case eTextureOp_Color:
		return TGL_SRC_COLOR;
	case eTextureOp_OneMinusColor:
		return TGL_ONE_MINUS_SRC_COLOR;
	case eTextureOp_Alpha:
		return TGL_SRC_ALPHA;
	case eTextureOp_OneMinusAlpha:
		return TGL_ONE_MINUS_SRC_ALPHA;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid texture op (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

TGLenum GetGLTextureSourceEnum(eTextureSource type) {
	switch (type) {
	case eTextureSource_Texture:
		return TGL_TEXTURE;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid texture source (%d)", type);
	return 0;
}
//-----------------------------------------------------------------------

TGLenum GetTGLTextureTargetEnum(eTextureTarget type) {
	return TGL_TEXTURE_2D;
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid texture target (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

TGLenum GetGLTextureFuncEnum(eTextureFunc type) {
	switch (type) {
	case eTextureFunc_Modulate:
		return TGL_MODULATE;
	case eTextureFunc_Replace:
		return TGL_REPLACE;
	case eTextureFunc_Add:
		return TGL_ADD;

	case eTextureFunc_Substract:
	case eTextureFunc_AddSigned:
	case eTextureFunc_Interpolate:
	case eTextureFunc_Dot3RGB:
	case eTextureFunc_Dot3RGBA:
	case eTextureFunc_LastEnum:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid texture function (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------
TGLenum GetGLDepthTestFuncEnum(eDepthTestFunc type) {
	switch (type) {
	case eDepthTestFunc_Never:
		return TGL_NEVER;
	case eDepthTestFunc_Less:
		return TGL_LESS;
	case eDepthTestFunc_LessOrEqual:
		return TGL_LEQUAL;
	case eDepthTestFunc_Greater:
		return TGL_GREATER;
	case eDepthTestFunc_GreaterOrEqual:
		return TGL_GEQUAL;
	case eDepthTestFunc_Equal:
		return TGL_EQUAL;
	case eDepthTestFunc_NotEqual:
		return TGL_NOTEQUAL;
	case eDepthTestFunc_Always:
		return TGL_ALWAYS;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid depth test function (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

TGLenum GetGLAlphaTestFuncEnum(eAlphaTestFunc type) {
	switch (type) {
	case eAlphaTestFunc_Never:
		return TGL_NEVER;
	case eAlphaTestFunc_Less:
		return TGL_LESS;
	case eAlphaTestFunc_LessOrEqual:
		return TGL_LEQUAL;
	case eAlphaTestFunc_Greater:
		return TGL_GREATER;
	case eAlphaTestFunc_GreaterOrEqual:
		return TGL_GEQUAL;
	case eAlphaTestFunc_Equal:
		return TGL_EQUAL;
	case eAlphaTestFunc_NotEqual:
		return TGL_NOTEQUAL;
	case eAlphaTestFunc_Always:
		return TGL_ALWAYS;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid alpha test function (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

TGLenum GetGLStencilFuncEnum(eStencilFunc type) {
	switch (type) {
	case eStencilFunc_Never:
		return TGL_NEVER;
	case eStencilFunc_Less:
		return TGL_LESS;
	case eStencilFunc_LessOrEqual:
		return TGL_LEQUAL;
	case eStencilFunc_Greater:
		return TGL_GREATER;
	case eStencilFunc_GreaterOrEqual:
		return TGL_GEQUAL;
	case eStencilFunc_Equal:
		return TGL_EQUAL;
	case eStencilFunc_NotEqual:
		return TGL_NOTEQUAL;
	case eStencilFunc_Always:
		return TGL_ALWAYS;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid stencil function (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

TGLenum GetGLStencilOpEnum(eStencilOp type) {
	switch (type) {
	case eStencilOp_Keep:
		return TGL_KEEP;
	case eStencilOp_Zero:
		return TGL_ZERO;
	case eStencilOp_Replace:
		return TGL_REPLACE;
	case eStencilOp_Increment:
		return TGL_INCR;
	case eStencilOp_Decrement:
		return TGL_DECR;
	case eStencilOp_Invert:
		return TGL_INVERT;
	case eStencilOp_IncrementWrap:
		return TGL_INCR_WRAP;
	case eStencilOp_DecrementWrap:
		return TGL_DECR_WRAP;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid stencil op (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetMatrixMode(eMatrix type) {
	switch (type) {
	case eMatrix_ModelView:
		tglMatrixMode(TGL_MODELVIEW);
		break;
	case eMatrix_Projection:
		tglMatrixMode(TGL_PROJECTION);
		break;
	case eMatrix_Texture:
		tglMatrixMode(TGL_TEXTURE);
		break;
	default:
		Hpl1::logError(Hpl1::kDebugOpenGL, "invalid matrix mode (%d)", type);
	}
}

LowLevelGraphicsTGL::LowLevelGraphicsTGL() {
	mlBatchArraySize = 20000;
	mlVertexCount = 0;
	mlIndexCount = 0;
	mlMultisampling = 0;
	mvVirtualSize.x = 800;
	mvVirtualSize.y = 600;
	mfGammaCorrection = 1.0;
	mpRenderTarget = nullptr;
#ifdef SCUMM_BIG_ENDIAN
	mpPixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
	mpPixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif

	Common::fill(mpCurrentTexture, mpCurrentTexture + MAX_TEXTUREUNITS, nullptr);

	mbClearColor = true;
	mbClearDepth = true;
	mbClearStencil = false;

	// Create the batch arrays:
	mlBatchStride = 13;
	// 3 Pos floats, 4 color floats, 3 Tex coord floats .
	mpVertexArray = (float *)hplMalloc(sizeof(float) * mlBatchStride * mlBatchArraySize);
	mpIndexArray = (unsigned int *)hplMalloc(sizeof(unsigned int) * mlBatchArraySize); // Index is one int.

	for (int i = 0; i < MAX_TEXTUREUNITS; i++) {
		mpTexCoordArray[i] = (float *)hplMalloc(sizeof(float) * 3 * mlBatchArraySize);
		mbTexCoordArrayActive[i] = false;
		mlTexCoordArrayCount[i] = 0;
	}
}

LowLevelGraphicsTGL::~LowLevelGraphicsTGL() {
	// SDL_SetGammaRamp(mvStartGammaArray[0],mvStartGammaArray[1],mvStartGammaArray[2]);

	hplFree(mpVertexArray);
	hplFree(mpIndexArray);
	for (int i = 0; i < MAX_TEXTUREUNITS; i++)
		hplFree(mpTexCoordArray[i]);
}

bool LowLevelGraphicsTGL::Init(int alWidth, int alHeight, int alBpp, int abFullscreen,
								int alMultisampling, const tString &asWindowCaption) {
	mvScreenSize.x = alWidth;
	mvScreenSize.y = alHeight;
	mlBpp = alBpp;

	mlMultisampling = alMultisampling;
	initGraphics(alWidth, alHeight, nullptr);
	TinyGL::createContext(alWidth, alHeight, mpPixelFormat, 256, false, 60 * 1024 * 1024);
	SetupGL();
	ShowCursor(false);
	g_system->updateScreen();
	return true;
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::CheckMultisampleCaps() {
}

//-----------------------------------------------------------------------

static void logOGLInfo(const LowLevelGraphicsTGL &graphics) {
	Hpl1::logInfo(Hpl1::kDebugOpenGL, "Max texture image units: %d\n",
				  graphics.GetCaps(eGraphicCaps_MaxTextureImageUnits));
	Hpl1::logInfo(Hpl1::kDebugOpenGL, "Max texture coord units: %d\n",
				  graphics.GetCaps(eGraphicCaps_MaxTextureCoordUnits));
	Hpl1::logInfo(Hpl1::kDebugOpenGL, "Two sided stencil: %d\n",
				  graphics.GetCaps(eGraphicCaps_TwoSideStencil));
	Hpl1::logInfo(Hpl1::kDebugOpenGL, "Vertex Buffer Object: %d\n",
				  graphics.GetCaps(eGraphicCaps_VertexBufferObject));
	Hpl1::logInfo(Hpl1::kDebugOpenGL, "Anisotropic filtering: %d\n",
				  graphics.GetCaps(eGraphicCaps_AnisotropicFiltering));
	Hpl1::logInfo(Hpl1::kDebugOpenGL, "Max Anisotropic degree: %d\n",
				  graphics.GetCaps(eGraphicCaps_MaxAnisotropicFiltering));
	Hpl1::logInfo(Hpl1::kDebugOpenGL, "Multisampling: %d\n",
				  graphics.GetCaps(eGraphicCaps_Multisampling));
}

void LowLevelGraphicsTGL::SetupGL() {
	// Inits GL stuff
	// Set Shade model and clear color.
	tglShadeModel(TGL_SMOOTH);
	tglClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	tglViewport(0, 0, mvScreenSize.x, mvScreenSize.y);

	// Depth Test setup
	tglClearDepth(1.0f);      // VAlues buffer is cleared with
	tglEnable(TGL_DEPTH_TEST); // enable depth testing
	tglDepthFunc(TGL_LEQUAL);  // function to do depth test with
	tglDisable(TGL_ALPHA_TEST);

	// Set best perspective correction
	tglHint(TGL_PERSPECTIVE_CORRECTION_HINT, TGL_NICEST);

	// Stencil setup
	tglClearStencil(0);

	// Clear the screen
	tglClear(TGL_COLOR_BUFFER_BIT | TGL_DEPTH_BUFFER_BIT /*| TGL_STENCIL_BUFFER_BIT*/);

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();

	/////  BEGIN BATCH ARRAY STUFF ///////////////

	// Enable all the vertex arrays that are used:
	tglEnableClientState(TGL_VERTEX_ARRAY);        // The positions
	tglEnableClientState(TGL_COLOR_ARRAY);         // The color
	tglEnableClientState(TGL_TEXTURE_COORD_ARRAY); // Tex coords
	tglDisableClientState(TGL_NORMAL_ARRAY);
	// Disable the once not used.
	//tglDisableClientState(TGL_INDEX_ARRAY); // color index
	//tglDisableClientState(TGL_EDGE_FLAG_ARRAY);

	///// END BATCH ARRAY STUFF ///////////////

	logOGLInfo(*this);
}
//-----------------------------------------------------------------------

int LowLevelGraphicsTGL::GetCaps(eGraphicCaps type) const {
	switch (type) {

	// Texture Rectangle
	case eGraphicCaps_TextureTargetRectangle:
		return 1;

	// Vertex Buffer Object
	case eGraphicCaps_VertexBufferObject:
		return 1; // gl 2.0

	// Two Sided Stencil
	case eGraphicCaps_TwoSideStencil:
		return 1; // gl 2.0

	// Max Texture Image Units
	case eGraphicCaps_MaxTextureImageUnits:
		return 0;
	// Max Texture Coord Units
	case eGraphicCaps_MaxTextureCoordUnits:
		return 0;
	// Texture Anisotropy
	case eGraphicCaps_AnisotropicFiltering:
		return 0; // gl 4.6

	// Texture Anisotropy
	case eGraphicCaps_MaxAnisotropicFiltering:
		return 0; // gl 4.6

	// Multisampling
	case eGraphicCaps_Multisampling:
		return 1; // gl 1.3

	// GL shaders
	case eGraphicCaps_GL_GpuPrograms:
		return 0; // gl 2.0

	case eGraphicCaps_GL_BlendFunctionSeparate:
		return 1; // gl 1.4

	case eGraphicCaps_GL_MultiTexture:
		return 0; // gl 1.4

	default:
		break;
	}
	Hpl1::logWarning(Hpl1::kDebugGraphics, "graphic options %d is not supported\n", type);
	return 0;
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::ShowCursor(bool toggle) {
	g_system->showMouse(toggle);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetVsyncActive(bool toggle) {
	if (g_system->hasFeature(OSystem::kFeatureVSync)) {
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureVSync, toggle);
		g_system->endGFXTransaction();
	}
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetMultisamplingActive(bool toggle) {
	return;
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetGammaCorrection(float afX) {
	mfGammaCorrection = afX;
}

float LowLevelGraphicsTGL::GetGammaCorrection() {
	return mfGammaCorrection;
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetClipPlane(int alIdx, const cPlanef &aPlane) {
	HPL1_UNIMPLEMENTED(LowLevelGraphicsTGL::SetClipPlane);
}

cPlanef LowLevelGraphicsTGL::GetClipPlane(int alIdx, const cPlanef &aPlane) {
	return mvClipPlanes[alIdx];
}

void LowLevelGraphicsTGL::SetClipPlaneActive(int alIdx, bool toggle) {
	HPL1_UNIMPLEMENTED(LowLevelGraphicsTGL::SetClipPlaneActive);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SaveScreenToBMP(const tString &asFile) {
	g_system->saveScreenshot();
}

//-----------------------------------------------------------------------

Bitmap2D *LowLevelGraphicsTGL::CreateBitmap2D(const cVector2l &size) {
	return hplNew(Bitmap2D, (size, mpPixelFormat));
}

//-----------------------------------------------------------------------

FontData *LowLevelGraphicsTGL::CreateFontData(const tString &asName) {
	return hplNew(FontData, (asName, this));
}

//-----------------------------------------------------------------------

iGpuProgram *LowLevelGraphicsTGL::CreateGpuProgram(const tString &vertex, const tString &fragment) {
	HPL1_UNIMPLEMENTED(LowLevelGraphicsTGL::CreateGpuProgram);
}

//-----------------------------------------------------------------------

Graphics::PixelFormat *LowLevelGraphicsTGL::GetPixelFormat() {
	return &mpPixelFormat;
}

//-----------------------------------------------------------------------

iTexture *LowLevelGraphicsTGL::CreateTexture(bool abUseMipMaps, eTextureType aType, eTextureTarget aTarget) {
	if (aTarget != eTextureTarget_2D)
		return nullptr;
	return hplNew(TGLTexture, ("", &mpPixelFormat, this, aType, abUseMipMaps, aTarget));
}

//-----------------------------------------------------------------------

iTexture *LowLevelGraphicsTGL::CreateTexture(const tString &asName, bool abUseMipMaps, eTextureType aType, eTextureTarget aTarget) {
	if (aTarget != eTextureTarget_2D)
		return nullptr;
	return hplNew(TGLTexture, (asName, &mpPixelFormat, this, aType, abUseMipMaps, aTarget));
}

//-----------------------------------------------------------------------

iTexture *LowLevelGraphicsTGL::CreateTexture(Bitmap2D *apBmp, bool abUseMipMaps, eTextureType aType,
											  eTextureTarget aTarget) {
	if (aTarget != eTextureTarget_2D)
		return nullptr;
	TGLTexture *pTex = hplNew(TGLTexture, ("", &mpPixelFormat, this, aType, abUseMipMaps, aTarget));
	pTex->CreateFromBitmap(apBmp);
	return pTex;
}

//-----------------------------------------------------------------------

iTexture *LowLevelGraphicsTGL::CreateTexture(const cVector2l &avSize, int alBpp, cColor aFillCol,
											  bool abUseMipMaps, eTextureType aType, eTextureTarget aTarget) {
	if (aTarget != eTextureTarget_2D)
		return nullptr;

	TGLTexture *pTex = NULL;

	if (aType == eTextureType_RenderTarget) {
		pTex = hplNew(TGLTexture, ("", &mpPixelFormat, this, aType, abUseMipMaps, aTarget));
		pTex->Create(avSize.x, avSize.y, aFillCol);
	} else {
		Bitmap2D *pBmp = CreateBitmap2D(avSize);
		pBmp->fillRect(cRect2l(0, 0, 0, 0), aFillCol);

		pTex = hplNew(TGLTexture, ("", &mpPixelFormat, this, aType, abUseMipMaps, aTarget));
		bool bRet = pTex->CreateFromBitmap(pBmp);

		hplDelete(pBmp);

		if (bRet == false) {
			hplDelete(pTex);
			return NULL;
		}
	}
	return pTex;
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::PushMatrix(eMatrix aMtxType) {
	SetMatrixMode(aMtxType);
	tglPushMatrix();
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::PopMatrix(eMatrix aMtxType) {
	SetMatrixMode(aMtxType);
	tglPopMatrix();
}
//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetMatrix(eMatrix aMtxType, const cMatrixf &a_mtxA) {
	SetMatrixMode(aMtxType);
	cMatrixf mtxTranpose = a_mtxA.GetTranspose();
	tglLoadMatrixf(mtxTranpose.v);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetIdentityMatrix(eMatrix aMtxType) {
	SetMatrixMode(aMtxType);
	tglLoadIdentity();
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::TranslateMatrix(eMatrix aMtxType, const cVector3f &avPos) {
	SetMatrixMode(aMtxType);
	tglTranslatef(avPos.x, avPos.y, avPos.z);
}

//-----------------------------------------------------------------------

/**
 * \todo fix so that there are X, Y , Z versions of this one.
 * \param aMtxType
 * \param &avRot
 */
void LowLevelGraphicsTGL::RotateMatrix(eMatrix aMtxType, const cVector3f &avRot) {
	SetMatrixMode(aMtxType);
	tglRotatef(1, avRot.x, avRot.y, avRot.z);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::ScaleMatrix(eMatrix aMtxType, const cVector3f &avScale) {
	SetMatrixMode(aMtxType);
	tglScalef(avScale.x, avScale.y, avScale.z);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetOrthoProjection(const cVector2f &avSize, float afMin, float afMax) {
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tglOrtho(0, avSize.x, avSize.y, 0, afMin, afMax);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetTexture(unsigned int alUnit, iTexture *apTex) {
	if (apTex == mpCurrentTexture[alUnit])
		return;

	TGLenum NewTarget = 0;
	if (apTex)
		NewTarget = GetTGLTextureTargetEnum(apTex->GetTarget());
	TGLenum LastTarget = 0;
	if (mpCurrentTexture[alUnit])
		LastTarget = GetTGLTextureTargetEnum(mpCurrentTexture[alUnit]->GetTarget());

	// If the current texture in this unit is a render target, unbind it.
	if (mpCurrentTexture[alUnit] && mpCurrentTexture[alUnit]->GetTextureType() == eTextureType_RenderTarget)
		error("render target not supported");

	// Disable this unit if NULL
	if (apTex == NULL) {
		tglDisable(LastTarget);
		// tglBindTexture(LastTarget,0);
		// Enable the unit, set the texture handle and bind the pbuffer
	} else {
		if (NewTarget != LastTarget && LastTarget != 0)
			tglDisable(LastTarget);

		TGLTexture *pSDLTex = static_cast<TGLTexture *>(apTex);

		tglBindTexture(NewTarget, pSDLTex->GetTextureHandle());
		tglEnable(NewTarget);

		// if it is a render target we need to do some more binding.
		if (pSDLTex->GetTextureType() == eTextureType_RenderTarget) {
			error("render target not supported");
		}
	}

	mpCurrentTexture[alUnit] = apTex;
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetActiveTextureUnit(unsigned int alUnit) {
	//tglActiveTexture(GL_TEXTURE0 + alUnit);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetTextureEnv(eTextureParam aParam, int alVal) {
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetTextureConstantColor(const cColor &color) {
	//float vColor[] = {color.r, color.g, color.b, color.a};

	//tglTexEnvfv(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_COLOR, &vColor[0]);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetColor(const cColor &aColor) {
	tglColor4f(aColor.r, aColor.g, aColor.b, aColor.a);
}

//-----------------------------------------------------------------------

iVertexBuffer *LowLevelGraphicsTGL::CreateVertexBuffer(tVertexFlag aFlags,
														eVertexBufferDrawType aDrawType, eVertexBufferUsageType aUsageType, int alReserveVtxSize, int alReserveIdxSize) {

	return hplNew(VertexBufferTGL, (this, aFlags, aDrawType, aUsageType, alReserveVtxSize, alReserveIdxSize));
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DrawRect(const cVector2f &avPos, const cVector2f &avSize, float afZ) {
	tglColor4f(1, 1, 1, 1);
	tglBegin(TGL_QUADS);
	{
		tglTexCoord2f(0.0, 0.0);
		tglVertex3f(avPos.x, avPos.y, afZ);
		tglTexCoord2f(1.0, 0.0);
		tglVertex3f(avPos.x + avSize.x, avPos.y, afZ);
		tglTexCoord2f(1.0, 1.0);
		tglVertex3f(avPos.x + avSize.x, avPos.y + avSize.y, afZ);
		tglTexCoord2f(0.0, 1.0);
		tglVertex3f(avPos.x, avPos.y + avSize.y, afZ);
	}
	tglEnd();
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::FlushRendering() {
	tglFlush();
}
void LowLevelGraphicsTGL::SwapBuffers() {
	tglFlush();
	TinyGL::presentBuffer();
	Graphics::Surface glBuffer;
	TinyGL::getSurfaceRef(glBuffer);
	g_system->copyRectToScreen(glBuffer.getPixels(), glBuffer.pitch,
									0, 0, glBuffer.w, glBuffer.h);
	g_system->updateScreen();
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DrawTri(const tVertexVec &avVtx) {
	assert(avVtx.size() == 3);

	tglBegin(TGL_TRIANGLES);
	{
		for (int i = 0; i < 3; i++) {
			tglTexCoord3f(avVtx[i].tex.x, avVtx[i].tex.y, avVtx[i].tex.z);
			tglColor4f(avVtx[i].col.r, avVtx[i].col.g, avVtx[i].col.b, avVtx[i].col.a);
			tglVertex3f(avVtx[i].pos.x, avVtx[i].pos.y, avVtx[i].pos.z);
		}
	}
	tglEnd();
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DrawTri(const cVertex *avVtx) {
	tglBegin(TGL_TRIANGLES);
	{
		for (int i = 0; i < 3; i++) {
			tglTexCoord3f(avVtx[i].tex.x, avVtx[i].tex.y, avVtx[i].tex.z);
			tglColor4f(avVtx[i].col.r, avVtx[i].col.g, avVtx[i].col.b, avVtx[i].col.a);
			tglVertex3f(avVtx[i].pos.x, avVtx[i].pos.y, avVtx[i].pos.z);
		}
	}
	tglEnd();
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DrawQuad(const tVertexVec &avVtx) {
	assert(avVtx.size() == 4);

	tglBegin(TGL_QUADS);
	{
		for (int i = 0; i < 4; i++) {
			tglTexCoord3f(avVtx[i].tex.x, avVtx[i].tex.y, avVtx[i].tex.z);
			tglColor4f(avVtx[i].col.r, avVtx[i].col.g, avVtx[i].col.b, avVtx[i].col.a);
			tglVertex3f(avVtx[i].pos.x, avVtx[i].pos.y, avVtx[i].pos.z);
		}
	}
	tglEnd();
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DrawQuadMultiTex(const tVertexVec &avVtx, const tVector3fVec &avExtraUvs) {
	return;
}

//-----------------------------------------------------------------------

iOcclusionQuery *LowLevelGraphicsTGL::CreateOcclusionQuery() {
	return hplNew(OcclusionQueryTGL, ());
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DestroyOcclusionQuery(iOcclusionQuery *apQuery) {
	if (apQuery)
		hplDelete(apQuery);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::ClearScreen() {
	TGLbitfield bitmask = 0;

	if (mbClearColor)
		bitmask |= TGL_COLOR_BUFFER_BIT;
	if (mbClearDepth)
		bitmask |= TGL_DEPTH_BUFFER_BIT;
	//if (mbClearStencil)
	//	bitmask |= TGL_STENCIL_BUFFER_BIT;

	tglClear(bitmask);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetClearColor(const cColor &aCol) {
	tglClearColor(aCol.r, aCol.g, aCol.b, aCol.a);
}
void LowLevelGraphicsTGL::SetClearDepth(float afDepth) {
	tglClearDepth(afDepth);
}
void LowLevelGraphicsTGL::SetClearStencil(int alVal) {
	tglClearStencil(alVal);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetClearColorActive(bool abX) {
	mbClearColor = abX;
}
void LowLevelGraphicsTGL::SetClearDepthActive(bool abX) {
	mbClearDepth = abX;
}
void LowLevelGraphicsTGL::SetClearStencilActive(bool abX) {
	mbClearStencil = abX;
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetColorWriteActive(bool abR, bool abG, bool abB, bool abA) {
	tglColorMask(abR, abG, abB, abA);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetDepthWriteActive(bool abX) {
	tglDepthMask(abX);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetDepthTestActive(bool abX) {
	if (abX)
		tglEnable(TGL_DEPTH_TEST);
	else
		tglDisable(TGL_DEPTH_TEST);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetDepthTestFunc(eDepthTestFunc aFunc) {
	tglDepthFunc(GetGLDepthTestFuncEnum(aFunc));
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetAlphaTestActive(bool abX) {
	if (abX)
		tglEnable(TGL_ALPHA_TEST);
	else
		tglDisable(TGL_ALPHA_TEST);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetAlphaTestFunc(eAlphaTestFunc aFunc, float afRef) {
	tglAlphaFunc(GetGLAlphaTestFuncEnum(aFunc), afRef);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetStencilActive(bool abX) {
	if (abX)
		tglEnable(TGL_STENCIL_TEST);
	else
		tglDisable(TGL_STENCIL_TEST);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetStencil(eStencilFunc aFunc, int alRef, unsigned int aMask,
									  eStencilOp aFailOp, eStencilOp aZFailOp, eStencilOp aZPassOp) {
	tglStencilFunc(GetGLStencilFuncEnum(aFunc), alRef, aMask);

	tglStencilOp(GetGLStencilOpEnum(aFailOp), GetGLStencilOpEnum(aZFailOp),
						 GetGLStencilOpEnum(aZPassOp));
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetStencilTwoSide(eStencilFunc aFrontFunc, eStencilFunc aBackFunc,
											 int alRef, unsigned int aMask, eStencilOp aFrontFailOp, eStencilOp aFrontZFailOp, eStencilOp aFrontZPassOp,
											 eStencilOp aBackFailOp, eStencilOp aBackZFailOp, eStencilOp aBackZPassOp) {
	error("Only single sided stencil supported");
}

void LowLevelGraphicsTGL::SetStencilTwoSide(bool abX) {
	if (!GetCaps(eGraphicCaps_TwoSideStencil))
		Hpl1::logError(Hpl1::kDebugOpenGL, "call to setStencilTwoSide with two side stencil disabled%c\n", '.');
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetCullActive(bool abX) {
	if (abX)
		tglEnable(TGL_CULL_FACE);
	else
		tglDisable(TGL_CULL_FACE);
	tglCullFace(TGL_BACK);
}
void LowLevelGraphicsTGL::SetCullMode(eCullMode aMode) {
	tglCullFace(TGL_BACK);
	if (aMode == eCullMode_Clockwise)
		tglFrontFace(TGL_CCW);
	else
		tglFrontFace(TGL_CW);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetScissorActive(bool toggle) {
	if (toggle)
		tglEnable(TGL_SCISSOR_TEST);
	else
		tglDisable(TGL_SCISSOR_TEST);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetScissorRect(const cRect2l &aRect) {
	//tglScissor(aRect.x, (mvScreenSize.y - aRect.y - 1) - aRect.h, aRect.w, aRect.h);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetBlendActive(bool abX) {
	if (abX)
		tglEnable(TGL_BLEND);
	else
		tglDisable(TGL_BLEND);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetBlendFunc(eBlendFunc aSrcFactor, eBlendFunc aDestFactor) {
	tglBlendFunc(GetGLBlendEnum(aSrcFactor), GetGLBlendEnum(aDestFactor));
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetBlendFuncSeparate(eBlendFunc aSrcFactorColor, eBlendFunc aDestFactorColor,
												eBlendFunc aSrcFactorAlpha, eBlendFunc aDestFactorAlpha) {
	HPL1_UNIMPLEMENTED(LowLevelGraphicsTGL::SetBlendFuncSeparate);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DrawQuad(const tVertexVec &avVtx, const cColor aCol) {
	assert(avVtx.size() == 4);

	tglBegin(TGL_QUADS);
	{
		// Make all this inline??
		for (int i = 0; i < 4; i++) {
			tglTexCoord3f(avVtx[i].tex.x, avVtx[i].tex.y, avVtx[i].tex.z);
			tglColor4f(aCol.r, aCol.g, aCol.b, aCol.a);
			tglVertex3f(avVtx[i].pos.x, avVtx[i].pos.y, avVtx[i].pos.z);
		}
	}
	tglEnd();
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DrawQuad(const tVertexVec &avVtx, const float afZ) {
	assert(avVtx.size() == 4);

	tglBegin(TGL_QUADS);
	{
		// Make all this inline??
		for (int i = 0; i < 4; i++) {
			tglTexCoord3f(avVtx[i].tex.x, avVtx[i].tex.y, afZ);
			tglColor4f(avVtx[i].col.r, avVtx[i].col.g, avVtx[i].col.b, avVtx[i].col.a);
			tglVertex3f(avVtx[i].pos.x, avVtx[i].pos.y, avVtx[i].pos.z);
		}
	}
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DrawQuad(const tVertexVec &avVtx, const float afZ, const cColor &aCol) {
	assert(avVtx.size() == 4);

	tglBegin(TGL_QUADS);
	{
		// Make all this inline??
		for (int i = 0; i < 4; i++) {
			tglTexCoord3f(avVtx[i].tex.x, avVtx[i].tex.y, afZ);
			tglColor4f(aCol.r, aCol.g, aCol.b, aCol.a);
			tglVertex3f(avVtx[i].pos.x, avVtx[i].pos.y, avVtx[i].pos.z);
		}
	}
	tglEnd();
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::AddVertexToBatch(const cVertex &apVtx) {
	// Coord
	mpVertexArray[mlVertexCount + 0] = apVtx.pos.x;
	mpVertexArray[mlVertexCount + 1] = apVtx.pos.y;
	mpVertexArray[mlVertexCount + 2] = apVtx.pos.z;
	// Color
	mpVertexArray[mlVertexCount + 3] = apVtx.col.r;
	mpVertexArray[mlVertexCount + 4] = apVtx.col.g;
	mpVertexArray[mlVertexCount + 5] = apVtx.col.b;
	mpVertexArray[mlVertexCount + 6] = apVtx.col.a;
	// Texture coord
	mpVertexArray[mlVertexCount + 7] = apVtx.tex.x;
	mpVertexArray[mlVertexCount + 8] = apVtx.tex.y;
	mpVertexArray[mlVertexCount + 9] = apVtx.tex.z;
	// Normal coord
	mpVertexArray[mlVertexCount + 10] = apVtx.norm.x;
	mpVertexArray[mlVertexCount + 11] = apVtx.norm.y;
	mpVertexArray[mlVertexCount + 12] = apVtx.norm.z;

	mlVertexCount = mlVertexCount + mlBatchStride;

	if (mlVertexCount / mlBatchStride >= mlBatchArraySize) {
		// Make the array larger.
	}
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::AddVertexToBatch(const cVertex *apVtx, const cVector3f *avTransform) {
	// Coord
	mpVertexArray[mlVertexCount + 0] = apVtx->pos.x + avTransform->x;
	mpVertexArray[mlVertexCount + 1] = apVtx->pos.y + avTransform->y;
	mpVertexArray[mlVertexCount + 2] = apVtx->pos.z + avTransform->z;

	/*Log("Trans: %s\n",avTransform->ToString().c_str());
	Log("Adding: %f:%f:%f\n",mpVertexArray[mlVertexCount + 0],
								mpVertexArray[mlVertexCount + 1],
								mpVertexArray[mlVertexCount + 2]);*/
	// Color
	mpVertexArray[mlVertexCount + 3] = apVtx->col.r;
	mpVertexArray[mlVertexCount + 4] = apVtx->col.g;
	mpVertexArray[mlVertexCount + 5] = apVtx->col.b;
	mpVertexArray[mlVertexCount + 6] = apVtx->col.a;
	// Texture coord
	mpVertexArray[mlVertexCount + 7] = apVtx->tex.x;
	mpVertexArray[mlVertexCount + 8] = apVtx->tex.y;
	mpVertexArray[mlVertexCount + 9] = apVtx->tex.z;

	/*Log("Tex: %f:%f:%f\n",mpVertexArray[mlVertexCount + 7],
		mpVertexArray[mlVertexCount + 8],
		mpVertexArray[mlVertexCount + 9]);*/

	// Normal coord
	mpVertexArray[mlVertexCount + 10] = apVtx->norm.x;
	mpVertexArray[mlVertexCount + 11] = apVtx->norm.y;
	mpVertexArray[mlVertexCount + 12] = apVtx->norm.z;

	mlVertexCount = mlVertexCount + mlBatchStride;

	if (mlVertexCount / mlBatchStride >= mlBatchArraySize) {
		// Make the array larger.
	}
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::AddVertexToBatch(const cVertex *apVtx, const cMatrixf *aMtx) {
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::AddVertexToBatch_Size2D(const cVertex *apVtx, const cVector3f *avTransform,
												   const cColor *apCol, const float &mfW, const float &mfH) {
	// Coord
	mpVertexArray[mlVertexCount + 0] = avTransform->x + mfW;
	mpVertexArray[mlVertexCount + 1] = avTransform->y + mfH;
	mpVertexArray[mlVertexCount + 2] = avTransform->z;

	// Color
	mpVertexArray[mlVertexCount + 3] = apCol->r;
	mpVertexArray[mlVertexCount + 4] = apCol->g;
	mpVertexArray[mlVertexCount + 5] = apCol->b;
	mpVertexArray[mlVertexCount + 6] = apCol->a;

	// Texture coord
	mpVertexArray[mlVertexCount + 7] = apVtx->tex.x;
	mpVertexArray[mlVertexCount + 8] = apVtx->tex.y;
	mpVertexArray[mlVertexCount + 9] = apVtx->tex.z;

	mlVertexCount = mlVertexCount + mlBatchStride;

	if (mlVertexCount / mlBatchStride >= mlBatchArraySize) {
		// Make the array larger.
	}
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::AddVertexToBatch_Raw(const cVector3f &avPos, const cColor &aColor,
												const cVector3f &avTex) {
	// Coord
	mpVertexArray[mlVertexCount + 0] = avPos.x;
	mpVertexArray[mlVertexCount + 1] = avPos.y;
	mpVertexArray[mlVertexCount + 2] = avPos.z;

	// Color
	mpVertexArray[mlVertexCount + 3] = aColor.r;
	mpVertexArray[mlVertexCount + 4] = aColor.g;
	mpVertexArray[mlVertexCount + 5] = aColor.b;
	mpVertexArray[mlVertexCount + 6] = aColor.a;

	// Texture coord
	mpVertexArray[mlVertexCount + 7] = avTex.x;
	mpVertexArray[mlVertexCount + 8] = avTex.y;
	mpVertexArray[mlVertexCount + 9] = avTex.z;

	mlVertexCount = mlVertexCount + mlBatchStride;
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::AddIndexToBatch(int alIndex) {
	mpIndexArray[mlIndexCount] = alIndex;
	mlIndexCount++;

	if (mlIndexCount >= mlBatchArraySize) {
		// Make the array larger.
	}
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::AddTexCoordToBatch(unsigned int alUnit, const cVector3f *apCoord) {
	unsigned int lCount = mlTexCoordArrayCount[alUnit];

	mpTexCoordArray[alUnit][lCount + 0] = apCoord->x;
	mpTexCoordArray[alUnit][lCount + 1] = apCoord->y;
	mpTexCoordArray[alUnit][lCount + 2] = apCoord->z;

	mlTexCoordArrayCount[alUnit] += 3;
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetBatchTextureUnitActive(unsigned int alUnit, bool active) {
	return;
}

//-----------------------------------------------------------------------

static void flushAutoClear(unsigned &indexCount, unsigned &vertexCount, unsigned *texCoordArray) {
	indexCount = 0;
	vertexCount = 0;
	Common::fill(texCoordArray, texCoordArray + MAX_TEXTUREUNITS, 0);
}

void LowLevelGraphicsTGL::FlushTriBatch(tVtxBatchFlag aTypeFlags, bool abAutoClear) {
	SetVtxBatchStates(aTypeFlags);
	SetUpBatchArrays();
	tglDrawElements(TGL_TRIANGLES, mlIndexCount, TGL_UNSIGNED_INT, mpIndexArray);
	if (abAutoClear)
		flushAutoClear(mlIndexCount, mlVertexCount, mlTexCoordArrayCount);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::FlushQuadBatch(tVtxBatchFlag aTypeFlags, bool abAutoClear) {
	SetVtxBatchStates(aTypeFlags);
	SetUpBatchArrays();
	tglDrawElements(TGL_QUADS, mlIndexCount, TGL_UNSIGNED_INT, mpIndexArray);
	if (abAutoClear)
		flushAutoClear(mlIndexCount, mlVertexCount, mlTexCoordArrayCount);
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::ClearBatch() {
	mlIndexCount = 0;
	mlVertexCount = 0;
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DrawLine(const cVector3f &avBegin, const cVector3f &avEnd, cColor aCol) {
	SetTexture(0, nullptr);
	// SetBlendActive(false);
	tglColor4f(aCol.r, aCol.g, aCol.b, aCol.a);
	tglBegin(TGL_LINES);
	{
		tglVertex3f(avBegin.x, avBegin.y, avBegin.z);
		tglVertex3f(avEnd.x, avEnd.y, avEnd.z);
	}
	tglEnd();
}

void LowLevelGraphicsTGL::DrawBoxMaxMin(const cVector3f &avMax, const cVector3f &avMin, cColor aCol) {
	SetTexture(0, NULL);
	SetBlendActive(false);
	tglColor4f(aCol.r, aCol.g, aCol.b, aCol.a);

	tglBegin(TGL_LINES);
	{
		// Pos Z Quad
		tglVertex3f(avMax.x, avMax.y, avMax.z);
		tglVertex3f(avMin.x, avMax.y, avMax.z);

		tglVertex3f(avMax.x, avMax.y, avMax.z);
		tglVertex3f(avMax.x, avMin.y, avMax.z);

		tglVertex3f(avMin.x, avMax.y, avMax.z);
		tglVertex3f(avMin.x, avMin.y, avMax.z);

		tglVertex3f(avMin.x, avMin.y, avMax.z);
		tglVertex3f(avMax.x, avMin.y, avMax.z);

		// Neg Z Quad
		tglVertex3f(avMax.x, avMax.y, avMin.z);
		tglVertex3f(avMin.x, avMax.y, avMin.z);

		tglVertex3f(avMax.x, avMax.y, avMin.z);
		tglVertex3f(avMax.x, avMin.y, avMin.z);

		tglVertex3f(avMin.x, avMax.y, avMin.z);
		tglVertex3f(avMin.x, avMin.y, avMin.z);

		tglVertex3f(avMin.x, avMin.y, avMin.z);
		tglVertex3f(avMax.x, avMin.y, avMin.z);

		// Lines between
		tglVertex3f(avMax.x, avMax.y, avMax.z);
		tglVertex3f(avMax.x, avMax.y, avMin.z);

		tglVertex3f(avMin.x, avMax.y, avMax.z);
		tglVertex3f(avMin.x, avMax.y, avMin.z);

		tglVertex3f(avMin.x, avMin.y, avMax.z);
		tglVertex3f(avMin.x, avMin.y, avMin.z);

		tglVertex3f(avMax.x, avMin.y, avMax.z);
		tglVertex3f(avMax.x, avMin.y, avMin.z);
	}
	tglEnd();
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DrawSphere(const cVector3f &avPos, float afRadius, cColor aCol) {
	int alSegments = 32;
	float afAngleStep = k2Pif / (float)alSegments;

	SetTexture(0, nullptr);
	SetBlendActive(false);
	tglColor4f(aCol.r, aCol.g, aCol.b, aCol.a);
	tglBegin(TGL_LINES);
	{
		// X Circle:
		for (float a = 0; a < k2Pif; a += afAngleStep) {
			tglVertex3f(avPos.x, avPos.y + sin(a) * afRadius,
					   avPos.z + cos(a) * afRadius);

			tglVertex3f(avPos.x, avPos.y + sin(a + afAngleStep) * afRadius,
					   avPos.z + cos(a + afAngleStep) * afRadius);
		}

		// Y Circle:
		for (float a = 0; a < k2Pif; a += afAngleStep) {
			tglVertex3f(avPos.x + cos(a) * afRadius, avPos.y,
					   avPos.z + sin(a) * afRadius);

			tglVertex3f(avPos.x + cos(a + afAngleStep) * afRadius, avPos.y,
					   avPos.z + sin(a + afAngleStep) * afRadius);
		}

		// Z Circle:
		for (float a = 0; a < k2Pif; a += afAngleStep) {
			tglVertex3f(avPos.x + cos(a) * afRadius, avPos.y + sin(a) * afRadius, avPos.z);

			tglVertex3f(avPos.x + cos(a + afAngleStep) * afRadius,
					   avPos.y + sin(a + afAngleStep) * afRadius,
					   avPos.z);
		}
	}
	tglEnd();
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DrawLine2D(const cVector2f &avBegin, const cVector2f &avEnd, float afZ, cColor aCol) {
	SetTexture(0, NULL);
	SetBlendActive(false);
	tglColor4f(aCol.r, aCol.g, aCol.b, aCol.a);
	tglBegin(TGL_LINES);
	{
		tglVertex3f(avBegin.x, avBegin.y, afZ);
		tglVertex3f(avEnd.x, avEnd.y, afZ);
	}
	tglEnd();
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DrawLineRect2D(const cRect2f &aRect, float afZ, cColor aCol) {
	SetTexture(0, nullptr);
	SetBlendActive(false);
	tglColor4f(aCol.r, aCol.g, aCol.b, aCol.a);
	tglBegin(TGL_LINE_STRIP);
	{
		tglVertex3f(aRect.x, aRect.y, afZ);
		tglVertex3f(aRect.x + aRect.w, aRect.y, afZ);
		tglVertex3f(aRect.x + aRect.w, aRect.y + aRect.h, afZ);
		tglVertex3f(aRect.x, aRect.y + aRect.h, afZ);
		tglVertex3f(aRect.x, aRect.y, afZ);
	}
	tglEnd();
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DrawFilledRect2D(const cRect2f &aRect, float afZ, cColor aCol) {
	SetTexture(0, NULL);
	tglColor4f(aCol.r, aCol.g, aCol.b, aCol.a);
	tglBegin(TGL_QUADS);
	{
		tglVertex3f(aRect.x, aRect.y, afZ);
		tglVertex3f(aRect.x + aRect.w, aRect.y, afZ);
		tglVertex3f(aRect.x + aRect.w, aRect.y + aRect.h, afZ);
		tglVertex3f(aRect.x, aRect.y + aRect.h, afZ);
	}
	tglEnd();
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::DrawLineCircle2D(const cVector2f &avCenter, float afRadius, float afZ, cColor aCol) {
	// Implement later
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::CopyContextToTexure(iTexture *apTex, const cVector2l &avPos,
											   const cVector2l &avSize, const cVector2l &avTexOffset) {
#if 0
	if (apTex == nullptr)
		return;

	int lScreenY = (mvScreenSize.y - avSize.y) - avPos.y;
	int lTexY = (apTex->getHeight() - avSize.y) - avTexOffset.y;

	// Log("TExoffset: %d %d\n",avTexOffset.x,lTexY);
	// Log("ScreenOffset: %d %d (h: %d s: %d p: %d)\n",avPos.x,lScreenY,mvScreenSize.y,
	//												avSize.y,avPos.y);

	SetTexture(0, apTex);
	tglCopyTexSubImage2D(GetGLTextureTargetEnum(apTex->GetTarget(), 0,
								 avTexOffset.x, lTexY, avPos.x, lScreenY, avSize.x, avSize.y));
#endif
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetRenderTarget(iTexture *pTex) {
	HPL1_UNIMPLEMENTED(LowLevelGraphicsTGL::SetRenderTarget);
}

//-----------------------------------------------------------------------

bool LowLevelGraphicsTGL::RenderTargetHasZBuffer() {
	return true;
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::FlushRenderTarget() {
	HPL1_UNIMPLEMENTED(LowLevelGraphicsTGL::FlushRenderTarget);
}

//-----------------------------------------------------------------------

cVector2f LowLevelGraphicsTGL::GetScreenSize() {
	return cVector2f((float)mvScreenSize.x, (float)mvScreenSize.y);
}

//-----------------------------------------------------------------------

cVector2f LowLevelGraphicsTGL::GetVirtualSize() {
	return mvVirtualSize;
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetVirtualSize(cVector2f avSize) {
	mvVirtualSize = avSize;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetUpBatchArrays() {
	// Set the arrays
	tglVertexPointer(3, TGL_FLOAT, sizeof(float) * mlBatchStride, mpVertexArray);
	tglColorPointer(4, TGL_FLOAT, sizeof(float) * mlBatchStride, &mpVertexArray[3]);
	tglNormalPointer(TGL_FLOAT, sizeof(float) * mlBatchStride, &mpVertexArray[10]);
	tglTexCoordPointer(3, TGL_FLOAT, sizeof(float) * mlBatchStride, &mpVertexArray[7]);
#if 0
	tglClientActiveTextureARB(GL_TEXTURE1_ARB);
	tglTexCoordPointer(3, TGL_FLOAT, sizeof(float) * mlBatchStride, &mpVertexArray[7]);
	tglClientActiveTextureARB(GL_TEXTURE2_ARB);
	tglTexCoordPointer(3, TGL_FLOAT, sizeof(float) * mlBatchStride, &mpVertexArray[7]);
#endif
}

//-----------------------------------------------------------------------

void LowLevelGraphicsTGL::SetVtxBatchStates(tVtxBatchFlag flags) {
	if (flags & eVtxBatchFlag_Position) {
		tglEnableClientState(TGL_VERTEX_ARRAY);
	} else {
		tglDisableClientState(TGL_VERTEX_ARRAY);
	}

	if (flags & eVtxBatchFlag_Color0) {
		tglEnableClientState(TGL_COLOR_ARRAY);
	} else {
		tglDisableClientState(TGL_COLOR_ARRAY);
	}

	if (flags & eVtxBatchFlag_Normal) {
		tglEnableClientState(TGL_NORMAL_ARRAY);
	} else {
		tglDisableClientState(TGL_NORMAL_ARRAY);
	}

	if (flags & eVtxBatchFlag_Texture0) {
		//tglClientActiveTexture(TGL_TEXTURE);
		tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);
	} else {
		//tglClientActiveTextureARB(TGL_TEXTURE0_ARB);
		tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);
	}
#if 0
	if (flags & eVtxBatchFlag_Texture1) {
		GL_CHECK(glClientActiveTextureARB(TGL_TEXTURE1_ARB));
		tglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	} else {
		tglClientActiveTextureARB(GL_TEXTURE1_ARB);
		tglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	if (flags & eVtxBatchFlag_Texture2) {
		tglClientActiveTextureARB(GL_TEXTURE2_ARB);
		tglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	} else {
		tglClientActiveTextureARB(GL_TEXTURE2_ARB);
		tglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
#endif
}

} // namespace hpl
