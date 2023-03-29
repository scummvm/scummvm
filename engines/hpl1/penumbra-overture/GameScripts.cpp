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

#include "hpl1/penumbra-overture/GameScripts.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/AttackHandler.h"
#include "hpl1/penumbra-overture/Credits.h"
#include "hpl1/penumbra-overture/DemoEndText.h"
#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/FadeHandler.h"
#include "hpl1/penumbra-overture/GameArea.h"
#include "hpl1/penumbra-overture/GameDamageArea.h"
#include "hpl1/penumbra-overture/GameEnemy.h"
#include "hpl1/penumbra-overture/GameEntity.h"
#include "hpl1/penumbra-overture/GameForceArea.h"
#include "hpl1/penumbra-overture/GameItem.h"
#include "hpl1/penumbra-overture/GameLadder.h"
#include "hpl1/penumbra-overture/GameLamp.h"
#include "hpl1/penumbra-overture/GameLink.h"
#include "hpl1/penumbra-overture/GameLiquidArea.h"
#include "hpl1/penumbra-overture/GameMessageHandler.h"
#include "hpl1/penumbra-overture/GameMusicHandler.h"
#include "hpl1/penumbra-overture/GameObject.h"
#include "hpl1/penumbra-overture/GameSaveArea.h"
#include "hpl1/penumbra-overture/GameStickArea.h"
#include "hpl1/penumbra-overture/GameSwingDoor.h"
#include "hpl1/penumbra-overture/GraphicsHelper.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/MainMenu.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Notebook.h"
#include "hpl1/penumbra-overture/NumericalPanel.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerHelper.h"
#include "hpl1/penumbra-overture/RadioHandler.h"
#include "hpl1/penumbra-overture/SaveHandler.h"

#include "hpl1/penumbra-overture/GlobalInit.h"

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SCRIPT FUNCTIONS
//////////////////////////////////////////////////////////////////////////

///////// HELPERS ////////////////////////////////////

//-----------------------------------------------------------------------

#define GAME_ENTITY_BEGIN(asName)                                       \
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName); \
	if (pEntity == NULL) {                                              \
		Warning("Couldn't find game entity '%s'\n", asName.c_str());    \
		return;                                                         \
	}

//-----------------------------------------------------------------------

tWString *gsTempString = nullptr;

//-----------------------------------------------------------------------

static void AddToTempString(tString asString) {
	*gsTempString += cString::To16Char(asString);
}
SCRIPT_DEFINE_FUNC_1(void, AddToTempString, string)

static void AddToTempStringTrans(tString asCat, tString asEntry) {
	cInit *mpInit = gpInit;
	*gsTempString += kTranslate(asCat, asEntry);
}
SCRIPT_DEFINE_FUNC_2(void, AddToTempStringTrans, string, string)

static void AddToTempStringAction(tString asAction) {
	cInit *mpInit = gpInit;
	iAction *pAction = gpInit->mpGame->GetInput()->GetAction(asAction);
	if (pAction) {
		tWString sString = kTranslate("ButtonNames", pAction->GetInputName());
		if (sString != _W(""))
			*gsTempString += sString;
		else
			*gsTempString += cString::To16Char(pAction->GetInputName());
	} else {
		*gsTempString += kTranslate("ButtonNames", "None");
	}
}
SCRIPT_DEFINE_FUNC_1(void, AddToTempStringAction, string)

//-----------------------------------------------------------------------

///////// GENERAL GAME ////////////////////////////////////

static void ResetGame() {
	gpInit->mpGame->ResetLogicTimer();

	gpInit->mpMapHandler->SetCurrentMapName("");
	gpInit->mpMainMenu->SetActive(true);
}
SCRIPT_DEFINE_FUNC(void, ResetGame)

//-----------------------------------------------------------------------

static void StartCredits() {
	gpInit->mpCredits->SetActive(true);
}
SCRIPT_DEFINE_FUNC(void, StartCredits)

static void StartDemoEndText() {
	gpInit->mpDemoEndText->SetActive(true);
}
SCRIPT_DEFINE_FUNC(void, StartDemoEndText)

//-----------------------------------------------------------------------

static void ClearSavedMaps() {
	gpInit->mpSaveHandler->ClearSavedMaps();
}
SCRIPT_DEFINE_FUNC(void, ClearSavedMaps)

//-----------------------------------------------------------------------

static tString GetActionKeyString(tString asAction) {
	iAction *pAction = gpInit->mpGame->GetInput()->GetAction(asAction);
	if (pAction) {
		return pAction->GetInputName();
	}

	return "ActionNotFound";
}
SCRIPT_DEFINE_FUNC_1(string, GetActionKeyString, string)

//-----------------------------------------------------------------------

static void AddMessageTrans(tString asTransCat,
							tString asTransName) {
	cInit *mpInit = gpInit;
	gpInit->mpGameMessageHandler->Add(kTranslate(asTransCat, asTransName));
}
SCRIPT_DEFINE_FUNC_2(void, AddMessageTrans, string, string)

//-----------------------------------------------------------------------

static void AddMessage(tString asMessage) {
	gpInit->mpGameMessageHandler->Add(cString::To16Char(asMessage));
}
SCRIPT_DEFINE_FUNC_1(void, AddMessage, string)

//-----------------------------------------------------------------------

static void AddMessageTempString() {
	gpInit->mpGameMessageHandler->Add(*gsTempString);
	*gsTempString = _W("");
}
SCRIPT_DEFINE_FUNC(void, AddMessageTempString)

//-----------------------------------------------------------------------

static void AddSubTitleTrans(tString asTransCat,
							 tString asTransName, float afTime) {
	cInit *mpInit = gpInit;
	gpInit->mpEffectHandler->GetSubTitle()->Add(kTranslate(asTransCat, asTransName), afTime, false);
}
SCRIPT_DEFINE_FUNC_3(void, AddSubTitleTrans, string, string, float)

//-----------------------------------------------------------------------

static void AddSubTitle(tString asMessage, float afTime) {
	gpInit->mpEffectHandler->GetSubTitle()->Add(cString::To16Char(asMessage), afTime, false);
}
SCRIPT_DEFINE_FUNC_2(void, AddSubTitle, string, float)

//-----------------------------------------------------------------------

static void AddSubTitleTempString(float afTime) {
	gpInit->mpEffectHandler->GetSubTitle()->Add(*gsTempString, afTime, false);
	*gsTempString = _W("");
}
SCRIPT_DEFINE_FUNC_1(void, AddSubTitleTempString, float)

//-----------------------------------------------------------------------

static void AddRadioMessage(tString asTransCat, tString asTransName,
							tString asSound) {
	cInit *mpInit = gpInit;
	gpInit->mpRadioHandler->Add(kTranslate(asTransCat, asTransName), asSound);
}
SCRIPT_DEFINE_FUNC_3(void, AddRadioMessage, string, string, string)

//-----------------------------------------------------------------------

static void SetRadioOnEndCallback(tString asFunc) {
	gpInit->mpRadioHandler->SetOnEndCallback(asFunc);
}
SCRIPT_DEFINE_FUNC_1(void, SetRadioOnEndCallback, string)

//-----------------------------------------------------------------------

static void SetInventoryMessage(tString asMessage) {
	gpInit->mpInventory->SetMessage(cString::To16Char(asMessage));
}
SCRIPT_DEFINE_FUNC_1(void, SetInventoryMessage, string)

static void SetInventoryMessageTrans(tString asTransCat,
									 tString asTransName) {
	cInit *mpInit = gpInit;
	gpInit->mpInventory->SetMessage(kTranslate(asTransCat, asTransName));
}
SCRIPT_DEFINE_FUNC_2(void, SetInventoryMessageTrans, string, string)

