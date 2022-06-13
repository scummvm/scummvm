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
#ifndef HPL_COLLIDER_ENTITY_H
#define HPL_COLLIDER_ENTITY_H

#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/graphics/Renderable.h"

namespace hpl {

	class iPhysicsWorld;
	class iPhysicsBody;

	class cColliderEntity : public iRenderable
	{
	public:
		cColliderEntity(const tString &asName,iPhysicsBody* apBody, iPhysicsWorld *apWorld);
		~cColliderEntity();

		iMaterial *GetMaterial(){ return NULL;}
		void UpdateGraphics(cCamera3D *apCamera, float afFrameTime, cRenderList *apRenderList){}
		iVertexBuffer* GetVertexBuffer(){ return NULL;}

		cBoundingVolume* GetBoundingVolume();

		cMatrixf* GetModelMatrix(cCamera3D* apCamera);
		int GetMatrixUpdateCount();

		eRenderableType GetRenderType(){ return eRenderableType_Dummy;}

		//Entity implementation
		tString GetEntityType(){ return "Collider";}
		bool IsVisible(){ return false; }

		bool IsShadowCaster(){ return false;}

		void UpdateLogic(float afTimeStep){}

	private:
		iPhysicsWorld* mpPhysicsWorld;
		iPhysicsBody* mpBody;
	};

	//-----------------------------------------------------------------------


};
#endif // HPL_COLLIDER_ENTITY_H
