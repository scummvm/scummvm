#ifndef PNOCOMMON_H
#define PNOCOMMON_H

#define COMPILE_COMMON

unsigned long OSystem_CopyRectToScreen(void *userData68KP);
unsigned long MemoryStream_ReadBuffer(void *userData68KP);
unsigned long OSystem_updateScreen_widePortrait(void *userData68KP);
unsigned long OSystem_updateScreen_wideLandscape(void *userData68KP);

#endif
