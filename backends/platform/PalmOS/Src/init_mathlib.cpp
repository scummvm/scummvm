#include <PalmOS.h>

#include "globals.h"
#include "init_mathlib.h"
#include "mathlib.h"

Err MathlibInit() {
	Err e;

	if ((e = SysLibFind(MathLibName, &MathLibRef)))
		if (e == sysErrLibNotFound)									// couldn't find lib
			e = SysLibLoad(LibType, MathLibCreator, &MathLibRef);

	if (e) return sysErrLibNotFound;

	e = MathLibOpen(MathLibRef, MathLibVersion);
	return e;
}

void MathlibRelease() {
	UInt16 useCount;

	if (MathLibRef != sysInvalidRefNum) {
		MathLibClose(MathLibRef, &useCount);

		if (!useCount)
			SysLibRemove(MathLibRef);
	}
}
