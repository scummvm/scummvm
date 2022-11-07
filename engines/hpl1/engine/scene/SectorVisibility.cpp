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

#include "hpl1/engine/scene/SectorVisibility.h"

#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/scene/PortalContainer.h"

#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// PORTAL VISIBILTY SET
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPortalVisibility::cPortalVisibility() {
}

//-----------------------------------------------------------------------

cPortalVisibility::~cPortalVisibility() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PORTAL VISIBILTY SET
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPortalVisibilitySet::cPortalVisibilitySet(cSectorVisibilityContainer *apContainer,
										   cPortalVisibilitySet *apParent) {
	mpParent = apParent;
	mpContainer = apContainer;
}

cPortalVisibilitySet::~cPortalVisibilitySet() {
	STLDeleteAll(mvVisibility);
}

//-----------------------------------------------------------------------

int cPortalVisibilitySet::AddPortalVisibility(cPortal *apPortal) {
	mvVisibility.push_back(hplNew(cPortalVisibility, ()));

	size_t lIdx = mvVisibility.size() - 1;

	mvVisibility[lIdx]->mpPortal = apPortal;

	// Calculate the shadow volume, range is not really need. Just set a high value.
	cShadowVolumeBV *pShadow = apPortal->GetBV()->GetShadowVolume(mpContainer->GetOrigin(), 9999.0f, true);
	if (pShadow) {
		mvVisibility[lIdx]->mShadow = *pShadow;
		mvVisibility[lIdx]->mbNullShadow = false;
	} else {
		mvVisibility[lIdx]->mbNullShadow = true;
	}

	return (int)lIdx;
}

//-----------------------------------------------------------------------

