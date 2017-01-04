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
	_curVideoNum = 0;
	_soundStarted = false;
	_pendingSounds = 0;
	_timeDrift = 0.0;
	_nextFrameTime = 0.0;
	_expectedFrameTime = 0.0;
	_rate = 0.0;
	_useSoundSync = false;
	_useSound = false;
	_soundChannel = nullptr;
	_soundGroup = nullptr;
	_soundChannelAdpcm = nullptr;
	_soundGroupAdpcm = nullptr;
	_prevRight = _prevLeft = 0;
	_useAdpcm = false;
	_useMono = false;
	_customChunkHandler = nullptr;
	_preserveColor0 = false;
	_safePalette = false;

	for (int i = 0; i < 256; i++)
		decompTable[i] = 0;
}

// Original name: CLHNM_New
hnm_t *HnmPlayer::resetInternals() {

	hnm_t *hnm = (hnm_t *)malloc(sizeof(*hnm));

	hnm->_frameNum = 0;
	hnm->_unused04 = 0;
	hnm->_file = nullptr;
	hnm->tmpBuffer[0] = nullptr;
	hnm->tmpBuffer[1] = nullptr;
	hnm->finalBuffer = nullptr;
	hnm->_readBuffer = nullptr;
	hnm->_unused896 = 0;
	hnm->_totalRead = 0;
	for (int i = 0; i < 256; i++) {
		hnm->_palette[i].a = 0;
		hnm->_palette[i].r = 0;
		hnm->_palette[i].g = 0;
		hnm->_palette[i].b = 0;
	}

	return hnm;
}

// Original name: CLHNM_SetFile
void HnmPlayer::setFile(hnm_t *hnm, Common::File *file) {
	hnm->_file = file;
}

// Original name: CLHNM_SetupTimer
void HnmPlayer::setupTimer(float rate) {
	_rate = 100.0 / rate;
}

// Original name: CLHNM_ResetInternalTimer
void HnmPlayer::resetInternalTimer() {
	_timeDrift = 0.0;
	_nextFrameTime = _expectedFrameTime = _vm->_timerTicks;
}

// Original name: CLHNM_Reset
void HnmPlayer::reset(hnm_t *hnm) {
	hnm->_frameNum = 0;
	hnm->_unused04 = 0;
	hnm->_totalRead = 0;
	_soundStarted = false;
	_pendingSounds = 0;
	resetInternalTimer();
}

// Original name: CLHNM_Init
void HnmPlayer::init() {
	_customChunkHandler = nullptr;
	_preserveColor0 = false;
}

// Original name: CLHNM_SetForceZero2Black
void HnmPlayer::setForceZero2Black(bool forceblack) {
	_preserveColor0 = forceblack;
}

// Original name: CLHNM_WaitLoop
void HnmPlayer::waitLoop(hnm_t *hnm) {
	_expectedFrameTime += _rate;
	_nextFrameTime = _expectedFrameTime - _timeDrift;
	if (_useSoundSync && _vm->_timerTicks > 1000.0 + _nextFrameTime)
		_useSound = false;
	while (_vm->_timerTicks < _nextFrameTime) ;  // waste time
	_timeDrift = _vm->_timerTicks - _nextFrameTime;
}

// Original name: CLHNM_WantsSound
void HnmPlayer::wantsSound(bool sound) {
	_useSound = sound;
}

// Original name: CLHNM_SetupSound
void HnmPlayer::setupSound(int16 numSounds, int16 length, int16 sampleSize, float rate, int16 mode) {
	_soundChannel = new SoundChannel(mode);
	_soundGroup = new SoundGroup(_vm, numSounds, length, sampleSize, rate, mode);
	if (sampleSize == 16)
		_soundGroup->reverse16All();
}

// Original name: CLHNM_SetupSoundADPCM
void HnmPlayer::setupSoundADPCM(int16 numSounds, int16 length, int16 sampleSize, float rate, int16 mode) {
	_soundChannelAdpcm = new SoundChannel(mode);
	_soundGroupAdpcm = new SoundGroup(_vm, numSounds, length, sampleSize, rate, mode);
}

