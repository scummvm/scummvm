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

#ifndef GAME_INVENTORY_H
#define GAME_INVENTORY_H

#include "hpl1/engine/engine.h"

using namespace hpl;

#include "hpl1/penumbra-overture/GameTypes.h"

class cInit;
class cInventory_GlobalSave;

class cGameItemType;
typedef Common::Array<cGameItemType *> tGameItemTypeVec;
typedef tGameItemTypeVec::iterator tGameItemTypeVecIt;

//--------------------------------------------

class iInventoryWidget {
public:
	iInventoryWidget(cInit *apInit, const cRect2f &aRect, cGfxObject *apGfxObject, float afZ);
	virtual ~iInventoryWidget() {
		if (mpGfxObject)
			mpDrawer->DestroyGfxObject(mpGfxObject);
	}

	virtual void Draw();

	virtual void OnDraw() {}
	virtual void OnMouseOver() {}
	virtual void OnMouseDown(eMButton aButton) {}
	virtual void OnMouseUp(eMButton aButton) {}
	virtual void OnDoubleClick(eMButton aButton) {}
	virtual void OnUpdate(float afTimeStep) {}
	virtual void OnShortcutDown(int alNum) {}

	cRect2f &GetRect() { return mRect; }
	cGfxObject *GetGfxObject() { return mpGfxObject; }

protected:
	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;
	cGfxObject *mpGfxObject;
	cRect2f mRect;
	float mfZ;
};

typedef Common::List<iInventoryWidget *> tInventoryWidgetList;
typedef tInventoryWidgetList::iterator tInventoryWidgetListIt;

//-----------------------------------------

class cInventoryItem;

class cInventorySlot : public iInventoryWidget {
	friend class cInventory;

public:
	cInventorySlot(cInit *apInit, const cVector2f &avPos, bool abEquip, int alIndex);

	void OnDraw();
	void OnMouseOver();
	void OnMouseDown(eMButton aButton);
	void OnMouseUp(eMButton aButton);
	void OnDoubleClick(eMButton aButton);
	void OnUpdate(float afTimeStep);
	void OnShortcutDown(int alNum);

	cInventoryItem *GetItem() { return mpItem; }
	void SetItem(cInventoryItem *apItem) { mpItem = apItem; }

	void SetIndex(int alX) { mlIndex = alX; }
	void SetEquip(bool abX) { mbEquip = abX; }

	void SetEquipIndex(int alX) { mlEquipIndex = alX; }
	int GetEquipIndex() { return mlEquipIndex; }

private:
	cInventoryItem *mpItem;
	int mlIndex;
	bool mbEquip;

	int mlEquipIndex;

	cGfxObject *mpGfxBack;

	FontData *mpFont;
};

typedef Common::List<cInventorySlot *> tInventorySlotList;
typedef tInventorySlotList::iterator tInventorySlotListIt;

//-----------------------------------------

class cGameItem;
class cInventoryItem {
	friend class cInventory;

public:
	cInventoryItem(cInit *apInit);
	~cInventoryItem();

	bool Init(cGameItem *apGameItem);
	bool InitFromFile(const tString &asFile);

	void Drop();

	void SetName(const tString &asName) { msName = asName; }
	const tString &GetName() { return msName; }

	void SetSubType(const tString &asSubType) { msSubType = asSubType; }
	const tString &GetSubType() { return msSubType; }

	const tWString &GetGameName() { return msGameName; }
	const tWString &GetDescription() { return msDescription; }

	const tString &GetHudModelFile() { return msHudModelFile; }
	const tString &GetHudModelName() { return msHudModelName; }

	const tString &GetEntityFile() { return msEntityFile; }

	cGfxObject *GetGfxObject() { return mpGfxObject; }
	cGfxObject *GetGfxObjectAdditive() { return mpGfxObjectAdditive; }

	eGameItemType GetItemType() { return mItemType; }

	bool CanBeDropped() { return mbCanBeDropped; }

	bool HasCount() { return mbHasCount; }
	int GetCount() { return mlCount; }
	void AddCount(int alX) { mlCount += alX; }
	void SetCount(int alX) { mlCount = alX; }

private:
	cInit *mpInit;
	tString msName;
	tString msSubType;

