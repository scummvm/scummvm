#include "cryolib.h"

soundgroup_t *CLSoundGroup_New(short numSounds, short arg4, short sampleSize, float rate, short mode) {
	soundgroup_t *sg;
	short i;

	sg = (soundgroup_t *)CLMemory_Alloc(sizeof(*sg));
	if (numSounds < CL_MAX_SOUNDS)
		sg->numSounds = numSounds;
	else {
		__libError = -3;
		__osError = 0;
		CLCheckError();
		sg->numSounds = CL_MAX_SOUNDS;
	}
	for (i = 0; i < sg->numSounds; i++) {
		sound_t *sound = CLSoundRaw_New(arg4, rate, sampleSize, mode);
		sg->sound[i] = sound;
		sound->ff_1A = arg4;
	}
	sg->soundIndex = 0;
	sg->playIndex = 0;
	sg->ff_106 = 1;

	return sg;
}

void CLSoundGroup_Free(soundgroup_t *sg) {
	short i;
	for (i = 0; i < sg->numSounds; i++)
		CLSoundRaw_Free(sg->sound[i]);
	CLMemory_Free(sg);
}

void CLSoundGroup_Reverse16All(soundgroup_t *sg) {
	short i;
	for (i = 0; i < sg->numSounds; i++)
		sg->sound[i]->reversed = 1;
}

void *CLSoundGroup_GetNextBuffer(soundgroup_t *sg) {
	sound_t *sound = sg->sound[sg->soundIndex];
	if (sg->ff_106)
		while (sound->locked) ;
	return ((char *)(*sound->sndHandle)) + sound->headerLen;
}

short CLSoundGroup_AssignDatas(soundgroup_t *sg, void *buffer, int length, short isSigned) {
	sound_t *sound = sg->sound[sg->soundIndex];
	if (sg->ff_106)
		while (sound->locked) ;
	else if (sound->locked)
		return 0;
	sound->buffer = (char *)buffer;
	CLSound_SetLength(sound, length);
	sound->length = length;
//	if(sound->reversed && sound->sampleSize == 16)
//		ReverseBlock16(buffer, length);
//	if(isSigned)
//		CLSound_Signed2NonSigned(buffer, length);
	if (sg->soundIndex == sg->numSounds - 1)
		sg->soundIndex = 0;
	else
		sg->soundIndex++;

	return 1;
}

short CLSoundGroup_SetDatas(soundgroup_t *sg, void *data, int length, short isSigned) {
	void *buffer;
	sound_t *sound = sg->sound[sg->soundIndex];
	if (length >= sound->ff_1A) {
		__libError = -10;
		__osError = 0;
		CLCheckError();
	}
	if (sg->ff_106)
		while (sound->locked) ;
	else if (sound->locked)
		return 0;
	buffer = ((char *)(*sound->sndHandle)) + sound->headerLen;
	sound->buffer = (char *)buffer;
	memcpy(buffer, data, length);
	CLSound_SetLength(sound, length);
	sound->length = length;
//	if(sound->reversed && sound->sampleSize == 16)
//		ReverseBlock16(buffer, length);
//	if(isSigned)
//		CLSound_Signed2NonSigned(buffer, length);
	if (sg->soundIndex == sg->numSounds - 1)
		sg->soundIndex = 0;
	else
		sg->soundIndex++;

	return 1;
}

void CLSoundGroup_PlayNextSample(soundgroup_t *sg, soundchannel_t *ch) {
	CLSoundChannel_Play(ch, sg->sound[sg->playIndex]);
	if (sg->playIndex == sg->numSounds - 1)
		sg->playIndex = 0;
	else
		sg->playIndex++;

}
