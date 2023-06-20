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

#ifndef GAME_GAME_TYPES_H
#define GAME_GAME_TYPES_H

#define kTranslate(asCat, asName) (mpInit->mpGame->GetResources()->Translate(asCat, asName))

#define kCopyToVar(aVar, aVal) (aVar->aVal = aVal)
#define kCopyFromVar(aVar, aVal) (aVal = aVar->aVal)

#include "hpl1/engine/engine.h"
#include "common/multimap.h"

using namespace hpl;

//---------------------------------

//////////////////////////////////////////////////////////////////////////
// ENUMERATIONS
//////////////////////////////////////////////////////////////////////////

//---------------------------------

enum eButtonHandlerState {
	eButtonHandlerState_Null,
	eButtonHandlerState_Game,
	eButtonHandlerState_MainMenu,
	eButtonHandlerState_MapLoadText,
	eButtonHandlerState_PreMenu,
	eButtonHandlerState_Credits,
	eButtonHandlerState_DemoEndText,
	eButtonHandlerState_Intro
};

//---------------------------------

enum ePlayerMoveState {
	ePlayerMoveState_Walk,
	ePlayerMoveState_Run,
	ePlayerMoveState_Still,
	ePlayerMoveState_Jump,
	ePlayerMoveState_Crouch,
	ePlayerMoveState_LastEnum,
};

//---------------------------------

enum ePlayerState {
	ePlayerState_Normal,
	ePlayerState_Push,
	ePlayerState_Move,
	ePlayerState_InteractMode,
	ePlayerState_UseItem,
	ePlayerState_Message,
	ePlayerState_Grab,
	ePlayerState_WeaponMelee,
	ePlayerState_Throw,
	ePlayerState_Climb,
	ePlayerState_LastEnum
};

//---------------------------------

enum ePlayerHandType {
	ePlayerHandType_Normal,
	ePlayerHandType_WeaponMelee,
	ePlayerHandType_Throw,
	ePlayerHandType_LastEnum
};

//---------------------------------

enum ePlayerDamageType {
	ePlayerDamageType_BloodSplash,
	ePlayerDamageType_Ice,
	ePlayerDamageType_LastEnum
};

//---------------------------------

enum eCrossHairState {
	eCrossHairState_Inactive,
	eCrossHairState_Active,
	eCrossHairState_Invalid,
	eCrossHairState_Grab,
	eCrossHairState_Examine,
	eCrossHairState_Pointer,
	eCrossHairState_Item,
	eCrossHairState_DoorLink,
	eCrossHairState_PickUp,
	eCrossHairState_Ladder,
	eCrossHairState_None,
	eCrossHairState_Cross,
	eCrossHairState_LastEnum
};

//---------------------------------

enum eGameDifficulty {
	eGameDifficulty_Easy = 0,
	eGameDifficulty_Normal = 1,
	eGameDifficulty_Hard = 2,
	eGameDifficulty_LastEnum
};

//---------------------------------

enum eGameEntityType {
	eGameEntityType_Unkown,
	eGameEntityType_Object,
	eGameEntityType_Door,
	eGameEntityType_DoorPanel,
	eGameEntityType_Area,
	eGameEntityType_Item,
	eGameEntityType_Link,
	eGameEntityType_Enemy,
	eGameEntityType_SwingDoor,
	eGameEntityType_StickArea,
	eGameEntityType_SaveArea,
	eGameEntityType_Lamp,
	eGameEntityType_Ladder,
	eGameEntityType_DamageArea,
	eGameEntityType_ForceArea,
	eGameEntityType_LiquidArea,
	eGameEntityType_LastEnum
};

//---------------------------------

enum eObjectInteractMode {
	eObjectInteractMode_Static,
	eObjectInteractMode_Push,
	eObjectInteractMode_Move,
	eObjectInteractMode_Grab,
	eObjectInteractMode_LastEnum,
};