	tWString msGameName;
	tWString msDescription;

	eGameItemType mItemType;

	tString msEntityFile;

	tString msHudModelFile;
	tString msHudModelName;

	cGfxObject *mpGfxObject;
	cGfxObject *mpGfxObjectAdditive;
	cGraphicsDrawer *mpDrawer;

	// cInventoryItem *mpItem;

	bool mbCanBeDropped;
	bool mbHasCount;
	int mlCount;
};

typedef Common::MultiMap<tString, cInventoryItem *> tInventoryItemMap;
typedef tInventoryItemMap::iterator tInventoryItemMapIt;

//-----------------------------------------

class cInventoryBattery : public iInventoryWidget {
public:
	cInventoryBattery(cInit *apInit, const cRect2f &aRect, cGfxObject *apGfxObject, float afZ);
	~cInventoryBattery();

	void OnDraw();
	void OnMouseOver();
	void OnUpdate(float afTimeStep);

private:
	cGfxObject *mpGfxBatteryMeter;
	cGfxObject *mpGfxBatteryMeterBar;
};

//-----------------------------------------

class cInventoryHealth : public iInventoryWidget {
public:
	cInventoryHealth(cInit *apInit, const cRect2f &aRect, cGfxObject *apGfxObject, float afZ);
	~cInventoryHealth();

	void OnDraw();
	void OnMouseOver();
	void OnUpdate(float afTimeStep);

private:
	cGfxObject *mpGfxFine;
	cGfxObject *mpGfxCaution;
	cGfxObject *mpGfxDanger;
};

//-----------------------------------------

class cInventoryContext {
public:
	cInventoryContext(cInit *apInit);
	~cInventoryContext();

	void SetActive(bool abX);
	bool IsActive() { return mbActive; }

	void Draw();

	void Update(float afTimeStep);

	void OnMouseDown(eMButton aButton);
	void OnMouseUp(eMButton aButton);

	void Setup(cInventoryItem *apItem, const cVector2f &avPos);

private:
	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;
	FontData *mpFont;

	cGfxObject *mpGfxBack;
	cGfxObject *mpGfxCorner11;
	cGfxObject *mpGfxCorner12;
	cGfxObject *mpGfxCorner21;
	cGfxObject *mpGfxCorner22;
	cGfxObject *mpGfxRight;
	cGfxObject *mpGfxLeft;
	cGfxObject *mpGfxTop;
	cGfxObject *mpGfxBottom;

	bool mbActive;

	float mfAlpha;

	tWStringVec *mpActionVec;

	float mfRowStart;
	float mfRowSize;
	float mfColLength;

	int mlSelectedRow;

	cInventoryItem *mpItem;
	cVector3f mvPos;
	cVector2f mvSize;
	cRect2f mRect;
};

//-----------------------------------------

kSaveData_BaseClass(cInventory) {
	kSaveData_ClassInit(cInventory) public : cContainerList<cStartPosEntity> mlstStartpos;

	cContainerList<cInventoryUseCallback> mlstUseCallbacks;
	cContainerList<cInventoryPickupCallback> mlstPickupCallbacks;
	cContainerList<cInventoryCombineCallback> mlstCombineCallbacks;

	virtual iSaveObject *CreateSaveObject(cSaveObjectHandler * apSaveObjectHandler, cGame * apGame);
	virtual int GetSaveCreatePrio();
};

//--------------------------------------------

class cInventory : public iUpdateable {
	friend class cSaveHandler;

public:
	cInventory(cInit *apInit);
	~cInventory();

	void OnStart();
	void Update(float afTimeStep);
	void Reset();
	void OnDraw();

	void SetActive(bool abX);
	bool IsActive();

	float GetAlpha() { return mfAlpha; }

	void AddWidget(iInventoryWidget *apWidget);

	void AddItem(cGameItem *apGameItem);
	void AddItemFromFile(const tString &asName, const tString &asFile, int alSlotIndex);

	void RemoveItem(cInventoryItem *apItem);
	cInventoryItem *GetItem(const tString &asName);

	void SetMousePos(const cVector2f &avPos);
	void AddMousePos(const cVector2f &avRel);

