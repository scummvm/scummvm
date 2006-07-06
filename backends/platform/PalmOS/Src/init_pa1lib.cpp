#include <PalmOS.h>
#include <Sonyclie.h>

#include "pa1lib.h"
#include "init_pa1lib.h"

void *sndStateOnFuncP = NULL;
void *sndStateOffFuncP = NULL;

#define SndStateOn(a,b,c)	if (sndStateOnFuncP)((sndStateOnType)(sndStateOnFuncP))(a, b, c);
#define SndStateOff(a)		if (sndStateOffFuncP)((sndStateOffType)(sndStateOffFuncP))(a);

void Pa1libInit(UInt16 vol) {
	Pa1Lib_Open();

	// Doesn't work on T4xx and T6xx series ?
	FtrGet(sonySysFtrCreatorSystem, sonySysFtrNumSystemAOutSndStateOnHandlerP, (UInt32*) &sndStateOnFuncP);
	FtrGet(sonySysFtrCreatorSystem, sonySysFtrNumSystemAOutSndStateOffHandlerP, (UInt32*) &sndStateOffFuncP);

	SndStateOn(aOutSndKindSp, vol, vol);
	SndStateOn(aOutSndKindHp, vol, vol);

	Pa1Lib_devHpVolume(vol, vol);
	Pa1Lib_devSpVolume(vol);
}

void Pa1libRelease() {
	SndStateOff(aOutSndKindSp);
	SndStateOff(aOutSndKindHp);

	Pa1Lib_Close();
}
