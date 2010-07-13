/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include <PalmOS.h>

#include "start.h"
#include "formTabs.h"
#include "forms.h"
#include "games.h"

#include "globals.h"

static TabType *myTabP;
static UInt16 lastTab = 0;

static GameInfoType *gameInfoP = NULL;

// Music
static Boolean MusicTabSave() {
	ControlType *cck1P, *cck2P;
	ListType *list1P, *list2P, *list3P;
	FieldType *fld1P;
	UInt16 tempo;
	FormPtr frmP;

	frmP = FrmGetActiveForm();

	cck1P = (ControlType *)GetObjectPtr(TabMusicMusicCheckbox);
	cck2P = (ControlType *)GetObjectPtr(TabMusicMultiMidiCheckbox);

	list1P = (ListType *)GetObjectPtr(TabMusicDriverList);
	list2P = (ListType *)GetObjectPtr(TabMusicRateList);
	list3P = (ListType *)GetObjectPtr(TabMusicQualityList);

	fld1P = (FieldType *)GetObjectPtr(TabMusicTempoField);

	tempo = StrAToI(FldGetTextPtr(fld1P));
	if (tempo < 50 || tempo > 200) {
		TabSetActive(frmP, myTabP, 0);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, TabMusicTempoField));
		FrmCustomAlert(FrmErrorAlert, "Invalid tempo value (50...200)", 0, 0);
		return false;
	}

	gameInfoP->musicInfo.sound.music = CtlGetValue(cck1P);
	gameInfoP->musicInfo.sound.multiMidi = CtlGetValue(cck2P);

	gameInfoP->musicInfo.sound.drvMusic = LstGetSelection(list1P);
	gameInfoP->musicInfo.sound.rate = LstGetSelection(list2P);
	gameInfoP->fmQuality = LstGetSelection(list3P);
	gameInfoP->musicInfo.sound.tempo = tempo;

	return true;
}

static void MusicTabInit() {
	ControlType *cck1P, *cck2P;
	ListType *list1P, *list2P, *list3P;
	FieldType *fld1P;
	MemHandle tempoH;
	Char *tempoP;

	cck1P = (ControlType *)GetObjectPtr(TabMusicMusicCheckbox);
	cck2P = (ControlType *)GetObjectPtr(TabMusicMultiMidiCheckbox);

	list1P = (ListType *)GetObjectPtr(TabMusicDriverList);
	list2P = (ListType *)GetObjectPtr(TabMusicRateList);
	list3P = (ListType *)GetObjectPtr(TabMusicQualityList);

	fld1P = (FieldType *)GetObjectPtr(TabMusicTempoField);

	CtlSetValue(cck1P, gameInfoP->musicInfo.sound.music);
	CtlSetValue(cck2P, gameInfoP->musicInfo.sound.multiMidi);

	if (gameInfoP->musicInfo.sound.drvMusic > 5)
		gameInfoP->musicInfo.sound.drvMusic = 0;


	LstSetSelection(list1P, gameInfoP->musicInfo.sound.drvMusic);
	LstSetTopItem(list1P, gameInfoP->musicInfo.sound.drvMusic);
	CtlSetLabel((ControlType *)GetObjectPtr(TabMusicDriverPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));

	LstSetSelection(list2P, gameInfoP->musicInfo.sound.rate);
	LstSetTopItem(list2P, gameInfoP->musicInfo.sound.rate);
	CtlSetLabel((ControlType *)GetObjectPtr(TabMusicRatePopTrigger), LstGetSelectionText(list2P, LstGetSelection(list2P)));

	LstSetSelection(list3P, gameInfoP->fmQuality);
	CtlSetLabel((ControlType *)GetObjectPtr(TabMusicQualityPopTrigger), LstGetSelectionText(list3P, LstGetSelection(list3P)));

	tempoH = MemHandleNew(FldGetMaxChars(fld1P)+1);
	tempoP = (Char *)MemHandleLock(tempoH);
	StrIToA(tempoP, gameInfoP->musicInfo.sound.tempo);
	MemHandleUnlock(tempoH);
	FldSetTextHandle(fld1P, tempoH);
}