//-----------------------------------------------------------------------

static void SetMessagesOverCallback(tString asFunction) {
	gpInit->mpGameMessageHandler->SetOnMessagesOverCallback(asFunction);
}
SCRIPT_DEFINE_FUNC_1(void, SetMessagesOverCallback, string)

//-----------------------------------------------------------------------

static void ChangeMap(tString asMapFile, tString asMapPos,
					  tString asStartSound, tString asStopSound,
					  float afFadeOutTime, float afFadeInTime,
					  tString asLoadTextCat, tString asLoadTextEntry) {
	// if(gpInit->mpRadioHandler->IsActive()) return;

	gpInit->mpMapHandler->ChangeMap(asMapFile, asMapPos, asStartSound, asStopSound,
									afFadeOutTime, afFadeInTime,
									asLoadTextCat, asLoadTextEntry);
}
SCRIPT_DEFINE_FUNC_8(void, ChangeMap, string, string, string, string, float, float, string, string)

//-----------------------------------------------------------------------

static void SetMapGameName(tString asName) {
	gpInit->mpMapHandler->SetMapGameName(cString::To16Char(asName));
}
SCRIPT_DEFINE_FUNC_1(void, SetMapGameName, string)

static void SetMapGameNameTrans(tString asTransCat, tString asTransEntry) {
	cInit *mpInit = gpInit;
	gpInit->mpMapHandler->SetMapGameName(kTranslate(asTransCat, asTransEntry));
}
SCRIPT_DEFINE_FUNC_2(void, SetMapGameNameTrans, string, string)

//-----------------------------------------------------------------------

static void AddNotebookTaskText(tString asName, tString asText) {
	gpInit->mpNotebook->AddTask(asName, cString::To16Char(asText));
}
SCRIPT_DEFINE_FUNC_2(void, AddNotebookTaskText, string, string)

//-----------------------------------------------------------------------

static void AddNotebookTask(tString asName, tString asTransCat,
							tString asTransEntry) {
	cInit *mpInit = gpInit;
	gpInit->mpNotebook->AddTask(asName, kTranslate(asTransCat, asTransEntry));
}
SCRIPT_DEFINE_FUNC_3(void, AddNotebookTask, string, string, string)

//-----------------------------------------------------------------------

static void RemoveNotebookTask(tString asName) {
	gpInit->mpNotebook->RemoveTask(asName);
}
SCRIPT_DEFINE_FUNC_1(void, RemoveNotebookTask, string)

//-----------------------------------------------------------------------

static void AddNotebookNote(tString asNameCat, tString asNameEntry,
							tString asTextCat, tString asTextEntry) {
	cInit *mpInit = gpInit;
	gpInit->mpNotebook->AddNote(kTranslate(asNameCat, asNameEntry), asTextCat, asTextEntry);
}
SCRIPT_DEFINE_FUNC_4(void, AddNotebookNote, string, string, string, string)

//-----------------------------------------------------------------------

static void StartNumericalPanel(tString asName, int alCode1, int alCode2, int alCode3, int alCode4,
								float afDifficulty, tString asCallback) {
	gpInit->mpNumericalPanel->SetActive(true);
	tIntVec vCode;
	vCode.resize(4);
	vCode[0] = alCode1;
	vCode[1] = alCode2;
	vCode[2] = alCode3;
	vCode[3] = alCode4;

	gpInit->mpNumericalPanel->SetUp(asName, asCallback);
	gpInit->mpNumericalPanel->SetCode(vCode);
}
SCRIPT_DEFINE_FUNC_7(void, StartNumericalPanel, string, int, int, int, int, float, string)

//-----------------------------------------------------------------------

static void SetInventoryActive(bool abX) {
	gpInit->mpInventory->SetActive(abX);
}
SCRIPT_DEFINE_FUNC_1(void, SetInventoryActive, bool)

//-----------------------------------------------------------------------

static void FadeIn(float afTime) {
	gpInit->mpFadeHandler->FadeIn(afTime);
}
SCRIPT_DEFINE_FUNC_1(void, FadeIn, float)

static void FadeOut(float afTime) {
	gpInit->mpFadeHandler->FadeOut(afTime);
}
SCRIPT_DEFINE_FUNC_1(void, FadeOut, float)

static bool IsFading() {
	return gpInit->mpFadeHandler->IsActive();
}
SCRIPT_DEFINE_FUNC(bool, IsFading)

//-----------------------------------------------------------------------

static void SetWideScreenActive(bool abActive) {
	gpInit->mpFadeHandler->SetWideScreenActive(abActive);
}
SCRIPT_DEFINE_FUNC_1(void, SetWideScreenActive, bool)

//-----------------------------------------------------------------------

static void AutoSave() {
	if (gpInit->mpPlayer->GetHealth() <= 0)
		return;

	// TODO: Some other effect here.
	// gpInit->mpGraphicsHelper->DrawLoadingScreen("other_saving.jpg");
	// gpInit->mpSaveHandler->SaveGameToFile("save/auto.sav");
	// gpInit->mpSaveHandler->AutoSave("Auto",5);
	gpInit->mpEffectHandler->GetSaveEffect()->AutoSave();
}
SCRIPT_DEFINE_FUNC(void, AutoSave)

//-----------------------------------------------------------------------

static void StartFlash(float afFadeIn, float afWhite, float afFadeOut) {
	gpInit->mpEffectHandler->GetFlash()->Start(afFadeIn, afWhite, afFadeOut);
}
SCRIPT_DEFINE_FUNC_3(void, StartFlash, float, float, float)

//-----------------------------------------------------------------------

static void SetWaveGravityActive(bool abX) {
	gpInit->mpEffectHandler->GetWaveGravity()->SetActive(abX);
}
SCRIPT_DEFINE_FUNC_1(void, SetWaveGravityActive, bool)

static void SetupWaveGravity(float afMaxAngle, float afSwingLength,
							 float afGravitySize, tString asAxis) {
	int lDir = cString::ToLowerCase(asAxis) == "x" ? 0 : 1;

	gpInit->mpEffectHandler->GetWaveGravity()->Setup(cMath::ToRad(afMaxAngle), afSwingLength, afGravitySize, lDir);
}
SCRIPT_DEFINE_FUNC_4(void, SetupWaveGravity, float, float, float, string)

//-----------------------------------------------------------------------

static void SetDepthOfFieldActive(bool abX, float afFadeTime) {
	gpInit->mpEffectHandler->GetDepthOfField()->SetActive(abX, afFadeTime);
}
SCRIPT_DEFINE_FUNC_2(void, SetDepthOfFieldActive, bool, float)

//-----------------------------------------------------------------------

static void SetupDepthOfField(float afNearPlane, float afFocalPlane, float afFarPlane) {
	gpInit->mpEffectHandler->GetDepthOfField()->SetUp(afNearPlane, afFocalPlane, afFarPlane);
	gpInit->mpEffectHandler->GetDepthOfField()->SetFocusBody(NULL);
}
SCRIPT_DEFINE_FUNC_3(void, SetupDepthOfField, float, float, float)

//-----------------------------------------------------------------------

static void FocusOnEntity(tString asEntity) {
	GAME_ENTITY_BEGIN(asEntity);

	if (pEntity->GetBodyNum() == 0) {
		Error("Entity %s had no bodies and can not be focus on.\n", pEntity->GetName().c_str());
		return;
	}

	gpInit->mpEffectHandler->GetDepthOfField()->FocusOnBody(pEntity->GetBody(0));
	gpInit->mpEffectHandler->GetDepthOfField()->SetFocusBody(NULL);
}
SCRIPT_DEFINE_FUNC_1(void, FocusOnEntity, string)

