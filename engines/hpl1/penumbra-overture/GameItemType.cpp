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

#include "hpl1/penumbra-overture/GameItemType.h"

#include "hpl1/penumbra-overture/GameEntity.h"
#include "hpl1/penumbra-overture/GameItem.h"
#include "hpl1/penumbra-overture/GameMessageHandler.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Notebook.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerHands.h"

#include "hpl1/penumbra-overture/HudModel_Throw.h"
#include "hpl1/penumbra-overture/HudModel_Weapon.h"
#include "hpl1/penumbra-overture/PlayerState_Weapon.h"
#include "hpl1/penumbra-overture/PlayerState_WeaponHaptX.h"

//////////////////////////////////////////////////////////////////////////
// BASE ITEM CLASS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameItemType::cGameItemType(cInit *apInit) {
	mpInit = apInit;
}

cGameItemType::~cGameItemType(void) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// NORMAL ITEM
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameItemType_Normal::cGameItemType_Normal(cInit *apInit)
	: cGameItemType(apInit) {
	mvActions.resize(2);
	mvActions[0] = kTranslate("Inventory", "Use");
	mvActions[1] = kTranslate("Inventory", "Drop");

	mvNonDropActions.resize(1);
	mvNonDropActions[0] = kTranslate("Inventory", "Use");
}

bool cGameItemType_Normal::OnAction(cInventoryItem *apItem, int alActionNum) {
	//////////////
	// Use
	if (alActionNum == 0) {
		if (mpInit->mpInventory->IsActive())
			mpInit->mpInventory->SetActive(false);

		mpInit->mpPlayer->ChangeState(ePlayerState_UseItem);
		mpInit->mpPlayer->SetCurrentItem(apItem);
	}
	//////////////
	// Drop
	else if (alActionNum == 1) {
		apItem->Drop();
		mpInit->mpInventory->RemoveItem(apItem);
	}

	return true;
}

//-----------------------------------------------------------------------

