#include <PalmOS.h>
#include "globals.h"
#include "init_arm.h"

void ARMInit() {
	// init global ARM only
	MemSet(gVars->arm, sizeof(gVars->arm), 0);
	ARM(PNO_COPYRECT	).pnoPtr = _PnoInit(RSC_COPYRECT, &ARM(PNO_COPYRECT).pnoDesc);
	ARM(PNO_COSTUMEPROC3).pnoPtr = _PceInit(RSC_COSTUMEPROC3);
	ARM(PNO_DRAWSTRIP	).pnoPtr = _PceInit(RSC_DRAWSTRIP);
	ARM(PNO_BLIT		).pnoPtr = _PnoInit(RSC_BLIT, &ARM(PNO_BLIT).pnoDesc);
}

void ARMRelease() {
	_PnoFree(&ARM(PNO_BLIT			).pnoDesc, ARM(PNO_BLIT).pnoPtr);
	_PceFree(ARM(PNO_DRAWSTRIP		).pnoPtr);
	_PceFree(ARM(PNO_COSTUMEPROC3	).pnoPtr);
	_PnoFree(&ARM(PNO_COPYRECT		).pnoDesc, ARM(PNO_COPYRECT).pnoPtr);
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