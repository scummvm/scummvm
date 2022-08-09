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

#include "hpl1/engine/graphics/ParticleEmitter3D.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/scene/Camera3D.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/game/Game.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/World3D.h"

#include "hpl1/engine/graphics/ParticleSystem3D.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iParticleEmitter3D::iParticleEmitter3D(tString asName, tMaterialVec *avMaterials,
									   unsigned int alMaxParticles, cVector3f avSize,
									   cGraphics *apGraphics, cResources *apResources)
	: iRenderable(asName), iParticleEmitter(avMaterials, alMaxParticles, avSize, apGraphics, apResources) {
	mpVtxBuffer = apGraphics->GetLowLevel()->CreateVertexBuffer(
		eVertexFlag_Position | eVertexFlag_Color0 | eVertexFlag_Texture0,
		eVertexBufferDrawType_Tri, eVertexBufferUsageType_Stream,
		alMaxParticles * 4, alMaxParticles * 6);

	// Fill the indices with quads
	for (int i = 0; i < (int)alMaxParticles; i++) {
		int lStart = i * 4;
		for (int j = 0; j < 3; j++)
			mpVtxBuffer->AddIndex(lStart + j);
		for (int j = 2; j < 5; j++)
			mpVtxBuffer->AddIndex(lStart + (j == 4 ? 0 : j));
	}

	// Fill with tetxure coords (will do for most particle systems)
	for (int i = 0; i < (int)alMaxParticles; i++) {
		mpVtxBuffer->AddVertex(eVertexFlag_Texture0, cVector3f(1, 1, 0));
		mpVtxBuffer->AddVertex(eVertexFlag_Texture0, cVector3f(0, 1, 0));
		mpVtxBuffer->AddVertex(eVertexFlag_Texture0, cVector3f(0, 0, 0));
		mpVtxBuffer->AddVertex(eVertexFlag_Texture0, cVector3f(1, 0, 0));
	}

	// Set default values for pos and col
	for (int i = 0; i < (int)alMaxParticles * 4; i++) {
		mpVtxBuffer->AddVertex(eVertexFlag_Position, 0);
		mpVtxBuffer->AddColor(eVertexFlag_Color0, cColor(1, 1));
	}

	mpVtxBuffer->Compile(0);

	// Setup vars
	mlDirectionUpdateCount = -1;
	mvDirection = cVector3f(0, 0, 0);

	mvMaxDrawSize = 0;

	mlAxisDrawUpdateCount = -1;

	mbApplyTransformToBV = false;

	mlUpdateCount = 0;
	mfTimeStepAccum = 0;

	// If Direction should be udpdated
	mbUsesDirection = false;

	mDrawType = eParticleEmitter3DType_FixedPoint;

	mCoordSystem = eParticleEmitter3DCoordSystem_World;
}

//-----------------------------------------------------------------------

