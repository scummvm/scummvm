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

#include "hpl1/engine/scene/PortalContainer.h"

#include "hpl1/engine/graphics/RenderList.h"
#include "hpl1/engine/graphics/Renderable.h"
#include "hpl1/engine/math/Frustum.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/scene/Light3D.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/scene/SectorVisibility.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// ENTITY ITERATOR
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPortalContainerEntityIterator::cPortalContainerEntityIterator(cPortalContainer *apContainer,
															   cBoundingVolume *apBV) {
	mpContainer = apContainer;
	mpBV = apBV;
	mbGlobal = true;

	mpSectorMap = &mpContainer->m_mapSectors;

	mEntityIt = mpContainer->m_setGlobalEntities.begin();

	if (mEntityIt == mpContainer->m_setGlobalEntities.end()) {
		mbGlobal = false;
	}

	// Get first sector with entities
	mSectorIt = mpContainer->m_mapSectors.begin();
	if (mSectorIt != mpContainer->m_mapSectors.end() &&
		((mSectorIt->second)->m_setEntities.empty() ||
		 !cMath::CheckCollisionBV(*mpBV, (mSectorIt->second)->mBV))) {
		for (; mSectorIt != mpContainer->m_mapSectors.end(); ++mSectorIt) {
			cSector *pSector = mSectorIt->second;
			if ((mSectorIt->second)->m_setEntities.empty() == false &&
				cMath::CheckCollisionBV(*mpBV, pSector->mBV)) {
				break;
			}
		}
	}

	if (mbGlobal == false && mSectorIt != apContainer->m_mapSectors.end()) {
		mpEntity3DSet = &(mSectorIt->second)->m_setEntities;
		mEntityIt = mpEntity3DSet->begin();
	}

	// Update the update count-
	++mpContainer->mlSectorVisitCount;
	mlIteratorCount = mpContainer->mlSectorVisitCount;
}

//-----------------------------------------------------------------------

bool cPortalContainerEntityIterator::HasNext() {
	if (mbGlobal == false && mSectorIt == mpContainer->m_mapSectors.end())
		return false;

	return true;
}

//-----------------------------------------------------------------------

