#include <PalmOS.h>
#include "extend.h"

#ifndef DISABLE_TAPWAVE

#include "tapwave.h"
#include "i_zodiac.h"

// _twBmpV3 = offscreen bitmap, must be set before this call
Err ZodiacInit(void **ptrP, Int32 w, Int32 h) {
	Err e;

	TwGfxSurfaceInfoType surface = {
		sizeof(TwGfxSurfaceInfoType),
		w, h, w * 2,
		twGfxLocationAcceleratorMemory,
		twGfxPixelFormatRGB565_LE
	};

	e = SysSetOrientation(sysOrientationLandscape);
	e = TwGfxOpen((TwGfxType **)&_twGfxLib, NULL);
	e = TwGfxAllocSurface(	(TwGfxType *)_twGfxLib,
							(TwGfxSurfaceType **)&_twSrc,
							&surface);

	e = TwGfxGetPalmDisplaySurface(	(TwGfxType *)_twGfxLib,
									(TwGfxSurfaceType **)&_twDst);

	return e;
}

Err ZodiacRelease(void **ptrP) {
	Err e = errNone;

	TwGfxFreeSurface((TwGfxSurfaceType *)_twSrc);
	TwGfxClose((TwGfxType *)_twGfxLib);

	return e;
}

#endif
