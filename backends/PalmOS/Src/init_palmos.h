#ifndef INIT_PALMOS_H
#define INIT_PALMOS_H

void PalmInit(UInt8 init);
void PalmRelease(UInt8 init);

Err PalmHRInit(UInt32 depth);
void PalmHRRelease();

#endif
