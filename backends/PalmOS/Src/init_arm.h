#ifndef INIT_ARM_H
#define INIT_ARM_H

void ARMInit();
void ARMRelease();

// calls
MemPtr	_PceInit(DmResID resID);
UInt32	_PceCall(void *armP, void *userDataP);
void	_PceFree(void *armP);

MemPtr	_PnoInit(DmResID resID, PnoDescriptor *pnoP);
UInt32	_PnoCall(PnoDescriptor *pnoP, void *userDataP);
void	_PnoFree(PnoDescriptor *pnoP, MemPtr armP);

#endif