	void SetNoteBookActive(bool abX) { mbNoteBookIsActive = abX; }
	bool GetNoteBookActive() { return mbNoteBookIsActive; }

	void OnMouseDown(eMButton aButton);
	void OnMouseUp(eMButton aButton);
	void OnDoubleClick(eMButton aButton);

	void OnInventoryDown();

	void OnShortcutDown(int alNum);

	cVector2f GetMousePos() { return mvMousePos; }
	// void SetMousePos(const cVector2f& avPos){ mvMousePos = avPos;}

	void SetItemName(const tWString &asName) {
		msItemName = asName;
		mbDrawText = true;
	}
	void SetItemDesc(const tWString &asDesc) {
		msItemDesc = asDesc;
		mbDrawText = true;
	}

	cInventoryItem *GetCurrentItem() { return mpCurrentItem; }
	void SetCurrentItem(cInventoryItem *apItem) { mpCurrentItem = apItem; }

	cInventorySlot *GetCurrentSlot() { return mpCurrentSlot; }
	void SetCurrentSlot(cInventorySlot *apSlot) { mpCurrentSlot = apSlot; }

	void SetCurrentItemOffset(const cVector2f &avOffset) { mvCurrentItemOffset = avOffset; }

	cInventorySlot *GetEquipSlot(int alIdx) { return mvEquipSlots[alIdx]; }

	cInventoryContext *GetContext() { return mpContext; }

	cGameItemType *GetItemType(int alIndex) { return mvItemTypes[alIndex]; }

	void SetDroppedInSlot(bool abX) { mbDroppedInSlot = abX; }

	void SetMessage(const tWString &asMessage);

	void AddPickupCallback(const tString &asItem, const tString &asFunction);
	void AddUseCallback(const tString &asItem, const tString &asObject, const tString &asFunction);
	void AddCombineCallback(const tString &asItem1, const tString &asItem2, const tString &asFunction);

	void RemovePickupCallback(const tString &asFunction);
	void RemoveUseCallback(const tString &asFunction);
	void RemoveCombineCallback(const tString &asFunction);

	void ClearCallbacks();

	void CheckPickupCallback(const tString &asItem);
	bool CheckUseCallback(const tString &asItem, const tString &asObject);
	bool CheckCombineCallback(const tString &asItem1, const tString &asItem2, int alSlotIndex);

	// Global save
	void SaveToGlobal(cInventory_GlobalSave *apSave);
	void LoadFromGlobal(cInventory_GlobalSave *apSave);

	// Save
	iSaveData *CreateSaveData();

private:
	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;
	cInventoryContext *mpContext;

	cGfxObject *mpGfxBackground;

	cGfxObject *mpBatteryMeter;
	cGfxObject *mpBatteryMeterBar;

	cGfxObject *mpHealthMan_Fine;
	cGfxObject *mpHealthMan_Caution;
	cGfxObject *mpHealthMan_Danger;
	cGfxObject *mpHealthTextFrame;

	cGfxObject *mpBagpack;

	tGameItemTypeVec mvItemTypes;

	bool mbActive;
	float mfAlpha;
	float mfTextAlpha;
	eCrossHairState mLastCrossHairState;

	tWString msItemName;
	tWString msItemDesc;
	bool mbDrawText;

	bool mbDroppedInSlot;

	bool mbCheckingCombineItems;

	bool mbNoteBookIsActive;

	FontData *mpFont;

	cVector2f mvMousePos;

	bool mbMessageActive;
	tWString msMessage;
	float mfMessageAlpha;
	cGfxObject *mpMessageBackground;

	cInventoryItem *mpCurrentItem;
	cVector2f mvCurrentItemOffset;
	cInventorySlot *mpCurrentSlot;

	tInventoryWidgetList mlstWidgets;
	tInventoryItemMap m_mapItems;
	tInventorySlotList mlstSlots;
	Common::Array<cInventorySlot *> mvEquipSlots;

	tInventoryPickupCallbackMap m_mapPickupCallbacks;
	tInventoryUseCallbackMap m_mapUseCallbacks;
	tInventoryCombineCallbackList mlstCombineCallbacks;
};

#endif // GAME_INVENTORY_H
