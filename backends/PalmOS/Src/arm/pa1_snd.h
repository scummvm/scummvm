#ifndef __PA1_SND_H__
#define __PA1_SND_H__

#include <PalmOS.h>

#define COMPILE_PA1SND

void pcm2adpcm(Int16 *src, UInt8 *dst, UInt32 length);

#endif