// Original name: CLHNM_CloseSound
void HnmPlayer::closeSound() {
	if (_soundChannel) {
		_soundChannel->stop();
		delete(_soundChannel);
		_soundChannel = nullptr;
	}
	if (_soundGroup) {
		delete(_soundGroup);
		_soundGroup = nullptr;
	}
	if (_soundChannelAdpcm) {
		_soundChannelAdpcm->stop();
		delete(_soundChannelAdpcm);
		_soundChannelAdpcm = nullptr;
	}
	if (_soundGroupAdpcm) {
		delete(_soundGroupAdpcm);
		_soundGroupAdpcm = nullptr;
	}
}

// Original name: CLHNM_LoadDecompTable
void HnmPlayer::loadDecompTable(int16 *buffer) {
	for (int16 i = 0; i < 256; i++) {
		int16 e = *buffer++;
		decompTable[i] = LE16(e);
	}
}

// Original name: CLHNM_DecompADPCM
void HnmPlayer::decompADPCM(byte *buffer, int16 *output, int size) {
	int16 l = _prevLeft;
	int16 r = _prevRight;
	size &= ~1;
	while (size--) {
		*output++ = l += decompTable[*buffer++];
		*output++ = r += decompTable[*buffer++];
		if (l > 512 || r > 512)
			error("decompADPCM - Unexpected values");
	}
	_prevLeft = l;
	_prevRight = r;
}

// Original name: CLHNM_ReadHeader
void HnmPlayer::readHeader(hnm_t *hnm) {
	hnm->_header._signature = hnm->_file->readUint32BE();
	hnm->_header._unusedFlag1 = hnm->_file->readByte();
	hnm->_header._unusedFlag2 = hnm->_file->readByte();
	hnm->_header._unusedReserved = hnm->_file->readByte();
	hnm->_header._unusedBpp = hnm->_file->readByte();
	hnm->_header._width = hnm->_file->readUint16LE();
	hnm->_header._height = hnm->_file->readUint16LE();
	hnm->_header._unusedFileSize = hnm->_file->readSint32LE();
	hnm->_header._numbFrame = hnm->_file->readSint32LE();
	hnm->_header._unusedTableOffset = hnm->_file->readSint32LE();
	hnm->_header._unusedSpeed = hnm->_file->readSint16LE();
	hnm->_header._unusedMaxBuffer = hnm->_file->readSint16LE();
	hnm->_header._bufferSize = hnm->_file->readSint32LE();
	hnm->_header._unusedUnknown = hnm->_file->readSint16LE();
	for (int i = 0; i < 14; i++)
		hnm->_header._unusedReserved2[i] = hnm->_file->readSByte();
	for (int i = 0; i < 16; i++)
		hnm->_header._unusedCopyright[i] = hnm->_file->readSByte();

	hnm->_header._bufferSize += 4096; //TODO: checkme
}

