#include "common/stdafx.h"
#include "palm.h"

Err sndCallback(void* UserDataP, SndStreamRef stream, void* bufferP, UInt32 *bufferSizeP) {
	SoundDataType *snd = (SoundDataType *)UserDataP;
	UInt32 size = *bufferSizeP;

	if (snd->set && snd->size) {
		UInt32 *dst = (UInt32 *)bufferP;
		UInt32 *src = (UInt32 *)snd->dataP;

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
		MemSet(bufferP, 128, 0);
		*bufferSizeP = 128;
	}

	return errNone;
}
