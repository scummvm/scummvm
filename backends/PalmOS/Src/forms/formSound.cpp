#include <PalmOS.h>

#include "start.h"
#include "forms.h"

static void SoundFormSave() {
	ControlType *cck1P, *cck2P, *cck3P, *cck4P;
	ListType *list1P;
	FieldType *fld1P, *fld2P, *fld3P;
	UInt8 tempo;
	UInt16 firstTrack;

	cck1P = (ControlType *)GetObjectPtr(SoundMusicCheckbox);
	cck2P = (ControlType *)GetObjectPtr(SoundMultiMidiCheckbox);
	cck3P = (ControlType *)GetObjectPtr(SoundMP3Checkbox);
	cck4P = (ControlType *)GetObjectPtr(SoundTrackLengthCheckbox);
	list1P = (ListType *)GetObjectPtr(SoundDriverList);
	fld1P = (FieldType *)GetObjectPtr(SoundTempoField);
	fld2P = (FieldType *)GetObjectPtr(SoundLengthSecsField);
	fld3P = (FieldType *)GetObjectPtr(SoundFirstTrackField);

	tempo = StrAToI(FldGetTextPtr(fld1P));
	if (tempo < 50 || tempo > 200) {
		FrmCustomAlert(FrmErrorAlert, "Invalid tempo value (50...200)", 0, 0);
		return;
	}

	firstTrack = StrAToI(FldGetTextPtr(fld3P));
	if (firstTrack < 1 || firstTrack > 999) {
		FrmCustomAlert(FrmErrorAlert, "Invalid track value (1...999)", 0, 0);
		return;
	}

	gPrefs->sound.music = CtlGetValue(cck1P);
	gPrefs->sound.multiMidi = CtlGetValue(cck2P);
	gPrefs->sound.MP3 = CtlGetValue(cck3P);
	gPrefs->sound.setDefaultTrackLength = CtlGetValue(cck4P);

	gPrefs->sound.driver = LstGetSelection(list1P);
	gPrefs->sound.tempo = tempo;
	gPrefs->sound.defaultTrackLength = StrAToI(FldGetTextPtr(fld2P));
	gPrefs->sound.firstTrack = firstTrack;
	FrmReturnToMain();
}

static void SoundFormInit() {
	ControlType *cck1P, *cck2P, *cck3P, *cck4P;
	ListType *list1P;
	FieldType *fld1P, *fld2P, *fld3P;
	FormPtr frmP;
	MemHandle tempoH, lengthH, firstTrackH;
	Char *tempoP, *lengthP, *firstTrackP;

	cck1P = (ControlType *)GetObjectPtr(SoundMusicCheckbox);
	cck2P = (ControlType *)GetObjectPtr(SoundMultiMidiCheckbox);
	cck3P = (ControlType *)GetObjectPtr(SoundMP3Checkbox);
	cck4P = (ControlType *)GetObjectPtr(SoundTrackLengthCheckbox);
	list1P = (ListType *)GetObjectPtr(SoundDriverList);
	fld1P = (FieldType *)GetObjectPtr(SoundTempoField);
	fld2P = (FieldType *)GetObjectPtr(SoundLengthSecsField);
	fld3P = (FieldType *)GetObjectPtr(SoundFirstTrackField);

	CtlSetValue(cck1P, gPrefs->sound.music);
	CtlSetValue(cck2P, gPrefs->sound.multiMidi);
	CtlSetValue(cck3P, gPrefs->sound.MP3);
	CtlSetValue(cck4P, gPrefs->sound.setDefaultTrackLength);

	LstSetSelection(list1P, gPrefs->sound.driver);
	CtlSetLabel((ControlType *)GetObjectPtr(SoundDriverPopTrigger), LstGetSelectionText(list1P, LstGetSelection(list1P)));

	tempoH = MemHandleNew(FldGetMaxChars(fld1P)+1);
	tempoP = (Char *)MemHandleLock(tempoH);
	StrIToA(tempoP, gPrefs->sound.tempo);
	MemHandleUnlock(tempoH);
	FldSetTextHandle(fld1P, tempoH);

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

	frmP = FrmGetActiveForm();
	FrmDrawForm(frmP);
}

Boolean SoundFormHandleEvent(EventPtr eventP) {
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			SoundFormInit();
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case SoundOKButton:
					SoundFormSave();
					break;

				case SoundCancelButton:
					FrmReturnToMain();
					break;

				case SoundDriverPopTrigger:
					FrmList(eventP, SoundDriverList);
					break;
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}