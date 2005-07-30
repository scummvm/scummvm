#include <PalmOS.h>

#include "formTabs.h"
#include "forms.h"

#include "start.h"
#include "games.h"
#include "skin.h"

#define errBadParam	0x1000

static TabType *myTabP;
static UInt16 lastTab = 0;

UInt8 gFormEditMode;

static void GameTabInit(GameInfoType *gameInfoP) {
	FieldType *fld1P, *fld2P, *fld3P;
	Char *nameP, *pathP, *gameP;
	MemHandle nameH, pathH, gameH;
	ListType *list1P;

	list1P = (ListType *)GetObjectPtr(TabGameInfoEngineList);

	fld1P = (FieldType *)GetObjectPtr(TabGameInfoEntryNameField);
	fld2P = (FieldType *)GetObjectPtr(TabGameInfoPathField);
	fld3P = (FieldType *)GetObjectPtr(TabGameInfoGameField);

	nameH = MemHandleNew(FldGetMaxChars(fld1P)+1);
	pathH = MemHandleNew(FldGetMaxChars(fld2P)+1);
	gameH = MemHandleNew(FldGetMaxChars(fld3P)+1);

	nameP = (Char *)MemHandleLock(nameH);
	pathP = (Char *)MemHandleLock(pathH);
	gameP = (Char *)MemHandleLock(gameH);

	if (gameInfoP) {
		LstSetSelection(list1P, gameInfoP->engine);
		StrCopy(nameP, gameInfoP->nameP);
		StrCopy(pathP, gameInfoP->pathP);
		StrCopy(gameP, gameInfoP->gameP);
	} else {
		LstSetSelection(list1P, 0);
		MemSet(nameP,MemHandleSize(nameH),0);
		MemSet(pathP,MemHandleSize(pathH),0);
		MemSet(gameP,MemHandleSize(gameH),0);
	}

	CtlSetLabel((ControlType *)GetObjectPtr(TabGameInfoEnginePopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));

	MemHandleUnlock(nameH);
	MemHandleUnlock(pathH);
	MemHandleUnlock(gameH);

	FldSetTextHandle(fld1P, nameH);
	FldSetTextHandle(fld2P, pathH);
	FldSetTextHandle(fld3P, gameH);
}

static Err GameTabSave(GameInfoType *gameInfoP) {
	FieldType *fld1P, *fld2P, *fld3P;
	ListType *list1P;

	FormType *frmP = FrmGetActiveForm();
	list1P = (ListType *)GetObjectPtr(TabGameInfoEngineList);

	fld1P = (FieldType *)GetObjectPtr(TabGameInfoEntryNameField);
	fld2P = (FieldType *)GetObjectPtr(TabGameInfoPathField);
	fld3P = (FieldType *)GetObjectPtr(TabGameInfoGameField);

	FldTrimText(fld1P);
	FldTrimText(fld2P);
	FldTrimText(fld3P);

	if (!gameInfoP) {
		if (FldGetTextLength(fld1P) == 0) {
			FrmCustomAlert(FrmWarnAlert,"You must specified an entry name.",0,0);
			TabSetActive(frmP, myTabP, 0);
			FrmSetFocus(frmP, FrmGetObjectIndex(frmP, TabGameInfoEntryNameField));
			return errBadParam;

		} else if (FldGetTextLength(fld2P) == 0) {
			FrmCustomAlert(FrmWarnAlert,"You must specified a path.",0,0);
			TabSetActive(frmP, myTabP, 0);
			FrmSetFocus(frmP, FrmGetObjectIndex(frmP, TabGameInfoPathField));
			return errBadParam;

		} else if (FldGetTextLength(fld3P) == 0) {
			FrmCustomAlert(FrmWarnAlert,"You must specified a game.",0,0);
			TabSetActive(frmP, myTabP, 0);
			FrmSetFocus(frmP, FrmGetObjectIndex(frmP, TabGameInfoGameField));
			return errBadParam;
		}

	} else {
		gameInfoP->engine = LstGetSelection(list1P);
		StrCopy(gameInfoP->nameP, FldGetTextPtr(fld1P));
		StrCopy(gameInfoP->pathP, FldGetTextPtr(fld2P));
		StrCopy(gameInfoP->gameP, FldGetTextPtr(fld3P));

		if (gameInfoP->pathP[StrLen(gameInfoP->pathP)-1] != '/')
			StrCat(gameInfoP->pathP, "/");

	}

	return errNone;
}