bool cPortalVisibilitySet::PortalExists(cPortal *apPortal) {
	for (size_t i = 0; i < mvVisibility.size(); ++i) {
		if (mvVisibility[i]->mpPortal == apPortal)
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SECTOR VISIBILTY
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSectorVisibility::cSectorVisibility(cSectorVisibilityContainer *apContainer) {
	mpSector = NULL;
	bStart = false;

	mpContainer = apContainer;
}

cSectorVisibility::~cSectorVisibility() {
}

//-----------------------------------------------------------------------

cPortalVisibilitySet *cSectorVisibility::GetSetConnectingFromSector(cSector *apSector) {
	for (size_t i = 0; i < mvVisibiltySets.size(); ++i) {
		if (mvVisibiltySets[i]->GetVisibility(0)->mpPortal->GetSector() == apSector) {
			return mvVisibiltySets[i];
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------

void cSectorVisibility::AddVisibilitySet(cPortalVisibilitySet *apSet) {
	mvVisibiltySets.push_back(apSet);
}

//-----------------------------------------------------------------------

bool cSectorVisibility::PortalExists(cPortal *apPortal) {
	for (size_t i = 0; i < mvVisibiltySets.size(); ++i) {
		if (mvVisibiltySets[i]->PortalExists(apPortal))
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------

bool cSectorVisibility::IntersectionBV(cBoundingVolume *apBV) {
	if (mvVisibiltySets.empty()) {
		// Log("Checking start sector %s\n",mpSector->GetId().c_str());
		return mpContainer->IntersectionBV(apBV, NULL);
	} else {
		// Log("Checking sector %s with %d sets\n", mpSector->GetId().c_str(), mvVisibiltySets.size());

		for (size_t i = 0; i < mvVisibiltySets.size(); ++i) {
			if (mpContainer->IntersectionBV(apBV, mvVisibiltySets[i])) {
				return true;
			}
		}
		return false;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SECTOR VISIBILTY CONTAINER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSectorVisibilityContainer::cSectorVisibilityContainer(eSectorVisibilityType aType) {
	mType = aType;

	mbLog = false;
	mlTabs = 0;
}

cSectorVisibilityContainer::~cSectorVisibilityContainer() {
	STLMapDeleteAll(m_mapSectors);
	STLDeleteAll(mlstPortalVisibilty);
}

//-----------------------------------------------------------------------

cPortalVisibilitySet *cSectorVisibilityContainer::CreatePortalVisibiltySet(cPortalVisibilitySet *apParent) {
	cPortalVisibilitySet *pSet = hplNew(cPortalVisibilitySet, (this, apParent));

	// Add to visibility list.
	mlstPortalVisibilty.push_back(pSet);

	return pSet;
}

//-----------------------------------------------------------------------

cSectorVisibility *cSectorVisibilityContainer::GetSectorVisibilty(cSector *apSector) {
	// Check if the sector has already been checked.
	tSectorVisibilityMapIt it = m_mapSectors.find(apSector);

	// The sector has not been added, create visibility and add.
	if (it == m_mapSectors.end()) {
		if (mbLog)
			Log("%sCreating Visibility sector for '%s'!\n", GetTabs().c_str(), apSector->GetId().c_str());

		cSectorVisibility *pVisSector = hplNew(cSectorVisibility, (this));
		pVisSector->mpSector = apSector;

		m_mapSectors.insert(tSectorVisibilityMap::value_type(apSector, pVisSector));

		return pVisSector;
	}
	// The sector exists, return it.
	else {
		if (mbLog)
			Log("%sVisibility sector for '%s' already exist!\n", GetTabs().c_str(), apSector->GetId().c_str());
		return it->second;
	}
}

//-----------------------------------------------------------------------

void cSectorVisibilityContainer::Compute(cPortalContainer *apContainer) {
	/////////////////////////////////////
	// Check what sectors the type starts in
	// Check the portals in these
	tSectorMap *pSectorMap = apContainer->GetSectorMap();

	if (mbLog)
		Log("Checking for start sectors\n");

	// Clear start sectors
	m_setStartSectors.clear();

	// Get the origin.
	if (mType == eSectorVisibilityType_BV)
		mvOrigin = mBoundingVolume.GetPosition();
	else if (mType == eSectorVisibilityType_Frustum)
		mvOrigin = mFrustum.GetOrigin();

	///////////////////////////////////
	// Check what start start sectors are
	tSectorMapIt it = pSectorMap->begin();
	for (; it != pSectorMap->end(); ++it) {
		cSector *pSector = it->second;

		if (cMath::PointBVCollision(mvOrigin, *pSector->GetBV())) {
			if (mbLog)
				Log("Sector '%s' is a start!\n", pSector->GetId().c_str());

			m_setStartSectors.insert(pSector);
		}
	}

	/////////////////////////////////
	// Iterate the start sectors
	tSectorSetIt startIt = m_setStartSectors.begin();
	for (; startIt != m_setStartSectors.end(); ++startIt) {
		cSector *pSector = *startIt;
		SearchSector(pSector, NULL, 0);
	}

	if (mbLog)
		Log("Done checking start sectors!\n");
}

tSectorVisibilityIterator cSectorVisibilityContainer::GetSectorIterator() {
	return tSectorVisibilityIterator(&m_mapSectors);
}

//-----------------------------------------------------------------------

void cSectorVisibilityContainer::SearchSector(cSector *apSector, cPortalVisibilitySet *apParentSet,
											  int alPortalIndex) {
	if (mbLog) {
		Log("%s--- Searching sector %s\n%s---------------------------------------\n",
			GetTabs().c_str(),
			apSector->GetId().c_str(),
			GetTabs().c_str());
		mlTabs++;
	}

	cSectorVisibility *pVisSector = GetSectorVisibilty(apSector);

	// Save all portals encountered here.
	tPortalList lstNewPortals;

	//////////////////////////////////
	// Go through all portals and see which
	tPortalList *pPortalList = NULL;

	// Get the portals to search
	if (apParentSet) {
		if (mbLog)
			Log("%sSearching portals from parent portal %d with index %d\n", GetTabs().c_str(), apParentSet->GetVisibility(alPortalIndex)->mpPortal->GetId(), alPortalIndex);
		pPortalList = apParentSet->GetVisibility(alPortalIndex)->mpPortal->GetPortalList();
	} else {
		if (mbLog)
			Log("%sNo parent set, searching all portals.\n", GetTabs().c_str());
		pPortalList = apSector->GetPortalList();
	}

	// Iterate the portals
	tPortalListIt it = pPortalList->begin();
	for (; it != pPortalList->end(); ++it) {
		cPortal *pPortal = *it;
		cSector *pTargetSector = pPortal->GetTargetSector();

		// Check if it is a start sector
		if (m_setStartSectors.find(pTargetSector) != m_setStartSectors.end()) {
			continue;
		}

		/////////////////////////////////////////
		// Check that the portal that it is intersected and does not already exist
		if (pVisSector->PortalExists(pPortal) == false &&
			IntersectionBV(pPortal->GetBV(), apParentSet) &&
			pPortal->GetActive()) {
			if (mbLog)
				Log("%sFound new valid portal %d\n", GetTabs().c_str(), pPortal->GetId());

			///////////////////////////////////////////////////////
			// Check if the portal is facing the right direction
			if (cMath::PlaneToPointDist(pPortal->GetPlane(), mvOrigin) < 0.0f) {
				continue;
			}

			cSectorVisibility *pTargetVisSector = GetSectorVisibilty(pTargetSector);

			////////////////////////////////////////
			// Check if there is another visibility set connecting to the same room
			cPortalVisibilitySet *pSet = pTargetVisSector->GetSetConnectingFromSector(pTargetSector);

			// If none exist, create new set.
			if (pSet == NULL) {
				// Create portal visibility and add it to the sector container
				pSet = CreatePortalVisibiltySet(apParentSet);
				// Add to the specific container
				pTargetVisSector->AddVisibilitySet(pSet);

				if (mbLog)
					Log("%sNo portal connecting from %s to %s, creating new visibility set\n",
						GetTabs().c_str(),
						apSector->GetId().c_str(),
						pTargetSector->GetId().c_str());
			}

			int lIdx = pSet->AddPortalVisibility(pPortal);

			SearchSector(pTargetSector, pSet, lIdx);
		} else {
			if (mbLog)
				Log("%sSkipped unvalid portal %d\n", GetTabs().c_str(), pPortal->GetId());
		}
	}

	if (mbLog) {
		mlTabs--;
		Log("%s------------------------------------\n%s--- Done searching sector %s!\n",
			GetTabs().c_str(),
			GetTabs().c_str(),
			apSector->GetId().c_str());
	}
}

//-----------------------------------------------------------------------

bool cSectorVisibilityContainer::IntersectionBV(cBoundingVolume *apBV, cPortalVisibilitySet *pVisibilitySet) {
	///////////////////////////////////////////
	// First check with the portal visibility
	if (pVisibilitySet) {
		cPortalVisibilitySet *pSet = pVisibilitySet;

		while (pSet) {
			bool bIntersection = false;

			size_t lVisibilityNum = pSet->GetVisibilityNum();
			for (size_t i = 0; i < lVisibilityNum; ++i) {
				if (pSet->GetVisibility(i)->mbNullShadow ||
					pSet->GetVisibility(i)->mShadow.CollideBoundingVolume(apBV)) {
					bIntersection = true;
					break;
				}
			}

			if (bIntersection == false)
				return false;

			pSet = pSet->GetParent();
		}
	}

	//////////////////////////////////////////
	// Check with the type
	// BV:
	if (mType == eSectorVisibilityType_BV) {
		return cMath::CheckCollisionBV(mBoundingVolume, *apBV);
	}
	// Frustum:
	else {
		return mFrustum.CollideBoundingVolume(apBV) != eFrustumCollision_Outside;
	}
}

//-----------------------------------------------------------------------

tString cSectorVisibilityContainer::GetTabs() {
	tString sTabs = "";
	for (int i = 0; i < mlTabs; ++i)
		sTabs += "  ";

	return sTabs;
}

//-----------------------------------------------------------------------

} // namespace hpl
