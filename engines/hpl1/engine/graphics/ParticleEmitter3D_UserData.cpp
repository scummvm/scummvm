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

// NEW
#include "hpl1/engine/graphics/Graphics.h"
// ---
#include "hpl1/engine/graphics/ParticleEmitter3D_UserData.h"
//#include "hpl1/engine/graphics/Mesh.h"
//#include "hpl1/engine/graphics/SubMesh.h"
#include "hpl1/engine/graphics/ParticleSystem3D.h"
#include "hpl1/engine/graphics/RenderList.h"

#include "hpl1/engine/impl/tinyXML/tinyxml.h"

#include "hpl1/engine/resources/MaterialManager.h"
#include "hpl1/engine/resources/Resources.h"
//#include "hpl1/engine/resources/MeshManager.h"

#include "hpl1/engine/physics/PhysicsBody.h"
#include "hpl1/engine/physics/PhysicsWorld.h"

#include "hpl1/engine/scene/World3D.h"

#include "hpl1/engine/math/Math.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cParticleEmitterData3D_UserData::cParticleEmitterData3D_UserData(
	const tString &asName, cResources *apResources, cGraphics *apGraphics)
	: iParticleEmitterData(asName, apResources, apGraphics) {
}

//-----------------------------------------------------------------------

iParticleEmitter *cParticleEmitterData3D_UserData::Create(tString asName, cVector3f avSize) {
	iParticleEmitter3D *pPE = hplNew(cParticleEmitter3D_UserData, (asName, &mvMaterials, avSize,
																   mpGraphics, mpResources, this));

	cMatrixf mtxOffset = cMath::MatrixRotate(mvAngleOffset, eEulerRotationOrder_XYZ);
	mtxOffset.SetTranslation(mvPosOffset);
	pPE->SetMatrix(mtxOffset);

	return pPE;
}

//////////////////////////

// NEW
static ePEType GetPEType(const char *apString) {
	if (apString == NULL)
		return ePEType_Normal;
	tString sType = cString::ToLowerCase(apString);

	if (sType == "beam")
		return ePEType_Beam;
	else if (sType == "normal")
		return ePEType_Normal;

	return ePEType_Normal;
}

// ---

/////////////////////////

static ePEStartPosType GetStartPosType(const char *apString) {
	if (apString == NULL)
		return ePEStartPosType_Box;

	tString sType = cString::ToLowerCase(apString);

	if (sType == "box")
		return ePEStartPosType_Box;
	else if (sType == "sphere")
		return ePEStartPosType_Sphere;

	return ePEStartPosType_Box;
}

/////////////////////////

static eParticleEmitter3DCoordSystem GetCoordSystem(const char *apString) {
	if (apString == NULL)
		return eParticleEmitter3DCoordSystem_World;

	tString sType = cString::ToLowerCase(apString);

	if (sType == "world")
		return eParticleEmitter3DCoordSystem_World;
	else if (sType == "local")
		return eParticleEmitter3DCoordSystem_Local;

	return eParticleEmitter3DCoordSystem_World;
}

/////////////////////////

static ePEDeathType GetDeathType(const char *apString) {
	if (apString == NULL)
		return ePEDeathType_Age;

	tString sType = cString::ToLowerCase(apString);

	if (sType == "age")
		return ePEDeathType_Age;

	return ePEDeathType_Age;
}

/////////////////////////

static eParticleEmitter3DType GetDrawType(const char *apString) {
	if (apString == NULL)
		return eParticleEmitter3DType_DynamicPoint;

	tString sType = cString::ToLowerCase(apString);

	if (sType == "point")
		return eParticleEmitter3DType_DynamicPoint;
	else if (sType == "line")
		return eParticleEmitter3DType_Line;
	else if (sType == "axis")
		return eParticleEmitter3DType_Axis;

	return eParticleEmitter3DType_DynamicPoint;
}

/////////////////////////

static ePEGravityType GetGravityType(const char *apString) {
	if (apString == NULL)
		return ePEGravityType_None;

	tString sType = cString::ToLowerCase(apString);

	if (sType == "none")
		return ePEGravityType_None;
	else if (sType == "vector")
		return ePEGravityType_Vector;
	else if (sType == "center")
		return ePEGravityType_Center;

	return ePEGravityType_None;
}

/////////////////////////

static ePESubDivType GetSubDivType(const char *apString) {
	if (apString == NULL)
		return ePESubDivType_Random;

	tString sType = cString::ToLowerCase(apString);

	if (sType == "random")
		return ePESubDivType_Random;
	else if (sType == "animation")
		return ePESubDivType_Animation;

	return ePESubDivType_Random;
}

////////////////////////
/// NEW

static ePEPartSpinType GetPartSpinType(const char *apString) {
	if (apString == NULL)
		return ePEPartSpinType_Constant;

	tString sType = cString::ToLowerCase(apString);

	if (sType == "constant")
		return ePEPartSpinType_Constant;
	else if (sType == "movement")
		return ePEPartSpinType_Movement;

	return ePEPartSpinType_Constant;
}

/// ---

/////////////////////////