static void DisplayInit(GameInfoType *gameInfoP) {
	ListType *list1P;

	list1P = (ListType *)GetObjectPtr(TabGameDisplayGfxListList);

	if (gameInfoP) {
		LstSetSelection(list1P, gameInfoP->gfxMode);
		CtlSetValue((ControlType *)GetObjectPtr(TabGameDisplayFilterCheckbox), gameInfoP->filter);
		CtlSetValue((ControlType *)GetObjectPtr(TabGameDisplayFullscreenCheckbox), gameInfoP->fullscreen);
		CtlSetValue((ControlType *)GetObjectPtr(TabGameDisplayAspectRatioCheckbox), gameInfoP->aspectRatio);

	} else {
		LstSetSelection(list1P, 2);
		CtlSetValue((ControlType *)GetObjectPtr(TabGameDisplayFilterCheckbox), 0);
		CtlSetValue((ControlType *)GetObjectPtr(TabGameDisplayFullscreenCheckbox), 0);
		CtlSetValue((ControlType *)GetObjectPtr(TabGameDisplayAspectRatioCheckbox), 0);
	}

	CtlSetLabel((ControlType *)GetObjectPtr(TabGameDisplayGfxPopupPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));
}

static Err DisplaySave(GameInfoType *gameInfoP) {
	ListType *list1P;
	ControlType *cck6P, *cck7P, *cck8P;

	FormType *frmP = FrmGetActiveForm();

	list1P = (ListType *)GetObjectPtr(TabGameDisplayGfxListList);
	cck6P = (ControlType *)GetObjectPtr(TabGameDisplayFilterCheckbox);
	cck7P = (ControlType *)GetObjectPtr(TabGameDisplayFullscreenCheckbox);
	cck8P = (ControlType *)GetObjectPtr(TabGameDisplayAspectRatioCheckbox);

	if (!gameInfoP) {
	} else {
		gameInfoP->gfxMode = LstGetSelection(list1P);
		gameInfoP->filter = CtlGetValue(cck6P);
		gameInfoP->fullscreen = CtlGetValue(cck7P);
		gameInfoP->aspectRatio = CtlGetValue(cck8P);
	}

	return errNone;
}

static void OptionsInit(GameInfoType *gameInfoP) {
	ListType *list2P, *list3P;
	FieldType *fld4P, *fld5P, *fld6P;
	Char *loadP, *roomP, *talkP;
	MemHandle loadH, roomH, talkH;

	list2P = (ListType *)GetObjectPtr(TabGameOptionsLanguageList);
	list3P = (ListType *)GetObjectPtr(TabGameOptionsPlatformList);

	fld4P = (FieldType *)GetObjectPtr(TabGameOptionsLoadSlotField);
	fld5P = (FieldType *)GetObjectPtr(TabGameOptionsStartRoomField);
	fld6P = (FieldType *)GetObjectPtr(TabGameOptionsTalkSpeedField);

	loadH = MemHandleNew(FldGetMaxChars(fld4P)+1);
	roomH = MemHandleNew(FldGetMaxChars(fld5P)+1);
	talkH = MemHandleNew(FldGetMaxChars(fld6P)+1);

	loadP = (Char *)MemHandleLock(loadH);
	roomP = (Char *)MemHandleLock(roomH);
	talkP = (Char *)MemHandleLock(talkH);

	if (gameInfoP) {
		LstSetSelection(list2P, gameInfoP->language);
		LstSetSelection(list3P, gameInfoP->platform);

		StrIToA(loadP, gameInfoP->loadSlot);
		StrIToA(roomP, gameInfoP->bootValue);
		StrIToA(talkP, gameInfoP->talkValue);

		CtlSetValue((ControlType *)GetObjectPtr(TabGameOptionsLoadSlotCheckbox), gameInfoP->autoLoad);
		CtlSetValue((ControlType *)GetObjectPtr(TabGameOptionsStartRoomCheckbox), gameInfoP->bootParam);
		CtlSetValue((ControlType *)GetObjectPtr(TabGameOptionsAmigaCheckbox), gameInfoP->setPlatform);
		CtlSetValue((ControlType *)GetObjectPtr(TabGameOptionsSubtitlesCheckbox), gameInfoP->subtitles);
		CtlSetValue((ControlType *)GetObjectPtr(TabGameOptionsTalkSpeedCheckbox), gameInfoP->talkSpeed);

	} else {
		LstSetSelection(list2P, 0);
		LstSetSelection(list3P, 0);

		StrIToA(loadP, 0);
		StrIToA(roomP, 0);
		StrIToA(talkP, 60);

		CtlSetValue((ControlType *)GetObjectPtr(TabGameOptionsLoadSlotCheckbox), 0);
		CtlSetValue((ControlType *)GetObjectPtr(TabGameOptionsStartRoomCheckbox), 0);
		CtlSetValue((ControlType *)GetObjectPtr(TabGameOptionsAmigaCheckbox), 0);
		CtlSetValue((ControlType *)GetObjectPtr(TabGameOptionsSubtitlesCheckbox), 1);
		CtlSetValue((ControlType *)GetObjectPtr(TabGameOptionsTalkSpeedCheckbox), 0);
	}

	MemHandleUnlock(loadH);
	MemHandleUnlock(roomH);
	MemHandleUnlock(talkH);

	FldSetTextHandle(fld4P, loadH);
	FldSetTextHandle(fld5P, roomH);
	FldSetTextHandle(fld6P, talkH);

	CtlSetLabel((ControlType *)GetObjectPtr(TabGameOptionsLanguagePopTrigger), LstGetSelectionText(list2P, LstGetSelection(list2P)));
	CtlSetLabel((ControlType *)GetObjectPtr(TabGameOptionsPlatformPopTrigger), LstGetSelectionText(list3P, LstGetSelection(list3P)));
}

