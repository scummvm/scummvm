#include <PalmOS.h>

#include "start.h"
#include "formTabs.h"
#include "forms.h"

static TabType *myTabP;
static UInt16 lastTab = 0;

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

	gPrefs->sound.music = CtlGetValue(cck1P);
	gPrefs->sound.multiMidi = CtlGetValue(cck2P);

	gPrefs->sound.drvMusic = LstGetSelection(list1P);
	gPrefs->sound.tempo = tempo;
	
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

	CtlSetValue(cck1P, gPrefs->sound.music);
	CtlSetValue(cck2P, gPrefs->sound.multiMidi);

	LstSetSelection(list1P, gPrefs->sound.drvMusic);
	CtlSetLabel((ControlType *)GetObjectPtr(TabMusicDriverPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));

	tempoH = MemHandleNew(FldGetMaxChars(fld1P)+1);
	tempoP = (Char *)MemHandleLock(tempoH);
	StrIToA(tempoP, gPrefs->sound.tempo);
	MemHandleUnlock(tempoH);
	FldSetTextHandle(fld1P, tempoH);
}

// Sound
static void SoundTabSave() {
	ControlType *cck1P;
	
	cck1P = (ControlType *)GetObjectPtr(TabSoundSoundCheckbox);
	gPrefs->sound.sfx = CtlGetValue(cck1P);
}

static void SoundTabInit() {
	ControlType *cck1P;
	
	cck1P = (ControlType *)GetObjectPtr(TabSoundSoundCheckbox);
	CtlSetValue(cck1P, gPrefs->sound.sfx);
}

// Audio CD
static Boolean AudioCDTabSave() {
	ControlType *cck3P, *cck4P;
	FieldType *fld2P, *fld3P;
	ListType *list1P;
	UInt16 firstTrack;
	FormPtr frmP;
	
	frmP = FrmGetActiveForm();

	cck3P = (ControlType *)GetObjectPtr(TabAudioCDMP3Checkbox);
	cck4P = (ControlType *)GetObjectPtr(TabAudioCDTrackLengthCheckbox);
	fld2P = (FieldType *)GetObjectPtr(TabAudioCDLengthSecsField);
	fld3P = (FieldType *)GetObjectPtr(TabAudioCDFirstTrackField);
	list1P = (ListType *)GetObjectPtr(TabAudioCDDriverList);

	firstTrack = StrAToI(FldGetTextPtr(fld3P));
	if (firstTrack < 1 || firstTrack > 999) {
		TabSetActive(frmP, myTabP, 2);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, TabAudioCDFirstTrackField));
		FrmCustomAlert(FrmErrorAlert, "Invalid track value (1...999)", 0, 0);
		return false;
	}

	gPrefs->sound.CD = CtlGetValue(cck3P);
	gPrefs->sound.setDefaultTrackLength = CtlGetValue(cck4P);

	gPrefs->sound.drvCD = LstGetSelection(list1P);

	gPrefs->sound.defaultTrackLength = StrAToI(FldGetTextPtr(fld2P));
	gPrefs->sound.firstTrack = firstTrack;
	
	return true;
}

static void AudioCDTabInit() {
	ControlType *cck3P, *cck4P;
	FieldType *fld2P, *fld3P;
	ListType *list1P;
	MemHandle lengthH, firstTrackH;
	Char *lengthP, *firstTrackP;

	cck3P = (ControlType *)GetObjectPtr(TabAudioCDMP3Checkbox);
	cck4P = (ControlType *)GetObjectPtr(TabAudioCDTrackLengthCheckbox);
	fld2P = (FieldType *)GetObjectPtr(TabAudioCDLengthSecsField);
	fld3P = (FieldType *)GetObjectPtr(TabAudioCDFirstTrackField);
	list1P = (ListType *)GetObjectPtr(TabAudioCDDriverList);

	LstSetSelection(list1P, gPrefs->sound.drvCD);
	CtlSetLabel((ControlType *)GetObjectPtr(TabAudioCDDriverPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));

	CtlSetValue(cck3P, gPrefs->sound.CD);
	CtlSetValue(cck4P, gPrefs->sound.setDefaultTrackLength);

	lengthH = MemHandleNew(FldGetMaxChars(fld2P)+1);
	lengthP = (Char *)MemHandleLock(lengthH);
	StrIToA(lengthP, gPrefs->sound.defaultTrackLength);
	MemHandleUnlock(lengthH);
	FldSetTextHandle(fld2P, lengthH);

	firstTrackH = MemHandleNew(FldGetMaxChars(fld3P)+1);
	firstTrackP = (Char *)MemHandleLock(firstTrackH);
	StrIToA(firstTrackP, gPrefs->sound.firstTrack);
	MemHandleUnlock(firstTrackH);
	FldSetTextHandle(fld3P, firstTrackH);
}

// Volume
static void VolumeTabSave() {
	SliderControlType *slid1P, *slid2P, *slid3P, *slid4P;
	
	slid1P = (SliderControlType *)GetObjectPtr(TabVolumeMasterSliderControl);
	slid2P = (SliderControlType *)GetObjectPtr(TabVolumeMusicSliderControl);
	slid3P = (SliderControlType *)GetObjectPtr(TabVolumeSfxSliderControl);
	slid4P = (SliderControlType *)GetObjectPtr(TabVolumeSpeechSliderControl);
		
	CtlGetSliderValues ((ControlType *)slid1P, 0, 0, 0, &gPrefs->volume.master);
	CtlGetSliderValues ((ControlType *)slid2P, 0, 0, 0, &gPrefs->volume.music);
	CtlGetSliderValues ((ControlType *)slid3P, 0, 0, 0, &gPrefs->volume.sfx);
	CtlGetSliderValues ((ControlType *)slid4P, 0, 0, 0, &gPrefs->volume.speech);
}

static void VolumeTabInit() {
	SliderControlType *slid1P, *slid2P, *slid3P, *slid4P;
	UInt16 value;
	
	slid1P = (SliderControlType *)GetObjectPtr(TabVolumeMasterSliderControl);
	slid2P = (SliderControlType *)GetObjectPtr(TabVolumeMusicSliderControl);
	slid3P = (SliderControlType *)GetObjectPtr(TabVolumeSfxSliderControl);
	slid4P = (SliderControlType *)GetObjectPtr(TabVolumeSpeechSliderControl);

	value = gPrefs->volume.master;
	CtlSetSliderValues ((ControlType *)slid1P, 0, 0, 0, &value);
	value = gPrefs->volume.music;
	CtlSetSliderValues ((ControlType *)slid2P, 0, 0, 0, &value);
	value = gPrefs->volume.sfx;
	CtlSetSliderValues ((ControlType *)slid3P, 0, 0, 0, &value);
	value = gPrefs->volume.speech;
	CtlSetSliderValues ((ControlType *)slid4P, 0, 0, 0, &value);
}


static void MusicFormSave() {
	if (!MusicTabSave()) return;
	SoundTabSave();
	if (!AudioCDTabSave()) return;
	VolumeTabSave();

	TabDeleteTabs(myTabP);
	FrmReturnToMain();
}
static void MusicFormInit() {
	TabType *tabP;
	FormType *frmP = FrmGetActiveForm();

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
			MusicFormInit();
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
					break;

				case TabAudioCDDriverPopTrigger:
					FrmList(eventP, TabAudioCDDriverList);
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, TabAudioCDDriverList));
					break;

				case MusicOKButton:
					MusicFormSave();
					break;

				case MusicCancelButton:
					TabDeleteTabs(myTabP);
					FrmReturnToMain();
					break;
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}
