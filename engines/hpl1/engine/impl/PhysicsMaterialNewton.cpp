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
#include "hpl1/engine/impl/PhysicsMaterialNewton.h"

#include "hpl1/engine/impl/PhysicsWorldNewton.h"
#include "hpl1/engine/impl/PhysicsBodyNewton.h"
#include "hpl1/engine/physics/SurfaceData.h"

#include "hpl1/engine/system/LowLevelSystem.h"
#include <algorithm>

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cPhysicsMaterialNewton::cPhysicsMaterialNewton(const tString &asName, iPhysicsWorld *apWorld,int alMatId)
		: iPhysicsMaterial(asName,apWorld)
	{
		cPhysicsWorldNewton *pNWorld = static_cast<cPhysicsWorldNewton*>(mpWorld);

		mpNewtonWorld = pNWorld->GetNewtonWorld();

		if(alMatId==-1)
		{
			mlMaterialId = NewtonMaterialCreateGroupID(mpNewtonWorld);
		}
		else
		{
			mlMaterialId = alMatId;
		}

		//Setup default properties
		mFrictionMode = ePhysicsMaterialCombMode_Average;
		mElasticityMode  = ePhysicsMaterialCombMode_Average;

		mfElasticity = 0.5f;
		mfStaticFriction = 0.3f;
		mfKineticFriction = 0.3f;

		//Log(" Created physics material '%s' with Newton id %d\n",asName.c_str(),mlMaterialId);
	}

	//-----------------------------------------------------------------------

	cPhysicsMaterialNewton::~cPhysicsMaterialNewton()
	{
		/*Might be just as well to let newton handle this*/
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cPhysicsMaterialNewton::SetElasticity(float afElasticity)
	{
		mfElasticity = afElasticity;

		UpdateMaterials();
	}

	float cPhysicsMaterialNewton::GetElasticity() const
	{
		return mfElasticity;
	}

	//-----------------------------------------------------------------------

	void cPhysicsMaterialNewton::SetStaticFriction(float afElasticity)
	{
		mfStaticFriction = afElasticity;

		UpdateMaterials();
	}

	float cPhysicsMaterialNewton::GetStaticFriction() const
	{
		return mfStaticFriction;
	}

	//-----------------------------------------------------------------------

	void cPhysicsMaterialNewton::SetKineticFriction(float afElasticity)
	{
		mfKineticFriction = afElasticity;

		UpdateMaterials();
	}

	float cPhysicsMaterialNewton::GetKineticFriction() const
	{
		return mfKineticFriction;
	}

	//-----------------------------------------------------------------------

	void cPhysicsMaterialNewton::SetFrictionCombMode(ePhysicsMaterialCombMode aMode)
	{
		mFrictionMode = aMode;

		UpdateMaterials();
	}

	ePhysicsMaterialCombMode cPhysicsMaterialNewton::GetFrictionCombMode() const
	{
		return mFrictionMode;
	}

	//-----------------------------------------------------------------------

	void cPhysicsMaterialNewton::SetElasticityCombMode(ePhysicsMaterialCombMode aMode)
	{
		mElasticityMode = aMode;

		UpdateMaterials();
	}

	//-----------------------------------------------------------------------

	ePhysicsMaterialCombMode cPhysicsMaterialNewton::GetElasticityCombMode() const
	{
		return mElasticityMode;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cPhysicsMaterialNewton::UpdateMaterials()
	{
#if 0
  		cPhysicsMaterialIterator MatIt = mpWorld->GetMaterialIterator();

		while(MatIt.HasNext())
		{
			cPhysicsMaterialNewton* pMat = static_cast<cPhysicsMaterialNewton*>(MatIt.Next());

			ePhysicsMaterialCombMode frictionMode =   (ePhysicsMaterialCombMode) std::max(mFrictionMode,
				pMat->mFrictionMode);
			ePhysicsMaterialCombMode elasticityMode = (ePhysicsMaterialCombMode) std::max(mElasticityMode,
				pMat->mElasticityMode);

			//If the material is the same do not blend.
			if(pMat == this){
				frictionMode = 	ePhysicsMaterialCombMode_Average;
				elasticityMode = ePhysicsMaterialCombMode_Average;
			}


			NewtonMaterialSetDefaultElasticity(mpNewtonWorld,mlMaterialId,pMat->mlMaterialId,
				Combine(elasticityMode,mfElasticity, pMat->mfElasticity));

			NewtonMaterialSetDefaultFriction(mpNewtonWorld,mlMaterialId,pMat->mlMaterialId,
				Combine(frictionMode,mfStaticFriction, pMat->mfStaticFriction),
				Combine(frictionMode,mfKineticFriction, pMat->mfKineticFriction));

			NewtonMaterialSetContinuousCollisionMode(mpNewtonWorld,mlMaterialId,pMat->mlMaterialId,
													1);

			NewtonMaterialSetCollisionCallback(mpNewtonWorld,mlMaterialId,pMat->mlMaterialId,
				(void*)NULL,BeginContactCallback,ProcessContactCallback,EndContactCallback);
		}
#endif

	}

	//-----------------------------------------------------------------------

	float cPhysicsMaterialNewton::Combine(ePhysicsMaterialCombMode aMode, float afX, float afY)
	{
		switch(aMode)
		{
		case ePhysicsMaterialCombMode_Average: return (afX + afY)/2;
		case ePhysicsMaterialCombMode_Min: return std::min(afX, afY);
		case ePhysicsMaterialCombMode_Max: return std::max(afX, afY);
		case ePhysicsMaterialCombMode_Multiply: return afX * afY;
		}

		return (afX + afY) /2;
	}

	//////////////////////////////////////////////////////////////////////////
	// STATIC NEWTON CALLBACKS
	//////////////////////////////////////////////////////////////////////////

	iPhysicsBody *cPhysicsMaterialNewton::mpContactBody1 = NULL;
	iPhysicsBody *cPhysicsMaterialNewton::mpContactBody2 = NULL;
	int cPhysicsMaterialNewton::mlContactNum =0;
	cPhysicsContactData cPhysicsMaterialNewton::mContactData;

	//-----------------------------------------------------------------------
	int cPhysicsMaterialNewton::BeginContactCallback(const NewtonMaterial* material,
									const NewtonBody* apBody1, const NewtonBody* apBody2)
	{
		mpContactBody1 = (cPhysicsBodyNewton*) NewtonBodyGetUserData(apBody1);
		mpContactBody2 = (cPhysicsBodyNewton*) NewtonBodyGetUserData(apBody2);

		if(mpContactBody1->GetCollide()==false) return 0;
		if(mpContactBody2->GetCollide()==false) return 0;

		if(mpContactBody1->IsActive()==false) return 0;
		if(mpContactBody2->IsActive()==false) return 0;

		if(mpContactBody1->IsRagDoll() && mpContactBody2->GetCollideRagDoll()==false) return 0;
		if(mpContactBody2->IsRagDoll() && mpContactBody1->GetCollideRagDoll()==false) return 0;

		if(mpContactBody1->IsCharacter() && mpContactBody2->GetCollideCharacter()==false) return 0;
		if(mpContactBody2->IsCharacter() && mpContactBody1->GetCollideCharacter()==false) return 0;

		//Log("----- Begin contact between body '%s' and '%s'.\n",mpContactBody1->GetName().c_str(),
		//													mpContactBody2->GetName().c_str());

		//Reset contact num
		mlContactNum =0;

		if(mpContactBody1->OnBeginCollision(mpContactBody2)==false) return 0;
		if(mpContactBody2->OnBeginCollision(mpContactBody1)==false) return 0;

		return 1;
	}

	//-----------------------------------------------------------------------

	int cPhysicsMaterialNewton::ProcessContactCallback(const NewtonMaterial* apMaterial,
										const NewtonContact* apContact)
	{
#if 0
  		//Log(" Process contact between body '%s' and '%s'.\n",mpContactBody1->GetName().c_str(),
		//													mpContactBody2->GetName().c_str());

		//Normal speed
		float fNormSpeed = NewtonMaterialGetContactNormalSpeed(apMaterial,apContact);
		if(mContactData.mfMaxContactNormalSpeed < fNormSpeed) mContactData.mfMaxContactNormalSpeed = fNormSpeed;

		//Tangent speed
		float fTanSpeed0 = NewtonMaterialGetContactTangentSpeed(apMaterial,apContact,0);
		float fTanSpeed1 = NewtonMaterialGetContactTangentSpeed(apMaterial,apContact,1);
		if(std::abs(mContactData.mfMaxContactTangentSpeed) < std::abs(fTanSpeed0)) mContactData.mfMaxContactTangentSpeed = fTanSpeed0;
		if(std::abs(mContactData.mfMaxContactTangentSpeed) < std::abs(fTanSpeed1)) mContactData.mfMaxContactTangentSpeed = fTanSpeed1;

		//Force
		cVector3f vForce;
		NewtonMaterialGetContactForce(apMaterial,vForce.v);
		mContactData.mvForce += vForce;

		//Position and normal
		cVector3f vPos, vNormal;
		NewtonMaterialGetContactPositionAndNormal(apMaterial,vPos.v, vNormal.v);

		mContactData.mvContactNormal += vNormal;
		mContactData.mvContactPosition += vPos;

		//cVector3f vForce;
		//NewtonMaterialGetContactForce(apMaterial,vForce.v);

		//Log(" Norm: %f Tan0: %f Tan1: %f\n",fNormSpeed, fTanSpeed0, fTanSpeed1);
		//Log("Force: %s\n",vForce.ToString().c_str());

		if(mpContactBody1->GetWorld()->GetSaveContactPoints())
		{
			cCollidePoint collidePoint;
			collidePoint.mfDepth = 1;
			NewtonMaterialGetContactPositionAndNormal (apMaterial, collidePoint.mvPoint.v,
														collidePoint.mvNormal.v);

			mpContactBody1->GetWorld()->GetContactPoints()->push_back(collidePoint);
		}

		mlContactNum++;

#endif
		return 1;

	}

	//-----------------------------------------------------------------------

	void cPhysicsMaterialNewton::EndContactCallback(const NewtonMaterial* apMaterial)
	{
		//Log("--- End contact between body '%s' and '%s'.\n",mpContactBody1->GetName().c_str(),
		//													mpContactBody2->GetName().c_str());

		if(mlContactNum <= 0) return;

		iPhysicsMaterial *pMaterial1 = mpContactBody1->GetMaterial();
		iPhysicsMaterial *pMaterial2 = mpContactBody2->GetMaterial();

		mContactData.mvContactNormal = mContactData.mvContactNormal / (float)mlContactNum;
		mContactData.mvContactPosition = mContactData.mvContactPosition / (float)mlContactNum;

		pMaterial1->GetSurfaceData()->CreateImpactEffect(mContactData.mfMaxContactNormalSpeed,
													mContactData.mvContactPosition,
													mlContactNum,pMaterial2->GetSurfaceData());

		int lPrio1 = pMaterial1->GetSurfaceData()->GetPriority();
		int lPrio2 = pMaterial2->GetSurfaceData()->GetPriority();

		if(lPrio1 >= lPrio2)
		{
			if(std::abs(mContactData.mfMaxContactNormalSpeed) > 0)
				pMaterial1->GetSurfaceData()->OnImpact(mContactData.mfMaxContactNormalSpeed,
														mContactData.mvContactPosition,
														mlContactNum,mpContactBody1);
			if(std::abs(mContactData.mfMaxContactTangentSpeed) > 0)
				pMaterial1->GetSurfaceData()->OnSlide(mContactData.mfMaxContactTangentSpeed,
														mContactData.mvContactPosition,
														mlContactNum,mpContactBody1,mpContactBody2);
		}

		if(lPrio2 >= lPrio1 && pMaterial2 != pMaterial1)
		{
			if(std::abs(mContactData.mfMaxContactNormalSpeed) > 0)
				pMaterial2->GetSurfaceData()->OnImpact(mContactData.mfMaxContactNormalSpeed,
														mContactData.mvContactPosition,
														mlContactNum,mpContactBody2);
			if(std::abs(mContactData.mfMaxContactTangentSpeed) > 0)
				pMaterial2->GetSurfaceData()->OnSlide(mContactData.mfMaxContactTangentSpeed,
														mContactData.mvContactPosition,
														mlContactNum,mpContactBody2,mpContactBody1);
		}

		mpContactBody1->OnCollide(mpContactBody2,&mContactData);
		mpContactBody2->OnCollide(mpContactBody1,&mContactData);

		//Reset contact data
		mpContactBody1 = NULL;
		mpContactBody2 = NULL;

		mlContactNum =0;

		mContactData.mfMaxContactTangentSpeed =0;
		mContactData.mfMaxContactNormalSpeed =0;

		mContactData.mvContactPosition = cVector3f(0,0,0);
		mContactData.mvContactNormal = cVector3f(0,0,0);
		mContactData.mvForce = cVector3f(0,0,0);
	}

	//-----------------------------------------------------------------------

}
