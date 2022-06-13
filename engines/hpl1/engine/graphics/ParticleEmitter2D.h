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
/*#ifndef HPL_PARTICLE_SYSTEM_2D_H
#define HPL_PARTICLE_SYSTEM_2D_H

#include "hpl1/engine/graphics/RenderObject2D.h"
#include "hpl1/engine/scene/Entity2D.h"
#include "hpl1/engine/graphics/ParticleSystem.h"


namespace hpl {

	class cResourceImage;

	//////////////////////////////////////////////////////
	/////////////// PARTICLE SYSTEM //////////////////////
	//////////////////////////////////////////////////////

	class cGraphics;

	class iParticleSystem2D : public iParticleSystem,public iEntity2D, public iRenderObject2DRenderer
	{
	public:
		iParticleSystem2D(tString asName,tMaterialVec* avMaterials,unsigned int alMaxParticles,
						cVector3f avSize, cGraphics* apGraphics);
		virtual ~iParticleSystem2D();

		void UpdateLogic(float afTimeStep);

		void Render();

		eParticleSystemType GetType(){return eParticleSystemType_2D;}

		const cRect2f& GetBoundingBox();
		virtual bool UpdateBoundingBox();
		tString GetEntityType(){ return "ParticleSystem"; }

		virtual bool IsDead(){ return mlNumOfParticles==0 && mbDying;}
		virtual void Kill(){ mbDying = true;}

	protected:
		cVector2f mvBBSize;
		cVertex* mpVtx;
		cVector2f mvDrawSize;

		void RenderToBatch(eMaterialRenderType aRenderType,	unsigned int &aIdxAdd);

		virtual void RenderParticles(eMaterialRenderType aRenderType, unsigned int &aIdxAdd);
	};

	typedef std::list<iParticleSystem2D*> tParticleSystem2DList;
	typedef tParticleSystem2DList::iterator tParticleSystem2DListIt;
};

#endif // HPL_PARTICLE_SYSTEM_2D_H*/
