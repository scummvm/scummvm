/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
 
#include <PalmOS.h>
#include <string.h>

#include "extend.h"
#include "globals.h"
#include "starterrsc.h"

#include "arm/native.h"

const Char *SCUMMVM_SAVEPATH = "/PALM/Programs/ScummVM/Saved/";

void PalmFatalError(const Char *err) {
	WinSetDrawWindow(WinGetDisplayWindow());
	WinPalette(winPaletteSetToDefault,0,0,0);

	// unlock to show the alert box
	if (gVars->screenLocked)
		WinScreenUnlock();
	
	if (OPTIONS_TST(kOptModeHiDensity))
		WinSetCoordinateSystem(kCoordinatesStandard);

	WinEraseWindow();
	FrmCustomAlert(FrmFatalErrorAlert, err, 0,0);

	// relock to prevent crash unloading gfx mode
	if (gVars->screenLocked)
		WinScreenLock(winLockDontCare);
}


void DrawStatus(Boolean show) {
	if (OPTIONS_TST(kOptDisableOnScrDisp))
		return;

	UInt8 x,y;
	UInt8 *screen = (UInt8 *)(BmpGetBits(WinGetBitmap(WinGetDisplayWindow())));
	UInt8 color = (show? gVars->indicator.on : gVars->indicator.off);

	if (gVars->screenLocked)
		screen = (screen == gVars->flipping.pageAddr1) ? gVars->flipping.pageAddr2 : gVars->flipping.pageAddr1;

	screen += gVars->screenPitch + 1;
	for(y=0; y < 4; y++) {
		for(x=0; x < 4; x++)
			screen[x] = color;

		screen += gVars->screenPitch;
	}
}

UInt16 StrReplace(Char *ioStr, UInt16 inMaxLen, const Char *inParamStr, const Char *fndParamStr) {
	Char *found;
	Boolean quit = false;
	UInt16 occurences = 0;
	UInt16 newLength;
	UInt16 l1 = StrLen(fndParamStr);
	UInt16 l2 = 0;
	UInt16 l3 = StrLen(ioStr);
	UInt16 next = 0;
		
	if (inParamStr)
		l2 = StrLen(inParamStr); // can be null to know how many occur.
	
	while (((found = StrStr(ioStr+next, fndParamStr)) != NULL) && (!quit)) {
		occurences++;
		newLength = (StrLen(ioStr) - l1 + l2);

		if ( newLength > inMaxLen ) {
			quit = true;
			occurences--;

		} else if (inParamStr) {
			MemMove(found + l2, found + l1, inMaxLen-(found-ioStr+l2));
			MemMove(found, inParamStr, l2);
			next = found - ioStr + l2;

		} else
			next = found - ioStr + l1;
	}
	
	if (inParamStr)
		ioStr[l3 + l2*occurences - l1*occurences] = 0;

	return occurences;
}

MemPtr _PceInit(DmResID resID) {
	MemHandle armH = DmGetResource('ARMC', resID);
	NativeFuncType *armP = (NativeFuncType *)MemHandleLock(armH);

	return armP;
}

UInt32 _PceCall(void *armP, void *userDataP) {
	return PceNativeCall((NativeFuncType *)armP, userDataP);
}

void _PceFree(void *armP) {
	MemHandle armH = MemPtrRecoverHandle(armP);

	MemPtrUnlock(armP);
	DmReleaseResource(armH);
}

MemPtr _PnoInit(DmResID resID, PnoDescriptor *pnoP) {
	MemHandle armH = DmGetResource('ARMC', resID);
	MemPtr armP = MemHandleLock(armH);
	PnoLoad(pnoP, armP);

	return armP;
}

UInt32 _PnoCall(PnoDescriptor *pnoP, void *userDataP) {
	return PnoCall(pnoP, userDataP);
}

void _PnoFree(PnoDescriptor *pnoP, MemPtr armP) {
	MemHandle armH = MemPtrRecoverHandle(armP);

	PnoUnload(pnoP);
	MemPtrUnlock(armP);
	DmReleaseResource(armH);
}
