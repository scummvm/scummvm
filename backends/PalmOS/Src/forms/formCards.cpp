#include <PalmOS.h>
#include <VFSMgr.h>

#include "start.h"
#include "forms.h"

typedef struct {
	UInt16 volRefNum;
	Char nameP[expCardInfoStringMaxLen+1];

} CardInfoType;

void CardSlotCreateDirs() {
	if (gPrefs->card.volRefNum != sysInvalidRefNum) {
		VFSDirCreate(gPrefs->card.volRefNum, "/PALM");
		VFSDirCreate(gPrefs->card.volRefNum, "/PALM/Programs");
		VFSDirCreate(gPrefs->card.volRefNum, "/PALM/Programs/ScummVM");
		VFSDirCreate(gPrefs->card.volRefNum, "/PALM/Programs/ScummVM/Games");
		VFSDirCreate(gPrefs->card.volRefNum, "/PALM/Programs/ScummVM/Saved");
	}
}

static void CardSlotFromShowHideOptions() {
	ControlType *cck1P;
	FormPtr frmP = FrmGetActiveForm();

	cck1P = (ControlType *)GetObjectPtr(CardSlotMoveCheckbox);

	if (CtlGetValue(cck1P)) {
		FrmShowObject(frmP, FrmGetObjectIndex (frmP, CardSlotDeleteCheckbox));
		FrmShowObject(frmP, FrmGetObjectIndex (frmP, CardSlotConfirmCheckbox));
	} else {
		FrmHideObject(frmP, FrmGetObjectIndex (frmP, CardSlotDeleteCheckbox));
		FrmHideObject(frmP, FrmGetObjectIndex (frmP, CardSlotConfirmCheckbox));
	}
}

static UInt16 CardSlotFormInit(Boolean display, Boolean bDraw) {
	Err err;
	UInt16 volRefNum;
	UInt32 volIterator = vfsIteratorStart;
	UInt8 counter = 0;
	UInt32 other = 1;

	MemHandle cards = NULL;
	CardInfoType *cardsInfo;

	while (volIterator != vfsIteratorStop) {
		err = VFSVolumeEnumerate(&volRefNum, &volIterator);

		if (!err) {
			Char labelP[expCardInfoStringMaxLen+1];
			err = VFSVolumeGetLabel(volRefNum, labelP, expCardInfoStringMaxLen+1);

			if (err || StrLen(labelP) == 0) {	// if no label try to retreive card type
				VolumeInfoType volInfo;
				err = VFSVolumeInfo(volRefNum, &volInfo);
				
				if (!err) {
					ExpCardInfoType info;
					err = ExpCardInfo(volInfo.slotRefNum, &info);
					StrCopy(labelP, info.deviceClassStr);
				}
				
				if (err)	// if err default name
					StrPrintF(labelP,"Other Card %ld", other++);
			}

			if (!cards)
				cards = MemHandleNew(sizeof(CardInfoType));
			else
				MemHandleResize(cards, MemHandleSize(cards) + sizeof(CardInfoType));
				
			cardsInfo = (CardInfoType *)MemHandleLock(cards);
			cardsInfo[counter].volRefNum = volRefNum;
			StrCopy(cardsInfo[counter].nameP, labelP);
			MemHandleUnlock(cards);
			counter++;
		}
	}

	if (display) {
		FormPtr frmP;
		ListPtr listP;
		ControlType *cck1P, *cck2P, *cck3P;
		UInt16 index;
		Int16 selected = -1;
		
		CardInfoType *cardsInfo;
		MemHandle items = NULL;

		listP = (ListType *)GetObjectPtr(CardSlotSlotList);
		cck1P = (ControlType *)GetObjectPtr(CardSlotMoveCheckbox);
		cck2P = (ControlType *)GetObjectPtr(CardSlotDeleteCheckbox);
		cck3P = (ControlType *)GetObjectPtr(CardSlotConfirmCheckbox);

		if (counter > 0) {
			cardsInfo = (CardInfoType *)MemHandleLock(cards);

			for (index = 0; index < counter; index++) {
				if (!items)
					items = MemHandleNew(sizeof(Char *));
				else
					MemHandleResize(items, MemHandleSize(items) + sizeof(Char *));

				itemsText = (Char **)MemHandleLock(items);
				itemsText[index] = cardsInfo[index].nameP;
				MemHandleUnlock(items);
				
				if (cardsInfo[index].volRefNum == gPrefs->card.volRefNum)
					selected = index;
			}

			itemsText = (Char **)MemHandleLock(items);
			LstSetListChoices (listP, itemsText, counter);
			LstSetSelection(listP, selected);
			// save globals and set list
			itemsText = (Char **)MemHandleLock(items);
			itemsList = (void *)cardsInfo;
			itemsType = ITEM_TYPE_CARD;

		} else {
			LstSetListChoices(listP, NULL, 0);
			// save globals and set list
			itemsText = NULL;
			itemsList = NULL;
			itemsType = ITEM_TYPE_CARD;
		}

		// bDraw = true -> draw whole from
		// bDraw = false -> redraw list
		if (bDraw) {
			CtlSetValue(cck1P, gPrefs->card.moveDB);
			CtlSetValue(cck2P, gPrefs->card.deleteDB);
			CtlSetValue(cck3P, gPrefs->card.confirmMoveDB);
			CardSlotFromShowHideOptions();
			frmP = FrmGetActiveForm();
			FrmDrawForm(frmP);

		} else {
			WinScreenLock(winLockCopy);
			LstDrawList(listP);
			WinScreenUnlock();
		}
	} else {	// if !display, we just want to retreive an avaliable card
		if (counter > 0) {
			UInt16 volRefNum;
			cardsInfo = (CardInfoType *)MemHandleLock(cards);
			volRefNum =  cardsInfo[0].volRefNum;	// return the first volref
			MemHandleUnlock(cards);
			MemHandleFree(cards);
			return volRefNum;
		}
	}

	return sysInvalidRefNum; // default
}

