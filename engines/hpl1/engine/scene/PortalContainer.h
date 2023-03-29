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

#ifndef HPL_PORTAL_CONTAINER_H
#define HPL_PORTAL_CONTAINER_H

#include "common/list.h"
#include "hpl1/engine/graphics/Renderable.h"
#include "hpl1/engine/math/BoundingVolume.h"
#include "hpl1/engine/scene/RenderableContainer.h"
#include "hpl1/std/map.h"
#include "hpl1/std/set.h"

namespace hpl {

class iLight3D;
class cSectorVisibilityContainer;

typedef Common::List<iRenderable *> tRenderableList;
typedef Common::List<iRenderable *>::iterator tRenderableListIt;

typedef Hpl1::Std::set<iRenderable *> tRenderableSet;
typedef Hpl1::Std::set<iRenderable *>::iterator tRenderableSetIt;

typedef Hpl1::Std::set<iEntity3D *> tEntity3DSet;
typedef Hpl1::Std::set<iEntity3D *>::iterator tEntity3DSetIt;

//----------------------------------------------------

class cPortalContainer;
class cSector;

//----------------------------------------------------

typedef Hpl1::Std::map<tString, cSector *> tSectorMap;
typedef Hpl1::Std::map<tString, cSector *>::iterator tSectorMapIt;

//----------------------------------------------------

class cPortalContainerEntityIterator {
public:
	cPortalContainerEntityIterator(cPortalContainer *apContainer,
								   cBoundingVolume *apBV);

	bool HasNext();

	iEntity3D *Next();

private:
	cPortalContainer *mpContainer;
	cBoundingVolume *mpBV;

	tSectorMap *mpSectorMap;
	tEntity3DSet *mpEntity3DSet;

	tEntity3DSetIt mEntityIt;
	tSectorMapIt mSectorIt;

	tEntity3DSet mIteratedSet;

	int mlIteratorCount;

	bool mbGlobal;
};

//----------------------------------------------------

class cPortalContainerCallback : public iEntityCallback {
public:
	cPortalContainerCallback(cPortalContainer *apContainer);

	void OnTransformUpdate(iEntity3D *apEntity);

private:
	cPortalContainer *mpContainer;
};

//----------------------------------------------------

class cPortalContainerEntityCallback : public iEntityCallback {
public:
	cPortalContainerEntityCallback(cPortalContainer *apContainer);

	void OnTransformUpdate(iEntity3D *apEntity);

private:
	cPortalContainer *mpContainer;
};

//----------------------------------------------------

class cSector;
class cPortal;

typedef Hpl1::Std::map<int, cPortal *> tPortalMap;
typedef Hpl1::Std::map<int, cPortal *>::iterator tPortalMapIt;

typedef Common::List<cPortal *> tPortalList;
typedef Common::List<cPortal *>::iterator tPortalListIt;

class cPortal {
	friend class cSector;

public:
	cPortal(int alId, cPortalContainer *apContainer);
	~cPortal();

	void SetTargetSector(tString asSectorId);
	cSector *GetTargetSector();

	cSector *GetSector();

	void AddPortalId(int alId);
	void SetNormal(const cVector3f &avNormal);
	void AddPoint(const cVector3f &avPoint);
	void SetTransform(const cMatrixf &a_mtxTrans);

	void Compile();

	bool IsVisible(cFrustum *apFrustum);

	tPortalList *GetPortalList();

	// Debug stuffs
	cBoundingVolume *GetBV() { return &mBV; }
	cVector3f GetNormal() { return mvNormal; }
	int GetId() { return mlId; }
	cPlanef &GetPlane() { return mPlane; }

	bool GetActive() { return mbActive; }
	void SetActive(bool abX) { mbActive = abX; }

private:
	cPortalContainer *mpContainer;

	int mlId;
	tString msSectorId;

	tString msTargetSectorId;
	cSector *mpTargetSector;
	cSector *mpSector;

	tIntVec mvPortalIds;
	tPortalList mlstPortals;
	bool mbPortalsNeedUpdate;

	bool mbActive;

	cVector3f mvNormal;
	cPlanef mPlane;
	cBoundingVolume mBV;
	tVector3fList mlstPoints;
};

//----------------------------------------------------

class cSector : public iRenderContainerData {
	friend class cPortalContainer;
	friend class cPortalContainerEntityIterator;

public:
	cSector(tString asId, cPortalContainer *apContainer);
	~cSector();

