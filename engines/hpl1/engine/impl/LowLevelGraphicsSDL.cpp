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

#if 0 // def WIN32
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "GLu32.lib")
#pragma comment(lib, "GLaux.lib")
#pragma comment(lib, "Cg.lib")
#pragma comment(lib, "CgGL.lib")
#pragma comment(lib, "SDL_ttf.lib")
#endif

#include <assert.h>
#include <stdlib.h>

#include "hpl1/engine/graphics/FontData.h"
#include "hpl1/engine/impl/LowLevelGraphicsSDL.h"

//#include "graphics/opengl/glad.h"
#include "hpl1/engine/impl/CGProgram.h"
#include "hpl1/engine/impl/SDLBitmap2D.h"
#include "hpl1/engine/impl/SDLFontData.h"
#include "hpl1/engine/impl/SDLTexture.h"
#include "hpl1/engine/impl/VertexBufferOGL.h"
#include "hpl1/engine/impl/VertexBufferVBO.h"
#include "hpl1/engine/system/LowLevelSystem.h"

#include "hpl1/engine/impl/OcclusionQueryOGL.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

GLenum ColorFormatToGL(eColorDataFormat aFormat) {
#if 0
  		switch(aFormat)
		{
		case eColorDataFormat_RGB:		return GL_RGB;
		case eColorDataFormat_RGBA:		return GL_RGBA;
		case eColorDataFormat_ALPHA:	return GL_ALPHA;
		case eColorDataFormat_BGR:		return GL_BGR_EXT;
		case eColorDataFormat_BGRA:		return GL_BGRA_EXT;
		};

#endif
	return 0;
}

//-------------------------------------------------