enum eGameDoorState {
	eGameDoorState_Open,
	eGameDoorState_Opening,
	eGameDoorState_Closed,
	eGameDoorState_Closing,
	eGameDoorState_LastEnum,
};

//---------------------------------
enum eGameEntityScriptType {
	eGameEntityScriptType_PlayerInteract,
	eGameEntityScriptType_PlayerExamine,
	eGameEntityScriptType_PlayerPick,
	eGameEntityScriptType_OnUpdate,
	eGameEntityScriptType_OnBreak,
	eGameEntityScriptType_LastEnum
};

//---------------------------------

enum eGameCollideScriptType {
	eGameCollideScriptType_Enter,
	eGameCollideScriptType_During,
	eGameCollideScriptType_Leave,
	eGameCollideScriptType_LastEnum
};

//---------------------------------

enum eGameItemType {
	eGameItemType_Normal,
	eGameItemType_Notebook,
	eGameItemType_Note,
	eGameItemType_Battery,
	eGameItemType_Flashlight,
	eGameItemType_Food,
	eGameItemType_Map,
	eGameItemType_GlowStick,
	eGameItemType_Flare,
	eGameItemType_Painkillers,
	eGameItemType_WeaponMelee,
	eGameItemType_Throw,
	eGameItemType_LastEnum
};

//---------------------------------

enum eGameTriggerType {
	eGameTriggerType_Sound = (1 << 0),
};

//---------------------------------

//////////////////////////////////////////////////////////////////////////
// CLASSES
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------

class cGameTimer : public iSerializable {
	kSerializableClassInit(cGameTimer) public : cGameTimer() : mbDeleteMe(false), mbPaused(false) {}

	tString msName;
	tString msCallback;

	bool mbGlobal;

	float mfTime;

	bool mbDeleteMe;
	bool mbPaused;

	const tString &GetName() { return msName; }
};

//-----------------------------------------

class cInventoryUseCallback : public iSerializable {
	kSerializableClassInit(cInventoryUseCallback) public : tString msItem;
	tString msObject;
	tString msFunction;
};

typedef Common::MultiMap<tString, cInventoryUseCallback *> tInventoryUseCallbackMap;
typedef tInventoryUseCallbackMap::iterator tInventoryUseCallbackMapIt;

//-----------------------------------------

class cInventoryPickupCallback : public iSerializable {
	kSerializableClassInit(cInventoryPickupCallback) public : tString msItem;
	tString msFunction;
};

typedef Common::MultiMap<tString, cInventoryPickupCallback *> tInventoryPickupCallbackMap;
typedef tInventoryPickupCallbackMap::iterator tInventoryPickupCallbackMapIt;

//-----------------------------------------

class cInventoryCombineCallback : public iSerializable {
	kSerializableClassInit(cInventoryCombineCallback) public : tString msItem1;
	tString msItem2;
	tString msFunction;
	bool bKillMe;
};

typedef Common::List<cInventoryCombineCallback *> tInventoryCombineCallbackList;
typedef tInventoryCombineCallbackList::iterator tInventoryCombineCallbackListIt;

//---------------------------------
class iGameEntity;
class cGameCollideScript {
public:
	cGameCollideScript();

	tString msFuncName[3];

	iGameEntity *mpEntity;
	bool mbCollides;
	bool mbDeleteMe;
};

typedef Hpl1::Std::map<tString, cGameCollideScript *> tGameCollideScriptMap;
typedef tGameCollideScriptMap::iterator tGameCollideScriptMapIt;

//---------------------------------

class cSaveGame_cGameCollideScript : public iSerializable {
	kSerializableClassInit(cSaveGame_cGameCollideScript) public : void LoadFrom(cGameCollideScript *apScript);
	void SaveTo(cGameCollideScript *apScript);

	tString msFuncName[3];
	tString msEntity;
	bool mbCollides;
};

//---------------------------------

#endif // GAME_GAME_TYPES_H