static Err OptionsSave(GameInfoType *gameInfoP) {
	FieldType *fld4P, *fld5P, *fld6P;
	ControlType *cck1P, *cck2P, *cck3P, *cck4P, *cck5P;
	ListType *list2P, *list3P;

	FormType *frmP = FrmGetActiveForm();

	list2P = (ListType *)GetObjectPtr(TabGameOptionsLanguageList);
	list3P = (ListType *)GetObjectPtr(TabGameOptionsPlatformList);

	fld4P = (FieldType *)GetObjectPtr(TabGameOptionsLoadSlotField);
	fld5P = (FieldType *)GetObjectPtr(TabGameOptionsStartRoomField);
	fld6P = (FieldType *)GetObjectPtr(TabGameOptionsTalkSpeedField);

	cck1P = (ControlType *)GetObjectPtr(TabGameOptionsLoadSlotCheckbox);
	cck2P = (ControlType *)GetObjectPtr(TabGameOptionsStartRoomCheckbox);
	cck3P = (ControlType *)GetObjectPtr(TabGameOptionsAmigaCheckbox);
	cck4P = (ControlType *)GetObjectPtr(TabGameOptionsSubtitlesCheckbox);
	cck5P = (ControlType *)GetObjectPtr(TabGameOptionsTalkSpeedCheckbox);

	if (!gameInfoP) {
		if (FldGetTextLength(fld5P) == 0 && CtlGetValue(cck2P) == 1) {
			FrmCustomAlert(FrmWarnAlert,"You must specified a room number.",0,0);
			TabSetActive(frmP, myTabP, 2);
			FrmSetFocus(frmP, FrmGetObjectIndex(frmP, TabGameOptionsStartRoomField));
			return errBadParam;

		} else if (FldGetTextLength(fld6P) == 0 && CtlGetValue(cck5P) == 1) {
			FrmCustomAlert(FrmWarnAlert,"You must specified a talk speed.",0,0);
			TabSetActive(frmP, myTabP, 2);
			FrmSetFocus(frmP, FrmGetObjectIndex(frmP, TabGameOptionsTalkSpeedField));
			return errBadParam;
		}
	} else {
		gameInfoP->language = LstGetSelection(list2P);
		gameInfoP->platform = LstGetSelection(list3P);

		gameInfoP->autoLoad = CtlGetValue(cck1P);
		gameInfoP->bootParam = CtlGetValue(cck2P);
		gameInfoP->setPlatform = CtlGetValue(cck3P);
		gameInfoP->subtitles = (CtlGetValue(cck4P));
		gameInfoP->talkSpeed = CtlGetValue(cck5P);

		gameInfoP->loadSlot = StrAToI(FldGetTextPtr(fld4P));
		gameInfoP->bootValue = StrAToI(FldGetTextPtr(fld5P));
		gameInfoP->talkValue = StrAToI(FldGetTextPtr(fld6P));
	}

	return errNone;
}

static void GameManInit(UInt16 index) {
	TabType *tabP;
	FormType *frmP = FrmGetActiveForm();
	UInt16 active = lastTab;

	tabP = TabNewTabs(3);
	TabAddContent(&frmP, tabP, "Game", TabGameInfoForm);
	TabAddContent(&frmP, tabP, "Display", TabGameDisplayForm);
	TabAddContent(&frmP, tabP, "Options", TabGameOptionsForm);

	if (index != dmMaxRecordIndex) {
		MemHandle recordH = NULL;
		GameInfoType *gameInfoP;

		recordH = DmQueryRecord(gameDB, index);
		gameInfoP = (GameInfoType *)MemHandleLock(recordH);

		GameTabInit(gameInfoP);
		DisplayInit(gameInfoP);
		OptionsInit(gameInfoP);

		MemHandleUnlock(recordH);
		CtlSetUsable((ControlType *)GetObjectPtr(GameEditDeleteButton),true);
	} else {
		active = 0; // new game ? start with first tab
		GameTabInit(0);
		DisplayInit(0);
		OptionsInit(0);
		CtlSetUsable((ControlType *)GetObjectPtr(GameEditDeleteButton),false);
	}

	FrmSetFocus(frmP, FrmGetObjectIndex(frmP, TabGameInfoEntryNameField));
	FrmDrawForm(frmP);
	TabSetActive(frmP, tabP, active);

	myTabP = tabP;
}

