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

#include "hpl1/engine/graphics/RenderState.h"
#include "hpl1/engine/graphics/Renderer3D.h"

#include "hpl1/debug.h"
#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/graphics/VertexBuffer.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/scene/Camera3D.h"
#include "hpl1/engine/scene/Light3D.h"
#include "hpl1/engine/scene/Light3DSpot.h"
#include "hpl1/engine/scene/PortalContainer.h"

namespace hpl {

template<class T>
static int GetCompareVal(T a, T b) {
	if (a == b)
		return 0;
	else
		return a < b ? -1 : 1;
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

int iRenderState::Compare(const iRenderState *apState) const {
	switch (mType) {
	case eRenderStateType_Sector:
		return CompareSector(apState);
	case eRenderStateType_Pass:
		return ComparePass(apState);
	case eRenderStateType_DepthTest:
		return CompareDepthTest(apState);
	case eRenderStateType_Depth:
		return CompareDepth(apState);
	case eRenderStateType_AlphaMode:
		return CompareAlpha(apState);
	case eRenderStateType_BlendMode:
		return CompareBlend(apState);
	case eRenderStateType_VertexProgram:
		return CompareVtxProg(apState);
	case eRenderStateType_FragmentProgram:
		return CompareFragProg(apState);
	case eRenderStateType_Texture:
		return CompareTexture(apState);
	case eRenderStateType_VertexBuffer:
		return CompareVtxBuff(apState);
	case eRenderStateType_Matrix:
		return CompareMatrix(apState);
	case eRenderStateType_Render:
		return CompareRender(apState);
	case eRenderStateType_GpuProgram:
		return compareGpuProgram(apState);
	default:
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------

void iRenderState::SetMode(cRenderSettings *apSettings) {
	switch (mType) {
	case eRenderStateType_Sector:
		SetSectorMode(apSettings);
		break;
	case eRenderStateType_Pass:
		SetPassMode(apSettings);
		break;
	case eRenderStateType_DepthTest:
		SetDepthTestMode(apSettings);
		break;
	case eRenderStateType_Depth:
		SetDepthMode(apSettings);
		break;
	case eRenderStateType_AlphaMode:
		SetAlphaMode(apSettings);
		break;
	case eRenderStateType_BlendMode:
		SetBlendMode(apSettings);
		break;
	case eRenderStateType_Texture:
		SetTextureMode(apSettings);
		break;
	case eRenderStateType_VertexBuffer:
		SetVtxBuffMode(apSettings);
		break;
	case eRenderStateType_Matrix:
		SetMatrixMode(apSettings);
		break;
	case eRenderStateType_Render:
		SetRenderMode(apSettings);
		break;
	case eRenderStateType_GpuProgram:
		setGpuProgMode(apSettings);
	default:
		break;
	}
}

//-----------------------------------------------------------------------

void iRenderState::Set(const iRenderState *apState) {
	mType = apState->mType;
	switch (mType) {
	case eRenderStateType_Sector:
		mpSector = apState->mpSector;
		break;
	case eRenderStateType_Pass:
		mlPass = apState->mlPass;
		break;
	case eRenderStateType_DepthTest:
		mbDepthTest = apState->mbDepthTest;
		break;
	case eRenderStateType_Depth:
		mfZ = apState->mfZ;
		break;

	case eRenderStateType_AlphaMode:
		mAlphaMode = apState->mAlphaMode;
		break;

	case eRenderStateType_BlendMode:
		mBlendMode = apState->mBlendMode;
		mChannelMode = apState->mChannelMode;
		break;

	case eRenderStateType_VertexProgram:
		mpVtxProgram = apState->mpVtxProgram;
		mpVtxProgramSetup = apState->mpVtxProgramSetup;
		mbUsesEye = apState->mbUsesEye;
		mbUsesLight = apState->mbUsesLight;
		mpLight = apState->mpLight;
		break;

	case eRenderStateType_FragmentProgram:
		mpFragProgram = apState->mpFragProgram;
		mpFragProgramSetup = apState->mpFragProgramSetup;
		break;

	case eRenderStateType_Texture:
		for (int i = 0; i < MAX_TEXTUREUNITS; i++)
			mpTexture[i] = apState->mpTexture[i];
		break;

	case eRenderStateType_VertexBuffer:
		mpVtxBuffer = apState->mpVtxBuffer;
		break;

	case eRenderStateType_Matrix:
		mpModelMatrix = apState->mpModelMatrix;
		mpInvModelMatrix = apState->mpInvModelMatrix;
		mvScale = apState->mvScale;
		break;

	case eRenderStateType_Render:
		mpObject = apState->mpObject;
		break;

	case eRenderStateType_GpuProgram:
		gpuProgram = apState->gpuProgram;
		gpuProgramSetup = apState->gpuProgramSetup;
		mbUsesEye = apState->mbUsesEye;
		mbUsesLight = apState->mbUsesLight;
		mpLight = apState->mpLight;

	default:
		break;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void iRenderState::SetSectorMode(cRenderSettings *apSettings) {
	if (apSettings->mbLog)
		Log("Sector: %d\n", mpSector);

	apSettings->mpSector = mpSector;
	if (mpSector) {
		apSettings->mpLowLevel->SetTextureConstantColor(apSettings->mAmbientColor * mpSector->GetAmbientColor());
	} else {
		apSettings->mpLowLevel->SetTextureConstantColor(apSettings->mAmbientColor);
	}

	// To make sure that new ambient is set:
	apSettings->mpFragmentProgram = NULL;
}

//-----------------------------------------------------------------------

void iRenderState::SetPassMode(cRenderSettings *apSettings) {
	if (apSettings->mbLog)
		Log("Pass: %d\n", mlPass);
}

//-----------------------------------------------------------------------

void iRenderState::SetDepthTestMode(cRenderSettings *apSettings) {
	if (apSettings->mbDepthTest != mbDepthTest) {
		apSettings->mpLowLevel->SetDepthTestActive(mbDepthTest);
		apSettings->mbDepthTest = mbDepthTest;

		if (apSettings->mbLog)
			Log("Setting depth test: %d\n", mbDepthTest ? 1 : 0);
	}
}
//-----------------------------------------------------------------------

void iRenderState::SetDepthMode(cRenderSettings *apSettings) {
}

//-----------------------------------------------------------------------

void iRenderState::SetAlphaMode(cRenderSettings *apSettings) {
	if (mAlphaMode != apSettings->mAlphaMode) {
		if (apSettings->mbLog)
			Log("Setting alpha: ");
		apSettings->mAlphaMode = mAlphaMode;

		if (mAlphaMode == eMaterialAlphaMode_Solid) {
			apSettings->mpLowLevel->SetAlphaTestActive(false);
			if (apSettings->mbLog)
				Log("Solid");
		} else {
			// apSettings->mpLowLevel->SetTextureConstantColor(cColor(0,0));
			// apSettings->mpLowLevel->SetTextureEnv(eTextureParam_ColorSource1,eTextureSource_Constant);

			// apSettings->mpLowLevel->SetAlphaTestActive(false);
			apSettings->mpLowLevel->SetAlphaTestActive(true);
			apSettings->mpLowLevel->SetAlphaTestFunc(eAlphaTestFunc_GreaterOrEqual, 0.6f);
			if (apSettings->mbLog)
				Log("Trans");
		}

		if (apSettings->mbLog)
			Log("\n");
	}
}

//-----------------------------------------------------------------------

void iRenderState::SetBlendMode(cRenderSettings *apSettings) {
	if (mBlendMode != apSettings->mBlendMode) {
		if (apSettings->mbLog)
			Log("Setting blend mode: ");
		apSettings->mBlendMode = mBlendMode;

		if (mBlendMode == eMaterialBlendMode_None) {
			apSettings->mpLowLevel->SetBlendActive(false);
			if (apSettings->mbLog)
				Log("None");
		} else {
			apSettings->mpLowLevel->SetBlendActive(true);

			switch (mBlendMode) {
			case eMaterialBlendMode_Add:
				apSettings->mpLowLevel->SetBlendFunc(eBlendFunc_One, eBlendFunc_One);
				if (apSettings->mbLog)
					Log("Add");
				break;
			case eMaterialBlendMode_Replace:
				apSettings->mpLowLevel->SetBlendFunc(eBlendFunc_One, eBlendFunc_Zero);
				if (apSettings->mbLog)
					Log("Replace");
				break;
			case eMaterialBlendMode_Mul:
				apSettings->mpLowLevel->SetBlendFunc(eBlendFunc_Zero, eBlendFunc_SrcColor);
				if (apSettings->mbLog)
					Log("Mul");
				break;
			case eMaterialBlendMode_MulX2:
				apSettings->mpLowLevel->SetBlendFunc(eBlendFunc_DestColor, eBlendFunc_SrcColor);
				if (apSettings->mbLog)
					Log("MulX2");
				break;
			case eMaterialBlendMode_Alpha:
				apSettings->mpLowLevel->SetBlendFunc(eBlendFunc_SrcAlpha, eBlendFunc_OneMinusSrcAlpha);
				if (apSettings->mbLog)
					Log("Alpha");
				break;
			case eMaterialBlendMode_DestAlphaAdd:
				apSettings->mpLowLevel->SetBlendFunc(eBlendFunc_DestAlpha, eBlendFunc_One);
				if (apSettings->mbLog)
					Log("DestAlphaAdd");
				break;
			default:
				break;
			}
		}

		if (apSettings->mbLog)
			Log("\n");
	}

	if (mChannelMode != apSettings->mChannelMode) {
		if (apSettings->mbLog)
			Log("Setting channel: ");
		apSettings->mChannelMode = mChannelMode;

		switch (mChannelMode) {
		case eMaterialChannelMode_RGBA:
			apSettings->mpLowLevel->SetColorWriteActive(true, true, true, true);
			if (apSettings->mbLog)
				Log("RGBA");
			break;
		case eMaterialChannelMode_RGB:
			apSettings->mpLowLevel->SetColorWriteActive(true, true, true, false);
			if (apSettings->mbLog)
				Log("RGB");
			break;
		case eMaterialChannelMode_A:
			apSettings->mpLowLevel->SetColorWriteActive(false, false, false, true);
			if (apSettings->mbLog)
				Log("A");
			break;
		case eMaterialChannelMode_Z:
			apSettings->mpLowLevel->SetColorWriteActive(false, false, false, false);
			if (apSettings->mbLog)
				Log("Z");
			break;
		default:
			break;
		}

		if (apSettings->mbLog)
			Log("\n");
	}
}

//-----------------------------------------------------------------------

void iRenderState::setGpuProgMode(cRenderSettings *settings) {
	if (gpuProgram != settings->gpuProgram) {
		if (gpuProgram != settings->gpuProgram) {
			if (gpuProgram) {
				gpuProgram->Bind();

				if (gpuProgramSetup) {
					gpuProgramSetup->Setup(gpuProgram, settings);
				}
				settings->gpuProgramSetup = gpuProgramSetup;

				// reset this so all matrix setting are set to vertex program.
				settings->mbMatrixWasNULL = false;
				if (mbUsesLight) {
					gpuProgram->SetColor4f("LightColor", mpLight->GetDiffuseColor());
					settings->mpLight = mpLight;
				} else {
					settings->mpLight = nullptr;
				}
				settings->mbUsesLight = mbUsesLight;
				settings->mbUsesEye = mbUsesEye;
			} else {
				settings->gpuProgram->UnBind();
			}
			settings->gpuProgram = gpuProgram;
		} else if (settings->gpuProgram && mbUsesLight && mpLight != settings->mpLight) {
			gpuProgram->SetColor4f("LightColor", mpLight->GetDiffuseColor());
			settings->mpLight = mpLight;
		}
	}
}

//-----------------------------------------------------------------------

void iRenderState::SetTextureMode(cRenderSettings *apSettings) {
	for (int i = 0; i < MAX_TEXTUREUNITS; i++) {
		if (apSettings->mpTexture[i] != mpTexture[i]) {
			if (apSettings->mbLog) {
				if (mpTexture[i] == NULL)
					Log("Setting texture: %d / %d : NULL\n", i, (size_t)mpTexture[i]);
				else
					Log("Setting texture: %d / %d : '%s'\n", i, (size_t)mpTexture[i],
						mpTexture[i]->GetName().c_str());
			}

			apSettings->mpLowLevel->SetTexture(i, mpTexture[i]);
			apSettings->mpTexture[i] = mpTexture[i];
		}
	}
}

//-----------------------------------------------------------------------

void iRenderState::SetVtxBuffMode(cRenderSettings *apSettings) {
	if (mpVtxBuffer != apSettings->mpVtxBuffer) {
		if (apSettings->mbLog)
			Log("Setting vertex buffer: %d\n", (size_t)mpVtxBuffer);
		if (apSettings->mpVtxBuffer)
			apSettings->mpVtxBuffer->UnBind();
		apSettings->mpVtxBuffer = mpVtxBuffer;

		if (mpVtxBuffer) {
			mpVtxBuffer->Bind();
		}
	}
}

//-----------------------------------------------------------------------

void iRenderState::SetMatrixMode(cRenderSettings *apSettings) {
	// It is a normal matrix
	if (mpModelMatrix) {
		cMatrixf mtxModel = cMath::MatrixMul(apSettings->mpCamera->GetViewMatrix(),
											 *mpModelMatrix);

		if (apSettings->mbLog)
			Log("Setting model matrix: %s ", cMath::MatrixToChar(*mpModelMatrix));

		apSettings->mpLowLevel->SetMatrix(eMatrix_ModelView, mtxModel);

		apSettings->mbMatrixWasNULL = false;
	}
	// NULL matrix
	else {
		// If NULL already is set, no need for changes.
		if (apSettings->mbMatrixWasNULL)
			return;

		if (apSettings->mbLog)
			Log("Setting model matrix: Identity (NULL) ");

		apSettings->mpLowLevel->SetMatrix(eMatrix_ModelView, apSettings->mpCamera->GetViewMatrix());

		apSettings->mbMatrixWasNULL = true;
	}

	if (apSettings->gpuProgram) {
		// Might be quicker if this is set directly
		apSettings->gpuProgram->SetMatrixf("worldViewProj",
										   eGpuProgramMatrix_ViewProjection,
										   eGpuProgramMatrixOp_Identity);
		if (apSettings->gpuProgramSetup) {
			apSettings->gpuProgramSetup->SetupMatrix(mpModelMatrix, apSettings);
		}

		if (apSettings->mbUsesLight) {
			if (apSettings->mbLog)
				Log("Light ");
			if (mpModelMatrix) {
				// Light position
				cVector3f vLocalLight = cMath::MatrixMul(*mpInvModelMatrix,
														 apSettings->mpLight->GetLightPosition());
				apSettings->gpuProgram->SetVec3f("LightPos", vLocalLight);

				// LightDir Div, use scale to make attenuation correct!
				cVector3f vLightDirDiv = mvScale / apSettings->mpLight->GetFarAttenuation();
				apSettings->gpuProgram->SetVec3f("LightDirMul", vLightDirDiv);

				if (apSettings->mbLog)
					Log("(%s) LightDirMul (%s) ", vLocalLight.ToString().c_str(), vLightDirDiv.ToString().c_str());

				// Light view projection
				if (apSettings->mpLight->GetLightType() == eLight3DType_Spot) {
					if (apSettings->mbLog)
						Log("SpotLightViewProj ");
					cLight3DSpot *pSpotLight = static_cast<cLight3DSpot *>(apSettings->mpLight);
					apSettings->gpuProgram->SetMatrixf("spotViewProj",
													   cMath::MatrixMul(pSpotLight->GetViewProjMatrix(), *mpModelMatrix));
				}
			} else {
				// Light position
				apSettings->gpuProgram->SetVec3f("LightPos", apSettings->mpLight->GetLightPosition());

				// LightDir Div
				apSettings->gpuProgram->SetVec3f("LightDirMul", 1.0f / apSettings->mpLight->GetFarAttenuation());

				// Light view projection
				if (apSettings->mpLight->GetLightType() == eLight3DType_Spot) {
					if (apSettings->mbLog)
						Log("SpotLightViewProj ");
					cLight3DSpot *pSpotLight = static_cast<cLight3DSpot *>(apSettings->mpLight);
					apSettings->gpuProgram->SetMatrixf("spotViewProj", pSpotLight->GetViewProjMatrix());
				}
			}
		}

		if (apSettings->mbUsesEye) {
			if (apSettings->mbLog)
				Log("Eye ");
			if (mpModelMatrix) {
				cVector3f vLocalEye = cMath::MatrixMul(*mpInvModelMatrix,
													   apSettings->mpCamera->GetEyePosition());
				apSettings->gpuProgram->SetVec3f("EyePos", vLocalEye);
			} else {
				apSettings->gpuProgram->SetVec3f("EyePos", apSettings->mpCamera->GetEyePosition());
			}
		}
	}

	if (apSettings->mbLog)
		Log("\n");
}

//-----------------------------------------------------------------------

void iRenderState::SetRenderMode(cRenderSettings *apSettings) {
	if (apSettings->mbLog)
		Log("Drawing\n-----------------\n");

	if (apSettings->mDebugFlags & eRendererDebugFlag_RenderLines) {
		apSettings->mpVtxBuffer->Draw(eVertexBufferDrawType_Lines);
	} else {
		apSettings->mpVtxBuffer->Draw();
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// COMPARE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

int iRenderState::CompareSector(const iRenderState *apState) const {
	return (size_t)mpSector < (size_t)apState->mpSector;
}
//-----------------------------------------------------------------------

int iRenderState::ComparePass(const iRenderState *apState) const {
	return (int)mlPass < (int)apState->mlPass;
}

//-----------------------------------------------------------------------

int iRenderState::CompareDepthTest(const iRenderState *apState) const {
	return (int)mbDepthTest < (int)apState->mbDepthTest;
}

//-----------------------------------------------------------------------

int iRenderState::CompareDepth(const iRenderState *apState) const {
	if (ABS(mfZ - apState->mfZ) < 0.00001f)
		return 0;
	else
		return mfZ < apState->mfZ ? 1 : -1;
}

//-----------------------------------------------------------------------
int iRenderState::CompareAlpha(const iRenderState *apState) const {
	return GetCompareVal((int)mAlphaMode, (int)apState->mAlphaMode);
}

//-----------------------------------------------------------------------

int iRenderState::CompareBlend(const iRenderState *apState) const {
	int lRet = GetCompareVal((int)mChannelMode, (int)apState->mChannelMode);
	if (lRet == 0) {
		return GetCompareVal((int)mBlendMode, (int)apState->mBlendMode);
	}
	return lRet;
}

//-----------------------------------------------------------------------

int iRenderState::CompareVtxProg(const iRenderState *apState) const {
	return GetCompareVal(mpVtxProgram, apState->mpVtxProgram);
}
//-----------------------------------------------------------------------

int iRenderState::CompareFragProg(const iRenderState *apState) const {
	return GetCompareVal(mpFragProgram, apState->mpFragProgram);
}

//-----------------------------------------------------------------------

int iRenderState::CompareTexture(const iRenderState *apState) const {
	for (int i = 0; i < MAX_TEXTUREUNITS - 1; ++i) {
		if (mpTexture[i] != apState->mpTexture[i])
			return GetCompareVal(mpTexture[i], apState->mpTexture[i]);
	}
	return GetCompareVal(mpTexture[MAX_TEXTUREUNITS - 1], apState->mpTexture[MAX_TEXTUREUNITS - 1]);
}
//-----------------------------------------------------------------------

int iRenderState::CompareVtxBuff(const iRenderState *apState) const {
	return GetCompareVal(mpVtxBuffer, apState->mpVtxBuffer);
}

//-----------------------------------------------------------------------

int iRenderState::CompareMatrix(const iRenderState *apState) const {
	return GetCompareVal(mpModelMatrix, apState->mpModelMatrix);
}

//-----------------------------------------------------------------------

int iRenderState::CompareRender(const iRenderState *apState) const {
	return GetCompareVal(mpObject, apState->mpObject);
}

int iRenderState::compareGpuProgram(const iRenderState *state) const {
	return GetCompareVal(gpuProgram, state->gpuProgram);
}

//-----------------------------------------------------------------------

} // namespace hpl
