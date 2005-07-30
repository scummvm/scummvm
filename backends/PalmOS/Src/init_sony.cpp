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
			//SysLibRemove(gVars->HRrefNum);	// never call this !!
	}
}
