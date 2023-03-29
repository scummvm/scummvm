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

#ifndef HPL_ENTITY_H
#define HPL_ENTITY_H

#include "hpl1/engine/game/SaveGame.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class iNode;

//------------------------------------

kSaveData_BaseClass(iEntity) {
	kSaveData_ClassInit(iEntity) public : int mlParentNodeId;
	tString msName;
	bool mbIsVisible;
	bool mbIsActive;
};

//------------------------------------

class iEntity : public iSaveObject {
	typedef iSaveObject super;

public:
	iEntity(tString asName) : msName(asName), mbIsVisible(true),
							  mbIsActive(true), mpParentNode(NULL) {}

	virtual ~iEntity();

	virtual tString GetEntityType() = 0;

	virtual void UpdateLogic(float afTimeStep) {}

	tString &GetName() { return msName; }
	void SetName(const tString &asName) { msName = asName; }

	iNode *GetParent() { return mpParentNode; }
	void SetParent(iNode *apNode) { mpParentNode = apNode; }
	bool HasParent() { return mpParentNode != NULL; }

	bool IsActive() { return mbIsActive; }
	void SetActive(bool abActive) { mbIsActive = abActive; }
	virtual bool IsVisible() { return mbIsVisible; }
	virtual void SetVisible(bool abVisible) { mbIsVisible = abVisible; }

	// SaveObject implementation
	virtual iSaveData *CreateSaveData();
	virtual void SaveToSaveData(iSaveData *apSaveData);
	virtual void LoadFromSaveData(iSaveData *apSaveData);
	virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

protected:
	iNode *mpParentNode;

	tString msName;
	bool mbIsVisible;
	bool mbIsActive;
};

typedef Common::List<iEntity *> tEntityList;
typedef tEntityList::iterator tEntityListIt;

typedef cSTLIterator<iEntity *, tEntityList, tEntityListIt> cEntityIterator;

} // namespace hpl

#endif // HPL_ENTITY2D_H