void cParticleEmitterData3D_UserData::LoadFromElement(TiXmlElement *apElement) {
	///////// GENERAL /////////////
	msName = cString::ToString(apElement->Attribute("Name"), "");

	// NEW

	mPEType = GetPEType(apElement->Attribute("PEType"));

	// ---

	mlMaxParticleNum = cString::ToInt(apElement->Attribute("MaxParticleNum"), 1);

	mbRespawn = cString::ToBool(apElement->Attribute("Respawn"), false);

	mfParticlesPerSecond = cString::ToFloat(apElement->Attribute("ParticlesPerSecond"), 1);
	mfStartTimeOffset = cString::ToFloat(apElement->Attribute("StartTimeOffset"), 0);

	mfWarmUpTime = cString::ToFloat(apElement->Attribute("WarmUpTime"), 0);
	mfWarmUpStepsPerSec = cString::ToFloat(apElement->Attribute("WarmUpStepsPerSec"), 60);

	mfMinPauseLength = cString::ToFloat(apElement->Attribute("MinPauseLength"), 0);
	mfMaxPauseLength = cString::ToFloat(apElement->Attribute("MaxPauseLength"), 0);

	mfMinPauseInterval = cString::ToFloat(apElement->Attribute("MinPauseInterval"), 0);
	mfMaxPauseInterval = cString::ToFloat(apElement->Attribute("MaxPauseInterval"), 0);

	mvPosOffset = cString::ToVector3f(apElement->Attribute("PosOffset"), 0);
	mvAngleOffset = cString::ToVector3f(apElement->Attribute("AngleOffset"), 0);
	mvAngleOffset.x = cMath::ToRad(mvAngleOffset.x);
	mvAngleOffset.y = cMath::ToRad(mvAngleOffset.y);
	mvAngleOffset.z = cMath::ToRad(mvAngleOffset.z);

	/////////// MATERIAL  ////////
	int lMaterialNum = cString::ToInt(apElement->Attribute("MaterialNum"), 1);
	float fAnimationLength = cString::ToFloat(apElement->Attribute("AnimationLength"), 1);
	tString sMaterial = cString::ToString(apElement->Attribute("Material"), "");

	mvSubDiv = cString::ToVector2l(apElement->Attribute("SubDiv"), 1);
	mSubDivType = GetSubDivType(apElement->Attribute("SubDivType"));

	if (lMaterialNum <= 1) {
		sMaterial = cString::SetFileExt(sMaterial, "mat");
		iMaterial *pMaterial = mpResources->GetMaterialManager()->CreateMaterial(sMaterial);

		if (pMaterial)
			mvMaterials.push_back(pMaterial);
	} else {
		for (int i = 1; i < lMaterialNum + 1; ++i) {
			tString sFileName;
			if (i > 9)
				sFileName = sMaterial + cString::ToString(i);
			else
				sFileName = sMaterial + "0" + cString::ToString(i);

			sFileName = cString::SetFileExt(sFileName, "mat");

			iMaterial *pMaterial = mpResources->GetMaterialManager()->CreateMaterial(sFileName);
			if (pMaterial)
				mvMaterials.push_back(pMaterial);
		}
	}

	mfFrameStep = 1 / (fAnimationLength / (float)mvMaterials.size());
	mfMaxFrameTime = ((float)mvMaterials.size()) - 0.0001f;

	///////// START POS //////////
	mStartPosType = GetStartPosType(apElement->Attribute("StartPosType"));

	mvMinStartPos = cString::ToVector3f(apElement->Attribute("MinStartPos"), 0);
	mvMaxStartPos = cString::ToVector3f(apElement->Attribute("MaxStartPos"), 0);

	mvMinStartAngles = cString::ToVector2f(apElement->Attribute("MinStartAngles"), 0);
	mvMaxStartAngles = cString::ToVector2f(apElement->Attribute("MaxStartAngles"), 0);
	mvMinStartAngles.x = cMath::ToRad(mvMinStartAngles.x);
	mvMinStartAngles.y = cMath::ToRad(mvMinStartAngles.y);
	mvMaxStartAngles.x = cMath::ToRad(mvMaxStartAngles.x);
	mvMaxStartAngles.y = cMath::ToRad(mvMaxStartAngles.y);

	mfMinStartRadius = cString::ToFloat(apElement->Attribute("MinStartRadius"), 0);
	mfMaxStartRadius = cString::ToFloat(apElement->Attribute("MaxStartRadius"), 0);

	// NEW
	//		cMesh *mshTempMesh = mpResources->GetMeshManager()->CreateMesh("character_roach.dae");
	//		mVBMeshData = mshTempMesh->GetSubMeshName("polySurfaceShape38")->GetVertexBuffer();
	//		mpfMeshVtxData = mVBMeshData->GetArray(eVertexFlag_Position);

	// ---

	/////////// MOVEMENT ////////
	mStartVelType = GetStartPosType(apElement->Attribute("StartVelType"));

	mvMinStartVel = cString::ToVector3f(apElement->Attribute("MinStartVel"), 0);
	mvMaxStartVel = cString::ToVector3f(apElement->Attribute("MaxStartVel"), 0);

	mvMinStartVelAngles = cString::ToVector2f(apElement->Attribute("MinStartVelAngles"), 0);
	mvMaxStartVelAngles = cString::ToVector2f(apElement->Attribute("MaxStartVelAngles"), 0);
	mvMinStartVelAngles.x = cMath::ToRad(mvMinStartVelAngles.x);
	mvMinStartVelAngles.y = cMath::ToRad(mvMinStartVelAngles.y);
	mvMaxStartVelAngles.x = cMath::ToRad(mvMaxStartVelAngles.x);
	mvMaxStartVelAngles.y = cMath::ToRad(mvMaxStartVelAngles.y);

	mfMinStartVelSpeed = cString::ToFloat(apElement->Attribute("MinStartVelSpeed"), 0);
	mfMaxStartVelSpeed = cString::ToFloat(apElement->Attribute("MaxStartVelSpeed"), 0);

	mfMinSpeedMultiply = cString::ToFloat(apElement->Attribute("MinSpeedMultiply"), 0);
	mfMaxSpeedMultiply = cString::ToFloat(apElement->Attribute("MaxSpeedMultiply"), 0);

	mvMinStartAcc = cString::ToVector3f(apElement->Attribute("MinStartAcc"), 0);
	mvMaxStartAcc = cString::ToVector3f(apElement->Attribute("MaxStartAcc"), 0);

	mfMinVelMaximum = cString::ToFloat(apElement->Attribute("MinVelMaximum"), 0);
	mfMaxVelMaximum = cString::ToFloat(apElement->Attribute("MaxVelMaximum"), 0);

	mbUsesDirection = cString::ToBool(apElement->Attribute("UsesDirection"), false);

	mGravityType = GetGravityType(apElement->Attribute("GravityType"));

	mvGravityAcc = cString::ToVector3f(apElement->Attribute("GravityAcc"), 0);

	mCoordSystem = GetCoordSystem(apElement->Attribute("CoordSystem"));

	// NEW

	mbUsePartSpin = cString::ToBool(apElement->Attribute("UsePartSpin"), false);
	mPartSpinType = GetPartSpinType(apElement->Attribute("PartSpinType"));
	mfMinSpinRange = cString::ToFloat(apElement->Attribute("MinSpinRange"), 0);
	mfMaxSpinRange = cString::ToFloat(apElement->Attribute("MaxSpinRange"), 0);

	mbUseRevolution = cString::ToBool(apElement->Attribute("UseRevolution"), false);
	mvMinRevVel = cString::ToVector3f(apElement->Attribute("MinRevVel"), 0);
	mvMaxRevVel = cString::ToVector3f(apElement->Attribute("MaxRevVel"), 0);

	// ---

	/////////// LIFESPAN ////////
	mfMinLifeSpan = cString::ToFloat(apElement->Attribute("MinLifeSpan"), 0);
	mfMaxLifeSpan = cString::ToFloat(apElement->Attribute("MaxLifeSpan"), 0);

	mDeathType = GetDeathType(apElement->Attribute("DeathType"));

	msDeathPS = cString::ToString(apElement->Attribute("DeathPS"), "");

	/////////// RENDERING ////////
	mDrawType = GetDrawType(apElement->Attribute("DrawType"));

	// eParticleEmitter3DType Heading
	// cVector2l subdivisions.

	mvMinStartSize = cString::ToVector2f(apElement->Attribute("MinStartSize"), 1);
	mvMaxStartSize = cString::ToVector2f(apElement->Attribute("MaxStartSize"), 1);

	mfStartRelSize = cString::ToFloat(apElement->Attribute("StartRelSize"), 0);
	mfMiddleRelSize = cString::ToFloat(apElement->Attribute("MiddleRelSize"), 0);
	mfMiddleRelSizeTime = cString::ToFloat(apElement->Attribute("MiddleRelSizeTime"), 0);
	mfMiddleRelSizeLength = cString::ToFloat(apElement->Attribute("MiddleRelSizeLength"), 0);
	mfEndRelSize = cString::ToFloat(apElement->Attribute("EndRelSize"), 0);

	mbMultiplyRGBWithAlpha = cString::ToBool(apElement->Attribute("MultiplyRGBWithAlpha"), false);

	/////////// COLOR  ////////

	mMinStartColor = cString::ToColor(apElement->Attribute("MinStartColor"), cColor(1, 1));
	mMaxStartColor = cString::ToColor(apElement->Attribute("MaxStartColor"), cColor(1, 1));

	mStartRelColor = cString::ToColor(apElement->Attribute("StartRelColor"), cColor(1, 1));
	mMiddleRelColor = cString::ToColor(apElement->Attribute("MiddleRelColor"), cColor(1, 1));
	mfMiddleRelColorTime = cString::ToFloat(apElement->Attribute("MiddleRelColorTime"), 0);
	mfMiddleRelColorLength = cString::ToFloat(apElement->Attribute("MiddleRelColorLength"), 0);
	mEndRelColor = cString::ToColor(apElement->Attribute("EndRelColor"), cColor(1, 1));

	/////////// COLLISION  ////////
	mbCollides = cString::ToBool(apElement->Attribute("Collides"), false);

	mfMinBounceAmount = cString::ToFloat(apElement->Attribute("MinBounceAmount"), 0);
	mfMaxBounceAmount = cString::ToFloat(apElement->Attribute("MaxBounceAmount"), 0);

	mlMinCollisionMax = cString::ToInt(apElement->Attribute("MinCollisionMax"), 0);
	mlMaxCollisionMax = cString::ToInt(apElement->Attribute("MaxCollisionMax"), 0);

	mlCollisionUpdateRate = cString::ToInt(apElement->Attribute("CollisionUpdateRate"), 0);

	// NEW
	////////// BEAM SPECIFIC //////////

	mbUseBeamNoise = cString::ToBool(apElement->Attribute("UseBeamNoise"), false);

	mlLowFreqPoints = cString::ToInt(apElement->Attribute("LowFreqPoints"), 4);
	mlHighFreqPoints = cString::ToInt(apElement->Attribute("HighFreqPoints"), 5);

	mvMinLowFreqNoise = cString::ToVector3f(apElement->Attribute("MinLowFreqNoise"), 0);
	mvMaxLowFreqNoise = cString::ToVector3f(apElement->Attribute("MaxLowFreqNoise"), 0);

	mvMinHighFreqNoise = cString::ToVector3f(apElement->Attribute("MinHighFreqNoise"), 0);
	mvMaxHighFreqNoise = cString::ToVector3f(apElement->Attribute("MaxHighFreqNoise"), 0);

	// ---
}