//-----------------------------------------------------------------------

static void SetConstantFocusOnEntity(tString asEntity) {
	if (asEntity == "") {
		gpInit->mpEffectHandler->GetDepthOfField()->SetFocusBody(NULL);
		return;
	}

	GAME_ENTITY_BEGIN(asEntity);

	if (pEntity->GetBodyNum() == 0) {
		Error("Entity %s had no bodies and can not be focus on.\n", pEntity->GetName().c_str());
		return;
	}

	gpInit->mpEffectHandler->GetDepthOfField()->SetFocusBody(pEntity->GetBody(0));
}
SCRIPT_DEFINE_FUNC_1(void, SetConstantFocusOnEntity, string)

//-----------------------------------------------------------------------

static void PlayGameMusic(tString asFile, float afVolume, float afFadeStep,
						  bool abLoop, int alPrio) {
	gpInit->mpMusicHandler->Play(asFile, abLoop, afVolume, afFadeStep, alPrio);
}
SCRIPT_DEFINE_FUNC_5(void, PlayGameMusic, string, float, float, bool, int)

//-----------------------------------------------------------------------

static void StopGameMusic(float afFadeStep, int alPrio) {
	gpInit->mpMusicHandler->Stop(afFadeStep, alPrio);
}
SCRIPT_DEFINE_FUNC_2(void, StopGameMusic, float, int)

//-----------------------------------------------------------------------

static void StartScreenShake(float afAmount, float afTime, float afFadeInTime, float afFadeOutTime) {
	gpInit->mpEffectHandler->GetShakeScreen()->Start(afAmount, afTime, afFadeInTime, afFadeOutTime);
}
SCRIPT_DEFINE_FUNC_4(void, StartScreenShake, float, float, float, float)

//-----------------------------------------------------------------------

static void CreateLightFlashAtArea(tString asArea, float afRadius,
								   float afR, float afG, float afB, float afA,
								   float afAddTime, float afNegTime) {
	cAreaEntity *pArea = gpInit->mpGame->GetScene()->GetWorld3D()->GetAreaEntity(asArea);
	if (pArea == NULL) {
		Error("Could not find area '%s'\n", asArea.c_str());
		return;
	}

	gpInit->mpMapHandler->AddLightFlash(pArea->m_mtxTransform.GetTranslation(), afRadius,
										cColor(afR, afG, afB, afA), afAddTime, afNegTime);
}
SCRIPT_DEFINE_FUNC_8(void, CreateLightFlashAtArea, string, float, float, float, float, float, float, float)

//-----------------------------------------------------------------------

///////// ATTACK ////////////////////////////////////

//-----------------------------------------------------------------------

static void CreateSplashDamage(tString asAreaName, float afRadius,
							   float afMinDamage, float afMaxDamge,
							   float afMinForce, float afMaxForce,
							   float afMaxImpulse, int alStrength) {
	///////////////////
	// Get area
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asAreaName);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Area) {
		Warning("Couldn't find area entity '%s'\n", asAreaName.c_str());
		return;
	}
	cGameArea *pArea = static_cast<cGameArea *>(pEntity);

	gpInit->mpAttackHandler->CreateSplashDamage(pArea->GetBody(0)->GetWorldPosition(),
												afRadius, afMinDamage, afMaxDamge,
												afMinForce, afMaxForce, afMaxImpulse,
												eAttackTargetFlag_Bodies |
													eAttackTargetFlag_Player |
													eAttackTargetFlag_Enemy,
												1, alStrength);
}
SCRIPT_DEFINE_FUNC_8(void, CreateSplashDamage, string, float, float, float, float, float, float, int)
//-----------------------------------------------------------------------

///////// GAME TIMER ////////////////////////////////////

//-----------------------------------------------------------------------

static void CreateTimer(tString asName, float afTime, tString asCallback, bool abGlobal) {
	gpInit->mpMapHandler->CreateTimer(asName, afTime, asCallback, abGlobal);
}
SCRIPT_DEFINE_FUNC_4(void, CreateTimer, string, float, string, bool)

static void DestroyTimer(tString asName) {
	cGameTimer *pTimer = gpInit->mpMapHandler->GetTimer(asName);
	if (pTimer == NULL) {
		Warning("Couldn't find timer '%s'\n", asName.c_str());
		return;
	}

	pTimer->mbDeleteMe = true;
}
SCRIPT_DEFINE_FUNC_1(void, DestroyTimer, string)

static void SetTimerPaused(tString asName, bool abPaused) {
	cGameTimer *pTimer = gpInit->mpMapHandler->GetTimer(asName);
	if (pTimer == NULL) {
		Warning("Couldn't find timer '%s'\n", asName.c_str());
		return;
	}

	pTimer->mbPaused = abPaused;
}
SCRIPT_DEFINE_FUNC_2(void, SetTimerPaused, string, bool)

static void SetTimerTime(tString asName, float afTime) {
	cGameTimer *pTimer = gpInit->mpMapHandler->GetTimer(asName);
	if (pTimer == NULL) {
		Warning("Couldn't find timer '%s'\n", asName.c_str());
		return;
	}

	pTimer->mfTime = afTime;
}
SCRIPT_DEFINE_FUNC_2(void, SetTimerTime, string, float)

static void AddTimerTime(tString asName, float afTime) {
	cGameTimer *pTimer = gpInit->mpMapHandler->GetTimer(asName);
	if (pTimer == NULL) {
		Warning("Couldn't find timer '%s'\n", asName.c_str());
		return;
	}

	pTimer->mfTime += afTime;
}
SCRIPT_DEFINE_FUNC_2(void, AddTimerTime, string, float)

static float GetTimerTime(tString asName) {
	cGameTimer *pTimer = gpInit->mpMapHandler->GetTimer(asName);
	if (pTimer == NULL) {
		Warning("Couldn't find timer '%s'\n", asName.c_str());
		return 0.0f;
	}

	return pTimer->mfTime;
}
SCRIPT_DEFINE_FUNC_1(float, GetTimerTime, string)

//-----------------------------------------------------------------------

///////// PLAYER ////////////////////////////////////

//-----------------------------------------------------------------------

static void GivePlayerDamage(float afAmount, tString asType) {
	ePlayerDamageType type = ePlayerDamageType_BloodSplash;
	tString sLowType = cString::ToLowerCase(asType);

	if (sLowType == "bloodsplash")
		type = ePlayerDamageType_BloodSplash;
	else if (sLowType == "ice")
		type = ePlayerDamageType_Ice;
	else
		Warning("Damage type %s does not exist!\n", asType.c_str());

	gpInit->mpPlayer->Damage(afAmount, type);
}
SCRIPT_DEFINE_FUNC_2(void, GivePlayerDamage, float, string)

//-----------------------------------------------------------------------

static void SetPlayerHealth(float afHealth) {
	gpInit->mpPlayer->SetHealth(afHealth);
}
SCRIPT_DEFINE_FUNC_1(void, SetPlayerHealth, float)

static float GetPlayerHealth() {
	return gpInit->mpPlayer->GetHealth();
}
SCRIPT_DEFINE_FUNC(float, GetPlayerHealth)

//-----------------------------------------------------------------------

