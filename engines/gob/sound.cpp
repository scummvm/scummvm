/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/sound.h"
#include "gob/game.h"
#include "gob/util.h"

namespace Gob {

void Snd::SquareWaveStream::playNote(int freq, int32 ms, uint rate) {
	_rate = rate;
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

int Snd::SquareWaveStream::readBuffer(int16 *buffer, const int numSamples) {
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

Snd::Snd(GobEngine *vm) : _vm(vm) {
	//CleanupFuncPtr cleanupFunc;// = &snd_cleanupFuncCallback();
	_cleanupFunc = 0;
	for (int i = 0; i < ARRAYSIZE(_loopingSounds); i++)
		_loopingSounds[i] = NULL;
	_playingSound = 0;
}

void Snd::loopSounds(void) {
	for (int i = 0; i < ARRAYSIZE(_loopingSounds); i++) {
		SoundDesc *snd = _loopingSounds[i];
		if (snd && !_vm->_mixer->isSoundHandleActive(snd->handle)) {
			if (snd->repCount-- > 0) {
				_vm->_mixer->playRaw(&snd->handle, snd->data, snd->size, snd->frequency, 0);
			} else {
				_loopingSounds[i] = NULL;
			}
		}
	}
}

void Snd::setBlasterPort(int16 port) {return;}

void Snd::speakerOn(int16 frequency, int32 length) {
	_speakerStream.playNote(frequency, length, _vm->_mixer->getOutputRate());
	if (!_vm->_mixer->isSoundHandleActive(_speakerHandle)) {
		_vm->_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_speakerHandle, &_speakerStream, -1, 255, 0, false);
	}
}

void Snd::speakerOff(void) {
	_vm->_mixer->stopHandle(_speakerHandle);
}

void Snd::playSample(Snd::SoundDesc *sndDesc, int16 repCount, int16 frequency) {
	if (frequency == 0)
		frequency = sndDesc->frequency;

	if (frequency <= 0) {
		warning("Attempted to play a sample with a frequency of %d", frequency);
		return;
	}
//	assert(frequency > 0);

	if (!_vm->_mixer->isSoundHandleActive(sndDesc->handle)) {
		_vm->_mixer->playRaw(&sndDesc->handle, sndDesc->data, sndDesc->size, frequency, 0);
	}

	sndDesc->repCount = repCount - 1;
	sndDesc->frequency = frequency;

	if (repCount > 1) {
		for (int i = 0; i < ARRAYSIZE(_loopingSounds); i++) {
			if (!_loopingSounds[i]) {
				_loopingSounds[i] = sndDesc;
				return;
			}
		}
		warning("Looping sounds list is full");
	}
}

void Snd::writeAdlib(int16 port, int16 data) {
	return;
}

Snd::SoundDesc *Snd::loadSoundData(const char *path) {
	Snd::SoundDesc *sndDesc;

	sndDesc = new Snd::SoundDesc;
	sndDesc->size = _vm->_dataio->getDataSize(path);
	sndDesc->data = _vm->_dataio->getData(path);

	return sndDesc;
}

void Snd::freeSoundDesc(Snd::SoundDesc *sndDesc, bool freedata) {
	_vm->_mixer->stopHandle(sndDesc->handle);

	for (int i = 0; i < ARRAYSIZE(_loopingSounds); i++) {
		if (_loopingSounds[i] == sndDesc)
			_loopingSounds[i] = NULL;
	}

	if (freedata) {
		delete[] sndDesc->data;
	}
	delete sndDesc;
}

}                               // End of namespace Gob



