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
	_soundStarted = false;
	_pendingSounds = 0;
	_timeDrift = 0.0;
	_nextFrameTime = 0.0;
	_expectedFrameTime = 0.0;
	_rate = 0.0;
	_useSoundSync = false;
	_useSound = true;
	_soundChannel = nullptr;
	_prevRight = _prevLeft = 0;
	_useAdpcm = false;
	_customChunkHandler = nullptr;
	_preserveColor0 = false;
	_safePalette = false;

	for (int i = 0; i < 256; i++)
		decompTable[i] = 0;
}

// Original name: CLHNM_New
void HnmPlayer::resetInternals() {
	_frameNum = 0;
	_file = nullptr;
	_tmpBuffer[0] = nullptr;
	_tmpBuffer[1] = nullptr;
	_finalBuffer = nullptr;
	_readBuffer = nullptr;
	for (int i = 0; i < 256; i++) {
		_palette[i].a = 0;
		_palette[i].r = 0;
		_palette[i].g = 0;
		_palette[i].b = 0;
	}
}

// Original name: CLHNM_SetFile
void HnmPlayer::setFile(Common::File *file) {
	_file = file;
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
void HnmPlayer::reset() {
	_frameNum = 0;
	_soundStarted = false;
	_pendingSounds = 0;
	resetInternalTimer();
}

// Original name: CLHNM_Init
void HnmPlayer::init() {
	_customChunkHandler = nullptr;
	_preserveColor0 = false;
	_useSound = true;
}

// Original name: CLHNM_SetForceZero2Black
void HnmPlayer::setForceZero2Black(bool forceblack) {
	_preserveColor0 = forceblack;
}

// Original name: CLHNM_WaitLoop
void HnmPlayer::waitLoop() {
	_expectedFrameTime += _rate;
	_nextFrameTime = _expectedFrameTime - _timeDrift;
	if (_useSoundSync && _vm->_timerTicks > 1000.0 + _nextFrameTime)
		_useSound = false;
	while (_vm->_timerTicks < _nextFrameTime) {} // waste time
	_timeDrift = _vm->_timerTicks - _nextFrameTime;
}

// Original name: CLHNM_WantsSound
void HnmPlayer::wantsSound(bool sound) {
	_useSound = sound;
}

// Original name: CLHNM_SetupSound
void HnmPlayer::setupSound(unsigned int rate, bool stereo, bool is16bits) {
	_soundChannel = new CSoundChannel(_vm->_mixer, rate, stereo, is16bits);
}

// Original name: CLHNM_CloseSound
void HnmPlayer::closeSound() {
	if (_soundChannel) {
		_soundChannel->stop();
		delete(_soundChannel);
		_soundChannel = nullptr;
	}
}

// Original name: CLHNM_LoadDecompTable
void HnmPlayer::loadDecompTable(int16 *buffer) {
	for (int16 i = 0; i < 256; i++) {
		int16 e = *buffer++;
		decompTable[i] = FROM_LE_16(e);
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
void HnmPlayer::readHeader() {
	_header._signature = _file->readUint32BE();
	_file->skip(4);
	_header._width = _file->readUint16LE();
	_header._height = _file->readUint16LE();
	_file->skip(4);
	_header._numbFrame = _file->readSint32LE();
	_file->skip(8);
	_header._bufferSize = _file->readSint32LE();
	_file->skip(32);

	_header._bufferSize += 4096; //TODO: checkme
}

// Original name: CLHNM_GetVersion
int16 HnmPlayer::getVersion() {
	if (_header._signature == MKTAG('H','N','M','4'))
		return 4;
	return -1;
}

// Original name: CLHNM_AllocMemory
void HnmPlayer::allocMemory() {
// TODO: rework this code
	_tmpBuffer[0] = (byte *)malloc(_header._bufferSize + 2);

	if (!_tmpBuffer[0])
		return;

	_tmpBuffer[1] = (byte *)malloc(_header._bufferSize + 2);

	if (!_tmpBuffer[1]) {
		free(_tmpBuffer[0]);
		_tmpBuffer[0] = nullptr;
		return;
	}

	_readBuffer = (byte *)malloc(_header._bufferSize + 2);
	if (!_readBuffer) {
		free(_tmpBuffer[0]);
		_tmpBuffer[0] = nullptr;
		free(_tmpBuffer[1]);
		_tmpBuffer[1] = nullptr;
	}
}

// Original name: CLHNM_DeallocMemory
void HnmPlayer::deallocMemory() {
	free(_tmpBuffer[0]);
	free(_tmpBuffer[1]);
	free(_readBuffer);

	_tmpBuffer[0] = nullptr;
	_tmpBuffer[1] = nullptr;
	_readBuffer = nullptr;
}

// Original name: CLHNM_SetFinalBuffer
void HnmPlayer::setFinalBuffer(byte *buffer) {
	_finalBuffer = buffer;
}

// Original name: CLHNM_GetFrameNum
int HnmPlayer::getFrameNum() {
	return _frameNum;
}

// Original name: CLHNM_TryRead
void HnmPlayer::tryRead(int size) {
	_file->read(_readBuffer, size);
}

// Original name: CLHNM_LoadFrame
bool HnmPlayer::loadFrame() {
	tryRead(4);
	int chunk = *(int *)_readBuffer;
	chunk = FROM_LE_32(chunk);
	chunk &= 0xFFFFFF;  // upper bit - keyframe mark?
	if (!chunk)
		return false;

	if (chunk - 4 > _header._bufferSize)
		error("loadFrame - Chunk size");

	tryRead(chunk - 4);
	_dataPtr = _readBuffer;
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
				l = GetBit() * 2;
				l += GetBit();
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
void HnmPlayer::desentrelace() {
	switch (_header._width) {
	case 320:
		desentrelace320(_newFrameBuffer, _finalBuffer, _header._height);
		break;
		//	case 480:
		//		CLHNM_Desentrelace480(_newFrameBuffer, finalBuffer, _header._height);
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
bool HnmPlayer::nextElement() {
	if (_frameNum == 0) {
		resetInternalTimer();
		_prevLeft = _prevRight = 0;
	}
	if (_frameNum == _header._numbFrame)
		return false;

	if (!loadFrame())
		return false;

	for (;;) {
		int sz = READ_LE_UINT32(_dataPtr) & 0xFFFFFF;
		_dataPtr += 4;
		int16 id = READ_LE_UINT16(_dataPtr);
		_dataPtr += 2;
		char h6 = *_dataPtr;
		_dataPtr += 1;
		char h7 = *_dataPtr;
		_dataPtr += 1;
		switch (id) {
		case MKTAG16('L', 'P'):
			changePalette();
			_dataPtr += sz - 8;
			break;
		case MKTAG16('Z', 'I'):
			_frameNum++;
			selectBuffers();
			decompLempelZiv(_dataPtr + 4, _newFrameBuffer);
#if 0
			switch (_header._width) {
			case 320:
				CLBlitter_RawCopy320ASM(_newFrameBuffer, _oldFrameBuffer, _header._height);
				break;
			case 480:
				CLBlitter_RawCopy480ASM(_newFrameBuffer, _oldFrameBuffer, _header._height);
				break;
			case 640:
				CLBlitter_RawCopy640ASM(_newFrameBuffer, _oldFrameBuffer, _header._height);
				break;
			default: 
				memcpy(_oldFrameBuffer, _newFrameBuffer, _header._width * _header._height);
			}
#else
			memcpy(_oldFrameBuffer, _newFrameBuffer, _header._bufferSize);  //TODO strange buffer size here
#endif
			if (!(h6 & 1))
				desentrelace();
			else {
				//				if(_header._width == 640)
				//					CLBlitter_RawCopy640(_newFrameBuffer, finalBuffer, _header._height);
				//				else
				memcpy(_finalBuffer, _newFrameBuffer, _header._height);   //TODO: wrong size?
			}

			if (!_soundStarted) {
				_soundChannel->play();
				_soundStarted = true;
			}

			return true;
		case MKTAG16('U', 'I'):
			_frameNum++;
			selectBuffers();
			decompUBA(_newFrameBuffer, _newFrameBuffer, _oldFrameBuffer, _dataPtr, _header._width, h6);
			if (!(h6 & 1))
				desentrelace();
			else {
				//				if(_header._width == 640)
				//					CLBlitter_RawCopy640(_newFrameBuffer, _finalBuffer, _header._height);
				//				else
				memcpy(_finalBuffer, _newFrameBuffer, _header._width * _header._height);
			}
			return true;

		case MKTAG16('d', 's'):
		case MKTAG16('D', 'S'):
			if (_useSound) {
				if (!h6) {
					int sound_size = sz - 8;
					if (!_useAdpcm) {
						_soundChannel->queueBuffer(_dataPtr, sound_size - 2, false, _soundStarted);
					} else {
#if 0
						// Not used in Lost Eden
						int16 *sound_buffer = (int16 *)_soundGroup->getNextBuffer();
						if (!_pendingSounds) {
							const int kDecompTableSize = 256 * sizeof(int16);
							loadDecompTable((int16 *)_dataPtr);
							decompADPCM(_dataPtr + kDecompTableSize, sound_buffer, sound_size - kDecompTableSize);
							_soundGroup->assignDatas(sound_buffer, (sound_size - kDecompTableSize) * 2, false);
						} else {
							decompADPCM(_dataPtr, sound_buffer, sound_size);
							_soundGroup->assignDatas(sound_buffer, sound_size * 2, false);
						}
						_pendingSounds++;
						if (_soundStarted)
							_soundGroup->playNextSample(_soundChannel);
#endif
					}
				} else
					error("nextElement - unexpected flag");
			}
			_dataPtr += sz - 8;
			break;
		default:
			if (_customChunkHandler)
				_customChunkHandler(_dataPtr, sz - 8, id, h6, h7);
			_dataPtr += sz - 8;
		}
	}
	return true;
}

// Original name: CLHNM_GetSoundChannel
CSoundChannel *HnmPlayer::getSoundChannel() {
	return _soundChannel;
}

// Original name: CLHNM_ChangePalette
void HnmPlayer::changePalette() {
	CLPalette_GetLastPalette(_palette);
	byte *pal = _dataPtr;
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
		color_t *color = _palette + fst;
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
		_palette[0].r = 0;
		_palette[0].g = 0;
		_palette[0].b = 0;
	}
#endif
	//	CLBlitter_Send2ScreenNextCopy(_palette, mincolor, maxcolor - mincolor);
	CLBlitter_Send2ScreenNextCopy(_palette, 0, 256);
}

// Original name: CLHNM_SelectBuffers
void HnmPlayer::selectBuffers() {
	if (_frameNum % 2) {
		_newFrameBuffer = _tmpBuffer[1];
		_oldFrameBuffer = _tmpBuffer[0];
	} else {
		_newFrameBuffer = _tmpBuffer[0];
		_oldFrameBuffer = _tmpBuffer[1];
	}
}

}   // namespace Cryo

