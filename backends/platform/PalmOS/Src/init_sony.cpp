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
#include <SonyClie.h>
#include "init_sony.h"

UInt16 SilkInit(UInt32 *retVersion) {
	SonySysFtrSysInfoP sonySysFtrSysInfoP;
	UInt32 version;
	UInt16 slkRefNum;
	Err e;

	// Sony HiRes+
	if (!(e = FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, (UInt32*)&sonySysFtrSysInfoP))) {
		if (sonySysFtrSysInfoP->libr & sonySysFtrSysInfoLibrSilk) {

			if ((e = SysLibFind(sonySysLibNameSilk, &slkRefNum)))
				if (e == sysErrLibNotFound)
					e = SysLibLoad(sonySysFileTSilkLib, sonySysFileCSilkLib, &slkRefNum);

			if (!e) {
				e = FtrGet(sonySysFtrCreator, sonySysFtrNumVskVersion, &version);
				if (e) {
					// v1 = NR
				 	e = SilkLibOpen(slkRefNum);
					if(!e) version = vskVersionNum1;

				} else {
					// v2 = NX/NZ
					// v3 = UX...
				 	e = VskOpen(slkRefNum);
				}
			}
		} else
			e = sysErrLibNotFound;
	}

	if (e) {
		version = 0;
		slkRefNum = sysInvalidRefNum;
	}

	*retVersion = version;
	return slkRefNum;
}

void SilkRelease(UInt16 slkRefNum) {
	if (slkRefNum != sysInvalidRefNum)
		SilkLibClose(slkRefNum);
}

UInt16 SonyHRInit(UInt32 depth) {
	SonySysFtrSysInfoP sonySysFtrSysInfoP;
	Err e;
	UInt16 HRrefNum;

	// test if sonyHR is present
	if (!(e = FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, (UInt32*)&sonySysFtrSysInfoP))) {
		if (sonySysFtrSysInfoP->libr & sonySysFtrSysInfoLibrHR) {		// HR available

			if ((e = SysLibFind(sonySysLibNameHR, &HRrefNum)))
				if (e == sysErrLibNotFound)								// can't find lib
					e = SysLibLoad( 'libr', sonySysFileCHRLib, &HRrefNum);

			// Now we can use HR lib. Executes Open library.
			if (!e) e = HROpen(HRrefNum);
		}
	}

	if (e) HRrefNum = sysInvalidRefNum;

	if (HRrefNum != sysInvalidRefNum) {
		UInt32 width = hrWidth;
		UInt32 height = hrHeight;
		Boolean color = true;

		e = HRWinScreenMode(HRrefNum, winScreenModeSet, &width, &height, &depth, &color);
		// error ? release and return an invalid reference number
		if (e) {
			SonyHRRelease(HRrefNum);
			HRrefNum = sysInvalidRefNum;
		}
	}

	return HRrefNum;
}

void SonyHRRelease(UInt16 HRrefNum) {
	if (HRrefNum != sysInvalidRefNum) {
			HRClose(HRrefNum);
			//SysLibRemove(HRrefNum);	// never call this !!
	}
}

UInt8 SonyScreenSize(UInt16 HRrefNum, Coord *stdw, Coord *stdh, Coord *fullw, Coord *fullh) {
	UInt32 version;
	UInt16 slkRefNum;
	UInt8 mode = 0;

	Coord sw = 160;
	Coord sh = 160;	
	Coord fw = sw;
	Coord fh = sh;

	if (HRrefNum != sysInvalidRefNum) {
		sw = hrWidth;
		sh = hrHeight;
		fw = sw;
		fh = sh;

		slkRefNum = SilkInit(&version);

		if (slkRefNum != sysInvalidRefNum) {
			if (version == vskVersionNum1) {
				SilkLibEnableResize(slkRefNum);
				SilkLibResizeDispWin(slkRefNum, silkResizeMax);
				HRWinGetWindowExtent(HRrefNum, &fw, &fh);
				SilkLibResizeDispWin(slkRefNum, silkResizeNormal);
				SilkLibDisableResize(slkRefNum);
				mode = SONY_PORTRAIT;

			} else {
				VskSetState(slkRefNum, vskStateEnable, (version == vskVersionNum2 ? vskResizeVertically : vskResizeHorizontally));
				VskSetState(slkRefNum, vskStateResize, vskResizeNone);
				HRWinGetWindowExtent(HRrefNum, &fw, &fh);
				VskSetState(slkRefNum, vskStateResize, vskResizeMax);
				VskSetState(slkRefNum, vskStateEnable, vskResizeDisable);
				mode = (version == vskVersionNum3 ? SONY_LANDSCAPE : SONY_PORTRAIT);
			}
			SilkRelease(slkRefNum);
		}
	}

	*stdw = sw;
	*stdh = sh;
	*fullw = fw;
	*fullh = fh;
	
	return mode;
}
