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

Snd::SquareWaveStream::SquareWaveStream() {
	_rate = 44100;
	_beepForever = false;
	_periodLength = 0;
	_periodSamples = 0;
	_remainingSamples = 0;
	_sampleValue = 0;
	_mixedSamples = 0;
}

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
	_mixedSamples = 0;
}

void Snd::SquareWaveStream::stop(uint32 milis) {
	if (!_beepForever)
		return;

	if (milis)
		update(milis);
	else
		_remainingSamples = 0;
}

void Snd::SquareWaveStream::update(uint32 milis) {
	uint32 neededSamples;

	if (!_beepForever || !_remainingSamples)
		return;

	neededSamples = (_rate * milis) / 1000;
	_remainingSamples =
		neededSamples > _mixedSamples ? neededSamples - _mixedSamples : 0;
	_beepForever = false;
}

int Snd::SquareWaveStream::readBuffer(int16 *buffer, const int numSamples) {
	for (int i = 0; i < numSamples; i++) {
		if (!_remainingSamples) {
			buffer[i] = 0;
			continue;
		}
		buffer[i] = _sampleValue;
		if (_periodSamples++ > _periodLength) {
			_periodSamples = 0;
			_sampleValue = -_sampleValue;
		}
		if (!_beepForever)
			_remainingSamples--;
		_mixedSamples++;
	}

	return numSamples;
}

Snd::Snd(GobEngine *vm) : _vm(vm) {
	_cleanupFunc = 0;
	_playingSound = 0;

	_rate = _vm->_mixer->getOutputRate();
	_end = true;
	_data = 0;
	_length = 0;
	_freq = 0;
	_repCount = 0;
	_offset = 0.0;

	_frac = 0.0;
	_cur = 0;
	_last = 0;

	_fade = false;
	_fadeVol = 255.0;
	_fadeVolStep = 0.0;
	_fadeSamples = 0;
	_curFadeSamples = 0;

	_compositionSamples = 0;
	_compositionSampleTypes = 0;
	_compositionSampleCount = 0;
	_compositionPos = -1;

	_vm->_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_handle,
			this, -1, 255, 0, false, true);
	_vm->_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
			&_speakerStream, -1, 255, 0, false, true);
}

void Snd::setBlasterPort(int16 port) {return;}

void Snd::speakerOn(int16 frequency, int32 length) {
	_speakerStream.playNote(frequency, length, _vm->_mixer->getOutputRate());
	_speakerStartTimeKey = _vm->_util->getTimeKey();
}

void Snd::speakerOff(void) {
	_speakerStream.stop(_vm->_util->getTimeKey() - _speakerStartTimeKey);
}

void Snd::speakerOnUpdate(uint32 milis) {
	_speakerStream.update(milis);
}

void Snd::stopSound(int16 fadeLength)
{
	Common::StackLock slock(_mutex);

	if (fadeLength <= 0) {
		_data = 0;
		_end = true;
		_playingSound = 0;
		return;
	}

	_fade = true;
	_fadeVol = 255.0;
	_fadeSamples = (int) (fadeLength * (((double) _rate) / 10.0));
	_fadeVolStep = 255.0 / ((double) _fadeSamples);
	_curFadeSamples = 0;
}

void Snd::waitEndPlay(bool interruptible, bool stopComp) {
	if (stopComp)
		_compositionPos = -1;
	while (!_end && !_vm->_quitRequested) {
		if (interruptible && (_vm->_util->checkKey() == 0x11B)) {
			WRITE_VAR(57, -1);
			return;
		}
		_vm->_util->longDelay(200);
	}
	stopSound(0);
}

void Snd::stopComposition(void) {
	if (_compositionPos != -1) {
		stopSound(0);
		_compositionPos = -1;
	}
}

