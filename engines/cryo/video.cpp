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
	safe_palette = false;

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
			error("decompADPCM - Unexpected values");
	}
	pred_l = l;
	pred_r = r;
}

// Original name: CLHNM_ReadHeader
void HnmPlayer::readHeader(hnm_t *hnm) {
	int32 size = sizeof(hnm->_header);
	hnm->_file->read(&hnm->_header, size);

	hnm->_header._width = LE16(hnm->_header._width);
	hnm->_header._height = LE16(hnm->_header._height);
	hnm->_header._unusedFileSize = LE32(hnm->_header._unusedFileSize);
	hnm->_header._numbFrame = LE32(hnm->_header._numbFrame);
	hnm->_header._unusedTableOffset = LE32(hnm->_header._unusedTableOffset);
	hnm->_header._unusedSpeed = LE16(hnm->_header._unusedSpeed);
	hnm->_header._unusedMaxBuffer = LE16(hnm->_header._unusedMaxBuffer);
	hnm->_header._bufferSize = LE32(hnm->_header._bufferSize);
	hnm->_header._unusedUnknown = LE16(hnm->_header._unusedUnknown);
	hnm->_header._bufferSize += 4096; //TODO: checkme
}

// Original name: CLHNM_GetVersion
int16 HnmPlayer::getVersion(hnm_t *hnm) {
	if (hnm->_header._signature == BE32('HNM4'))
		return 4;
	return -1;
}

// Original name: CLHNM_AllocMemory
void HnmPlayer::allocMemory(hnm_t *hnm) {
	hnm->tmpBuffer[0] = (byte *)malloc(hnm->_header._bufferSize + 2);

	if (!hnm->tmpBuffer[0])
		return;

	hnm->tmpBuffer[1] = (byte *)malloc(hnm->_header._bufferSize + 2);

	if (!hnm->tmpBuffer[1]) {
		free(hnm->tmpBuffer[0]);
		hnm->tmpBuffer[0] = nullptr;
		return;
	}

	hnm->_readBuffer = (byte *)malloc(hnm->_header._bufferSize + 2);
	if (!hnm->_readBuffer) {
		free(hnm->tmpBuffer[0]);
		hnm->tmpBuffer[0] = nullptr;
		free(hnm->tmpBuffer[1]);
		hnm->tmpBuffer[1] = nullptr;
	}
}

// Original name: CLHNM_DeallocMemory
void HnmPlayer::deallocMemory(hnm_t *hnm) {
	free(hnm->tmpBuffer[0]);
	free(hnm->tmpBuffer[1]);
	free(hnm->_readBuffer);

	hnm->tmpBuffer[0] = nullptr;
	hnm->tmpBuffer[1] = nullptr;
	hnm->_readBuffer = nullptr;
}

// Original name: CLHNM_SetFinalBuffer
void HnmPlayer::setFinalBuffer(hnm_t *hnm, byte *buffer) {
	hnm->finalBuffer = buffer;
}

// Original name: CLHNM_GetFrameNum
int HnmPlayer::getFrameNum(hnm_t *hnm) {
	return hnm->_frameNum;
}

// Original name: CLHNM_TryRead
void HnmPlayer::tryRead(hnm_t *hnm, int size) {
	hnm->_file->read(hnm->_readBuffer, size);
}

// Original name: CLHNM_LoadFrame
bool HnmPlayer::loadFrame(hnm_t *hnm) {
	int chunk;
	tryRead(hnm, 4);
	chunk = *(int *)hnm->_readBuffer;
	chunk = LE32(chunk);
	chunk &= 0xFFFFFF;  // upper bit - keyframe mark?
	if (!chunk)
		return false;

	if (chunk - 4 > hnm->_header._bufferSize)
		error("loadFrame - Chunk size");

	tryRead(hnm, chunk - 4);
	hnm->_dataPtr = hnm->_readBuffer;
	hnm->_totalRead += chunk;
	return true;
}

// Original name CLHNM_DecompLempelZiv
void HnmPlayer::decompLempelZiv(byte *buffer, byte *output) {
	byte *inp = buffer;
	byte *out = output;

	unsigned int queue = 0;
	int qpos = -1;

	//TODO: fix for BE
#define GetBit() ( 1 & ( (qpos >= 0) ? (queue >> qpos--) : (queue = *(unsigned int*)((inp += 4) - 4)) >> ((qpos = 30) + 1) ) )

	for (;;) {
		if (GetBit()) {
			*out++ = *inp++;
		} else {
			int l, o;
			if (GetBit()) {
				l = *inp & 7;
				o = *(uint16 *)inp >> 3;
				inp += 2;
				o -= 8192;
				if (!l)
					l = *inp++;
				if (!l)
					break;
			} else {
				l = GetBit() * 2 + GetBit();
				o = *(inp++) - 256;
			}
			l += 2;
			while (l--) {
				*out = *(out + o);
				out++;
			}
		}
	}

#undef GetBit

	return;
}