iParticleEmitter3D::~iParticleEmitter3D() {
	hplDelete(mpVtxBuffer);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void iParticleEmitter3D::SetSubDivUV(const cVector2l &avSubDiv) {
	// Check so that there is any subdivision and that no sub divison axis is
	// equal or below zero
	if ((avSubDiv.x > 1 || avSubDiv.x > 1) && (avSubDiv.x > 0 && avSubDiv.y > 0)) {
		int lSubDivNum = avSubDiv.x * avSubDiv.y;

		mvSubDivUV.resize(lSubDivNum);

		float fInvW = 1.0f / (float)avSubDiv.x;
		float fInvH = 1.0f / (float)avSubDiv.y;

		for (int x = 0; x < avSubDiv.x; ++x)
			for (int y = 0; y < avSubDiv.y; ++y) {
				int lIdx = y * avSubDiv.x + x;

				float fX = (float)x;
				float fY = (float)y;

				cPESubDivision *pSubDiv = &mvSubDivUV[lIdx];

				pSubDiv->mvUV[0] = cVector3f((fX + 1) * fInvW, (fY + 1) * fInvH, 0); // 1,1
				pSubDiv->mvUV[1] = cVector3f(fX * fInvW, (fY + 1) * fInvH, 0);       // 0,1
				pSubDiv->mvUV[2] = cVector3f(fX * fInvW, fY * fInvH, 0);             // 0,0
				pSubDiv->mvUV[3] = cVector3f((fX + 1) * fInvW, fY * fInvH, 0);       // 1,0
			}
	}
}

//-----------------------------------------------------------------------

void iParticleEmitter3D::UpdateLogic(float afTimeStep) {
	if (IsActive() == false)
		return;

	// Skip as it has never been sued before right?
	/*if(mbUsesDirection)
	{
		if(mlDirectionUpdateCount != GetMatrixUpdateCount())
		{
			mlDirectionUpdateCount = GetMatrixUpdateCount();
			cMatrixf mtxInv = cMath::MatrixInverse(GetWorldMatrix());
			mvDirection = mtxInv.GetUp();
		}
	}*/

	Update(afTimeStep);
}

//-----------------------------------------------------------------------

iMaterial *iParticleEmitter3D::GetMaterial() {
	return (*mvMaterials)[(int)mfFrame];
}

//-----------------------------------------------------------------------

static inline void SetCol(float *apCol, const cColor &aCol) {
	apCol[0] = aCol.r;
	apCol[1] = aCol.g;
	apCol[2] = aCol.b;
	apCol[3] = aCol.a;
}

static inline void SetPos(float *apPos, const cVector3f &aPos) {
	apPos[0] = aPos.x;
	apPos[1] = aPos.y;
	apPos[2] = aPos.z;
}

static inline void SetTex(float *apTex, const cVector3f &aPos) {
	apTex[0] = aPos.x;
	apTex[1] = aPos.y;
	apTex[2] = aPos.z;
}

void iParticleEmitter3D::UpdateGraphics(cCamera3D *apCamera, float afFrameTime, cRenderList *apRenderList) {
	if (apCamera == NULL)
		return;

	if (mbUpdateGfx) {
		float *pPosArray = mpVtxBuffer->GetArray(eVertexFlag_Position);
		float *pColArray = mpVtxBuffer->GetArray(eVertexFlag_Color0);
		// NEW
		if (mPEType == ePEType_Beam) {
			//				for (int i=0; i<(int)mlNumOfParticles; i++)
			//				{
			//					cParticle *pParticle = mvParticles[i];

			//					for (int j = 0; j < (int) pParticle->mvBeamPoints->size(); j++)
			//					{

			//					}
			//				}

		} else {
			//////////////////////////////////////////////////
			// SUB DIVISION SET UP
			if (mvSubDivUV.size() > 1) {
				float *pTexArray = mpVtxBuffer->GetArray(eVertexFlag_Texture0);

				for (int i = 0; i < (int)mlNumOfParticles; i++) {
					cParticle *pParticle = mvParticles[i];

					cPESubDivision &subDiv = mvSubDivUV[pParticle->mlSubDivNum];

					SetTex(&pTexArray[i * 12 + 0 * 3], subDiv.mvUV[0]);
					SetTex(&pTexArray[i * 12 + 1 * 3], subDiv.mvUV[1]);
					SetTex(&pTexArray[i * 12 + 2 * 3], subDiv.mvUV[2]);
					SetTex(&pTexArray[i * 12 + 3 * 3], subDiv.mvUV[3]);

					/*SetTex(&pTexArray[i*12 + 0*3], cVector3f(1,1,0));
					SetTex(&pTexArray[i*12 + 1*3], cVector3f(0,1,0));
					SetTex(&pTexArray[i*12 + 2*3], cVector3f(0,0,0));
					SetTex(&pTexArray[i*12 + 3*3], cVector3f(1,0,0));*/
				}
			}

			//////////////////////////////////////////////////
			// FIXED POINT
			if (mDrawType == eParticleEmitter3DType_FixedPoint) {
				cVector3f vAdd[4] = {
					cVector3f(mvDrawSize.x, -mvDrawSize.y, 0),
					cVector3f(-mvDrawSize.x, -mvDrawSize.y, 0),
					cVector3f(-mvDrawSize.x, mvDrawSize.y, 0),
					cVector3f(mvDrawSize.x, mvDrawSize.y, 0)};

				// NEW

				// ---

				int lVtxStride = kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)];
				int lVtxQuadSize = lVtxStride * 4;

				for (int i = 0; i < (int)mlNumOfParticles; i++) {
					cParticle *pParticle = mvParticles[i];

					// This is not the fastest thing possible...
					cVector3f vParticlePos = pParticle->mvPos;

					if (mCoordSystem == eParticleEmitter3DCoordSystem_Local) {
						vParticlePos = cMath::MatrixMul(mpParentSystem->GetWorldMatrix(), vParticlePos);
					}

					cVector3f vPos = cMath::MatrixMul(apCamera->GetViewMatrix(), vParticlePos);

					SetPos(&pPosArray[i * lVtxQuadSize + 0 * lVtxStride], vPos + vAdd[0]);
					SetCol(&pColArray[i * 16 + 0 * 4], pParticle->mColor);

					SetPos(&pPosArray[i * lVtxQuadSize + 1 * lVtxStride], vPos + vAdd[1]);
					SetCol(&pColArray[i * 16 + 1 * 4], pParticle->mColor);

					SetPos(&pPosArray[i * lVtxQuadSize + 2 * lVtxStride], vPos + vAdd[2]);
					SetCol(&pColArray[i * 16 + 2 * 4], pParticle->mColor);

					SetPos(&pPosArray[i * lVtxQuadSize + 3 * lVtxStride], vPos + vAdd[3]);
					SetCol(&pColArray[i * 16 + 3 * 4], pParticle->mColor);
				}
			}
			//////////////////////////////////////////////////
			// DYNAMIC POINT
			else if (mDrawType == eParticleEmitter3DType_DynamicPoint) {
				cVector3f vAdd[4] = {
					cVector3f(mvDrawSize.x, -mvDrawSize.y, 0),
					cVector3f(-mvDrawSize.x, -mvDrawSize.y, 0),
					cVector3f(-mvDrawSize.x, mvDrawSize.y, 0),
					cVector3f(mvDrawSize.x, mvDrawSize.y, 0)};

				int lVtxStride = kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)];
				int lVtxQuadSize = lVtxStride * 4;

				for (int i = 0; i < (int)mlNumOfParticles; i++) {
					cParticle *pParticle = mvParticles[i];

					// This is not the fastest thing possible
					cVector3f vParticlePos = pParticle->mvPos;

					if (mCoordSystem == eParticleEmitter3DCoordSystem_Local) {
						vParticlePos = cMath::MatrixMul(mpParentSystem->GetWorldMatrix(), vParticlePos);
					}

					cVector3f vPos = cMath::MatrixMul(apCamera->GetViewMatrix(), vParticlePos);

					// NEW

					cVector3f vParticleSize = pParticle->mvSize;
					cColor &colParticleColor = pParticle->mColor;

					if (mbUsePartSpin) {
						cMatrixf mtxRotationMatrix = cMath::MatrixRotateZ(pParticle->mfSpin);

						SetPos(&pPosArray[i * lVtxQuadSize + 0 * lVtxStride], vPos + cMath::MatrixMul(mtxRotationMatrix, vAdd[0] * vParticleSize));
						SetCol(&pColArray[i * 16 + 0 * 4], colParticleColor);

						SetPos(&pPosArray[i * lVtxQuadSize + 1 * lVtxStride], vPos + cMath::MatrixMul(mtxRotationMatrix, vAdd[1] * vParticleSize));
						SetCol(&pColArray[i * 16 + 1 * 4], colParticleColor);

						SetPos(&pPosArray[i * lVtxQuadSize + 2 * lVtxStride], vPos + cMath::MatrixMul(mtxRotationMatrix, vAdd[2] * vParticleSize));
						SetCol(&pColArray[i * 16 + 2 * 4], colParticleColor);

						SetPos(&pPosArray[i * lVtxQuadSize + 3 * lVtxStride], vPos + cMath::MatrixMul(mtxRotationMatrix, vAdd[3] * vParticleSize));
						SetCol(&pColArray[i * 16 + 3 * 4], colParticleColor);

					} else {
						//--

						SetPos(&pPosArray[i * lVtxQuadSize + 0 * lVtxStride], vPos + vAdd[0] * vParticleSize);
						SetCol(&pColArray[i * 16 + 0 * 4], colParticleColor);

						SetPos(&pPosArray[i * lVtxQuadSize + 1 * lVtxStride], vPos + vAdd[1] * vParticleSize);
						SetCol(&pColArray[i * 16 + 1 * 4], colParticleColor);

						SetPos(&pPosArray[i * lVtxQuadSize + 2 * lVtxStride], vPos + vAdd[2] * vParticleSize);
						SetCol(&pColArray[i * 16 + 2 * 4], colParticleColor);

						SetPos(&pPosArray[i * lVtxQuadSize + 3 * lVtxStride], vPos + vAdd[3] * vParticleSize);
						SetCol(&pColArray[i * 16 + 3 * 4], colParticleColor);
					}
				}
			}
			//////////////////////////////////////////////////
			// LINE
			else if (mDrawType == eParticleEmitter3DType_Line) {
				/*cVector3f vAdd[4] = {
					cVector3f(mvDrawSize.x, -mvDrawSize.y, 0),
					cVector3f(-mvDrawSize.x, -mvDrawSize.y, 0),
					cVector3f(-mvDrawSize.x, mvDrawSize.y, 0),
					cVector3f(mvDrawSize.x, mvDrawSize.y, 0)};*/

				int lVtxStride = kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)];
				int lVtxQuadSize = lVtxStride * 4;

				for (int i = 0; i < (int)mlNumOfParticles; i++) {
					cParticle *pParticle = mvParticles[i];

					// This is not the fastest thing possible...

					cVector3f vParticlePos1 = pParticle->mvPos;
					cVector3f vParticlePos2 = pParticle->mvLastPos;

					if (mCoordSystem == eParticleEmitter3DCoordSystem_Local) {
						vParticlePos1 = cMath::MatrixMul(mpParentSystem->GetWorldMatrix(), vParticlePos1);
						vParticlePos2 = cMath::MatrixMul(mpParentSystem->GetWorldMatrix(), vParticlePos2);
					}

					cVector3f vPos1 = cMath::MatrixMul(apCamera->GetViewMatrix(), vParticlePos1);
					cVector3f vPos2 = cMath::MatrixMul(apCamera->GetViewMatrix(), vParticlePos2);

					cVector3f vDirY;
					cVector2f vDirX;

					if (vPos1 == vPos2) {
						vDirY = cVector3f(0, 1, 0);
						vDirX = cVector2f(1, 0);
					} else {
						vDirY = vPos1 - vPos2;
						vDirY.Normalise();
						vDirX = cVector2f(vDirY.y, -vDirY.x);
						vDirX.Normalise();
					}

					vDirX = vDirX * mvDrawSize.x * pParticle->mvSize.x;
					vDirY = vDirY * mvDrawSize.y * pParticle->mvSize.y;

					SetPos(&pPosArray[i * lVtxQuadSize + 0 * lVtxStride], vPos2 + vDirY * -1 + vDirX);
					SetCol(&pColArray[i * 16 + 0 * 4], pParticle->mColor);

					SetPos(&pPosArray[i * lVtxQuadSize + 1 * lVtxStride], vPos2 + vDirY * -1 + vDirX * -1);
					SetCol(&pColArray[i * 16 + 1 * 4], pParticle->mColor);

					SetPos(&pPosArray[i * lVtxQuadSize + 2 * lVtxStride], vPos1 + vDirY + vDirX * -1);
					SetCol(&pColArray[i * 16 + 2 * 4], pParticle->mColor);

					SetPos(&pPosArray[i * lVtxQuadSize + 3 * lVtxStride], vPos1 + vDirY + vDirX);
					SetCol(&pColArray[i * 16 + 3 * 4], pParticle->mColor);
				}
			}
			//////////////////////////////////////////////////
			// AXIS
			else if (mDrawType == eParticleEmitter3DType_Axis) {
				if (mlAxisDrawUpdateCount != GetMatrixUpdateCount()) {
					mlAxisDrawUpdateCount = GetMatrixUpdateCount();
					cMatrixf mtxInv = cMath::MatrixInverse(GetWorldMatrix());
					mvRight = mtxInv.GetRight();
					mvForward = mtxInv.GetForward();
				}

				cVector3f vAdd[4];
				/*=
				{
					mvRight		 +	mvForward * 1,
					mvRight * -1 +	mvForward * 1,
					mvRight * -1 +	mvForward * -1,
					mvRight		 +	mvForward * -1
				};*/

				int lVtxStride = kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)];
				int lVtxQuadSize = lVtxStride * 4;

				for (int i = 0; i < (int)mlNumOfParticles; i++) {
					cParticle *pParticle = mvParticles[i];

					// This is not the fastest thing possible
					cVector3f vParticlePos = pParticle->mvPos;

					if (mCoordSystem == eParticleEmitter3DCoordSystem_Local) {
						vParticlePos = cMath::MatrixMul(mpParentSystem->GetWorldMatrix(), vParticlePos);
					}

					cVector3f vPos = vParticlePos; // cMath::MatrixMul(apCamera->GetViewMatrix(), vParticlePos);
					cVector2f &vSize = pParticle->mvSize;

					vAdd[0] = mvRight * vSize.x + mvForward * vSize.y;
					vAdd[1] = mvRight * -vSize.x + mvForward * vSize.y;
					vAdd[2] = mvRight * -vSize.x + mvForward * -vSize.y;
					vAdd[3] = mvRight * vSize.x + mvForward * -vSize.y;

					cColor &colParticleColor = pParticle->mColor;

					SetPos(&pPosArray[i * lVtxQuadSize + 0 * lVtxStride], vPos + vAdd[0]);
					SetCol(&pColArray[i * 16 + 0 * 4], colParticleColor);

					SetPos(&pPosArray[i * lVtxQuadSize + 1 * lVtxStride], vPos + vAdd[1]);
					SetCol(&pColArray[i * 16 + 1 * 4], colParticleColor);

					SetPos(&pPosArray[i * lVtxQuadSize + 2 * lVtxStride], vPos + vAdd[2]);
					SetCol(&pColArray[i * 16 + 2 * 4], colParticleColor);

					SetPos(&pPosArray[i * lVtxQuadSize + 3 * lVtxStride], vPos + vAdd[3]);
					SetCol(&pColArray[i * 16 + 3 * 4], colParticleColor);
				}
			}

			mpVtxBuffer->SetElementNum(mlNumOfParticles * 6);

			mbUpdateGfx = false;

			// Update the vertex buffer data

			if (mvSubDivUV.size() > 1)
				mpVtxBuffer->UpdateData(eVertexFlag_Position | eVertexFlag_Color0 | eVertexFlag_Texture0, false);
			else
				mpVtxBuffer->UpdateData(eVertexFlag_Position | eVertexFlag_Color0, false);
		}
	}
}

