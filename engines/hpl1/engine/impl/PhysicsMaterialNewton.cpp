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

#include "hpl1/engine/impl/PhysicsMaterialNewton.h"

#include "hpl1/engine/impl/PhysicsBodyNewton.h"
#include "hpl1/engine/impl/PhysicsWorldNewton.h"
#include "hpl1/engine/libraries/newton/Newton.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/physics/PhysicsMaterial.h"
#include "hpl1/engine/physics/SurfaceData.h"

#include "common/util.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPhysicsMaterialNewton::cPhysicsMaterialNewton(const tString &asName, iPhysicsWorld *apWorld, int alMatId)
	: iPhysicsMaterial(asName, apWorld) {
	cPhysicsWorldNewton *pNWorld = static_cast<cPhysicsWorldNewton *>(mpWorld);

	mpNewtonWorld = pNWorld->GetNewtonWorld();

	if (alMatId == -1) {
		mlMaterialId = NewtonMaterialCreateGroupID(mpNewtonWorld);
	} else {
		mlMaterialId = alMatId;
	}

	// Setup default properties
	mFrictionMode = ePhysicsMaterialCombMode_Average;
	mElasticityMode = ePhysicsMaterialCombMode_Average;

	mfElasticity = 0.5f;
	mfStaticFriction = 0.3f;
	mfKineticFriction = 0.3f;

	// Log(" Created physics material '%s' with Newton id %d\n",asName.c_str(),mlMaterialId);
}

//-----------------------------------------------------------------------

