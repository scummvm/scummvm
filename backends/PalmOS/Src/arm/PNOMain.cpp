#include "PACEInterfaceLib.h"
#include "ArmNative.h"

// Linker still looks for ARMlet_Main as entry point, but the
// "ARMlet" name is now officially discouraged.  Compare an
// contrast to "PilotMain" for 68K applications.
#define PNO_Main ARMlet_Main

// ------------------------

extern "C"
unsigned long PNO_Main(
	const void *emulStateP, 
	void *userData68KP, 
	Call68KFuncType *call68KFuncP);

unsigned long PNO_Main(const void *emulStateP, void *userData68KP, Call68KFuncType *call68KFuncP) {
	unsigned long retVal = 0;

#ifdef COMPILE_PACE
	// needed before making any OS calls using the 
	// PACEInterface library
	InitPACEInterface(emulStateP, call68KFuncP);
#endif

#ifdef COMPILE_PA1SND
	ARMPa1SndPtr userData = (ARMPa1SndPtr)userData68KP;
	pcm2adpcm	(	(Int16 *)ReadUnaligned32(&(userData->srcP)),
					(UInt8 *)ReadUnaligned32(&(userData->dstP)),
					ReadUnaligned32(&(userData->length))
				);
#endif

#ifdef COMPILE_STREAMSND
	retVal = (unsigned long)sndCallback;
#endif

#ifdef COMPILE_OWIDELS
	O_WideLandscape(userData68KP);
#endif

#ifdef COMPILE_OWIDEPT
	O_WidePortrait(userData68KP);
#endif

#ifdef COMPILE_OCOPYRECT
	O_CopyRectToScreen(userData68KP);
#endif

	return retVal;
}
