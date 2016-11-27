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

#include "cryo/cryo.h"
#include "cryo/video.h"

namespace Cryo {
HnmPlayer::HnmPlayer(CryoEngine *vm) : _vm(vm) {
	curVideoNum = 0;
	sound_started = false;
	pending_sounds = 0;
	time_drift = 0.0;
	next_frame_time = 0.0;
	expected_frame_time = 0.0;
	hnm_rate = 0.0;
	use_sound_sync = false;
	use_sound = false;
	soundChannel = nullptr;
	soundGroup = nullptr;
	soundChannel_adpcm = nullptr;
	soundGroup_adpcm = nullptr;
	pred_r = pred_l = 0;
	use_adpcm = false;
	custom_chunk_handler = nullptr;
	preserve_color0 = false;

	for (int i = 0; i < 256; i++)
		decomp_table[i] = 0;
}

// Original name: CLHNM_SetupTimer
void HnmPlayer::setupTimer(float rate) {
	hnm_rate = 100.0 / rate;
}

// Original name: CLHNM_ResetInternalTimer
void HnmPlayer::resetInternalTimer() {
	time_drift = 0.0;
	next_frame_time = expected_frame_time = TimerTicks;
}

// Original name: CLHNM_Reset
void HnmPlayer::reset(hnm_t *hnm) {
	hnm->_frameNum = 0;
	hnm->ff_4 = 0;
	hnm->_totalRead = 0;
	sound_started = false;
	pending_sounds = 0;
	resetInternalTimer();
}

// Original name: CLHNM_Init
void HnmPlayer::init() {
	custom_chunk_handler = nullptr;
	preserve_color0 = false;
}

// Original name: CLHNM_SetForceZero2Black
void HnmPlayer::setForceZero2Black(bool forceblack) {
	preserve_color0 = forceblack;
}

// Original name: CLHNM_WaitLoop
void HnmPlayer::waitLoop(hnm_t *hnm) {
	expected_frame_time += hnm_rate;
	next_frame_time = expected_frame_time - time_drift;
	if (use_sound_sync && TimerTicks > 1000.0 + next_frame_time)
		use_sound = false;
	while (TimerTicks < next_frame_time) ;  // waste time
	time_drift = TimerTicks - next_frame_time;
}

// Original name: CLHNM_WantsSound
void HnmPlayer::wantsSound(bool sound) {
	use_sound = sound;
}

// Original name: CLHNM_SetupSound
void HnmPlayer::setupSound(int16 numSounds, int16 length, int16 sampleSize, float rate, int16 mode) {
	soundChannel = CLSoundChannel_New(mode);
	soundGroup = CLSoundGroup_New(numSounds, length, sampleSize, rate, mode);
	if (sampleSize == 16)
		CLSoundGroup_Reverse16All(soundGroup);
}

// Original name: CLHNM_SetupSoundADPCM
void HnmPlayer::setupSoundADPCM(int16 numSounds, int16 length, int16 sampleSize, float rate, int16 mode) {
	soundChannel_adpcm = CLSoundChannel_New(mode);
	soundGroup_adpcm = CLSoundGroup_New(numSounds, length, sampleSize, rate, mode);
}

// Original name: CLHNM_SoundInADPCM
void HnmPlayer::soundInADPCM(bool isAdpcm) {
	use_adpcm = isAdpcm;
}

// Original name: CLHNM_CloseSound
void HnmPlayer::closeSound() {
	if (soundChannel) {
		CLSoundChannel_Stop(soundChannel);
		CLSoundChannel_Free(soundChannel);
		soundChannel = nullptr;
	}
	if (soundGroup) {
		CLSoundGroup_Free(soundGroup);
		soundGroup = nullptr;
	}
	if (soundChannel_adpcm) {
		CLSoundChannel_Stop(soundChannel_adpcm);
		CLSoundChannel_Free(soundChannel_adpcm);
		soundChannel = nullptr;
	}
	if (soundGroup_adpcm) {
		CLSoundGroup_Free(soundGroup_adpcm);
		soundGroup = nullptr;
	}
}

// Original name: CLHNM_LoadDecompTable
void HnmPlayer::loadDecompTable(int16 *buffer) {
	for (int16 i = 0; i < 256; i++) {
		int16 e = *buffer++;
		decomp_table[i] = LE16(e);
	}
}

// Original name: CLHNM_DecompADPCM
void HnmPlayer::decompADPCM(byte *buffer, int16 *output, int size) {
	int16 l = pred_l;
	int16 r = pred_r;
	size &= ~1;
	while (size--) {
		*output++ = l += decomp_table[*buffer++];
		*output++ = r += decomp_table[*buffer++];
		if (l > 512 || r > 512)
			error("CLHNM_DecompADPCM - Unexpected values");
	}
	pred_l = l;
	pred_r = r;
}

// Original name: CLHNM_NextElement
bool HnmPlayer::nextElement(hnm_t *hnm) {
	int sz;
	int16 id;
	char h6, h7;
	int16 i;
	if (hnm->_frameNum == 0) {
		resetInternalTimer();
		pred_l = pred_r = 0;
	}
	if (hnm->_frameNum == hnm->_header._numbFrame)
		return false;

	if (!CLHNM_LoadFrame(hnm))
		return false;

	for (;;) {
		sz = PLE32(hnm->_dataPtr) & 0xFFFFFF;
		hnm->_dataPtr += 4;
		id = *(int16 *)hnm->_dataPtr;
		hnm->_dataPtr += 2;
		h6 = *hnm->_dataPtr;
		hnm->_dataPtr += 1;
		h7 = *hnm->_dataPtr;
		hnm->_dataPtr += 1;
		hnm->_chunkId = id;
		switch (id) {
		case BE16('PL'):
			CLHNM_ChangePalette(hnm);
			hnm->_dataPtr += sz - 8;
			break;
		case BE16('IZ'):
			hnm->_frameNum++;
			CLHNM_SelectBuffers(hnm);
			CLHNM_DecompLempelZiv(hnm->_dataPtr + 4, hnm->_newFrameBuffer);
			switch (hnm->_header._width) {
				//			case 320: CLBlitter_RawCopy320ASM(hnm->new_frame_buffer, hnm->old_frame_buffer, hnm->header.height); break;
				//			case 480: CLBlitter_RawCopy480ASM(hnm->new_frame_buffer, hnm->old_frame_buffer, hnm->header.height); break;
				//			case 640: CLBlitter_RawCopy640ASM(hnm->new_frame_buffer, hnm->old_frame_buffer, hnm->header.height); break;
				//			default: memcpy(hnm->old_frame_buffer, hnm->new_frame_buffer, hnm->header.width * hnm->header.height);
		default:
			memcpy(hnm->_oldFrameBuffer, hnm->_newFrameBuffer, hnm->_header._bufferSize);  //TODO strange buffer size here
			}
			if (!(h6 & 1))
				CLHNM_Desentrelace(hnm);
			else {
				//				if(hnm->header.width == 640)
				//					CLBlitter_RawCopy640(hnm->new_frame_buffer, hnm->final_buffer, hnm->header.height);
				//				else
				memcpy(hnm->finalBuffer, hnm->_newFrameBuffer, hnm->_header._height);   //TODO: wrong size?
			}
			if (use_adpcm) {
				if (!sound_started) {
					for (i = 0; i < pending_sounds; i++)
						CLSoundGroup_PlayNextSample(soundGroup_adpcm, soundChannel);
					sound_started = true;
				}
			} else if (!sound_started) {
				for (i = 0; i < pending_sounds; i++)
					CLSoundGroup_PlayNextSample(soundGroup, soundChannel);
				sound_started = true;
			}

			return true;
		case BE16('IU'):
			hnm->_frameNum++;
			CLHNM_SelectBuffers(hnm);
			CLHNM_DecompUBA(hnm->_newFrameBuffer, hnm->_newFrameBuffer, hnm->_oldFrameBuffer, hnm->_dataPtr, hnm->_header._width, h6);
			if (!(h6 & 1))
				CLHNM_Desentrelace(hnm);
			else {
				//				if(hnm->header.width == 640)
				//					CLBlitter_RawCopy640(hnm->new_frame_buffer, hnm->final_buffer, hnm->header.height);
				//				else
				memcpy(hnm->finalBuffer, hnm->_newFrameBuffer, hnm->_header._width * hnm->_header._height);
			}
			return true;

		case BE16('sd'):
		case BE16('SD'):
			if (use_sound) {
				if (!h6) {
					int sound_size = sz - 8;
					if (!use_adpcm) {
						CLSoundGroup_SetDatas(soundGroup, hnm->_dataPtr, sound_size - 2, false);
						if (sound_started)
							CLSoundGroup_PlayNextSample(soundGroup, soundChannel);
						else
							pending_sounds++;
					} else {
						int16 *sound_buffer = (int16 *)CLSoundGroup_GetNextBuffer(soundGroup_adpcm);
						if (!pending_sounds) {
							const int kDecompTableSize = 256 * sizeof(int16);
							loadDecompTable((int16 *)hnm->_dataPtr);
							decompADPCM(hnm->_dataPtr + kDecompTableSize, sound_buffer, sound_size - kDecompTableSize);
							CLSoundGroup_AssignDatas(soundGroup_adpcm, sound_buffer, (sound_size - kDecompTableSize) * 2, false);
						} else {
							decompADPCM(hnm->_dataPtr, sound_buffer, sound_size);
							CLSoundGroup_AssignDatas(soundGroup_adpcm, sound_buffer, sound_size * 2, false);
						}
						pending_sounds++;
						if (sound_started)
							CLSoundGroup_PlayNextSample(soundGroup_adpcm, soundChannel);
					}
				} else
					error("CLHNM_NextElement - unexpected flag");
			}
			hnm->_dataPtr += sz - 8;
			break;
		default:
			if (custom_chunk_handler)
				custom_chunk_handler(hnm->_dataPtr, sz - 8, id, h6, h7);
			hnm->_dataPtr += sz - 8;
		}
	}
	return true;
}

// Original name: CLHNM_GetSoundChannel
soundchannel_t *HnmPlayer::getSoundChannel() {
	return soundChannel;
}

// Original name: CLHNM_FlushPreloadBuffer
void HnmPlayer::flushPreloadBuffer(hnm_t *hnm) {
}

}   // namespace Cryo

