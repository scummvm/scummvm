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

#include "hpl1/engine/physics/Physics.h"
#include "hpl1/engine/physics/LowLevelPhysics.h"

#include "hpl1/engine/physics/PhysicsWorld.h"
#include "hpl1/engine/physics/SurfaceData.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/impl/tinyXML/tinyxml.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPhysics::cPhysics(iLowLevelPhysics *apLowLevelPhysics) : iUpdateable("HPL_Physics") {
	mpLowLevelPhysics = apLowLevelPhysics;

	mpGameWorld = NULL;

	mlMaxImpacts = 6;
	mfImpactDuration = 0.4f;

	mbLog = false;
}

//-----------------------------------------------------------------------

cPhysics::~cPhysics() {
	Log("Exiting Physics Module\n");
	Log("--------------------------------------------------------\n");

	STLDeleteAll(mlstWorlds);
	STLMapDeleteAll(m_mapSurfaceData);

	Log("--------------------------------------------------------\n\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cPhysics::Init(cResources *apResources) {
	mpResources = apResources;
}

//-----------------------------------------------------------------------

void cPhysics::Update(float afTimeStep) {
	UpdateImpactCounts(afTimeStep);
}

//-----------------------------------------------------------------------

iPhysicsWorld *cPhysics::CreateWorld(bool abAddSurfaceData) {
	iPhysicsWorld *pWorld = mpLowLevelPhysics->CreateWorld();
	mlstWorlds.push_back(pWorld);

	if (abAddSurfaceData) {
		tSurfaceDataMapIt it = m_mapSurfaceData.begin();
		for (; it != m_mapSurfaceData.end(); ++it) {
			cSurfaceData *apData = it->second;
			apData->ToMaterial(pWorld);
		}
	}

	return pWorld;
}

//-----------------------------------------------------------------------

void cPhysics::DestroyWorld(iPhysicsWorld *apWorld) {
	STLFindAndDelete(mlstWorlds, apWorld);
}

//-----------------------------------------------------------------------

bool cPhysics::CanPlayImpact() {
	if ((int)mlstImpactCounts.size() >= mlMaxImpacts)
		return false;
	else
		return true;
}

void cPhysics::AddImpact() {
	mlstImpactCounts.push_back(cPhysicsImpactCount());
}

//-----------------------------------------------------------------------

cSurfaceData *cPhysics::CreateSurfaceData(const tString &asName) {
	cSurfaceData *pData = hplNew(cSurfaceData, (asName, this, mpResources));

	m_mapSurfaceData.insert(tSurfaceDataMap::value_type(asName, pData));

	return pData;
}

cSurfaceData *cPhysics::GetSurfaceData(const tString &asName) {
	tSurfaceDataMapIt it = m_mapSurfaceData.find(asName);
	if (it == m_mapSurfaceData.end())
		return NULL;

	return it->second;
}

//-----------------------------------------------------------------------

bool cPhysics::LoadSurfaceData(const tString &asFile) {
	//////////////////////////////////
	// Open document
	TiXmlDocument *pXmlDoc = hplNew(TiXmlDocument, (asFile.c_str()));
	if (pXmlDoc->LoadFile() == false) {
		Error("Couldn't load XML file '%s'!\n", asFile.c_str());
		hplDelete(pXmlDoc);
		return false;
	}

	/////////////////////////
	// Get the root.
	TiXmlElement *pRootElem = pXmlDoc->RootElement();

	//////////////////////////
	// Iterate children
	TiXmlElement *pChildElem = pRootElem->FirstChildElement("Material");
	for (; pChildElem != NULL; pChildElem = pChildElem->NextSiblingElement("Material")) {
		tString sName = cString::ToString(pChildElem->Attribute("Name"), "");
		if (sName == "")
			continue;

		cSurfaceData *pData = CreateSurfaceData(sName);

		// Get properties
		pData->SetElasticity(cString::ToFloat(pChildElem->Attribute("Elasticity"), 0.5f));
		pData->SetKineticFriction(cString::ToFloat(pChildElem->Attribute("KineticFriction"), 0.3f));
		pData->SetStaticFriction(cString::ToFloat(pChildElem->Attribute("StaticFriction"), 0.3f));

		pData->SetPriority(cString::ToInt(pChildElem->Attribute("Priority"), 0));

		pData->SetElasticityCombMode(GetCombMode(pChildElem->Attribute("ElasticityMode")));
		pData->SetFrictionCombMode(GetCombMode(pChildElem->Attribute("FrictionMode")));

		pData->GetStepType(cString::ToString(pChildElem->Attribute("StepType"), ""));

		pData->SetMinScrapeSpeed(cString::ToFloat(pChildElem->Attribute("MinScrapeSpeed"), 0.7f));
		pData->SetMinScrapeFreq(cString::ToFloat(pChildElem->Attribute("MinScrapeFreq"), 0.7f));
		pData->SetMinScrapeFreqSpeed(cString::ToFloat(pChildElem->Attribute("MinScrapeFreqSpeed"), 0.7f));
		pData->SetMiddleScrapeSpeed(cString::ToFloat(pChildElem->Attribute("MiddleScrapeSpeed"), 1.2f));
		pData->SetMaxScrapeFreqSpeed(cString::ToFloat(pChildElem->Attribute("MaxScrapeFreqSpeed"), 3));
		pData->SetMaxScrapeFreq(cString::ToFloat(pChildElem->Attribute("MaxScrapeFreq"), 2));
		pData->SetMinScrapeContacts(cString::ToInt(pChildElem->Attribute("MinScrapeContacts"), 4));
		pData->SetScrapeSoundName(cString::ToString(pChildElem->Attribute("ScrapeSoundName"), ""));

		pData->SetMinRollSpeed(cString::ToFloat(pChildElem->Attribute("MinRollSpeed"), 0.7f));
		pData->SetMinRollFreq(cString::ToFloat(pChildElem->Attribute("MinRollFreq"), 0.7f));
		pData->SetMinRollVolume(cString::ToFloat(pChildElem->Attribute("MinRollVolume"), 0.7f));
		pData->SetMinRollFreqSpeed(cString::ToFloat(pChildElem->Attribute("MinRollFreqSpeed"), 0.7f));
		pData->SetMiddleRollSpeed(cString::ToFloat(pChildElem->Attribute("MiddleRollSpeed"), 1.2f));
		pData->SetMaxRollFreqSpeed(cString::ToFloat(pChildElem->Attribute("MaxRollFreqSpeed"), 3));
		pData->SetMaxRollFreq(cString::ToFloat(pChildElem->Attribute("MaxRollFreq"), 2));
		pData->SetMaxRollVolume(cString::ToFloat(pChildElem->Attribute("MaxRollVolume"), 2));
		pData->SetRollSoundName(cString::ToString(pChildElem->Attribute("RollSoundName"), ""));

		// Axes
		tString sAxisVec = cString::ToString(pChildElem->Attribute("RollAxis"), "");
		tStringVec vAxes;
		tFlag axisFlags = 0;
		cString::GetStringVec(sAxisVec, vAxes);
		for (size_t i = 0; i < vAxes.size(); ++i) {
			tString sAxis = cString::ToLowerCase(vAxes[i]);
			if (sAxis == "x")
				axisFlags |= eRollAxisFlag_X;
			else if (sAxis == "y")
				axisFlags |= eRollAxisFlag_Y;
			else if (sAxis == "z")
				axisFlags |= eRollAxisFlag_Z;
		}
		pData->SetRollAxisFlags(axisFlags);

		/////////////////////////
		// Get Impact data
		TiXmlElement *pImpactElem = pChildElem->FirstChildElement("Impact");
		for (; pImpactElem != NULL; pImpactElem = pImpactElem->NextSiblingElement("Impact")) {
			float fMinSpeed = cString::ToFloat(pImpactElem->Attribute("MinSpeed"), 1);

			cSurfaceImpactData *pImpactData = pData->CreateImpactData(fMinSpeed);

			pImpactData->SetSoundName(cString::ToString(pImpactElem->Attribute("SoundName"), ""));
			pImpactData->SetPSName(cString::ToString(pImpactElem->Attribute("PSName"), ""));
			pImpactData->SetPSPrio(cString::ToInt(pImpactElem->Attribute("PSPrio"), 10));
		}

		/////////////////////////
		// Get Hit data
		TiXmlElement *pHitElem = pChildElem->FirstChildElement("Hit");
		for (; pHitElem != NULL; pHitElem = pHitElem->NextSiblingElement("Hit")) {
			float fMinSpeed = cString::ToFloat(pHitElem->Attribute("MinSpeed"), 1);

			cSurfaceImpactData *pHitData = pData->CreateHitData(fMinSpeed);

			pHitData->SetSoundName(cString::ToString(pHitElem->Attribute("SoundName"), ""));
			pHitData->SetPSName(cString::ToString(pHitElem->Attribute("PSName"), ""));
			pHitData->SetPSPrio(cString::ToInt(pHitElem->Attribute("PSPrio"), 10));
		}

		/*Log("Added %s e: %f sf: %f kf: %f emode: %d fmode: %d\n", pData->GetName().c_str(),
				pData->GetElasticity(), pData->GetStaticFriction(), pData->GetKineticFriction(),
				pData->GetElasticityCombMode(), pData->GetFrictionCombMode());*/
	}

	hplDelete(pXmlDoc);
	return true;

	return true;
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

ePhysicsMaterialCombMode cPhysics::GetCombMode(const char *apName) {
	if (apName == NULL)
		return ePhysicsMaterialCombMode_Average;

	tString sMode = cString::ToLowerCase(apName);

	if (sMode == "average")
		return ePhysicsMaterialCombMode_Average;
	if (sMode == "min")
		return ePhysicsMaterialCombMode_Min;
	if (sMode == "max")
		return ePhysicsMaterialCombMode_Max;
	if (sMode == "multiply")
		return ePhysicsMaterialCombMode_Multiply;

	return ePhysicsMaterialCombMode_Average;
}

//-----------------------------------------------------------------------

void cPhysics::UpdateImpactCounts(float afTimeStep) {
	tPhysicsImpactCountListIt it = mlstImpactCounts.begin();
	while (it != mlstImpactCounts.end()) {
		it->mfCount += afTimeStep;
		if (it->mfCount > mfImpactDuration) {
			it = mlstImpactCounts.erase(it);
		} else {
			++it;
		}
	}
}

//-----------------------------------------------------------------------

} // namespace hpl
