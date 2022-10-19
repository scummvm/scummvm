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

#ifndef HPL_PARTICLE_EMITTER_3D_H
#define HPL_PARTICLE_EMITTER_3D_H

#include "hpl1/engine/graphics/ParticleEmitter.h"
#include "hpl1/engine/graphics/Renderable.h"
#include "hpl1/engine/scene/Entity3D.h"

namespace hpl {

class cResourceImage;
class cGraphics;
class cWorld3D;
class cParticleSystem3D;

//---------------------------------------------

// NEW
enum ePEType {
	ePEType_Normal,
	ePEType_Beam,
	ePEType_LastEnum,
};

// ---

enum eParticleEmitter3DType {
	eParticleEmitter3DType_FixedPoint,
	eParticleEmitter3DType_DynamicPoint,
	eParticleEmitter3DType_Line,
	eParticleEmitter3DType_Axis,
	eParticleEmitter3DType_LastEnum,
};

enum eParticleEmitter3DCoordSystem {
	eParticleEmitter3DCoordSystem_World,
	eParticleEmitter3DCoordSystem_Local,
	eParticleEmitter3DCoordSystem_LastEnum,
};

//------------------------------------------

kSaveData_ChildClass(iRenderable, iParticleEmitter3D) {
	kSaveData_ClassInit(iParticleEmitter3D) public : tString msDataName;
	cVector3f mvDataSize;

	int mlTime;
	bool mbDying;
	float mfFrame;

	virtual iSaveObject *CreateSaveObject(cSaveObjectHandler * apSaveObjectHandler, cGame * apGame);
	virtual int GetSaveCreatePrio();
};

//------------------------------------------

class cPESubDivision {
public:
	cVector3f mvUV[4];
};

//------------------------------------------

//////////////////////////////////////////////////////
/////////////// PARTICLE SYSTEM //////////////////////
//////////////////////////////////////////////////////

class iParticleEmitter3D : public iParticleEmitter, public iRenderable {
public:
	iParticleEmitter3D(tString asName, tMaterialVec *avMaterials, unsigned int alMaxParticles,
					   cVector3f avSize, cGraphics *apGraphics, cResources *apResources);
	virtual ~iParticleEmitter3D();

	void UpdateLogic(float afTimeStep);

	void Render() {}

	eParticleEmitterType GetType() { return eParticleEmitterType_3D; }

	void SetSubDivUV(const cVector2l &avSubDiv);

	void SetWorld(cWorld3D *apWorld) { mpWorld = apWorld; }

	void SetSystem(cParticleSystem3D *apSystem) { mpParentSystem = apSystem; }

	// Entity implementation
	tString GetEntityType() { return "ParticleEmitter3D"; }
	bool IsVisible();
	void SetVisible(bool abVisible) { SetRendered(abVisible); }

	// Renderable implementation
	void UpdateGraphics(cCamera3D *apCamera, float afFrameTime, cRenderList *apRenderList);

	iMaterial *GetMaterial();
	iVertexBuffer *GetVertexBuffer();

	bool IsShadowCaster() { return false; }

	cBoundingVolume *GetBoundingVolume();

	cMatrixf *GetModelMatrix(cCamera3D *apCamera);

	int GetMatrixUpdateCount() { return GetTransformUpdateCount(); }
	eRenderableType GetRenderType() { return eRenderableType_Normal; }

	// SaveObject implementation
	virtual iSaveData *CreateSaveData();
	virtual void SaveToSaveData(iSaveData *apSaveData);
	virtual void LoadFromSaveData(iSaveData *apSaveData);
	virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

protected:
	int mlUpdateCount;
	float mfTimeStepAccum;

	iVertexBuffer *mpVtxBuffer;

	Common::Array<cPESubDivision> mvSubDivUV;

	cVector3f mvDirection;
	int mlDirectionUpdateCount;

	cVector3f mvRight;
	cVector3f mvForward;
	int mlAxisDrawUpdateCount;

	cParticleSystem3D *mpParentSystem;

	// Set by the particle system implementation.
	cVector2f mvDrawSize;
	cVector2f mvMaxDrawSize;

	eParticleEmitter3DType mDrawType;

	eParticleEmitter3DCoordSystem mCoordSystem;

	cWorld3D *mpWorld;

	bool mbUsesDirection;

	// NEW
	bool mbUsePartSpin;
	bool mbUseRevolution;

	ePEType mPEType;

	// ---
};
} // namespace hpl

#endif // HPL_PARTICLE_EMITTER_3D_H
