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

#include "hpl1/penumbra-overture/Inventory.h"

#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/GameItem.h"
#include "hpl1/penumbra-overture/GameItemType.h"
#include "hpl1/penumbra-overture/GameMessageHandler.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Notebook.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/RadioHandler.h"
#include "hpl1/penumbra-overture/SaveHandler.h"

#include "hpl1/algorithms.h"
#include "hpl1/penumbra-overture/GlobalInit.h"

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cInventory::cInventory(cInit *apInit) : iUpdateable("Inventory") {
	mpInit = apInit;
	mpDrawer = apInit->mpGame->GetGraphics()->GetDrawer();

	mpGfxBackground = mpDrawer->CreateGfxObject("inventory_background.bmp", "diffalpha2d");

	// mpBatteryMeter = mpDrawer->CreateGfxObject("inventory_battery_meter.bmp","diffalpha2d");
	// mpBatteryMeterBar = mpDrawer->CreateGfxObject("inventory_battery_meter_bar.bmp","diffalpha2d");

	// mpBagpack = mpDrawer->CreateGfxObject("inventory_backpack.bmp","diffalpha2d");

	// mpHealthFrame = mpDrawer->CreateGfxObject("inventory_health_frame.bmp","diffalpha2d");
	// mpHealthBack = mpDrawer->CreateGfxObject("inventory_health_background.bmp","diffalpha2d");
	// mpHealthMan_Fine = mpDrawer->CreateGfxObject("inventory_health_fine.bmp","diffalpha2d");
	// mpHealthMan_Caution = mpDrawer->CreateGfxObject("inventory_health_caution.bmp","diffalpha2d");
	// mpHealthMan_Danger = mpDrawer->CreateGfxObject("inventory_health_danger.bmp","diffalpha2d");
	// mpHealthTextFrame = mpDrawer->CreateGfxObject("inventory_health_text_slot.bmp","diffalpha2d");

	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("verdana.fnt");

	mpContext = hplNew(cInventoryContext, (mpInit));

	mbMessageActive = false;
	msMessage = _W("");
	mfMessageAlpha = 0;

	mpMessageBackground = mpDrawer->CreateGfxObject("effect_black.bmp", "diffalpha2d");

	///////////////////////////////////
	// Init normal slots
	cInventorySlot *pSlot = NULL;
	int lCount = 0;

	cVector2f vSlotBegin(400 - 77 * 2.5f, 15 + 69 + 5);
	for (float y = 0; y < 4; ++y)
		for (float x = 0; x < 6; ++x) {
			pSlot = hplNew(cInventorySlot, (mpInit, vSlotBegin + cVector2f(x * 77, y * 69), false, lCount++));
			AddWidget(pSlot);
			mlstSlots.push_back(pSlot);
		}

	///////////////////////////////////
	// Init equip slots
	int lEquipSlot = 0;
	// cVector2f vEquipSlotBegin = cVector2f(400 - 77*5,10);
	cVector2f vEquipSlotBegin = cVector2f(400 - 77 * 4.5f, 15);
	for (float x = 0; x < 9; ++x) {
		pSlot = hplNew(cInventorySlot, (mpInit, vEquipSlotBegin + cVector2f(x * 77, 0), true, lCount++));
		AddWidget(pSlot);
		mlstSlots.push_back(pSlot);
		pSlot->SetEquipIndex(lEquipSlot++);
		mvEquipSlots.push_back(pSlot);
	}

	///////////////////////////////////
	// Init other widgests
	cInventoryBattery *pBattery = hplNew(cInventoryBattery, (mpInit,
															 cRect2f(400 - 77 * 3.5f, 15 + 69 + 5, 77, 135),
															 NULL, 30.0f));
	AddWidget(pBattery);

	cInventoryHealth *pHealth = hplNew(cInventoryHealth, (mpInit,
														  cRect2f(400 - 77 * 3.5f, 15 + 69 * 3 + 5,
																  77, 135),
														  NULL, 30.0f));
	AddWidget(pHealth);

	///////////////////////////////////
	// Init items types
	Hpl1::resizeAndFill(mvItemTypes, eGameItemType_LastEnum, nullptr);

	mvItemTypes[eGameItemType_Normal] = hplNew(cGameItemType_Normal, (mpInit));
	mvItemTypes[eGameItemType_Notebook] = hplNew(cGameItemType_Notebook, (mpInit));
	mvItemTypes[eGameItemType_Note] = hplNew(cGameItemType_Note, (mpInit));
	mvItemTypes[eGameItemType_Battery] = hplNew(cGameItemType_Battery, (mpInit));
	mvItemTypes[eGameItemType_Flashlight] = hplNew(cGameItemType_Flashlight, (mpInit));
	mvItemTypes[eGameItemType_GlowStick] = hplNew(cGameItemType_GlowStick, (mpInit));
	mvItemTypes[eGameItemType_Flare] = hplNew(cGameItemType_Flare, (mpInit));
	mvItemTypes[eGameItemType_Painkillers] = hplNew(cGameItemType_Painkillers, (mpInit));
	mvItemTypes[eGameItemType_WeaponMelee] = hplNew(cGameItemType_WeaponMelee, (mpInit));
	mvItemTypes[eGameItemType_Throw] = hplNew(cGameItemType_Throw, (mpInit));

	Reset();
}

