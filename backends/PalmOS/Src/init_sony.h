#ifndef INIT_SONY_H
#define INIT_SONY_H

UInt16 SilkInit(UInt32 *retVersion);
void SilkRelease(UInt16 slkRefNum);

UInt16 SonyHRInit(UInt32 depth);
void SonyHRRelease(UInt16 HRrefNum);

#endif
