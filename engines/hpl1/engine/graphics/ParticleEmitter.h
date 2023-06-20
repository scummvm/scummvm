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

#ifndef HPL_PARTICLE_EMITTER_H
#define HPL_PARTICLE_EMITTER_H

#include "common/array.h"
#include "common/list.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "common/stablemap.h"

namespace hpl {

enum eParticleEmitterType {
	eParticleEmitterType_2D,
	eParticleEmitterType_3D,
	eParticleEmitterType_LastEnum,
};

// NEW

enum ePENoiseType {
	ePENoiseType_LowFreq,
	ePENoiseType_HighFreq,
	ePENoiseType_Both,
	ePENoiseType_None,
	ePENoiseType_LastEnum,
};

typedef struct
{
	float fRelToBeamPos;
	float fRelToBendPos;
	int lLowFreqNoiseIdx;
	int lHighFreqNoiseIdx;
	ePENoiseType noiseType;
} tBeamNoisePoint;

// ---

//////////////////////////////////////////////////////
/////////////// PARTICLE /////////////////////////////
//////////////////////////////////////////////////////

class cParticle {
public:
	cParticle() {}

	cVector3f mvPos;
	cVector3f mvLastPos;
	cVector3f mvLastCollidePos;
	cVector3f mvAcc;
	cVector3f mvVel;

	float mfSpeedMul;
	float mfMaxSpeed;

	cColor mStartColor;
	cColor mColor;

	cVector2f mvStartSize;
	cVector2f mvSize;

	float mfStartLife;
	float mfLife;
	float mfLifeSize_MiddleStart;
	float mfLifeSize_MiddleEnd;

	float mfLifeColor_MiddleStart;
	float mfLifeColor_MiddleEnd;

	int mlSubDivNum;

	float mfBounceAmount;
	int mlBounceCount;

	cVector3f mvExtra;

	// NEW

	float mfSpin;
	float mfSpinVel;
	float mfSpinFactor;

	cVector3f mvRevolutionVel;

	// Beam Specific

	int mlLowFreqPoints;
	int mlHighFreqPoints;
	Common::Array<cVector3f> mvBeamPoints;

	// ---
};

typedef Common::Array<cParticle *> tParticleVec;
typedef tParticleVec::iterator tParticleVecIt;

//////////////////////////////////////////////////////
/////////////// PARTICLE SYSTEM //////////////////////
//////////////////////////////////////////////////////

class cGraphics;
class cResources;

class iParticleEmitter {
public:
	iParticleEmitter(tMaterialVec *avMaterials, unsigned int alMaxParticles,
					 cVector3f avSize, cGraphics *apGraphics, cResources *apResources);
	virtual ~iParticleEmitter();

	void Update(float afTimeStep);

	virtual void Render() = 0;

	virtual eParticleEmitterType GetType() = 0;

	tString GetEntityType() { return "ParticleEmitter"; }

	virtual bool IsDead() { return mlNumOfParticles == 0 && mbDying; }
	virtual void Kill() { mbDying = true; }

	void KillInstantly();

	virtual bool IsDying() { return mbDying; }

	void SetDataName(const tString &asName) { msDataName = asName; }
	void SetDataSize(const cVector3f &avSize) { mvDataSize = avSize; }

	int GetParticleNum() { return mlNumOfParticles; }

protected:
	cGraphics *mpGraphics;
	cResources *mpResources;

	tString msDataName;
	cVector3f mvDataSize;

	tParticleVec mvParticles;
	unsigned int mlNumOfParticles;
	unsigned int mlMaxParticles;

	cMatrixf m_mtxTemp;

	tMaterialVec *mvMaterials;

	// Vars for easier updating.
	float mfTime;
	bool mbDying;
	float mfFrame;

	bool mbUpdateGfx;
	bool mbUpdateBV;

	virtual void UpdateMotion(float afTimeStep) = 0;

	virtual void SetParticleDefaults(cParticle *apParticle) = 0;

	/**
	 * Swaps the position with index and the last particle and lowers the number of particles.
	 * \param alIndex
	 */
	void SwapRemove(unsigned int alIndex);
	cParticle *CreateParticle();
};

typedef Common::List<iParticleEmitter *> tParticleEmitterList;
typedef tParticleEmitterList::iterator tParticleEmitterListIt;

//////////////////////////////////////////////////////
/////////////// PARTICLE DATA ///////////////////////
//////////////////////////////////////////////////////

class cResources;

class iParticleEmitterData {
	friend class cParticleSystemData3D;

public:
	/**
	 * This inits the data needed for the particles system type
	 * \param &asName name of the type
	 * \param apResources
	 * \param apGraphics
	 */
	iParticleEmitterData(const tString &asName, cResources *apResources, cGraphics *apGraphics);
	virtual ~iParticleEmitterData();

	void AddMaterial(iMaterial *apMaterial);

	const tString &GetName() { return msName; }

	virtual iParticleEmitter *Create(tString asName, cVector3f avSize) = 0;

	float GetWarmUpTime() const { return mfWarmUpTime; }
	float GetWarmUpStepsPerSec() const { return mfWarmUpStepsPerSec; }

protected:
	cResources *mpResources;
	cGraphics *mpGraphics;

	tString msName;
	tMaterialVec mvMaterials;

	float mfWarmUpTime;
	float mfWarmUpStepsPerSec;
};

typedef Common::StableMap<tString, iParticleEmitterData *> tParticleEmitterDataMap;
typedef tParticleEmitterDataMap::iterator tParticleEmitterDataMapIt;

} // namespace hpl

#endif // HPL_PARTICLE_EMITTER_H