GLenum TextureTargetToGL(eTextureTarget aTarget) {
#if 0
  		switch(aTarget)
		{
		case eTextureTarget_1D:		return GL_TEXTURE_1D;
		case eTextureTarget_2D:		return GL_TEXTURE_2D;
		case eTextureTarget_Rect:	return GL_TEXTURE_RECTANGLE_NV;
		case eTextureTarget_CubeMap:	return GL_TEXTURE_CUBE_MAP_ARB;
		case eTextureTarget_3D:		return GL_TEXTURE_3D;
		}
#endif
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLowLevelGraphicsSDL::cLowLevelGraphicsSDL() {
#if 0
  		mlBatchArraySize = 20000;
		mlVertexCount = 0;
		mlIndexCount =0;
		mlMultisampling =0;

		mvVirtualSize.x = 800;
		mvVirtualSize.y = 600;

#ifdef WIN32
			mhKeyTrapper = NULL;
#endif

		mpPixelFormat = hplNew(cSDLPixelFormat, () );

		mpRenderTarget=NULL;

		for(int i=0;i<MAX_TEXTUREUNITS;i++)
			mpCurrentTexture[i] = NULL;

		mbClearColor = true;
		mbClearDepth = true;
		mbClearStencil = false;

		//Create the batch arrays:
		mlBatchStride = 13;
		//3 Pos floats, 4 color floats, 3 Tex coord floats .
		mpVertexArray = (float*)hplMalloc(sizeof(float) * mlBatchStride * mlBatchArraySize);
		mpIndexArray = (unsigned int*)hplMalloc(sizeof(unsigned int) * mlBatchArraySize); //Index is one int.

		for(int i=0;i<MAX_TEXTUREUNITS;i++)
		{
			mpTexCoordArray[i] = (float*)hplMalloc(sizeof(float) * 3 * mlBatchArraySize);
			mbTexCoordArrayActive[i] = false;
			mlTexCoordArrayCount[i]=0;
		}

		//Init extra stuff
		InitCG();

		TTF_Init();
#endif
}

//-----------------------------------------------------------------------

cLowLevelGraphicsSDL::~cLowLevelGraphicsSDL() {
#if 0
  	//#ifdef WIN32
		//	if(mhKeyTrapper) FreeLibrary(mhKeyTrapper);
		//#endif


		SDL_SetGammaRamp(mvStartGammaArray[0],mvStartGammaArray[1],mvStartGammaArray[2]);

		hplFree(mpVertexArray);
		hplFree(mpIndexArray);
		for(int i=0;i<MAX_TEXTUREUNITS;i++)	hplFree(mpTexCoordArray[i]);

		hplDelete(mpPixelFormat);

		//Exit extra stuff
		ExitCG();
		TTF_Quit();
#endif
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cLowLevelGraphicsSDL::Init(int alWidth, int alHeight, int alBpp, int abFullscreen,
								int alMultisampling, const tString &asWindowCaption) {
#if 0
  		mvScreenSize.x = alWidth;
		mvScreenSize.y = alHeight;
		mlBpp = alBpp;

		mlMultisampling = alMultisampling;

		//Set some GL Attributes
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		// Multisampling
		if(mlMultisampling > 0)
		{
			if(SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1)==-1)
			{
				Error("Multisample buffers not supported!\n");
			}
			else
			{
				if(SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, mlMultisampling)==-1)
				{
					Error("Couldn't set multisampling samples to %d\n",mlMultisampling);
				}
			}
		}

		unsigned int mlFlags = SDL_OPENGL;

		if(abFullscreen) mlFlags |= SDL_FULLSCREEN;

		Log(" Setting video mode: %d x %d - %d bpp\n",alWidth, alHeight, alBpp);
		mpScreen = SDL_SetVideoMode( alWidth, alHeight, alBpp, mlFlags);
		if(mpScreen==NULL){
			Error("Could not set display mode setting a lower one!\n");
			mvScreenSize = cVector2l(640,480);
			mpScreen = SDL_SetVideoMode( mvScreenSize.x, mvScreenSize.y, alBpp, mlFlags);
			if(mpScreen==NULL)
			{
				FatalError("Unable to initialize display!\n");
				return false;
			}
			else
			{
				SetWindowCaption(asWindowCaption);
				CreateMessageBoxW(_W("Warning!"),
									_W("Could not set displaymode and 640x480 is used instead!\n"));
			}
		}
		else
		{
			SetWindowCaption(asWindowCaption);
		}

		Log(" Init Glee...");
		if(GLeeInit())
		{
			Log("OK\n");
		}
		else
		{
			Log("ERROR!\n");
			Error(" Couldn't init glee!\n");
		}

		///Setup up windows specifc context:
#if defined(WIN32)
			mGLContext = wglGetCurrentContext();
			mDeviceContext = wglGetCurrentDC();
#elif defined(__linux__)
		/*gDpy = XOpenDisplay(NULL);
		glCtx = gPBuffer = 0;*/
#endif

		//Check Multisample properties
		CheckMultisampleCaps();

		//Turn off cursor as default
		ShowCursor(false);

		//Gamma
		mfGammaCorrection = 1.0f;
		SDL_GetGammaRamp(mvStartGammaArray[0],mvStartGammaArray[1],mvStartGammaArray[2]);

		SDL_SetGamma(mfGammaCorrection,mfGammaCorrection,mfGammaCorrection);

		//GL
		Log(" Setting up OpenGL\n");
		SetupGL();

		//Set the clear color
		SDL_GL_SwapBuffers();

		return true;
#endif
	return false;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::CheckMultisampleCaps() {
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetupGL() {
#if 0
  		//Inits GL stuff
		//Set Shade model and clear color.
		glShadeModel(GL_SMOOTH);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		//Depth Test setup
		glClearDepth(1.0f);//VAlues buffer is cleared with
		glEnable(GL_DEPTH_TEST); //enable depth testing
		glDepthFunc(GL_LEQUAL); //function to do depth test with
		glDisable(GL_ALPHA_TEST);

		//Set best perspective correction
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		//int lStencilBits=-1;
		//glGetIntegerv(GL_STENCIL_BITS,&lStencilBits);
		//Log(" Stencil bits: %d\n",lStencilBits);

		//Stencil setup
		glClearStencil(0);

		//Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);

		/////  BEGIN BATCH ARRAY STUFF ///////////////

		//Enable all the vertex arrays that are used:
		glEnableClientState(GL_VERTEX_ARRAY ); //The positions
		glEnableClientState(GL_COLOR_ARRAY ); //The color
		glEnableClientState(GL_TEXTURE_COORD_ARRAY); //Tex coords
		glDisableClientState(GL_NORMAL_ARRAY);
		//Disable the once not used.
		glDisableClientState(GL_INDEX_ARRAY); //color index
		glDisableClientState(GL_EDGE_FLAG_ARRAY);

		///// END BATCH ARRAY STUFF ///////////////

		//Show some info
		Log("  Max texture image units: %d\n",GetCaps(eGraphicCaps_MaxTextureImageUnits));
		Log("  Max texture coord units: %d\n",GetCaps(eGraphicCaps_MaxTextureCoordUnits));
		Log("  Two sided stencil: %d\n",GetCaps(eGraphicCaps_TwoSideStencil));
		Log("  Vertex Buffer Object: %d\n",GetCaps(eGraphicCaps_VertexBufferObject));

		Log("  Anisotropic filtering: %d\n",GetCaps(eGraphicCaps_AnisotropicFiltering));
		if(GetCaps(eGraphicCaps_AnisotropicFiltering))
			Log("  Max Anisotropic degree: %d\n",GetCaps(eGraphicCaps_MaxAnisotropicFiltering));

		Log("  Multisampling: %d\n",GetCaps(eGraphicCaps_Multisampling));

		Log("  Vertex Program: %d\n",GetCaps(eGraphicCaps_GL_VertexProgram));
		Log("  Fragment Program: %d\n",GetCaps(eGraphicCaps_GL_FragmentProgram));

		Log("  NV Register Combiners: %d\n",GetCaps(eGraphicCaps_GL_NVRegisterCombiners));
		Log("  NV Register Combiners Stages: %d\n",GetCaps(eGraphicCaps_GL_NVRegisterCombiners_MaxStages));

		Log("  ATI Fragment Shader: %d\n",GetCaps(eGraphicCaps_GL_ATIFragmentShader));
#endif
}
//-----------------------------------------------------------------------

int cLowLevelGraphicsSDL::GetCaps(eGraphicCaps aType) {
#if 0
  		switch(aType)
		{
		//Texture Rectangle
		case eGraphicCaps_TextureTargetRectangle:
			{
				return 1;//GLEE_ARB_texture_rectangle?1:0;
			}


		//Vertex Buffer Object
		case eGraphicCaps_VertexBufferObject:
			{
				return GLEE_ARB_vertex_buffer_object?1:0;
			}

		//Two Sided Stencil
		case eGraphicCaps_TwoSideStencil:
			{
				//DEBUG:
				//return 0;

				if(GLEE_EXT_stencil_two_side) return 1;
				else if(GLEE_ATI_separate_stencil) return 1;
				else return 0;
			}

		//Max Texture Image Units
		case eGraphicCaps_MaxTextureImageUnits:
			{
				//DEBUG:
				//return 2;

				int lUnits;
				glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB,(GLint *)&lUnits);
				return lUnits;
			}

		//Max Texture Coord Units
		case eGraphicCaps_MaxTextureCoordUnits:
			{
				int lUnits;
				glGetIntegerv(GL_MAX_TEXTURE_COORDS_ARB,(GLint *)&lUnits);
				return lUnits;
			}
		//Texture Anisotropy
		case eGraphicCaps_AnisotropicFiltering:
			{
				if(GLEE_EXT_texture_filter_anisotropic) return 1;
				else return 0;
			}

		//Texture Anisotropy
		case eGraphicCaps_MaxAnisotropicFiltering:
			{
				if(!GLEE_EXT_texture_filter_anisotropic) return 0;

				float fMax;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,&fMax);
				return (int)fMax;
			}

		//Multisampling
		case eGraphicCaps_Multisampling:
			{
				if(GLEE_ARB_multisample) return 1;
				return 0;
			}


		//GL Vertex program
		case eGraphicCaps_GL_VertexProgram:
			{
				//Debbug:
				//return 0;

				if(GLEE_ARB_vertex_program) return 1;
				else return 0;
			}

		//GL Fragment program
		case eGraphicCaps_GL_FragmentProgram:
			{
				//Debbug:
				//return 0;

				if(GLEE_ARB_fragment_program) return 1;
				else return 0;
			}

		//GL NV register combiners
		case eGraphicCaps_GL_NVRegisterCombiners:
			{
				if(GLEE_NV_register_combiners) return 1;
				else return 0;
			}

		//GL NV register combiners Max stages
		case eGraphicCaps_GL_NVRegisterCombiners_MaxStages:
			{
				int lStages;
				glGetIntegerv(GL_MAX_GENERAL_COMBINERS_NV,(GLint *)&lStages);
				return lStages;
			}

		//GL ATI Fragment Shader
		case eGraphicCaps_GL_ATIFragmentShader:
			{
				if(GLEE_ATI_fragment_shader) return 1;
				else return 0;
			}
		}

#endif
	return 0;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::ShowCursor(bool abX) {
#if 0
  		if(abX)
			SDL_ShowCursor(SDL_ENABLE);
		else
			SDL_ShowCursor(SDL_DISABLE);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetVsyncActive(bool abX) {
#if 0
#if defined(WIN32)
		if(GLEE_WGL_EXT_swap_control)
		{
			wglSwapIntervalEXT(abX ? 1 : 0);
		}
#elif defined(__linux__)
		if (GLEE_GLX_SGI_swap_control)
		{
			glXSwapIntervalSGI(abX ? 1 : 0);
		}
#endif
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetMultisamplingActive(bool abX) {
#if 0
  		if(!GLEE_ARB_multisample || mlMultisampling<=0) return;

		if(abX)
			glEnable(GL_MULTISAMPLE_ARB);
		else
			glDisable(GL_MULTISAMPLE_ARB);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetGammaCorrection(float afX) {
#if 0
  		//if(mfGammaCorrection == afX) return;

		mfGammaCorrection = afX;

		SDL_SetGamma(mfGammaCorrection,mfGammaCorrection,mfGammaCorrection);

		/*Uint16 GammaArray[3][256];

		for (int iIndex = 0; iIndex < 256; iIndex++)
		{
			Uint16 iArrayValue = iIndex * ((int)(afX*127.0f) + 128);

			if (iArrayValue > 65535)
				iArrayValue = 65535;

			GammaArray[0][iIndex] =
			GammaArray[1][iIndex] =
			GammaArray[2][iIndex] = iArrayValue;

		}

		//Set the GammaArray values into the display device context.
		int bReturn = SDL_SetGammaRamp(GammaArray[0],GammaArray[1],GammaArray[2]);*/
		/*if(bReturn!=-1) Log("Setting gamma worked!\n");
		else		Log("Setting gamma FAILED!\n");*/
#endif
}

float cLowLevelGraphicsSDL::GetGammaCorrection() {
	return mfGammaCorrection;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetClipPlane(int alIdx, const cPlanef &aPlane) {
#if 0
  		mvClipPlanes[alIdx] = aPlane;

		double vPlane[4];
		vPlane[0] = aPlane.a;
		vPlane[1] = aPlane.b;
		vPlane[2] = aPlane.c;
		vPlane[3] = aPlane.d;
		glClipPlane(GL_CLIP_PLANE0 + alIdx,vPlane);
#endif
}
cPlanef cLowLevelGraphicsSDL::GetClipPlane(int alIdx, const cPlanef &aPlane) {
	return mvClipPlanes[alIdx];
}
void cLowLevelGraphicsSDL::SetClipPlaneActive(int alIdx, bool abX) {
#if 0
  		if(abX) glEnable(GL_CLIP_PLANE0 + alIdx);
		else	glDisable(GL_CLIP_PLANE0 + alIdx);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SaveScreenToBMP(const tString &asFile) {
#if 0
  		glFinish();

		cSDLBitmap2D *pBmp = hplNew( cSDLBitmap2D, (mpPixelFormat) );
		pBmp->Create(cVector2l(mvScreenSize.x,mvScreenSize.y),32);

		unsigned char *pDestPixels = (unsigned char*)pBmp->GetSurface()->pixels;
		unsigned char *pSrcPixels = (unsigned char*)hplMalloc(mvScreenSize.x * mvScreenSize.y * 4);

		SDL_LockSurface(pBmp->GetSurface());
		glReadBuffer(GL_BACK);
		glReadPixels(0,0,mvScreenSize.x,mvScreenSize.y,GL_RGBA,GL_UNSIGNED_BYTE,pSrcPixels);

		for(int y=0; y<mvScreenSize.y; ++y)
		{
			for(int x=0; x<mvScreenSize.x; ++x)
			{
				unsigned char* pDestPix = &pDestPixels[((mvScreenSize.x * y) + x) * 4];
				unsigned char* pSrcPix = &pSrcPixels[((mvScreenSize.x * ((mvScreenSize.y-1) - y))
														+ x) * 4];

				pDestPix[0] = pSrcPix[0];
				pDestPix[1] = pSrcPix[1];
				pDestPix[2] = pSrcPix[2];
				pDestPix[3] = 255;
			}
		}

		SDL_UnlockSurface(pBmp->GetSurface());
		SDL_SaveBMP(pBmp->GetSurface(),asFile.c_str());

		hplFree(pSrcPixels);
		hplDelete(pBmp);
#endif
}

//-----------------------------------------------------------------------

iBitmap2D *cLowLevelGraphicsSDL::CreateBitmap2D(const cVector2l &avSize, unsigned int alBpp) {
	cSDLBitmap2D *pBmp = hplNew(cSDLBitmap2D, (mpPixelFormat));
	pBmp->Create(avSize, alBpp);

	return pBmp;
}

//-----------------------------------------------------------------------

iFontData *cLowLevelGraphicsSDL::CreateFontData(const tString &asName) {
	return hplNew(cSDLFontData, (asName, this));
}

//-----------------------------------------------------------------------

iBitmap2D *cLowLevelGraphicsSDL::CreateBitmap2DFromSurface(SDL_Surface *apSurface, const tString &asType) {
	cSDLBitmap2D *pBmp = hplNew(cSDLBitmap2D, (apSurface, mpPixelFormat, asType));

	pBmp->msType = asType;

	return pBmp;
}

//-----------------------------------------------------------------------

iGpuProgram *cLowLevelGraphicsSDL::CreateGpuProgram(const tString &asName, eGpuProgramType aType) {
	return hplNew(cCGProgram, (asName, mCG_Context, aType));
}

//-----------------------------------------------------------------------

Graphics::PixelFormat *cLowLevelGraphicsSDL::GetPixelFormat() {
	return mpPixelFormat;
}

//-----------------------------------------------------------------------

iTexture *cLowLevelGraphicsSDL::CreateTexture(bool abUseMipMaps, eTextureType aType, eTextureTarget aTarget) {
	return hplNew(cSDLTexture, ("", mpPixelFormat, this, aType, abUseMipMaps, aTarget));
}

//-----------------------------------------------------------------------

iTexture *cLowLevelGraphicsSDL::CreateTexture(const tString &asName, bool abUseMipMaps, eTextureType aType, eTextureTarget aTarget) {
	return hplNew(cSDLTexture, (asName, mpPixelFormat, this, aType, abUseMipMaps, aTarget));
}

//-----------------------------------------------------------------------

iTexture *cLowLevelGraphicsSDL::CreateTexture(iBitmap2D *apBmp, bool abUseMipMaps, eTextureType aType,
											  eTextureTarget aTarget) {
	cSDLTexture *pTex = hplNew(cSDLTexture, ("", mpPixelFormat, this, aType, abUseMipMaps, aTarget));
	pTex->CreateFromBitmap(apBmp);

	return pTex;
}

//-----------------------------------------------------------------------

iTexture *cLowLevelGraphicsSDL::CreateTexture(const cVector2l &avSize, int alBpp, cColor aFillCol,
											  bool abUseMipMaps, eTextureType aType, eTextureTarget aTarget) {
	cSDLTexture *pTex = NULL;

	if (aType == eTextureType_RenderTarget) {
		pTex = hplNew(cSDLTexture, ("", mpPixelFormat, this, aType, abUseMipMaps, aTarget));
		pTex->Create(avSize.x, avSize.y, aFillCol);
	} else {
		iBitmap2D *pBmp = CreateBitmap2D(avSize, alBpp);
		pBmp->FillRect(cRect2l(0, 0, 0, 0), aFillCol);

		pTex = hplNew(cSDLTexture, ("", mpPixelFormat, this, aType, abUseMipMaps, aTarget));
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
#if 0
  		SetMatrixMode(aMtxType);
		glPushMatrix();
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::PopMatrix(eMatrix aMtxType) {
#if 0
  		SetMatrixMode(aMtxType);
		glPopMatrix();
#endif
}
//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetMatrix(eMatrix aMtxType, const cMatrixf &a_mtxA) {
#if 0
  		SetMatrixMode(aMtxType);
		cMatrixf mtxTranpose = a_mtxA.GetTranspose();
		glLoadMatrixf(mtxTranpose.v);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetIdentityMatrix(eMatrix aMtxType) {
#if 0
  		SetMatrixMode(aMtxType);
		glLoadIdentity();
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::TranslateMatrix(eMatrix aMtxType, const cVector3f &avPos) {
#if 0
  		SetMatrixMode(aMtxType);
		glTranslatef(avPos.x,avPos.y,avPos.z);
#endif
}

//-----------------------------------------------------------------------

/**
 * \todo fix so that there are X, Y , Z versions of this one.
 * \param aMtxType
 * \param &avRot
 */
void cLowLevelGraphicsSDL::RotateMatrix(eMatrix aMtxType, const cVector3f &avRot) {
#if 0
  		SetMatrixMode(aMtxType);
		glRotatef(1,avRot.x,avRot.y,avRot.z);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::ScaleMatrix(eMatrix aMtxType, const cVector3f &avScale) {
#if 0
  		SetMatrixMode(aMtxType);
		glScalef(avScale.x,avScale.y,avScale.z);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetOrthoProjection(const cVector2f &avSize, float afMin, float afMax) {
#if 0
  		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,avSize.x,avSize.y,0,afMin,afMax);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetTexture(unsigned int alUnit, iTexture *apTex) {
#if 0
  		if(apTex == mpCurrentTexture[alUnit]) return;

		GLenum NewTarget=0;
		if(apTex)
			NewTarget = GetGLTextureTargetEnum(apTex->GetTarget());
		GLenum LastTarget=0;
		if(mpCurrentTexture[alUnit])
			LastTarget = GetGLTextureTargetEnum(mpCurrentTexture[alUnit]->GetTarget());

		//Check if multi texturing is supported.
		if(GLEE_ARB_multitexture){
			glActiveTextureARB(GL_TEXTURE0_ARB + alUnit);
		}

		//If the current texture in this unit is a render target, unbind it.
		if(mpCurrentTexture[alUnit]){
			if(mpCurrentTexture[alUnit]->GetTextureType() == eTextureType_RenderTarget)
			{
				cSDLTexture *pSDLTex = static_cast<cSDLTexture *> (mpCurrentTexture[alUnit]);

				glBindTexture(LastTarget, pSDLTex->GetTextureHandle());
				cPBuffer* pBuffer = pSDLTex->GetPBuffer();
				pBuffer->UnBind();
			}
		}

		//Disable this unit if NULL
		if(apTex == NULL)
		{
			glDisable(LastTarget);
			//glBindTexture(LastTarget,0);
		}
		//Enable the unit, set the texture handle and bind the pbuffer
		else
		{
			if(NewTarget != LastTarget) glDisable(LastTarget);

			cSDLTexture *pSDLTex = static_cast<cSDLTexture*> (apTex);

			glBindTexture(NewTarget, pSDLTex->GetTextureHandle());
			glEnable(NewTarget);

			//if it is a render target we need to do some more binding.
			if(pSDLTex->GetTextureType() == eTextureType_RenderTarget)
			{
				cPBuffer* pBuffer = pSDLTex->GetPBuffer();
				pBuffer->Bind();
			}
		}

		mpCurrentTexture[alUnit] = apTex;
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetActiveTextureUnit(unsigned int alUnit) {
#if 0
  		glActiveTextureARB(GL_TEXTURE0_ARB + alUnit);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetTextureEnv(eTextureParam aParam, int alVal) {
#if 0
  		GLenum lParam = GetGLTextureParamEnum(aParam);

		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE_ARB);

		if(aParam==eTextureParam_ColorFunc || aParam==eTextureParam_AlphaFunc){
			glTexEnvi(GL_TEXTURE_ENV,lParam,GetGLTextureFuncEnum((eTextureFunc)alVal));
		}
		else if(aParam>=eTextureParam_ColorSource0 && aParam<=eTextureParam_AlphaSource2){
			glTexEnvi(GL_TEXTURE_ENV,lParam,GetGLTextureSourceEnum((eTextureSource)alVal));
		}
		else if(aParam>=eTextureParam_ColorOp0 && aParam<=eTextureParam_AlphaOp2){
			glTexEnvi(GL_TEXTURE_ENV,lParam,GetGLTextureOpEnum((eTextureOp)alVal));
		}
		else {
			glTexEnvi(GL_TEXTURE_ENV,lParam,alVal);
		}
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetTextureConstantColor(const cColor &aColor) {
#if 0
  		float vColor[4] = {	aColor.r, aColor.g, aColor.b, aColor.a	};

		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, &vColor[0]);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetColor(const cColor &aColor) {
#if 0
  		glColor4f(aColor.r, aColor.g, aColor.b, aColor.a);
#endif
}

//-----------------------------------------------------------------------

iVertexBuffer *cLowLevelGraphicsSDL::CreateVertexBuffer(tVertexFlag aFlags,
														eVertexBufferDrawType aDrawType,
														eVertexBufferUsageType aUsageType,
														int alReserveVtxSize, int alReserveIdxSize) {
	// return hplNew( cVertexBufferVBO,(this, aFlags,aDrawType,aUsageType,alReserveVtxSize,alReserveIdxSize) );
	// return hplNew( cVertexBufferOGL, (this, aFlags,aDrawType,aUsageType,alReserveVtxSize,alReserveIdxSize) );

	if (GetCaps(eGraphicCaps_VertexBufferObject)) {
		return hplNew(cVertexBufferVBO, (this, aFlags, aDrawType, aUsageType, alReserveVtxSize, alReserveIdxSize));
	} else {
		// Error("VBO is not supported, using Vertex array!\n");
		return hplNew(cVertexBufferOGL, (this, aFlags, aDrawType, aUsageType, alReserveVtxSize, alReserveIdxSize));
	}
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawRect(const cVector2f &avPos, const cVector2f &avSize, float afZ) {
#if 0
  		glColor4f(1,1,1,1);
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0.0, 0.0); glVertex3f(avPos.x, avPos.y,afZ);
			glTexCoord2f(1.0, 0.0); glVertex3f(avPos.x+avSize.x, avPos.y,afZ);
			glTexCoord2f(1.0, 1.0); glVertex3f(avPos.x+avSize.x, avPos.y+avSize.y,afZ);
			glTexCoord2f(0.0, 1.0); glVertex3f(avPos.x, avPos.y+avSize.y,afZ);
		}
		glEnd();
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::FlushRendering() {
#if 0
  		glFlush();
#endif
}
void cLowLevelGraphicsSDL::SwapBuffers() {
#if 0
  		glFlush();
		SDL_GL_SwapBuffers();
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawTri(const tVertexVec &avVtx) {
#if 0
  		assert(avVtx.size()==3);

		glBegin(GL_TRIANGLES);
		{
			for(int i=0;i<3;i++){
				glTexCoord3f(avVtx[i].tex.x,avVtx[i].tex.y,avVtx[i].tex.z);
				glColor4f(avVtx[i].col.r,avVtx[i].col.g,avVtx[i].col.b,avVtx[i].col.a);
				glVertex3f(avVtx[i].pos.x,avVtx[i].pos.y,avVtx[i].pos.z);
			}
		}
		glEnd();
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawTri(const cVertex *avVtx) {
#if 0
  		glBegin(GL_TRIANGLES);
		{
			for(int i=0;i<3;i++){
				glTexCoord3f(avVtx[i].tex.x,avVtx[i].tex.y,avVtx[i].tex.z);
				glColor4f(avVtx[i].col.r,avVtx[i].col.g,avVtx[i].col.b,avVtx[i].col.a);
				glVertex3f(avVtx[i].pos.x,avVtx[i].pos.y,avVtx[i].pos.z);
			}
		}
		glEnd();
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawQuad(const tVertexVec &avVtx) {
#if 0
  		assert(avVtx.size()==4);

		glBegin(GL_QUADS);
		{
			for(int i=0;i<4;i++){
				glTexCoord3f(avVtx[i].tex.x,avVtx[i].tex.y,avVtx[i].tex.z);
				glColor4f(avVtx[i].col.r,avVtx[i].col.g,avVtx[i].col.b,avVtx[i].col.a);
				glVertex3f(avVtx[i].pos.x,avVtx[i].pos.y,avVtx[i].pos.z);
			}
		}
		glEnd();
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawQuadMultiTex(const tVertexVec &avVtx, const tVector3fVec &avExtraUvs) {
#if 0
  		int lExtraUnits = (int)avExtraUvs.size()/4;
		glBegin(GL_QUADS);
		{
			for(int i=0;i<4;i++)
			{
				glMultiTexCoord3fARB(GL_TEXTURE0_ARB,avVtx[i].tex.x,avVtx[i].tex.y,avVtx[i].tex.z);

				for(int unit=0; unit<lExtraUnits; ++unit)
				{
					glMultiTexCoord3fARB(GL_TEXTURE0_ARB + unit + 1,
										avExtraUvs[unit*4 + i].x, avExtraUvs[unit*4 + i].y, avExtraUvs[unit*4 + i].z);
				}

				glColor4f(avVtx[i].col.r,avVtx[i].col.g,avVtx[i].col.b,avVtx[i].col.a);
				glVertex3f(avVtx[i].pos.x,avVtx[i].pos.y,avVtx[i].pos.z);
			}
		}
		glEnd();
#endif
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
#if 0
  		GLbitfield bitmask=0;

		if(mbClearColor)bitmask |= GL_COLOR_BUFFER_BIT;
		if(mbClearDepth)bitmask |= GL_DEPTH_BUFFER_BIT;
		if(mbClearStencil)bitmask |= GL_STENCIL_BUFFER_BIT;

		glClear(bitmask);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetClearColor(const cColor &aCol) {
#if 0
  		glClearColor(aCol.r, aCol.g, aCol.b, aCol.a);
#endif
}
void cLowLevelGraphicsSDL::SetClearDepth(float afDepth) {
#if 0
  		glClearDepth(afDepth);
#endif
}
void cLowLevelGraphicsSDL::SetClearStencil(int alVal) {
#if 0
  		glClearStencil(alVal);
#endif
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
#if 0
  		glColorMask(abR,abG,abB,abA);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetDepthWriteActive(bool abX) {
#if 0
  		glDepthMask(abX);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetDepthTestActive(bool abX) {
#if 0
  		if(abX) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetDepthTestFunc(eDepthTestFunc aFunc) {
#if 0
  		glDepthFunc(GetGLDepthTestFuncEnum(aFunc));
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetAlphaTestActive(bool abX) {
#if 0
  		if(abX) glEnable(GL_ALPHA_TEST);
		else glDisable(GL_ALPHA_TEST);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetAlphaTestFunc(eAlphaTestFunc aFunc, float afRef) {
#if 0
  		glAlphaFunc(GetGLAlphaTestFuncEnum(aFunc),afRef);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetStencilActive(bool abX) {
#if 0
  		if(abX) glEnable(GL_STENCIL_TEST);
		else glDisable(GL_STENCIL_TEST);
#endif
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
  		if(GLEE_EXT_stencil_two_side)
		{
			//glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);//shouldn't be needed..
			glActiveStencilFaceEXT(GL_FRONT);
		}
		glStencilFunc(GetGLStencilFuncEnum(aFunc), alRef, aMask);

		glStencilOp(GetGLStencilOpEnum(aFailOp), GetGLStencilOpEnum(aZFailOp),
					GetGLStencilOpEnum(aZPassOp));
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetStencilTwoSide(eStencilFunc aFrontFunc, eStencilFunc aBackFunc,
											 int alRef, unsigned int aMask,
											 eStencilOp aFrontFailOp, eStencilOp aFrontZFailOp, eStencilOp aFrontZPassOp,
											 eStencilOp aBackFailOp, eStencilOp aBackZFailOp, eStencilOp aBackZPassOp) {
#if 0
  		//Nvidia implementation
		if(GLEE_EXT_stencil_two_side)
		{
			glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);

			//Front
			glActiveStencilFaceEXT(GL_FRONT);
			glStencilFunc(GetGLStencilFuncEnum(aFrontFunc), alRef, aMask);

			glStencilOp(GetGLStencilOpEnum(aFrontFailOp), GetGLStencilOpEnum(aFrontZFailOp),
						GetGLStencilOpEnum(aFrontZPassOp));
			//Back
			glActiveStencilFaceEXT(GL_BACK);
			glStencilFunc(GetGLStencilFuncEnum(aBackFunc), alRef, aMask);

			glStencilOp(GetGLStencilOpEnum(aBackFailOp), GetGLStencilOpEnum(aBackZFailOp),
						GetGLStencilOpEnum(aBackZPassOp));
		}
		//Ati implementation
		else if(GLEE_ATI_separate_stencil)
		{
			//Front
			glStencilOpSeparateATI( GL_FRONT, GetGLStencilOpEnum(aFrontFailOp),
								GetGLStencilOpEnum(aFrontZFailOp),
								GetGLStencilOpEnum(aFrontZPassOp));
			//Back
			glStencilOpSeparateATI( GL_BACK, GetGLStencilOpEnum(aBackFailOp),
								GetGLStencilOpEnum(aBackZFailOp),
								GetGLStencilOpEnum(aBackZPassOp));

			//Front and Back function
			glStencilFuncSeparateATI(GetGLStencilFuncEnum(aFrontFunc),
									GetGLStencilFuncEnum(aBackFunc),
									alRef, aMask);
		}
		else
		{
			FatalError("Only single sided stencil supported!\n");
		}
#endif
}

void cLowLevelGraphicsSDL::SetStencilTwoSide(bool abX) {
#if 0
  		if(GLEE_EXT_stencil_two_side)
		{
			glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
		}
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetCullActive(bool abX) {
#if 0
  		if(abX) glEnable(GL_CULL_FACE);
		else glDisable(GL_CULL_FACE);
		glCullFace(GL_BACK);
#endif
}
void cLowLevelGraphicsSDL::SetCullMode(eCullMode aMode) {
#if 0
  		glCullFace(GL_BACK);
		if(aMode == eCullMode_Clockwise) glFrontFace(GL_CCW);
		else
		glFrontFace(GL_CW);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetScissorActive(bool abX) {
#if 0
  		if(abX) glEnable(GL_SCISSOR_TEST);
		else glDisable(GL_SCISSOR_TEST);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetScissorRect(const cRect2l &aRect) {
#if 0
  		glScissor(aRect.x, (mvScreenSize.y - aRect.y - 1)-aRect.h, aRect.w, aRect.h);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetBlendActive(bool abX) {
#if 0
  		if(abX)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetBlendFunc(eBlendFunc aSrcFactor, eBlendFunc aDestFactor) {
#if 0
  		glBlendFunc(GetGLBlendEnum(aSrcFactor),GetGLBlendEnum(aDestFactor));
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetBlendFuncSeparate(eBlendFunc aSrcFactorColor, eBlendFunc aDestFactorColor,
												eBlendFunc aSrcFactorAlpha, eBlendFunc aDestFactorAlpha) {
#if 0
  		if(GLEE_EXT_blend_func_separate)
		{
			glBlendFuncSeparateEXT(GetGLBlendEnum(aSrcFactorColor),
								GetGLBlendEnum(aDestFactorColor),
								GetGLBlendEnum(aSrcFactorAlpha),
								GetGLBlendEnum(aDestFactorAlpha));
		}
		else
		{
			glBlendFunc(GetGLBlendEnum(aSrcFactorColor),GetGLBlendEnum(aDestFactorColor));
		}
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawQuad(const tVertexVec &avVtx, const cColor aCol) {
#if 0
  		assert(avVtx.size()==4);

		glBegin(GL_QUADS);
		{
			//Make all this inline??
			for(int i=0;i<4;i++){
				glTexCoord3f(avVtx[i].tex.x,avVtx[i].tex.y,avVtx[i].tex.z);
				glColor4f(aCol.r,aCol.g,aCol.b,aCol.a);
				glVertex3f(avVtx[i].pos.x,avVtx[i].pos.y,avVtx[i].pos.z);
			}
		}
		glEnd();
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawQuad(const tVertexVec &avVtx, const float afZ) {
#if 0
  		assert(avVtx.size()==4);

		glBegin(GL_QUADS);
		{
			//Make all this inline??
			for(int i=0;i<4;i++){
				glTexCoord3f(avVtx[i].tex.x,avVtx[i].tex.y,afZ);
				glColor4f(avVtx[i].col.r,avVtx[i].col.g,avVtx[i].col.b,avVtx[i].col.a);
				glVertex3f(avVtx[i].pos.x,avVtx[i].pos.y,avVtx[i].pos.z);
			}
		}
		glEnd();
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawQuad(const tVertexVec &avVtx, const float afZ, const cColor &aCol) {
#if 0
  		assert(avVtx.size()==4);

		glBegin(GL_QUADS);
		{
			//Make all this inline??
			for(int i=0;i<4;i++){
				glTexCoord3f(avVtx[i].tex.x,avVtx[i].tex.y,afZ);
				glColor4f(aCol.r,aCol.g,aCol.b,aCol.a);
				glVertex3f(avVtx[i].pos.x,avVtx[i].pos.y,avVtx[i].pos.z);
			}
		}
		glEnd();
#endif
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

void cLowLevelGraphicsSDL::SetBatchTextureUnitActive(unsigned int alUnit, bool abActive) {
#if 0
  		glClientActiveTextureARB(GL_TEXTURE0_ARB+alUnit);

		if(abActive==false){
			glTexCoordPointer(3,GL_FLOAT,sizeof(float)*mlBatchStride, &mpVertexArray[7]);
		}
		else {
			glTexCoordPointer(3,GL_FLOAT,0, &mpTexCoordArray[alUnit][0]);
		}
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::FlushTriBatch(tVtxBatchFlag aTypeFlags, bool abAutoClear) {
#if 0
  		SetVtxBatchStates(aTypeFlags);
		SetUpBatchArrays();

		glDrawElements(GL_TRIANGLES,mlIndexCount,GL_UNSIGNED_INT, mpIndexArray);

		if(abAutoClear){
			mlIndexCount = 0;
			mlVertexCount = 0;
			for(int i=0;i<MAX_TEXTUREUNITS;i++)
				mlTexCoordArrayCount[i]=0;
		}
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::FlushQuadBatch(tVtxBatchFlag aTypeFlags, bool abAutoClear) {
#if 0
  		SetVtxBatchStates(aTypeFlags);
		SetUpBatchArrays();

		glDrawElements(GL_QUADS,mlIndexCount,GL_UNSIGNED_INT, mpIndexArray);

		if(abAutoClear){
			mlIndexCount = 0;
			mlVertexCount = 0;
			for(int i=0;i<MAX_TEXTUREUNITS;i++)
				mlTexCoordArrayCount[i]=0;
		}
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::ClearBatch() {
	mlIndexCount = 0;
	mlVertexCount = 0;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawLine(const cVector3f &avBegin, const cVector3f &avEnd, cColor aCol) {
#if 0
  		SetTexture(0,NULL);
		//SetBlendActive(false);
		glColor4f(aCol.r,aCol.g,aCol.b,aCol.a);
		glBegin(GL_LINES);
		{
			glVertex3f(avBegin.x,avBegin.y,avBegin.z);
			glVertex3f(avEnd.x,avEnd.y,avEnd.z);
		}
		glEnd();
#endif
}

void cLowLevelGraphicsSDL::DrawBoxMaxMin(const cVector3f &avMax, const cVector3f &avMin, cColor aCol) {
#if 0
  		SetTexture(0,NULL);
		SetBlendActive(false);
		glColor4f(aCol.r,aCol.g,aCol.b,aCol.a);

		glBegin(GL_LINES);
		{
			//Pos Z Quad
			glVertex3f(avMax.x,avMax.y,avMax.z);
			glVertex3f(avMin.x,avMax.y,avMax.z);

			glVertex3f(avMax.x,avMax.y,avMax.z);
			glVertex3f(avMax.x,avMin.y,avMax.z);

			glVertex3f(avMin.x,avMax.y,avMax.z);
			glVertex3f(avMin.x,avMin.y,avMax.z);

			glVertex3f(avMin.x,avMin.y,avMax.z);
			glVertex3f(avMax.x,avMin.y,avMax.z);

			//Neg Z Quad
			glVertex3f(avMax.x,avMax.y,avMin.z);
			glVertex3f(avMin.x,avMax.y,avMin.z);

			glVertex3f(avMax.x,avMax.y,avMin.z);
			glVertex3f(avMax.x,avMin.y,avMin.z);

			glVertex3f(avMin.x,avMax.y,avMin.z);
			glVertex3f(avMin.x,avMin.y,avMin.z);

			glVertex3f(avMin.x,avMin.y,avMin.z);
			glVertex3f(avMax.x,avMin.y,avMin.z);

			//Lines between
			glVertex3f(avMax.x,avMax.y,avMax.z);
			glVertex3f(avMax.x,avMax.y,avMin.z);

			glVertex3f(avMin.x,avMax.y,avMax.z);
			glVertex3f(avMin.x,avMax.y,avMin.z);

			glVertex3f(avMin.x,avMin.y,avMax.z);
			glVertex3f(avMin.x,avMin.y,avMin.z);

			glVertex3f(avMax.x,avMin.y,avMax.z);
			glVertex3f(avMax.x,avMin.y,avMin.z);
		}
		glEnd();
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawSphere(const cVector3f &avPos, float afRadius, cColor aCol) {
#if 0
  		int alSegments = 32;
		float afAngleStep = k2Pif /(float)alSegments;

		SetTexture(0,NULL);
		SetBlendActive(false);
		glColor4f(aCol.r,aCol.g,aCol.b,aCol.a);
		glBegin(GL_LINES);
		{
			//X Circle:
			for(float a=0; a< k2Pif; a+= afAngleStep)
			{
				glVertex3f(avPos.x, avPos.y + sin(a)*afRadius,
									avPos.z + cos(a)*afRadius);

				glVertex3f(avPos.x, avPos.y + sin(a+afAngleStep)*afRadius,
									avPos.z + cos(a+afAngleStep)*afRadius);
			}

			//Y Circle:
			for(float a=0; a< k2Pif; a+= afAngleStep)
			{
				glVertex3f(avPos.x + cos(a)*afRadius, avPos.y,
									avPos.z + sin(a)*afRadius);

				glVertex3f(avPos.x + cos(a+afAngleStep)*afRadius, avPos.y ,
							avPos.z+ sin(a+afAngleStep)*afRadius);
			}

			//Z Circle:
			for(float a=0; a< k2Pif; a+= afAngleStep)
			{
				glVertex3f(avPos.x + cos(a)*afRadius, avPos.y + sin(a)*afRadius, avPos.z);

				glVertex3f(avPos.x + cos(a+afAngleStep)*afRadius,
							avPos.y + sin(a+afAngleStep)*afRadius,
							avPos.z);
			}

		}
		glEnd();
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawLine2D(const cVector2f &avBegin, const cVector2f &avEnd, float afZ, cColor aCol) {
#if 0
  		SetTexture(0,NULL);
		SetBlendActive(false);
		glColor4f(aCol.r,aCol.g,aCol.b,aCol.a);
		glBegin(GL_LINES);
		{
			glVertex3f(avBegin.x,avBegin.y,afZ);
			glVertex3f(avEnd.x,avEnd.y,afZ);
		}
		glEnd();
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawLineRect2D(const cRect2f &aRect, float afZ, cColor aCol) {
#if 0
  		SetTexture(0, NULL);
		SetBlendActive(false);
		glColor4f(aCol.r,aCol.g,aCol.b,aCol.a);
		glBegin(GL_LINE_STRIP);
		{
			glVertex3f(aRect.x,aRect.y,afZ);
			glVertex3f(aRect.x+aRect.w,aRect.y,afZ);
			glVertex3f(aRect.x+aRect.w,aRect.y+aRect.h,afZ);
			glVertex3f(aRect.x,aRect.y+aRect.h,afZ);
			glVertex3f(aRect.x,aRect.y,afZ);
		}
		glEnd();
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::DrawFilledRect2D(const cRect2f &aRect, float afZ, cColor aCol) {
#if 0
  		SetTexture(0, NULL);
		glColor4f(aCol.r,aCol.g,aCol.b,aCol.a);
		glBegin(GL_QUADS);
		{
			glVertex3f(aRect.x,aRect.y,afZ);
			glVertex3f(aRect.x+aRect.w,aRect.y,afZ);
			glVertex3f(aRect.x+aRect.w,aRect.y+aRect.h,afZ);
			glVertex3f(aRect.x,aRect.y+aRect.h,afZ);
		}
		glEnd();
#endif
}

//-----------------------------------------------------------------------

/**
 * \todo Implement this.
 * \param avCenter
 * \param afRadius
 * \param afZ
 * \param aCol
 */
void cLowLevelGraphicsSDL::DrawLineCircle2D(const cVector2f &avCenter, float afRadius, float afZ, cColor aCol) {
	// Implement later
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::CopyContextToTexure(iTexture *apTex, const cVector2l &avPos,
											   const cVector2l &avSize, const cVector2l &avTexOffset) {
#if 0
  		if(apTex==NULL)return;

		int lScreenY = (mvScreenSize.y - avSize.y) - avPos.y;
		int lTexY = (apTex->GetHeight() - avSize.y) - avTexOffset.y;

		//Log("TExoffset: %d %d\n",avTexOffset.x,lTexY);
		//Log("ScreenOffset: %d %d (h: %d s: %d p: %d)\n",avPos.x,lScreenY,mvScreenSize.y,
		//												avSize.y,avPos.y);

		SetTexture(0, apTex);
		glCopyTexSubImage2D(GetGLTextureTargetEnum(apTex->GetTarget()),0,
							avTexOffset.x, lTexY,
							avPos.x, lScreenY, avSize.x, avSize.y);
#endif
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
#if 0
  		//Set the arrays
		glVertexPointer(3,GL_FLOAT, sizeof(float)*mlBatchStride, mpVertexArray);
		glColorPointer(4,GL_FLOAT,sizeof(float)*mlBatchStride, &mpVertexArray[3]);
		glNormalPointer(GL_FLOAT,sizeof(float)*mlBatchStride, &mpVertexArray[10]);

		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		glTexCoordPointer(3,GL_FLOAT,sizeof(float)*mlBatchStride, &mpVertexArray[7]);
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glTexCoordPointer(3,GL_FLOAT,sizeof(float)*mlBatchStride, &mpVertexArray[7]);
		glClientActiveTextureARB(GL_TEXTURE2_ARB);
		glTexCoordPointer(3,GL_FLOAT,sizeof(float)*mlBatchStride, &mpVertexArray[7]);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetVtxBatchStates(tVtxBatchFlag aFlags) {
#if 0
  		if(aFlags & eVtxBatchFlag_Position)	glEnableClientState(GL_VERTEX_ARRAY );
		else glDisableClientState(GL_VERTEX_ARRAY );

		if(aFlags & eVtxBatchFlag_Color0) glEnableClientState(GL_COLOR_ARRAY );
		else glDisableClientState(GL_COLOR_ARRAY );

		if(aFlags & eVtxBatchFlag_Normal) glEnableClientState(GL_NORMAL_ARRAY );
		else glDisableClientState(GL_NORMAL_ARRAY );


		if(aFlags & eVtxBatchFlag_Texture0){
			glClientActiveTextureARB(GL_TEXTURE0_ARB);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY );
		}
		else {
			glClientActiveTextureARB(GL_TEXTURE0_ARB);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY );
		}

		if(aFlags & eVtxBatchFlag_Texture1){
			glClientActiveTextureARB(GL_TEXTURE1_ARB);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY );
		}
		else {
			glClientActiveTextureARB(GL_TEXTURE1_ARB);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY );
		}

		if(aFlags & eVtxBatchFlag_Texture2){
			glClientActiveTextureARB(GL_TEXTURE2_ARB);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY );
		}
		else {
			glClientActiveTextureARB(GL_TEXTURE2_ARB);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY );
		}

#endif
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLBlendEnum(eBlendFunc aType) {
#if 0
  		switch(aType)
		{
		case eBlendFunc_Zero:					return GL_ZERO;
		case eBlendFunc_One:					return GL_ONE;
		case eBlendFunc_SrcColor:				return GL_SRC_COLOR;
		case eBlendFunc_OneMinusSrcColor:		return GL_ONE_MINUS_SRC_COLOR;
		case eBlendFunc_DestColor:				return GL_DST_COLOR;
		case eBlendFunc_OneMinusDestColor:		return GL_ONE_MINUS_DST_COLOR;
		case eBlendFunc_SrcAlpha:				return GL_SRC_ALPHA;
		case eBlendFunc_OneMinusSrcAlpha:		return GL_ONE_MINUS_SRC_ALPHA;
		case eBlendFunc_DestAlpha:				return GL_DST_ALPHA;
		case eBlendFunc_OneMinusDestAlpha:		return GL_ONE_MINUS_DST_ALPHA;
		case eBlendFunc_SrcAlphaSaturate:		return GL_SRC_ALPHA_SATURATE;
		}
#endif
	return 0;
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLTextureParamEnum(eTextureParam aType) {
#if 0
  		switch(aType)
		{
		case eTextureParam_ColorFunc:		return GL_COMBINE_RGB_ARB;
		case eTextureParam_AlphaFunc:		return GL_COMBINE_ALPHA_ARB;
		case eTextureParam_ColorSource0:	return GL_SOURCE0_RGB_ARB;
		case eTextureParam_ColorSource1:	return GL_SOURCE1_RGB_ARB;
		case eTextureParam_ColorSource2:	return GL_SOURCE2_RGB_ARB;
		case eTextureParam_AlphaSource0:	return GL_SOURCE0_ALPHA_ARB;
		case eTextureParam_AlphaSource1:	return GL_SOURCE1_ALPHA_ARB;
		case eTextureParam_AlphaSource2:	return GL_SOURCE2_ALPHA_ARB;
		case eTextureParam_ColorOp0:		return GL_OPERAND0_RGB_ARB;
		case eTextureParam_ColorOp1:		return GL_OPERAND1_RGB_ARB;
		case eTextureParam_ColorOp2:		return GL_OPERAND2_RGB_ARB;
		case eTextureParam_AlphaOp0:		return GL_OPERAND0_ALPHA_ARB;
		case eTextureParam_AlphaOp1:		return GL_OPERAND1_ALPHA_ARB;
		case eTextureParam_AlphaOp2:		return GL_OPERAND2_ALPHA_ARB;
		case eTextureParam_ColorScale:		return GL_RGB_SCALE_ARB;
		case eTextureParam_AlphaScale:		return GL_ALPHA_SCALE;
		}
#endif
	return 0;
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLTextureOpEnum(eTextureOp aType) {
#if 0
  		switch(aType)
		{
		case eTextureOp_Color:			return GL_SRC_COLOR;
		case eTextureOp_OneMinusColor:	return GL_ONE_MINUS_SRC_COLOR;
		case eTextureOp_Alpha:			return GL_SRC_ALPHA;
		case eTextureOp_OneMinusAlpha:	return GL_ONE_MINUS_SRC_ALPHA;
	}
#endif
	return 0;
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLTextureSourceEnum(eTextureSource aType) {
#if 0
  		switch(aType)
		{
		case eTextureSource_Texture:	return GL_TEXTURE;
		case eTextureSource_Constant:	return GL_CONSTANT_ARB;
		case eTextureSource_Primary:	return GL_PRIMARY_COLOR_ARB;
		case eTextureSource_Previous:	return GL_PREVIOUS_ARB;
		}
#endif
	return 0;
}
//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLTextureTargetEnum(eTextureTarget aType) {
#if 0
  		switch(aType)
		{
		case eTextureTarget_1D:		return GL_TEXTURE_1D;
		case eTextureTarget_2D:		return GL_TEXTURE_2D;
		case eTextureTarget_Rect:
			{
				return GL_TEXTURE_RECTANGLE_NV;
			}
		case eTextureTarget_CubeMap:	return GL_TEXTURE_CUBE_MAP_ARB;
		case eTextureTarget_3D:		return GL_TEXTURE_3D;
		}
#endif
	return 0;
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLTextureFuncEnum(eTextureFunc aType) {
#if 0
  		switch(aType)
		{
		case eTextureFunc_Modulate:		return GL_MODULATE;
		case eTextureFunc_Replace:		return GL_REPLACE;
		case eTextureFunc_Add:			return GL_ADD;
		case eTextureFunc_Substract:	return GL_SUBTRACT_ARB;
		case eTextureFunc_AddSigned:	return GL_ADD_SIGNED_ARB;
		case eTextureFunc_Interpolate:	return GL_INTERPOLATE_ARB;
		case eTextureFunc_Dot3RGB:		return GL_DOT3_RGB_ARB;
		case eTextureFunc_Dot3RGBA:		return GL_DOT3_RGBA_ARB;
		}
#endif
	return 0;
}

//-----------------------------------------------------------------------
GLenum cLowLevelGraphicsSDL::GetGLDepthTestFuncEnum(eDepthTestFunc aType) {
#if 0
  		switch(aType)
		{
		case eDepthTestFunc_Never:			return GL_NEVER;
		case eDepthTestFunc_Less:				return GL_LESS;
		case eDepthTestFunc_LessOrEqual:		return GL_LEQUAL;
		case eDepthTestFunc_Greater:			return GL_GREATER;
		case eDepthTestFunc_GreaterOrEqual:	return GL_GEQUAL;
		case eDepthTestFunc_Equal:			return GL_EQUAL;
		case eDepthTestFunc_NotEqual:			return GL_NOTEQUAL;
		case eDepthTestFunc_Always:			return GL_ALWAYS;
		}
#endif
	return 0;
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLAlphaTestFuncEnum(eAlphaTestFunc aType) {
#if 0
  		switch(aType)
		{
		case eAlphaTestFunc_Never:			return GL_NEVER;
		case eAlphaTestFunc_Less:				return GL_LESS;
		case eAlphaTestFunc_LessOrEqual:		return GL_LEQUAL;
		case eAlphaTestFunc_Greater:			return GL_GREATER;
		case eAlphaTestFunc_GreaterOrEqual:	return GL_GEQUAL;
		case eAlphaTestFunc_Equal:			return GL_EQUAL;
		case eAlphaTestFunc_NotEqual:			return GL_NOTEQUAL;
		case eAlphaTestFunc_Always:			return GL_ALWAYS;
		}
#endif
	return 0;
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLStencilFuncEnum(eStencilFunc aType) {
#if 0
  		switch(aType)
		{
		case eStencilFunc_Never:			return GL_NEVER;
		case eStencilFunc_Less:				return GL_LESS;
		case eStencilFunc_LessOrEqual:		return GL_LEQUAL;
		case eStencilFunc_Greater:			return GL_GREATER;
		case eStencilFunc_GreaterOrEqual:	return GL_GEQUAL;
		case eStencilFunc_Equal:			return GL_EQUAL;
		case eStencilFunc_NotEqual:			return GL_NOTEQUAL;
		case eStencilFunc_Always:			return GL_ALWAYS;
		}
#endif
	return 0;
}

//-----------------------------------------------------------------------

GLenum cLowLevelGraphicsSDL::GetGLStencilOpEnum(eStencilOp aType) {
#if 0
  		switch(aType)
		{
		case eStencilOp_Keep:			return GL_KEEP;
		case eStencilOp_Zero:			return GL_ZERO;
		case eStencilOp_Replace:		return GL_REPLACE;
		case eStencilOp_Increment:		return GL_INCR;
		case eStencilOp_Decrement:		return GL_DECR;
		case eStencilOp_Invert:			return GL_INVERT;
		case eStencilOp_IncrementWrap:	return GL_INCR_WRAP_EXT;
		case eStencilOp_DecrementWrap:	return GL_DECR_WRAP_EXT;
		}
#endif
	return 0;
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::InitCG() {
#if 0
  		mCG_Context = cgCreateContext();
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::ExitCG() {
#if 0
  		cgDestroyContext(mCG_Context);
#endif
}

//-----------------------------------------------------------------------

void cLowLevelGraphicsSDL::SetMatrixMode(eMatrix mType) {
#if 0
  		switch(mType)
		{
			case eMatrix_ModelView: glMatrixMode(GL_MODELVIEW);break;
			case eMatrix_Projection: glMatrixMode(GL_PROJECTION); break;
			case eMatrix_Texture: glMatrixMode(GL_TEXTURE); break;
		}
#endif
}

//-----------------------------------------------------------------------

} // namespace hpl