// Audio CD
static Boolean AudioCDTabSave() {
	ControlType *cck3P;
	FieldType *fld2P, *fld3P;
	ListType *list1P, *list2P;
	UInt16 firstTrack;
	FormPtr frmP;

	frmP = FrmGetActiveForm();

	cck3P = (ControlType *)GetObjectPtr(TabAudioCDMP3Checkbox);
	fld2P = (FieldType *)GetObjectPtr(TabAudioCDLengthSecsField);
	fld3P = (FieldType *)GetObjectPtr(TabAudioCDFirstTrackField);
	list1P = (ListType *)GetObjectPtr(TabAudioCDDriverList);
	list2P = (ListType *)GetObjectPtr(TabAudioCDFormatList);

	firstTrack = StrAToI(FldGetTextPtr(fld3P));
	if (firstTrack < 1 || firstTrack > 999) {
		TabSetActive(frmP, myTabP, 2);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, TabAudioCDFirstTrackField));
		FrmCustomAlert(FrmErrorAlert, "Invalid track value (1...999)", 0, 0);
		return false;
	}

	gameInfoP->musicInfo.sound.CD = CtlGetValue(cck3P);

	gameInfoP->musicInfo.sound.drvCD = LstGetSelection(list1P);
	gameInfoP->musicInfo.sound.frtCD = LstGetSelection(list2P);

	gameInfoP->musicInfo.sound.defaultTrackLength = StrAToI(FldGetTextPtr(fld2P));
	gameInfoP->musicInfo.sound.firstTrack = firstTrack;

	return true;
}

static void AudioCDTabInit() {
	ControlType *cck3P;
	FieldType *fld2P, *fld3P;
	ListType *list1P, *list2P;
	MemHandle lengthH, firstTrackH;
	Char *lengthP, *firstTrackP;

	cck3P = (ControlType *)GetObjectPtr(TabAudioCDMP3Checkbox);
	fld2P = (FieldType *)GetObjectPtr(TabAudioCDLengthSecsField);
	fld3P = (FieldType *)GetObjectPtr(TabAudioCDFirstTrackField);
	list1P = (ListType *)GetObjectPtr(TabAudioCDDriverList);
	list2P = (ListType *)GetObjectPtr(TabAudioCDFormatList);

	LstSetSelection(list1P, gameInfoP->musicInfo.sound.drvCD);
	CtlSetLabel((ControlType *)GetObjectPtr(TabAudioCDDriverPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));

	LstSetSelection(list2P, gameInfoP->musicInfo.sound.frtCD);
	CtlSetLabel((ControlType *)GetObjectPtr(TabAudioCDFormatPopTrigger), LstGetSelectionText(list2P, LstGetSelection(list2P)));

	CtlSetValue(cck3P, gameInfoP->musicInfo.sound.CD);

	lengthH = MemHandleNew(FldGetMaxChars(fld2P)+1);
	lengthP = (Char *)MemHandleLock(lengthH);
	StrIToA(lengthP, gameInfoP->musicInfo.sound.defaultTrackLength);
	MemHandleUnlock(lengthH);
	FldSetTextHandle(fld2P, lengthH);

	firstTrackH = MemHandleNew(FldGetMaxChars(fld3P)+1);
	firstTrackP = (Char *)MemHandleLock(firstTrackH);
	StrIToA(firstTrackP, gameInfoP->musicInfo.sound.firstTrack);
	MemHandleUnlock(firstTrackH);
	FldSetTextHandle(fld3P, firstTrackH);
}

