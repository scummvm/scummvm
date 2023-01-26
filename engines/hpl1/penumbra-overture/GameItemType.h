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
 * This file is part of Penumbra Overture.
 */

#ifndef GAME_GAME_ITEM_TYPE_H
#define GAME_GAME_ITEM_TYPE_H

#include "hpl1/engine/engine.h"

using namespace hpl;

#include "hpl1/penumbra-overture/GameTypes.h"
#include "hpl1/penumbra-overture/PlayerHelper.h"

class cInit;
class cInventoryItem;
class iGameEntity;
class cGameItem;

//----------------------------------------------

class cGameItemType {
public:
	cGameItemType(cInit *apInit);
	virtual ~cGameItemType();

	tWStringVec *GetActions(bool mbCanBeDropped) {
		if (mbCanBeDropped)
			return &mvActions;
		else
			return &mvNonDropActions;
	}

	virtual bool OnPickUp(cInventoryItem *apItem, bool abInGame) { return true; }

	virtual bool OnAction(cInventoryItem *apItem, int alActionNum) { return true; }

	virtual void OnUse(cInventoryItem *apItem, iGameEntity *apEntity) {}

	virtual tString GetShortCutAction(cInventoryItem *apItem) { return ""; }

	virtual tWString GetString(cInventoryItem *apItem) { return _W(""); }

protected:
	cInit *mpInit;

	tWStringVec mvActions;
	tWStringVec mvNonDropActions;
};

//----------------------------------------------
// NORMAL

class cGameItemType_Normal : public cGameItemType {
public:
	cGameItemType_Normal(cInit *apInit);

	bool OnAction(cInventoryItem *apItem, int alActionNum);

	void OnUse(cInventoryItem *apItem, iGameEntity *apEntity);
};

//----------------------------------------------
// NOTEBOOK

class cGameItemType_Notebook : public cGameItemType {
public:
	cGameItemType_Notebook(cInit *apInit);

	bool OnPickUp(cInventoryItem *apItem, bool abInGame);
	bool OnAction(cInventoryItem *apItem, int alActionNum);

	tString GetShortCutAction(cInventoryItem *apItem) { return "NoteBook"; }
};

//----------------------------------------------
// NOTE

class cGameItemType_Note : public cGameItemType {
public:
	cGameItemType_Note(cInit *apInit);

	bool OnPickUp(cInventoryItem *apItem, bool abInGame);
};

//----------------------------------------------

// BATTERY

class cGameItemType_Battery : public cGameItemType {
public:
	cGameItemType_Battery(cInit *apInit);

	bool OnPickUp(cInventoryItem *apItem, bool abInGame);
};
//----------------------------------------------

// FLASHLIGHT

class cGameItemType_Flashlight : public cGameItemType {
public:
	cGameItemType_Flashlight(cInit *apInit);

	bool OnAction(cInventoryItem *apItem, int alActionNum);

	tWString GetString(cInventoryItem *apItem);

	tString GetShortCutAction(cInventoryItem *apItem) { return "Flashlight"; }
};

//----------------------------------------------

// WEAPON MELEE

class cGameItemType_WeaponMelee : public cGameItemType {
public:
	cGameItemType_WeaponMelee(cInit *apInit);

	bool OnAction(cInventoryItem *apItem, int alActionNum);
};

//----------------------------------------------

// THROW

class cGameItemType_Throw : public cGameItemType {
public:
	cGameItemType_Throw(cInit *apInit);

	bool OnAction(cInventoryItem *apItem, int alActionNum);
};

//----------------------------------------------

// GLOW STICK

class cGameItemType_GlowStick : public cGameItemType {
public:
	cGameItemType_GlowStick(cInit *apInit);

	bool OnAction(cInventoryItem *apItem, int alActionNum);

	tWString GetString(cInventoryItem *apItem);

	tString GetShortCutAction(cInventoryItem *apItem) { return "GlowStick"; }
};

//----------------------------------------------

// FLARE

class cGameItemType_Flare : public cGameItemType {
public:
	cGameItemType_Flare(cInit *apInit);

	bool OnAction(cInventoryItem *apItem, int alActionNum);
};

//----------------------------------------------

// PAINKILLERS

class cGameItemType_Painkillers : public cGameItemType {
public:
	cGameItemType_Painkillers(cInit *apInit);

	bool OnAction(cInventoryItem *apItem, int alActionNum);
};

//----------------------------------------------

#endif // GAME_GAME_ITEM_TYPE_H
