#include "PACEInterfaceLib.h"
#include "native.h"

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

#ifdef COMPILE_WIDELANDSCAPE
	OSystem_updateScreen_wideLandscape(userData68KP);
#endif

#ifdef COMPILE_WIDEPORTRAIT
	OSystem_updateScreen_widePortrait(userData68KP);
#endif

#ifdef COMPILE_COPYRECT
	OSystem_CopyRectToScreen(userData68KP);
#endif

#ifdef COMPILE_COSTUMEPROC3
	retVal = CostumeRenderer_proc3(userData68KP);
#endif

#ifdef COMPILE_DRAWSTRIP
	Gdi_drawStripToScreen(userData68KP);
#endif

#ifdef COMPILE_BLIT
	Display_blit(userData68KP);
#endif

	return ByteSwap32(retVal);
}
