#include <PalmOS.h>

#include "globals.h"
#include "init_palmos.h"

static UInt16 autoOffDelay;

void PalmInit(UInt8 init) {
	// set screen depth
	UInt32 depth = 8;
	WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);

	if (init & INIT_AUTOOFF) {
		autoOffDelay = SysSetAutoOffTime(0);
		EvtResetAutoOffTimer();
	}

}

void PalmRelease(UInt8 init) {
	SysSetAutoOffTime(autoOffDelay);
	EvtResetAutoOffTimer();
}

Err PalmHRInit(UInt32 depth) {
	Err e;
	UInt32 width = 320;
	UInt32 height = 320;
	Boolean color = true;

	e = WinScreenMode (winScreenModeSet, &width, &height, &depth, &color);

	if (!e) {
		UInt32 attr;
		WinScreenGetAttribute(winScreenDensity, &attr);
		e = (attr != kDensityDouble);
	}

	return e;
}

void PalmHRRelease() {
	// should i do something here ?
}