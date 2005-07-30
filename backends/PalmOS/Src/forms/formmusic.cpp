#include <PalmOS.h>

#include "start.h"
#include "formTabs.h"
#include "forms.h"
#include "games.h"

#include "globals.h"

static TabType *myTabP;
static UInt16 lastTab = 0;

static MusicInfoType *musicInfoP = NULL;

// Music
static Boolean MusicTabSave() {
	ControlType *cck1P, *cck2P;
	ListType *list1P;
	FieldType *fld1P;
	UInt16 tempo;
	FormPtr frmP;

	frmP = FrmGetActiveForm();

	cck1P = (ControlType *)GetObjectPtr(TabMusicMusicCheckbox);
	cck2P = (ControlType *)GetObjectPtr(TabMusicMultiMidiCheckbox);
	list1P = (ListType *)GetObjectPtr(TabMusicDriverList);
	fld1P = (FieldType *)GetObjectPtr(TabMusicTempoField);

	tempo = StrAToI(FldGetTextPtr(fld1P));
	if (tempo < 50 || tempo > 200) {
		TabSetActive(frmP, myTabP, 0);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, TabMusicTempoField));
		FrmCustomAlert(FrmErrorAlert, "Invalid tempo value (50...200)", 0, 0);
		return false;
	}

	musicInfoP->sound.music = CtlGetValue(cck1P);
	musicInfoP->sound.multiMidi = CtlGetValue(cck2P);

	musicInfoP->sound.drvMusic = LstGetSelection(list1P);
	musicInfoP->sound.tempo = tempo;

	return true;
}

static void MusicTabInit() {
	ControlType *cck1P, *cck2P;
	ListType *list1P;
	FieldType *fld1P;
	MemHandle tempoH;
	Char *tempoP;

	cck1P = (ControlType *)GetObjectPtr(TabMusicMusicCheckbox);
	cck2P = (ControlType *)GetObjectPtr(TabMusicMultiMidiCheckbox);
	list1P = (ListType *)GetObjectPtr(TabMusicDriverList);
	fld1P = (FieldType *)GetObjectPtr(TabMusicTempoField);

	CtlSetValue(cck1P, musicInfoP->sound.music);
	CtlSetValue(cck2P, musicInfoP->sound.multiMidi);

	if (musicInfoP->sound.drvMusic > 5)
		musicInfoP->sound.drvMusic = 0;

	LstSetSelection(list1P, musicInfoP->sound.drvMusic);
	CtlSetLabel((ControlType *)GetObjectPtr(TabMusicDriverPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));

	tempoH = MemHandleNew(FldGetMaxChars(fld1P)+1);
	tempoP = (Char *)MemHandleLock(tempoH);
	StrIToA(tempoP, musicInfoP->sound.tempo);
	MemHandleUnlock(tempoH);
	FldSetTextHandle(fld1P, tempoH);
}

// Sound
static void SoundTabSave() {
	ControlType *cck1P;
	ListType *list1P;

	cck1P = (ControlType *)GetObjectPtr(TabSoundSoundCheckbox);
	list1P = (ListType *)GetObjectPtr(TabSoundRateList);

	musicInfoP->sound.sfx = CtlGetValue(cck1P);
	musicInfoP->sound.rate = LstGetSelection(list1P);
}

