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

#ifndef HPL_PARTICLE_SYSTEM_3D_H
#define HPL_PARTICLE_SYSTEM_3D_H

#include "hpl1/engine/graphics/RenderList.h"
#include "hpl1/engine/resources/ResourceBase.h"
#include "hpl1/engine/scene/Entity3D.h"

namespace hpl {

class iParticleEmitter3D;
class iParticleEmitterData;
class cParticleManager;

//----------------------------------------------------

class cParticleSystem3D;

class cParticleSystemData3D : public iResourceBase {
public:
	cParticleSystemData3D(const tString &asName, cResources *apResources, cGraphics *apGraphics);
	virtual ~cParticleSystemData3D();

	cParticleSystem3D *Create(tString asName, cVector3f avSize, const cMatrixf &a_mtxTransform);

	bool LoadFromFile(const tString &asFile);

	void AddEmitterData(iParticleEmitterData *apData);

	bool reload() { return false; }
	void unload() {}
	void destroy() {}

	iParticleEmitterData *GetEmitterData(int alIdx) const { return mvEmitterData[alIdx]; }

private:
	cResources *mpResources;
	cGraphics *mpGraphics;

	Common::Array<iParticleEmitterData *> mvEmitterData;
};

//----------------------------------------------------

class cSaveData_ParticleEmitter3D : public iSerializable {
	kSerializableClassInit(cSaveData_ParticleEmitter3D) public : bool mbActive;
	bool mbDying;
};

kSaveData_ChildClass(iEntity3D, cParticleSystem3D) {
	kSaveData_ClassInit(cParticleSystem3D) public :

		tString msDataName;
	cVector3f mvDataSize;

	cContainerVec<cSaveData_ParticleEmitter3D> mvEmitters;

	iSaveObject *CreateSaveObject(cSaveObjectHandler * apSaveObjectHandler, cGame * apGame);
	int GetSaveCreatePrio();
};

//----------------------------------------------------

class cParticleSystem3D : public iEntity3D {
	typedef iEntity3D super;

public:
	cParticleSystem3D(const tString asName, cParticleSystemData3D *apData,
					  cResources *apResources, cGraphics *apGraphics);
	~cParticleSystem3D();

	void SetVisible(bool abVisible);

	void UpdateLogic(float afTimeStep);

	bool IsDead();
	bool IsDying();

	void Kill();
	void KillInstantly();

	void AddEmitter(iParticleEmitter3D *apEmitter);
	iParticleEmitter3D *GetEmitter(int alIdx);
	int GetEmitterNum();

	tString GetEntityType() { return "ParticleSystem3D"; }

	void SetDataName(const tString &asName) { msDataName = asName; }
	void SetDataSize(const cVector3f &avSize) { mvDataSize = avSize; }

	const tString &GetDataName() { return msDataName; }
	const cVector3f &GetDataSize() { return mvDataSize; }

	void SetParticleManager(cParticleManager *apParticleManager) {
		mpParticleManager = apParticleManager;
	}

	cParticleSystemData3D *GetData() { return mpData; }

	// SaveObject implementation
	iSaveData *CreateSaveData();
	void SaveToSaveData(iSaveData *apSaveData);
	void LoadFromSaveData(iSaveData *apSaveData);
	void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

private:
	cResources *mpResources;
	cGraphics *mpGraphics;
	cParticleManager *mpParticleManager;
	cParticleSystemData3D *mpData;

	Common::Array<iParticleEmitter3D *> mvEmitters;

	tString msDataName;
	cVector3f mvDataSize;

	bool mbFirstUpdate;
};

} // namespace hpl

#endif // HPL_PARTICLE_SYSTEM_3D_H
