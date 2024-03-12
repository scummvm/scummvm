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

#ifndef HPL_SECTOR_VISIBILTY_H
#define HPL_SECTOR_VISIBILTY_H

#include "hpl1/engine/math/BoundingVolume.h"
#include "hpl1/engine/math/Frustum.h"

namespace hpl {

class cPortal;
class cSector;
class cPortalContainer;
class cSectorVisibilityContainer;

enum eSectorVisibilityType {
	eSectorVisibilityType_BV,
	eSectorVisibilityType_Frustum,
};

//----------------------------------------------------

/**
 * Represent the visibility for one portal
 */
class cPortalVisibility {
public:
	cPortalVisibility();
	~cPortalVisibility();

	cShadowVolumeBV mShadow;
	cPortal *mpPortal;
	bool mbNullShadow;
};

//----------------------------------------------------

/**
 * Represent the visibility from one sector to another
 * this may include several portals.
 */
class cPortalVisibilitySet {
	friend class cSectorVisibilityContainer;

public:
	cPortalVisibilitySet(cSectorVisibilityContainer *apContainer, cPortalVisibilitySet *apParent);
	~cPortalVisibilitySet();

	cPortalVisibilitySet *GetParent() { return mpParent; }

	inline cPortalVisibility *GetVisibility(size_t alIdx) { return mvVisibility[alIdx]; }
	inline size_t GetVisibilityNum() { return mvVisibility.size(); }

	bool PortalExists(cPortal *apPortal);

	int AddPortalVisibility(cPortal *apPortal);

private:
	Common::Array<cPortalVisibility *> mvVisibility;
	cPortalVisibilitySet *mpParent;

	cSectorVisibilityContainer *mpContainer;
};

//----------------------------------------------------

/**
 * Represent the visibility of one sector.
 */
class cSectorVisibility {
	friend class cSectorVisibilityContainer;

public:
	cSectorVisibility(cSectorVisibilityContainer *apContainer);
	~cSectorVisibility();

	bool PortalExists(cPortal *apPortal);

	cPortalVisibilitySet *GetSetConnectingFromSector(cSector *apSector);

	void AddVisibilitySet(cPortalVisibilitySet *apSet);

	cSector *GetSector() const { return mpSector; }

	bool IntersectionBV(cBoundingVolume *apBV);

private:
	cSector *mpSector;
	Common::Array<cPortalVisibilitySet *> mvVisibiltySets;
	bool bStart;

	cSectorVisibilityContainer *mpContainer;
};

typedef Common::StableMap<cSector *, cSectorVisibility *> tSectorVisibilityMap;
typedef tSectorVisibilityMap::iterator tSectorVisibilityMapIt;

//----------------------------------------------------

typedef Common::List<cPortalVisibilitySet *> tPortalVisibilitySetList;
typedef tPortalVisibilitySetList::iterator tPortalVisibilitySetListIt;

typedef Hpl1::Std::set<cSector *> tSectorSet;
typedef tSectorSet::iterator tSectorSetIt;

typedef cSTLMapIterator<cSectorVisibility *, tSectorVisibilityMap, tSectorVisibilityMapIt> tSectorVisibilityIterator;

/**
 * Container of visibility of all the sectors.
 */
class cSectorVisibilityContainer {
public:
	cSectorVisibilityContainer(eSectorVisibilityType aType);
	~cSectorVisibilityContainer();

	void SetFrustum(const cFrustum &aFrustum) { mFrustum = aFrustum; }
	void SetBV(const cBoundingVolume &aBV) { mBoundingVolume = aBV; }

	const cVector3f &GetOrigin() { return mvOrigin; }

	void Compute(cPortalContainer *apContainer);

	cPortalVisibilitySet *CreatePortalVisibiltySet(cPortalVisibilitySet *apParent);

	cSectorVisibility *GetSectorVisibilty(cSector *apSector);

	tSectorVisibilityIterator GetSectorIterator();

	bool IntersectionBV(cBoundingVolume *apBV, cPortalVisibilitySet *apSet);

	bool mbLog;

private:
	void SearchSector(cSector *apSector, cPortalVisibilitySet *apParentSet, int alPortalIndex);

	tString GetTabs();

	// cPortalContainer *mpContainer;

	tSectorSet m_setStartSectors;

	tSectorVisibilityMap m_mapSectors;
	eSectorVisibilityType mType;

	tPortalVisibilitySetList mlstPortalVisibilty;

	cVector3f mvOrigin;
	cBoundingVolume mBoundingVolume;
	cFrustum mFrustum;

	int mlTabs;
};

//----------------------------------------------------

} // namespace hpl

#endif // HPL_SECTOR_VISIBILTY_H