void cGameItemType_Normal::OnUse(cInventoryItem *apItem, iGameEntity *apEntity) {
	bool bUsed = apEntity->OnUseItem(apItem);

	if (mpInit->mpInventory->CheckUseCallback(apItem->GetName(), apEntity->GetName()) == false) {
		if (bUsed == false) {
			tString sEntry = "CannotUseItem" + cString::ToString(cMath::RandRectl(1, 5));
			mpInit->mpGameMessageHandler->Add(kTranslate("Inventory", sEntry));
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// NOTEBOOK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameItemType_Notebook::cGameItemType_Notebook(cInit *apInit)
	: cGameItemType(apInit) {
	mvActions.resize(1);
	mvActions[0] = kTranslate("Inventory", "Read");

	mvNonDropActions.resize(1);
	mvNonDropActions[0] = kTranslate("Inventory", "Read");
}

bool cGameItemType_Notebook::OnPickUp(cInventoryItem *apItem, bool abInGame) {
	// mpInit->mpInventory->SetNoteBookActive(true);
	// return false;
	return true;
}

bool cGameItemType_Notebook::OnAction(cInventoryItem *apItem, int alActionNum) {
	//////////////
	// Read
	if (alActionNum == 0) {
		mpInit->mpNotebook->SetActive(true);
	}

	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// NOTE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameItemType_Note::cGameItemType_Note(cInit *apInit)
	: cGameItemType(apInit) {
	mvActions.resize(1);
	mvActions[0] = kTranslate("Inventory", "Read");

	mvNonDropActions.resize(1);
	mvNonDropActions[0] = kTranslate("Inventory", "Read");
}

bool cGameItemType_Note::OnPickUp(cInventoryItem *apItem, bool abInGame) {
	cNotebook_Note *pNote = mpInit->mpNotebook->AddNote(apItem->GetGameName(), "Notes", apItem->GetName());

	if (abInGame) {
		mpInit->mpNotebook->SetActive(true);
		mpInit->mpNotebook->SetSelectedNote(pNote);
		mpInit->mpNotebook->GetStateMachine()->ChangeState(eNotebookState_Note);
	}

	return false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// BATTERY
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameItemType_Battery::cGameItemType_Battery(cInit *apInit)
	: cGameItemType(apInit) {
	mvActions.resize(1);
	mvActions[0] = kTranslate("Inventory", "Read");

	mvNonDropActions.resize(1);
	mvNonDropActions[0] = kTranslate("Inventory", "Read");
}

bool cGameItemType_Battery::OnPickUp(cInventoryItem *apItem, bool abInGame) {
	mpInit->mpPlayer->AddPower(10.0f);
	return false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// FLASHLIGHT
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameItemType_Flashlight::cGameItemType_Flashlight(cInit *apInit)
	: cGameItemType(apInit) {
	mvActions.resize(1);
	mvActions[0] = kTranslate("Inventory", "Turn onoff");

	mvNonDropActions.resize(1);
	mvNonDropActions[0] = kTranslate("Inventory", "Turn onoff");
}

bool cGameItemType_Flashlight::OnAction(cInventoryItem *apItem, int alActionNum) {
	//////////////
	// Read
	if (alActionNum == 0) {
		mpInit->mpPlayer->StartFlashLightButton();
	}

	return true;
}

//-----------------------------------------------------------------------

tWString cGameItemType_Flashlight::GetString(cInventoryItem *apItem) {
	if (mpInit->mpPlayer->GetFlashLight()->IsActive())
		return kTranslate("Inventory", "On");

	return kTranslate("Inventory", "Off");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// GLOW STICK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameItemType_GlowStick::cGameItemType_GlowStick(cInit *apInit)
	: cGameItemType(apInit) {
	mvActions.resize(1);
	mvActions[0] = kTranslate("Inventory", "Equip");

	mvNonDropActions.resize(1);
	mvNonDropActions[0] = kTranslate("Inventory", "Equip");
}

bool cGameItemType_GlowStick::OnAction(cInventoryItem *apItem, int alActionNum) {
	//////////////
	// Equip
	if (alActionNum == 0) {
		if (mpInit->mpPlayer->GetGlowStick()->IsActive()) {
			mpInit->mpPlayer->GetGlowStick()->SetActive(false);
		} else {
			mpInit->mpPlayer->GetGlowStick()->SetActive(true);
			mpInit->mpPlayer->GetFlashLight()->SetActive(false);
		}
	}

	return true;
}

//-----------------------------------------------------------------------

tWString cGameItemType_GlowStick::GetString(cInventoryItem *apItem) {
	if (mpInit->mpPlayer->GetGlowStick()->IsActive())
		return kTranslate("Inventory", "On");

	return kTranslate("Inventory", "Off");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// FLARE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameItemType_Flare::cGameItemType_Flare(cInit *apInit)
	: cGameItemType(apInit) {
	mvActions.resize(1);
	mvActions[0] = kTranslate("Inventory", "Equip");

	mvNonDropActions.resize(1);
	mvNonDropActions[0] = kTranslate("Inventory", "Equip");
}

bool cGameItemType_Flare::OnAction(cInventoryItem *apItem, int alActionNum) {
	//////////////
	// Equip
	if (alActionNum == 0) {
		if (mpInit->mpPlayer->GetFlare()->IsActive() == false) {
			mpInit->mpPlayer->GetFlare()->SetActive(true);

			apItem->AddCount(-1);
			if (apItem->GetCount() <= 0)
				mpInit->mpInventory->RemoveItem(apItem);
		} else {
			mpInit->mpPlayer->GetFlare()->SetActive(false);
		}
	}

	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// WEAPON MELEE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameItemType_WeaponMelee::cGameItemType_WeaponMelee(cInit *apInit)
	: cGameItemType(apInit) {
	mvActions.resize(1);
	mvActions[0] = kTranslate("Inventory", "Equip");

	mvNonDropActions.resize(1);
	mvNonDropActions[0] = kTranslate("Inventory", "Equip");
}

//-----------------------------------------------------------------------

bool cGameItemType_WeaponMelee::OnAction(cInventoryItem *apItem, int alActionNum) {
	//////////////
	// Equip
	if (alActionNum == 0) {
		// Make sure the model is loaded
		if (mpInit->mpPlayerHands->GetModel(apItem->GetHudModelName()) == NULL)
			mpInit->mpPlayerHands->AddModelFromFile(apItem->GetHudModelFile());

		if (mpInit->mpPlayerHands->GetCurrentModel(1) &&
			mpInit->mpPlayerHands->GetCurrentModel(1)->msName == apItem->GetHudModelName()) {
			// mpInit->mpPlayerHands->SetCurrentModel(1, "");

			mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
		} else {
			iHudModel *pHudModel = mpInit->mpPlayerHands->GetModel(apItem->GetHudModelName());
			if (pHudModel == NULL) {
				Error("Hud model with name '%s' does not exist!\n", apItem->GetHudModelName().c_str());
				return true;
			}

			if (mpInit->mbHasHaptics == false)
				mpInit->mpPlayerHands->SetCurrentModel(1, apItem->GetHudModelName());

			//////////////////////////
			// Set up the melee state
			iPlayerState *pState = mpInit->mpPlayer->GetStateData(ePlayerState_WeaponMelee);

			cHudModel_WeaponMelee *pMeleeHud = static_cast<cHudModel_WeaponMelee *>(pHudModel);

			cPlayerState_WeaponMelee *pMeleeState = static_cast<cPlayerState_WeaponMelee *>(pState);
			pMeleeState->SetHudWeapon(pMeleeHud);

			// change state to melee
			mpInit->mpPlayer->ChangeState(ePlayerState_WeaponMelee);
		}
	}

	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// WEAPON MELEE HAPTX
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameItemType_WeaponMeleeHaptX::cGameItemType_WeaponMeleeHaptX(cInit *apInit)
	: cGameItemType(apInit) {
	mvActions.resize(1);
	mvActions[0] = kTranslate("Inventory", "Equip");

	mvNonDropActions.resize(1);
	mvNonDropActions[0] = kTranslate("Inventory", "Equip");
}

//-----------------------------------------------------------------------

bool cGameItemType_WeaponMeleeHaptX::OnAction(cInventoryItem *apItem, int alActionNum) {
	//////////////
	// Equip
	if (alActionNum == 0) {
		// Make sure the model is loaded
		if (mpInit->mpPlayerHands->GetModel(apItem->GetHudModelName()) == NULL)
			mpInit->mpPlayerHands->AddModelFromFile(apItem->GetHudModelFile());

		iHudModel *pHudModel = mpInit->mpPlayerHands->GetModel(apItem->GetHudModelName());
		if (pHudModel == NULL) {
			Error("Hud model with name '%s' does not exist!\n", apItem->GetHudModelName().c_str());
			return true;
		}
		cHudModel_WeaponMelee *pMeleeHud = static_cast<cHudModel_WeaponMelee *>(pHudModel);

		iPlayerState *pState = mpInit->mpPlayer->GetStateData(ePlayerState_WeaponMelee);
		cPlayerState_WeaponMeleeHaptX *pMeleeState = static_cast<cPlayerState_WeaponMeleeHaptX *>(pState);

		if (mpInit->mpPlayer->GetState() == ePlayerState_WeaponMelee && pMeleeState->GetHudWeapon() == pMeleeHud) {
			mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
			// pMeleeState->SetHudWeapon(NULL);
		} else {
			if (mpInit->mbHasHaptics == false)
				mpInit->mpPlayerHands->SetCurrentModel(1, apItem->GetHudModelName());

			if (mpInit->mpPlayer->GetState() == ePlayerState_WeaponMelee) {
				mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
			}

			//////////////////////////
			// Set up the melee state
			iPlayerState *pState2 = mpInit->mpPlayer->GetStateData(ePlayerState_WeaponMelee);

			cPlayerState_WeaponMeleeHaptX *pMeleeState2 = static_cast<cPlayerState_WeaponMeleeHaptX *>(pState2);
			pMeleeState2->SetHudWeapon(pMeleeHud);

			// change state to melee
			mpInit->mpPlayer->ChangeState(ePlayerState_WeaponMelee);

			mpInit->mpInventory->SetActive(false);
		}
	}

	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// THROW
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameItemType_Throw::cGameItemType_Throw(cInit *apInit)
	: cGameItemType(apInit) {
	mvActions.resize(1);
	mvActions[0] = kTranslate("Inventory", "Equip");

	mvNonDropActions.resize(1);
	mvNonDropActions[0] = kTranslate("Inventory", "Equip");
}

//-----------------------------------------------------------------------

bool cGameItemType_Throw::OnAction(cInventoryItem *apItem, int alActionNum) {
	//////////////
	// Equip
	if (alActionNum == 0) {
		// Make sure the model is loaded
		if (mpInit->mpPlayerHands->GetModel(apItem->GetHudModelName()) == NULL)
			mpInit->mpPlayerHands->AddModelFromFile(apItem->GetHudModelFile());

		if (mpInit->mpPlayerHands->GetCurrentModel(1) &&
			mpInit->mpPlayerHands->GetCurrentModel(1)->msName == apItem->GetHudModelName()) {
			// mpInit->mpPlayerHands->SetCurrentModel(1, "");

			mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
		} else {
			iHudModel *pHudModel = mpInit->mpPlayerHands->GetModel(apItem->GetHudModelName());
			if (pHudModel == NULL) {
				Error("Hud model with name '%s' does not exist!\n", apItem->GetHudModelName().c_str());
				return true;
			}

			mpInit->mpPlayerHands->SetCurrentModel(1, apItem->GetHudModelName());

			//////////////////////////
			// Set up the throw state
			iPlayerState *pState = mpInit->mpPlayer->GetStateData(ePlayerState_Throw);
			cPlayerState_Throw *pThrowState = static_cast<cPlayerState_Throw *>(pState);

			cHudModel_Throw *pHud = static_cast<cHudModel_Throw *>(pHudModel);
			pHud->SetItem(apItem);

			pThrowState->SetHudObject(pHud);

			// change state to melee
			mpInit->mpPlayer->ChangeState(ePlayerState_Throw);
		}
	}

	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PAINLKILLERS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameItemType_Painkillers::cGameItemType_Painkillers(cInit *apInit)
	: cGameItemType(apInit) {
	mvActions.resize(1);
	mvActions[0] = kTranslate("Inventory", "Eat");

	mvNonDropActions.resize(1);
	mvNonDropActions[0] = kTranslate("Inventory", "Eat");
}

bool cGameItemType_Painkillers::OnAction(cInventoryItem *apItem, int alActionNum) {
	//////////////
	// Eat
	if (alActionNum == 0) {
		mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("player_eat_painkillers", false, 1);

		mpInit->mpPlayer->SetHealth(100);

		apItem->AddCount(-1);
		if (apItem->GetCount() <= 0) {
			mpInit->mpInventory->RemoveItem(apItem);
		}
	}

	return true;
}

//-----------------------------------------------------------------------