// Volume
static void VolumeTabSave() {
	SliderControlType *slid1P, *slid2P, *slid3P, *slid4P, *slid5P;

	slid1P = (SliderControlType *)GetObjectPtr(TabVolumePalmSliderControl);
	slid2P = (SliderControlType *)GetObjectPtr(TabVolumeMusicSliderControl);
	slid3P = (SliderControlType *)GetObjectPtr(TabVolumeSfxSliderControl);
	slid4P = (SliderControlType *)GetObjectPtr(TabVolumeSpeechSliderControl);
	slid5P = (SliderControlType *)GetObjectPtr(TabVolumeAudioCDSliderControl);

	CtlGetSliderValues ((ControlType *)slid1P, 0, 0, 0, &gameInfoP->musicInfo.volume.palm);
	CtlGetSliderValues ((ControlType *)slid2P, 0, 0, 0, &gameInfoP->musicInfo.volume.music);
	CtlGetSliderValues ((ControlType *)slid3P, 0, 0, 0, &gameInfoP->musicInfo.volume.sfx);
	CtlGetSliderValues ((ControlType *)slid4P, 0, 0, 0, &gameInfoP->musicInfo.volume.speech);
	CtlGetSliderValues ((ControlType *)slid5P, 0, 0, 0, &gameInfoP->musicInfo.volume.audiocd);
}

static void VolumeTabInit() {
	SliderControlType *slid1P, *slid2P, *slid3P, *slid4P, *slid5P;
	UInt16 value;

	slid1P = (SliderControlType *)GetObjectPtr(TabVolumePalmSliderControl);
	slid2P = (SliderControlType *)GetObjectPtr(TabVolumeMusicSliderControl);
	slid3P = (SliderControlType *)GetObjectPtr(TabVolumeSfxSliderControl);
	slid4P = (SliderControlType *)GetObjectPtr(TabVolumeSpeechSliderControl);
	slid5P = (SliderControlType *)GetObjectPtr(TabVolumeAudioCDSliderControl);

	value = gameInfoP->musicInfo.volume.palm;
	CtlSetSliderValues ((ControlType *)slid1P, 0, 0, 0, &value);
	value = gameInfoP->musicInfo.volume.music;
	CtlSetSliderValues ((ControlType *)slid2P, 0, 0, 0, &value);
	value = gameInfoP->musicInfo.volume.sfx;
	CtlSetSliderValues ((ControlType *)slid3P, 0, 0, 0, &value);
	value = gameInfoP->musicInfo.volume.speech;
	CtlSetSliderValues ((ControlType *)slid4P, 0, 0, 0, &value);
	value = gameInfoP->musicInfo.volume.audiocd;
	CtlSetSliderValues ((ControlType *)slid5P, 0, 0, 0, &value);
}

static void MusicFormSave(UInt16 index) {
	if (index != dmMaxRecordIndex) {
		MemHandle recordH;
		GameInfoType *ogameInfoP;

		if (!MusicTabSave()) return;
		if (!AudioCDTabSave()) return;
		VolumeTabSave();

		recordH = DmGetRecord(gameDB, index);
		ogameInfoP = (GameInfoType *)MemHandleLock(recordH);
		DmWrite(ogameInfoP, 0, gameInfoP, sizeof(GameInfoType));
		MemHandleUnlock(recordH);
		DmReleaseRecord (gameDB, index, 0);
	}

	MemPtrFree(gameInfoP);
	gameInfoP = NULL;

	TabDeleteTabs(myTabP);
	FrmReturnToMain();
}
static void MusicFormInit(UInt16 index) {
	TabType *tabP;
	FormType *frmP = FrmGetActiveForm();

	if (index != dmMaxRecordIndex) {
		MemHandle recordH = NULL;
		GameInfoType *ogameInfoP;

		recordH = DmQueryRecord(gameDB, index);
		ogameInfoP = (GameInfoType *)MemHandleLock(recordH);

		if (!ogameInfoP) {
			FrmCustomAlert(FrmErrorAlert, "An error occurred.",0,0);
			return;
		}

		gameInfoP = (GameInfoType *)MemPtrNew(sizeof(GameInfoType));
		MemMove(gameInfoP, ogameInfoP, sizeof(GameInfoType));
		MemHandleUnlock(recordH);

	} else {
		FrmCustomAlert(FrmWarnAlert, "Select an entry first.",0,0);
		FrmReturnToMain();
		return;
	}

	tabP = TabNewTabs(3);
	TabAddContent(&frmP, tabP, "Sound", TabMusicForm);
	TabAddContent(&frmP, tabP, "Volume", TabVolumeForm);
	TabAddContent(&frmP, tabP, "Audio CD", TabAudioCDForm);

	MusicTabInit();
	AudioCDTabInit();
	VolumeTabInit();

	FrmDrawForm(frmP);
	TabSetActive(frmP, tabP, lastTab);

	myTabP = tabP;
}

