#ifndef __PA1_SND_H__
#define __PA1_SND_H__

#include <PalmOS.h>

#define COMPILE_STREAMSND
#define COMPILE_PACE

Err sndCallback(void* UserDataP, SndStreamRef stream, void* bufferP, UInt32 *bufferSizeP);

#endif