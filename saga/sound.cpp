/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "saga/saga.h"

#include "saga/sound.h"
#include "saga/game_mod.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Saga {
#define BUFFER_SIZE 4096

// Routines to convert 12 bit linear samples to the
// Dialogic or Oki ADPCM coding format.
class VOXInputStream : public AudioStream {
private:
	const byte *_buf;
	uint32 _pos;
	uint32 _inputLen;
	bool _evenPos;

	struct adpcmStatus {
		int16 last;
		int16 stepIndex;
	} _status;

	int16 stepAdjust(byte);
	int16 adpcmDecode(byte);

public:
	VOXInputStream(const byte *input, int inputLen);
	~VOXInputStream() {};

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const	{ return _pos >= _inputLen; }
	bool isStereo() const	{ return false; }
	int getRate() const	{ return 22050; }
};


VOXInputStream::VOXInputStream(const byte *input, int inputLen)
	: _buf(input), _pos(0), _inputLen(inputLen), _evenPos(true) {

	_status.last = 0;
	_status.stepIndex = 0;
}

int VOXInputStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;

	while (samples < numSamples && !endOfData()) {
		const int len = MIN(numSamples - samples, (int) (_inputLen - _pos));

		// * 16 effectively converts 12-bit input to 16-bit output
		for (int i = 0; i < len; i++) {
			if (_evenPos)
				buffer[i] = adpcmDecode((_buf[_pos] >> 4) & 0x0f) * 16;
			else {
				buffer[i] = adpcmDecode(_buf[_pos] & 0x0f) * 16;
				_pos++;
			}
			_evenPos = !_evenPos;
		}

		samples += len;
	}
	return samples;
}

// adjust the step for use on the next sample.
int16 VOXInputStream::stepAdjust(byte code) {
	static int16 adjusts[] = {-1, -1, -1, -1, 2, 4, 6, 8};

	return adjusts[code & 0x07];
}

static int16 stepSize[49] = { 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41,
	45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173,
	190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658,
	724, 796, 876, 963, 1060, 1166, 1282, 1408, 1552 };

// Decode Linear to ADPCM
int16 VOXInputStream::adpcmDecode(byte code) {
	int16 diff, E, SS, samp;

	SS = stepSize[_status.stepIndex];
	E = SS/8;
	if (code & 0x01)
		E += SS/4;
	if (code & 0x02)
		E += SS/2;
	if (code & 0x04)
		E += SS;
	diff = (code & 0x08) ? -E : E;
	samp = _status.last + diff;

    // Clip the values to +/- 2^11 (supposed to be 12 bits)
	if(samp > 2048)
		samp = 2048;
	if(samp < -2048)
		samp = -2048;

	_status.last = samp;
	_status.stepIndex += stepAdjust(code);
	if(_status.stepIndex < 0)
		_status.stepIndex = 0;
	if(_status.stepIndex > 48)
		_status.stepIndex = 48;

	return samp;
}

AudioStream *makeVOXStream(const byte *input, int size) {
	AudioStream *audioStream = new VOXInputStream(input, size);

	return audioStream;
}

Sound::Sound(SagaEngine *vm, SoundMixer *mixer, int enabled) : 
	_vm(vm), _mixer(mixer), _enabled(enabled) {

	_soundInitialized = 1;
	return;
}

Sound::~Sound() {
	if (!_soundInitialized) {
		return;
	}

	_soundInitialized = 0;
}

int Sound::playSoundBuffer(PlayingSoundHandle *handle, SOUNDBUFFER *buf, int volume, bool loop) {
	byte flags;

	if (!_soundInitialized) {
		return FAILURE;
	}

	flags = SoundMixer::FLAG_AUTOFREE;

	if (loop)
		flags |= SoundMixer::FLAG_LOOP;

	if (buf->s_samplebits == 16)
		flags |= (SoundMixer::FLAG_16BITS | SoundMixer::FLAG_LITTLE_ENDIAN);
	if (buf->s_stereo)
		flags |= SoundMixer::FLAG_STEREO;
	if (!buf->s_signed)
		flags |= SoundMixer::FLAG_UNSIGNED;

	// FIXME: Remove the code below if the code above works.

#if 0
	int game_id = GAME_GetGame();

	if((game_id == GAME_ITE_DISK) || (game_id == GAME_ITE_DEMO)) {
		flags = SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE;
	} else {
		flags = SoundMixer::FLAG_AUTOFREE | SoundMixer::FLAG_16BITS |
			SoundMixer::FLAG_LITTLE_ENDIAN;
	}
#endif

	_mixer->playRaw(handle, buf->s_buf, buf->s_buf_len, buf->s_freq, flags, -1, volume);

	return SUCCESS;
}

int Sound::playSound(SOUNDBUFFER *buf, int volume) {
	return playSoundBuffer(&_effectHandle, buf, 2 * volume, false);
}

int Sound::pauseSound() {
	if (!_soundInitialized) {
		return FAILURE;
	}

	_mixer->pauseHandle(_effectHandle, true);

	return SUCCESS;
}

int Sound::resumeSound() {
	if (!_soundInitialized) {
		return FAILURE;
	}

	_mixer->pauseHandle(_effectHandle, false);

	return SUCCESS;
}

int Sound::stopSound() {
	if (!_soundInitialized) {
		return FAILURE;
	}

	_mixer->stopHandle(_effectHandle);

	return SUCCESS;
}

int Sound::playVoice(SOUNDBUFFER *buf) {
	return playSoundBuffer(&_voiceHandle, buf, 255, false);
}

int Sound::playVoxVoice(SOUNDBUFFER *buf) {
	AudioStream *audioStream;

	audioStream = makeVOXStream(buf->s_buf, buf->s_buf_len);
	_mixer->playInputStream(&_voiceHandle, audioStream, false);

	return SUCCESS;
}

int Sound::pauseVoice() {
	if (!_soundInitialized) {
		return FAILURE;
	}

	_mixer->pauseHandle(_voiceHandle, true);

	return SUCCESS;
}

int Sound::resumeVoice() {
	if (!_soundInitialized) {
		return FAILURE;
	}

	_mixer->pauseHandle(_voiceHandle, false);

	return SUCCESS;
}

int Sound::stopVoice() {
	if (!_soundInitialized) {
		return FAILURE;
	}

	_mixer->stopHandle(_voiceHandle);

	return SUCCESS;
}

} // End of namespace Saga
