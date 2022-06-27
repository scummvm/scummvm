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

#include "hpl1/engine/impl/CGProgram.h"
#include "hpl1/engine/impl/SDLTexture.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/system/String.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

tString cCGProgram::msForceFP = "AUTO";
tString cCGProgram::msForceVP = "AUTO";

cCGProgram::cCGProgram(tString asName, CGcontext aContext, eGpuProgramType aType)
	: iGpuProgram(asName, aType) {
#if 0
  		mContext = aContext;

		mProgram = NULL;

#define CG_CHECK(type, p)                                                                        \
	else if (msForce##type == #p) {                                                              \
		mProfile = cgGLIsProfileSupported(CG_PROFILE_##p) ? CG_PROFILE_##p : CG_PROFILE_UNKNOWN; \
	}
		if(mProgramType == eGpuProgramType_Vertex)
		{
			if (msForceVP == "AUTO") {
				mProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
			} CG_CHECK(VP,ARBVP1)
			CG_CHECK(VP,VP40)
			CG_CHECK(VP,VP30)
			CG_CHECK(VP,VP20)
			CG_CHECK(VP,GLSLV) else {
				Log("Forced VP %s unknown\n",msForceVP.c_str());
				mProfile = CG_PROFILE_UNKNOWN;
			}
		}
		else
		{
			if (msForceFP == "AUTO") {
				mProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
			} CG_CHECK(FP,ARBFP1)
			CG_CHECK(FP,FP40)
			CG_CHECK(FP,FP30)
			CG_CHECK(FP,FP20)
			CG_CHECK(FP,GLSLF) else {
				Log("Forced FP %s unknown\n",msForceFP.c_str());
				mProfile = CG_PROFILE_UNKNOWN;
			}
		}
#undef CG_CHECK
		if(mbDebugInfo)
		{
			if(mProfile == CG_PROFILE_UNKNOWN)
				Log("CG: '%s' using profile: UKNOWN\n",asName.c_str());
			else
				Log("CG: '%s' using profile: '%s'\n",asName.c_str(),cgGetProfileString(mProfile));
		}

		for(int i=0; i< MAX_TEXTUREUNITS; ++i)
		{
			mvTexUnitParam[i] = NULL;
		}

		cgGLSetOptimalOptions(mProfile);
#endif
}

cCGProgram::~cCGProgram() {
#if 0
  		if(mProgram) cgDestroyProgram(mProgram);
#endif
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cCGProgram::reload() {
	return false;
}

//-----------------------------------------------------------------------

void cCGProgram::unload() {
}

//-----------------------------------------------------------------------

void cCGProgram::destroy() {
}

//-----------------------------------------------------------------------

bool cCGProgram::CreateFromFile(const tString &asFile, const tString &asEntry) {
#if 0
  if(mProfile == CG_PROFILE_UNKNOWN)
		{
			CGerror err = cgGetError();
			Log("Could not find a working profile for cg file '%s'!\n",msName.c_str());
			if (err != CG_NO_ERROR)Log(" %s\n", cgGetErrorString(err));
			return false;
		}

		//Log("Creating CG prog\n");

		mProgram = cgCreateProgramFromFile(mContext, CG_SOURCE, asFile.c_str(),mProfile,
									asEntry.c_str(),NULL);

		//Log("getting CG errors\n");

		CGerror err = cgGetError();
		if (err != CG_NO_ERROR)
		{
			Log(" %s\n", cgGetErrorString(err));
			const char* pString = cgGetLastListing(mContext);
			int lIdx=0;
			int lLastNewLine=0;

			Log(" -----------------------------------\n");
			while(pString[lIdx]!=0)//true)
			{
				Log("%c",pString[lIdx]);
				/*if(pString[lIdx]==0)
				{
					tString sStr = cString::Sub(pString, lLastNewLine,(lIdx-lLastNewLine));
					Log(" %s",sStr.c_str());
					break;
				}
				else if(pString[lIdx]=='\n')
				{
					tString sStr = cString::Sub(pString, lLastNewLine,(lIdx-lLastNewLine));
					Log(" %s\n",sStr.c_str());
					lLastNewLine = lIdx+1;
				}*/
				lIdx++;
			}
			Log(" -----------------------------------\n");

			Log("CG: Error loading: '%s'!\n",asFile.c_str());
			return false;
		}

		if(mProgram==NULL){
			Log("Error loading: '%s'!\n",asFile.c_str());
			return false;
		}

		//Log("Loading CG program\n");

		cgGLLoadProgram(mProgram);

		err = cgGetError();
		if (err != CG_NO_ERROR)
		{
			Log(" %s\n", cgGetErrorString(err));

			cgDestroyProgram(mProgram);
			mProgram = NULL;

			return false;
		}


		msFile = asFile;
		msEntry = asEntry;

		///////////////////////////////
		//Look for texture units.
		//Log("File: %s\n", msFile.c_str());
		int lCount =0;
		CGparameter Param = cgGetFirstParameter(mProgram, CG_PROGRAM);
		for(; Param != NULL; Param = cgGetNextParameter(Param))
		{
			//Check if it is a texture type
			CGparameterclass paramClass = cgGetParameterClass(Param);
			if(	paramClass != CG_PARAMETERCLASS_SAMPLER)
			{
				continue;
			}

			//Get the unit number
			int lUnit = lCount;
			const char *pSemantic = cgGetParameterSemantic(Param);
			if(pSemantic)
			{
				lUnit =	cString::ToInt(cString::Sub(pSemantic,7).c_str(),0);
			}
			tString sName = cgGetParameterName(Param);
			//Log(" Texture %d: %s\n",lUnit,sName.c_str());

			mvTexUnitParam[lUnit] = Param;

			++lCount;
		}
#endif

	return true;
}

//-----------------------------------------------------------------------

void cCGProgram::Bind() {
#if 0
  		cgGLBindProgram(mProgram);
		cgGLEnableProfile(mProfile);
#endif
}

//-----------------------------------------------------------------------

void cCGProgram::UnBind() {
#if 0
  		cgGLDisableProfile(mProfile);
#endif
}

//-----------------------------------------------------------------------

bool cCGProgram::SetFloat(const tString &asName, float afX) {
#if 0
  		CGparameter Param = GetParam(asName, CG_FLOAT);
		if(Param==NULL)return false;

		cgGLSetParameter1f(Param, afX);
#endif
	return true;
}

//-----------------------------------------------------------------------

bool cCGProgram::SetVec2f(const tString &asName, float afX, float afY) {
#if 0
  		CGparameter Param = GetParam(asName, CG_FLOAT2);
		if(Param==NULL)return false;

		cgGLSetParameter2f(Param, afX, afY);
#endif

	return true;
}

//-----------------------------------------------------------------------

bool cCGProgram::SetVec3f(const tString &asName, float afX, float afY, float afZ) {
#if 0
  		CGparameter Param = GetParam(asName, CG_FLOAT3);
		if(Param==NULL)return false;

		cgGLSetParameter3f(Param, afX, afY, afZ);
#endif

	return true;
}

//-----------------------------------------------------------------------

bool cCGProgram::SetVec4f(const tString &asName, float afX, float afY, float afZ, float afW) {
#if 0
  		CGparameter Param = GetParam(asName, CG_FLOAT4);
		if(Param==NULL)return false;

		cgGLSetParameter4f(Param, afX, afY, afZ, afW);
#endif

	return true;
}

//-----------------------------------------------------------------------

bool cCGProgram::SetMatrixf(const tString &asName, const cMatrixf &mMtx) {
#if 0
  		CGparameter Param = GetParam(asName, CG_FLOAT4x4);
		if(Param==NULL)return false;

		cgGLSetMatrixParameterfr(Param,&mMtx.m[0][0]);
#endif

	return true;
}

//-----------------------------------------------------------------------

bool cCGProgram::SetMatrixf(const tString &asName, eGpuProgramMatrix mType,
							eGpuProgramMatrixOp mOp) {
#if 0
  		CGparameter Param = GetParam(asName, CG_FLOAT4x4);
		if(Param==NULL)return false;

		CGGLenum MtxType;
		CGGLenum OpType;
		switch(mType){
			case eGpuProgramMatrix_View:				MtxType=CG_GL_MODELVIEW_MATRIX;break;
			case eGpuProgramMatrix_Projection:		MtxType=CG_GL_PROJECTION_MATRIX;break;
			case eGpuProgramMatrix_Texture:			MtxType=CG_GL_TEXTURE_MATRIX;break;
			case eGpuProgramMatrix_ViewProjection:	MtxType=CG_GL_MODELVIEW_PROJECTION_MATRIX;break;
		}
		switch(mOp){
			case eGpuProgramMatrixOp_Identity:		OpType=CG_GL_MATRIX_IDENTITY; break;
			case eGpuProgramMatrixOp_Inverse:		OpType=CG_GL_MATRIX_INVERSE; break;
			case eGpuProgramMatrixOp_Transpose:		OpType=CG_GL_MATRIX_TRANSPOSE; break;
			case eGpuProgramMatrixOp_InverseTranspose:OpType=CG_GL_MATRIX_INVERSE_TRANSPOSE; break;
		}

		cgGLSetStateMatrixParameter(Param,MtxType, OpType);
#endif

	return true;
}

//-----------------------------------------------------------------------

bool cCGProgram::SetTexture(const tString &asName, iTexture *apTexture, bool abAutoDisable) {
#if 0
  		CGtype textureType;

			if(apTexture)
		{
			switch(apTexture->GetTarget())
			{
				case eTextureTarget_1D:			textureType = CG_SAMPLER1D; break;
				case eTextureTarget_2D:			textureType = CG_SAMPLER2D; break;
				case eTextureTarget_CubeMap:	textureType = CG_SAMPLERCUBE; break;
				case eTextureTarget_Rect:		textureType = CG_SAMPLERRECT; break;
			}

			CGparameter Param = GetParam(asName, textureType);
			if(Param==NULL)return false;

			cSDLTexture* pSDLTex = static_cast<cSDLTexture*>(apTexture);

			//Log("Intializing Tex %s(%d): %d\n",cgGetParameterName(Param),
			//		Param,pSDLTex->GetTextureHandle());
			cgGLSetTextureParameter(Param, pSDLTex->GetTextureHandle());
			cgGLEnableTextureParameter(Param);
		}
		else
		{
			CGparameter Param = GetParam(asName, CG_SAMPLER2D);
			if(Param==NULL)return false;

			cgGLDisableTextureParameter(Param);
		}
#endif

	return true;
}

//-----------------------------------------------------------------------

bool cCGProgram::SetTextureToUnit(int alUnit, iTexture *apTexture) {
#if 0
  		if(mvTexUnitParam[alUnit]==NULL || alUnit >= MAX_TEXTUREUNITS) return false;

		cSDLTexture* pSDLTex = static_cast<cSDLTexture*>(apTexture);

		if(apTexture)
		{
			//Log("Intializing TexUnit %s(%d): %d\n",cgGetParameterName(mvTexUnitParam[alUnit]),
			//		mvTexUnitParam[alUnit],pSDLTex->GetTextureHandle());
			cgGLSetTextureParameter(mvTexUnitParam[alUnit], pSDLTex->GetTextureHandle());
			cgGLEnableTextureParameter(mvTexUnitParam[alUnit]);
		}
		else
		{
			cgGLDisableTextureParameter(mvTexUnitParam[alUnit]);
		}
#endif

	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

CGparameter cCGProgram::GetParam(const tString &asName, CGtype aType) {
#if 0
  		CGparameter Param = cgGetNamedParameter(mProgram, asName.c_str());
		if(Param==NULL)return NULL;

		CGtype type= cgGetParameterType(Param);
		if(type!=aType)return NULL;

#endif
	return 0;
}

//-----------------------------------------------------------------------

} // namespace hpl