// Original name: CLHNM_GetVersion
int16 HnmPlayer::getVersion(hnm_t *hnm) {
	if (hnm->_header._signature == MKTAG('H','N','M','4'))
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
	tryRead(hnm, 4);
	int chunk = *(int *)hnm->_readBuffer;
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
	 //	return;
	byte *out_start = output;
	byte count;
	unsigned int code;
	uint16 offs;
	byte mode;
	byte swap;

	if ((flags & 1) == 0) {
		//HNM4 classic
		int twolinesabove = -(width * 2);
		for (;;) {
			code = READ_LE_UINT32(input) & 0xFFFFFF; //input++;
			count = code & 0x1F;
			if (count) {
				input += 3;
				offs = code >> 9;
				//
				mode = (code >> 5) & 0xF;
				swap = mode >> 3;
				byte *ref = ((mode & 1) ? prev_buffer : curr_buffer) + (output - out_start) + (offs * 2) - 32768;
				int shft1, shft2;
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
				case 0x60: {
					count = *(input++);
					byte color = *(input++);
					while (count--) {
						*(output++) = color;
						*(output++) = color;
					}
					break;
					}
				default:
					return;
				}
			}
		}
	} else {
		assert(0);
		//HNM4 hires
		for (;;) {
			code = READ_LE_UINT32(input) & 0xFFFFFF;
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
	if (hnm->_frameNum == 0) {
		resetInternalTimer();
		_prevLeft = _prevRight = 0;
	}
	if (hnm->_frameNum == hnm->_header._numbFrame)
		return false;

	if (!loadFrame(hnm))
		return false;

	for (;;) {
		int sz = READ_LE_UINT32(hnm->_dataPtr) & 0xFFFFFF;
		hnm->_dataPtr += 4;
		int16 id = READ_LE_UINT16(hnm->_dataPtr);
		hnm->_dataPtr += 2;
		char h6 = *hnm->_dataPtr;
		hnm->_dataPtr += 1;
		char h7 = *hnm->_dataPtr;
		hnm->_dataPtr += 1;
		hnm->_chunkId = id;
		switch (id) {
		case MKTAG16('L', 'P'):
			changePalette(hnm);
			hnm->_dataPtr += sz - 8;
			break;
		case MKTAG16('Z', 'I'):
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
			if (_useAdpcm) {
				if (!_soundStarted) {
					for (int16 i = 0; i < _pendingSounds; i++)
						_soundGroupAdpcm->playNextSample(_soundChannel);
					_soundStarted = true;
				}
			} else if (!_soundStarted) {
				for (int16 i = 0; i < _pendingSounds; i++)
					_soundGroup->playNextSample(_soundChannel);
				_soundStarted = true;
			}

			return true;
		case MKTAG16('U', 'I'):
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

		case MKTAG16('d', 's'):
		case MKTAG16('D', 'S'):
			if (_useSound) {
				if (!h6) {
					int sound_size = sz - 8;
					if (!_useAdpcm) {
						_soundGroup->setDatas(hnm->_dataPtr, sound_size - 2, false);
						if (_soundStarted)
							_soundGroup->playNextSample(_soundChannel);
						else
							_pendingSounds++;
					} else {
						int16 *sound_buffer = (int16 *)_soundGroupAdpcm->getNextBuffer();
						if (!_pendingSounds) {
							const int kDecompTableSize = 256 * sizeof(int16);
							loadDecompTable((int16 *)hnm->_dataPtr);
							decompADPCM(hnm->_dataPtr + kDecompTableSize, sound_buffer, sound_size - kDecompTableSize);
							_soundGroupAdpcm->assignDatas(sound_buffer, (sound_size - kDecompTableSize) * 2, false);
						} else {
							decompADPCM(hnm->_dataPtr, sound_buffer, sound_size);
							_soundGroupAdpcm->assignDatas(sound_buffer, sound_size * 2, false);
						}
						_pendingSounds++;
						if (_soundStarted)
							_soundGroupAdpcm->playNextSample(_soundChannel);
					}
				} else
					error("nextElement - unexpected flag");
			}
			hnm->_dataPtr += sz - 8;
			break;
		default:
			if (_customChunkHandler)
				_customChunkHandler(hnm->_dataPtr, sz - 8, id, h6, h7);
			hnm->_dataPtr += sz - 8;
		}
	}
	return true;
}

// Original name: CLHNM_GetSoundChannel
SoundChannel *HnmPlayer::getSoundChannel() {
	return _soundChannel;
}

// Original name: CLHNM_FlushPreloadBuffer
void HnmPlayer::flushPreloadBuffer(hnm_t *hnm) {
}

// Original name: CLHNM_ChangePalette
void HnmPlayer::changePalette(hnm_t *hnm) {
	CLPalette_GetLastPalette(hnm->_palette);
	byte *pal = hnm->_dataPtr;
	if (*(uint16 *)pal == 0xFFFF)
		return;

	int16 mincolor = 255;
	int16 maxcolor = 0;
	do {
		uint16 fst = *pal++;
		uint16 cnt = *pal++;
		if (cnt == 0)
			cnt = 256;
		debug("hnm: setting palette, fst = %d, cnt = %d, last = %d", fst, cnt, fst + cnt - 1);
		assert(fst + cnt <= 256);
		if (mincolor > fst)
			mincolor = fst;
		if (maxcolor < fst + cnt)
			maxcolor = fst + cnt;
		color_t *color = hnm->_palette + fst;
		if (_safePalette) {
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

// Original name: CLHNM_Done
void HnmPlayer::done() {
}

// Original name: CLHNM_Dispose
void HnmPlayer::dispose(hnm_t *hnm) {
	free(hnm);
}

// Original name: CLHNM_CanLoop
void HnmPlayer::canLoop(hnm_t *hnm, bool canLoop) {
	hnm->_canLoop = canLoop;
}

// Original name: CLHNM_SoundInADPCM
void HnmPlayer::soundInADPCM(bool isAdpcm) {
	_useAdpcm = isAdpcm;
}

// Original name: CLHNM_SoundMono
void HnmPlayer::soundMono(bool isMono) {
	_useMono = isMono;
}

}   // namespace Cryo