cInventory::~cInventory(void) {
	hplDelete(mpContext);

	ClearCallbacks();

	for (size_t i = 0; i < mvItemTypes.size(); ++i) {
		if (mvItemTypes[i])
			hplDelete(mvItemTypes[i]);
	}

	STLDeleteAll(mlstWidgets);
	STLMapDeleteAll(m_mapItems);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC WIDGET METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iInventoryWidget::iInventoryWidget(cInit *apInit, const cRect2f &aRect, cGfxObject *apGfxObject, float afZ) {
	mpInit = apInit;
	mRect = aRect;
	mpGfxObject = apGfxObject;
	mfZ = afZ;

	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();
}

void iInventoryWidget::Draw() {
	if (mpGfxObject == NULL)
		return;

	cVector2l vSize = mpGfxObject->GetMaterial()->GetImage(eMaterialTexture_Diffuse)->GetSize();
	mpDrawer->DrawGfxObject(mpGfxObject, cVector3f(mRect.x, mRect.y, mfZ),
							cVector2f((float)vSize.x, (float)vSize.y),
							cColor(1, mpInit->mpInventory->GetAlpha()));
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// INVENTORY SLOT METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cInventorySlot::cInventorySlot(cInit *apInit, const cVector2f &avPos, bool abEquip, int alIndex)
	: iInventoryWidget(apInit, cRect2f(avPos.x, avPos.y, 77, 66), NULL, 10.0f) {
	mpGfxObject = mpDrawer->CreateGfxObject("inventory_slot.bmp", "diffalpha2d");
	mpItem = NULL;
	mbEquip = abEquip;
	mlIndex = alIndex;

	mpGfxBack = NULL;

	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("verdana.fnt");
}

//-----------------------------------------------------------------------

void cInventorySlot::OnDraw() {
	if (mpItem) {
		cVector3f vPos(mRect.x + 4, mRect.y + 4, 5);
		cVector2l vSize = mpItem->GetGfxObject()->GetMaterial()->GetImage(eMaterialTexture_Diffuse)->GetSize();

		mpDrawer->DrawGfxObject(mpItem->GetGfxObject(), vPos,
								cVector2f((float)vSize.x, (float)vSize.y),
								cColor(1, mpInit->mpInventory->GetAlpha()));

		cGameItemType *pType = mpInit->mpInventory->GetItemType(mpItem->GetItemType());

		if (pType && pType->GetString(mpItem) != _W("")) {
			tWString sString = pType->GetString(mpItem);
			mpFont->draw(vPos + cVector3f(0, 0, 1), 12, cColor(1, mpInit->mpInventory->GetAlpha()), eFontAlign_Left,
						 sString);
		} else if (mpItem->HasCount()) {
			mpFont->draw(vPos + cVector3f(0, 0, 1), 12, cColor(1, mpInit->mpInventory->GetAlpha()), eFontAlign_Left,
						 Common::U32String::format("%d", mpItem->GetCount()));
		}
	}

	// Equip slot specifics
	if (mbEquip) {
		if (mpGfxBack == NULL) {
			mpGfxBack = mpDrawer->CreateGfxObject("inventory_slot_equip" + cString::ToString(mlEquipIndex + 1) + ".bmp", "diffalpha2d");
		}

		mpDrawer->DrawGfxObject(mpGfxBack, cVector3f(mRect.x, mRect.y, 1), cVector2f(77, 66),
								cColor(1, mpInit->mpInventory->GetAlpha() * 0.23f));
		/*cVector3f vPos(mRect.x + 2,mRect.y + 3 ,11);
		tString sString = cString::ToString(mlEquipIndex+1);

		cColor Col(1,1,1,mpInit->mpInventory->GetAlpha());

		//mpFont->Draw(vPos,13,Col,eFontAlign_Center,sString.c_str());

		//Col = cColor(0,0,0,mpInit->mpInventory->GetAlpha());
		mpFont->Draw(vPos+cVector3f(1,1,-1),13,Col,eFontAlign_Center,sString.c_str());
		mpFont->Draw(vPos+cVector3f(-1,-1,-1),13,Col,eFontAlign_Center,sString.c_str());*/
	}
}

//-----------------------------------------------------------------------

void cInventorySlot::OnMouseOver() {
	/////////////////////////////
	// Moving item
	if (mpInit->mpInventory->GetCurrentItem()) {
		cInventoryItem *pCurrentItem = mpInit->mpInventory->GetCurrentItem();
		if (mpInit->mpInventory->GetCurrentSlot() == this) {
			cGameItemType *pType = mpInit->mpInventory->GetItemType(pCurrentItem->GetItemType());
			tString sShortcutAction = pType->GetShortCutAction(pCurrentItem);
			if (sShortcutAction != "") {
				tWString wsName = pCurrentItem->GetGameName();

				iAction *pAction = mpInit->mpGame->GetInput()->GetAction(sShortcutAction);
				if (pAction)
					wsName = wsName + _W(" (") + cString::To16Char(pAction->GetInputName()) + _W(")");

				mpInit->mpInventory->SetItemName(wsName);
			} else {
				mpInit->mpInventory->SetItemName(pCurrentItem->GetGameName());
			}
			mpInit->mpInventory->SetItemDesc(pCurrentItem->GetDescription());
		} else if (mpItem) {
			mpInit->mpInventory->SetItemDesc(_W(""));

			tWString sDesc = kTranslate("Inventory", "Combine") + _W(" ") + pCurrentItem->GetGameName() + _W(" ") +
							 kTranslate("Inventory", "with") + _W(" ") + mpItem->GetGameName();
			mpInit->mpInventory->SetItemName(sDesc);
		}
	}
	/////////////////////////////
	// Not moving item
	else {
		if (mpItem) {
			cGameItemType *pType = mpInit->mpInventory->GetItemType(mpItem->GetItemType());
			tString sShortcutAction = pType->GetShortCutAction(mpItem);
			if (sShortcutAction != "") {
				tWString wsName = mpItem->GetGameName();

				iAction *pAction = mpInit->mpGame->GetInput()->GetAction(sShortcutAction);
				if (pAction)
					wsName = wsName + _W(" (") + cString::To16Char(pAction->GetInputName()) + _W(")");

				mpInit->mpInventory->SetItemName(wsName);
			} else {
				mpInit->mpInventory->SetItemName(mpItem->GetGameName());
			}
			mpInit->mpInventory->SetItemDesc(mpItem->GetDescription());
		}
	}
}

//-----------------------------------------------------------------------

void cInventorySlot::OnMouseDown(eMButton aButton) {
	/////////////////////////////////
	// Left button
	if (aButton == eMButton_Left) {
		if (mpItem != NULL) {
			mpInit->mpInventory->SetCurrentItem(mpItem);
			mpInit->mpInventory->SetCurrentSlot(this);
			mpItem = NULL;

			cVector2f vOffset = cVector2f(mRect.x + 4, mRect.y + 4) -
								mpInit->mpInventory->GetMousePos();
			mpInit->mpInventory->SetCurrentItemOffset(vOffset);
		}
	}
	/////////////////////////////////
	// Right button
	else if (aButton == eMButton_Right) {
		if (mpItem) {
			mpInit->mpInventory->GetContext()->SetActive(true);
			mpInit->mpInventory->GetContext()->Setup(mpItem, mpInit->mpInventory->GetMousePos());
		}
	}
}

//-----------------------------------------------------------------------

void cInventorySlot::OnMouseUp(eMButton aButton) {
	/////////////////////////////////
	// Left mouse
	if (aButton == eMButton_Left) {
		if (mpInit->mpInventory->GetCurrentItem()) {
			if (mpItem == NULL) {
				mpItem = mpInit->mpInventory->GetCurrentItem();
			} else {
				tString sCurrentItemName = mpInit->mpInventory->GetCurrentItem()->GetName();
				if (mpInit->mpInventory->CheckCombineCallback(mpItem->GetName(), sCurrentItemName, mlIndex)) {
					// mpInit->mpInventory->RemoveItem(mpInit->mpInventory->GetCurrentItem());
					if (mpInit->mpInventory->GetItem(sCurrentItemName))
						mpInit->mpInventory->GetCurrentSlot()->SetItem(mpInit->mpInventory->GetCurrentItem());
				} else {
					mpInit->mpInventory->GetCurrentSlot()->SetItem(mpInit->mpInventory->GetCurrentItem());
				}
			}
			mpInit->mpInventory->SetCurrentItem(NULL);
			mpInit->mpInventory->SetCurrentSlot(NULL);

			mpInit->mpInventory->SetDroppedInSlot(true);
		}
	}
}

//-----------------------------------------------------------------------

void cInventorySlot::OnDoubleClick(eMButton aButton) {
	/////////////////////////////////
	// Left mouse
	if (aButton == eMButton_Left) {
		// The player might be holding the item when double clicking.
		cInventoryItem *pItem = mpItem;
		if (pItem == NULL && mpInit->mpInventory->GetCurrentSlot() == this) {
			pItem = mpInit->mpInventory->GetCurrentItem();
			if (pItem) {
				mpInit->mpInventory->GetCurrentSlot()->SetItem(mpInit->mpInventory->GetCurrentItem());
				mpInit->mpInventory->SetCurrentItem(NULL);
				mpInit->mpInventory->SetCurrentSlot(NULL);
			}
		}

		if (pItem) {
			cGameItemType *pItemType = mpInit->mpInventory->GetItemType(pItem->GetItemType());
			if (pItemType == NULL) {
				Warning("Itemtype %d does not exist\n", pItem->GetItemType());
				return;
			}

			// Check there is a callback for the item with object = ""
			if (mpInit->mpInventory->CheckUseCallback(pItem->GetName(), "")) {
				mpInit->mpInventory->SetActive(false);
			}

			// Use the action of the item
			else if (pItemType->OnAction(pItem, 0) == false) {
				mpInit->mpInventory->SetActive(false);
			}
		}
	}
}

//-----------------------------------------------------------------------

void cInventorySlot::OnShortcutDown(int alNum) {
	if (mpItem) {
		cInventorySlot *pSlot = mpInit->mpInventory->GetEquipSlot(alNum);

		if (pSlot == this)
			return;

		cInventoryItem *pThisItem = mpItem;

		if (pSlot->mpItem)
			SetItem(pSlot->mpItem);
		else
			SetItem(NULL);

		pSlot->SetItem(pThisItem);
	}
}

//-----------------------------------------------------------------------

void cInventorySlot::OnUpdate(float afTimeStep) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC ITEM METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cInventoryItem::cInventoryItem(cInit *apInit) {
	mpInit = apInit;
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();
	mpGfxObject = NULL;
	mpGfxObjectAdditive = NULL;
}

cInventoryItem::~cInventoryItem() {
	if (mpGfxObject)
		mpDrawer->DestroyGfxObject(mpGfxObject);
	if (mpGfxObjectAdditive)
		mpDrawer->DestroyGfxObject(mpGfxObjectAdditive);
}

//-----------------------------------------------------------------------

bool cInventoryItem::Init(cGameItem *apGameItem) {
	// Init all stuff
	msName = apGameItem->GetName();

	if (apGameItem->GetImageFile() != "") {
		mpGfxObject = mpDrawer->CreateGfxObject(apGameItem->GetImageFile(), "diffalpha2d");
		mpGfxObjectAdditive = mpDrawer->CreateGfxObject(apGameItem->GetImageFile(), "diffadditive2d");
	} else
		mpGfxObject = NULL;

	msDescription = apGameItem->GetDescription();
	msGameName = apGameItem->GetGameName();

	mItemType = apGameItem->GetItemType();

	msSubType = apGameItem->GetSubType();

	msHudModelFile = apGameItem->GetHudModelFile();
	msHudModelName = apGameItem->GetHudModelName();

	mbCanBeDropped = apGameItem->CanBeDropped();
	mbHasCount = apGameItem->HasCount();
	mlCount = apGameItem->GetCount();

	msEntityFile = apGameItem->GetMeshEntity()->GetSourceFile();

	return true;
}

//-----------------------------------------------------------------------

bool cInventoryItem::InitFromFile(const tString &asFile) {
	tString sEntityFile = cString::SetFileExt(asFile, "ent");
	tString sPath = mpInit->mpGame->GetResources()->GetFileSearcher()->GetFilePath(sEntityFile);

	if (sPath != "") {
		TiXmlDocument *pEntityDoc = hplNew(TiXmlDocument, ());
		if (pEntityDoc->LoadFile(sPath.c_str()) == false) {
			Error("Couldn't load '%s'!\n", sPath.c_str());
			return false;
		} else {
			TiXmlElement *pRootElem = pEntityDoc->FirstChildElement();
			TiXmlElement *pMainElem = pRootElem->FirstChildElement("MAIN");
			TiXmlElement *pGameElem = pRootElem->FirstChildElement("GAME");

			msSubType = cString::ToString(pMainElem->Attribute("Subtype"), "");

			tString sImageFile = cString::ToString(pGameElem->Attribute("ImageFile"), "");
			mbCanBeDropped = cString::ToBool(pGameElem->Attribute("CanBeDropped"), true);
			mbHasCount = cString::ToBool(pGameElem->Attribute("HasCount"), false);
			mlCount = cString::ToInt(pGameElem->Attribute("Count"), 1);

			msHudModelFile = cString::ToString(pGameElem->Attribute("HudModelFile"), "");
			msHudModelName = cString::ToString(pGameElem->Attribute("HudModelName"), "");

			tString sNameCat = cString::ToString(pGameElem->Attribute("NameCat"), "");
			tString sNameEntry = cString::ToString(pGameElem->Attribute("NameEntry"), "");
			tString sDescCat = cString::ToString(pGameElem->Attribute("DescCat"), "");
			tString sDescEntry = cString::ToString(pGameElem->Attribute("DescEntry"), "");

			msGameName = kTranslate(sNameCat, sNameEntry);
			msDescription = kTranslate(sDescCat, sDescEntry);

			mItemType = cEntityLoader_GameItem::ToItemType(pGameElem->Attribute("ItemType"));

			if (sImageFile != "") {
				mpGfxObject = mpDrawer->CreateGfxObject(sImageFile, "diffalpha2d");
				mpGfxObjectAdditive = mpDrawer->CreateGfxObject(sImageFile, "diffadditive2d");
			} else
				mpGfxObject = NULL;

			msEntityFile = sEntityFile;
		}
		hplDelete(pEntityDoc);
	} else {
		Error("Entity file '%s' was not found!\n", asFile.c_str());
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------

void cInventoryItem::Drop() {
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
	cCamera3D *pCamera = mpInit->mpPlayer->GetCamera();

	cMatrixf mtxItem = cMatrixf::Identity;
	mtxItem.SetTranslation(pCamera->GetPosition());

	iEntity3D *pEntity = pWorld->CreateEntity(msName, mtxItem, msEntityFile, true);
	if (pEntity) {
		cMeshEntity *pMesh = static_cast<cMeshEntity *>(pEntity);

		if (pMesh->GetBody()) {
			pMesh->GetBody()->AddImpulse(pCamera->GetForward() * 3.2f);
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// INVENTORY BATTERY
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cInventoryBattery::cInventoryBattery(cInit *apInit, const cRect2f &aRect, cGfxObject *apGfxObject,
									 float afZ)
	: iInventoryWidget(apInit, aRect, apGfxObject, afZ) {
	mpGfxObject = mpDrawer->CreateGfxObject("inventory_slot_double.bmp", "diffalpha2d");
	mpGfxBatteryMeter = mpDrawer->CreateGfxObject("inventory_battery_meter.bmp", "diffalpha2d");
	mpGfxBatteryMeterBar = mpDrawer->CreateGfxObject("inventory_battery_meter_bar.bmp", "diffalpha2d");
}

cInventoryBattery::~cInventoryBattery() {
	mpDrawer->DestroyGfxObject(mpGfxBatteryMeter);
	mpDrawer->DestroyGfxObject(mpGfxBatteryMeterBar);
}

void cInventoryBattery::OnDraw() {
	cVector3f vPos = cVector3f(mRect.x, mRect.y, mfZ + 1);
	cVector2f vSize(mRect.w, mRect.h);
	float fAlpha = mpInit->mpInventory->GetAlpha();
	float fPercent = mpInit->mpPlayer->GetPower() / 100.0f;

	mpDrawer->DrawGfxObject(mpGfxBatteryMeter, vPos, vSize, cColor(1, fAlpha));

	mpDrawer->DrawGfxObject(mpGfxBatteryMeterBar, cVector3f(vPos.x + 26, vPos.y + 16, 4),
							cVector2f(21, 102),
							cColor(0, 0, 0, fAlpha));
	mpDrawer->DrawGfxObject(mpGfxBatteryMeterBar,
							cVector3f(vPos.x + 26, vPos.y + 16 + (102 * (1 - fPercent)), 5),
							cVector2f(21, 102 * fPercent),
							cColor(1.0f - fPercent, fPercent, 0, fAlpha));

	// mpFont->Draw(cVector3f(vBatteryPos.x+50, vBatteryPos.y+6,10),20,cColor(0.91f,1,0.91f,mfAlpha),eFontAlign_Left,
	//_W("%.0f%%"),mpInit->mpPlayer->GetPower());
}

void cInventoryBattery::OnMouseOver() {
	char sPercent[256];
	snprintf(sPercent, 256, " %.0f%% ", mpInit->mpPlayer->GetPower());
	mpInit->mpInventory->SetItemName(kTranslate("Inventory", "BatteryLevel"));
	mpInit->mpInventory->SetItemDesc(kTranslate("Inventory", "BatteryLevelDesc1") +
									 cString::To16Char(sPercent) +
									 kTranslate("Inventory", "BatteryLevelDesc2"));
}

void cInventoryBattery::OnUpdate(float afTimeStep) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// INVENTORY HEALTH
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cInventoryHealth::cInventoryHealth(cInit *apInit, const cRect2f &aRect, cGfxObject *apGfxObject,
								   float afZ)
	: iInventoryWidget(apInit, aRect, apGfxObject, afZ) {
	mpGfxObject = mpDrawer->CreateGfxObject("inventory_slot_double.bmp", "diffalpha2d");
	mpGfxFine = mpDrawer->CreateGfxObject("inventory_health_fine.bmp", "diffalpha2d");
	mpGfxCaution = mpDrawer->CreateGfxObject("inventory_health_caution.bmp", "diffalpha2d");
	mpGfxDanger = mpDrawer->CreateGfxObject("inventory_health_danger.bmp", "diffalpha2d");
}

cInventoryHealth::~cInventoryHealth() {
	mpDrawer->DestroyGfxObject(mpGfxFine);
	mpDrawer->DestroyGfxObject(mpGfxCaution);
	mpDrawer->DestroyGfxObject(mpGfxDanger);
}

void cInventoryHealth::OnDraw() {
	cVector3f vPos = cVector3f(mRect.x, mRect.y, mfZ + 1);
	cVector2f vSize(mRect.w, mRect.h);
	float fAlpha = mpInit->mpInventory->GetAlpha();
	float fPercent = mpInit->mpPlayer->GetHealth() / 100.0f;

	cGfxObject *pGfxMan;
	if (fPercent > 0.75f) {
		pGfxMan = mpGfxFine;
	} else if (fPercent > 0.3f) {
		pGfxMan = mpGfxCaution;
	} else {
		pGfxMan = mpGfxDanger;
	}

	mpDrawer->DrawGfxObject(pGfxMan, cVector3f(vPos.x + 6, vPos.y + 6, 2), cVector2f(mRect.w - 12, mRect.h - 12),
							cColor(1.0f - fPercent, fPercent, 0, fAlpha));
}

void cInventoryHealth::OnMouseOver() {
	float fPercent = mpInit->mpPlayer->GetHealth() / 100.0f;
	tWString wsText;

	if (fPercent > 0.75f) {
		wsText = kTranslate("Inventory", "HealthFine");
	} else if (fPercent > 0.3f) {
		wsText = kTranslate("Inventory", "HealthCaution");
	} else {
		wsText = kTranslate("Inventory", "HealthDanger");
	}

	mpInit->mpInventory->SetItemName(kTranslate("Inventory", "Health"));
	mpInit->mpInventory->SetItemDesc(wsText);
}

void cInventoryHealth::OnUpdate(float afTimeStep) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// INVENTORY CONTEXT METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cInventoryContext::cInventoryContext(cInit *apInit) {
	mpInit = apInit;
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();

	mbActive = false;

	mpGfxBack = mpDrawer->CreateGfxObject("inventory_context_back.bmp", "diffalpha2d");
	mpGfxCorner11 = mpDrawer->CreateGfxObject("inventory_context_11.bmp", "diffalpha2d");
	mpGfxCorner12 = mpDrawer->CreateGfxObject("inventory_context_12.bmp", "diffalpha2d");
	mpGfxCorner21 = mpDrawer->CreateGfxObject("inventory_context_21.bmp", "diffalpha2d");
	mpGfxCorner22 = mpDrawer->CreateGfxObject("inventory_context_22.bmp", "diffalpha2d");
	mpGfxRight = mpDrawer->CreateGfxObject("inventory_context_right.bmp", "diffalpha2d");
	mpGfxLeft = mpDrawer->CreateGfxObject("inventory_context_left.bmp", "diffalpha2d");
	mpGfxTop = mpDrawer->CreateGfxObject("inventory_context_top.bmp", "diffalpha2d");
	mpGfxBottom = mpDrawer->CreateGfxObject("inventory_context_bottom.bmp", "diffalpha2d");

	mvPos.z = 40.0f;

	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("verdana.fnt");

	mfRowStart = 0;
	mfRowSize = 15;
	mfColLength = 100;

	mlSelectedRow = -1;

	mfAlpha = 0.0f;

	mpActionVec = NULL;
}

//-----------------------------------------------------------------------

cInventoryContext::~cInventoryContext() {
}

//-----------------------------------------------------------------------

void cInventoryContext::SetActive(bool abX) {
	mbActive = abX;

	if (mbActive) {

	} else {
	}
}

//-----------------------------------------------------------------------

void cInventoryContext::Draw() {
	if (mfAlpha <= 0.0f || mpActionVec == NULL)
		return;

	float fTotalAlpha = mfAlpha * mpInit->mpInventory->GetAlpha();

	mpDrawer->DrawGfxObject(mpGfxBack, mvPos, mvSize, cColor(1, fTotalAlpha));

	// Corners
	cVector2f vCornerSize(3, 3);
	mpDrawer->DrawGfxObject(mpGfxCorner11, mvPos - vCornerSize, vCornerSize, cColor(1, fTotalAlpha));
	mpDrawer->DrawGfxObject(mpGfxCorner21, mvPos + cVector3f(mfColLength, -vCornerSize.y, 0),
							vCornerSize, cColor(1, fTotalAlpha));
	mpDrawer->DrawGfxObject(mpGfxCorner12, mvPos + cVector3f(-vCornerSize.x, mvSize.y, 0),
							vCornerSize, cColor(1, fTotalAlpha));
	mpDrawer->DrawGfxObject(mpGfxCorner22, mvPos + cVector3f(mvSize.x, mvSize.y, 0),
							vCornerSize, cColor(1, fTotalAlpha));

	// Sides
	mpDrawer->DrawGfxObject(mpGfxLeft, mvPos + cVector3f(-vCornerSize.x, 0, 0),
							cVector2f(vCornerSize.x, mvSize.y), cColor(1, fTotalAlpha));
	mpDrawer->DrawGfxObject(mpGfxRight, mvPos + cVector3f(mvSize.x, 0, 0),
							cVector2f(vCornerSize.x, mvSize.y), cColor(1, fTotalAlpha));
	mpDrawer->DrawGfxObject(mpGfxTop, mvPos + cVector3f(0, -vCornerSize.y, 0),
							cVector2f(mvSize.x, vCornerSize.y), cColor(1, fTotalAlpha));
	mpDrawer->DrawGfxObject(mpGfxTop, mvPos + cVector3f(0, mvSize.y, 0),
							cVector2f(mvSize.x, vCornerSize.y), cColor(1, fTotalAlpha));

	for (int i = 0; i < (int)mpActionVec->size(); i++) {
		/*cColor Col = */ mlSelectedRow == i ? cColor(0.2f, 1.0f, 0.2f, mfAlpha) : cColor(0.65f, 0.65f, 0.65f, fTotalAlpha);

		// mpFont->Draw(mvPos + cVector3f(2.0f,(float)i*mfRowSize,1),14,Col,
		//			eFontAlign_Left,(*mpActionVec)[i].c_str());
	}
}

//-----------------------------------------------------------------------

void cInventoryContext::Update(float afTimeStep) {
	////////////////////////////////
	// Change alpha
	if (mbActive == false) {
		mfAlpha -= 2.5f * afTimeStep;
		if (mfAlpha < 0)
			mfAlpha = 0;

		return;
	} else {
		mfAlpha += 2.3f * afTimeStep;
		if (mfAlpha > 1)
			mfAlpha = 1;
	}

	mpInit->mpInventory->SetItemDesc(mpItem->GetDescription());
	mpInit->mpInventory->SetItemName(mpItem->GetGameName());

	///////////////////////////////////////
	// Check if the mouse is over any row
	cVector2f vMousePos = mpInit->mpInventory->GetMousePos();
	mlSelectedRow = -1;

	for (int i = 0; i < (int)mpActionVec->size(); i++) {
		cRect2f Rect(mvPos.x, mvPos.y + mfRowStart + (float)i * mfRowSize,
					 mfColLength, mfRowSize);
		if (cMath::PointBoxCollision(vMousePos, Rect)) {
			mlSelectedRow = i;
			break;
		}
	}
}

//-----------------------------------------------------------------------

void cInventoryContext::OnMouseDown(eMButton aButton) {
	/////////////////////////////////
	// Left button
	if (aButton == eMButton_Left) {
		if (mlSelectedRow != -1) {
			cGameItemType *pItemType = mpInit->mpInventory->GetItemType(mpItem->GetItemType());

			if (pItemType->OnAction(mpItem, mlSelectedRow) == false) {
				mpInit->mpInventory->SetActive(false);
			}
		}
		mbActive = false;
	}
	/////////////////////////////////
	// Right button
	else if (aButton == eMButton_Right) {
		mbActive = false;
	}
}

//-----------------------------------------------------------------------

void cInventoryContext::OnMouseUp(eMButton aButton) {
}

//-----------------------------------------------------------------------

void cInventoryContext::Setup(cInventoryItem *apItem, const cVector2f &avPos) {
	mpItem = apItem;
	mvPos = avPos;

	cGameItemType *pItemType = mpInit->mpInventory->GetItemType(apItem->GetItemType());
	mpActionVec = pItemType->GetActions(apItem->CanBeDropped());

	mvSize = cVector2f(mfColLength, (float)mpActionVec->size() * mfRowSize + 8);

	mRect = cRect2f(mvPos.x, mvPos.y, mvSize.x, mvSize.y);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cInventory::OnStart() {
}

//-----------------------------------------------------------------------

void cInventory::Update(float afTimeStep) {
	////////////////////////////////
	// Change alpha
	if (mbActive == false) {
		mfAlpha -= 2.5f * afTimeStep;
		if (mfAlpha < 0)
			mfAlpha = 0;

		mfMessageAlpha -= 3.1f * afTimeStep;
		if (mfMessageAlpha < 0)
			mfMessageAlpha = 0;

		return;
	} else {
		mfAlpha += 2.3f * afTimeStep;
		if (mfAlpha > 1)
			mfAlpha = 1;
	}

	///////////////////////////////
	// Iterate widgets
	if (mpContext->IsActive() == false) {
		mbDrawText = false;

		tInventoryWidgetListIt it = mlstWidgets.begin();
		for (; it != mlstWidgets.end(); ++it) {
			iInventoryWidget *pWidget = *it;

			pWidget->OnUpdate(afTimeStep);

			if (cMath::PointBoxCollision(mvMousePos, pWidget->GetRect())) {
				pWidget->OnMouseOver();
			}
		}
	}
	/////////////////////
	// Context Menu
	mpContext->Update(afTimeStep);

	/////////////////////
	// Inventory message
	if (mbMessageActive) {
		mfMessageAlpha += 2.7f * afTimeStep;
		if (mfMessageAlpha > 1)
			mfMessageAlpha = 1;
	} else {
		mfMessageAlpha -= 3.1f * afTimeStep;
		if (mfMessageAlpha < 0)
			mfMessageAlpha = 0;
	}

	/////////////////////
	// Text alpha
	if (mbDrawText) {
		mfTextAlpha += 2.8f * afTimeStep;
		if (mfTextAlpha > 1.0f)
			mfTextAlpha = 1.0f;
	} else {
		mfTextAlpha -= 3.1f * afTimeStep;
		if (mfTextAlpha < 0.0f)
			mfTextAlpha = 0.0f;
	}
}

//-----------------------------------------------------------------------

void cInventory::Reset() {
	// reset variables
	mLastCrossHairState = eCrossHairState_Active;

	mbActive = false;

	mfAlpha = 0.0f;
	mfTextAlpha = 0.0f;

	msItemName = _W("");
	msItemDesc = _W("");

	mpCurrentItem = NULL;
	mpCurrentSlot = NULL;

	mbCheckingCombineItems = false;

	mbMessageActive = false;

	mbNoteBookIsActive = false;

	// Remove all callbacks
	ClearCallbacks();

	// Clear all the slots
	// Normal
	tInventorySlotListIt it = mlstSlots.begin();
	for (; it != mlstSlots.end(); ++it) {
		cInventorySlot *pSlot = *it;
		pSlot->SetItem(NULL);
	}
	// Equip
	for (size_t i = 0; i < mvEquipSlots.size(); ++i) {
		mvEquipSlots[i]->SetItem(NULL);
	}

	STLMapDeleteAll(m_mapItems);
}

//-----------------------------------------------------------------------

void cInventory::OnDraw() {
	if (mfAlpha <= 0.0f)
		return;

	///////////////////////////////
	// Draw background
	mpDrawer->DrawGfxObject(mpGfxBackground, cVector3f(0, 0, 0), cVector2f(800, 600), cColor(1, mfAlpha));

	// Draw backpack and hands
	// mpDrawer->DrawGfxObject(mpBagpack,cVector3f(0,600 - (160*sqrt(mfAlpha)),-1),cVector2f(800,160),cColor(1,mfAlpha));

	///////////////////////////////
	// Draw power remaining:

	///////////////////////////////
	// Draw health
	/*fPercent = mpInit->mpPlayer->GetHealth()/100.0f;
	cVector2f vManPos(640,120);
	mpDrawer->DrawGfxObject(mpHealthFrame,cVector3f(vManPos.x,vManPos.y,2),cVector2f(150,266),cColor(1,mfAlpha));
	mpDrawer->DrawGfxObject(mpHealthBack,cVector3f(vManPos.x,vManPos.y,2),cVector2f(150,266),
							cColor((1-fPercent)*0.7f + 0.3f,fPercent*0.7f + 0.3f, 0.3f, mfAlpha));

	cGfxObject *pGfxMan;
	tWString wsText;
	if(fPercent > 0.75f){		pGfxMan = mpHealthMan_Fine;
								wsText = kTranslate("Inventory","HealthFine");
	}
	else if(fPercent > 0.3f){	pGfxMan = mpHealthMan_Caution;
								wsText = kTranslate("Inventory","HealthCaution");
	}
	else{						pGfxMan = mpHealthMan_Danger;
								wsText = kTranslate("Inventory","HealthDanger");
	}

	mpDrawer->DrawGfxObject(pGfxMan,cVector3f(vManPos.x,vManPos.y,2),cVector2f(150,266),cColor(1,mfAlpha));

	mpDrawer->DrawGfxObject(mpHealthTextFrame,cVector3f(vManPos.x,vManPos.y+266+10,2),cVector2f(150,66),cColor(1,mfAlpha));
	mpFont->Draw(cVector3f(vManPos.x+75,vManPos.y+266+10+4,10),20,cColor(0.91f,1,0.91f,mfAlpha),eFontAlign_Center,
					wsText.c_str());*/

	///////////////////////////////
	// Draw Widgets
	tInventoryWidgetListIt it = mlstWidgets.begin();
	for (; it != mlstWidgets.end(); ++it) {
		iInventoryWidget *pWidget = *it;

		pWidget->Draw();
		pWidget->OnDraw();
	}

	//////////////////////////////
	// Draw Selected item
	if (mpCurrentItem) {
		cVector3f vPos(mvMousePos.x + mvCurrentItemOffset.x,
					   mvMousePos.y + mvCurrentItemOffset.y, 15);
		cVector2l vSize = mpCurrentItem->GetGfxObject()->GetMaterial()->GetImage(eMaterialTexture_Diffuse)->GetSize();

		mpDrawer->DrawGfxObject(mpCurrentItem->GetGfxObject(), vPos,
								cVector2f((float)vSize.x, (float)vSize.y),
								cColor(1, mpInit->mpInventory->GetAlpha()));
	}

	//////////////////////////
	// Draw context
	mpContext->Draw();

	//////////////////////////
	// Draw message
	if (mfMessageAlpha > 0) {
		float fMessAlpha = mfAlpha * mfMessageAlpha;

		cVector3f vMessPos = cVector3f(40, 275, 110);

		mpDrawer->DrawGfxObject(mpMessageBackground, cVector3f(0, vMessPos.y - 8, vMessPos.z - 2),
								cVector2f(800, 17 * 4 + 8 * 2), cColor(1, 1, 1, fMessAlpha * 0.92f));

		mpFont->drawWordWrap(vMessPos + cVector3f(0, 0, 0), 720, 16, 17, cColor(1, 1, 1, fMessAlpha), eFontAlign_Left,
							 msMessage);
		mpFont->drawWordWrap(vMessPos + cVector3f(0, 1, -1), 720, 16, 17, cColor(0, 0, 0, fMessAlpha), eFontAlign_Left,
							 msMessage);
	}

	//////////////////////////
	// Draw text
	if (mpInit->mpRadioHandler->IsActive() == false || mpInit->mbSubtitles == false) {
		float fTextAlpha = mfAlpha * mfTextAlpha * (1 - mfMessageAlpha);
		mpFont->draw(cVector3f(400, 460, 10), 19, cColor(1, 1, 1, fTextAlpha), eFontAlign_Center,
					 msItemName);
		mpFont->draw(cVector3f(400 + 1, 460 + 1, 9), 19, cColor(0, 0, 0, fTextAlpha), eFontAlign_Center,
					 msItemName);

		mpFont->drawWordWrap(cVector3f(80, 480, 10), 640, 16, 17, cColor(1, 1, 1, fTextAlpha), eFontAlign_Left,
							 msItemDesc);
		mpFont->drawWordWrap(cVector3f(80 + 1, 480 + 1, 9), 640, 16, 17, cColor(0, 0, 0, fTextAlpha), eFontAlign_Left,
							 msItemDesc);
	}
}

//-----------------------------------------------------------------------

void cInventory::SetActive(bool abX) {
	if (mbActive == abX)
		return;
	mbActive = abX;

	if (mbActive) {
		mLastCrossHairState = mpInit->mpPlayer->GetCrossHairState();

		mvMousePos = cVector2f(400, 300);
		mpInit->mpPlayer->SetCrossHairPos(mvMousePos);
		mpInit->mpPlayer->SetCrossHairState(eCrossHairState_Pointer);
	} else {
		mpInit->mpPlayer->SetCrossHairState(mLastCrossHairState);
		mbMessageActive = false;
	}
}

//-----------------------------------------------------------------------

bool cInventory::IsActive() {
	return mbActive;
}

//-----------------------------------------------------------------------

void cInventory::OnInventoryDown() {
	if (mbMessageActive) {
		mbMessageActive = false;
		mpInit->mpPlayer->SetCrossHairState(eCrossHairState_Pointer);
		return;
	}

	SetActive(false);
	mpInit->mpPlayer->SetCrossHairPos(cVector2f(400, 300));
	mvMousePos = cVector2f(400, 300);
}

//-----------------------------------------------------------------------

void cInventory::AddWidget(iInventoryWidget *apWidget) {
	mlstWidgets.push_back(apWidget);
}

//-----------------------------------------------------------------------

void cInventory::AddItem(cGameItem *apGameItem) {
	tWString sMessage = apGameItem->GetGameName(); // kTranslate("Inventory","YouPickedUp") + _W(" ")+apGameItem->GetGameName();
	mpInit->mpEffectHandler->GetSubTitle()->Add(sMessage, 2.0f, true);

	//////////////////////////
	// If the item has a count, check if it exists
	if (apGameItem->HasCount()) {
		cInventoryItem *pFoundItem = NULL;

		tInventoryItemMapIt it = m_mapItems.begin();
		for (; it != m_mapItems.end(); ++it) {
			cInventoryItem *pItem = it->second;
			if (pItem->GetItemType() == apGameItem->GetItemType() &&
				pItem->GetSubType() == apGameItem->GetSubType()) {
				pFoundItem = pItem;
				break;
			}
		}

		if (pFoundItem) {
			pFoundItem->AddCount(apGameItem->GetCount());
			CheckPickupCallback(apGameItem->GetName());
			return;
		}
	}

	//////////////////////////
	// Create item
	cInventoryItem *pItem = hplNew(cInventoryItem, (mpInit));
	pItem->Init(apGameItem);

	cGameItemType *pType = GetItemType(pItem->GetItemType());
	if (pType->OnPickUp(pItem, true) == false) {
		CheckPickupCallback(pItem->GetName());
		hplDelete(pItem);
		return;
	}

	m_mapItems.insert(tInventoryItemMap::value_type(pItem->GetName(), pItem));

	tInventorySlotListIt it = mlstSlots.begin();
	for (; it != mlstSlots.end(); ++it) {
		cInventorySlot *pSlot = *it;

		if (pSlot->GetItem() == NULL) {
			pSlot->SetItem(pItem);
			CheckPickupCallback(pItem->GetName());
			return;
		}
	}

	// TODO: Show message that there is no room for the item.
}

//-----------------------------------------------------------------------

void cInventory::AddItemFromFile(const tString &asName, const tString &asFile, int alSlotIndex) {
	/////////////////////////////////
	// Create Item
	cInventoryItem *pItem = hplNew(cInventoryItem, (mpInit));
	pItem->SetName(asName);
	pItem->InitFromFile(asFile);

	cGameItemType *pType = GetItemType(pItem->GetItemType());
	if (pType->OnPickUp(pItem, false) == false) {
		CheckPickupCallback(pItem->GetName());
		hplDelete(pItem);
		return;
	}

	//////////////////////////
	// If the item has a count, check if it exists
	if (pItem->HasCount()) {
		cInventoryItem *pFoundItem = NULL;

		tInventoryItemMapIt it = m_mapItems.begin();
		for (; it != m_mapItems.end(); ++it) {
			cInventoryItem *pTempItem = it->second;
			if (pTempItem->GetItemType() == pItem->GetItemType() &&
				pItem->GetSubType() == pTempItem->GetSubType()) {
				pFoundItem = pTempItem;
				break;
			}
		}

		if (pFoundItem) {
			pFoundItem->AddCount(pItem->GetCount());
			CheckPickupCallback(pItem->GetName());
			hplDelete(pItem);
			return;
		}
	}

	//////////////////////////
	// Add to inventory
	m_mapItems.insert(tInventoryItemMap::value_type(pItem->GetName(), pItem));

	int lCount = 0;
	// int lCurrentSlot = -1;

	tInventorySlotListIt it = mlstSlots.begin();
	for (; it != mlstSlots.end(); ++it) {
		cInventorySlot *pSlot = *it;

		if ((alSlotIndex < 0 && pSlot->GetItem() == NULL && (mpCurrentItem == NULL || mpCurrentSlot != pSlot)) ||
			(alSlotIndex == lCount)) {
			pSlot->SetItem(pItem);
			CheckPickupCallback(pItem->GetName());
			break;
		}
		lCount++;
	}
}

//-----------------------------------------------------------------------

void cInventory::RemoveItem(cInventoryItem *apItem) {
	tInventorySlotListIt SlotIt = mlstSlots.begin();
	for (; SlotIt != mlstSlots.end(); SlotIt++) {
		cInventorySlot *pSlot = *SlotIt;

		if (pSlot->GetItem() == apItem) {
			pSlot->SetItem(NULL);
		}
	}

	tInventoryItemMapIt it = m_mapItems.begin();
	for (; it != m_mapItems.end(); ++it) {
		if (it->second == apItem) {
			m_mapItems.erase(it);
			break;
		}
	}

	if (mpCurrentItem == apItem) {
		mpCurrentItem = NULL;
	}

	hplDelete(apItem);
}

//-----------------------------------------------------------------------

cInventoryItem *cInventory::GetItem(const tString &asName) {
	tInventoryItemMapIt it = m_mapItems.find(asName);
	if (it != m_mapItems.end()) {
		return it->second;
	}

	return NULL;
}

//-----------------------------------------------------------------------

void cInventory::OnMouseDown(eMButton aButton) {
	if (mbMessageActive) {
		mbMessageActive = false;
		mpInit->mpPlayer->SetCrossHairState(eCrossHairState_Pointer);
		return;
	}

	// To this to remove context temporarly.
	if (aButton == eMButton_Right)
		return;

	/////////////////////////////////
	// Context is active
	if (mpContext->IsActive()) {
		mpContext->OnMouseDown(aButton);
	}
	/////////////////////////////////
	// Normal
	else {
		tInventoryWidgetListIt it = mlstWidgets.begin();
		for (; it != mlstWidgets.end(); ++it) {
			iInventoryWidget *pWidget = *it;

			if (cMath::PointBoxCollision(mvMousePos, pWidget->GetRect())) {
				pWidget->OnMouseDown(aButton);
			}
		}
	}
}

void cInventory::OnMouseUp(eMButton aButton) {
	/////////////////////////////////
	// Context is active
	if (mpContext->IsActive()) {
		mpContext->OnMouseUp(aButton);
	}
	/////////////////////////////////
	// Normal
	else {
		mbDroppedInSlot = false;

		tInventoryWidgetListIt it = mlstWidgets.begin();
		for (; it != mlstWidgets.end(); ++it) {
			iInventoryWidget *pWidget = *it;

			if (cMath::PointBoxCollision(mvMousePos, pWidget->GetRect())) {
				pWidget->OnMouseUp(aButton);
			}
		}

		/////////////////////////////////////////
		// Item is dropped outside of slots or returned to its previous slot.
		if (mpCurrentItem && mbDroppedInSlot == false && aButton == eMButton_Left) {
			if (mpCurrentItem->CanBeDropped() && mpCurrentItem->HasCount() == false) {
				mpCurrentItem->Drop();
				RemoveItem(mpCurrentItem);
			} else {
				mpInit->mpInventory->GetCurrentSlot()->SetItem(mpInit->mpInventory->GetCurrentItem());
			}

			mpCurrentItem = NULL;
			mpCurrentSlot = NULL;
		}
	}
}

//-----------------------------------------------------------------------

void cInventory::OnDoubleClick(eMButton aButton) {
	if (mbMessageActive)
		return;

	if (mpContext->IsActive() == false) {
		tInventoryWidgetListIt it = mlstWidgets.begin();
		for (; it != mlstWidgets.end(); ++it) {
			iInventoryWidget *pWidget = *it;

			if (cMath::PointBoxCollision(mvMousePos, pWidget->GetRect())) {
				pWidget->OnDoubleClick(aButton);
			}
		}
	}
}

//-----------------------------------------------------------------------

void cInventory::AddMousePos(const cVector2f &avRel) {
	if (mbMessageActive)
		return;

	mvMousePos += avRel;

	if (mvMousePos.x < 0)
		mvMousePos.x = 0;
	if (mvMousePos.x >= 800)
		mvMousePos.x = 800;
	if (mvMousePos.y < 0)
		mvMousePos.y = 0;
	if (mvMousePos.y >= 600)
		mvMousePos.y = 600;

	mpInit->mpPlayer->SetCrossHairPos(mvMousePos);
}

void cInventory::SetMousePos(const cVector2f &avPos) {
	if (mbMessageActive)
		return;

	mvMousePos = avPos;

	mpInit->mpPlayer->SetCrossHairPos(mvMousePos);
}

//-----------------------------------------------------------------------

void cInventory::OnShortcutDown(int alNum) {
	if (mbMessageActive)
		return;

	if (mbActive) {
		tInventoryWidgetListIt it = mlstWidgets.begin();
		for (; it != mlstWidgets.end(); ++it) {
			iInventoryWidget *pWidget = *it;

			if (cMath::PointBoxCollision(mvMousePos, pWidget->GetRect())) {
				pWidget->OnShortcutDown(alNum);
			}
		}
	} else {
		cInventoryItem *pItem = mvEquipSlots[alNum]->GetItem();
		if (pItem != NULL) {
			// Check there is a callback for the item with object = ""
			if (mpInit->mpInventory->CheckUseCallback(pItem->GetName(), "")) {
				return;
			}

			cGameItemType *pType = mpInit->mpInventory->GetItemType(mvEquipSlots[alNum]->GetItem()->GetItemType());

			pType->OnAction(mvEquipSlots[alNum]->GetItem(), 0);
		}
	}
}

//-----------------------------------------------------------------------

void cInventory::SetMessage(const tWString &asMessage) {
	mbMessageActive = true;
	msMessage = asMessage;

	mpInit->mpPlayer->SetCrossHairState(eCrossHairState_None);
}

//-----------------------------------------------------------------------

void cInventory::AddPickupCallback(const tString &asItem, const tString &asFunction) {
	// Log("Adding callback %s %s\n",asItem.c_str(),asFunction.c_str());

	cInventoryPickupCallback *pCallback = hplNew(cInventoryPickupCallback, ());
	pCallback->msFunction = asFunction;
	pCallback->msItem = asItem;

	m_mapPickupCallbacks.insert(tInventoryPickupCallbackMap::value_type(asItem, pCallback));
}

//-----------------------------------------------------------------------

void cInventory::AddUseCallback(const tString &asItem, const tString &asObject, const tString &asFunction) {
	// Check if the item - object combo already exist.
	tInventoryUseCallbackMapIt it = m_mapUseCallbacks.find(asItem);
	if (it != m_mapUseCallbacks.end()) {
		int lCount = (int)m_mapUseCallbacks.count(asItem);
		for (int i = 0; i < lCount; ++i) {
			cInventoryUseCallback *pCallback = it->second;
			if (pCallback->msObject == asObject) {
				pCallback->msFunction = asFunction;
				return;
			}
		}
	}

	// Add new.
	cInventoryUseCallback *pCallback = hplNew(cInventoryUseCallback, ());
	pCallback->msFunction = asFunction;
	pCallback->msItem = asItem;
	pCallback->msObject = asObject;

	m_mapUseCallbacks.insert(tInventoryUseCallbackMap::value_type(asItem, pCallback));
}

//-----------------------------------------------------------------------

void cInventory::AddCombineCallback(const tString &asItem1, const tString &asItem2, const tString &asFunction) {
	cInventoryCombineCallback *pCallback = hplNew(cInventoryCombineCallback, ());
	pCallback->msFunction = asFunction;
	pCallback->msItem1 = asItem1;
	pCallback->msItem2 = asItem2;
	pCallback->bKillMe = false;

	mlstCombineCallbacks.push_back(pCallback);
}

//-----------------------------------------------------------------------

template<typename Map>
void removeCallbacks(Map &callbackMap, const tString &fn) {
	auto newEnd = Hpl1::removeIf(callbackMap.begin(), callbackMap.end(), [&](typename Map::value_type &p) -> bool {
		if (p.second->msFunction == fn) {
			hplDelete(p.second);
			return true;
		}
		return false;
	});
	callbackMap.erase(newEnd, callbackMap.end());
}

void cInventory::RemovePickupCallback(const tString &asFunction) {
	removeCallbacks(m_mapPickupCallbacks, asFunction);
}

void cInventory::RemoveUseCallback(const tString &asFunction) {
	removeCallbacks(m_mapUseCallbacks, asFunction);
}

void cInventory::RemoveCombineCallback(const tString &asFunction) {
	tInventoryCombineCallbackListIt it = mlstCombineCallbacks.begin();
	for (; it != mlstCombineCallbacks.end(); ++it) {
		cInventoryCombineCallback *pCallback = *it;

		if (pCallback->msFunction == asFunction) {
			if (mbCheckingCombineItems) {
				pCallback->bKillMe = true;
			} else {
				mlstCombineCallbacks.erase(it);
				hplDelete(pCallback);
			}
			return;
		}
	}
}

//-----------------------------------------------------------------------

void cInventory::CheckPickupCallback(const tString &asItem) {
	tInventoryPickupCallbackMapIt it = m_mapPickupCallbacks.find(asItem);
	if (it == m_mapPickupCallbacks.end())
		return;
	cInventoryPickupCallback *pCallback = it->second;

	tString sCommand = pCallback->msFunction + "(\"" + asItem + "\")";
	mpInit->RunScriptCommand(sCommand);
}

//-----------------------------------------------------------------------

bool cInventory::CheckUseCallback(const tString &asItem, const tString &asObject) {
	tInventoryUseCallbackMapIt it = m_mapUseCallbacks.find(asItem);
	if (it == m_mapUseCallbacks.end())
		return false;

	int lItemCount = (int)m_mapUseCallbacks.count(asItem);
	tString sCommand = "";

	for (int i = 0; i < lItemCount; ++i) {
		cInventoryUseCallback *pCallback = it->second;
		if (pCallback->msObject == asObject) {
			sCommand = pCallback->msFunction + "(\"" + asItem + "\", \"" + asObject + "\")";
			break;
		}
		++it;
	}

	if (sCommand != "") {
		mpInit->RunScriptCommand(sCommand);
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------

bool cInventory::CheckCombineCallback(const tString &asItem1, const tString &asItem2, int alSlotIndex) {
	cInventoryCombineCallback *pFinalCallback = NULL;

	mbCheckingCombineItems = true;

	tInventoryCombineCallbackListIt it = mlstCombineCallbacks.begin();
	for (; it != mlstCombineCallbacks.end(); ++it) {
		cInventoryCombineCallback *pCallback = *it;

		if ((pCallback->msItem1 == asItem1 && pCallback->msItem2 == asItem2) ||
			(pCallback->msItem1 == asItem2 && pCallback->msItem2 == asItem1)) {
			pFinalCallback = pCallback;
			break;
		}
	}

	mbCheckingCombineItems = false;

	if (pFinalCallback) {
		char sString[30];
		snprintf(sString, 30, "%d", alSlotIndex);

		tString sCommand = pFinalCallback->msFunction + "(\"" + asItem1 + "\", \"" + asItem2 + "\", " +
						   cString::ToString(sString, "") + ")";
		mpInit->RunScriptCommand(sCommand);
		return true;
	} else {
		// SetActive(false);
		tString sEntry = "CannotCombineItems" + cString::ToString(cMath::RandRectl(1, 5));
		// mpInit->mpGameMessageHandler->Add(kTranslate("Inventory", sEntry));
		SetMessage(kTranslate("Inventory", sEntry));
	}

	// Check if any callbacks should be killed
	for (; it != mlstCombineCallbacks.end();) {
		cInventoryCombineCallback *pCallback = *it;
		if (pCallback->bKillMe) {
			it = mlstCombineCallbacks.erase(it);
			hplDelete(pCallback);
		} else {
			++it;
		}
	}

	return false;
}

//-----------------------------------------------------------------------

void cInventory::ClearCallbacks() {
	STLMapDeleteAll(m_mapPickupCallbacks);
	STLMapDeleteAll(m_mapUseCallbacks);
	STLDeleteAll(mlstCombineCallbacks);
}

//-----------------------------------------------------------------------

void cInventory::SaveToGlobal(cInventory_GlobalSave *apSave) {
	apSave->mbNoteBookActive = mbNoteBookIsActive;

	////////////////////////////
	// Items
	tInventoryItemMapIt ItemIt = m_mapItems.begin();
	for (; ItemIt != m_mapItems.end(); ++ItemIt) {
		cInventoryItem *pItem = ItemIt->second;
		cInventoryItem_GlobalSave saveItem;

		saveItem.msName = pItem->msName;
		saveItem.msGameName = pItem->msGameName;
		saveItem.msDescription = pItem->msDescription;

		saveItem.mItemType = pItem->mItemType;

		saveItem.msSubType = pItem->msSubType;

		saveItem.msEntityFile = pItem->msEntityFile;
		saveItem.msGfxObjectFile = pItem->mpGfxObject->GetSourceFile();

		saveItem.msHudModelFile = pItem->msHudModelFile;
		saveItem.msHudModelName = pItem->msHudModelName;

		saveItem.mbCanBeDropped = pItem->mbCanBeDropped;
		saveItem.mbHasCount = pItem->mbHasCount;
		saveItem.mlCount = pItem->mlCount;

		apSave->mlstItems.Add(saveItem);
	}

	////////////////////////////
	// Equip slots
	apSave->mvEquipSlots.Resize(mvEquipSlots.size());
	for (size_t i = 0; i < mvEquipSlots.size(); i++) {
		if (mvEquipSlots[i]->mpItem)
			apSave->mvEquipSlots[i].msItemName = mvEquipSlots[i]->mpItem->GetName();
		else
			apSave->mvEquipSlots[i].msItemName = "";
	}

	////////////////////////////
	// Normal slots
	tInventorySlotListIt SlotIt = mlstSlots.begin();
	for (; SlotIt != mlstSlots.end(); ++SlotIt) {
		cInventorySlot *pSlot = *SlotIt;
		cInventorySlot_GlobalSave saveSlot;

		if (pSlot->mpItem)
			saveSlot.msItemName = pSlot->mpItem->GetName();
		else
			saveSlot.msItemName = "";

		apSave->mlstSlots.Add(saveSlot);
	}
}

//-----------------------------------------------------------------------

void cInventory::LoadFromGlobal(cInventory_GlobalSave *apSave) {
	mbNoteBookIsActive = apSave->mbNoteBookActive;

	////////////////////////////
	// Items
	cContainerListIterator<cInventoryItem_GlobalSave> ItemIt = apSave->mlstItems.GetIterator();
	while (ItemIt.HasNext()) {
		cInventoryItem *pItem = hplNew(cInventoryItem, (mpInit));
		cInventoryItem_GlobalSave saveItem = ItemIt.Next();

		pItem->msName = saveItem.msName;
		pItem->msGameName = saveItem.msGameName;
		pItem->msDescription = saveItem.msDescription;

		pItem->mItemType = saveItem.mItemType;

		pItem->msSubType = saveItem.msSubType;

		pItem->msEntityFile = saveItem.msEntityFile;
		pItem->mpGfxObject = mpDrawer->CreateGfxObject(saveItem.msGfxObjectFile, "diffalpha2d");
		pItem->mpGfxObjectAdditive = mpDrawer->CreateGfxObject(saveItem.msGfxObjectFile, "diffadditive2d");

		pItem->msHudModelFile = saveItem.msHudModelFile;
		pItem->msHudModelName = saveItem.msHudModelName;

		pItem->mbCanBeDropped = saveItem.mbCanBeDropped;
		pItem->mbHasCount = saveItem.mbHasCount;
		pItem->mlCount = saveItem.mlCount;

		m_mapItems.insert(tInventoryItemMap::value_type(pItem->msName, pItem));
	}

	////////////////////////////
	// Equip slots
	for (size_t i = 0; i < mvEquipSlots.size(); i++) {
		if (apSave->mvEquipSlots[i].msItemName != "")
			mvEquipSlots[i]->mpItem = GetItem(apSave->mvEquipSlots[i].msItemName);
		else
			mvEquipSlots[i]->mpItem = NULL;
	}

	////////////////////////////
	// Normal slots
	cContainerListIterator<cInventorySlot_GlobalSave> SlotIt = apSave->mlstSlots.GetIterator();
	tInventorySlotListIt SlotListIt = mlstSlots.begin();
	while (SlotIt.HasNext()) {
		cInventorySlot *pSlot = *SlotListIt;
		cInventorySlot_GlobalSave saveSlot = SlotIt.Next();

		if (saveSlot.msItemName != "")
			pSlot->mpItem = GetItem(saveSlot.msItemName);
		else
			pSlot->mpItem = NULL;

		++SlotListIt;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------

kBeginSerializeBase(cInventoryUseCallback)
	kSerializeVar(msItem, eSerializeType_String)
		kSerializeVar(msObject, eSerializeType_String)
			kSerializeVar(msFunction, eSerializeType_String)
				kEndSerialize()

	//-------------------------------------------------------------------

	kBeginSerializeBase(cInventoryPickupCallback)
		kSerializeVar(msItem, eSerializeType_String)
			kSerializeVar(msFunction, eSerializeType_String)
				kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cInventoryCombineCallback)
		kSerializeVar(msItem1, eSerializeType_String)
			kSerializeVar(msItem2, eSerializeType_String)
				kSerializeVar(msFunction, eSerializeType_String)
					kSerializeVar(bKillMe, eSerializeType_Bool)
						kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cSaveData_cInventory)
		kSerializeClassContainer(mlstUseCallbacks, cInventoryUseCallback, eSerializeType_Class)
			kSerializeClassContainer(mlstPickupCallbacks, cInventoryPickupCallback, eSerializeType_Class)
				kSerializeClassContainer(mlstCombineCallbacks, cInventoryCombineCallback, eSerializeType_Class)
					kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveObject *cSaveData_cInventory::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	///////////////////////
	// Use callbacks
	{
		cContainerListIterator<cInventoryUseCallback> it = mlstUseCallbacks.GetIterator();
		while (it.HasNext()) {
			cInventoryUseCallback &temp = it.Next();
			gpInit->mpInventory->AddUseCallback(temp.msItem, temp.msObject, temp.msFunction);
		}
	}

	///////////////////////
	// Pickup callbacks
	{
		cContainerListIterator<cInventoryPickupCallback> it = mlstPickupCallbacks.GetIterator();
		while (it.HasNext()) {
			cInventoryPickupCallback &temp = it.Next();
			gpInit->mpInventory->AddPickupCallback(temp.msItem, temp.msFunction);
		}
	}
	///////////////////////
	// Combine callbacks
	{
		cContainerListIterator<cInventoryCombineCallback> it = mlstCombineCallbacks.GetIterator();
		while (it.HasNext()) {
			cInventoryCombineCallback &temp = it.Next();
			gpInit->mpInventory->AddCombineCallback(temp.msItem1, temp.msItem2, temp.msFunction);
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------

int cSaveData_cInventory::GetSaveCreatePrio() {
	return 4;
}

//-----------------------------------------------------------------------

iSaveData *cInventory::CreateSaveData() {
	cSaveData_cInventory *pData = hplNew(cSaveData_cInventory, ());

	//////////////////////
	// Use callbacks
	{
		tInventoryUseCallbackMapIt it = m_mapUseCallbacks.begin();
		for (; it != m_mapUseCallbacks.end(); ++it) {
			pData->mlstUseCallbacks.Add(*(it->second));
		}
	}

	//////////////////////
	// Pickup callbacks
	{
		tInventoryPickupCallbackMapIt it = m_mapPickupCallbacks.begin();
		for (; it != m_mapPickupCallbacks.end(); ++it) {
			pData->mlstPickupCallbacks.Add(*(it->second));
		}
	}

	//////////////////////
	// Combine callbacks
	{
		tInventoryCombineCallbackListIt it = mlstCombineCallbacks.begin();
		for (; it != mlstCombineCallbacks.end(); ++it) {
			pData->mlstCombineCallbacks.Add(*(*it));
		}
	}

	return pData;
}

//-----------------------------------------------------------------------
