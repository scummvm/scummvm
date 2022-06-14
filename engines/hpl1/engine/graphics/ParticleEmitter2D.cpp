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

/*#include "hpl1/engine/graphics/ParticleSystem2D.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/resources/ResourceImage.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/system/LowLevelSystem.h"


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iParticleSystem2D::iParticleSystem2D(tString asName,tMaterialVec *avMaterials,
									unsigned int alMaxParticles, cVector3f avSize,cGraphics *apGraphics)
	: iEntity2D(asName), iParticleSystem(avMaterials, alMaxParticles,avSize,apGraphics)
	{
		//have this in data perhaps?
		tVertexVec vVtx;

		mpVtx = hplNewArray( cVertex, 4 * avMaterials->size() );
		for(int i=0;i<(int)avMaterials->size();i++)
		{
			vVtx = (*avMaterials)[i]->GetImage(eMaterialTexture_Diffuse)->GetVertexVecCopy(0,1);

			for(int j=0;j<4;j++){
				mpVtx[i*4 + j] = vVtx[j];
			}
		}
	}

	//-----------------------------------------------------------------------

	iParticleSystem2D::~iParticleSystem2D()
	{
		hplDeleteArray(mpVtx);
	}

	//-----------------------------------------------------------------------



	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void iParticleSystem2D::UpdateLogic(float afTimeStep)
	{
		Update(afTimeStep);
	}

	//-----------------------------------------------------------------------

	void iParticleSystem2D::Render()
	{
		int lFrame =0;
		if(mvMaterials->size()>1){
			lFrame = (int)mfFrame;//So far no support for sepperate animations for particles.
		}

		mpGraphics->GetRenderer2D()->AddObject(cRenderObject2D((*mvMaterials)[lFrame], this,
												ePrimitiveType_Tri,
											GetWorldPosition().z));
	}

	//-----------------------------------------------------------------------

	const cRect2f& iParticleSystem2D::GetBoundingBox()
	{
		return mBoundingBox;
	}

	//-----------------------------------------------------------------------

	bool iParticleSystem2D::UpdateBoundingBox()
	{
		cVector3f vPos = GetWorldPosition();
		mBoundingBox.w = mvBBSize.x;
		mBoundingBox.h = mvBBSize.y;
		mBoundingBox.x = vPos.x - mvBBSize.x/2;
		mBoundingBox.y = vPos.y - mvBBSize.y/2;

		return true;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PROTECTED METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void iParticleSystem2D::RenderToBatch(eMaterialRenderType aRenderType, unsigned int &aIdxAdd)
	{
		RenderParticles(aRenderType, aIdxAdd);
	}

	//-----------------------------------------------------------------------

	void iParticleSystem2D::RenderParticles(eMaterialRenderType aRenderType, unsigned int &aIdxAdd)
	{
		unsigned int lFrame =0;;
		if(mvMaterials->size()>1){
			lFrame = (int)mfFrame;
			if(lFrame>=mvMaterials->size())lFrame = (unsigned int)mvMaterials->size()-1;
		}

		cVertex* pVtx = &mpVtx[lFrame*4];

		for(int i=0;i<(int)mlNumOfParticles;i++)
		{
			cParticle *pParticle = mvParticles[i];

			pVtx[0].pos = pParticle->mvPos;
			pVtx[0].pos.x -= mvDrawSize.x;
			pVtx[0].pos.y -= mvDrawSize.y;
			pVtx[0].col = pParticle->mColor;
			mpGraphics->GetLowLevel()->AddVertexToBatch(&pVtx[0]);

			pVtx[1].pos = pParticle->mvPos;
			pVtx[1].pos.x += mvDrawSize.x;
			pVtx[1].pos.y -= mvDrawSize.y;
			pVtx[1].col = pParticle->mColor;
			mpGraphics->GetLowLevel()->AddVertexToBatch(&pVtx[1]);

			pVtx[2].pos = pParticle->mvPos;
			pVtx[2].pos.x += mvDrawSize.x;
			pVtx[2].pos.y += mvDrawSize.y;
			pVtx[2].col = pParticle->mColor;
			mpGraphics->GetLowLevel()->AddVertexToBatch(&pVtx[2]);


			pVtx[3].pos = pParticle->mvPos;
			pVtx[3].pos.x -= mvDrawSize.x;
			pVtx[3].pos.y += mvDrawSize.y;
			pVtx[3].col = pParticle->mColor;
			mpGraphics->GetLowLevel()->AddVertexToBatch(&pVtx[3]);

			mpGraphics->GetLowLevel()->AddIndexToBatch(aIdxAdd + 0);
			mpGraphics->GetLowLevel()->AddIndexToBatch(aIdxAdd + 1);
			mpGraphics->GetLowLevel()->AddIndexToBatch(aIdxAdd + 2);

			mpGraphics->GetLowLevel()->AddIndexToBatch(aIdxAdd + 2);
			mpGraphics->GetLowLevel()->AddIndexToBatch(aIdxAdd + 3);
			mpGraphics->GetLowLevel()->AddIndexToBatch(aIdxAdd + 0);

			aIdxAdd +=4;

		}
	}
	//-----------------------------------------------------------------------
}*/
