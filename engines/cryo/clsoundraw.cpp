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