static void SoundTabInit() {
	ControlType *cck1P;
	ListType *list1P;

	cck1P = (ControlType *)GetObjectPtr(TabSoundSoundCheckbox);
	list1P = (ListType *)GetObjectPtr(TabSoundRateList);

	CtlSetValue(cck1P, musicInfoP->sound.sfx);
	LstSetSelection(list1P, musicInfoP->sound.rate);
	CtlSetLabel((ControlType *)GetObjectPtr(TabSoundRatePopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));
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

	musicInfoP->sound.CD = CtlGetValue(cck3P);

	musicInfoP->sound.drvCD = LstGetSelection(list1P);
	musicInfoP->sound.frtCD = LstGetSelection(list2P);

	musicInfoP->sound.defaultTrackLength = StrAToI(FldGetTextPtr(fld2P));
	musicInfoP->sound.firstTrack = firstTrack;

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

	LstSetSelection(list1P, musicInfoP->sound.drvCD);
	CtlSetLabel((ControlType *)GetObjectPtr(TabAudioCDDriverPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));

	LstSetSelection(list2P, musicInfoP->sound.frtCD);
	CtlSetLabel((ControlType *)GetObjectPtr(TabAudioCDFormatPopTrigger), LstGetSelectionText(list2P, LstGetSelection(list2P)));

	CtlSetValue(cck3P, musicInfoP->sound.CD);

	lengthH = MemHandleNew(FldGetMaxChars(fld2P)+1);
	lengthP = (Char *)MemHandleLock(lengthH);
	StrIToA(lengthP, musicInfoP->sound.defaultTrackLength);
	MemHandleUnlock(lengthH);
	FldSetTextHandle(fld2P, lengthH);

	firstTrackH = MemHandleNew(FldGetMaxChars(fld3P)+1);
	firstTrackP = (Char *)MemHandleLock(firstTrackH);
	StrIToA(firstTrackP, musicInfoP->sound.firstTrack);
	MemHandleUnlock(firstTrackH);
	FldSetTextHandle(fld3P, firstTrackH);
}

// Volume
static void VolumeTabSave() {
	SliderControlType *slid1P, *slid2P, *slid3P, *slid4P, *slid5P;

	slid1P = (SliderControlType *)GetObjectPtr(TabVolumeMasterSliderControl);
	slid2P = (SliderControlType *)GetObjectPtr(TabVolumeMusicSliderControl);
	slid3P = (SliderControlType *)GetObjectPtr(TabVolumeSfxSliderControl);
	slid4P = (SliderControlType *)GetObjectPtr(TabVolumeSpeechSliderControl);
	slid5P = (SliderControlType *)GetObjectPtr(TabVolumeAudioCDSliderControl);

	CtlGetSliderValues ((ControlType *)slid1P, 0, 0, 0, &musicInfoP->volume.master);
	CtlGetSliderValues ((ControlType *)slid2P, 0, 0, 0, &musicInfoP->volume.music);
	CtlGetSliderValues ((ControlType *)slid3P, 0, 0, 0, &musicInfoP->volume.sfx);
	CtlGetSliderValues ((ControlType *)slid4P, 0, 0, 0, &musicInfoP->volume.speech);
	CtlGetSliderValues ((ControlType *)slid5P, 0, 0, 0, &musicInfoP->volume.audiocd);
}

static void VolumeTabInit() {
	SliderControlType *slid1P, *slid2P, *slid3P, *slid4P, *slid5P;
	UInt16 value;

	slid1P = (SliderControlType *)GetObjectPtr(TabVolumeMasterSliderControl);
	slid2P = (SliderControlType *)GetObjectPtr(TabVolumeMusicSliderControl);
	slid3P = (SliderControlType *)GetObjectPtr(TabVolumeSfxSliderControl);
	slid4P = (SliderControlType *)GetObjectPtr(TabVolumeSpeechSliderControl);
	slid5P = (SliderControlType *)GetObjectPtr(TabVolumeAudioCDSliderControl);

	value = musicInfoP->volume.master;
	CtlSetSliderValues ((ControlType *)slid1P, 0, 0, 0, &value);
	value = musicInfoP->volume.music;
	CtlSetSliderValues ((ControlType *)slid2P, 0, 0, 0, &value);
	value = musicInfoP->volume.sfx;
	CtlSetSliderValues ((ControlType *)slid3P, 0, 0, 0, &value);
	value = musicInfoP->volume.speech;
	CtlSetSliderValues ((ControlType *)slid4P, 0, 0, 0, &value);
	value = musicInfoP->volume.audiocd;
	CtlSetSliderValues ((ControlType *)slid5P, 0, 0, 0, &value);
}


