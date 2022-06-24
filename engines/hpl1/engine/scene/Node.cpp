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

#include "hpl1/engine/scene/Node.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iNode::~iNode() {
	for (tEntityListIt it = mlstEntity.begin(); it != mlstEntity.end(); it++) {
		iEntity *pEntity = *it;
		pEntity->SetParent(NULL);
	}
	mlstEntity.clear();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

int iNode::SetVisible(bool abX, bool abCascade) {
	int lNum = 0;
	for (tEntityListIt it = mlstEntity.begin(); it != mlstEntity.end(); it++) {
		(*it)->SetVisible(abX);
		lNum++;
	}

	if (abCascade) {
		for (tNodeListIt NIt = mlstNode.begin(); NIt != mlstNode.end(); NIt++) {
			(*NIt)->SetVisible(abX, abCascade);
		}
	}
	return lNum;
}

//-----------------------------------------------------------------------

bool iNode::AddEntity(iEntity *apEntity) {
	if (apEntity->HasParent())
		return false;

	mlstEntity.push_back(apEntity);
	apEntity->SetParent(this);

	return true;
}

//-----------------------------------------------------------------------

bool iNode::RemoveEntity(iEntity *apEntity) {
	for (tEntityListIt it = mlstEntity.begin(); it != mlstEntity.end(); it++) {
		if (*it == apEntity) {
			apEntity->SetParent(NULL);
			mlstEntity.erase(it);
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------

void iNode::ClearEntities() {
	mlstEntity.clear();
}

//-----------------------------------------------------------------------

cNodeIterator iNode::GetChildIterator() {
	return cNodeIterator(&mlstNode);
}

cEntityIterator iNode::GetEntityIterator() {
	return cEntityIterator(&mlstEntity);
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerializeVirtual(cSaveData_iNode, iSaveData)
	kSerializeVarContainer(mlstEntities, eSerializeType_Int32)
		kSerializeVarContainer(mlstNodes, eSerializeType_Int32)
			kEndSerialize()

	//-----------------------------------------------------------------------

	void iNode::SaveToSaveData(iSaveData *apSaveData) {
	kSaveData_SaveToBegin(iNode);

	kSaveData_SaveIdList(mlstEntity, tEntityListIt, mlstEntities);
	kSaveData_SaveIdList(mlstNode, tNodeListIt, mlstNodes);
}

//-----------------------------------------------------------------------

void iNode::LoadFromSaveData(iSaveData *apSaveData) {
	kSaveData_LoadFromBegin(iNode);
}

//-----------------------------------------------------------------------

void iNode::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	kSaveData_SetupBegin(iNode);

	kSaveData_LoadIdList(mlstEntity, mlstEntities, iEntity *);
	kSaveData_LoadIdList(mlstNode, mlstNodes, iNode *);
}

//-----------------------------------------------------------------------

} // namespace hpl
