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

#include "hpl1/engine/scene/Area2D.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/physics/Body2D.h"
#include "hpl1/engine/physics/Collider2D.h"
#include "hpl1/engine/impl/tinyXML/tinyxml.h"


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cArea2D::cArea2D(const tString& asName,const tString& asTypeName,cCollider2D* apCollider )
		: iEntity2D(asName)
	{
		UpdateBoundingBox();

		msType = asTypeName;
		mpCollider = apCollider;
	}

	//-----------------------------------------------------------------------

	cArea2D::~cArea2D()
	{
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	bool cArea2D::CheckWorldCollision(tFlag alFlags)
	{
		cRect2f CollideRect = GetBoundingBox();
		return mpCollider->CollideRect(CollideRect,alFlags,NULL)!=0?true:false;
	}

	//-----------------------------------------------------------------------

	bool cArea2D::CheckBodyCollision(cBody2D *apBody)
	{
		return false;
	}

	//-----------------------------------------------------------------------

	bool cArea2D::LoadData(TiXmlElement* apRootElem)
	{
		mvCustom.x = cString::ToFloat(apRootElem->Attribute("SizeX"),0);
		mvCustom.y = cString::ToFloat(apRootElem->Attribute("SizeY"),0);
		mvCustom.z = cString::ToFloat(apRootElem->Attribute("SizeZ"),0);

		mvSize.x = cString::ToFloat(apRootElem->Attribute("Width"),0);
		mvSize.y = cString::ToFloat(apRootElem->Attribute("Height"),0);

		mbIsActive = cString::ToBool(apRootElem->Attribute("Active"),true);

		return true;
	}

	//-----------------------------------------------------------------------

	const cRect2f& cArea2D::GetBoundingBox()
	{
		return mBoundingBox;
	}

	//-----------------------------------------------------------------------

	bool cArea2D::UpdateBoundingBox()
	{
		mBoundingBox = cRect2f(cVector2f(GetWorldPosition().x-mvSize.x/2,
			GetWorldPosition().y-mvSize.y/2),mvSize);

		return true;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	//-----------------------------------------------------------------------
}
