#ifndef INIT_STUFFS_H
#define INIT_STUFFS_H

#define kOS5Version		sysMakeROMVersion(5,0,0,sysROMStageRelease,0)

void StuffsGetFeatures();
UInt32 StuffsGetPitch(Coord fullw);
void *StuffsForceVG();
void StuffsReleaseVG(void *vg);

#endif