//-----------------------------------------------------------------------

iVertexBuffer *iParticleEmitter3D::GetVertexBuffer() {
	return mpVtxBuffer;
}

//-----------------------------------------------------------------------

bool iParticleEmitter3D::IsVisible() {
	if (IsActive() == false)
		return false;
	return IsRendered();
}

//-----------------------------------------------------------------------

cBoundingVolume *iParticleEmitter3D::GetBoundingVolume() {
	if (mbUpdateBV) {
		cVector3f vMin(100000, 100000, 100000);
		cVector3f vMax(-100000, -100000, -100000);

		for (int i = 0; i < (int)mlNumOfParticles; i++) {
			cParticle *pParticle = mvParticles[i];

			// X
			if (pParticle->mvPos.x < vMin.x)
				vMin.x = pParticle->mvPos.x;
			else if (pParticle->mvPos.x > vMax.x)
				vMax.x = pParticle->mvPos.x;

			// Y
			if (pParticle->mvPos.y < vMin.y)
				vMin.y = pParticle->mvPos.y;
			else if (pParticle->mvPos.y > vMax.y)
				vMax.y = pParticle->mvPos.y;

			// Z
			if (pParticle->mvPos.z < vMin.z)
				vMin.z = pParticle->mvPos.z;
			else if (pParticle->mvPos.z > vMax.z)
				vMax.z = pParticle->mvPos.z;
		}

		vMax += cVector3f(mvMaxDrawSize.x, mvMaxDrawSize.y, mvMaxDrawSize.x);
		vMin -= cVector3f(mvMaxDrawSize.x, mvMaxDrawSize.y, mvMaxDrawSize.x);

		mBoundingVolume.SetLocalMinMax(vMin, vMax);

		// Log("Min: (%f, %f, %f) Max: (%f, %f, %f)\n", vMin.x, vMin.y, vMin.z, vMax.x,vMax.y,vMax.z);

		if (mCoordSystem == eParticleEmitter3DCoordSystem_Local) {
			mBoundingVolume.SetTransform(mpParentSystem->GetWorldMatrix());
		}

		mbUpdateBV = false;
	}

	return &mBoundingVolume;
}