static void GameManSave(UInt16 index) {
	MemHandle recordH;
	GameInfoType *gameInfoP, newGameInfo;

	if (GameTabSave(0) == errBadParam) return;
	if (DisplaySave(0) == errBadParam) return;
	if (OptionsSave(0) == errBadParam) return;

	if (index != dmMaxRecordIndex) {
		recordH = DmGetRecord(gameDB, index);
		gameInfoP = (GameInfoType *)MemHandleLock(recordH);
		MemMove(&newGameInfo, gameInfoP, sizeof(GameInfoType));

	} else {
		index = dmMaxRecordIndex;
		GamUnselect();
		recordH = DmNewRecord(gameDB, &index, sizeof(GameInfoType));
		gameInfoP = (GameInfoType *)MemHandleLock(recordH);

		MemSet(&newGameInfo, sizeof(GameInfoType), 0);
		newGameInfo.version	= curItemVersion;
		newGameInfo.icnID = 0xFFFF;
		newGameInfo.selected = true;

		// default sound data
		newGameInfo.musicInfo.volume.master = 192;
		newGameInfo.musicInfo.volume.music = 192;
		newGameInfo.musicInfo.volume.sfx = 192;
		newGameInfo.musicInfo.volume.speech = 192;
		newGameInfo.musicInfo.volume.audiocd = 50;

		newGameInfo.musicInfo.sound.tempo = 100;
		newGameInfo.musicInfo.sound.defaultTrackLength = 10;
		newGameInfo.musicInfo.sound.firstTrack = 1;
	}

	GameTabSave(&newGameInfo);
	DisplaySave(&newGameInfo);
	OptionsSave(&newGameInfo);

	DmWrite(gameInfoP, 0, &newGameInfo, sizeof(GameInfoType));

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

	TabDeleteTabs(myTabP);
	FrmReturnToMain();
	SknUpdateList();
}

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
		if (!direct) {
			TabDeleteTabs(myTabP);
			FrmReturnToMain();
		}
		GamSortList();
		SknUpdateList();
	}
}

Boolean EditGameFormHandleEvent(EventPtr eventP) {
	FormPtr frmP = FrmGetActiveForm();
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			switch (gFormEditMode) {
				case edtModeAdd:
					GameManInit(dmMaxRecordIndex);
					break;
				case edtModeEdit:
				case edtModeParams:
				default :
					GameManInit(GamGetSelected());
					break;
			}
			handled = true;
			break;

		case keyDownEvent:
			switch (eventP->data.keyDown.chr) {
				case chrLineFeed:
				case chrCarriageReturn:
					return true;
			}
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case (GameEditForm + 1) :
				case (GameEditForm + 2) :
				case (GameEditForm + 3) :
					lastTab = (eventP->data.ctlSelect.controlID - GameEditForm - 1);
					TabSetActive(frmP, myTabP, lastTab);
					break;

				case GameEditOKButton:
					switch (gFormEditMode) {
						case edtModeAdd:
							GameManSave(dmMaxRecordIndex);
							break;
						case edtModeEdit:
						case edtModeParams:
						default :
							GameManSave(GamGetSelected());
							break;
					}
					break;

				case GameEditCancelButton:
					TabDeleteTabs(myTabP);
					FrmReturnToMain();
					break;

				case GameEditDeleteButton:
					EditGameFormDelete(false);
					break;

				case TabGameInfoEnginePopTrigger:
					FrmList(eventP, TabGameInfoEngineList);
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabGameInfoEngineList));
					break;

				case TabGameDisplayGfxPopupPopTrigger:
					FrmList(eventP, TabGameDisplayGfxListList);
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabGameDisplayGfxListList));
					break;

				case TabGameOptionsLanguagePopTrigger:
					FrmList(eventP, TabGameOptionsLanguageList);
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabGameOptionsLanguageList));
					break;

				case TabGameOptionsPlatformPopTrigger:
					FrmList(eventP, TabGameOptionsPlatformList);
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabGameOptionsPlatformList));
					break;
			}
			handled = true;
			break;

		default:
			break;
	}

	return handled;
}
