#include <PalmOS.h>

#ifndef DISABLE_SONY
#include <SonyClie.h>
#endif

#include <PalmNavigator.h>
#include <HsExtCommon.h>
#include <HsNavCommon.h>
#include <PalmGoLCD.h>

#include "globals.h"		// for OPTIONS_DEF()
#include "init_stuffs.h"
#include "stuffs.h"

#ifndef DISABLE_TAPWAVE
#define __TWKEYS_H__	// bad hack
#include "tapwave.h"
#endif

#ifndef DISABLE_PA1LIB
#include "Pa1Lib.h"
#endif

#ifndef DISABLE_LIGHTSPEED
#include "lightspeed_public.h"
#endif

// TODO : check the depth to set correct value
// works only for 8bit for now
UInt32 StuffsGetPitch(Coord fullw) {
	UInt32 pitch = 0;

	if (OPTIONS_TST(kOptModeHiDensity)) {
		WinScreenGetAttribute(winScreenRowBytes, &pitch);

		// FIXME : hack for TT3 simulator (and real ?) return 28 on landscape mode
		if (pitch < fullw)
			pitch = fullw;

	} else {
		pitch = fullw;
	}

	return pitch;
}

void *StuffsForceVG() {
	// create an empty form to force the VG to be shown
	FormType *frmP = FrmNewForm(4567, NULL, 0,0,0,0, false, 0, 0, 0);
	FrmDrawForm(frmP);
	return frmP;
}

void StuffsReleaseVG(void *vg) {
	FrmDeleteForm((FormPtr)vg);
}

void StuffsGetFeatures() {
	UInt32 ulProcessorType, manufacturer, version, depth;
	Boolean color;

#ifndef DISABLE_TAPWAVE
	// Tapwave Zodiac libs ?
	if (!FtrGet(sysFileCSystem, sysFtrNumOEMCompanyID, &manufacturer))
		if (manufacturer == twCreatorID) {
			OPTIONS_SET(kOptDeviceZodiac);
			OPTIONS_SET(kOpt5WayNavigatorV2);
		}
#endif

	// Hi-Density present ?
	if (!FtrGet(sysFtrCreator, sysFtrNumWinVersion, &version))
		if (version >= 4)
			OPTIONS_SET(kOptModeHiDensity);

	// OS5 ?
	if (!FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version))
		if (version >= kOS5Version)
			OPTIONS_SET(kOptDeviceOS5);

	// ARM ?
	if (!FtrGet(sysFileCSystem, sysFtrNumProcessorID, &ulProcessorType))
		if (sysFtrNumProcessorIsARM(ulProcessorType))
			OPTIONS_SET(kOptDeviceARM);
		else if (ulProcessorType == sysFtrNumProcessorx86)
			OPTIONS_SET(kOptDeviceProcX86);

	// 5Way Navigator
	if (!FtrGet(hsFtrCreator, hsFtrIDNavigationSupported, &version)) {
		if (version >= 2)
			OPTIONS_SET(kOpt5WayNavigatorV2);

	} else if (!FtrGet(sysFtrCreator, sysFtrNumFiveWayNavVersion, &version)) {
		if (version >= 2)
			OPTIONS_SET(kOpt5WayNavigatorV2);
		else
			OPTIONS_SET(kOpt5WayNavigatorV1);

	} else if (!FtrGet(navFtrCreator, navFtrVersion, &version)) {
		if (version >= 2)
			OPTIONS_SET(kOpt5WayNavigatorV2);
		else
			OPTIONS_SET(kOpt5WayNavigatorV1);
	}

	// Palm Sound API ?
	if (!FtrGet(sysFileCSoundMgr, sndFtrIDVersion, &version))
		if (version >= 1)
			OPTIONS_SET(kOptPalmSoundAPI);

#ifndef DISABLE_PA1LIB
	// Sony Pa1 Sound API
	if (Pa1Lib_Open()) {
		OPTIONS_SET(kOptSonyPa1LibAPI);
		Pa1Lib_Close();
	}
#endif

	// GoLCD
	if (!FtrGet(goLcdLibCreator, goLcdLibFtrNum, &version))
		OPTIONS_SET(kOptGoLcdAPI);

#ifndef DISABLE_LIGHTSPEED
	// Lightspeed
	if (LS_Installed())
		OPTIONS_SET(kOptLightspeedAPI);
#endif

	// check for 16bit mode
	if (!WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &depth, &color))
		OPTIONS_SET(((depth & 0x8000) ? kOptMode16Bit : kOptNone));

}
