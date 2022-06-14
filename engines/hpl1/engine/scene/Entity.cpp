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

#include "hpl1/engine/scene/Entity.h"

#include "hpl1/engine/scene/Node.h"
#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iEntity::~iEntity()
	{
		if(mpParentNode)
		{
			mpParentNode->RemoveEntity(this);
		}
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// SAVE OBJECT STUFF
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	kBeginSerializeVirtual(cSaveData_iEntity, iSaveData)
	kSerializeVar(mlParentNodeId, eSerializeType_Int32)
	kSerializeVar(msName, eSerializeType_String)
	kSerializeVar(mbIsVisible, eSerializeType_Bool)
	kSerializeVar(mbIsActive, eSerializeType_Bool)
	kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveData* iEntity::CreateSaveData()
	{
		return NULL;
	}

	//-----------------------------------------------------------------------

	void iEntity::SaveToSaveData(iSaveData *apSaveData)
	{
		kSaveData_SaveToBegin(iEntity);

		kSaveData_SaveTo(mbIsActive);
		kSaveData_SaveTo(mbIsVisible);
		kSaveData_SaveTo(msName);

		kSaveData_SaveObject(mpParentNode,mlParentNodeId);
	}

	//-----------------------------------------------------------------------

	void iEntity::LoadFromSaveData(iSaveData *apSaveData)
	{
		kSaveData_LoadFromBegin(iEntity);

		kSaveData_LoadFrom(mbIsActive);
		kSaveData_LoadFrom(mbIsVisible);
		kSaveData_LoadFrom(msName);
	}

	//-----------------------------------------------------------------------

	void iEntity::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame)
	{
		kSaveData_SetupBegin(iEntity);

		//kSaveData_LoadObject(mpParentNode,mlParentNodeId,iNode*);
	}

	//-----------------------------------------------------------------------
}