static void SetPlayerPose(tString asPose, bool abChangeDirectly) {
	tString sPose = cString::ToLowerCase(asPose);

	if (sPose == "stand") {
		gpInit->mpPlayer->ChangeMoveState(ePlayerMoveState_Walk, abChangeDirectly);
	} else if (sPose == "crouch") {
		gpInit->mpPlayer->ChangeMoveState(ePlayerMoveState_Crouch, abChangeDirectly);
	} else {
		Warning("Player pose mode '%s' does not exist\n", asPose.c_str());
	}
}
SCRIPT_DEFINE_FUNC_2(void, SetPlayerPose, string, bool)

static void SetPlayerActive(bool abActive) {
	gpInit->mpPlayer->SetActive(abActive);
}
SCRIPT_DEFINE_FUNC_1(void, SetPlayerActive, bool)

//-----------------------------------------------------------------------

static void StartPlayerLookAt(tString asEntityName, float afSpeedMul, float afMaxSpeed) {
	///////////////////
	// Get entity
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asEntityName);
	if (pEntity == NULL) {
		Warning("Couldn't find entity '%s'\n", asEntityName.c_str());
		return;
	}
	if (pEntity->GetBody(0) == NULL) {
		Warning("Couldn't find a body in entity '%s'\n", asEntityName.c_str());
		return;
	}

	gpInit->mpPlayer->GetLookAt()->SetTarget(pEntity->GetBody(0)->GetWorldPosition(), afSpeedMul, afMaxSpeed);
	gpInit->mpPlayer->GetLookAt()->SetActive(true);
}
SCRIPT_DEFINE_FUNC_3(void, StartPlayerLookAt, string, float, float)

static void StopPlayerLookAt() {
	gpInit->mpPlayer->GetLookAt()->SetActive(false);
}
SCRIPT_DEFINE_FUNC(void, StopPlayerLookAt)

//-----------------------------------------------------------------------

static void StartPlayerFearFilter(float afStrength) {
	gpInit->mpPlayer->GetFearFilter()->SetActive(true);
}
SCRIPT_DEFINE_FUNC_1(void, StartPlayerFearFilter, float)

static void StopPlayerFearFilter() {
	gpInit->mpPlayer->GetFearFilter()->SetActive(false);
}
SCRIPT_DEFINE_FUNC(void, StopPlayerFearFilter)

//-----------------------------------------------------------------------

static void SetFlashlightDisabled(bool abDisabled) {
	gpInit->mpPlayer->GetFlashLight()->SetDisabled(abDisabled);
}
SCRIPT_DEFINE_FUNC_1(void, SetFlashlightDisabled, bool)

//-----------------------------------------------------------------------

///////// INVENTORY ////////////////////////////////////

//-----------------------------------------------------------------------

static void AddPickupCallback(tString asItem, tString asFunction) {
	gpInit->mpInventory->AddPickupCallback(asItem, asFunction);
}
SCRIPT_DEFINE_FUNC_2(void, AddPickupCallback, string, string)

static void AddUseCallback(tString asItem, tString asEntity, tString asFunction) {
	gpInit->mpInventory->AddUseCallback(asItem, asEntity, asFunction);
}
SCRIPT_DEFINE_FUNC_3(void, AddUseCallback, string, string, string)

static void AddCombineCallback(tString asItem1, tString asItem2, tString asFunction) {
	gpInit->mpInventory->AddCombineCallback(asItem1, asItem2, asFunction);
}
SCRIPT_DEFINE_FUNC_3(void, AddCombineCallback, string, string, string)

//-----------------------------------------------------------------------

static void RemovePickupCallback(tString asFunction) {
	gpInit->mpInventory->RemovePickupCallback(asFunction);
}
SCRIPT_DEFINE_FUNC_1(void, RemovePickupCallback, string)

static void RemoveUseCallback(tString asFunction) {
	gpInit->mpInventory->RemoveUseCallback(asFunction);
}
SCRIPT_DEFINE_FUNC_1(void, RemoveUseCallback, string)

static void RemoveCombineCallback(tString asFunction) {
	gpInit->mpInventory->RemoveCombineCallback(asFunction);
}
SCRIPT_DEFINE_FUNC_1(void, RemoveCombineCallback, string)

//-----------------------------------------------------------------------

static bool HasItem(tString asName) {
	// if(cString::ToLowerCase(asName)=="notebook") return gpInit->mpInventory->GetNoteBookActive();

	return gpInit->mpInventory->GetItem(asName) != NULL;
}
SCRIPT_DEFINE_FUNC_1(bool, HasItem, string)

static void RemoveItem(tString asName) {
	cInventoryItem *pItem = gpInit->mpInventory->GetItem(asName);
	if (pItem) {
		gpInit->mpInventory->RemoveItem(pItem);
	} else {
		Warning("Cannot find item '%s' in inventory\n", asName.c_str());
	}
}
SCRIPT_DEFINE_FUNC_1(void, RemoveItem, string)

static void GiveItem(tString asName, tString asEntityFile, int alSlotIndex) {
	gpInit->mpInventory->AddItemFromFile(asName, asEntityFile, alSlotIndex);
}
SCRIPT_DEFINE_FUNC_3(void, GiveItem, string, string, int)

//-----------------------------------------------------------------------

///////// GAME ENTITY PROPERTIES //////////////////////////////////

//-----------------------------------------------------------------------

static void ReplaceEntity(tString asName, tString asBodyName,
						  tString asNewName, tString asNewFile) {
	GAME_ENTITY_BEGIN(asName)

	if (pEntity->GetBodyNum() == 0) {
		Error("Entity '%s' contains no bodies!\n", pEntity->GetName().c_str());
		return;
	}

	iPhysicsBody *pBody = NULL;

	if (asBodyName != "" && pEntity->GetBodyNum() > 1) {
		for (int i = 0; i < pEntity->GetBodyNum(); ++i) {
			tString sBodyName = cString::Sub(pEntity->GetBody(i)->GetName(), (int)asName.size() + 1);
			if (sBodyName == asBodyName) {
				pBody = pEntity->GetBody(i);
				break;
			}
		}

		if (pBody == NULL) {
			Error("Body '%s' could not be found in entity '%s'!\n", asBodyName.c_str(), asName.c_str());
			return;
		}
	} else {
		pBody = pEntity->GetBody(0);
	}

	cMatrixf mtxTransform = pBody->GetWorldMatrix();

	gpInit->mpMapHandler->RemoveGameEntity(pEntity);

	cWorld3D *pWorld = gpInit->mpGame->GetScene()->GetWorld3D();
	pWorld->CreateEntity(asNewName, mtxTransform, asNewFile, true);
}
SCRIPT_DEFINE_FUNC_4(void, ReplaceEntity, string, string, string, string)

//-----------------------------------------------------------------------

static void SetGameEntityActive(tString asName, bool abX) {
	GAME_ENTITY_BEGIN(asName)

	pEntity->SetActive(abX);
}
SCRIPT_DEFINE_FUNC_2(void, SetGameEntityActive, string, bool)

static bool GetGameEntityActive(tString asName) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName);
	if (pEntity == NULL) {
		Warning("Couldn't find game entity '%s'\n", asName.c_str());
		return false;
	}

	return pEntity->IsActive();
}
SCRIPT_DEFINE_FUNC_1(bool, GetGameEntityActive, string)

//-----------------------------------------------------------------------

static void CreateGameEntityVar(tString asEntName, tString asVarName, int alVal) {
	GAME_ENTITY_BEGIN(asEntName);

	pEntity->CreateVar(asVarName, alVal);
}
SCRIPT_DEFINE_FUNC_3(void, CreateGameEntityVar, string, string, int)

static void SetGameEntityVar(tString asEntName, tString asVarName, int alVal) {
	GAME_ENTITY_BEGIN(asEntName);

	pEntity->SetVar(asVarName, alVal);
}
SCRIPT_DEFINE_FUNC_3(void, SetGameEntityVar, string, string, int)

