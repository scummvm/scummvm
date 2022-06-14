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

#include "hpl1/engine/graphics/ParticleEmitter.h"
#include "hpl1/engine/system/LowLevelSystem.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/graphics/MaterialHandler.h"
#include "hpl1/engine/resources/ImageManager.h"
#include "hpl1/engine/resources/MaterialManager.h"


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// DATA LOADER
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iParticleEmitterData::iParticleEmitterData(const tString &asName,cResources* apResources,
											cGraphics *apGraphics)
	{
		msName = asName;
		mpResources = apResources;
		mpGraphics = apGraphics;

		mfWarmUpTime =0;
		mfWarmUpStepsPerSec = 20;
	}

	//-----------------------------------------------------------------------

	iParticleEmitterData::~iParticleEmitterData()
	{
		for(int i=0;i<(int)mvMaterials.size();i++)
		{
			if(mvMaterials[i]) mpResources->GetMaterialManager()->Destroy(mvMaterials[i]);
		}
	}
	//-----------------------------------------------------------------------

	void iParticleEmitterData::AddMaterial(iMaterial *apMaterial)
	{
		mvMaterials.push_back(apMaterial);
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iParticleEmitter::iParticleEmitter(tMaterialVec *avMaterials,
									unsigned int alMaxParticles, cVector3f avSize,
									cGraphics *apGraphics, cResources *apResources)
	{
		mpGraphics = apGraphics;
		mpResources = apResources;

		mvParticles.resize(alMaxParticles);
		for(int i=0;i<(int)alMaxParticles;i++)
		{
			mvParticles[i] = hplNew( cParticle, () );
		}
		mlMaxParticles = alMaxParticles;
		mlNumOfParticles =0;

		mvMaterials = avMaterials;

		//Update vars:
		mbDying = false;
		mfTime =0;
		mfFrame =0;

		mbUpdateGfx = true;
		mbUpdateBV = true;
	}

	//-----------------------------------------------------------------------

	iParticleEmitter::~iParticleEmitter()
	{
		for(int i=0;i<(int)mvParticles.size();i++){
			hplDelete(mvParticles[i]);
		}
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void iParticleEmitter::Update(float afTimeStep)
	{
		UpdateMotion(afTimeStep);

		mfTime++;
		mbUpdateGfx = true;
		mbUpdateBV = true;
	}

	//-----------------------------------------------------------------------

	void iParticleEmitter::KillInstantly()
	{
		mlMaxParticles = 0;
		mlNumOfParticles = 0;
		mbDying = true;
	}

	//-----------------------------------------------------------------------


	//////////////////////////////////////////////////////////////////////////
	// PROTECTED METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cParticle* iParticleEmitter::CreateParticle()
	{
		if(mlNumOfParticles == mlMaxParticles) return NULL;
		++mlNumOfParticles;
		return mvParticles[mlNumOfParticles-1];
	}

	//-----------------------------------------------------------------------

	void iParticleEmitter::SwapRemove(unsigned int alIndex)
	{
		if(alIndex < mlNumOfParticles-1)
		{
			cParticle* pTemp = mvParticles[alIndex];
			mvParticles[alIndex] = mvParticles[mlNumOfParticles-1];
			mvParticles[mlNumOfParticles-1] = pTemp;
		}
		mlNumOfParticles--;
	}

	//-----------------------------------------------------------------------

}
