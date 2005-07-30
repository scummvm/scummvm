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
	PnoProc *func[] = {
#if defined(COMPILE_COMMON)
			OSystem_CopyRectToScreen,
			OSystem_updateScreen_widePortrait,
			OSystem_updateScreen_wideLandscape,
//			MemoryStream_ReadBuffer

#elif defined(COMPILE_QUEEN)
			Display_blit

#elif defined(COMPILE_SCUMM)
			Gdi_drawStripToScreen,
			CostumeRenderer_proc3

#elif defined(COMPILE_SWORD1)
			Screen_draw,
			Screen_drawSprite,
			Screen_fastShrink,
			Screen_renderParallax,
			Screen_decompressTony,
			Screen_decompressRLE7,
			Screen_decompressRLE0

#endif
	};

	// needed before making any OS calls using the
	// PACEInterface library
	InitPACEInterface(emulStateP, call68KFuncP);

	unsigned long retVal = 0;
	PnoType *pno = (PnoType *)ByteSwap32(userData68KP);
	UInt32 funcID = ReadUnaligned32(&pno->funcID);
	void *dataP = (void *)ReadUnaligned32(&pno->dataP);
/*
char buf[100];
StrIToA(buf,funcID);
WinDrawChars(buf,StrLen(buf),30,0);
*/
	retVal = func[funcID](dataP);

	return (retVal);
}
