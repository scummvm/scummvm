#include <PalmOS.h>

#include "forms.h"

#include "start.h"
#include "games.h"
#include "skin.h"

UInt8 gFormEditMode;

/***********************************************************************
 *
 * FUNCTION:    EditGameFormSave
 * FUNCTION:    EditGameFormInit
 * FUNCTION:    EditGameFormHandleEvent
 *
 * DESCRIPTION: 
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
void EditGameFormDelete(Boolean direct) {
	UInt16 index = GamGetSelected();

	if (index == dmMaxRecordIndex) {
		FrmCustomAlert(FrmWarnAlert, "Select an entry first.",0,0);
		return;

	} else if (FrmCustomAlert(FrmConfirmAlert, "Do you really want to delete this entry ?", 0, 0) == FrmConfirmYes) {
		DmRemoveRecord(gameDB, index);
		if (!direct)
			FrmReturnToMain();
		GamSortList();
		SknUpdateList();
	}
}

static void EditGameFormSave(UInt16 index) {

	FieldType *fld1P, *fld2P, *fld3P, *fld4P, *fld5P, *fld6P;	// need to change this with good names
	ControlType *cck1P, *cck2P, *cck3P, *cck4P, *cck5P;	
	ListType *list1P, *list2P, *list3P;
	FormPtr frmP;

	MemHandle recordH;
	GameInfoType *gameInfo, newGameInfo;
//	UInt16 index;

	list1P = (ListType *)GetObjectPtr(EditGameGfxListList);
	list2P = (ListType *)GetObjectPtr(EditGameLanguageList);
	list3P = (ListType *)GetObjectPtr(EditGamePlatformList);
	
	fld1P = (FieldType *)GetObjectPtr(EditGameEntryNameField);
	fld2P = (FieldType *)GetObjectPtr(EditGamePathField);
	fld3P = (FieldType *)GetObjectPtr(EditGameGameField);
	fld4P = (FieldType *)GetObjectPtr(EditGameLoadSlotField);
	fld5P = (FieldType *)GetObjectPtr(EditGameStartRoomField);
	fld6P = (FieldType *)GetObjectPtr(EditGameTalkSpeedField);
	
	cck1P = (ControlType *)GetObjectPtr(EditGameLoadSlotCheckbox);
	cck2P = (ControlType *)GetObjectPtr(EditGameStartRoomCheckbox);
	cck3P = (ControlType *)GetObjectPtr(EditGameAmigaCheckbox);
	cck4P = (ControlType *)GetObjectPtr(EditGameSubtitlesCheckbox);
	cck5P = (ControlType *)GetObjectPtr(EditGameTalkSpeedCheckbox);

	frmP = FrmGetActiveForm();

	FldTrimText(fld1P);
	FldTrimText(fld2P);
	FldTrimText(fld3P);

	if (FldGetTextLength(fld1P) == 0) {
		FrmCustomAlert(FrmWarnAlert,"You must specified an entry name.",0,0);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, EditGameEntryNameField));
		return;
	} else if (FldGetTextLength(fld2P) == 0) {
		FrmCustomAlert(FrmWarnAlert,"You must specified a path.",0,0);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, EditGamePathField));
		return;
	} else if (FldGetTextLength(fld3P) == 0) {
		FrmCustomAlert(FrmWarnAlert,"You must specified a game.",0,0);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, EditGameGameField));
		return;
	} else if (FldGetTextLength(fld5P) == 0 && CtlGetValue(cck2P) == 1) {
		FrmCustomAlert(FrmWarnAlert,"You must specified a room number.",0,0);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, EditGameStartRoomField));
		return;
	} else if (FldGetTextLength(fld6P) == 0 && CtlGetValue(cck5P) == 1) {
		FrmCustomAlert(FrmWarnAlert,"You must specified a talk speed.",0,0);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, EditGameTalkSpeedField));
		return;
	}

//	index = GamGetSelected();

	if (index != dmMaxRecordIndex) {
		recordH = DmGetRecord(gameDB, index);
	} else {
		index = dmMaxRecordIndex;
		GamUnselect();
		recordH = DmNewRecord(gameDB, &index, sizeof(GameInfoType));
	}

	gameInfo = (GameInfoType *)MemHandleLock(recordH);

	StrCopy(newGameInfo.nameP, FldGetTextPtr(fld1P));
	StrCopy(newGameInfo.pathP, FldGetTextPtr(fld2P));
	StrCopy(newGameInfo.gameP, FldGetTextPtr(fld3P));

	if (newGameInfo.pathP[StrLen(newGameInfo.pathP)-1] != '/')
		StrCat(newGameInfo.pathP, "/");

	newGameInfo.version	= curItemVersion;
	newGameInfo.icnID = 0xFFFF;
	newGameInfo.gfxMode = LstGetSelection(list1P);
	newGameInfo.language = LstGetSelection(list2P);
	newGameInfo.platform = LstGetSelection(list3P);
	newGameInfo.selected = true;
	newGameInfo.autoLoad = CtlGetValue(cck1P);
	newGameInfo.loadSlot = StrAToI(FldGetTextPtr(fld4P));
	newGameInfo.bootParam = CtlGetValue(cck2P);
	newGameInfo.bootValue = StrAToI(FldGetTextPtr(fld5P));
	newGameInfo.setPlatform = CtlGetValue(cck3P);
	newGameInfo.subtitles = !(CtlGetValue(cck4P));
	newGameInfo.talkValue = StrAToI(FldGetTextPtr(fld6P));
	newGameInfo.talkSpeed = CtlGetValue(cck5P);

	DmWrite(gameInfo, 0, &newGameInfo, sizeof(GameInfoType));

	MemHandleUnlock(recordH);
	DmReleaseRecord (gameDB, index, 0);
	GamSortList();
	// update list position
	{
		RectangleType rArea;
		UInt16 posIndex, maxView;
		
		// get the sorted index
		index = GamGetSelected();
		// if new item is out of the list bounds, change current list pos
		SknGetListBounds(&rArea, NULL);
		maxView = rArea.extent.y / sknInfoListItemSize;
		posIndex = gPrefs->listPosition;
		
		// if out of the current list position
		if (!(index >= posIndex && index < (posIndex + maxView)))
			gPrefs->listPosition = index;	// this value is corrected in SknUpdateList if needed
	}

	FrmReturnToMain();
	SknUpdateList();
}

static void EditGameFormInit(UInt16 index) {

	FieldType *fld1P, *fld2P, *fld3P, *fld4P, *fld5P, *fld6P;
	FormPtr frmP;
	ListType *list1P, *list2P, *list3P;

	Char *nameP, *pathP, *gameP, *loadP, *roomP, *talkP;
	MemHandle nameH, pathH, gameH, loadH, roomH, talkH;

	MemHandle recordH = NULL;
	GameInfoType *game;

	list1P = (ListType *)GetObjectPtr(EditGameGfxListList);
	list2P = (ListType *)GetObjectPtr(EditGameLanguageList);
	list3P = (ListType *)GetObjectPtr(EditGamePlatformList);

	fld1P = (FieldType *)GetObjectPtr(EditGameEntryNameField);
	fld2P = (FieldType *)GetObjectPtr(EditGamePathField);
	fld3P = (FieldType *)GetObjectPtr(EditGameGameField);
	fld4P = (FieldType *)GetObjectPtr(EditGameLoadSlotField);
	fld5P = (FieldType *)GetObjectPtr(EditGameStartRoomField);
	fld6P = (FieldType *)GetObjectPtr(EditGameTalkSpeedField);

	nameH = MemHandleNew(FldGetMaxChars(fld1P)+1);
	pathH = MemHandleNew(FldGetMaxChars(fld2P)+1);
	gameH = MemHandleNew(FldGetMaxChars(fld3P)+1);
	loadH = MemHandleNew(FldGetMaxChars(fld4P)+1);
	roomH = MemHandleNew(FldGetMaxChars(fld5P)+1);
	talkH = MemHandleNew(FldGetMaxChars(fld6P)+1);

	nameP = (Char *)MemHandleLock(nameH);
	pathP = (Char *)MemHandleLock(pathH);
	gameP = (Char *)MemHandleLock(gameH);
	loadP = (Char *)MemHandleLock(loadH);
	roomP = (Char *)MemHandleLock(roomH);
	talkP = (Char *)MemHandleLock(talkH);

	frmP = FrmGetActiveForm();

	if (index != dmMaxRecordIndex) {
		recordH = DmQueryRecord(gameDB, index);
		game = (GameInfoType *)MemHandleLock(recordH);
		StrCopy(nameP, game->nameP);
		StrCopy(pathP, game->pathP);
		StrCopy(gameP, game->gameP);

		LstSetSelection(list1P, game->gfxMode);
		LstSetSelection(list2P, game->language);
		LstSetSelection(list3P, game->platform);
		
		StrIToA(loadP, game->loadSlot);
		StrIToA(roomP, game->bootValue);
		StrIToA(talkP, game->talkValue);

		CtlSetValue((ControlType *)GetObjectPtr(EditGameLoadSlotCheckbox), game->autoLoad);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameStartRoomCheckbox), game->bootParam);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameAmigaCheckbox), game->setPlatform);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameSubtitlesCheckbox), !game->subtitles);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameTalkSpeedCheckbox), game->talkSpeed);
		
		MemHandleUnlock(recordH);
		CtlSetUsable((ControlType *)GetObjectPtr(EditGameDeleteButton),true);
	}
	else {
		MemSet(nameP,MemHandleSize(nameH),0);
		MemSet(pathP,MemHandleSize(pathH),0);
		MemSet(gameP,MemHandleSize(gameH),0);

		StrIToA(loadP, 0);
		StrIToA(roomP, 0);
		StrIToA(talkP, 60);

		CtlSetValue((ControlType *)GetObjectPtr(EditGameLoadSlotCheckbox), 0);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameStartRoomCheckbox), 0);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameAmigaCheckbox), 0);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameSubtitlesCheckbox), 0);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameTalkSpeedCheckbox), 0);

		LstSetSelection(list1P, 1);
		LstSetSelection(list2P, 0);
		LstSetSelection(list3P, 0);
		CtlSetUsable((ControlType *)GetObjectPtr(EditGameDeleteButton),false);
		
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, EditGameEntryNameField));
	}

	MemHandleUnlock(nameH);
	MemHandleUnlock(pathH);
	MemHandleUnlock(gameH);
	MemHandleUnlock(loadH);
	MemHandleUnlock(roomH);
	MemHandleUnlock(talkH);

	FldSetTextHandle(fld1P, nameH);
	FldSetTextHandle(fld2P, pathH);
	FldSetTextHandle(fld3P, gameH);
	FldSetTextHandle(fld4P, loadH);
	FldSetTextHandle(fld5P, roomH);
	FldSetTextHandle(fld6P, talkH);

	CtlSetLabel((ControlType *)GetObjectPtr(EditGameGfxPopupPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));
	CtlSetLabel((ControlType *)GetObjectPtr(EditGameLanguagePopTrigger), LstGetSelectionText(list2P, LstGetSelection(list2P)));
	CtlSetLabel((ControlType *)GetObjectPtr(EditGamePlatformPopTrigger), LstGetSelectionText(list3P, LstGetSelection(list3P)));

	FrmDrawForm(frmP);
}

Boolean EditGameFormHandleEvent(EventPtr eventP)
{
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			switch (gFormEditMode) {
				case edtModeAdd:
					EditGameFormInit(dmMaxRecordIndex);
					break;
				case edtModeEdit:
				case edtModeParams:
				default :
					EditGameFormInit(GamGetSelected());
					break;
			}
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case EditGameOKButton:
					switch (gFormEditMode) {
						case edtModeAdd:
							EditGameFormSave(dmMaxRecordIndex);
							break;
						case edtModeEdit:
						case edtModeParams:
						default :
							EditGameFormSave(GamGetSelected());
							break;
					}
					break;

				case EditGameCancelButton:
					FrmReturnToMain();
					break;
				
				case EditGameDeleteButton:
					EditGameFormDelete(false);
					break;

				case EditGameGfxPopupPopTrigger:
					FrmList(eventP, EditGameGfxListList);
					break;

				case EditGameLanguagePopTrigger:
					FrmList(eventP, EditGameLanguageList);
					break;

				case EditGamePlatformPopTrigger:
					FrmList(eventP, EditGamePlatformList);
					break;
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}
