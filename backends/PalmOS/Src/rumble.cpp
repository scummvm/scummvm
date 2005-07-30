#include <PalmOS.h>
#include "globals.h"
#include "vibrate.h"
#include "rumble.h"

#ifndef DISABLE_TAPWAVE
#include "tapwave.h"

TwDeviceHandle hRumble;
#endif

Boolean RumbleExists() {
	Boolean exists = false;
	Err e;

#ifndef DISABLE_TAPWAVE
	if (OPTIONS_TST(kOptDeviceZodiac)) {
		e = TwDeviceOpen(&hRumble, "vibrator0", "w");
		if (!e) {
			exists = true;
			TwDeviceClose(hRumble);
		}

	} else
#endif
	{

		UInt32 romVersion;

		e = FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
		if (!e) {
			if (romVersion >= sysMakeROMVersion(4,0,0,sysROMStageRelease,0)) {
				Boolean active = false;
				e = HwrVibrateAttributes(0, kHwrVibrateActive, &active);
				exists = (!e) ? true : exists;
			}
		}
	}

	return exists;
}

void RumbleRun(Boolean active) {
#ifndef DISABLE_TAPWAVE
	if (OPTIONS_TST(kOptDeviceZodiac)) {
		if (active) {
			UInt8 buf[] = { '\64', '\100' };
			TwDeviceControl(hRumble, 'play', buf, sizeof(buf));
		} else {
			TwDeviceControl(hRumble, 'stop', 0, 0);
		}

	} else
#endif
	{
		UInt32 pattern	= active ? 0xFF000000 : 0x00000000;

		HwrVibrateAttributes(1, kHwrVibratePattern, &pattern);
		HwrVibrateAttributes(1, kHwrVibrateActive, &active);
	}
}

Boolean RumbleInit() {
	Boolean done = false;

#ifndef DISABLE_TAPWAVE
	if (OPTIONS_TST(kOptDeviceZodiac)) {
		Err e = TwDeviceOpen(&hRumble, "vibrator0", "w");
		done = (!e);

	} else
#endif
	{
		if (RumbleExists()) {
			UInt16 cycle	= (SysTicksPerSecond())/2;
			UInt32 pattern	= 0xFF000000;
			UInt16 delay	= 1;
			UInt16 repeat	= 1;

			HwrVibrateAttributes(1, kHwrVibrateRate, &cycle);
			HwrVibrateAttributes(1, kHwrVibratePattern, &pattern);
			HwrVibrateAttributes(1, kHwrVibrateDelay, &delay);
			HwrVibrateAttributes(1, kHwrVibrateRepeatCount, &repeat);

			done = true;
		}
	}

	return done;
}

void RumbleRelease() {
#ifndef DISABLE_TAPWAVE
	if (OPTIONS_TST(kOptDeviceZodiac)) {
		TwDeviceClose(hRumble);

	} else
#endif
	{
		UInt32 pattern	= 0x00000000;
		Boolean active = false;

		HwrVibrateAttributes(1, kHwrVibratePattern, &pattern);
		HwrVibrateAttributes(1, kHwrVibrateActive, &active);
	}
}
