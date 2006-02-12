/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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
#ifndef PALMOS_ARM

		UInt32 romVersion;

		e = FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
		if (!e) {
			if (romVersion >= sysMakeROMVersion(4,0,0,sysROMStageRelease,0)) {
				Boolean active = false;
				e = HwrVibrateAttributes(0, kHwrVibrateActive, &active);
				exists = (!e) ? true : exists;
			}
		}
#endif
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
#ifndef PALMOS_ARM
		UInt32 pattern	= active ? 0xFF000000 : 0x00000000;

		HwrVibrateAttributes(1, kHwrVibratePattern, &pattern);
		HwrVibrateAttributes(1, kHwrVibrateActive, &active);
#endif
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
#ifndef PALMOS_ARM
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
#endif
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
#ifndef PALMOS_ARM
		UInt32 pattern	= 0x00000000;
		Boolean active = false;

		HwrVibrateAttributes(1, kHwrVibratePattern, &pattern);
		HwrVibrateAttributes(1, kHwrVibrateActive, &active);
#endif
	}
}