//-----------------------------------------------------------------------

cMatrixf *iParticleEmitter3D::GetModelMatrix(cCamera3D *apCamera) {
	if (apCamera) {
		if (mDrawType == eParticleEmitter3DType_Axis) {
			m_mtxTemp = cMatrixf::Identity;
		}
		// This is really not good...
		else if (mCoordSystem == eParticleEmitter3DCoordSystem_World) {
			m_mtxTemp = cMath::MatrixInverse(apCamera->GetViewMatrix());
		} else {
			m_mtxTemp = cMath::MatrixInverse(apCamera->GetViewMatrix());
			// m_mtxTemp = cMath::MatrixMul(cMath::MatrixInverse(apCamera->GetViewMatrix()),
			//								GetWorldMatrix());
		}

		// m_mtxTemp.SetTranslation(cVector3f(0,0,0));//GetWorldMatrix().GetTranslation());

		// m_mtxTemp = cMatrixf::Identity;

		// cMatrixf mtxCam = apCamera->GetViewMatrix();
		// Log("MATRIX: %s\n",mtxCam.ToString().c_str());

		return &m_mtxTemp;

	} else {
		return &GetWorldMatrix();
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cSaveData_iParticleEmitter3D, cSaveData_iRenderable)
	/*kSerializeVar(msDataName, eSerializeType_String)
	kSerializeVar(mvDataSize, eSerializeType_Vector3f)
	kSerializeVar(mlTime, eSerializeType_Int32)
	kSerializeVar(mbDying, eSerializeType_Bool)
	kSerializeVar(mfFrame, eSerializeType_Float32)*/
	kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveObject *cSaveData_iParticleEmitter3D::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	/*cWorld3D *pWorld = apGame->GetScene()->GetWorld3D();

	return pWorld->CreateParticleSystem(msName,msDataName,mvDataSize);*/
	return NULL;
}