static void MusicFormSave(UInt16 index) {
	if (index != dmMaxRecordIndex) {
		MemHandle recordH;
		GameInfoType *gameInfoP;

		if (!MusicTabSave()) return;
		SoundTabSave();
		if (!AudioCDTabSave()) return;
		VolumeTabSave();

		recordH = DmGetRecord(gameDB, index);
		gameInfoP = (GameInfoType *)MemHandleLock(recordH);
		DmWrite(gameInfoP, OffsetOf(GameInfoType, musicInfo), musicInfoP, sizeof(MusicInfoType));
		MemHandleUnlock(recordH);
		DmReleaseRecord (gameDB, index, 0);
	}

	MemPtrFree(musicInfoP);
	musicInfoP = NULL;

	TabDeleteTabs(myTabP);
	FrmReturnToMain();
}
static void MusicFormInit(UInt16 index) {
	TabType *tabP;
	FormType *frmP = FrmGetActiveForm();

	if (index != dmMaxRecordIndex) {
		MemHandle recordH = NULL;
		GameInfoType *gameInfoP;

		recordH = DmQueryRecord(gameDB, index);
		gameInfoP = (GameInfoType *)MemHandleLock(recordH);

		if (!gameInfoP) {
			FrmCustomAlert(FrmErrorAlert, "An error occured.",0,0);
			return;
		}

		musicInfoP = (MusicInfoType *)MemPtrNew(sizeof(MusicInfoType));
		MemMove(musicInfoP, &gameInfoP->musicInfo, sizeof(MusicInfoType));
		MemHandleUnlock(recordH);

	} else {
		FrmCustomAlert(FrmWarnAlert, "Select an entry first.",0,0);
		FrmReturnToMain();
		return;
	}

	tabP = TabNewTabs(4);
	TabAddContent(&frmP, tabP, "Music", TabMusicForm);
	TabAddContent(&frmP, tabP, "Sound", TabSoundForm);
	TabAddContent(&frmP, tabP, "Audio CD", TabAudioCDForm);
	TabAddContent(&frmP, tabP, "Volume", TabVolumeForm);

	MusicTabInit();
	SoundTabInit();
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

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case (MusicForm + 1) :
				case (MusicForm + 2) :
				case (MusicForm + 3) :
				case (MusicForm + 4) :
					lastTab = (eventP->data.ctlSelect.controlID - MusicForm - 1);
					TabSetActive(frmP, myTabP, lastTab);
					break;

				case TabMusicDriverPopTrigger:
					FrmList(eventP, TabMusicDriverList);
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabMusicDriverList));

					if (!OPTIONS_TST(kOptDeviceZodiac) && !OPTIONS_TST(kOptSonyPa1LibAPI)) {
						ListType *list1P = (ListType *)GetObjectPtr(TabMusicDriverList);

						if (LstGetSelection(list1P) == 1) {
							FrmCustomAlert(FrmInfoAlert, "There is no built-in MIDI support on your device.", 0, 0);
							LstSetSelection(list1P, 0);
							CtlSetLabel((ControlType *)GetObjectPtr(TabMusicDriverPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));
						}
					}
					break;

				case TabSoundRatePopTrigger:
					FrmList(eventP, TabSoundRateList);
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabSoundRateList));

					if (!OPTIONS_TST(kOptPalmSoundAPI)) {
						ListType *list1P = (ListType *)GetObjectPtr(TabSoundRateList);

						if (LstGetSelection(list1P) != 0) {
							FrmCustomAlert(FrmInfoAlert, "You cannot use this rate on your device.", 0, 0);
							LstSetSelection(list1P, 0);
							CtlSetLabel((ControlType *)GetObjectPtr(TabSoundRatePopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));
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
