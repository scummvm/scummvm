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
#include "hpl1/engine/scene/Entity2D.h"
#include "hpl1/engine/scene/Node2D.h"
#include "hpl1/engine/scene/GridMap2D.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------
	iEntity2D::iEntity2D(tString asName) :	iEntity(asName), mvPosition(0), mvRotation(0), mvScale(0),
		mvLastPosition(0), mvLastRotation(0), mvLastScale(0),
		mpGridObject(NULL)
	{

	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cVector3f iEntity2D::GetWorldPosition()
	{
		if(mpParentNode)
		{
			cNode2D* pNode2D = static_cast<cNode2D*>(mpParentNode);

			return pNode2D->GetPosition() + mvPosition;
		}
		else
			return mvPosition;
	}

	//-----------------------------------------------------------------------

	cVector3f iEntity2D::GetWorldRotation()
	{
		if(mpParentNode)
		{
			cNode2D* pNode2D = static_cast<cNode2D*>(mpParentNode);

			return pNode2D->GetRotation() + mvRotation;
		}
		else
			return mvRotation;
	}

	//-----------------------------------------------------------------------

	cVector3f iEntity2D::GetWorldScale()
	{
		if(mpParentNode)
		{
			cNode2D* pNode2D = static_cast<cNode2D*>(mpParentNode);

			return pNode2D->GetScale() + mvScale;
		}
		else
			return mvScale;
	}

	//-----------------------------------------------------------------------

	void iEntity2D::SetPosition(const cVector3f& avPos)
	{
		mvLastPosition = mvPosition;
		mvPosition = avPos;

		if(UpdateBoundingBox())
			if(mpGridObject)
				mpGridObject->Update(GetBoundingBox());

	}

	//-----------------------------------------------------------------------

	void iEntity2D::SetRotation(const cVector3f& avRot)
	{
		mvLastRotation = mvRotation;
		mvRotation = avRot;

		if(UpdateBoundingBox())
			if(mpGridObject)
				mpGridObject->Update(GetBoundingBox());
	}

	//-----------------------------------------------------------------------

	void iEntity2D::SetScale(const cVector3f& avScale)
	{
		mvLastScale = mvScale;
		mvScale = avScale;

		if(UpdateBoundingBox())
			if(mpGridObject)
				mpGridObject->Update(GetBoundingBox());
	}

	//-----------------------------------------------------------------------
}
