#include "cryo/cryolib.h"

namespace Cryo {

sound_t *CLSoundRaw_New(short arg1, float rate, short sampleSize, short mode) {
	sound_t *sound;
	CLBeginCheck;

	sound = (sound_t *)CLMemory_Alloc(sizeof(*sound));
	if (sound) {
		sound->ff_1A = arg1;
		sound->rate = rate;
		sound->sampleSize = sampleSize;
		sound->buffer = 0;
//		sound->sndHandle = CLMemory_AllocHandle(arg1 + 100);
//		if(!sound->sndHandle)
//		{
//			__libError = -1;
//			__osError = MemError();
//			CLCheckError();
//		}
//		else
		{
			CLSound_PrepareSample(sound, mode);
			CLNoError;
		}
	} else {
		__libError = -1;
		__osError = MemError();
		CLCheckError();
	}

	CLEndCheck;
	return sound;
}

void CLSoundRaw_Free(sound_t *sound) {
	while (sound->locked) ;
//	CLMemory_FreeHandle(sound->sndHandle);
	CLMemory_Free(sound);
}

void CLSoundRaw_AssignBuffer(sound_t *sound, void *buffer, int bufferOffs, int length) {
	char *buf;
	CLSound_SetLength(sound, length);
	sound->length = length;
	buf = bufferOffs + (char *)buffer;
//	if(CLSound_GetWantsDesigned())
//		CLSound_Signed2NonSigned(buf, length);
	sound->buffer = buf;
//	if(sound->reversed && sound->sampleSize == 16)
//		ReverseBlock16(buf, length);
}

} // End of namespace Cryo