static void AddGameEntityVar(tString asEntName, tString asVarName, int alVal) {
	GAME_ENTITY_BEGIN(asEntName);

	pEntity->AddVar(asVarName, alVal);
}
SCRIPT_DEFINE_FUNC_3(void, AddGameEntityVar, string, string, int)

static int GetGameEntityVar(tString asEntName, tString asVarName) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asEntName);
	if (pEntity == NULL) {
		Warning("Couldn't find game entity '%s'\n", asEntName.c_str());
		return 0;
	}

	return pEntity->GetVar(asVarName);
}
SCRIPT_DEFINE_FUNC_2(int, GetGameEntityVar, string, string)

//-----------------------------------------------------------------------

static void SetGameEntityMaxExamineDist(tString asName, float afDist) {
	GAME_ENTITY_BEGIN(asName)

	pEntity->SetMaxExamineDist(afDist);
}
SCRIPT_DEFINE_FUNC_2(void, SetGameEntityMaxExamineDist, string, float)

static void SetGameEntityMaxInteractDist(tString asName, float afDist) {
	GAME_ENTITY_BEGIN(asName)

	pEntity->SetMaxInteractDist(afDist);
}
SCRIPT_DEFINE_FUNC_2(void, SetGameEntityMaxInteractDist, string, float)

//-----------------------------------------------------------------------

static void SetGameEntityDescriptionTrans(tString asName,
										  tString asTransCat,
										  tString asTransName) {
	cInit *mpInit = gpInit;
	GAME_ENTITY_BEGIN(asName)

	pEntity->SetDescription(kTranslate(asTransCat, asTransName));
	pEntity->SetShowDescritionOnce(false);
}
SCRIPT_DEFINE_FUNC_3(void, SetGameEntityDescriptionTrans, string, string, string)

//-----------------------------------------------------------------------

static void SetGameEntityDescriptionOnceTrans(tString asName,
											  tString asTransCat,
											  tString asTransName) {
	cInit *mpInit = gpInit;
	GAME_ENTITY_BEGIN(asName)

	pEntity->SetDescription(kTranslate(asTransCat, asTransName));
	pEntity->SetShowDescritionOnce(true);
}
SCRIPT_DEFINE_FUNC_3(void, SetGameEntityDescriptionOnceTrans, string, string, string)

//-----------------------------------------------------------------------

static void SetGameEntityDescription(tString asName, tString asMessage) {
	GAME_ENTITY_BEGIN(asName)

	pEntity->SetDescription(cString::To16Char(asMessage));
	pEntity->SetShowDescritionOnce(false);
}
SCRIPT_DEFINE_FUNC_2(void, SetGameEntityDescription, string, string)

static void SetGameEntityGameNameTrans(tString asName,
									   tString asTransCat,
									   tString asTransName) {
	cInit *mpInit = gpInit;
	GAME_ENTITY_BEGIN(asName)

	pEntity->SetGameName(kTranslate(asTransCat, asTransName));
}
SCRIPT_DEFINE_FUNC_3(void, SetGameEntityGameNameTrans, string, string, string)

//-----------------------------------------------------------------------

static void ChangeEntityAnimation(tString asName,
								  tString asAnimation,
								  bool abLoop) {
	GAME_ENTITY_BEGIN(asName)

	pEntity->GetMeshEntity()->PlayName(asAnimation, abLoop, true);
}
SCRIPT_DEFINE_FUNC_3(void, ChangeEntityAnimation, string, string, bool)

//-----------------------------------------------------------------------

static void SetEntityHealth(tString asName, float afHealth) {
	GAME_ENTITY_BEGIN(asName)

	pEntity->SetHealth(afHealth);
}
SCRIPT_DEFINE_FUNC_2(void, SetEntityHealth, string, float)

static void DamageEntity(tString asName, float afDamage, int alStrength) {
	GAME_ENTITY_BEGIN(asName)

	pEntity->Damage(afDamage, alStrength);
}
SCRIPT_DEFINE_FUNC_3(void, DamageEntity, string, float, int)

//-----------------------------------------------------------------------

static void SetDoorState(tString asName,
						 tString asState) {
	/*iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName);
	if(pEntity==NULL || pEntity->GetType() != eGameEntityType_Door)
	{
		Warning("Couldn't find door entity '%s'\n",asName.c_str());
		return;
	}

	cGameDoor *pDoor = static_cast<cGameDoor*>(pEntity);

	asState = cString::ToLowerCase(asState);
	eGameDoorState DoorState;

	if(asState == "open") DoorState = eGameDoorState_Open;
	else if(asState == "closed") DoorState = eGameDoorState_Closed;
	else if(asState == "opening") DoorState = eGameDoorState_Opening;
	else if(asState == "closing") DoorState = eGameDoorState_Closing;
	else{
		Warning("Invalid door state '%s'\n",asState.c_str());
		return;
	}

	pDoor->ChangeDoorState(DoorState);*/
}
SCRIPT_DEFINE_FUNC_2(void, SetDoorState, string, string)

//-----------------------------------------------------------------------

static void SetObjectInteractMode(tString asName, tString asMode) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Object) {
		Warning("Couldn't find object entity '%s'\n", asName.c_str());
		return;
	}

	cGameObject *pObject = static_cast<cGameObject *>(pEntity);

	pObject->SetInteractMode(cEntityLoader_GameObject::ToInteractMode(asMode.c_str()));
}
SCRIPT_DEFINE_FUNC_2(void, SetObjectInteractMode, string, string)

//-----------------------------------------------------------------------

static void SetupLink(tString asName,
					  tString asMapFile, tString asMapPos,
					  tString asStartSound, tString asStopSound,
					  float afFadeOutTime, float afFadeInTime) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Link) {
		Warning("Couldn't find object entity '%s'\n", asName.c_str());
		return;
	}

	cGameLink *pLink = static_cast<cGameLink *>(pEntity);

	pLink->msMapFile = asMapFile;
	pLink->msMapPos = asMapPos;
	pLink->msStartSound = asStartSound;
	pLink->msStopSound = asStopSound;
	pLink->mfFadeInTime = afFadeInTime;
	pLink->mfFadeOutTime = afFadeOutTime;
	pLink->msLoadTextCat = "";
	pLink->msLoadTextEntry = "";
}
SCRIPT_DEFINE_FUNC_7(void, SetupLink, string, string, string, string, string, float, float)

//-----------------------------------------------------------------------

static void SetupLinkLoadText(tString asName,
							  tString asMapFile, tString asMapPos,
							  tString asStartSound, tString asStopSound,
							  float afFadeOutTime, float afFadeInTime,
							  tString asTextCat, tString asTextEntry) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Link) {
		Warning("Couldn't find object entity '%s'\n", asName.c_str());
		return;
	}

	cGameLink *pLink = static_cast<cGameLink *>(pEntity);

	pLink->msMapFile = asMapFile;
	pLink->msMapPos = asMapPos;
	pLink->msStartSound = asStartSound;
	pLink->msStopSound = asStopSound;
	pLink->mfFadeInTime = afFadeInTime;
	pLink->mfFadeOutTime = afFadeOutTime;
	pLink->msLoadTextCat = asTextCat;
	pLink->msLoadTextEntry = asTextEntry;
}
SCRIPT_DEFINE_FUNC_9(void, SetupLinkLoadText, string, string, string, string, string, float, float, string, string)

//-----------------------------------------------------------------------