Boolean MusicFormHandleEvent(EventPtr eventP) {
	FormPtr frmP = FrmGetActiveForm();
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			MusicFormInit(GamGetSelected());
			handled = true;
			break;

		case frmCloseEvent:
			MusicFormSave(dmMaxRecordIndex);
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case (MusicForm + 1) :
				case (MusicForm + 2) :
				case (MusicForm + 3) :
					lastTab = (eventP->data.ctlSelect.controlID - MusicForm - 1);
					TabSetActive(frmP, myTabP, lastTab);
					break;

				case TabMusicDriverPopTrigger:
					FrmList(eventP, TabMusicDriverList);
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabMusicDriverList));

					if (!OPTIONS_TST(kOptDeviceZodiac) && !OPTIONS_TST(kOptSonyPa1LibAPI)) {
						ListType *list1P = (ListType *)GetObjectPtr(TabMusicDriverList);

						if (LstGetSelection(list1P) == 4) {
							FrmCustomAlert(FrmInfoAlert, "There is no built-in MIDI support on your device.", 0, 0);
							LstSetSelection(list1P, 0);
							CtlSetLabel((ControlType *)GetObjectPtr(TabMusicDriverPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));
						}
					}
					break;

				case TabMusicQualityPopTrigger:
					FrmList(eventP, TabMusicQualityList);
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabMusicQualityList));
					break;

				case TabMusicRatePopTrigger:
					FrmList(eventP, TabMusicRateList);
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabMusicRateList));

					if (!OPTIONS_TST(kOptPalmSoundAPI)) {
						ListType *list1P = (ListType *)GetObjectPtr(TabMusicRateList);

						if (LstGetSelection(list1P) != 0) {
							FrmCustomAlert(FrmInfoAlert, "You cannot use this rate on your device.", 0, 0);
							LstSetSelection(list1P, 0);
							CtlSetLabel((ControlType *)GetObjectPtr(TabMusicRatePopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));
						}
					}
					break;

				case TabAudioCDFormatPopTrigger:
					FrmList(eventP, TabAudioCDFormatList);
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabAudioCDFormatList));

					if (LstGetSelection((ListPtr)GetObjectPtr(TabAudioCDDriverList)) == 1) {
						ListType *list1P = (ListType *)GetObjectPtr(TabAudioCDFormatList);
						LstSetSelection(list1P, 0);
						CtlSetLabel((ControlType *)GetObjectPtr(TabAudioCDFormatPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));
					}
					break;

				case TabAudioCDDriverPopTrigger:
					FrmList(eventP, TabAudioCDDriverList);
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabAudioCDDriverList));

					if (LstGetSelection((ListPtr)GetObjectPtr(TabAudioCDDriverList)) == 1) {
						ListType *list1P = (ListType *)GetObjectPtr(TabAudioCDFormatList);
						LstSetSelection(list1P, 0);
						CtlSetLabel((ControlType *)GetObjectPtr(TabAudioCDFormatPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));
					}
					break;

				case MusicOKButton:
					MusicFormSave(GamGetSelected());
					break;

				case MusicCancelButton:
					MusicFormSave(dmMaxRecordIndex);
					break;
			}
			handled = true;
			break;

		default:
			break;
	}

	return handled;
}
