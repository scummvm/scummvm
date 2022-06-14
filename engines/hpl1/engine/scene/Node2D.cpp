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

#include "hpl1/engine/scene/Node2D.h"
#include "hpl1/engine/scene/Entity2D.h"
#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cNode2D::cNode2D()
	{
		mvPosition = cVector3f(0);
	}

	//-----------------------------------------------------------------------

	cNode2D::~cNode2D()
	{
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iNode* cNode2D::CreateChild()
	{
		cNode2D *pNode = hplNew( cNode2D, () );

		mlstNode.push_back(pNode);

		return pNode;
	}

	//-----------------------------------------------------------------------

	cNode2D* cNode2D::CreateChild2D()
	{
		return static_cast<cNode2D*>(CreateChild());
	}

	//-----------------------------------------------------------------------

	void cNode2D::SetPosition(cVector3f avPos)
	{
		cVector3f vPosAdd = avPos - mvPosition;
		mvPosition = avPos;

		//Update all the entities
		for(tEntityListIt it = mlstEntity.begin();it != mlstEntity.end();it++)
		{
			iEntity2D* pEntity = static_cast<iEntity2D*>(*it);

			pEntity->SetPosition(pEntity->GetLocalPosition());
			//call an update??
		}

		for(tNodeListIt NIt = mlstNode.begin(); NIt != mlstNode.end(); NIt++)
		{
			cNode2D* pNode = static_cast<cNode2D*>(*NIt);

			pNode->SetPosition(pNode->mvPosition + vPosAdd);
		}
	}

	//-----------------------------------------------------------------------

	void cNode2D::SetRotation(cVector3f avRot)
	{
	}

	//-----------------------------------------------------------------------

	void cNode2D::SetScale(cVector3f avScale)
	{
	}

	//-----------------------------------------------------------------------
}
