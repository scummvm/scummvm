/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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

#include "sound/audiostream.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/sound.h"

namespace Gob {

// TODO: This is a very primitive square wave generator. The only thing is
//       has in common with the PC speaker is that it sounds terrible.

class SquareWaveStream : public AudioStream {
private:
	uint _rate;
	bool _beepForever;
	uint32 _periodLength;
	uint32 _periodSamples;
	uint32 _remainingSamples;
	int16 _sampleValue;

public:
	SquareWaveStream() {}
	~SquareWaveStream() {}

	void playNote(int freq, int32 ms);

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const	{ return _remainingSamples == 0; }
	bool isStereo() const	{ return false; }
	int getRate() const	{ return _rate; }
};

void SquareWaveStream::playNote(int freq, int32 ms) {
	_rate = _vm->_mixer->getOutputRate();
	_periodLength = _rate / (2 * freq);
	_periodSamples = 0;
	_sampleValue = 6000;
	if (ms == -1) {
		_remainingSamples = 1;
		_beepForever = true;
	} else {
		_remainingSamples = (_rate * ms) / 1000;
		_beepForever = false;
	}
}

int SquareWaveStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;

	while (samples < numSamples && _remainingSamples > 0) {
		*buffer++ = _sampleValue;
		if (_periodSamples++ > _periodLength) {
			_periodSamples = 0;
			_sampleValue = -_sampleValue;
		}
		samples++;
		if (!_beepForever)
			_remainingSamples--;
	}

	return samples;
}

SquareWaveStream speakerStream;
SoundHandle speakerHandle;
Snd_SoundDesc *snd_loopingSounds[5]; // Should be enough

void snd_initSound(void) {
	for (int i = 0; i < ARRAYSIZE(snd_loopingSounds); i++)
		snd_loopingSounds[i] = NULL;
}

void snd_loopSounds(void) {
	for (int i = 0; i < ARRAYSIZE(snd_loopingSounds); i++) {
		Snd_SoundDesc *snd = snd_loopingSounds[i];
		if (snd && !_vm->_mixer->isSoundHandleActive(snd->handle)) {
			if (snd->repCount-- > 0) {
				_vm->_mixer->playRaw(&snd->handle, snd->data, snd->size, snd->frequency, 0);
			} else {
				snd_loopingSounds[i] = NULL;
			}
		}
	}
}

int16 snd_checkProAudio(void) {return 0;}
int16 snd_checkAdlib(void) {return 0;}
int16 snd_checkBlaster(void) {return 0;}
void snd_setBlasterPort(int16 port) {return;}

void snd_speakerOn(int16 frequency, int32 length) {
	speakerStream.playNote(frequency, length);
	if (!_vm->_mixer->isSoundHandleActive(speakerHandle)) {
		_vm->_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &speakerHandle, &speakerStream, -1, 255, 0, false);
	}
}

void snd_speakerOff(void) {
	_vm->_mixer->stopHandle(speakerHandle);
}

void snd_stopSound(int16 arg){return;}
void snd_setResetTimerFlag(char flag){return;}

void snd_playSample(Snd_SoundDesc *sndDesc, int16 repCount, int16 frequency) {
	assert(frequency > 0);

	if (!_vm->_mixer->isSoundHandleActive(sndDesc->handle)) {
		_vm->_mixer->playRaw(&sndDesc->handle, sndDesc->data, sndDesc->size, frequency, 0);
	}

	sndDesc->repCount = repCount - 1;
	sndDesc->frequency = frequency;

	if (repCount > 1) {
		for (int i = 0; i < ARRAYSIZE(snd_loopingSounds); i++) {
			if (!snd_loopingSounds[i]) {
				snd_loopingSounds[i] = sndDesc;
				return;
			}
		}
		warning("Looping sounds list is full");
	}
}

void snd_cleanupFuncCallback() {;}
CleanupFuncPtr (snd_cleanupFunc);
//CleanupFuncPtr snd_cleanupFunc;// = &snd_cleanupFuncCallback();

int16 snd_soundPort;
char snd_playingSound;

void snd_writeAdlib(int16 port, int16 data) {
	return;
}

Snd_SoundDesc *snd_loadSoundData(const char *path) {
	Snd_SoundDesc *sndDesc;
	int32 size;

	size = data_getDataSize(path);
	sndDesc = (Snd_SoundDesc *)malloc(size);
	sndDesc->size = size;
	sndDesc->data = data_getData(path);

	return sndDesc;
}

void snd_freeSoundData(Snd_SoundDesc *sndDesc) {
	_vm->_mixer->stopHandle(sndDesc->handle);

	for (int i = 0; i < ARRAYSIZE(snd_loopingSounds); i++) {
		if (snd_loopingSounds[i] == sndDesc)
			snd_loopingSounds[i] = NULL;
	}

	free(sndDesc->data);
	free(sndDesc);
}

void snd_playComposition(Snd_SoundDesc ** samples, int16 *composit, int16 freqVal) {;}
void snd_waitEndPlay(void) {;}

}                               // End of namespace Gob