cPhysicsMaterialNewton::~cPhysicsMaterialNewton() {
	/*Might be just as well to let newton handle this*/
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cPhysicsMaterialNewton::SetElasticity(float afElasticity) {
	mfElasticity = afElasticity;

	UpdateMaterials();
}

float cPhysicsMaterialNewton::GetElasticity() const {
	return mfElasticity;
}

//-----------------------------------------------------------------------

void cPhysicsMaterialNewton::SetStaticFriction(float afElasticity) {
	mfStaticFriction = afElasticity;

	UpdateMaterials();
}

float cPhysicsMaterialNewton::GetStaticFriction() const {
	return mfStaticFriction;
}

//-----------------------------------------------------------------------

void cPhysicsMaterialNewton::SetKineticFriction(float afElasticity) {
	mfKineticFriction = afElasticity;

	UpdateMaterials();
}

float cPhysicsMaterialNewton::GetKineticFriction() const {
	return mfKineticFriction;
}

//-----------------------------------------------------------------------

void cPhysicsMaterialNewton::SetFrictionCombMode(ePhysicsMaterialCombMode aMode) {
	mFrictionMode = aMode;

	UpdateMaterials();
}

ePhysicsMaterialCombMode cPhysicsMaterialNewton::GetFrictionCombMode() const {
	return mFrictionMode;
}

//-----------------------------------------------------------------------

void cPhysicsMaterialNewton::SetElasticityCombMode(ePhysicsMaterialCombMode aMode) {
	mElasticityMode = aMode;

	UpdateMaterials();
}

//-----------------------------------------------------------------------

ePhysicsMaterialCombMode cPhysicsMaterialNewton::GetElasticityCombMode() const {
	return mElasticityMode;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cPhysicsMaterialNewton::UpdateMaterials() {
	cPhysicsMaterialIterator MatIt = mpWorld->GetMaterialIterator();

	while (MatIt.HasNext()) {
		cPhysicsMaterialNewton *pMat = static_cast<cPhysicsMaterialNewton *>(MatIt.Next());

		ePhysicsMaterialCombMode frictionMode = (ePhysicsMaterialCombMode)MAX(mFrictionMode,
																			  pMat->mFrictionMode);
		ePhysicsMaterialCombMode elasticityMode = (ePhysicsMaterialCombMode)MAX(mElasticityMode,
																				pMat->mElasticityMode);

		// If the material is the same do not blend.
		if (pMat == this) {
			frictionMode = ePhysicsMaterialCombMode_Average;
			elasticityMode = ePhysicsMaterialCombMode_Average;
		}

		NewtonMaterialSetDefaultElasticity(mpNewtonWorld, mlMaterialId, pMat->mlMaterialId,
										   Combine(elasticityMode, mfElasticity, pMat->mfElasticity));

		NewtonMaterialSetDefaultFriction(mpNewtonWorld, mlMaterialId, pMat->mlMaterialId,
										 Combine(frictionMode, mfStaticFriction, pMat->mfStaticFriction),
										 Combine(frictionMode, mfKineticFriction, pMat->mfKineticFriction));

		NewtonMaterialSetContinuousCollisionMode(mpNewtonWorld, mlMaterialId, pMat->mlMaterialId,
												 1);

		NewtonMaterialSetCollisionCallback(mpNewtonWorld, mlMaterialId, pMat->mlMaterialId,
										   nullptr, BeginContactCallback, ProcessContactCallback);
	}
}

//-----------------------------------------------------------------------

float cPhysicsMaterialNewton::Combine(ePhysicsMaterialCombMode aMode, float afX, float afY) {
	switch (aMode) {
	case ePhysicsMaterialCombMode_Average:
		return (afX + afY) / 2;
	case ePhysicsMaterialCombMode_Min:
		return MIN(afX, afY);
	case ePhysicsMaterialCombMode_Max:
		return MAX(afX, afY);
	case ePhysicsMaterialCombMode_Multiply:
		return afX * afY;
	default:
		break;
	}

	return (afX + afY) / 2;
}

//////////////////////////////////////////////////////////////////////////
// STATIC NEWTON CALLBACKS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
int cPhysicsMaterialNewton::BeginContactCallback(const NewtonMaterial *material,
												 const NewtonBody *body0, const NewtonBody *body1, int) {
	iPhysicsBody *contactBody0 = (cPhysicsBodyNewton *)NewtonBodyGetUserData(body0);
	iPhysicsBody *contactBody1 = (cPhysicsBodyNewton *)NewtonBodyGetUserData(body1);

	if (contactBody0->GetCollide() == false)
		return 0;
	if (contactBody1->GetCollide() == false)
		return 0;

	if (contactBody0->IsActive() == false)
		return 0;
	if (contactBody1->IsActive() == false)
		return 0;

	if (contactBody0->IsRagDoll() && contactBody1->GetCollideRagDoll() == false)
		return 0;
	if (contactBody1->IsRagDoll() && contactBody0->GetCollideRagDoll() == false)
		return 0;

	if (contactBody0->IsCharacter() && contactBody1->GetCollideCharacter() == false)
		return 0;
	if (contactBody1->IsCharacter() && contactBody0->GetCollideCharacter() == false)
		return 0;

	if (contactBody0->OnBeginCollision(contactBody1) == false)
		return 0;
	if (contactBody1->OnBeginCollision(contactBody0) == false)
		return 0;

	return 1;
}

//-----------------------------------------------------------------------

class ContactProcessor {
public:
	ContactProcessor(const NewtonJoint *joint);

	bool processNext();
	void endProcessing();

private:
	void *_contact;
	int _contacts;
	const NewtonJoint *_joint;
	NewtonBody *_body0;
	NewtonBody *_body1;
	cPhysicsBodyNewton *_contactBody0;
	cPhysicsBodyNewton *_contactBody1;
	cPhysicsContactData _contactData;
};

ContactProcessor::ContactProcessor(const NewtonJoint *joint) : _joint(joint), _contacts(0), _contact(nullptr) {
	_body0 = NewtonJointGetBody0(joint);
	_body1 = NewtonJointGetBody1(joint);
	_contactBody0 = (cPhysicsBodyNewton *)NewtonBodyGetUserData(_body0);
	_contactBody1 = (cPhysicsBodyNewton *)NewtonBodyGetUserData(_body1);
	_contact = NewtonContactJointGetFirstContact(_joint);
}

bool ContactProcessor::processNext() {
	NewtonMaterial *_material = NewtonContactGetMaterial(_contact);
	float fNormSpeed = NewtonMaterialGetContactNormalSpeed(_material);
	if (_contactData.mfMaxContactNormalSpeed < fNormSpeed)
		_contactData.mfMaxContactNormalSpeed = fNormSpeed;

	// Tangent speed
	float fTanSpeed0 = NewtonMaterialGetContactTangentSpeed(_material, 0);
	float fTanSpeed1 = NewtonMaterialGetContactTangentSpeed(_material, 1);
	if (ABS(_contactData.mfMaxContactTangentSpeed) < ABS(fTanSpeed0))
		_contactData.mfMaxContactTangentSpeed = fTanSpeed0;
	if (ABS(_contactData.mfMaxContactTangentSpeed) < ABS(fTanSpeed1))
		_contactData.mfMaxContactTangentSpeed = fTanSpeed1;

	// Force
	float force[3];
	NewtonMaterialGetContactForce(_material, _body0, force);
	_contactData.mvForce += cVector3f::fromArray(force);

	// Position and normal
	float matPos[3], matNormal[3];
	NewtonMaterialGetContactPositionAndNormal(_material, _body0, matPos, matNormal);
	_contactData.mvContactNormal += cVector3f::fromArray(matNormal);
	_contactData.mvContactPosition += cVector3f::fromArray(matPos);

	if (_contactBody0->GetWorld()->GetSaveContactPoints()) {
		NewtonMaterialGetContactPositionAndNormal(_material, _body0, matPos, matNormal);
		cCollidePoint collidePoint;
		collidePoint.mfDepth = 1;
		collidePoint.mvPoint = cVector3f::fromArray(matPos);
		collidePoint.mvNormal = cVector3f::fromArray(matNormal);
		_contactBody0->GetWorld()->GetContactPoints()->push_back(collidePoint);
	}
	++_contacts;
	return (_contact = NewtonContactJointGetNextContact(_joint, _contact));
}

void ContactProcessor::endProcessing() {
	if (_contacts == 0)
		return;

	iPhysicsMaterial *pMaterial1 = _contactBody0->GetMaterial();
	iPhysicsMaterial *pMaterial2 = _contactBody1->GetMaterial();

	_contactData.mvContactNormal = _contactData.mvContactNormal / (float)_contacts;
	_contactData.mvContactPosition = _contactData.mvContactPosition / (float)_contacts;

	pMaterial1->GetSurfaceData()->CreateImpactEffect(_contactData.mfMaxContactNormalSpeed,
													 _contactData.mvContactPosition,
													 _contacts, pMaterial2->GetSurfaceData());

	int lPrio1 = pMaterial1->GetSurfaceData()->GetPriority();
	int lPrio2 = pMaterial2->GetSurfaceData()->GetPriority();

	if (lPrio1 >= lPrio2) {
		if (ABS(_contactData.mfMaxContactNormalSpeed) > 0)
			pMaterial1->GetSurfaceData()->OnImpact(_contactData.mfMaxContactNormalSpeed,
												   _contactData.mvContactPosition,
												   _contacts, _contactBody0);
		if (ABS(_contactData.mfMaxContactTangentSpeed) > 0)
			pMaterial1->GetSurfaceData()->OnSlide(_contactData.mfMaxContactTangentSpeed,
												  _contactData.mvContactPosition,
												  _contacts, _contactBody0, _contactBody1);
	}

	if (lPrio2 >= lPrio1 && pMaterial2 != pMaterial1) {
		if (ABS(_contactData.mfMaxContactNormalSpeed) > 0)
			pMaterial2->GetSurfaceData()->OnImpact(_contactData.mfMaxContactNormalSpeed,
												   _contactData.mvContactPosition,
												   _contacts, _contactBody1);
		if (ABS(_contactData.mfMaxContactTangentSpeed) > 0)
			pMaterial2->GetSurfaceData()->OnSlide(_contactData.mfMaxContactTangentSpeed,
												  _contactData.mvContactPosition,
												  _contacts, _contactBody1, _contactBody0);
	}

	_contactBody0->OnCollide(_contactBody1, &_contactData);
	_contactBody1->OnCollide(_contactBody0, &_contactData);
}

void cPhysicsMaterialNewton::ProcessContactCallback(const NewtonJoint *joint, float, int) {
	ContactProcessor processor(joint);

	while (processor.processNext()) {
	}
	processor.endProcessing();
}

} // namespace hpl