//-----------------------------------------------------------------------

bool cParticleEmitterData3D_UserData::OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams) {
	if (pBody->IsActive() == false || pBody->GetCollide() == false || pBody->IsCharacter()) {
		return true;
	}

	if (mfShortestDist > apParams->mfDist) {
		mbIntersected = true;
		mfShortestDist = apParams->mfDist;
		*mpIntersectPos = apParams->mvPoint;
		*mpIntersectNormal = apParams->mvNormal;
	}

	return true;
}

bool cParticleEmitterData3D_UserData::CheckCollision(const cVector3f &avStart,
													 const cVector3f &avEnd,
													 iPhysicsWorld *apPhysicsWorld,
													 cVector3f *apNormalVec,
													 cVector3f *apPosVec) {
	mfShortestDist = 99999.0f;
	mbIntersected = false;

	mpIntersectPos = apPosVec;
	mpIntersectNormal = apNormalVec;

	apPhysicsWorld->CastRay(this, avStart, avEnd, true, true, true);

	return mbIntersected;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cParticleEmitter3D_UserData::cParticleEmitter3D_UserData(
	tString asName, tMaterialVec *apMaterials,
	cVector3f avSize, cGraphics *apGraphics, cResources *apResources,
	cParticleEmitterData3D_UserData *apData)
	: iParticleEmitter3D(asName, apMaterials, apData->mlMaxParticleNum,
						 avSize, apGraphics, apResources) {
	mpData = apData;

	mvDrawSize = 1;

	mfTime = 0;

	mfCreateCount = 0;

	mfPauseCount = 0;
	mfPauseWaitCount = 0;
	mbPaused = false;

	mfCollideCount = 1.0f / (float)mpData->mlCollisionUpdateRate;

	mCoordSystem = apData->mCoordSystem;

	mDrawType = apData->mDrawType;

	mbUsesDirection = apData->mbUsesDirection;

	// mbDying = !apData->mbRespawn;
	mbRespawn = apData->mbRespawn;

	SetSubDivUV(mpData->mvSubDiv);

	// NEW
	mbUsePartSpin = apData->mbUsePartSpin;
	mbUseRevolution = apData->mbUseRevolution;

	mPEType = apData->mPEType;

	// Calculate max size of particles.
	float fSizeMul = apData->mfStartRelSize;
	if (fSizeMul < apData->mfMiddleRelSize)
		fSizeMul = apData->mfMiddleRelSize;
	if (fSizeMul < apData->mfEndRelSize)
		fSizeMul = apData->mfEndRelSize;
	mvMaxDrawSize = apData->mvMaxStartSize * fSizeMul;

	/*		int i = 0;
			int j = 0;
			int k = 0;
			switch ( mPEType )
			{
				case ePEType_Beam:
					hplDelete(mpVtxBuffer);								// Destroy the current Vertex Buffer, as it's filled with quads we don't need now :)

					if ( apData->mbUseBeamNoise )
					{
						// Calculate the Frequency Points Lookup Table, this is needed to create the Vertex Buffer.It is also needed by every beam particle created,
						// as it needs to know the type and index of each when updating.
						while ( (i < apData->mlLowFreqPoints)  && (j < apData->mlHighFreqPoints) )
						{
							float fLowRelPos = (float) i / (apData->mlLowFreqPoints -1 );
							float fHighRelPos = (float) j / (apData->mlHighFreqPoints -1 );
							float fLastLowRelPos = 0.0f;

							tBeamNoisePoint tempBNP;

				//			tempBNP.lLowFreqNoiseIdx = i;
				//			tempBNP.lHighFreqNoiseIdx = j;
							tempBNP.fRelToBendPos = (fHighRelPos-fLastLowRelPos)/(fLowRelPos-fLastLowRelPos);

							if ( fLowRelPos <= fHighRelPos )
							{
								if ( fLowRelPos == fHighRelPos )
								{
									tempBNP.noiseType = ePENoiseType_Both;
									++i;
									++j;
								}
								else
								{
									tempBNP.noiseType = ePENoiseType_LowFreq;
									++i;
								}

								tempBNP.fRelToBeamPos = fLowRelPos;
								apData->mvLFIndices.push_back (k);

							}
							else
							{
								tempBNP.fRelToBeamPos = fHighRelPos;
								tempBNP.noiseType = ePENoiseType_HighFreq;
								++j;
							}

							fLastLowRelPos = fLowRelPos;


							mpData->mvBeamNoisePoints.push_back(tempBNP);
							++k;
						}
					}
					else
					{
						tBeamNoisePoint tempBNP;

						tempBNP.fRelToBeamPos = 0.0f;
						tempBNP.fRelToBendPos = 0.0f;
						tempBNP.noiseType = ePENoiseType_None;

						mpData->mvBeamNoisePoints.push_back(tempBNP);

						tempBNP.fRelToBeamPos = 1.0f;
						tempBNP.fRelToBendPos = 1.0f;
						tempBNP.noiseType = ePENoiseType_None;

						mpData->mvBeamNoisePoints.push_back(tempBNP);
					}

					mpVtxBuffer = apGraphics->GetLowLevel()->CreateVertexBuffer ( eVertexFlag_Position,		// Now that we know how many points we have, we create the Vertex Buffer
							eVertexBufferDrawType_Tri, eVertexBufferUsageType_Stream,
							mlMaxParticles*mpData->mvBeamNoisePoints.size()*2, mlMaxParticles*(mpData->mvBeamNoisePoints.size()-1)*6);

					for ( int i = 0; i < (int) mlMaxParticles; ++i )
					{
						for ( int j = 0; j < (int) mpData->mvBeamNoisePoints.size()-1; ++j )
						{
							int lStart = j*2;
							mpVtxBuffer->AddIndex(lStart);		// Half quad ( tri )
							mpVtxBuffer->AddIndex(lStart+1);
							mpVtxBuffer->AddIndex(lStart+3);

							mpVtxBuffer->AddIndex(lStart+3);	// The other half
							mpVtxBuffer->AddIndex(lStart+2);
							mpVtxBuffer->AddIndex(lStart);
						}

					}

					for ( int i = 0; i < (int) mlMaxParticles; ++i )
					{
						for (int j = 0; j < (int) mpData->mvBeamNoisePoints.size(); ++j )
						{
							mpVtxBuffer->AddVertex(eVertexFlag_Texture0, cVector2f(mpData->mvBeamNoisePoints[j].fRelToBeamPos,1));
							mpVtxBuffer->AddVertex(eVertexFlag_Texture0, cVector2f(mpData->mvBeamNoisePoints[j].fRelToBeamPos,0));
						}
					}

					for ( int i = 0; i < (int) mlMaxParticles; ++i )
					{
						for (int j = 0; j < (int) mpData->mvBeamNoisePoints.size()*2; ++j )
						{
							mpVtxBuffer->AddVertex(eVertexFlag_Position, 0);
							mpVtxBuffer->AddColor(eVertexFlag_Color0, cColor(1,1));
						}
					}

					mpVtxBuffer->Compile(0);





					break;
				case ePEType_Normal:
					break;
				default:
					break;
				}
	*/
	// ---
}

//-----------------------------------------------------------------------

cParticleEmitter3D_UserData::~cParticleEmitter3D_UserData() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cParticleEmitter3D_UserData::IsDying() {
	if (mbRespawn == false || mbDying)
		return true;

	return false;
}

//-----------------------------------------------------------------------

void cParticleEmitter3D_UserData::Kill() {
	mbRespawn = false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cParticleEmitter3D_UserData::SetParticleDefaults(cParticle *apParticle) {
	///////////////////////////////////
	// Start Color
	apParticle->mStartColor = cMath::RandRectColor(mpData->mMinStartColor, mpData->mMaxStartColor);
	apParticle->mColor = apParticle->mStartColor * mpData->mStartRelColor;

	///////////////////////////////////
	// Start Size
	if (mpData->mvMinStartSize.y == 0 && mpData->mvMaxStartSize.y == 0)
		apParticle->mvStartSize = cMath::RandRectf(mpData->mvMinStartSize.x, mpData->mvMaxStartSize.x);
	else
		apParticle->mvStartSize = cMath::RandRectVector2f(mpData->mvMinStartSize, mpData->mvMaxStartSize);
	apParticle->mvSize = apParticle->mvStartSize * mpData->mfStartRelSize;

	////////////////////////////////////
	// Start sub division
	if (mvSubDivUV.size() > 1) {
		if (mpData->mSubDivType == ePESubDivType_Animation) {
			apParticle->mlSubDivNum = 0;
		} else {
			apParticle->mlSubDivNum = cMath::RandRectl(0, (int)mvSubDivUV.size() - 1);
		}
	}

	////////////////////////////////////
	// Start collision
	apParticle->mfBounceAmount = cMath::RandRectf(mpData->mfMinBounceAmount, mpData->mfMaxBounceAmount);
	apParticle->mlBounceCount = cMath::RandRectl(mpData->mlMinCollisionMax, mpData->mlMaxCollisionMax);

	////////////////////////////////////
	// Start Position

	// Matrix to start from
	cMatrixf mtxStart;
	if (mpData->mCoordSystem == eParticleEmitter3DCoordSystem_World) {
		mtxStart = GetWorldMatrix();
	} else {
		// Not what should be here, perhaps only identity.
		mtxStart = GetLocalMatrix();
	}

	// Sphere or box start
	if (mpData->mStartPosType == ePEStartPosType_Box) {
		apParticle->mvPos = mtxStart.GetTranslation() +
							cMath::RandRectVector3f(mpData->mvMinStartPos, mpData->mvMaxStartPos);
	} else if (mpData->mStartPosType == ePEStartPosType_Sphere) {
		cVector2f vRot = cMath::RandRectVector2f(mpData->mvMinStartAngles,
												 mpData->mvMaxStartAngles);
		cMatrixf mtxRot = cMath::MatrixRotate(vRot, eEulerRotationOrder_XYZ);
		cVector3f vPos = cVector3f(0, cMath::RandRectf(mpData->mfMinStartRadius, mpData->mfMaxStartRadius), 0);

		apParticle->mvPos = mtxStart.GetTranslation() + cMath::MatrixMul(mtxRot, vPos);
	}

	// NEW

	//		if (mpData->mVBMeshData)
	//		{
	/*
				cVector3f vTrans = mtxStart.GetTranslation();
				posI = cMath::RandRectl(0,mpData->mVBMeshData->GetVertexNum()-1);
				vTrans.x += mpData->mpfMeshVtxData[posI*4];
				vTrans.y += mpData->mpfMeshVtxData[posI*4 + 1];
				vTrans.z += mpData->mpfMeshVtxData[posI*4 + 2];

				apParticle->mvPos = vTrans;
				*/
	//			apParticle->mvPos = mtxStart.GetTranslation() + mpData->mVBMeshData->GetVector3( eVertexFlag_Position, cMath::RandRectl(0,mpData->mVBMeshData->GetVertexNum()-1) );
	//			++posI;
	//		if ( posI == mpData->mVBMeshData->GetVertexNum() )
	//			posI = 0;
	//		}

	// ---

	apParticle->mvLastPos = apParticle->mvPos;
	apParticle->mvLastCollidePos = apParticle->mvPos;

	////////////////////////////////////
	// Start Velocity

	// Sphere or box start
	if (mpData->mStartVelType == ePEStartPosType_Box) {
		apParticle->mvVel = cMath::RandRectVector3f(mpData->mvMinStartVel, mpData->mvMaxStartVel);
	} else if (mpData->mStartVelType == ePEStartPosType_Sphere) {
		cVector2f vRot = cMath::RandRectVector2f(mpData->mvMinStartVelAngles,
												 mpData->mvMaxStartVelAngles);
		cMatrixf mtxRot = cMath::MatrixRotate(vRot, eEulerRotationOrder_XYZ);
		cVector3f vPos = cVector3f(0, cMath::RandRectf(mpData->mfMinStartVelSpeed, mpData->mfMaxStartVelSpeed), 0);

		apParticle->mvVel = cMath::MatrixMul(mtxRot, vPos);
	}

	// If it uses the direction,
	if (mpData->mbUsesDirection && mpData->mCoordSystem == eParticleEmitter3DCoordSystem_World) {
		apParticle->mvVel = cMath::MatrixMul(mtxStart.GetRotation(), apParticle->mvVel);
	}

	apParticle->mfMaxSpeed = cMath::RandRectf(mpData->mfMinVelMaximum, mpData->mfMaxVelMaximum);

	apParticle->mfSpeedMul = cMath::RandRectf(mpData->mfMinSpeedMultiply, mpData->mfMaxSpeedMultiply);

	////////////////////////////////////
	// Start Acceleration
	apParticle->mvAcc = cMath::RandRectVector3f(mpData->mvMinStartAcc, mpData->mvMaxStartAcc);

	// NEW
	////////////////////////////////////
	// Start Spin Velocity
	if (mpData->mPartSpinType == ePEPartSpinType_Constant) {
		apParticle->mfSpinVel = cMath::RandRectf(mpData->mfMinSpinRange, mpData->mfMaxSpinRange);
	} else if (mpData->mPartSpinType == ePEPartSpinType_Movement) {
		apParticle->mfSpinFactor = cMath::RandRectf(mpData->mfMinSpinRange, mpData->mfMaxSpinRange);
		apParticle->mfSpinVel = 0.0f;
	}
	apParticle->mfSpin = cMath::RandRectf(0.0f, k2Pif);

	////////////////////////////////////
	// Start Revolution Velocity
	apParticle->mvRevolutionVel = cMath::RandRectVector3f(mpData->mvMinRevVel, mpData->mvMaxRevVel);

	// ---

	///////////////////////////////////
	// Life Span
	apParticle->mfStartLife = cMath::RandRectf(mpData->mfMinLifeSpan, mpData->mfMaxLifeSpan);
	apParticle->mfLife = apParticle->mfStartLife;

	apParticle->mfLifeSize_MiddleStart = apParticle->mfLife * (1 - mpData->mfMiddleRelSizeTime);
	apParticle->mfLifeSize_MiddleEnd = apParticle->mfLife * (1 - (mpData->mfMiddleRelSizeTime +
																  mpData->mfMiddleRelSizeLength));

	apParticle->mfLifeColor_MiddleStart = apParticle->mfLife * (1 - mpData->mfMiddleRelColorTime);
	apParticle->mfLifeColor_MiddleEnd = apParticle->mfLife * (1 - (mpData->mfMiddleRelColorTime +
																   mpData->mfMiddleRelColorLength));

	/*Log("Created particle with Pos: (%s) Color: (%s) Size (%s) Vel: (%s) Acc: (%s) Life: %f SizeMiddleStart: %f SizeMiddleEnd: %f\n",
				apParticle->mvPos.ToString().c_str(),
				apParticle->mColor.ToString().c_str(),
				apParticle->mvSize.ToString().c_str(),
				apParticle->mvVel.ToString().c_str(),
				apParticle->mvAcc.ToString().c_str(),
				apParticle->mfLife,
				apParticle->mfLifeSize_MiddleStart,
				apParticle->mfLifeSize_MiddleEnd);*/
	// NEW
	/////////////////////////////////////
	// Beam Specific
	if (mPEType == ePEType_Beam) {
		/*
		if (apParticle->mvBeamPoints.size() == 0 )
		{
			for (int i = 0; i < (int) mpData->mvBeamNoisePoints.size(); ++i)
				apParticle->mvBeamPoints.push_back(cVector(0.0f));
		}


		int j = 1;
		cVector3f vLFPointPos = apParticle->mvPos + apParticle->mvVel * mpData->mvBeamNoisePoints.at(mpData->mvLFIndices[j]).fRelToBeamPos;
		cVector3f vLastLFPointPos = apParticle->mvPos;
		cVector3f vDir;
		for ( int i = 1; i < (int) mpData->mvBeamNoisePoints.size(); ++i )
		{
			apParticle->mvBeamPoints[i] = vLastLFPointPos + vDir * mpData->mvBeamNoisePoints[i].fRelToBendPos;

			switch (mpData->mvBeamNoisePoints[i].noiseType)
			{

			case ePENoiseType_HighFreq:
				apParticle->mvBeamPoints[i] += cMath::RandRectVector3f(mpData->mvMinHighFreqNoise, mpData->mvMaxHighFreqNoise);
				break;
			case ePENoiseType_LowFreq:
				++j;
				vLastLFPointPos = vLFPointPos;
				vLFPointPos = apParticle->mvPos + apParticle->mvVel * mpData->mvBeamNoisePoints.at(mpData->mvLFIndices[j]).fRelToBeamPos + cMath::RandRectVector3f(mpData->mvMinLowFreqNoise, mpData->mvMaxLowFreqNoise);
				vDir = vLFPointPos - vLastLFPointPos;
				break;
			case ePENoiseType_Both:
				apParticle->mvBeamPoints[i] += cMath::RandRectVector3f(mpData->mvMinHighFreqNoise, mpData->mvMaxHighFreqNoise);
				++j;
				vLastLFPointPos = vLFPointPos;
				vLFPointPos = apParticle->mvPos + apParticle->mvVel * mpData->mvBeamNoisePoints.at(mpData->mvLFIndices[j]).fRelToBeamPos + cMath::RandRectVector3f(mpData->mvMinLowFreqNoise, mpData->mvMaxLowFreqNoise);
				vDir = vLFPointPos - vLastLFPointPos;
				break;
			default:
				break;
			}
		}
		*/
	}

	// ---
}

//-----------------------------------------------------------------------

void cParticleEmitter3D_UserData::UpdateMotion(float afTimeStep) {
	///////////////////////////////////////////////
	// If not rendered this previous frame check if it should be paused.
	// To be safe skip systems that has a parent...
	if (mbDying == false && mbRespawn && mpParentSystem->GetParent() == NULL) {
		if (GetGlobalRenderCount() != cRenderList::GetGlobalRenderCount()) {
			if (mlUpdateCount > 120) {
				return;
			}
			mlUpdateCount++;
		} else {
			mlUpdateCount = 0;
		}
	}

	///////////////////////////////////////////
	// Check if the emitter is ready to start.
	if (mfTime < mpData->mfStartTimeOffset) {
		mfTime += afTimeStep;
		return;
	}

	///////////////////////////////////////////
	// Delay check
	if (mbPaused) {
		mfPauseCount -= afTimeStep;
		if (mfPauseCount <= 0) {
			mbPaused = false;

			mfPauseWaitCount = cMath::RandRectf(mpData->mfMinPauseInterval, mpData->mfMaxPauseInterval);
		}
	} else if (mpData->mfMinPauseLength > 0 && mpData->mfMinPauseInterval > 0) {
		mfPauseWaitCount -= afTimeStep;
		if (mfPauseWaitCount <= 0) {
			mfPauseCount = cMath::RandRectf(mpData->mfMinPauseLength, mpData->mfMaxPauseLength);
			mbPaused = true;
		}
	}

	///////////////////////////////////////////
	// Particle creation
	if (mbPaused == false && mlNumOfParticles < mlMaxParticles) {
		mfCreateCount += mpData->mfParticlesPerSecond * afTimeStep;

		while (mfCreateCount >= 0.99999f && (mlNumOfParticles < mlMaxParticles)) {
			SetParticleDefaults(CreateParticle());
			mfCreateCount -= 1.0f;
		}
	}

	///////////////////////////////////////////
	// Collision Update
	bool bColliding = false;
	if (mpData->mbCollides) {
		mfCollideCount -= afTimeStep;
		if (mfCollideCount <= 0) {
			mfCollideCount = 1.0f / (float)mpData->mlCollisionUpdateRate;
			bColliding = true;
		}
	}

	///////////////////////////////////////////
	// Particle update
	for (unsigned int i = 0; i < mlNumOfParticles; ++i) {
		cParticle *pParticle = mvParticles[i];

		////////////
		// Position Update
		pParticle->mvLastPos = pParticle->mvPos;

		pParticle->mvPos += pParticle->mvVel * afTimeStep;

		////////////
		// Speed Update
		pParticle->mvVel += pParticle->mvAcc * afTimeStep;

		// gravity
		switch (mpData->mGravityType) {
		case ePEGravityType_None: {
			break;
		}
		case ePEGravityType_Vector: {
			pParticle->mvVel += mpData->mvGravityAcc * afTimeStep;
			break;
		}
		case ePEGravityType_Center: {
			cVector3f vDir;
			cMatrixf mtxStart;
			if (mpData->mCoordSystem == eParticleEmitter3DCoordSystem_World) {
				vDir = pParticle->mvPos - GetWorldMatrix().GetTranslation();
			} else {
				// Perhaps on mvPos is needed.. and no substraction.
				vDir = pParticle->mvPos - GetLocalMatrix().GetTranslation();
			}

			vDir.Normalise();

			pParticle->mvVel += vDir * mpData->mvGravityAcc.y * afTimeStep;

			break;
		}
		case ePEGravityType_LastEnum:
			break;
		}

		if (pParticle->mfMaxSpeed > 0) {
			float fSpeed = pParticle->mvVel.Length();
			if (fSpeed > pParticle->mfMaxSpeed) {
				pParticle->mvVel = (pParticle->mvVel / fSpeed) * pParticle->mfMaxSpeed;
			}
		}

		if (pParticle->mfSpeedMul != 0 && pParticle->mfSpeedMul != 1) {
			pParticle->mvVel = pParticle->mvVel * pow(pParticle->mfSpeedMul, afTimeStep);
		}

		// NEW
		///////////
		// Spin Update
		if (mpData->mbUsePartSpin) {
			pParticle->mfSpin += pParticle->mfSpinVel * afTimeStep;

			if (mpData->mPartSpinType == ePEPartSpinType_Movement)
				pParticle->mfSpinVel = pParticle->mvVel.Length() * pParticle->mfSpinFactor;

			if (pParticle->mfSpin >= k2Pif)
				pParticle->mfSpin -= k2Pif;
			else if (pParticle->mfSpin <= -k2Pif)
				pParticle->mfSpin += k2Pif;
		}

		// ---

		// NEW
		// Revolution
		if (mbUseRevolution) {
			// pParticle->mvRevolution += pParticle->mvRevolutionVel * afTimeStep;
			cMatrixf mtxRotationMatrix = cMath::MatrixRotate(pParticle->mvRevolutionVel * afTimeStep, eEulerRotationOrder_XYZ);
			pParticle->mvPos = cMath::MatrixMul(mtxRotationMatrix, pParticle->mvPos);
			pParticle->mvVel = cMath::MatrixMul(mtxRotationMatrix, pParticle->mvVel);
		}

		// ---

		////////////
		// Collison update
		if (bColliding) {
			cVector3f vPos, vNormal;

			if (mpData->CheckCollision(pParticle->mvLastCollidePos, pParticle->mvPos,
									   mpWorld->GetPhysicsWorld(),
									   &vNormal, &vPos)) {
				// Log("Coll pos: %s\n",vPos.ToString().c_str());
				// Log("Coll normal: %s\n",vNormal.ToString().c_str());

				pParticle->mvPos = vPos;

				float fSpeed = pParticle->mvVel.Length();

				cVector3f vReflection = pParticle->mvVel -
										(vNormal * 2 * cMath::Vector3Dot(pParticle->mvVel, vNormal));
				vReflection.Normalise();

				pParticle->mvVel = vReflection * (fSpeed * pParticle->mfBounceAmount);

				pParticle->mlBounceCount--;
				if (pParticle->mlBounceCount <= 0) {
					pParticle->mfLife = 0;
				}
			}

			pParticle->mvLastCollidePos = pParticle->mvPos;
		}

		////////////
		// Life Update
		pParticle->mfLife -= afTimeStep;

		if (pParticle->mfLife <= 0) {
			if (mbRespawn) {
				if (mbPaused)
					SwapRemove(i);
				else
					SetParticleDefaults(pParticle);
			} else {
				SwapRemove(i);
				mlMaxParticles--;

				if (mlMaxParticles <= 0) {
					mbDying = true;
				}
			}
		}

		////////////
		// Subdiv Update

		if (mpData->mSubDivType == ePESubDivType_Animation) {
			float fLifePercent = (1.0f - (pParticle->mfLife / pParticle->mfStartLife));
			pParticle->mlSubDivNum = (int)(fLifePercent * (float)mvSubDivUV.size() - 0.0001f);
		}

		////////////
		// Color Update

		// Start
		if (pParticle->mfLife > pParticle->mfLifeColor_MiddleStart) {
			float fT = (pParticle->mfLife - pParticle->mfLifeColor_MiddleStart) /
					   (pParticle->mfStartLife - pParticle->mfLifeColor_MiddleStart);

			pParticle->mColor = (pParticle->mStartColor * mpData->mStartRelColor * fT) +
								(pParticle->mStartColor * mpData->mMiddleRelColor * (1 - fT));
		}
		// Middle
		else if (pParticle->mfLife > pParticle->mfLifeColor_MiddleEnd) {
			pParticle->mColor = pParticle->mStartColor * mpData->mMiddleRelColor;
		}
		// End
		else {
			float fT = pParticle->mfLife / pParticle->mfLifeColor_MiddleEnd;

			pParticle->mColor = (pParticle->mStartColor * mpData->mMiddleRelColor * fT) +
								(pParticle->mStartColor * mpData->mEndRelColor * (1 - fT));
		}

		if (mpData->mbMultiplyRGBWithAlpha) {
			pParticle->mColor.r *= pParticle->mColor.a;
			pParticle->mColor.g *= pParticle->mColor.a;
			pParticle->mColor.b *= pParticle->mColor.a;
		}

		////////////
		// Size Update

		// Start
		if (pParticle->mfLife > pParticle->mfLifeSize_MiddleStart) {
			float fT = (pParticle->mfLife - pParticle->mfLifeSize_MiddleStart) /
					   (pParticle->mfStartLife - pParticle->mfLifeSize_MiddleStart);

			pParticle->mvSize = (pParticle->mvStartSize * mpData->mfStartRelSize * fT) +
								(pParticle->mvStartSize * mpData->mfMiddleRelSize * (1 - fT));
		}
		// Middle
		else if (pParticle->mfLife > pParticle->mfLifeSize_MiddleEnd) {
			pParticle->mvSize = pParticle->mvStartSize * mpData->mfMiddleRelSize;
		}
		// End
		else {
			float fT = pParticle->mfLife / pParticle->mfLifeSize_MiddleEnd;

			pParticle->mvSize = (pParticle->mvStartSize * mpData->mfMiddleRelSize * fT) +
								(pParticle->mvStartSize * mpData->mfEndRelSize * (1 - fT));
		}
	}

	///////////////////////////////////////////
	// Frame Update
	if (mvMaterials->size() > 1) {
		mfFrame += mpData->mfFrameStep * afTimeStep;
		if (mfFrame >= mpData->mfMaxFrameTime) {
			mfFrame = 0;
		}
	}
}

//-----------------------------------------------------------------------
} // namespace hpl