static void SetAreaCustomIcon(tString asName, tString asIcon) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Area) {
		Warning("Couldn't find area entity '%s'\n", asName.c_str());
		return;
	}
	cGameArea *pArea = static_cast<cGameArea *>(pEntity);

	eCrossHairState aCrosshair = eCrossHairState_None;

	tString sIconName = cString::ToLowerCase(asIcon);

	if (sIconName == "active")
		aCrosshair = eCrossHairState_Active;
	else if (sIconName == "inactive")
		aCrosshair = eCrossHairState_Inactive;
	else if (sIconName == "invalid")
		aCrosshair = eCrossHairState_Invalid;
	else if (sIconName == "grab")
		aCrosshair = eCrossHairState_Grab;
	else if (sIconName == "examine")
		aCrosshair = eCrossHairState_Examine;
	else if (sIconName == "pointer")
		aCrosshair = eCrossHairState_Pointer;
	else if (sIconName == "item")
		aCrosshair = eCrossHairState_Item;
	else if (sIconName == "doorlink")
		aCrosshair = eCrossHairState_DoorLink;
	else if (sIconName == "pickup")
		aCrosshair = eCrossHairState_PickUp;
	else if (sIconName == "none")
		aCrosshair = eCrossHairState_None;
	else
		Warning("Icon type %s not found!\n", asIcon.c_str());

	pArea->SetCustomIcon(aCrosshair);
}
SCRIPT_DEFINE_FUNC_2(void, SetAreaCustomIcon, string, string)

//-----------------------------------------------------------------------

static void AddEnemyPatrolNode(tString asEnemy, tString asNode, float afTime,
							   tString asAnimation) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asEnemy);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Enemy) {
		Warning("Couldn't find enemy entity '%s'\n", asEnemy.c_str());
		return;
	}
	iGameEnemy *pEnemy = static_cast<iGameEnemy *>(pEntity);

	pEnemy->AddPatrolNode(asNode, afTime, asAnimation);
}
SCRIPT_DEFINE_FUNC_4(void, AddEnemyPatrolNode, string, string, float, string)

/**
 * Clears all the patrol nodes for the enemy
 * \param asEnemy The Name of the enemy.
 */
static void ClearEnemyPatrolNodes(tString asEnemy) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asEnemy);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Enemy) {
		Warning("Couldn't find enemy entity '%s'\n", asEnemy.c_str());
		return;
	}
	iGameEnemy *pEnemy = static_cast<iGameEnemy *>(pEntity);

	pEnemy->ClearPatrolNodes();
}
SCRIPT_DEFINE_FUNC_1(void, ClearEnemyPatrolNodes, string)

//-----------------------------------------------------------------------

static void SetEnemyDeathCallback(tString asEnemy, tString asFunction) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asEnemy);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Enemy) {
		Warning("Couldn't find enemy entity '%s'\n", asEnemy.c_str());
		return;
	}
	iGameEnemy *pEnemy = static_cast<iGameEnemy *>(pEntity);

	pEnemy->SetOnDeathCallback(asFunction);
}
SCRIPT_DEFINE_FUNC_2(void, SetEnemyDeathCallback, string, string)

//-----------------------------------------------------------------------

static void SetEnemyAttackCallback(tString asEnemy, tString asFunction) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asEnemy);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Enemy) {
		Warning("Couldn't find enemy entity '%s'\n", asEnemy.c_str());
		return;
	}
	iGameEnemy *pEnemy = static_cast<iGameEnemy *>(pEntity);

	pEnemy->SetOnAttackCallback(asFunction);
}
SCRIPT_DEFINE_FUNC_2(void, SetEnemyAttackCallback, string, string)

//-----------------------------------------------------------------------

static float GetEnemyHealth(tString asEnemy) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asEnemy);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Enemy) {
		Warning("Couldn't find enemy entity '%s'\n", asEnemy.c_str());
		return 0;
	}
	iGameEnemy *pEnemy = static_cast<iGameEnemy *>(pEntity);

	return pEnemy->GetHealth();
}
SCRIPT_DEFINE_FUNC_1(float, GetEnemyHealth, string)

//-----------------------------------------------------------------------

static void SetEnemyUseTriggers(tString asEnemy, bool abUseTriggers) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asEnemy);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Enemy) {
		Warning("Couldn't find enemy entity '%s'\n", asEnemy.c_str());
		return;
	}
	iGameEnemy *pEnemy = static_cast<iGameEnemy *>(pEntity);

	pEnemy->SetUsesTriggers(abUseTriggers);
}
SCRIPT_DEFINE_FUNC_2(void, SetEnemyUseTriggers, string, bool)

//-----------------------------------------------------------------------

static void ShowEnemyPlayer(tString asEnemy) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asEnemy);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Enemy) {
		Warning("Couldn't find enemy entity '%s'\n", asEnemy.c_str());
		return;
	}
	iGameEnemy *pEnemy = static_cast<iGameEnemy *>(pEntity);

	pEnemy->SetLastPlayerPos(gpInit->mpPlayer->GetCharacterBody()->GetFeetPosition() +
							 cVector3f(0, 0.1f, 0));
	pEnemy->ChangeState(STATE_HUNT);
}
SCRIPT_DEFINE_FUNC_1(void, ShowEnemyPlayer, string)

//-----------------------------------------------------------------------

static void SetDoorLocked(tString asDoor, bool abLocked) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asDoor);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_SwingDoor) {
		Warning("Couldn't find swing door entity '%s'\n", asDoor.c_str());
		return;
	}
	cGameSwingDoor *pDoor = static_cast<cGameSwingDoor *>(pEntity);

	pDoor->SetLocked(abLocked);
}
SCRIPT_DEFINE_FUNC_2(void, SetDoorLocked, string, bool)

//-----------------------------------------------------------------------

static void SetupStickArea(tString asArea, bool abCanDeatch,
						   bool abMoveBody, bool abRotateBody,
						   bool abCheckCenterInArea, float afPoseTime,
						   tString asAttachSound, tString asDetachSound,
						   tString asAttachPS, tString asDetachPS,
						   tString asAttachFunc, tString asDetachFunc) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asArea);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_StickArea) {
		Warning("Couldn't find stick area '%s'\n", asArea.c_str());
		return;
	}
	cGameStickArea *pArea = static_cast<cGameStickArea *>(pEntity);

	pArea->SetCanDeatch(abCanDeatch);

	pArea->SetRotateBody(abRotateBody);
	pArea->SetMoveBody(abMoveBody);

	pArea->SetCheckCenterInArea(abCheckCenterInArea);

	pArea->SetPoseTime(afPoseTime);

	pArea->SetAttachSound(asAttachSound);
	pArea->SetDetachSound(asDetachSound);

	pArea->SetAttachPS(asAttachPS);
	pArea->SetDetachPS(asDetachPS);

	pArea->SetAttachFunction(asAttachFunc);
	pArea->SetDetachFunction(asDetachFunc);
}
SCRIPT_DEFINE_FUNC_12(void, SetupStickArea, string, bool, bool, bool, bool, float,
					  string, string, string, string, string, string)

//-----------------------------------------------------------------------

static void AllowAttachment() {
	cGameStickArea::mbAllowAttachment = true;
}
SCRIPT_DEFINE_FUNC(void, AllowAttachment)

//-----------------------------------------------------------------------

static void SetLampLit(tString asName, bool abLit, bool abFade) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Lamp) {
		Warning("Couldn't find lamp '%s'\n", asName.c_str());
		return;
	}
	cGameLamp *pLamp = static_cast<cGameLamp *>(pEntity);

	pLamp->SetLit(abLit, abFade);
}
SCRIPT_DEFINE_FUNC_3(void, SetLampLit, string, bool, bool)