	bool TryToAdd(iRenderable *apObject, bool abStatic);
	bool TryToAddEntity(iEntity3D *apEntity);

	void AddPortal(cPortal *apPortal);

	void GetVisible(cFrustum *apFrustum, cRenderList *apRenderList, cPortal *apStartPortal);

	void RemoveDynamic(iRenderable *apObject);
	void RemoveEntity(iEntity3D *apEntity);

	cPortal *GetPortal(int alId);

	void SetAmbientColor(const cColor &aAmbient) { mAmbient = aAmbient; }
	const cColor &GetAmbientColor() { return mAmbient; }

	// Debug stuffs
	cBoundingVolume *GetBV() { return &mBV; }
	tPortalList *GetPortalList() { return &mlstPortals; }
	tString &GetId() { return msId; }

private:
	cPortalContainer *mpContainer;

	tString msId;
	cBoundingVolume mBV;

	int mlVisitCount;

	tRenderableSet m_setStaticObjects;
	tRenderableSet m_setDynamicObjects;

	tEntity3DSet m_setEntities;

	tPortalList mlstPortals;

	cColor mAmbient;
};

//----------------------------------------------------

class cPortalContainer : public iRenderableContainer {
	friend class cPortalContainerCallback;
	friend class cPortalContainerEntityCallback;
	friend class cPortalContainerEntityIterator;

public:
	cPortalContainer();
	virtual ~cPortalContainer();

	bool AddEntity(iEntity3D *pEntity);
	bool RemoveEntity(iEntity3D *pEntity);

	bool Add(iRenderable *apRenderable, bool abStatic);
	bool Remove(iRenderable *apRenderable);

	void AddLightShadowCasters(iLight3D *apLight, cFrustum *apFrustum, cRenderList *apRenderList);

	void AddToRenderList(iRenderable *apObject, cFrustum *apFrustum, cRenderList *apRenderList);

	void GetVisible(cFrustum *apFrustum, cRenderList *apRenderList);

	void Compile();

	// Portal Specific Stuff
	/**
	 * Adds a new sector to the container. All sectors must be created before anything else.
	 */
	void AddSector(tString asSectorId);

	/**
	 * Adds a new renderable to a specific sector.This also recalculates the sector's bounding volume.
	 * Object must be (and is considered) static!
	 */
	bool AddToSector(iRenderable *apRenderable, tString asSector);

	/*
	 * Adds a portal to a sector.
	 */
	bool AddPortal(cPortal *apPortal, tString asSector);

	cSector *GetSector(tString asId);

	int GetSectorVisitCount() const { return mlSectorVisitCount; }

	cPortalContainerEntityIterator GetEntityIterator(cBoundingVolume *apBV);

	// Visibility tools
	cSectorVisibilityContainer *CreateVisibiltyFromBV(cBoundingVolume *apBV);
	cSectorVisibilityContainer *CreateVisibiltyFromFrustum(cFrustum *apFrustum);

	// Debug stuff
	tSectorMap *GetSectorMap() { return &m_mapSectors; }
	tStringList *GetVisibleSectorsList() { return &mlstVisibleSectors; }

	tRenderableSet *GetGlobalDynamicObjectSet() { return &m_setGlobalDynamicObjects; }
	tRenderableList *GetGlobalStaticObjectList() { return &mlstGlobalStaticObjects; }

private:
	void ComputeSectorVisibilty(cSectorVisibilityContainer *apContainer);

	tSectorMap m_mapSectors;

	int mlSectorVisitCount;

	cPortalContainerCallback *mpEntityCallback;
	cPortalContainerEntityCallback *mpNormalEntityCallback;

	// List with dynamic objects that are not in any sector.
	tRenderableSet m_setGlobalDynamicObjects;
	// List with static objects that are not in any sector.
	tRenderableList mlstGlobalStaticObjects;

	// Global dynamic entities
	tEntity3DSet m_setGlobalEntities;

	tStringList mlstVisibleSectors;

	int mlEntityIterateCount;
};

} // namespace hpl

#endif // HPL_PORTAL_CONTAINER_H
