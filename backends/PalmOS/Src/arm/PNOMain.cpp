#include "PACEInterfaceLib.h"
#include "ArmNative.h"

// Linker still looks for ARMlet_Main as entry point, but the
// "ARMlet" name is now officially discouraged.  Compare an
// contrast to "PilotMain" for 68K applications.
#define PNO_Main ARMlet_Main
// entry point
extern "C"
unsigned long PNO_Main(const void *emulStateP, void *userData68KP, Call68KFuncType *call68KFuncP);

#ifndef WIN32

#pragma thumb off
asm void * __ARMlet_Take_Func_Addr__(void *f)
{
    sub     r0, r0, r10         //  0 convert pointer to zero-based address
    ldr     r12, [pc, #8]       //  4 load zero-based address of this routine plus offset into r12
    sub     r12, pc, r12        //  8 compute start of PNO by subtracting this from PC
    add     r0, r0, r12         // 12 add PNO start to function pointer
    bx      lr                  // 16 return to caller
    dcd     __ARMlet_Take_Func_Addr__ + 16    // 20
}
#pragma thumb reset

#else
#define	__ARMlet_Take_Func_Addr__(x)	x
#endif

unsigned long PNO_Main(const void *emulStateP, void *userData68KP, Call68KFuncType *call68KFuncP) {
/*	const PnoProc call[] = {
		(PnoProc)__ARMlet_Take_Func_Addr__(OSystem_PALMOS_update_screen__wide_portrait),
		(PnoProc)__ARMlet_Take_Func_Addr__(OSystem_PALMOS_update_screen__wide_landscape),
		//OSystem_PALMOS_copy_rect
	};
*/
#ifndef WIN32
	// needed before making any OS calls using the 
	// PACEInterface library
	InitPACEInterface(emulStateP, call68KFuncP);
#else
	global.call68KFuncP = call68KFuncP;
	global.emulStateP = emulStateP;
	global.userData68KP = userData68KP;
#endif

	UInt32 run = ByteSwap32(*(UInt32 *)userData68KP);

	switch (run) {
		case 0:
			OSystem_PALMOS_update_screen__wide_portrait(userData68KP);
			break;
		case 1:
			OSystem_PALMOS_update_screen__wide_landscape(userData68KP);
			break;
	}

	return 0;
//	return call[run](userData68KP);
}
