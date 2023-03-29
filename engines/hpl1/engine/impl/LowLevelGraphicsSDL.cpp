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

#include "hpl1/engine/impl/LowLevelGraphicsSDL.h"
#include "hpl1/engine/graphics/font_data.h"

#include "hpl1/engine/graphics/bitmap2D.h"
#include "hpl1/engine/graphics/font_data.h"
#include "hpl1/engine/impl/CGProgram.h"
#include "hpl1/engine/impl/SDLTexture.h"
#include "hpl1/engine/impl/VertexBufferOGL.h"
#include "hpl1/engine/impl/VertexBufferVBO.h"
#include "hpl1/engine/system/low_level_system.h"

#include "common/algorithm.h"
#include "common/system.h"
#include "engines/util.h"
#include "hpl1/debug.h"
#include "hpl1/engine/impl/OcclusionQueryOGL.h"
#include "hpl1/graphics.h"
#include "hpl1/opengl.h"

#ifdef USE_OPENGL

namespace hpl {

GLenum ColorFormatToGL(eColorDataFormat format) {
	switch (format) {
	case eColorDataFormat_RGB:
		return GL_RGB;
	case eColorDataFormat_RGBA:
		return GL_RGBA;
	case eColorDataFormat_ALPHA:
		return GL_ALPHA;
	case eColorDataFormat_BGR:
		return GL_BGR;
	case eColorDataFormat_BGRA:
		return GL_BGRA;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid color format (%d)\n", format);
	return GL_RGB;
}

GLenum TextureTargetToGL(eTextureTarget target) {
	switch (target) {
	case eTextureTarget_1D:
		return GL_TEXTURE_1D;
	case eTextureTarget_2D:
		return GL_TEXTURE_2D;
	case eTextureTarget_Rect:
		return GL_TEXTURE_RECTANGLE;
	case eTextureTarget_CubeMap:
		return GL_TEXTURE_CUBE_MAP;
	case eTextureTarget_3D:
		return GL_TEXTURE_3D;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid texture target (%d)\n", target);
	return GL_TEXTURE_1D;
}

cLowLevelGraphicsSDL::cLowLevelGraphicsSDL() {
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

cLowLevelGraphicsSDL::~cLowLevelGraphicsSDL() {
	// SDL_SetGammaRamp(mvStartGammaArray[0],mvStartGammaArray[1],mvStartGammaArray[2]);

	hplFree(mpVertexArray);
	hplFree(mpIndexArray);
	for (int i = 0; i < MAX_TEXTUREUNITS; i++)
		hplFree(mpTexCoordArray[i]);
}

bool cLowLevelGraphicsSDL::Init(int alWidth, int alHeight, int alBpp, int abFullscreen,
								int alMultisampling, const tString &asWindowCaption) {
	mvScreenSize.x = alWidth;
	mvScreenSize.y = alHeight;
	mlBpp = alBpp;

	mlMultisampling = alMultisampling;
	initGraphics3d(alWidth, alHeight);
	SetupGL();
	ShowCursor(false);
	// CheckMultisampleCaps();
	g_system->updateScreen();
	return true;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::CheckMultisampleCaps() {
}

//-----------------------------------------------------------------------

static void logOGLInfo(const cLowLevelGraphicsSDL &graphics) {
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

void cLowLevelGraphicsSDL::SetupGL() {
	// Inits GL stuff
	// Set Shade model and clear color.
	GL_CHECK(glShadeModel(GL_SMOOTH));
	GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));

	// Depth Test setup
	GL_CHECK(glClearDepth(1.0f));      // VAlues buffer is cleared with
	GL_CHECK(glEnable(GL_DEPTH_TEST)); // enable depth testing
	GL_CHECK(glDepthFunc(GL_LEQUAL));  // function to do depth test with
	GL_CHECK(glDisable(GL_ALPHA_TEST));

	// Set best perspective correction
	GL_CHECK(glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST));

	// Stencil setup
	GL_CHECK(glClearStencil(0));

	// Clear the screen
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

	GL_CHECK(glMatrixMode(GL_MODELVIEW));
	GL_CHECK(glLoadIdentity());
	GL_CHECK(glMatrixMode(GL_PROJECTION));
	GL_CHECK(glLoadIdentity());

	/////  BEGIN BATCH ARRAY STUFF ///////////////

	// Enable all the vertex arrays that are used:
	GL_CHECK(glEnableClientState(GL_VERTEX_ARRAY));        // The positions
	GL_CHECK(glEnableClientState(GL_COLOR_ARRAY));         // The color
	GL_CHECK(glEnableClientState(GL_TEXTURE_COORD_ARRAY)); // Tex coords
	GL_CHECK(glDisableClientState(GL_NORMAL_ARRAY));
	// Disable the once not used.
	GL_CHECK(glDisableClientState(GL_INDEX_ARRAY)); // color index
	GL_CHECK(glDisableClientState(GL_EDGE_FLAG_ARRAY));

	///// END BATCH ARRAY STUFF ///////////////

	logOGLInfo(*this);
}
//-----------------------------------------------------------------------

int cLowLevelGraphicsSDL::GetCaps(eGraphicCaps type) const {
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
	case eGraphicCaps_MaxTextureImageUnits: {
		int lUnits;
		GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint *)&lUnits));
		return lUnits;
	}
	// Max Texture Coord Units
	case eGraphicCaps_MaxTextureCoordUnits: {
		int lUnits = 0;
		GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_COORDS, (GLint *)&lUnits));
		return lUnits;
	}
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
		return Hpl1::areShadersAvailable(); // gl 2.0

	case eGraphicCaps_GL_BlendFunctionSeparate:
		return 1; // gl 1.4

	case eGraphicCaps_GL_MultiTexture:
		return GLAD_GL_ARB_multitexture; // gl 1.4

	default:
		break;
	}
	Hpl1::logWarning(Hpl1::kDebugGraphics, "graphic options %d is not supported\n", type);
	return 0;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::ShowCursor(bool toggle) {
	g_system->showMouse(toggle);
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetMultisamplingActive(bool toggle) {
	if (!GetCaps(eGraphicCaps_Multisampling) || mlMultisampling <= 0)
		return;

	if (toggle)
		glEnable(GL_MULTISAMPLE);
	else
		glDisable(GL_MULTISAMPLE);
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetGammaCorrection(float afX) {
	mfGammaCorrection = afX;
}

float cLowLevelGraphicsSDL::GetGammaCorrection() {
	return mfGammaCorrection;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetClipPlane(int alIdx, const cPlanef &aPlane) {
	mvClipPlanes[alIdx] = aPlane;

	double vPlane[4];
	vPlane[0] = aPlane.a;
	vPlane[1] = aPlane.b;
	vPlane[2] = aPlane.c;
	vPlane[3] = aPlane.d;
	GL_CHECK(glClipPlane(GL_CLIP_PLANE0 + alIdx, vPlane));
}

cPlanef cLowLevelGraphicsSDL::GetClipPlane(int alIdx, const cPlanef &aPlane) {
	return mvClipPlanes[alIdx];
}

void cLowLevelGraphicsSDL::SetClipPlaneActive(int alIdx, bool toggle) {
	if (toggle)
		glEnable(GL_CLIP_PLANE0 + alIdx);
	else
		glDisable(GL_CLIP_PLANE0 + alIdx);
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SaveScreenToBMP(const tString &asFile) {
	GL_CHECK(glFinish());
	g_system->saveScreenshot();
}

//-----------------------------------------------------------------------

Bitmap2D *cLowLevelGraphicsSDL::CreateBitmap2D(const cVector2l &size) {
	return hplNew(Bitmap2D, (size, mpPixelFormat));
}

//-----------------------------------------------------------------------

FontData *cLowLevelGraphicsSDL::CreateFontData(const tString &asName) {
	return hplNew(FontData, (asName, this));
}

//-----------------------------------------------------------------------

iGpuProgram *cLowLevelGraphicsSDL::CreateGpuProgram(const tString &vertex, const tString &fragment) {
	return hplNew(cCGProgram, (vertex, fragment));
}

//-----------------------------------------------------------------------

Graphics::PixelFormat *cLowLevelGraphicsSDL::GetPixelFormat() {
	return &mpPixelFormat;
}

//-----------------------------------------------------------------------

iTexture *cLowLevelGraphicsSDL::CreateTexture(bool abUseMipMaps, eTextureType aType, eTextureTarget aTarget) {
	return hplNew(cSDLTexture, ("", &mpPixelFormat, this, aType, abUseMipMaps, aTarget));
}

//-----------------------------------------------------------------------

iTexture *cLowLevelGraphicsSDL::CreateTexture(const tString &asName, bool abUseMipMaps, eTextureType aType, eTextureTarget aTarget) {
	return hplNew(cSDLTexture, (asName, &mpPixelFormat, this, aType, abUseMipMaps, aTarget));
}

//-----------------------------------------------------------------------

iTexture *cLowLevelGraphicsSDL::CreateTexture(Bitmap2D *apBmp, bool abUseMipMaps, eTextureType aType,
											  eTextureTarget aTarget) {
	cSDLTexture *pTex = hplNew(cSDLTexture, ("", &mpPixelFormat, this, aType, abUseMipMaps, aTarget));
	pTex->CreateFromBitmap(apBmp);

	return pTex;
}

//-----------------------------------------------------------------------

iTexture *cLowLevelGraphicsSDL::CreateTexture(const cVector2l &avSize, int alBpp, cColor aFillCol,
											  bool abUseMipMaps, eTextureType aType, eTextureTarget aTarget) {
	cSDLTexture *pTex = NULL;

	if (aType == eTextureType_RenderTarget) {
		pTex = hplNew(cSDLTexture, ("", &mpPixelFormat, this, aType, abUseMipMaps, aTarget));
		pTex->Create(avSize.x, avSize.y, aFillCol);
	} else {
		Bitmap2D *pBmp = CreateBitmap2D(avSize);
		pBmp->fillRect(cRect2l(0, 0, 0, 0), aFillCol);

		pTex = hplNew(cSDLTexture, ("", &mpPixelFormat, this, aType, abUseMipMaps, aTarget));
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

void cLowLevelGraphicsSDL::PushMatrix(eMatrix aMtxType) {
	SetMatrixMode(aMtxType);
	GL_CHECK(glPushMatrix());
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::PopMatrix(eMatrix aMtxType) {
	SetMatrixMode(aMtxType);
	GL_CHECK(glPopMatrix());
}
//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetMatrix(eMatrix aMtxType, const cMatrixf &a_mtxA) {
	SetMatrixMode(aMtxType);
	cMatrixf mtxTranpose = a_mtxA.GetTranspose();
	GL_CHECK(glLoadMatrixf(mtxTranpose.v));
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetIdentityMatrix(eMatrix aMtxType) {
	SetMatrixMode(aMtxType);
	GL_CHECK(glLoadIdentity());
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::TranslateMatrix(eMatrix aMtxType, const cVector3f &avPos) {
	SetMatrixMode(aMtxType);
	GL_CHECK(glTranslatef(avPos.x, avPos.y, avPos.z));
}

//-----------------------------------------------------------------------

/**
 * \todo fix so that there are X, Y , Z versions of this one.
 * \param aMtxType
 * \param &avRot
 */
void cLowLevelGraphicsSDL::RotateMatrix(eMatrix aMtxType, const cVector3f &avRot) {
	SetMatrixMode(aMtxType);
	GL_CHECK(glRotatef(1, avRot.x, avRot.y, avRot.z));
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::ScaleMatrix(eMatrix aMtxType, const cVector3f &avScale) {
	SetMatrixMode(aMtxType);
	GL_CHECK(glScalef(avScale.x, avScale.y, avScale.z));
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetOrthoProjection(const cVector2f &avSize, float afMin, float afMax) {
	GL_CHECK(glMatrixMode(GL_PROJECTION));
	GL_CHECK(glLoadIdentity());
	GL_CHECK(glOrtho(0, avSize.x, avSize.y, 0, afMin, afMax));
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetTexture(unsigned int alUnit, iTexture *apTex) {
	if (apTex == mpCurrentTexture[alUnit])
		return;

	GLenum NewTarget = 0;
	if (apTex)
		NewTarget = GetGLTextureTargetEnum(apTex->GetTarget());
	GLenum LastTarget = 0;
	if (mpCurrentTexture[alUnit])
		LastTarget = GetGLTextureTargetEnum(mpCurrentTexture[alUnit]->GetTarget());

	// Check if multi texturing is supported.
	if (GetCaps(eGraphicCaps_GL_MultiTexture)) {
		GL_CHECK(glActiveTexture(GL_TEXTURE0 + alUnit));
	}

	// If the current texture in this unit is a render target, unbind it.
	if (mpCurrentTexture[alUnit] && mpCurrentTexture[alUnit]->GetTextureType() == eTextureType_RenderTarget)
		error("render target not supported");

	// Disable this unit if NULL
	if (apTex == NULL) {
		GL_CHECK(glDisable(LastTarget));
		// glBindTexture(LastTarget,0);
		// Enable the unit, set the texture handle and bind the pbuffer
	} else {
		if (NewTarget != LastTarget && LastTarget != 0)
			GL_CHECK(glDisable(LastTarget));

		cSDLTexture *pSDLTex = static_cast<cSDLTexture *>(apTex);

		GL_CHECK(glBindTexture(NewTarget, pSDLTex->GetTextureHandle()));
		GL_CHECK(glEnable(NewTarget));

		// if it is a render target we need to do some more binding.
		if (pSDLTex->GetTextureType() == eTextureType_RenderTarget) {
			error("render target not supported");
		}
	}

	mpCurrentTexture[alUnit] = apTex;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetActiveTextureUnit(unsigned int alUnit) {
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + alUnit));
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetTextureEnv(eTextureParam aParam, int alVal) {
	GLenum lParam = GetGLTextureParamEnum(aParam);

	GL_CHECK(glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE));

	if (aParam == eTextureParam_ColorFunc || aParam == eTextureParam_AlphaFunc) {
		glTexEnvi(GL_TEXTURE_ENV, lParam, GetGLTextureFuncEnum((eTextureFunc)alVal));
	} else if (aParam >= eTextureParam_ColorSource0 && aParam <= eTextureParam_AlphaSource2) {
		glTexEnvi(GL_TEXTURE_ENV, lParam, GetGLTextureSourceEnum((eTextureSource)alVal));
	} else if (aParam >= eTextureParam_ColorOp0 && aParam <= eTextureParam_AlphaOp2) {
		glTexEnvi(GL_TEXTURE_ENV, lParam, GetGLTextureOpEnum((eTextureOp)alVal));
	} else {
		glTexEnvi(GL_TEXTURE_ENV, lParam, alVal);
	}
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetTextureConstantColor(const cColor &color) {
	float vColor[] = {color.r, color.g, color.b, color.a};

	GL_CHECK(glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, &vColor[0]));
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetColor(const cColor &aColor) {
	GL_CHECK(glColor4f(aColor.r, aColor.g, aColor.b, aColor.a));
}

//-----------------------------------------------------------------------

iVertexBuffer *cLowLevelGraphicsSDL::CreateVertexBuffer(tVertexFlag aFlags,
														eVertexBufferDrawType aDrawType, eVertexBufferUsageType aUsageType, int alReserveVtxSize, int alReserveIdxSize) {

	if (GetCaps(eGraphicCaps_VertexBufferObject))
		return hplNew(cVertexBufferVBO, (this, aFlags, aDrawType, aUsageType, alReserveVtxSize, alReserveIdxSize));
	return hplNew(cVertexBufferOGL, (this, aFlags, aDrawType, aUsageType, alReserveVtxSize, alReserveIdxSize));
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawRect(const cVector2f &avPos, const cVector2f &avSize, float afZ) {
	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0, 0.0);
		glVertex3f(avPos.x, avPos.y, afZ);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(avPos.x + avSize.x, avPos.y, afZ);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(avPos.x + avSize.x, avPos.y + avSize.y, afZ);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(avPos.x, avPos.y + avSize.y, afZ);
	}
	glEnd();
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::FlushRendering() {
	GL_CHECK(glFlush());
}
void cLowLevelGraphicsSDL::SwapBuffers() {
	GL_CHECK(glFlush());
	g_system->updateScreen();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawTri(const tVertexVec &avVtx) {
	assert(avVtx.size() == 3);

	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i < 3; i++) {
			glTexCoord3f(avVtx[i].tex.x, avVtx[i].tex.y, avVtx[i].tex.z);
			glColor4f(avVtx[i].col.r, avVtx[i].col.g, avVtx[i].col.b, avVtx[i].col.a);
			glVertex3f(avVtx[i].pos.x, avVtx[i].pos.y, avVtx[i].pos.z);
		}
	}
	glEnd();
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawTri(const cVertex *avVtx) {
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i < 3; i++) {
			glTexCoord3f(avVtx[i].tex.x, avVtx[i].tex.y, avVtx[i].tex.z);
			glColor4f(avVtx[i].col.r, avVtx[i].col.g, avVtx[i].col.b, avVtx[i].col.a);
			glVertex3f(avVtx[i].pos.x, avVtx[i].pos.y, avVtx[i].pos.z);
		}
	}
	glEnd();
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawQuad(const tVertexVec &avVtx) {
	assert(avVtx.size() == 4);

	glBegin(GL_QUADS);
	{
		for (int i = 0; i < 4; i++) {
			glTexCoord3f(avVtx[i].tex.x, avVtx[i].tex.y, avVtx[i].tex.z);
			glColor4f(avVtx[i].col.r, avVtx[i].col.g, avVtx[i].col.b, avVtx[i].col.a);
			glVertex3f(avVtx[i].pos.x, avVtx[i].pos.y, avVtx[i].pos.z);
		}
	}
	glEnd();
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawQuadMultiTex(const tVertexVec &avVtx, const tVector3fVec &avExtraUvs) {
	int lExtraUnits = (int)avExtraUvs.size() / 4;
	glBegin(GL_QUADS);
	{
		for (int i = 0; i < 4; i++) {
			glMultiTexCoord3fARB(GL_TEXTURE0_ARB, avVtx[i].tex.x, avVtx[i].tex.y, avVtx[i].tex.z);

			for (int unit = 0; unit < lExtraUnits; ++unit) {
				glMultiTexCoord3fARB(GL_TEXTURE0_ARB + unit + 1,
									 avExtraUvs[unit * 4 + i].x, avExtraUvs[unit * 4 + i].y, avExtraUvs[unit * 4 + i].z);
			}

			glColor4f(avVtx[i].col.r, avVtx[i].col.g, avVtx[i].col.b, avVtx[i].col.a);
			glVertex3f(avVtx[i].pos.x, avVtx[i].pos.y, avVtx[i].pos.z);
		}
	}
	glEnd();
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

iOcclusionQuery *cLowLevelGraphicsSDL::CreateOcclusionQuery() {
	return hplNew(cOcclusionQueryOGL, ());
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DestroyOcclusionQuery(iOcclusionQuery *apQuery) {
	if (apQuery)
		hplDelete(apQuery);
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::ClearScreen() {
	GLbitfield bitmask = 0;

	if (mbClearColor)
		bitmask |= GL_COLOR_BUFFER_BIT;
	if (mbClearDepth)
		bitmask |= GL_DEPTH_BUFFER_BIT;
	if (mbClearStencil)
		bitmask |= GL_STENCIL_BUFFER_BIT;

	GL_CHECK(glClear(bitmask));
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetClearColor(const cColor &aCol) {
	GL_CHECK(glClearColor(aCol.r, aCol.g, aCol.b, aCol.a));
}
void cLowLevelGraphicsSDL::SetClearDepth(float afDepth) {
	GL_CHECK(glClearDepth(afDepth));
}
void cLowLevelGraphicsSDL::SetClearStencil(int alVal) {
	GL_CHECK(glClearStencil(alVal));
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetClearColorActive(bool abX) {
	mbClearColor = abX;
}
void cLowLevelGraphicsSDL::SetClearDepthActive(bool abX) {
	mbClearDepth = abX;
}
void cLowLevelGraphicsSDL::SetClearStencilActive(bool abX) {
	mbClearStencil = abX;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetColorWriteActive(bool abR, bool abG, bool abB, bool abA) {
	glColorMask(abR, abG, abB, abA);
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetDepthWriteActive(bool abX) {
	glDepthMask(abX);
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetDepthTestActive(bool abX) {
	if (abX)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetDepthTestFunc(eDepthTestFunc aFunc) {
	GL_CHECK(glDepthFunc(GetGLDepthTestFuncEnum(aFunc)));
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetAlphaTestActive(bool abX) {
	if (abX)
		glEnable(GL_ALPHA_TEST);
	else
		glDisable(GL_ALPHA_TEST);
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetAlphaTestFunc(eAlphaTestFunc aFunc, float afRef) {
	GL_CHECK(glAlphaFunc(GetGLAlphaTestFuncEnum(aFunc), afRef));
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetStencilActive(bool abX) {
	if (abX)
		glEnable(GL_STENCIL_TEST);
	else
		glDisable(GL_STENCIL_TEST);
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

/*void cLowLevelGraphicsSDL::SetStencilTwoSideActive(bool abX)
{
	if(GLEE_EXT_stencil_two_side)
	{
		glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
	}
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetStencilFace(eStencilFace aFace)
{
	if(GLEE_EXT_stencil_two_side)
	{
		if(aFace == eStencilFace_Front) glActiveStencilFaceEXT(GL_FRONT);
		else							glActiveStencilFaceEXT(GL_BACK);
	}
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetStencilFunc(eStencilFunc aFunc,int alRef, unsigned int aMask)
{
	glStencilFunc(GetGLStencilFuncEnum(aFunc), alRef, aMask);
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetStencilOp(eStencilOp aFailOp,eStencilOp aZFailOp,eStencilOp aZPassOp)
{
	glStencilOp(GetGLStencilOpEnum(aFailOp), GetGLStencilOpEnum(aZFailOp),
				GetGLStencilOpEnum(aZPassOp));
}*/

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetStencil(eStencilFunc aFunc, int alRef, unsigned int aMask,
									  eStencilOp aFailOp, eStencilOp aZFailOp, eStencilOp aZPassOp) {
#if 0
	if (GetCaps(eGraphicCaps_TwoSideStencil)) {
		//glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);//shouldn't be needed..
		//glActiveStencilFace(GL_FRONT);
	}
#endif
	GL_CHECK(glStencilFunc(GetGLStencilFuncEnum(aFunc), alRef, aMask));

	GL_CHECK(glStencilOp(GetGLStencilOpEnum(aFailOp), GetGLStencilOpEnum(aZFailOp),
						 GetGLStencilOpEnum(aZPassOp)));
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetStencilTwoSide(eStencilFunc aFrontFunc, eStencilFunc aBackFunc,
											 int alRef, unsigned int aMask, eStencilOp aFrontFailOp, eStencilOp aFrontZFailOp, eStencilOp aFrontZPassOp,
											 eStencilOp aBackFailOp, eStencilOp aBackZFailOp, eStencilOp aBackZPassOp) {
	if (GetCaps(eGraphicCaps_TwoSideStencil)) {
		GL_CHECK(glStencilFuncSeparate(GL_FRONT, GetGLStencilFuncEnum(aFrontFunc), alRef, aMask));
		GL_CHECK(glStencilOpSeparate(GL_FRONT, GetGLStencilOpEnum(aFrontFailOp), GetGLStencilOpEnum(aFrontZFailOp),
									 GetGLStencilOpEnum(aFrontZPassOp)))
		GL_CHECK(glStencilFuncSeparate(GL_BACK, GetGLStencilFuncEnum(aBackFunc), alRef, aMask));
		GL_CHECK(glStencilOpSeparate(GL_BACK, GetGLStencilOpEnum(aBackFailOp), GetGLStencilOpEnum(aBackZFailOp),
									 GetGLStencilOpEnum(aBackZPassOp)));
	} else
		error("Only single sided stencil supported");
}

void cLowLevelGraphicsSDL::SetStencilTwoSide(bool abX) {
	if (!GetCaps(eGraphicCaps_TwoSideStencil))
		Hpl1::logError(Hpl1::kDebugOpenGL, "call to setStencilTwoSide with two side stencil disabled%c\n", '.');
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetCullActive(bool abX) {
	if (abX)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	GL_CHECK_FN();
	GL_CHECK(glCullFace(GL_BACK));
}
void cLowLevelGraphicsSDL::SetCullMode(eCullMode aMode) {
	GL_CHECK(glCullFace(GL_BACK));
	if (aMode == eCullMode_Clockwise)
		glFrontFace(GL_CCW);
	else
		glFrontFace(GL_CW);
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetScissorActive(bool toggle) {
	if (toggle)
		glEnable(GL_SCISSOR_TEST);
	else
		glDisable(GL_SCISSOR_TEST);
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetScissorRect(const cRect2l &aRect) {
	glScissor(aRect.x, (mvScreenSize.y - aRect.y - 1) - aRect.h, aRect.w, aRect.h);
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetBlendActive(bool abX) {
	if (abX)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetBlendFunc(eBlendFunc aSrcFactor, eBlendFunc aDestFactor) {
	GL_CHECK(glBlendFunc(GetGLBlendEnum(aSrcFactor), GetGLBlendEnum(aDestFactor)));
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetBlendFuncSeparate(eBlendFunc aSrcFactorColor, eBlendFunc aDestFactorColor,
												eBlendFunc aSrcFactorAlpha, eBlendFunc aDestFactorAlpha) {
	if (GetCaps(eGraphicCaps_GL_BlendFunctionSeparate)) {

		glBlendFuncSeparate(GetGLBlendEnum(aSrcFactorColor),
							GetGLBlendEnum(aDestFactorColor),
							GetGLBlendEnum(aSrcFactorAlpha),
							GetGLBlendEnum(aDestFactorAlpha));
	} else {
		glBlendFunc(GetGLBlendEnum(aSrcFactorColor), GetGLBlendEnum(aDestFactorColor));
	}
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawQuad(const tVertexVec &avVtx, const cColor aCol) {
	assert(avVtx.size() == 4);

	glBegin(GL_QUADS);
	{
		// Make all this inline??
		for (int i = 0; i < 4; i++) {
			glTexCoord3f(avVtx[i].tex.x, avVtx[i].tex.y, avVtx[i].tex.z);
			glColor4f(aCol.r, aCol.g, aCol.b, aCol.a);
			glVertex3f(avVtx[i].pos.x, avVtx[i].pos.y, avVtx[i].pos.z);
		}
	}
	glEnd();
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawQuad(const tVertexVec &avVtx, const float afZ) {
	assert(avVtx.size() == 4);

	glBegin(GL_QUADS);
	{
		// Make all this inline??
		for (int i = 0; i < 4; i++) {
			glTexCoord3f(avVtx[i].tex.x, avVtx[i].tex.y, afZ);
			glColor4f(avVtx[i].col.r, avVtx[i].col.g, avVtx[i].col.b, avVtx[i].col.a);
			glVertex3f(avVtx[i].pos.x, avVtx[i].pos.y, avVtx[i].pos.z);
		}
	}
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawQuad(const tVertexVec &avVtx, const float afZ, const cColor &aCol) {
	assert(avVtx.size() == 4);

	glBegin(GL_QUADS);
	{
		// Make all this inline??
		for (int i = 0; i < 4; i++) {
			glTexCoord3f(avVtx[i].tex.x, avVtx[i].tex.y, afZ);
			glColor4f(aCol.r, aCol.g, aCol.b, aCol.a);
			glVertex3f(avVtx[i].pos.x, avVtx[i].pos.y, avVtx[i].pos.z);
		}
	}
	glEnd();
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::AddVertexToBatch(const cVertex &apVtx) {
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

void cLowLevelGraphicsSDL::AddVertexToBatch(const cVertex *apVtx, const cVector3f *avTransform) {
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

void cLowLevelGraphicsSDL::AddVertexToBatch(const cVertex *apVtx, const cMatrixf *aMtx) {
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::AddVertexToBatch_Size2D(const cVertex *apVtx, const cVector3f *avTransform,
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

void cLowLevelGraphicsSDL::AddVertexToBatch_Raw(const cVector3f &avPos, const cColor &aColor,
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

void cLowLevelGraphicsSDL::AddIndexToBatch(int alIndex) {
	mpIndexArray[mlIndexCount] = alIndex;
	mlIndexCount++;

	if (mlIndexCount >= mlBatchArraySize) {
		// Make the array larger.
	}
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::AddTexCoordToBatch(unsigned int alUnit, const cVector3f *apCoord) {
	unsigned int lCount = mlTexCoordArrayCount[alUnit];

	mpTexCoordArray[alUnit][lCount + 0] = apCoord->x;
	mpTexCoordArray[alUnit][lCount + 1] = apCoord->y;
	mpTexCoordArray[alUnit][lCount + 2] = apCoord->z;

	mlTexCoordArrayCount[alUnit] += 3;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetBatchTextureUnitActive(unsigned int alUnit, bool active) {
	GL_CHECK(glClientActiveTextureARB(GL_TEXTURE0_ARB + alUnit));

	if (active)
		glTexCoordPointer(3, GL_FLOAT, 0, &mpTexCoordArray[alUnit][0]);
	else
		glTexCoordPointer(3, GL_FLOAT, sizeof(float) * mlBatchStride, &mpVertexArray[7]);
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

static void flushAutoClear(unsigned &indexCount, unsigned &vertexCount, unsigned *texCoordArray) {
	indexCount = 0;
	vertexCount = 0;
	Common::fill(texCoordArray, texCoordArray + MAX_TEXTUREUNITS, 0);
}

void cLowLevelGraphicsSDL::FlushTriBatch(tVtxBatchFlag aTypeFlags, bool abAutoClear) {
	SetVtxBatchStates(aTypeFlags);
	SetUpBatchArrays();
	GL_CHECK(glDrawElements(GL_TRIANGLES, mlIndexCount, GL_UNSIGNED_INT, mpIndexArray));
	if (abAutoClear)
		flushAutoClear(mlIndexCount, mlVertexCount, mlTexCoordArrayCount);
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::FlushQuadBatch(tVtxBatchFlag aTypeFlags, bool abAutoClear) {
	SetVtxBatchStates(aTypeFlags);
	SetUpBatchArrays();
	GL_CHECK(glDrawElements(GL_QUADS, mlIndexCount, GL_UNSIGNED_INT, mpIndexArray));
	if (abAutoClear)
		flushAutoClear(mlIndexCount, mlVertexCount, mlTexCoordArrayCount);
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::ClearBatch() {
	mlIndexCount = 0;
	mlVertexCount = 0;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawLine(const cVector3f &avBegin, const cVector3f &avEnd, cColor aCol) {
	SetTexture(0, nullptr);
	// SetBlendActive(false);
	glColor4f(aCol.r, aCol.g, aCol.b, aCol.a);
	glBegin(GL_LINES);
	{
		glVertex3f(avBegin.x, avBegin.y, avBegin.z);
		glVertex3f(avEnd.x, avEnd.y, avEnd.z);
	}
	glEnd();
	GL_CHECK_FN();
}

void cLowLevelGraphicsSDL::DrawBoxMaxMin(const cVector3f &avMax, const cVector3f &avMin, cColor aCol) {
	SetTexture(0, NULL);
	SetBlendActive(false);
	glColor4f(aCol.r, aCol.g, aCol.b, aCol.a);

	glBegin(GL_LINES);
	{
		// Pos Z Quad
		glVertex3f(avMax.x, avMax.y, avMax.z);
		glVertex3f(avMin.x, avMax.y, avMax.z);

		glVertex3f(avMax.x, avMax.y, avMax.z);
		glVertex3f(avMax.x, avMin.y, avMax.z);

		glVertex3f(avMin.x, avMax.y, avMax.z);
		glVertex3f(avMin.x, avMin.y, avMax.z);

		glVertex3f(avMin.x, avMin.y, avMax.z);
		glVertex3f(avMax.x, avMin.y, avMax.z);

		// Neg Z Quad
		glVertex3f(avMax.x, avMax.y, avMin.z);
		glVertex3f(avMin.x, avMax.y, avMin.z);

		glVertex3f(avMax.x, avMax.y, avMin.z);
		glVertex3f(avMax.x, avMin.y, avMin.z);

		glVertex3f(avMin.x, avMax.y, avMin.z);
		glVertex3f(avMin.x, avMin.y, avMin.z);

		glVertex3f(avMin.x, avMin.y, avMin.z);
		glVertex3f(avMax.x, avMin.y, avMin.z);

		// Lines between
		glVertex3f(avMax.x, avMax.y, avMax.z);
		glVertex3f(avMax.x, avMax.y, avMin.z);

		glVertex3f(avMin.x, avMax.y, avMax.z);
		glVertex3f(avMin.x, avMax.y, avMin.z);

		glVertex3f(avMin.x, avMin.y, avMax.z);
		glVertex3f(avMin.x, avMin.y, avMin.z);

		glVertex3f(avMax.x, avMin.y, avMax.z);
		glVertex3f(avMax.x, avMin.y, avMin.z);
	}
	glEnd();
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawSphere(const cVector3f &avPos, float afRadius, cColor aCol) {
	int alSegments = 32;
	float afAngleStep = k2Pif / (float)alSegments;

	SetTexture(0, nullptr);
	SetBlendActive(false);
	glColor4f(aCol.r, aCol.g, aCol.b, aCol.a);
	glBegin(GL_LINES);
	{
		// X Circle:
		for (float a = 0; a < k2Pif; a += afAngleStep) {
			glVertex3f(avPos.x, avPos.y + sin(a) * afRadius,
					   avPos.z + cos(a) * afRadius);

			glVertex3f(avPos.x, avPos.y + sin(a + afAngleStep) * afRadius,
					   avPos.z + cos(a + afAngleStep) * afRadius);
		}

		// Y Circle:
		for (float a = 0; a < k2Pif; a += afAngleStep) {
			glVertex3f(avPos.x + cos(a) * afRadius, avPos.y,
					   avPos.z + sin(a) * afRadius);

			glVertex3f(avPos.x + cos(a + afAngleStep) * afRadius, avPos.y,
					   avPos.z + sin(a + afAngleStep) * afRadius);
		}

		// Z Circle:
		for (float a = 0; a < k2Pif; a += afAngleStep) {
			glVertex3f(avPos.x + cos(a) * afRadius, avPos.y + sin(a) * afRadius, avPos.z);

			glVertex3f(avPos.x + cos(a + afAngleStep) * afRadius,
					   avPos.y + sin(a + afAngleStep) * afRadius,
					   avPos.z);
		}
	}
	glEnd();
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawLine2D(const cVector2f &avBegin, const cVector2f &avEnd, float afZ, cColor aCol) {
	SetTexture(0, NULL);
	SetBlendActive(false);
	glColor4f(aCol.r, aCol.g, aCol.b, aCol.a);
	glBegin(GL_LINES);
	{
		glVertex3f(avBegin.x, avBegin.y, afZ);
		glVertex3f(avEnd.x, avEnd.y, afZ);
	}
	glEnd();
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawLineRect2D(const cRect2f &aRect, float afZ, cColor aCol) {
	SetTexture(0, nullptr);
	SetBlendActive(false);
	glColor4f(aCol.r, aCol.g, aCol.b, aCol.a);
	glBegin(GL_LINE_STRIP);
	{
		glVertex3f(aRect.x, aRect.y, afZ);
		glVertex3f(aRect.x + aRect.w, aRect.y, afZ);
		glVertex3f(aRect.x + aRect.w, aRect.y + aRect.h, afZ);
		glVertex3f(aRect.x, aRect.y + aRect.h, afZ);
		glVertex3f(aRect.x, aRect.y, afZ);
	}
	glEnd();
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawFilledRect2D(const cRect2f &aRect, float afZ, cColor aCol) {
	SetTexture(0, NULL);
	glColor4f(aCol.r, aCol.g, aCol.b, aCol.a);
	glBegin(GL_QUADS);
	{
		glVertex3f(aRect.x, aRect.y, afZ);
		glVertex3f(aRect.x + aRect.w, aRect.y, afZ);
		glVertex3f(aRect.x + aRect.w, aRect.y + aRect.h, afZ);
		glVertex3f(aRect.x, aRect.y + aRect.h, afZ);
	}
	glEnd();
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawLineCircle2D(const cVector2f &avCenter, float afRadius, float afZ, cColor aCol) {
	// Implement later
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::CopyContextToTexure(iTexture *apTex, const cVector2l &avPos,
											   const cVector2l &avSize, const cVector2l &avTexOffset) {
	if (apTex == nullptr)
		return;

	int lScreenY = (mvScreenSize.y - avSize.y) - avPos.y;
	int lTexY = (apTex->getHeight() - avSize.y) - avTexOffset.y;

	// Log("TExoffset: %d %d\n",avTexOffset.x,lTexY);
	// Log("ScreenOffset: %d %d (h: %d s: %d p: %d)\n",avPos.x,lScreenY,mvScreenSize.y,
	//												avSize.y,avPos.y);

	SetTexture(0, apTex);
	GL_CHECK(glCopyTexSubImage2D(GetGLTextureTargetEnum(apTex->GetTarget()), 0,
								 avTexOffset.x, lTexY, avPos.x, lScreenY, avSize.x, avSize.y));
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetRenderTarget(iTexture *pTex) {
#if 0
  	if(pTex == mpRenderTarget)return;
		mpRenderTarget = pTex;

		if(pTex==NULL)
		{
#ifdef WIN32
			if (!wglMakeCurrent(mDeviceContext, mGLContext)){
				Log("Something went wrong...");
			}
#elif defined(__linux__)
		/*if (!glXMakeCurrent(dpy, gPBuffer, glCtx)) {
				Log("Something went wrong...");
			}*/
#endif
		}
		else
		{
			if(pTex->GetTextureType() != eTextureType_RenderTarget)return;

			cSDLTexture* pSDLTex = static_cast<cSDLTexture*>(pTex);
			cPBuffer* pPBuffer = pSDLTex->GetPBuffer();

			//pPBuffer->UnBind();//needed?

			if (!pPBuffer->MakeCurrentContext()){
				Log("PBuffer::Activate() failed.\n");
			}
		}


		//Old OGL 1.1 Code:
		/*FlushRenderTarget();

		mpRenderTarget = pTex;

		if(mpRenderTarget==NULL)
			glViewport(0,0,mvScreenSize.x,mvScreenSize.y);
		else
			glViewport(0,0,pTex->GetWidth(),pTex->GetHeight());*/
#endif
}

//-----------------------------------------------------------------------

bool cLowLevelGraphicsSDL::RenderTargetHasZBuffer() {
	return true;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::FlushRenderTarget() {

	// Old OGL 1.1 Code:
	/*if(mpRenderTarget!=NULL)
	{
		SetTexture(0, mpRenderTarget);

		//Log("w: %d\n",mpRenderTarget->GetWidth());

		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0,
						mpRenderTarget->GetWidth(), mpRenderTarget->GetHeight(), 0);
	}*/
}

//-----------------------------------------------------------------------

cVector2f cLowLevelGraphicsSDL::GetScreenSize() {
	return cVector2f((float)mvScreenSize.x, (float)mvScreenSize.y);
}

//-----------------------------------------------------------------------

cVector2f cLowLevelGraphicsSDL::GetVirtualSize() {
	return mvVirtualSize;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetVirtualSize(cVector2f avSize) {
	mvVirtualSize = avSize;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetUpBatchArrays() {
	// Set the arrays
	glVertexPointer(3, GL_FLOAT, sizeof(float) * mlBatchStride, mpVertexArray);
	glColorPointer(4, GL_FLOAT, sizeof(float) * mlBatchStride, &mpVertexArray[3]);
	glNormalPointer(GL_FLOAT, sizeof(float) * mlBatchStride, &mpVertexArray[10]);

	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glTexCoordPointer(3, GL_FLOAT, sizeof(float) * mlBatchStride, &mpVertexArray[7]);
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glTexCoordPointer(3, GL_FLOAT, sizeof(float) * mlBatchStride, &mpVertexArray[7]);
	glClientActiveTextureARB(GL_TEXTURE2_ARB);
	glTexCoordPointer(3, GL_FLOAT, sizeof(float) * mlBatchStride, &mpVertexArray[7]);
	GL_CHECK_FN();
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetVtxBatchStates(tVtxBatchFlag flags) {
	if (flags & eVtxBatchFlag_Position) {
		GL_CHECK(glEnableClientState(GL_VERTEX_ARRAY));
	} else {
		GL_CHECK(glDisableClientState(GL_VERTEX_ARRAY));
	}

	if (flags & eVtxBatchFlag_Color0) {
		GL_CHECK(glEnableClientState(GL_COLOR_ARRAY));
	} else {
		GL_CHECK(glDisableClientState(GL_COLOR_ARRAY));
	}

	if (flags & eVtxBatchFlag_Normal) {
		GL_CHECK(glEnableClientState(GL_NORMAL_ARRAY));
	} else {
		GL_CHECK(glDisableClientState(GL_NORMAL_ARRAY));
	}

	if (flags & eVtxBatchFlag_Texture0) {
		GL_CHECK(glClientActiveTextureARB(GL_TEXTURE0_ARB));
		GL_CHECK(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
	} else {
		GL_CHECK(glClientActiveTextureARB(GL_TEXTURE0_ARB));
		GL_CHECK(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
	}

	if (flags & eVtxBatchFlag_Texture1) {
		GL_CHECK(glClientActiveTextureARB(GL_TEXTURE1_ARB));
		GL_CHECK(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
	} else {
		GL_CHECK(glClientActiveTextureARB(GL_TEXTURE1_ARB));
		GL_CHECK(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
	}

	if (flags & eVtxBatchFlag_Texture2) {
		GL_CHECK(glClientActiveTextureARB(GL_TEXTURE2_ARB));
		GL_CHECK(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
	} else {
		GL_CHECK(glClientActiveTextureARB(GL_TEXTURE2_ARB));
		GL_CHECK(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
	}
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLBlendEnum(eBlendFunc type) {
	switch (type) {
	case eBlendFunc_Zero:
		return GL_ZERO;
	case eBlendFunc_One:
		return GL_ONE;
	case eBlendFunc_SrcColor:
		return GL_SRC_COLOR;
	case eBlendFunc_OneMinusSrcColor:
		return GL_ONE_MINUS_SRC_COLOR;
	case eBlendFunc_DestColor:
		return GL_DST_COLOR;
	case eBlendFunc_OneMinusDestColor:
		return GL_ONE_MINUS_DST_COLOR;
	case eBlendFunc_SrcAlpha:
		return GL_SRC_ALPHA;
	case eBlendFunc_OneMinusSrcAlpha:
		return GL_ONE_MINUS_SRC_ALPHA;
	case eBlendFunc_DestAlpha:
		return GL_DST_ALPHA;
	case eBlendFunc_OneMinusDestAlpha:
		return GL_ONE_MINUS_DST_ALPHA;
	case eBlendFunc_SrcAlphaSaturate:
		return GL_SRC_ALPHA_SATURATE;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid blend op (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLTextureParamEnum(eTextureParam type) {
	switch (type) {
	case eTextureParam_ColorFunc:
		return GL_COMBINE_RGB;
	case eTextureParam_AlphaFunc:
		return GL_COMBINE_ALPHA;
	case eTextureParam_ColorSource0:
		return GL_SOURCE0_RGB;
	case eTextureParam_ColorSource1:
		return GL_SOURCE1_RGB;
	case eTextureParam_ColorSource2:
		return GL_SOURCE2_RGB;
	case eTextureParam_AlphaSource0:
		return GL_SOURCE0_ALPHA;
	case eTextureParam_AlphaSource1:
		return GL_SOURCE1_ALPHA;
	case eTextureParam_AlphaSource2:
		return GL_SOURCE2_ALPHA;
	case eTextureParam_ColorOp0:
		return GL_OPERAND0_RGB;
	case eTextureParam_ColorOp1:
		return GL_OPERAND1_RGB;
	case eTextureParam_ColorOp2:
		return GL_OPERAND2_RGB;
	case eTextureParam_AlphaOp0:
		return GL_OPERAND0_ALPHA;
	case eTextureParam_AlphaOp1:
		return GL_OPERAND1_ALPHA;
	case eTextureParam_AlphaOp2:
		return GL_OPERAND2_ALPHA;
	case eTextureParam_ColorScale:
		return GL_RGB_SCALE;
	case eTextureParam_AlphaScale:
		return GL_ALPHA_SCALE;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid texture parameter (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLTextureOpEnum(eTextureOp type) {
	switch (type) {
	case eTextureOp_Color:
		return GL_SRC_COLOR;
	case eTextureOp_OneMinusColor:
		return GL_ONE_MINUS_SRC_COLOR;
	case eTextureOp_Alpha:
		return GL_SRC_ALPHA;
	case eTextureOp_OneMinusAlpha:
		return GL_ONE_MINUS_SRC_ALPHA;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid texture op (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLTextureSourceEnum(eTextureSource type) {
	switch (type) {
	case eTextureSource_Texture:
		return GL_TEXTURE;
	case eTextureSource_Constant:
		return GL_CONSTANT;
	case eTextureSource_Primary:
		return GL_PRIMARY_COLOR;
	case eTextureSource_Previous:
		return GL_PREVIOUS;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid texture source (%d)", type);
	return 0;
}
//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLTextureTargetEnum(eTextureTarget type) {
	switch (type) {
	case eTextureTarget_1D:
		return GL_TEXTURE_1D;
	case eTextureTarget_2D:
		return GL_TEXTURE_2D;
	case eTextureTarget_Rect:
		return GL_TEXTURE_RECTANGLE;
	case eTextureTarget_CubeMap:
		return GL_TEXTURE_CUBE_MAP;
	case eTextureTarget_3D:
		return GL_TEXTURE_3D;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid texture target (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLTextureFuncEnum(eTextureFunc type) {
	switch (type) {
	case eTextureFunc_Modulate:
		return GL_MODULATE;
	case eTextureFunc_Replace:
		return GL_REPLACE;
	case eTextureFunc_Add:
		return GL_ADD;
	case eTextureFunc_Substract:
		return GL_SUBTRACT;
	case eTextureFunc_AddSigned:
		return GL_ADD_SIGNED;
	case eTextureFunc_Interpolate:
		return GL_INTERPOLATE;
	case eTextureFunc_Dot3RGB:
		return GL_DOT3_RGB;
	case eTextureFunc_Dot3RGBA:
		return GL_DOT3_RGBA;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid texture function (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------
GLenum cLowLevelGraphicsSDL::GetGLDepthTestFuncEnum(eDepthTestFunc type) {
	switch (type) {
	case eDepthTestFunc_Never:
		return GL_NEVER;
	case eDepthTestFunc_Less:
		return GL_LESS;
	case eDepthTestFunc_LessOrEqual:
		return GL_LEQUAL;
	case eDepthTestFunc_Greater:
		return GL_GREATER;
	case eDepthTestFunc_GreaterOrEqual:
		return GL_GEQUAL;
	case eDepthTestFunc_Equal:
		return GL_EQUAL;
	case eDepthTestFunc_NotEqual:
		return GL_NOTEQUAL;
	case eDepthTestFunc_Always:
		return GL_ALWAYS;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid depth test function (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLAlphaTestFuncEnum(eAlphaTestFunc type) {
	switch (type) {
	case eAlphaTestFunc_Never:
		return GL_NEVER;
	case eAlphaTestFunc_Less:
		return GL_LESS;
	case eAlphaTestFunc_LessOrEqual:
		return GL_LEQUAL;
	case eAlphaTestFunc_Greater:
		return GL_GREATER;
	case eAlphaTestFunc_GreaterOrEqual:
		return GL_GEQUAL;
	case eAlphaTestFunc_Equal:
		return GL_EQUAL;
	case eAlphaTestFunc_NotEqual:
		return GL_NOTEQUAL;
	case eAlphaTestFunc_Always:
		return GL_ALWAYS;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid alpha test function (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLStencilFuncEnum(eStencilFunc type) {
	switch (type) {
	case eStencilFunc_Never:
		return GL_NEVER;
	case eStencilFunc_Less:
		return GL_LESS;
	case eStencilFunc_LessOrEqual:
		return GL_LEQUAL;
	case eStencilFunc_Greater:
		return GL_GREATER;
	case eStencilFunc_GreaterOrEqual:
		return GL_GEQUAL;
	case eStencilFunc_Equal:
		return GL_EQUAL;
	case eStencilFunc_NotEqual:
		return GL_NOTEQUAL;
	case eStencilFunc_Always:
		return GL_ALWAYS;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid stencil function (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLStencilOpEnum(eStencilOp type) {
	switch (type) {
	case eStencilOp_Keep:
		return GL_KEEP;
	case eStencilOp_Zero:
		return GL_ZERO;
	case eStencilOp_Replace:
		return GL_REPLACE;
	case eStencilOp_Increment:
		return GL_INCR;
	case eStencilOp_Decrement:
		return GL_DECR;
	case eStencilOp_Invert:
		return GL_INVERT;
	case eStencilOp_IncrementWrap:
		return GL_INCR_WRAP;
	case eStencilOp_DecrementWrap:
		return GL_DECR_WRAP;
	default:
		break;
	}
	Hpl1::logError(Hpl1::kDebugOpenGL, "invalid stencil op (%d)", type);
	return 0;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetMatrixMode(eMatrix type) {
	switch (type) {
	case eMatrix_ModelView:
		GL_CHECK(glMatrixMode(GL_MODELVIEW));
		break;
	case eMatrix_Projection:
		GL_CHECK(glMatrixMode(GL_PROJECTION));
		break;
	case eMatrix_Texture:
		GL_CHECK(glMatrixMode(GL_TEXTURE));
		break;
	default:
		Hpl1::logError(Hpl1::kDebugOpenGL, "invalid matrix mode (%d)", type);
	}
}

//-----------------------------------------------------------------------

} // namespace hpl

#endif // USE_OPENGL
