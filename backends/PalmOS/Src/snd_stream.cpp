#ifndef COMPILE_STREAMSND
#	include "stdafx.h"
#	include "palm.h"
#else
#	include "ARMNative.h"
#	include <endianutils.h>
#endif

#ifndef ByteSwap32
#define ByteSwap32(x)	x
#else
#define READ_LE_UINT32(x) *x
#endif

Err sndCallback(void* UserDataP, SndStreamRef stream, void* bufferP, UInt32 *bufferSizeP) {
	SoundDataType *snd = (SoundDataType *)UserDataP;
	UInt32 size = *bufferSizeP;

#ifdef COMPILE_STREAMSND
	// endian
	snd->set	= ByteSwap32(snd->set);
	snd->size	= ByteSwap32(snd->size);
#endif

	if (snd->set && snd->size) {
		UInt32 *dst = (UInt32 *)bufferP;
		UInt32 *src = (UInt32 *)ByteSwap32(snd->dataP);

		size = (snd->size / 16);
		while (size--) {
			*dst++ = READ_LE_UINT32(src++);
			*dst++ = READ_LE_UINT32(src++);
			*dst++ = READ_LE_UINT32(src++);
			*dst++ = READ_LE_UINT32(src++);
		}
		snd->set = false;	

	} else {
		snd->size = size;
		MemSet(bufferP, size, 0);
	}

#ifdef COMPILE_STREAMSND
	// endian
	snd->set	= ByteSwap32(snd->set);
	snd->size	= ByteSwap32(snd->size);
#endif

	return errNone;
}