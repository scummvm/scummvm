#include <PalmOS.h>

#include "start.h"
#include "forms.h"

/***********************************************************************
 *
 * FUNCTION:    VolumeFormSave
 * FUNCTION:    VolumeFormInit
 * FUNCTION:    VolumeFormHandleEvent
 *
 * DESCRIPTION: 
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/

static void VolumeFormSave() {
	SliderControlType *slid1P, *slid2P, *slid3P, *slid4P, *slid5P;
	
	slid1P = (SliderControlType *)GetObjectPtr(VolumeSpeakerSliderControl);
	slid2P = (SliderControlType *)GetObjectPtr(VolumeHeadphoneSliderControl);

	slid3P = (SliderControlType *)GetObjectPtr(VolumeMasterSliderControl);
	slid4P = (SliderControlType *)GetObjectPtr(VolumeMusicSliderControl);
	slid5P = (SliderControlType *)GetObjectPtr(VolumeSfxSliderControl);
	
	CtlGetSliderValues ((ControlType *)slid1P, 0, 0, 0, &gPrefs->volume.speaker) ;
	CtlGetSliderValues ((ControlType *)slid2P, 0, 0, 0, &gPrefs->volume.headphone) ;

	CtlGetSliderValues ((ControlType *)slid3P, 0, 0, 0, &gPrefs->volume.master);
	CtlGetSliderValues ((ControlType *)slid4P, 0, 0, 0, &gPrefs->volume.music);
	CtlGetSliderValues ((ControlType *)slid5P, 0, 0, 0, &gPrefs->volume.sfx);
	
	FrmReturnToMain();
}

static void VolumeFormInit() {
	SliderControlType *slid1P, *slid2P, *slid3P, *slid4P, *slid5P;
	FormPtr frmP;
	UInt16 value;
	
	slid1P = (SliderControlType *)GetObjectPtr(VolumeSpeakerSliderControl);
	slid2P = (SliderControlType *)GetObjectPtr(VolumeHeadphoneSliderControl);

	slid3P = (SliderControlType *)GetObjectPtr(VolumeMasterSliderControl);
	slid4P = (SliderControlType *)GetObjectPtr(VolumeMusicSliderControl);
	slid5P = (SliderControlType *)GetObjectPtr(VolumeSfxSliderControl);

	value = gPrefs->volume.speaker;
	CtlSetSliderValues ((ControlType *)slid1P, 0, 0, 0, &value);
	value = gPrefs->volume.headphone;
	CtlSetSliderValues ((ControlType *)slid2P, 0, 0, 0, &value);

	value = gPrefs->volume.master;
	CtlSetSliderValues ((ControlType *)slid3P, 0, 0, 0, &value);
	value = gPrefs->volume.music;
	CtlSetSliderValues ((ControlType *)slid4P, 0, 0, 0, &value);
	value = gPrefs->volume.sfx;
	CtlSetSliderValues ((ControlType *)slid5P, 0, 0, 0, &value);

	frmP = FrmGetActiveForm();
	FrmDrawForm(frmP);
}

Boolean VolumeFormHandleEvent(EventPtr eventP) {
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			VolumeFormInit();
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case VolumeOKButton:
					VolumeFormSave();
					break;

				case VolumeCancelButton:
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