//-----------------------------------------------------------------------

int cSaveData_iParticleEmitter3D::GetSaveCreatePrio() {
	return 3;
}

//-----------------------------------------------------------------------

iSaveData *iParticleEmitter3D::CreateSaveData() {
	// return hplNew( cSaveData_iParticleEmitter3D, () );
	return NULL;
}

//-----------------------------------------------------------------------

void iParticleEmitter3D::SaveToSaveData(iSaveData *apSaveData) {
	/*kSaveData_SaveToBegin(iParticleEmitter3D);

	kSaveData_SaveTo(msDataName);
	kSaveData_SaveTo(mvDataSize);
	kSaveData_SaveTo(mlTime);
	kSaveData_SaveTo(mbDying);
	kSaveData_SaveTo(mfFrame);*/
}

//-----------------------------------------------------------------------

void iParticleEmitter3D::LoadFromSaveData(iSaveData *apSaveData) {
	/*kSaveData_LoadFromBegin(iParticleEmitter3D);

	kSaveData_LoadFrom(mlTime);
	kSaveData_LoadFrom(mbDying);
	kSaveData_LoadFrom(mfFrame);*/
}

//-----------------------------------------------------------------------

void iParticleEmitter3D::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	// kSaveData_SetupBegin(iParticleEmitter3D);
}

//-----------------------------------------------------------------------
} // namespace hpl
