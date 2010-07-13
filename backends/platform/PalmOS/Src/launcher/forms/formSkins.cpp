#include <PalmOS.h>

#include "palmdefs.h"
#include "start.h"
#include "forms.h"
#include "skin.h"
#include "globals.h"

static Int16 SkinsFormCompare(SkinInfoType *a, SkinInfoType *b, SortRecordInfoPtr, SortRecordInfoPtr, MemHandle) {
	return StrCompare(a->nameP, b->nameP);
}

static void SkinsFormInit(Boolean bDraw) {
	MemHandle skins = NULL;
	SkinInfoType *skinsInfo;
	UInt16 numSkins = 0;

	FormPtr frmP;
	ListType *listP;
	MemHandle items = NULL;
	ControlType *cck1P;
	DmSearchStateType stateInfo;
	UInt16 cardNo;
	LocalID dbID;

	Err errInfo;
	Char nameP[32];

	itemsText = NULL;

	// parse and save skins
	Err err = DmGetNextDatabaseByTypeCreator(true, &stateInfo, 'skin', appFileCreator, false, &cardNo, &dbID);
	while (!err && dbID) {
		errInfo = DmDatabaseInfo (cardNo, dbID, nameP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		if (!errInfo)
		{
			if (!skins)
				skins = MemHandleNew(sizeof(SkinInfoType));
			else
				MemHandleResize(skins, MemHandleSize(skins) + sizeof(SkinInfoType));

			skinsInfo = (SkinInfoType *)MemHandleLock(skins);
			StrCopy(skinsInfo[numSkins].nameP, nameP);
			skinsInfo[numSkins].cardNo = cardNo;
			skinsInfo[numSkins].dbID = dbID;
			MemHandleUnlock(skins);
			numSkins++;
		}
		err = DmGetNextDatabaseByTypeCreator(false, &stateInfo, 'skin', appFileCreator, false, &cardNo, &dbID);
	}

	Int16 selected = -1;

	cck1P = (ControlType *)GetObjectPtr(SkinsSoundClickCheckbox);
	CtlSetValue(cck1P, gPrefs->soundClick);

	listP = (ListType *)GetObjectPtr(SkinsSkinList);
	skinsInfo = (SkinInfoType *)MemHandleLock(skins);
	SysQSort(skinsInfo, numSkins, sizeof(SkinInfoType), (CmpFuncPtr)SkinsFormCompare, 0);

	// create itemsText (TODO: create a custom draw function)
	for (UInt16 index=0; index < numSkins; index++)
	{
		if (!items)
			items = MemHandleNew(sizeof(Char *));
		else
			MemHandleResize(items, MemHandleSize(items) + sizeof(Char *));

		itemsText = (Char **)MemHandleLock(items);
		itemsText[index] = skinsInfo[index].nameP;
		MemHandleUnlock(items);

		if (	gPrefs->skin.cardNo == skinsInfo[index].cardNo &&
				gPrefs->skin.dbID == skinsInfo[index].dbID &&
				StrCompare(gPrefs->skin.nameP, skinsInfo[index].nameP) == 0)
			selected = index;
	}
	// save globals and set list
	itemsText = (Char **)MemHandleLock(items);
	itemsList = (void *)skinsInfo;
	itemsType = ITEM_TYPE_SKIN;

	LstSetListChoices (listP, itemsText, numSkins);
	LstSetSelection(listP, selected);

	// bDraw = true -> draw whole from
	// bDraw = false -> redraw list
	if (bDraw) {
		frmP = FrmGetActiveForm();
		FrmDrawForm(frmP);
	} else {
		WinScreenLock(winLockCopy);
		LstDrawList(listP);
		WinScreenUnlock();
//		LstSetSelection(listP, 0);
	}
}

static void SkinsFormExit(Boolean bSave) {
	MemHandle skins;
	MemHandle items;
	SkinInfoType *skinsInfo;

	ListType *listP;
	Int16 selected;

	listP = (ListType *)GetObjectPtr(SkinsSkinList);
	selected = LstGetSelection(listP);

	if (bSave && selected == -1)	{	// may never occurred...
		FrmCustomAlert(FrmWarnAlert, "You didn't select a skin.", 0, 0);
		return;
	}

	skinsInfo = (SkinInfoType *)itemsList;
	skins = MemPtrRecoverHandle(skinsInfo);
	items = MemPtrRecoverHandle(itemsText);

	itemsText = NULL;
	itemsList = NULL;
	itemsType = ITEM_TYPE_UNKNOWN;

	if (bSave) {
		ControlType *cck1P;

		StrCopy(gPrefs->skin.nameP, skinsInfo[selected].nameP);
		gPrefs->skin.cardNo = skinsInfo[selected].cardNo;
		gPrefs->skin.dbID =  skinsInfo[selected].dbID;

/*		DmOpenRef skinDB = SknOpenSkin();
		UInt32 depth = SknGetDepth(skinDB);
		SknCloseSkin(skinDB);

		if (depth != 8 && depth != 16) depth = 8;

		if (depth == 16 && !OPTIONS_TST(kOptMode16Bit)) {
			FrmCustomAlert(FrmInfoAlert, "You can only use 8bit skin on your device.", 0, 0);
			gPrefs->skin.cardNo	= cardNo;
			gPrefs->skin.dbID	= dbID;
		}
*/
		cck1P = (ControlType *)GetObjectPtr(SkinsSoundClickCheckbox);
		gPrefs->soundClick = CtlGetValue(cck1P);
	}

	FrmReturnToMain();

	MemHandleUnlock(items);
	MemHandleUnlock(skins);
	MemHandleFree(items);
	MemHandleFree(skins);

	if (bSave)
		SknApplySkin();
}

static void SkinsFormBeam() {
	SkinInfoType *skinsInfo;

	ListType *listP;
	Int16 selected;
	Err err;

	listP = (ListType *)GetObjectPtr(SkinsSkinList);
	selected = LstGetSelection(listP);

	if (selected == -1)	{	// may never occurred...
		FrmCustomAlert(FrmWarnAlert, "You didn't select a skin.", 0, 0);
		return;
	}

	skinsInfo = (SkinInfoType *)itemsList;
	err = SendDatabase(0, skinsInfo[selected].dbID, "skin.pdb", "\nScummVM Skin");

//	if (err)
//		FrmCustomAlert(FrmErrorAlert, "Unable to beam this skin.",0,0);
}

static void SkinsFormDelete() {
	MemHandle skins;
	MemHandle items;
	SkinInfoType *skinsInfo;

	ListType *listP;
	Int16 selected;

	listP = (ListType *)GetObjectPtr(SkinsSkinList);
	selected = LstGetSelection(listP);

	if (selected == -1)	{	// may never occurred...
		FrmCustomAlert(FrmInfoAlert, "You didn't select a skin.", 0, 0);
		return;
	}

	skinsInfo = (SkinInfoType *)itemsList;
	skins = MemPtrRecoverHandle(skinsInfo);
	items = MemPtrRecoverHandle(itemsText);

	if (	gPrefs->skin.cardNo == skinsInfo[selected].cardNo &&
			gPrefs->skin.dbID == skinsInfo[selected].dbID &&
			StrCompare(gPrefs->skin.nameP, skinsInfo[selected].nameP) == 0) {
			FrmCustomAlert(FrmInfoAlert, "You cannot delete the active skin.",0,0);
			return;

	} else {
		Err err = DmDeleteDatabase(0, skinsInfo[selected].dbID);
		if (!err) {

			itemsText = NULL;
			itemsList = NULL;
			itemsType = ITEM_TYPE_UNKNOWN;

			MemHandleUnlock(items);
			MemHandleUnlock(skins);
			MemHandleFree(items);
			MemHandleFree(skins);

			SkinsFormInit(false);
		} else {
			FrmCustomAlert(FrmErrorAlert, "Skin deletion failed.",0,0);
		}
	}

}
Boolean SkinsFormHandleEvent(EventPtr eventP) {
	Boolean handled = false;

	switch (eventP->eType) {

		case frmOpenEvent:
			SkinsFormInit(true);
			handled = true;
			break;

		case frmCloseEvent:
			SkinsFormExit(false);
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case SkinsOKButton:
					SkinsFormExit(true);
					break;

				case SkinsCancelButton:
					SkinsFormExit(false);
					break;

				case SkinsBeamButton:
					SkinsFormBeam();
					break;

				case SkinsDeleteButton:
					SkinsFormDelete();
					break;
			}
			handled = true;
			break;

		default:
			break;
	}

	return handled;
}