iEntity3D *cPortalContainerEntityIterator::Next() {
	iEntity3D *pEntity = *mEntityIt;
	pEntity->SetIteratorCount(mlIteratorCount);

	++mEntityIt;

	bool bNextEntity = false;
	do {
		////////////////////////////
		// Search Global
		if (mbGlobal) {
			if (mEntityIt == mpContainer->m_setGlobalEntities.end()) {
				mbGlobal = false;
				// If there are no sectors, just return the entity.
				if (mSectorIt == mpContainer->m_mapSectors.end())
					return pEntity;

				mpEntity3DSet = &(mSectorIt->second)->m_setEntities;
				mEntityIt = mpEntity3DSet->begin();
			}
		}
		////////////////////////////7
		// Search Sectors
		else {
			if (mEntityIt == mpEntity3DSet->end()) {
				++mSectorIt;
				if (mSectorIt != mpContainer->m_mapSectors.end() &&
					((mSectorIt->second)->m_setEntities.empty() ||
					 !cMath::CheckCollisionBV(*mpBV, (mSectorIt->second)->mBV))) {
					for (; mSectorIt != mpContainer->m_mapSectors.end(); ++mSectorIt) {
						cSector *pSector = mSectorIt->second;
						if (pSector->m_setEntities.empty() == false &&
							cMath::CheckCollisionBV(*mpBV, pSector->mBV)) {
							break;
						}
					}
				}

				if (mSectorIt != mpContainer->m_mapSectors.end()) {
					mpEntity3DSet = &(mSectorIt->second)->m_setEntities;
					mEntityIt = mpEntity3DSet->begin();
				}
			}
		}

		bNextEntity = true;
		if (mbGlobal == false && mSectorIt == mpContainer->m_mapSectors.end())
			bNextEntity = false;
		else if ((*mEntityIt)->GetIteratorCount() != mlIteratorCount)
			bNextEntity = false;

		if (bNextEntity)
			++mEntityIt;
	} while (bNextEntity);

	return pEntity;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PORTAL
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPortal::cPortal(int alId, cPortalContainer *apContainer) {
	mlId = alId;
	mpContainer = apContainer;

	mpTargetSector = NULL;

	mbPortalsNeedUpdate = true;

	mbActive = true;
}

cPortal::~cPortal() {
}

//-----------------------------------------------------------------------

void cPortal::SetTargetSector(tString asSectorId) {
	msTargetSectorId = asSectorId;
}

cSector *cPortal::GetTargetSector() {
	// Set the pointer here so that it becomes more flexible
	// it also eases up loading
	if (mpTargetSector == NULL) {
		mpTargetSector = mpContainer->GetSector(msTargetSectorId);

		if (mpTargetSector == NULL)
			Error("Portal %d in sector %s target sector %s is NOT valid!\n", mlId, msSectorId.c_str(),
				  msTargetSectorId.c_str());
	}

	return mpTargetSector;
}

//-----------------------------------------------------------------------

cSector *cPortal::GetSector() {
	return mpSector;
}

//-----------------------------------------------------------------------

void cPortal::AddPortalId(int alId) {
	mvPortalIds.push_back(alId);
}
void cPortal::SetNormal(const cVector3f &avNormal) {
	mvNormal = avNormal;
}
void cPortal::AddPoint(const cVector3f &avPoint) {
	mlstPoints.push_back(avPoint);
}
void cPortal::SetTransform(const cMatrixf &a_mtxTrans) {
	mBV.SetTransform(a_mtxTrans);
}
//-----------------------------------------------------------------------

bool cPortal::IsVisible(cFrustum *apFrustum) {
	if (mbActive == false)
		return false;

	// Check if the frustum is on the positive side of a plane
	// made from the portal normal and center.
	if (cMath::PlaneToPointDist(mPlane, apFrustum->GetOrigin()) >= 0.0f) {
		// Check if the portal collides with frustum
		if (apFrustum->CollideBoundingVolume(&mBV) != eFrustumCollision_Outside ||
			cMath::CheckCollisionBV(*apFrustum->GetOriginBV(), mBV)) {
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------

tPortalList *cPortal::GetPortalList() {
	if (mbPortalsNeedUpdate) {
		mbPortalsNeedUpdate = false;

		for (size_t i = 0; i < mvPortalIds.size(); i++) {
			cPortal *pPortal = GetTargetSector()->GetPortal(mvPortalIds[i]);

			if (pPortal)
				mlstPortals.push_back(pPortal);
		}
	}

	return &mlstPortals;
}

//-----------------------------------------------------------------------

void cPortal::Compile() {
	////////////////////////////////////////
	// Calculate the bounding volume
	cVector3f vMin = mlstPoints.front();
	cVector3f vMax = mlstPoints.front();

	tVector3fListIt it = mlstPoints.begin();
	for (; it != mlstPoints.end(); it++) {
		cVector3f &vP = *it;

		if (vMax.x < vP.x)
			vMax.x = vP.x;
		else if (vMin.x > vP.x)
			vMax.x = vP.x;
		if (vMax.y < vP.y)
			vMax.y = vP.y;
		else if (vMin.y > vP.y)
			vMax.y = vP.y;
		if (vMax.z < vP.z)
			vMax.z = vP.z;
		else if (vMin.z > vP.z)
			vMax.z = vP.z;
	}

	mBV.SetLocalMinMax(vMin, vMax);

	////////////////////////////////////////
	// Calculate the plane

	mPlane.FromNormalPoint(mvNormal, mBV.GetWorldCenter());
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SECTOR
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSector::cSector(tString asId, cPortalContainer *apContainer) {
	msId = asId;

	mpContainer = apContainer;

	mBV.SetPosition(0);
	mBV.SetLocalMinMax(cVector3f(100000, 100000, 100000), cVector3f(-100000, -100000, -100000));

	mlVisitCount = -1;

	mAmbient = cColor(1, 1);
}

//-----------------------------------------------------------------------

cSector::~cSector() {
	STLDeleteAll(mlstPortals);
}

//-----------------------------------------------------------------------

void cSector::AddPortal(cPortal *apPortal) {
	apPortal->msSectorId = msId;
	apPortal->mpSector = this;

	mlstPortals.push_back(apPortal);

	cVector3f vObjectMax = apPortal->GetBV()->GetMax();
	cVector3f vObjectMin = apPortal->GetBV()->GetMin();

	cVector3f vMin = mBV.GetLocalMin();
	cVector3f vMax = mBV.GetLocalMax();

	// Check if the bounding volume should be expanded.
	if (vMax.x < vObjectMax.x)
		vMax.x = vObjectMax.x;
	if (vMax.y < vObjectMax.y)
		vMax.y = vObjectMax.y;
	if (vMax.z < vObjectMax.z)
		vMax.z = vObjectMax.z;

	if (vMin.x > vObjectMin.x)
		vMin.x = vObjectMin.x;
	if (vMin.y > vObjectMin.y)
		vMin.y = vObjectMin.y;
	if (vMin.z > vObjectMin.z)
		vMin.z = vObjectMin.z;

	mBV.SetLocalMinMax(vMin, vMax);
}

//-----------------------------------------------------------------------

cPortal *cSector::GetPortal(int alId) {
	tPortalListIt it = mlstPortals.begin();
	for (; it != mlstPortals.end(); ++it) {
		cPortal *pPortal = *it;
		if (pPortal->mlId == alId)
			return pPortal;
	}

	return NULL;
}

//-----------------------------------------------------------------------

bool cSector::TryToAdd(iRenderable *apObject, bool abStatic) {
	// bool bLog=true;
	// if(bLog) Log("-- Trying to add %s to sector '%s'\n",apObject->GetName().c_str(), msId.c_str());

	// Check if the objects collides with the sector
	// If so add it.
	if (apObject->CollidesWithBV(&mBV)) {
		if (abStatic) {
			apObject->GetRenderContainerDataList()->push_back(this);
			// if(bLog) Log("   Adding as static! Sectors: %d\n",apObject->GetRenderContainerDataList()->size());
			// Add as static object.
			m_setStaticObjects.insert(apObject);

			// Set this sector as data in the container data list.
			// This is useful for culling later on.
			apObject->GetRenderContainerDataList()->push_back(this);
		} else {
			// Set this sector as data in the container data list.
			apObject->GetRenderContainerDataList()->push_back(this);
			// if(bLog) Log("   Adding as dynamic!\n");

			// Log("Adding dynamic %d %s\n",(size_t)apObject,apObject->GetName().c_str());
			// Add as a dynamic object
			m_setDynamicObjects.insert(apObject);
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------

bool cSector::TryToAddEntity(iEntity3D *apEntity) {
	// bool bLog=false;
	// if(bLog) Log("-- Trying to add %s to sector '%s'\n",apEntity->GetName().c_str(), msId.c_str());

	// Check if the objects collides with the sector
	// If so add it.
	if (cMath::CheckCollisionBV(*apEntity->GetBoundingVolume(), mBV)) {
		// if(bLog) Log("-- Adding as dynamic!\n");
		// Set this sector as data in the container data list.
		apEntity->GetRenderContainerDataList()->push_back(this);

		// Log("Adding dynamic %d %s\n",(size_t)apObject,apObject->GetName().c_str());
		// Add as a dynamic object
		m_setEntities.insert(apEntity);

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------

void cSector::RemoveDynamic(iRenderable *apObject) {
	m_setDynamicObjects.erase(apObject);
}

//-----------------------------------------------------------------------

void cSector::RemoveEntity(iEntity3D *apEntity) {
	m_setEntities.erase(apEntity);
}

//-----------------------------------------------------------------------

void cSector::GetVisible(cFrustum *apFrustum, cRenderList *apRenderList, cPortal *apStartPortal) {
	// Set the sector as visited.
	mlVisitCount = mpContainer->GetSectorVisitCount();

	mpContainer->GetVisibleSectorsList()->push_back(msId);

	//////////////////////////////////////////////////////
	// Add all visible objects in the room to the render list
	// Static
	tRenderableSetIt it = m_setStaticObjects.begin();
	for (; it != m_setStaticObjects.end(); ++it) {
		iRenderable *pObject = *it;

		if (pObject->CollidesWithFrustum(apFrustum)) {
			mpContainer->AddToRenderList(pObject, apFrustum, apRenderList);
		}
	}
	// Dynamic
	it = m_setDynamicObjects.begin();
	for (; it != m_setDynamicObjects.end(); ++it) {
		iRenderable *pObject = *it;

		if (pObject->CollidesWithFrustum(apFrustum)) {
			mpContainer->AddToRenderList(pObject, apFrustum, apRenderList);
		}
	}

	/////////////////////////////////////////////
	// Iterate all portals and and process them.
	tPortalListIt PortIt;
	tPortalListIt PortEnd;

	// If this room is seen looking through a portal, get the portals seen
	if (apStartPortal) {
		tPortalList *pPortList = apStartPortal->GetPortalList();
		PortIt = pPortList->begin();
		PortEnd = pPortList->end();
	}
	// If you are in in the center of the room, check all portals.
	else {
		PortIt = mlstPortals.begin();
		PortEnd = mlstPortals.end();
	}

	for (; PortIt != PortEnd; ++PortIt) {
		cPortal *pPortal = *PortIt;
		cSector *pTargetSector = pPortal->GetTargetSector();

		if (pTargetSector == NULL)
			continue;

		// If sector has been visited, skip it
		if (pTargetSector->mlVisitCount == mpContainer->GetSectorVisitCount())
			continue;

		if (pPortal->IsVisible(apFrustum)) {
			pTargetSector->GetVisible(apFrustum, apRenderList, pPortal);
		}
	}
}

//-----------------------------------------------------------------------

bool gbCallbackActive = true;

//////////////////////////////////////////////////////////////////////////
// PORTAL CONTAINER ENTITY CALLBACK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPortalContainerEntityCallback::cPortalContainerEntityCallback(cPortalContainer *apContainer) {
	mpContainer = apContainer;
}

//-----------------------------------------------------------------------

void cPortalContainerEntityCallback::OnTransformUpdate(iEntity3D *apEntity) {
	if (gbCallbackActive == false)
		return;

	tRenderContainerDataList *pDataList = apEntity->GetRenderContainerDataList();

	// Log("Removing %s from container\n",apEntity->GetName().c_str());
	//  If empty then the object is in the global list.
	if (pDataList->empty()) {
		mpContainer->m_setGlobalEntities.erase(apEntity);
	}
	// The object is in one or more sectors
	else {
		// Iterate the sectors and remove the object from them.
		tRenderContainerDataListIt it = pDataList->begin();
		for (; it != pDataList->end(); ++it) {
			cSector *pSector = static_cast<cSector *>(*it);
			pSector->RemoveEntity(apEntity);
			// Log(" Removing %s to sector %s\n",	apEntity->GetName().c_str(),
			//									pSector->GetId().c_str());
		}

		// Clear the data list.
		pDataList->clear();
	}

	// Check what new sectors the object belong to.
	bool bAdded = false;

	tSectorMapIt it = mpContainer->m_mapSectors.begin();
	for (; it != mpContainer->m_mapSectors.end(); ++it) {
		cSector *pSector = it->second;
		if (pSector->TryToAddEntity(apEntity)) {
			// Log(" Adding %s to sector %s\n",apEntity->GetName().c_str(),
			//								pSector->GetId().c_str());
			bAdded = true;
		}
	}

	// If not added in any sector, add to global list.
	if (bAdded == false) {
		mpContainer->m_setGlobalEntities.insert(apEntity);
	}
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PORTAL CONTAINER CALLBACK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPortalContainerCallback::cPortalContainerCallback(cPortalContainer *apContainer) {
	mpContainer = apContainer;
}

//-----------------------------------------------------------------------

void cPortalContainerCallback::OnTransformUpdate(iEntity3D *apEntity) {
	if (gbCallbackActive == false)
		return;

	// Get the renderable and retrieve the render container data list.
	iRenderable *apRenderable = static_cast<iRenderable *>(apEntity);
	tRenderContainerDataList *pDataList = apRenderable->GetRenderContainerDataList();

	// Log("Removing %s from container\n",apRenderable->GetName().c_str());
	//  If empty then the object is in the global list.
	if (pDataList->empty()) {
		mpContainer->m_setGlobalDynamicObjects.erase(apRenderable);
	}
	// The object is in one or more sectors
	else {
		// Iterate the sectors and remove the object from them.
		tRenderContainerDataListIt it = pDataList->begin();
		for (; it != pDataList->end(); ++it) {
			cSector *pSector = static_cast<cSector *>(*it);
			pSector->RemoveDynamic(apRenderable);

			// Log("Removed from sector %s\n",pSector->GetId().c_str());
		}

		// Clear the data list.
		pDataList->clear();
	}

	// Check what new sectors the object belong to.
	bool bAdded = false;

	// Setting NULL as center sector.
	apEntity->SetCurrentSector(NULL);
	cVector3f vEntityWorldPos = apRenderable->GetBoundingVolume()->GetWorldCenter();
	bool bFoundCenter = false;

	// Log("Setting NULL to '%s'\n", apEntity->GetName().c_str());

	tSectorMapIt it = mpContainer->m_mapSectors.begin();
	for (; it != mpContainer->m_mapSectors.end(); ++it) {
		cSector *pSector = it->second;
		if (pSector->TryToAdd(apRenderable, false)) {
			bAdded = true;
		}

		if (bFoundCenter == false) {
			if (cMath::PointBVCollision(vEntityWorldPos, *pSector->GetBV())) {
				apEntity->SetCurrentSector(pSector);
				bFoundCenter = true;
				// Log("Setting sector %d to '%s'\n",apEntity->GetCurrentSector(),apEntity->GetName().c_str());
			} else if (cMath::CheckCollisionBV(*apEntity->GetBoundingVolume(), *pSector->GetBV())) {
				apEntity->SetCurrentSector(pSector);
			}
		}
	}

	// If not added in any sector, add to global list.
	if (bAdded == false) {
		mpContainer->m_setGlobalDynamicObjects.insert(apRenderable);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPortalContainer::cPortalContainer() {
	mpEntityCallback = hplNew(cPortalContainerCallback, (this));
	mpNormalEntityCallback = hplNew(cPortalContainerEntityCallback, (this));

	mlSectorVisitCount = 0;

	mlEntityIterateCount = 0;
}

//-----------------------------------------------------------------------

cPortalContainer::~cPortalContainer() {
	hplDelete(mpEntityCallback);
	hplDelete(mpNormalEntityCallback);

	STLMapDeleteAll(m_mapSectors);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cPortalContainer::Add(iRenderable *apRenderable, bool abStatic) {
	if (apRenderable == NULL) {
		Warning("Trying to add NULL object to portal container!\n");
		return false;
	}

	bool bLog = false;

	if (bLog)
		Log("-------------\n");
	if (bLog)
		Log("Adding in portal container: %s\n", apRenderable->GetName().c_str());

	bool bAdded = false;
	if (abStatic) {
		// Set the center sector to NULL
		apRenderable->SetCurrentSector(NULL);
		cVector3f vEntityWorldPos = apRenderable->GetBoundingVolume()->GetWorldCenter();
		bool bFoundCenter = false;
		// Log("Setting center sector to NULL to '%s'\n",apRenderable->GetName().c_str());

		// Try to add it to all sectors the renderable touches
		tSectorMapIt it = m_mapSectors.begin();
		for (; it != m_mapSectors.end(); it++) {
			cSector *pSector = it->second;

			if (pSector->TryToAdd(apRenderable, true)) {
				bAdded = true;
				// Log("Adding as static in sector %s\n",pSector->GetId().c_str());
			}
			// Check if center is in this portal.
			if (bFoundCenter == false) {
				if (cMath::PointBVCollision(vEntityWorldPos, *pSector->GetBV())) {
					apRenderable->SetCurrentSector(pSector);
					bFoundCenter = true;
					// Log("Setting sector %d to '%s'\n",apEntity->GetCurrentSector(),apEntity->GetName().c_str());
				} else if (cMath::CheckCollisionBV(*apRenderable->GetBoundingVolume(), *pSector->GetBV())) {
					apRenderable->SetCurrentSector(pSector);
				}
			}
		}

		// If not added in any sector, add to global list.
		if (bAdded == false) {
			mlstGlobalStaticObjects.push_back(apRenderable);
			// Log("Adding as static in global\n");
		}
	} else {
		// Set the center sector to NULL
		apRenderable->SetCurrentSector(NULL);
		cVector3f vEntityWorldPos = apRenderable->GetBoundingVolume()->GetWorldCenter();
		bool bFoundCenter = false;

		// Add a callback so that the sectors the belongs to are changed
		// when it moves.
		// Only add it if there are any sectors... otherwise it is pointless.
		if (m_mapSectors.empty() == false)
			apRenderable->AddCallback(mpEntityCallback);

		// Try to add it to all sectors the renderable touches
		tSectorMapIt it = m_mapSectors.begin();
		for (; it != m_mapSectors.end(); it++) {
			cSector *pSector = it->second;

			if (pSector->TryToAdd(apRenderable, false)) {
				bAdded = true;
				if (bLog)
					Log("Adding as dynamic in sector %s\n", pSector->GetId().c_str());
			}

			// Check if center is in this portal.
			if (bFoundCenter == false) {
				if (cMath::PointBVCollision(vEntityWorldPos, *pSector->GetBV())) {
					apRenderable->SetCurrentSector(pSector);
					bFoundCenter = true;
					// Log("Setting sector %d to '%s'\n",apEntity->GetCurrentSector(),apEntity->GetName().c_str());
				} else if (cMath::CheckCollisionBV(*apRenderable->GetBoundingVolume(), *pSector->GetBV())) {
					apRenderable->SetCurrentSector(pSector);
				}
			}
		}

		// If not added in any sector, add to global list.
		if (bAdded == false) {
			m_setGlobalDynamicObjects.insert(apRenderable);
		}
	}

	if (bLog)
		Log("-------------\n");

	return true;
}

//-----------------------------------------------------------------------

bool cPortalContainer::Remove(iRenderable *apRenderable) {
	// Log("Removing %d %s\n",(size_t)apRenderable,apRenderable->GetName().c_str());

	/// Log("Trying to remove: %s\n",apRenderable->GetName().c_str());
	tRenderContainerDataList *pDataList = apRenderable->GetRenderContainerDataList();

	// If empty then the object is in the global list.
	if (pDataList->empty()) {
		m_setGlobalDynamicObjects.erase(apRenderable);
		// Log("Data list empty!\n");
	}
	// The object is in one or more sectors
	else {
		// Iterate the sectors and remove the object from them.
		tRenderContainerDataListIt it = pDataList->begin();
		for (; it != pDataList->end(); ++it) {
			cSector *pSector = static_cast<cSector *>(*it);
			pSector->RemoveDynamic(apRenderable);
			// Log("Removed from sector '%s'\n",pSector->GetId().c_str());
		}

		// Clear the data list.
		pDataList->clear();
	}

	return true;
}

//-----------------------------------------------------------------------

bool cPortalContainer::AddEntity(iEntity3D *apEntity) {
	if (apEntity == NULL) {
		Warning("Trying to add NULL object to portal container!\n");
		return false;
	}

	bool bLog = false;

	if (bLog)
		Log("-------------\n");
	if (bLog)
		Log("Adding in portal container: %s\n", apEntity->GetName().c_str());

	bool bAdded = false;
	// Add a callback so that the sectors the belongs to are changed
	// when it moves.
	// Only add it if there are any sectors... otherwise it is pointless.
	if (m_mapSectors.empty() == false) {
		if (bLog)
			Log(" Adding callback for %s\n", apEntity->GetName().c_str());
		apEntity->AddCallback(mpNormalEntityCallback);
	}

	// Try to add it to all sectors the renderable touches
	tSectorMapIt it = m_mapSectors.begin();
	for (; it != m_mapSectors.end(); it++) {
		cSector *pSector = it->second;

		if (pSector->TryToAddEntity(apEntity)) {
			bAdded = true;
			if (bLog)
				Log(" Adding as dynamic in sector %s\n", pSector->GetId().c_str());
		}
	}

	// If not added in any sector, add to global list.
	if (bAdded == false) {
		m_setGlobalEntities.insert(apEntity);
		if (bLog)
			Log(" Adding as Global\n");
	}

	if (bLog)
		Log("-------------\n");

	return true;
}

//-----------------------------------------------------------------------

bool cPortalContainer::RemoveEntity(iEntity3D *apEntity) {
	tRenderContainerDataList *pDataList = apEntity->GetRenderContainerDataList();

	// If empty then the object is in the global list.
	if (pDataList->empty()) {
		m_setGlobalEntities.erase(apEntity);
	}
	// The object is in one or more sectors
	else {
		// Iterate the sectors and remove the object from them.
		tRenderContainerDataListIt it = pDataList->begin();
		for (; it != pDataList->end(); ++it) {
			cSector *pSector = static_cast<cSector *>(*it);
			pSector->RemoveEntity(apEntity);
		}

		// Clear the data list.
		pDataList->clear();
	}

	return true;
}

//-----------------------------------------------------------------------

void cPortalContainer::AddLightShadowCasters(iLight3D *apLight, cFrustum *apFrustum, cRenderList *apRenderList) {
	const bool bLog = false;

	if (bLog)
		Log("Checking for shadow casters in '%s'!\n", apLight->GetName().c_str());

	if (apLight->GetCastShadows() == false)
		return;

	if (bLog)
		Log("Found one!\n");

	tRenderContainerDataList *pDataList = apLight->GetRenderContainerDataList();

	apLight->ClearCasters(apLight->IsStatic() ? false : true);

	// The light is not in any sector
	if (pDataList->empty()) {
		if (bLog)
			Log("Checking global!\n");

		// Do not add more if all static has already been added.
		if (!(apLight->IsStatic() && apLight->AllStaticCastersAdded())) {
			tRenderableListIt it = mlstGlobalStaticObjects.begin();
			for (; it != mlstGlobalStaticObjects.end(); it++)
				apLight->AddShadowCaster(*it, apFrustum, true, apRenderList);
		}

		// Add Dynamic objects
		tRenderableSetIt it = m_setGlobalDynamicObjects.begin();
		for (; it != m_setGlobalDynamicObjects.end(); it++)
			apLight->AddShadowCaster(*it, apFrustum, false, apRenderList);

	} else {
		if (bLog)
			Log("Checking sectors!\n");

		// Iterate the sectors and check for shadow casters.
		tRenderContainerDataListIt it = pDataList->begin();
		for (; it != pDataList->end(); ++it) {
			cSector *pSector = static_cast<cSector *>(*it);

			if (bLog)
				Log("SECTOR: %s\n", pSector->GetId().c_str());

			// If the light is static and the static list is not filled yet.
			if (!(apLight->IsStatic() && apLight->AllStaticCastersAdded())) {
				tRenderableSetIt it = pSector->m_setStaticObjects.begin();
				for (; it != pSector->m_setStaticObjects.end(); ++it) {
					iRenderable *pR = *it;
					if (bLog)
						Log("Adding static '%s' type: %s\n", pR->GetName().c_str(), pR->GetEntityType().c_str());
					apLight->AddShadowCaster(pR, apFrustum, true, apRenderList);
				}
			}

			// Add dynamic objects
			tRenderableSetIt it = pSector->m_setDynamicObjects.begin();
			for (; it != pSector->m_setDynamicObjects.end(); ++it) {
				iRenderable *pR = *it;
				if (bLog)
					Log("Adding dynamic '%s' type: %s\n", pR->GetName().c_str(), pR->GetEntityType().c_str());
				apLight->AddShadowCaster(pR, apFrustum, false, apRenderList);
			}
		}
	}

	if (apLight->IsStatic()) {
		apLight->SetAllStaticCastersAdded(true);
	}
}

//-----------------------------------------------------------------------

void cPortalContainer::AddToRenderList(iRenderable *apObject, cFrustum *apFrustum,
									   cRenderList *apRenderList) {
	// If the light was added and it was the first time,
	// add shadow casters.
	if (apRenderList->Add(apObject)) {
		if (apObject->GetRenderType() == eRenderableType_Light) {
			AddLightShadowCasters(static_cast<iLight3D *>(apObject), apFrustum, apRenderList);
		}
	}
}

//-----------------------------------------------------------------------

void cPortalContainer::GetVisible(cFrustum *apFrustum, cRenderList *apRenderList) {
	gbCallbackActive = false;

	// Clear debug
	mlstVisibleSectors.clear();

	////////////////////////////////////////////////
	// Get a container with all the visible sectors
	cSectorVisibilityContainer *pVisSectorCont = CreateVisibiltyFromFrustum(apFrustum);

	// Iterate visible sectors, check for intersection with object and add the valid ones.
	tSectorVisibilityIterator SectorIt = pVisSectorCont->GetSectorIterator();
	while (SectorIt.HasNext()) {
		cSectorVisibility *pVisSector = SectorIt.Next();
		cSector *pSector = pVisSector->GetSector();

		mlstVisibleSectors.push_back(pSector->GetId());

		//////////////////////////////////////////////////////
		// Add all visible objects in the sector to the render list
		// Static
		tRenderableSetIt it = pSector->m_setStaticObjects.begin();
		for (; it != pSector->m_setStaticObjects.end(); ++it) {
			iRenderable *pObject = *it;

			if (pVisSector->IntersectionBV(pObject->GetBoundingVolume())) {
				AddToRenderList(pObject, apFrustum, apRenderList);
			}
		}
		// Dynamic
		// Log("-------START------\n");
		it = pSector->m_setDynamicObjects.begin();
		for (; it != pSector->m_setDynamicObjects.end(); ++it) {
			iRenderable *pObject = *it;

			// Log("Checking %d\n",(size_t)pObject);

			if (pVisSector->IntersectionBV(pObject->GetBoundingVolume())) {
				AddToRenderList(pObject, apFrustum, apRenderList);
			}
		}
		// Log("------END-------\n");
	}

	//////////////////////////////////////////
	// Add global dynamic objects
	{
		tRenderableSetIt it = m_setGlobalDynamicObjects.begin();
		for (; it != m_setGlobalDynamicObjects.end(); ++it) {
			iRenderable *pObject = *it;
			// Log("Testing %s\n",pObject->GetName().c_str());
			if (pObject->CollidesWithFrustum(apFrustum)) {
				AddToRenderList(pObject, apFrustum, apRenderList);
			}
		}
	}

	//////////////////////////////////////////
	// Add global static objects
	{
		tRenderableListIt it = mlstGlobalStaticObjects.begin();
		for (; it != mlstGlobalStaticObjects.end(); ++it) {
			iRenderable *pObject = *it;

			if (pObject->CollidesWithFrustum(apFrustum)) {
				AddToRenderList(pObject, apFrustum, apRenderList);
			}
		}
	}

	// Delete visible sectors.
	hplDelete(pVisSectorCont);

	gbCallbackActive = true;
}

//-----------------------------------------------------------------------

void cPortalContainer::Compile() {
	/*When octrees are used, they should be compiled here */

	////////////////////////////////////////////////////////
	// Go through all normal entities and update them
	//(this since sectors might have been created  after their creation).

	tEntity3DSet setEntities;

	// Sectors
	tSectorMapIt secIt = m_mapSectors.begin();
	for (; secIt != m_mapSectors.end(); secIt++) {
		cSector *pSector = secIt->second;

		tEntity3DSetIt entIt = pSector->m_setEntities.begin();
		for (; entIt != pSector->m_setEntities.end(); ++entIt) {
			iEntity3D *pEntity = *entIt;
			setEntities.insert(pEntity);
		}
	}

	// Global
	tEntity3DSetIt entIt = m_setGlobalEntities.begin();
	for (; entIt != m_setGlobalEntities.end(); ++entIt) {
		iEntity3D *pEntity = *entIt;
		setEntities.insert(pEntity);
	}

	entIt = setEntities.begin();
	for (; entIt != setEntities.end(); ++entIt) {
		iEntity3D *pEntity = *entIt;
		mpNormalEntityCallback->OnTransformUpdate(pEntity);
	}
}

//-----------------------------------------------------------------------

void cPortalContainer::AddSector(tString asId) {
	cSector *pSector = hplNew(cSector, (asId, this));

	m_mapSectors.insert(tSectorMap::value_type(asId, pSector));
}

//-----------------------------------------------------------------------

bool cPortalContainer::AddToSector(iRenderable *apRenderable, tString asSector) {
	tSectorMapIt it = m_mapSectors.find(asSector);
	if (it == m_mapSectors.end()) {
		Warning("Sector %s not found!\n", asSector.c_str());
		return false;
	}

	cSector *pSector = it->second;

	pSector->m_setStaticObjects.insert(apRenderable);

	// Setting the sector is useful for some culling.
	apRenderable->GetRenderContainerDataList()->push_back(pSector);

	// Set center sector.
	apRenderable->SetCurrentSector(pSector);

	cVector3f vObjectMax = apRenderable->GetBoundingVolume()->GetMax();
	cVector3f vObjectMin = apRenderable->GetBoundingVolume()->GetMin();

	cVector3f vMin = pSector->mBV.GetLocalMin();
	cVector3f vMax = pSector->mBV.GetLocalMax();

	// Check if the bounding volume should be expanded.
	if (vMax.x < vObjectMax.x)
		vMax.x = vObjectMax.x;
	if (vMax.y < vObjectMax.y)
		vMax.y = vObjectMax.y;
	if (vMax.z < vObjectMax.z)
		vMax.z = vObjectMax.z;

	if (vMin.x > vObjectMin.x)
		vMin.x = vObjectMin.x;
	if (vMin.y > vObjectMin.y)
		vMin.y = vObjectMin.y;
	if (vMin.z > vObjectMin.z)
		vMin.z = vObjectMin.z;

	pSector->mBV.SetLocalMinMax(vMin, vMax);
	// Quick fix for thin stuff. (not working it seems)
	// pSector->mBV.SetLocalMinMax(vMin - cVector3f(0.1f), vMax+cVector3f(0.1f));

	return true;
}

//-----------------------------------------------------------------------

bool cPortalContainer::AddPortal(cPortal *apPortal, tString asSector) {
	tSectorMapIt it = m_mapSectors.find(asSector);
	if (it == m_mapSectors.end()) {
		Warning("Sector %s not found!\n", asSector.c_str());
		return false;
	}

	cSector *pSector = it->second;

	pSector->AddPortal(apPortal);

	return true;
}

//-----------------------------------------------------------------------

cSector *cPortalContainer::GetSector(tString asId) {
	tSectorMapIt it = m_mapSectors.find(asId);
	if (it == m_mapSectors.end())
		return NULL;

	return it->second;
}

//-----------------------------------------------------------------------

cPortalContainerEntityIterator cPortalContainer::GetEntityIterator(cBoundingVolume *apBV) {
	return cPortalContainerEntityIterator(this, apBV);
}

//-----------------------------------------------------------------------

cSectorVisibilityContainer *cPortalContainer::CreateVisibiltyFromBV(cBoundingVolume *apBV) {
	cSectorVisibilityContainer *pContainer = hplNew(cSectorVisibilityContainer, (eSectorVisibilityType_BV));
	pContainer->SetBV(*apBV);

	pContainer->Compute(this);

	return pContainer;
}

//-----------------------------------------------------------------------

cSectorVisibilityContainer *cPortalContainer::CreateVisibiltyFromFrustum(cFrustum *apFrustum) {
	cSectorVisibilityContainer *pContainer = hplNew(cSectorVisibilityContainer, (eSectorVisibilityType_Frustum));
	pContainer->SetFrustum(*apFrustum);

	pContainer->Compute(this);

	return pContainer;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// OLD CODE
//////////////////////////////////////////////////////////////////////////

/*void cPortalContainer::GetVisible(cFrustum* apFrustum,cRenderList *apRenderList)
{
	//Clear debug
	mlstVisibleSectors.clear();

	//////////////////////////////////////////////////
	//Find the sector that the camera is in and add the
	//objects in it.
	tSectorMapIt SectorIt = m_mapSectors.begin();
	for(; SectorIt != m_mapSectors.end(); SectorIt++)
	{
		cSector* pSector =  SectorIt->second;

		if(cMath::PointBVCollision(apFrustum->GetOrigin(), *pSector->GetBV()))
		{
			pSector->GetVisible(apFrustum, apRenderList, NULL);
		}
	}

	//Inc sector visit count so next time no sectors will be considered visited.
	mlSectorVisitCount++;

	//////////////////////////////////////////
	//Add global dynamic objects
	{
		tRenderableSetIt it = m_setGlobalDynamicObjects.begin();
		for(;it != m_setGlobalDynamicObjects.end(); ++it)
		{
			iRenderable *pObject = *it;
			//Log("Testing %s\n",pObject->GetName().c_str());
			if(pObject->CollidesWithFrustum(apFrustum))
			{
				//Log("Added %s\n",pObject->GetName().c_str());

				AddToRenderList(pObject,apFrustum,apRenderList);
			}
		}
	}

	//////////////////////////////////////////
	//Add global static objects
	{
		tRenderableListIt it = mlstGlobalStaticObjects.begin();
		for(;it != mlstGlobalStaticObjects.end(); ++it)
		{
			iRenderable *pObject = *it;

			if(pObject->CollidesWithFrustum(apFrustum))
			{
				AddToRenderList(pObject,apFrustum,apRenderList);
			}
		}
	}

	//DEBUG:
	//Test the visible sector stuff
	cSectorVisibilityContainer *pVisSector = CreateVisibiltyFromFrustum(apFrustum);
	hplDelete(pVisSector);
}*/
} // namespace hpl