void Snd::nextCompositionPos(void) {
	int8 slot;

	while ((++_compositionPos < 50) && ((slot = _composition[_compositionPos]) != -1)) {
		if ((slot >= 0) && (slot < _compositionSampleCount) &&
				(_compositionSamples[slot] != 0) && !(_compositionSampleTypes[slot] & 8)) {
			setSample(_compositionSamples[slot], 1, 0, 0);
			return;
		}
		if (_compositionPos == 49)
			_compositionPos = -1;
	}
	_compositionPos = -1;
}

void Snd::playComposition(int16 *composition, int16 freqVal, SoundDesc **sndDescs,
		int8 *sndTypes, int8 sndCount) {
	int i;

	waitEndPlay();
	stopComposition();

	_compositionSamples = sndDescs ? sndDescs : _vm->_game->_soundSamples;
	_compositionSampleTypes = sndTypes ? sndTypes : _vm->_game->_soundTypes;
	_compositionSampleCount = sndCount;

	i = -1;
	do {
		i++;
		_composition[i] = composition[i];
	} while ((i < 50) && (composition[i] != -1));

	nextCompositionPos();
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
	if (sndDesc == _curSoundDesc)
		stopSound(0);

	if (freedata) {
		delete[] sndDesc->data;
	}
	delete sndDesc;
}

void Snd::setSample(Snd::SoundDesc *sndDesc, int16 repCount, int16 frequency, int16 fadeLength) {
	if (frequency <= 0)
		frequency = sndDesc->frequency;

	_curSoundDesc = sndDesc;
	sndDesc->repCount = repCount - 1;
	sndDesc->frequency = frequency;

	_data = (int8 *) sndDesc->data;
	_length = sndDesc->size;
	_freq = frequency;
	_ratio = ((double) _freq) / _rate;
	_offset = 0.0;
	_frac = 0;
	_last = _cur;
	_cur = _data[0];
	_repCount = repCount;
	_end = false;
	_playingSound = 1;

	_curFadeSamples = 0;
	if (fadeLength == 0) {
		_fade = false;
		_fadeVol = 255.0;
		_fadeSamples = 0;
		_fadeVolStep = 0.0;
	} else {
		_fade = true;
		_fadeVol = 0.0;
		_fadeSamples = (int) (fadeLength * (((double) _rate) / 10.0));
		_fadeVolStep = -(255.0 / ((double) _fadeSamples));
	}
}

void Snd::playSample(Snd::SoundDesc *sndDesc, int16 repCount, int16 frequency, int16 fadeLength) {
	Common::StackLock slock(_mutex);

	if (!_end)
		return; 

	setSample(sndDesc, repCount, frequency, fadeLength);
}

void Snd::checkEndSample(void) {
	if (_compositionPos != -1)
		nextCompositionPos();
	else if ((_repCount == -1) || (--_repCount > 0)) {
		_offset = 0.0;
		_frac = 0.0;
		_end = false;
		_playingSound = 1;
	} else {
		_end = true;
		_playingSound = 0;
	}
}

int Snd::readBuffer(int16 *buffer, const int numSamples) {
	memset(buffer, 0, numSamples);

	for (int i = 0; i < numSamples; i++) {
		Common::StackLock slock(_mutex);

		if (!_data)
			return i;
		if (_end || (_offset >= _length))
			checkEndSample();
		if (_end)
			return i;

		*buffer++ = (int16) ((_last + (_cur - _last) * _frac) * _fadeVol);
		_frac += _ratio;
		_offset += _ratio;
		while ((_frac > 1) && (_offset < _length)) {
			_frac -= 1;
			_last = _cur;
			_cur = _data[(int) _offset];
		}

		if (_fade) {
			if (++_curFadeSamples >= _fadeSamples) {
				if (_fadeVolStep > 0) {
					_data = 0;
					_end = true;
					_playingSound = 0;
					_compositionPos = -1;
				} else {
					_fadeVol = 255.0;
					_fade = false;
				}
			} else
				_fadeVol -= _fadeVolStep;
		}
	}
	return numSamples;
}

} // End of namespace Gob