// Original name: CLHNM_Desentrelace320
void HnmPlayer::desentrelace320(byte *frame_buffer, byte *final_buffer, uint16 height) {
	unsigned int *input = (unsigned int *)frame_buffer;
	unsigned int *line0 = (unsigned int *)final_buffer;
	unsigned int *line1 = (unsigned int *)(final_buffer + 320);
	int count = (height) / 2;
	while (count--) {
		int16 i;
		for (i = 0; i < 320 / 4; i++) {
			unsigned int p0 = *input++;
			unsigned int p4 = *input++;
#if 0
			*line0++ = ((p4 & 0xFF00) >> 8) | ((p4 & 0xFF000000) >> 16) | ((p0 & 0xFF00) << 8) | (p0 & 0xFF000000);
			//			*line0++ = (p0 & 0xFF000000) | ((p0 & 0xFF00) << 8) | ((p4 & 0xFF000000) >> 16) | ((p4 & 0xFF00) >> 8);
			*line1++ = ((p0 & 0xFF0000) << 8) | ((p0 & 0xFF) << 16) | ((p4 & 0xFF0000) >> 8) | (p4 & 0xFF);
#else
			*line0++ = (p0 & 0xFF) | ((p0 & 0xFF0000) >> 8) | ((p4 & 0xFF) << 16) | ((p4 & 0xFF0000) << 8);
			*line1++ = ((p0 & 0xFF00) >> 8) | ((p0 & 0xFF000000) >> 16) | ((p4 & 0xFF00) << 8) | (p4 & 0xFF000000);
#endif
		}
		line0 += 320 / 4;
		line1 += 320 / 4;
	}
}

// Original name: CLHNM_Desentrelace
void HnmPlayer::desentrelace(hnm_t *hnm) {
	switch (hnm->_header._width) {
	case 320:
		desentrelace320(hnm->_newFrameBuffer, hnm->finalBuffer, hnm->_header._height);
		break;
		//	case 480:
		//		CLHNM_Desentrelace480(hnm->new_frame_buffer, hnm->final_buffer, hnm->header.height);
		//		break;
	default:
		error("desentrelace - Unexpected width");
	}
}

