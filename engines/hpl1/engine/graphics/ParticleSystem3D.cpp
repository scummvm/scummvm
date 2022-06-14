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

#include "hpl1/engine/graphics/ParticleSystem3D.h"

#include "hpl1/engine/impl/tinyXML/tinyxml.h"

#include "hpl1/engine/graphics/ParticleEmitter3D_UserData.h"

#include "hpl1/engine/resources/ParticleManager.h"

#include "hpl1/engine/game/Game.h"

#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/World3D.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CREATOR
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cParticleSystemData3D::cParticleSystemData3D(const tString &asName,
											 cResources *apResources, cGraphics *apGraphics)
	: iResourceBase(asName, 0) {
	mpResources = apResources;
	mpGraphics = apGraphics;
}

//-----------------------------------------------------------------------

cParticleSystemData3D::~cParticleSystemData3D() {
	STLDeleteAll(mvEmitterData);
}

//-----------------------------------------------------------------------

void cParticleSystemData3D::AddEmitterData(iParticleEmitterData *apData) {
	mvEmitterData.push_back(apData);
}

//-----------------------------------------------------------------------

cParticleSystem3D *cParticleSystemData3D::Create(tString asName, cVector3f avSize,
												 const cMatrixf &a_mtxTransform) {
	if (mvEmitterData.empty()) {
		Warning("Particle system '%s' has no emitters.\n", msName.c_str());
		return NULL;
	}

	cParticleSystem3D *pPS = hplNew(cParticleSystem3D, (asName, this, mpResources, mpGraphics));
	pPS->SetMatrix(a_mtxTransform);

	for (size_t i = 0; i < mvEmitterData.size(); ++i) {
		///////////////////////////
		// Create and add
		iParticleEmitter3D *pPE = static_cast<iParticleEmitter3D *>(mvEmitterData[i]->Create(asName, avSize));
		pPS->AddEmitter(pPE);
		pPE->SetSystem(pPS);
	}

	return pPS;
}

//-----------------------------------------------------------------------

