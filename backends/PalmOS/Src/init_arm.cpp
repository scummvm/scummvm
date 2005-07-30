#include <PalmOS.h>
#include "globals.h"
#include "init_arm.h"

static void PnoInit(DmResID resID,PNOInitType *pnoP) {
	// Load and allocate PNO
	MemHandle armH = DmGetResource('ARMC', resID);

	if (armH) {
		MemPtr armP = MemHandleLock(armH);
		PnoLoad(&pnoP->pnoDesc, armP);
		MemPtrUnlock(armP);
		DmReleaseResource(armH);

		// Init PNO
		PnoEntryHeader *header = (PnoEntryHeader *)ALIGN_4BYTE(pnoP->headerBuffer);
		pnoP->alignedHeader = header;

		header->r10Value		= pnoP->pnoDesc.r10Value;
		header->pnoMainAddress	= pnoP->pnoDesc.pnoMainAddress;
	}
}

void ARMInit() {
	// init global ARM only
	MemSet(gVars->arm, sizeof(gVars->arm), 0);
	PnoInit(RSC_ARMCOMMON, &ARM(ARM_COMMON));
	PnoInit(RSC_ARMENGINE, &ARM(ARM_ENGINE));
}

void ARMRelease() {
	PnoUnload(&ARM(ARM_ENGINE).pnoDesc);
	PnoUnload(&ARM(ARM_COMMON).pnoDesc);
}