// Original name: CLHNM_DecompUBA
void HnmPlayer::decompUBA(byte *output, byte *curr_buffer, byte *prev_buffer, byte *input, int width, char flags) {
	unsigned int code;
	byte mode, count, color;
	uint16 offs;
	byte *ref;
	byte *out_start = output;
	byte swap;
	int shft1, shft2;
	 //	return;
	if ((flags & 1) == 0) {
		//HNM4 classic
		int twolinesabove = -(width * 2);
		for (;;) {
			code = PLE32(input) & 0xFFFFFF; //input++;
			count = code & 0x1F;
			if (count) {
				input += 3;
				mode = (code >> 5) & 0xF;
				offs = code >> 9;
				//
				swap = mode >> 3;
				ref = ((mode & 1) ? prev_buffer : curr_buffer) + (output - out_start) + (offs * 2) - 32768;
				if (mode & 2) {
					// ref += twolinesabove;
					shft1 = twolinesabove + 1;
					shft2 = 0;
					//swap ^= 1;
				} else {
					shft1 = 0;
					shft2 = 1;
				}
				while (count--) {
					byte b0 = ref[shft1];
					byte b1 = ref[shft2];
					output[swap] = b0;
					output[swap ^ 1] = b1;
					output += 2;
					ref += (mode & 4) ? -2 : 2;
				}
			} else {
				input++;
				mode = code & 0xFF; // bits 0..4 are zero
				switch (mode) {
				case 0:
					*(output++) = *(input++);
					*(output++) = *(input++);
					break;
				case 0x20:
					output += 2 * *(input++);
					break;
				case 0x40:
					output += 2 * (code >> 8);
					input += 2;
					break;
				case 0x60:
					count = *(input++);
					color = *(input++);
					while (count--) {
						*(output++) = color;
						*(output++) = color;
					}
					break;
				default:
					return;
				}
			}
		}
	} else {
		assert(0);
		//HNM4 hires
		for (;;) {
			code = PLE32(input) & 0xFFFFFF;
			input++;
			count = code & 0x3F;
			if (count) {
				mode = (code >> 5) & 0xF;
				offs = code >> 9;
				//
			} else {
				mode = code & 0xFF; // bits 0..5 are zero
				switch (mode) {
				case 0x00:
					output += *input++;
					break;
				case 0x40:
					*output++ = *input++;
					*(output++ + width) = *input++;
					break;
				case 0x80:
					output += width;
					break;
				default:
					return;
				}
			}
		}
	}
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

	if (!loadFrame(hnm))
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
			changePalette(hnm);
			hnm->_dataPtr += sz - 8;
			break;
		case BE16('IZ'):
			hnm->_frameNum++;
			selectBuffers(hnm);
			decompLempelZiv(hnm->_dataPtr + 4, hnm->_newFrameBuffer);
			switch (hnm->_header._width) {
				//			case 320: CLBlitter_RawCopy320ASM(hnm->new_frame_buffer, hnm->old_frame_buffer, hnm->header.height); break;
				//			case 480: CLBlitter_RawCopy480ASM(hnm->new_frame_buffer, hnm->old_frame_buffer, hnm->header.height); break;
				//			case 640: CLBlitter_RawCopy640ASM(hnm->new_frame_buffer, hnm->old_frame_buffer, hnm->header.height); break;
				//			default: memcpy(hnm->old_frame_buffer, hnm->new_frame_buffer, hnm->header.width * hnm->header.height);
			default:
				memcpy(hnm->_oldFrameBuffer, hnm->_newFrameBuffer, hnm->_header._bufferSize);  //TODO strange buffer size here
			}
			if (!(h6 & 1))
				desentrelace(hnm);
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
			selectBuffers(hnm);
			decompUBA(hnm->_newFrameBuffer, hnm->_newFrameBuffer, hnm->_oldFrameBuffer, hnm->_dataPtr, hnm->_header._width, h6);
			if (!(h6 & 1))
				desentrelace(hnm);
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
					error("nextElement - unexpected flag");
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

// Original name: CLHNM_ChangePalette
void HnmPlayer::changePalette(hnm_t *hnm) {
	int16 mincolor, maxcolor;
	uint16 fst, cnt;
	byte *pal;
	color_t *color;
	CLPalette_GetLastPalette(hnm->_palette);
	pal = hnm->_dataPtr;
	if (*(uint16 *)pal == 0xFFFF)
		return;
	mincolor = 255;
	maxcolor = 0;
	do {
		fst = *pal++;
		cnt = *pal++;
		if (cnt == 0)
			cnt = 256;
		debug("hnm: setting palette, fst = %d, cnt = %d, last = %d", fst, cnt, fst + cnt - 1);
		assert(fst + cnt <= 256);
		if (mincolor > fst)
			mincolor = fst;
		if (maxcolor < fst + cnt)
			maxcolor = fst + cnt;
		color = hnm->_palette + fst;
		if (safe_palette) {
			while (cnt--) {
				byte r = *pal++;
				byte g = *pal++;
				byte b = *pal++;
				int16 rr = r << 10;
				int16 gg = g << 10;
				int16 bb = b << 10;
				if (color->r != rr || color->g != gg || color->b != bb)
					CLBlitter_OneBlackFlash();
				color->r = rr;
				color->g = gg;
				color->b = bb;
				color++;
			}
		} else {
			while (cnt--) {
				byte r = *pal++;
				byte g = *pal++;
				byte b = *pal++;
				color->r = r << 10;
				color->g = g << 10;
				color->b = b << 10;
				color++;
			}
		}

	} while (*(uint16 *)pal != 0xFFFF);
#if 0
	if (preserve_color0) {
		hnm->palette[0].r = 0;
		hnm->palette[0].g = 0;
		hnm->palette[0].b = 0;
	}
#endif
	//	CLBlitter_Send2ScreenNextCopy(hnm->palette, mincolor, maxcolor - mincolor);
	CLBlitter_Send2ScreenNextCopy(hnm->_palette, 0, 256);
}

// Original name: CLHNM_SelectBuffers
void HnmPlayer::selectBuffers(hnm_t *hnm) {
	if (hnm->_frameNum % 2) {
		hnm->_newFrameBuffer = hnm->tmpBuffer[1];
		hnm->_oldFrameBuffer = hnm->tmpBuffer[0];
	} else {
		hnm->_newFrameBuffer = hnm->tmpBuffer[0];
		hnm->_oldFrameBuffer = hnm->tmpBuffer[1];
	}
}

}   // namespace Cryo