bool cParticleSystemData3D::LoadFromFile(const tString &asFile) {
	TiXmlDocument *pXmlDoc = hplNew(TiXmlDocument, (asFile.c_str()));
	if (pXmlDoc->LoadFile() == false) {
		Warning("Couldn't open XML file %s\n", asFile.c_str());
		hplDelete(pXmlDoc);
		return false;
	}

	TiXmlElement *pRootElem = pXmlDoc->RootElement();

	TiXmlElement *pEmitterElem = pRootElem->FirstChildElement("ParticleEmitter");
	for (; pEmitterElem != NULL; pEmitterElem = pEmitterElem->NextSiblingElement("ParticleEmitter")) {
		cParticleEmitterData3D_UserData *pPE = hplNew(cParticleEmitterData3D_UserData, ("",
																						mpResources, mpGraphics));

		pPE->LoadFromElement(pEmitterElem);

		mvEmitterData.push_back(pPE);
	}

	hplDelete(pXmlDoc);
	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cParticleSystem3D::cParticleSystem3D(const tString asName,
									 cParticleSystemData3D *apData,
									 cResources *apResources, cGraphics *apGraphics)
	: iEntity3D(asName) {
	mpResources = apResources;
	mpGraphics = apGraphics;
	mpParticleManager = NULL;
	mpData = apData;

	mbFirstUpdate = true;
}

//-----------------------------------------------------------------------

cParticleSystem3D::~cParticleSystem3D() {
	for (size_t i = 0; i < mvEmitters.size(); ++i) {
		hplDelete(mvEmitters[i]);
	}
	if (mpParticleManager)
		mpParticleManager->Destroy(mpData);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cParticleSystem3D::SetVisible(bool abVisible) {
	if (mbIsVisible == abVisible)
		return;

	mbIsVisible = abVisible;

	for (size_t i = 0; i < mvEmitters.size(); ++i) {
		mvEmitters[i]->SetVisible(mbIsVisible);
	}
}

//-----------------------------------------------------------------------

bool cParticleSystem3D::IsDead() {
	size_t lCount = 0;

	for (size_t i = 0; i < mvEmitters.size(); ++i) {
		iParticleEmitter3D *pPE = mvEmitters[i];

		if (pPE->IsDead())
			lCount++;
	}

	if (lCount == mvEmitters.size())
		return true;

	return false;
}

bool cParticleSystem3D::IsDying() {
	size_t lCount = 0;

	for (size_t i = 0; i < mvEmitters.size(); ++i) {
		iParticleEmitter3D *pPE = mvEmitters[i];

		if (pPE->IsDying())
			lCount++;
	}

	if (lCount == mvEmitters.size())
		return true;

	return false;
}

//-----------------------------------------------------------------------

void cParticleSystem3D::Kill() {
	SetIsSaved(false);
	for (size_t i = 0; i < mvEmitters.size(); ++i) {
		iParticleEmitter3D *pPE = mvEmitters[i];

		pPE->Kill();
	}
}

//-----------------------------------------------------------------------

void cParticleSystem3D::KillInstantly() {
	SetIsSaved(false);
	for (size_t i = 0; i < mvEmitters.size(); ++i) {
		iParticleEmitter3D *pPE = mvEmitters[i];

		pPE->KillInstantly();
	}
}

//-----------------------------------------------------------------------

void cParticleSystem3D::UpdateLogic(float afTimeStep) {
	if (IsActive() == false)
		return;

	for (size_t i = 0; i < mvEmitters.size(); ++i) {
		iParticleEmitter3D *pPE = mvEmitters[i];

		//////////////////////////
		// Warm Up
		if (mbFirstUpdate) {
			iParticleEmitterData *pData = mpData->GetEmitterData((int)i);

			if (pData->GetWarmUpTime() > 0) {
				float fTime = pData->GetWarmUpTime();
				float fStepSize = 1.0f / pData->GetWarmUpStepsPerSec();

				while (fTime > 0) {
					pPE->UpdateLogic(fStepSize);
					fTime -= fStepSize;
				}
			}
			mbFirstUpdate = false;
		}

		//////////////////////////
		// Update
		pPE->UpdateLogic(afTimeStep);
	}
}

//-----------------------------------------------------------------------

void cParticleSystem3D::AddEmitter(iParticleEmitter3D *apEmitter) {
	mvEmitters.push_back(apEmitter);

	AddChild(apEmitter);
}

//-----------------------------------------------------------------------

iParticleEmitter3D *cParticleSystem3D::GetEmitter(int alIdx) {
	return mvEmitters[alIdx];
}

//-----------------------------------------------------------------------

int cParticleSystem3D::GetEmitterNum() {
	return (int)mvEmitters.size();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

kBeginSerializeBase(cSaveData_ParticleEmitter3D)
	kSerializeVar(mbActive, eSerializeType_Bool)
		kSerializeVar(mbDying, eSerializeType_Bool)
			kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerialize(cSaveData_cParticleSystem3D, cSaveData_iEntity3D)
		kSerializeVar(msDataName, eSerializeType_String)
			kSerializeVar(mvDataSize, eSerializeType_Vector3f)

				kSerializeClassContainer(mvEmitters, cSaveData_ParticleEmitter3D, eSerializeType_Class)
					kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveObject *cSaveData_cParticleSystem3D::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	cWorld3D *pWorld = apGame->GetScene()->GetWorld3D();

	cParticleSystem3D *pPS = pWorld->CreateParticleSystem(msName, msDataName, mvDataSize,
														  m_mtxLocalTransform);

	return pPS;
}

//-----------------------------------------------------------------------

int cSaveData_cParticleSystem3D::GetSaveCreatePrio() {
	return 3;
}

//-----------------------------------------------------------------------

iSaveData *cParticleSystem3D::CreateSaveData() {
	return hplNew(cSaveData_cParticleSystem3D, ());
}

//-----------------------------------------------------------------------

void cParticleSystem3D::SaveToSaveData(iSaveData *apSaveData) {
	kSaveData_SaveToBegin(cParticleSystem3D);

	kSaveData_SaveTo(msDataName);
	kSaveData_SaveTo(mvDataSize);

	pData->mvEmitters.Resize(GetEmitterNum());
	for (int i = 0; i < GetEmitterNum(); ++i) {
		iParticleEmitter3D *pPE = GetEmitter(i);

		pData->mvEmitters[i].mbDying = pPE->IsDying();
		pData->mvEmitters[i].mbActive = pPE->IsActive();
	}
}

//-----------------------------------------------------------------------

void cParticleSystem3D::LoadFromSaveData(iSaveData *apSaveData) {
	kSaveData_LoadFromBegin(cParticleSystem3D);

	kSaveData_LoadFrom(msDataName);
	kSaveData_LoadFrom(mvDataSize);

	if (GetEmitterNum() != pData->mvEmitters.Size()) {
		Error("Saved emitter number in %s/%d does not match loaded, killing system!\n",
			  GetName().c_str(),
			  GetSaveObjectId());

		for (int i = 0; i < GetEmitterNum(); ++i) {
			iParticleEmitter3D *pPE = GetEmitter(i);
			pPE->KillInstantly();
			return;
		}
	}

	for (int i = 0; i < GetEmitterNum(); ++i) {
		iParticleEmitter3D *pPE = GetEmitter(i);

		pPE->SetActive(pData->mvEmitters[i].mbActive);

		if (pData->mvEmitters[i].mbDying)
			pPE->KillInstantly();
	}
}

//-----------------------------------------------------------------------

void cParticleSystem3D::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	kSaveData_SetupBegin(cParticleSystem3D);
}

//-----------------------------------------------------------------------
} // namespace hpl