//-----------------------------------------------------------------------

static void SetLampFlicker(tString asName, bool abFlicker) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Lamp) {
		Warning("Couldn't find lamp '%s'\n", asName.c_str());
		return;
	}
	cGameLamp *pLamp = static_cast<cGameLamp *>(pEntity);

	pLamp->SetFlicker(abFlicker);
}
SCRIPT_DEFINE_FUNC_2(void, SetLampFlicker, string, bool)

//-----------------------------------------------------------------------

static void SetLampLitChangeCallback(tString asName, tString asCallback) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Lamp) {
		Warning("Couldn't find lamp '%s'\n", asName.c_str());
		return;
	}
	cGameLamp *pLamp = static_cast<cGameLamp *>(pEntity);

	pLamp->SetLitChangeCallback(asCallback);
}
SCRIPT_DEFINE_FUNC_2(void, SetLampLitChangeCallback, string, string)

//-----------------------------------------------------------------------

static void SetupLadder(tString asName,
						tString asAttachSound,
						tString asClimbUpSound,
						tString asClimbDownSound) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_Ladder) {
		Warning("Couldn't find ladder '%s'\n", asName.c_str());
		return;
	}
	cGameLadder *pLadder = static_cast<cGameLadder *>(pEntity);

	pLadder->SetAttachSound(asAttachSound);
	pLadder->SetClimbUpSound(asClimbUpSound);
	pLadder->SetClimbDownSound(asClimbDownSound);
}
SCRIPT_DEFINE_FUNC_4(void, SetupLadder, string, string, string, string)

//-----------------------------------------------------------------------

static void SetupDamageArea(tString asName, float afDamage,
							float afUpdatesPerSec, int alStrength,
							bool abDisableObjects, bool abDisableEnemies) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_DamageArea) {
		Warning("Couldn't find damage area '%s'\n", asName.c_str());
		return;
	}
	cGameDamageArea *pDamage = static_cast<cGameDamageArea *>(pEntity);

	pDamage->SetDamage(afDamage);
	pDamage->SetUpdatesPerSec(afUpdatesPerSec);
	pDamage->SetStrength(alStrength);
	pDamage->SetDisableObjects(abDisableObjects);
	pDamage->SetDisableEnemies(abDisableEnemies);
}
SCRIPT_DEFINE_FUNC_6(void, SetupDamageArea, string, float, float, int, bool, bool)

//-----------------------------------------------------------------------

static void SetupForceArea(tString asName,
						   float afMaxForce, float afConstant,
						   float afDestSpeed, float afMaxMass,
						   bool abMulWithMass, bool abForceAtPoint,
						   bool abAffectBodies, bool abAffectCharacters) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_ForceArea) {
		Warning("Couldn't find force area '%s'\n", asName.c_str());
		return;
	}
	cGameForceArea *pForce = static_cast<cGameForceArea *>(pEntity);

	pForce->SetMaxForce(afMaxForce);
	pForce->SetConstant(afConstant);
	pForce->SetDestSpeed(afDestSpeed);
	pForce->SetMaxMass(afMaxMass);
	pForce->SetMulWithMass(abMulWithMass);
	pForce->SetForceAtPoint(abForceAtPoint);
	pForce->SetAffectBodies(abAffectBodies);
	pForce->SetAffectCharacters(abAffectCharacters);
}
SCRIPT_DEFINE_FUNC_9(void, SetupForceArea, string, float, float, float, float, bool, bool, bool, bool)

//-----------------------------------------------------------------------

static void SetupLiquidArea(tString asName,
							float afDensity, float afLinearViscosity,
							float afAngularViscosity,
							tString asPhysicsMaterial,
							float fR, float fG, float fB,
							bool abHasWaves) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_LiquidArea) {
		Warning("Couldn't find liquid area '%s'\n", asName.c_str());
		return;
	}
	cGameLiquidArea *pLiquid = static_cast<cGameLiquidArea *>(pEntity);

	pLiquid->SetDensity(afDensity);
	pLiquid->SetLinearViscosity(afLinearViscosity);
	pLiquid->SetAngularViscosity(afAngularViscosity);

	pLiquid->SetPhysicsMaterial(asPhysicsMaterial);

	pLiquid->SetColor(cColor(fR, fG, fB, 1));

	pLiquid->SetHasWaves(abHasWaves);
}
SCRIPT_DEFINE_FUNC_9(void, SetupLiquidArea, string, float, float, float, string, float, float, float, bool)

//-----------------------------------------------------------------------

static void SetupSaveArea(tString asName,
						  tString asMessageCat, tString asMessageEntry,
						  tString asSound) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asName);
	if (pEntity == NULL || pEntity->GetType() != eGameEntityType_SaveArea) {
		Warning("Couldn't find save area '%s'\n", asName.c_str());
		return;
	}
	cGameSaveArea *pSave = static_cast<cGameSaveArea *>(pEntity);

	pSave->SetMessageCat(asMessageCat);
	pSave->SetMessageEntry(asMessageEntry);
	pSave->SetSound(asSound);
}
SCRIPT_DEFINE_FUNC_4(void, SetupSaveArea, string, string, string, string)

//-----------------------------------------------------------------------

///////// GAME ENTITY CALLBACKS //////////////////////////////////

//-----------------------------------------------------------------------

static eGameCollideScriptType GetGameCollideScriptType(const tString &asType) {
	tString sName = cString::ToLowerCase(asType);

	if (sName == "enter")
		return eGameCollideScriptType_Enter;
	if (sName == "leave")
		return eGameCollideScriptType_Leave;
	if (sName == "during")
		return eGameCollideScriptType_During;

	Warning("Collide Type %s doesn't exist!\n", asType.c_str());

	return eGameCollideScriptType_LastEnum;
}

//////////////////////////////

static void AddEntityCollideCallback(tString asType,
									 tString asDestName,
									 tString asEntityName,
									 tString asFuncName) {
	if (cString::ToLowerCase(asDestName) == "player") {
		eGameCollideScriptType type = GetGameCollideScriptType(asType);

		if (type != eGameCollideScriptType_LastEnum)
			gpInit->mpPlayer->AddCollideScript(type, asFuncName, asEntityName);
	} else {
		iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asDestName);
		if (pEntity == NULL) {
			Warning("Couldn't find entity '%s'\n", asDestName.c_str());
			return;
		}

		eGameCollideScriptType type = GetGameCollideScriptType(asType);

		if (type != eGameCollideScriptType_LastEnum)
			pEntity->AddCollideScript(type, asFuncName, asEntityName);
	}
}
SCRIPT_DEFINE_FUNC_4(void, AddEntityCollideCallback, string, string, string, string)

//////////////////////////////

static void RemoveEntityCollideCallback(tString asType,
										tString asDestName,
										tString asEntityName) {
	if (cString::ToLowerCase(asDestName) == "player") {
		eGameCollideScriptType type = GetGameCollideScriptType(asType);

		if (type != eGameCollideScriptType_LastEnum)
			gpInit->mpPlayer->RemoveCollideScript(type, asEntityName);
	} else {
		iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asDestName);
		if (pEntity == NULL) {
			Warning("Couldn't find entity '%s'\n", asDestName.c_str());
			return;
		}

		eGameCollideScriptType type = GetGameCollideScriptType(asType);

		if (type != eGameCollideScriptType_LastEnum) {
			pEntity->RemoveCollideScript(type, asEntityName);
		}
	}
}
SCRIPT_DEFINE_FUNC_3(void, RemoveEntityCollideCallback, string, string, string)

//-----------------------------------------------------------------------

