/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "cryo/cryolib.h"

namespace Cryo {

soundgroup_t *CLSoundGroup_New(int16 numSounds, int16 arg4, int16 sampleSize, float rate, int16 mode) {
	soundgroup_t *sg;
	int16 i;

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
	int16 i;
	for (i = 0; i < sg->numSounds; i++)
		CLSoundRaw_Free(sg->sound[i]);
	CLMemory_Free(sg);
}

void CLSoundGroup_Reverse16All(soundgroup_t *sg) {
	int16 i;
	for (i = 0; i < sg->numSounds; i++)
		sg->sound[i]->reversed = 1;
}

void *CLSoundGroup_GetNextBuffer(soundgroup_t *sg) {
	sound_t *sound = sg->sound[sg->soundIndex];
	if (sg->ff_106)
		while (sound->locked) ;
	return ((char *)(*sound->sndHandle)) + sound->headerLen;
}

int16 CLSoundGroup_AssignDatas(soundgroup_t *sg, void *buffer, int length, int16 isSigned) {
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

int16 CLSoundGroup_SetDatas(soundgroup_t *sg, void *data, int length, int16 isSigned) {
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

} // End of namespace Cryo
