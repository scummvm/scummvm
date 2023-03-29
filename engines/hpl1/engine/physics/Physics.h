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

#ifndef HPL_Physics_H
#define HPL_Physics_H

#include "common/list.h"
#include "hpl1/engine/game/Updateable.h"
#include "hpl1/engine/physics/PhysicsMaterial.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/std/map.h"

namespace hpl {

class iLowLevelPhysics;
class iPhysicsWorld;
class cSurfaceData;
class cWorld3D;
class cResources;

//------------------------------------------------

typedef Common::List<iPhysicsWorld *> tPhysicsWorldList;
typedef tPhysicsWorldList::iterator tPhysicsWorldListIt;

typedef Hpl1::Std::map<tString, cSurfaceData *> tSurfaceDataMap;
typedef tSurfaceDataMap::iterator tSurfaceDataMapIt;

//------------------------------------------------

class cPhysicsImpactCount {
public:
	cPhysicsImpactCount() { mfCount = 0; }

	float mfCount;
};

typedef Common::List<cPhysicsImpactCount> tPhysicsImpactCountList;
typedef tPhysicsImpactCountList::iterator tPhysicsImpactCountListIt;

//------------------------------------------------

class cPhysics : public iUpdateable {
public:
	cPhysics(iLowLevelPhysics *apLowLevelPhysics);
	~cPhysics();

	void Init(cResources *apResources);

	void Update(float afTimeStep);

	iPhysicsWorld *CreateWorld(bool abAddSurfaceData);
	void DestroyWorld(iPhysicsWorld *apWorld);

	cSurfaceData *CreateSurfaceData(const tString &asName);
	cSurfaceData *GetSurfaceData(const tString &asName);
	bool LoadSurfaceData(const tString &asFile);

	iLowLevelPhysics *GetLowLevel() { return mpLowLevelPhysics; }

	void SetGameWorld(cWorld3D *apWorld) { mpGameWorld = apWorld; }
	cWorld3D *GetGameWorld() { return mpGameWorld; }

	void SetImpactDuration(float afX) { mfImpactDuration = afX; }
	float GetImpactDuration() { return mfImpactDuration; }

	void SetMaxImpacts(int alX) { mlMaxImpacts = alX; }
	int GetMaxImpacts() { return mlMaxImpacts; }
	int GetNumOfImpacts() { return (int)mlstImpactCounts.size(); }

	bool CanPlayImpact();
	void AddImpact();

	void SetDebugLog(bool abX) { mbLog = abX; }
	bool GetDebugLog() { return mbLog; }

private:
	ePhysicsMaterialCombMode GetCombMode(const char *apName);

	void UpdateImpactCounts(float afTimeStep);

	iLowLevelPhysics *mpLowLevelPhysics;
	cResources *mpResources;

	cWorld3D *mpGameWorld;

	tPhysicsWorldList mlstWorlds;
	tSurfaceDataMap m_mapSurfaceData;

	tPhysicsImpactCountList mlstImpactCounts;
	float mfImpactDuration;
	int mlMaxImpacts;
	bool mbLog;
};

} // namespace hpl

#endif // HPL_Physics_H