static eGameEntityScriptType GetGameScriptType(const tString &asType) {
	tString sName = cString::ToLowerCase(asType);

	if (sName == "playerinteract")
		return eGameEntityScriptType_PlayerInteract;
	if (sName == "playerexamine")
		return eGameEntityScriptType_PlayerExamine;
	if (sName == "playerlook")
		return eGameEntityScriptType_PlayerPick;
	if (sName == "onupdate")
		return eGameEntityScriptType_OnUpdate;
	if (sName == "onbreak")
		return eGameEntityScriptType_OnBreak;

	Warning("Script type '%s' doesn't exist!\n", asType.c_str());

	return eGameEntityScriptType_LastEnum;
}

/////////////////////

static void AddEntityCallback(tString asType,
							  tString asDestName,
							  tString asFuncName) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asDestName);
	if (pEntity == NULL) {
		Warning("Couldn't find entity '%s'\n", asDestName.c_str());
		return;
	}

	eGameEntityScriptType type = GetGameScriptType(asType);

	if (type != eGameEntityScriptType_LastEnum) {
		pEntity->AddScript(type, asFuncName);
	}
}
SCRIPT_DEFINE_FUNC_3(void, AddEntityCallback, string, string, string)

/////////////////////

static void RemoveEntityCallback(tString asType,
								 tString asDestName) {
	iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asDestName);
	if (pEntity == NULL) {
		Warning("Couldn't find entity '%s'\n", asDestName.c_str());
		return;
	}

	eGameEntityScriptType type = GetGameScriptType(asType);

	if (type != eGameEntityScriptType_LastEnum) {
		pEntity->RemoveScript(type);
	}
}
SCRIPT_DEFINE_FUNC_2(void, RemoveEntityCallback, string, string)

//-----------------------------------------------------------------------

///////// GAME SOUND //////////////////////////////////

//-----------------------------------------------------------------------

static void CreateSoundEntityAt(tString asType, tString asDestName,
								tString asSoundName, tString asSoundFile) {
	cWorld3D *pWorld = gpInit->mpGame->GetScene()->GetWorld3D();
	iPhysicsWorld *pPhysicsWorld = gpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	int lType = 0;
	tString sTypeLow = cString::ToLowerCase(asType);

	if (sTypeLow == "body")
		lType = 1;
	else if (sTypeLow == "joint")
		lType = 2;
	else if (sTypeLow == "entity")
		lType = 3;

	if (lType == 0) {
		Warning("Cannot find type '%s' for sound entity position.\n", asType.c_str());
		return;
	}

	cVector3f vPos;

	////////////////////////
	// Body
	if (lType == 1) {
		iPhysicsBody *pBody = pPhysicsWorld->GetBody(asDestName);
		if (pBody == NULL) {
			Warning("Body '%s' coudln't be found!\n", asDestName.c_str());
			return;
		}

		vPos = pBody->GetLocalPosition();
	}
	////////////////////////
	// Joint
	else if (lType == 2) {
		iPhysicsJoint *pJoint = pPhysicsWorld->GetJoint(asDestName);
		if (pJoint == NULL) {
			Warning("Body '%s' coudln't be found!\n", asDestName.c_str());
			return;
		}

		vPos = pJoint->GetPivotPoint();
	}
	////////////////////////
	// Entity
	{
		iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(asDestName);
		if (pEntity == NULL) {
			Warning("Couldn't find entity '%s'\n", asDestName.c_str());
			return;
		}

		if (pEntity->GetMeshEntity())
			vPos = pEntity->GetMeshEntity()->GetWorldPosition();
		else
			vPos = pEntity->GetBody(0)->GetLocalPosition();
	}

	////////////////////////
	// Create sound.
	cSoundEntity *pSound = pWorld->CreateSoundEntity(asSoundName, asSoundFile, true);
	if (pSound) {
		pSound->SetPosition(vPos);
	}
}
SCRIPT_DEFINE_FUNC_4(void, CreateSoundEntityAt, string, string, string, string)

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameScripts::Init() {
	LowLevelSystem *pLowLevelSystem = gpInit->mpGame->GetSystem()->GetLowLevel();
	gsTempString = new tWString;

	// Game helper
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddToTempString));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddToTempStringTrans));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddToTempStringAction));

	// Game general
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(ResetGame));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(StartCredits));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(StartDemoEndText));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(ClearSavedMaps));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(GetActionKeyString));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddMessageTrans));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddMessage));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddMessageTempString));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddSubTitleTrans));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddSubTitle));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddSubTitleTempString));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddRadioMessage));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetRadioOnEndCallback));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetMessagesOverCallback));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(ChangeMap));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetMapGameName));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetMapGameNameTrans));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddNotebookTaskText));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddNotebookTask));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(RemoveNotebookTask));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddNotebookNote));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(StartNumericalPanel));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetInventoryActive));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(FadeIn));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(FadeOut));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(IsFading));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetWideScreenActive));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AutoSave));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(StartFlash));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetWaveGravityActive));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetupWaveGravity));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetDepthOfFieldActive));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetupDepthOfField));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(FocusOnEntity));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetConstantFocusOnEntity));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(PlayGameMusic));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(StopGameMusic));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(StartScreenShake));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(CreateLightFlashAtArea));

	// Attack
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(CreateSplashDamage));

	// Game timer
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(CreateTimer));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(DestroyTimer));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetTimerPaused));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetTimerTime));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddTimerTime));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(GetTimerTime));

	// Player
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(GivePlayerDamage));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetPlayerHealth));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(GetPlayerHealth));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetPlayerPose));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetPlayerActive));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(StartPlayerLookAt));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(StopPlayerLookAt));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(StartPlayerFearFilter));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(StopPlayerFearFilter));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetFlashlightDisabled));

	// Inventory
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddPickupCallback));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddUseCallback));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddCombineCallback));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(RemovePickupCallback));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(RemoveUseCallback));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(RemoveCombineCallback));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(HasItem));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(RemoveItem));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(GiveItem));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetInventoryMessage));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetInventoryMessageTrans));

	// Game entity properties
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(ReplaceEntity));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetGameEntityActive));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(GetGameEntityActive));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetGameEntityMaxExamineDist));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetGameEntityMaxInteractDist));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(CreateGameEntityVar));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetGameEntityVar));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddGameEntityVar));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(GetGameEntityVar));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetGameEntityDescriptionTrans));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetGameEntityDescriptionOnceTrans));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetGameEntityDescription));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetGameEntityGameNameTrans));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(ChangeEntityAnimation));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetEntityHealth));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(DamageEntity));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetDoorState));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetObjectInteractMode));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetupLink));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetupLinkLoadText));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetAreaCustomIcon));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddEnemyPatrolNode));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(ClearEnemyPatrolNodes));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetEnemyDeathCallback));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetEnemyAttackCallback));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(GetEnemyHealth));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetEnemyUseTriggers));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(ShowEnemyPlayer));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetDoorLocked));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetupStickArea));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AllowAttachment));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetLampLit));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetLampLitChangeCallback));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetLampFlicker));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetupLadder));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetupDamageArea));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetupForceArea));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetupLiquidArea));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(SetupSaveArea));

	// Game entity callbacks
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddEntityCollideCallback));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(RemoveEntityCollideCallback));

	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(AddEntityCallback));
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(RemoveEntityCallback));

	// Game sound
	pLowLevelSystem->addScriptFunc(SCRIPT_REGISTER_FUNC(CreateSoundEntityAt));
}

void cGameScripts::finalize() {
	delete gsTempString;
}

//-----------------------------------------------------------------------