static void CardSlotFormExit(Boolean bSave) {
	MemHandle cards;
	MemHandle items;
	CardInfoType *cardsInfo;
	UInt16 updateCode = frmRedrawUpdateMS;

	if (itemsText && itemsList) {
		cardsInfo = (CardInfoType *)itemsList;
		cards = MemPtrRecoverHandle(cardsInfo);
		items = MemPtrRecoverHandle(itemsText);
	
		itemsText = NULL;
		itemsList = NULL;
	} else {
		cards = NULL;
		items = NULL;
	}
	itemsType = ITEM_TYPE_UNKNOWN;

	if (bSave) {
		ListType *listP;
		ControlType *cck1P, *cck2P, *cck3P;
		Int16 selected;

		listP = (ListType *)GetObjectPtr(CardSlotSlotList);
		cck1P = (ControlType *)GetObjectPtr(CardSlotMoveCheckbox);
		cck2P = (ControlType *)GetObjectPtr(CardSlotDeleteCheckbox);
		cck3P = (ControlType *)GetObjectPtr(CardSlotConfirmCheckbox);
		selected = LstGetSelection(listP);

		if (selected == -1) {
			gPrefs->card.volRefNum = sysInvalidRefNum;
		} else if (gPrefs->card.volRefNum != cardsInfo[selected].volRefNum) {
			updateCode = frmRedrawUpdateMSImport;
			gPrefs->card.volRefNum = cardsInfo[selected].volRefNum;
		}

		gPrefs->card.moveDB = CtlGetValue(cck1P);
		gPrefs->card.deleteDB = CtlGetValue(cck2P);
		gPrefs->card.confirmMoveDB = CtlGetValue(cck3P);
		CardSlotCreateDirs();
	}

	FrmReturnToMain(updateCode);

	if (items && cards) {
		MemHandleUnlock(items);
		MemHandleUnlock(cards);
		MemHandleFree(items);
		MemHandleFree(cards);
	}
}

void CardSlotFormUpdate() {
	if (itemsType == ITEM_TYPE_CARD) {
		if (itemsText && itemsList) {
			MemHandle cards;
			MemHandle items;
			ListType *listP;

			listP = (ListType *)GetObjectPtr(CardSlotSlotList);
			cards = MemPtrRecoverHandle(itemsList);
			items = MemPtrRecoverHandle(itemsText);

			itemsText = NULL;
			itemsList = NULL;
			itemsType = ITEM_TYPE_UNKNOWN;

			MemHandleUnlock(items);
			MemHandleUnlock(cards);
			MemHandleFree(items);
			MemHandleFree(cards);
		}
		CardSlotFormInit(true, false);
	}
}

Boolean CardSlotFormHandleEvent(EventPtr eventP) {
	Boolean handled = false;

	switch (eventP->eType) {
	
		case frmOpenEvent:
			CardSlotFormInit(true, true);
			handled = true;
			break;

		case frmCloseEvent:
			CardSlotFormExit(false);
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case CardSlotOkButton:
					CardSlotFormExit(true);
					break;

				case CardSlotCancelButton:
					CardSlotFormExit(false);
					break;
				
				case CardSlotMoveCheckbox:
					CardSlotFromShowHideOptions();
					break;
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}

UInt16 parseCards() {
	UInt16 volRefNum = CardSlotFormInit(false, false);
	CardSlotFormExit(false);
	
	return volRefNum;
}