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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "gob/sound/sound.h"

namespace Gob {

SoundBlaster::SoundBlaster(Audio::Mixer &mixer) : _mixer(&mixer) {
	_playingSound = 0;
	_curSoundDesc = 0;

	_rate = _mixer->getOutputRate();
	_end = true;
	_data = 0;
	_length = 0;
	_freq = 0;
	_repCount = 0;

	_offset = 0;
	_offsetFrac = 0;
	_offsetInc = 0;

	_cur = 0;
	_last = 0;

	_fade = false;
	_fadeVol = 65536;
	_fadeVolStep = 0;
	_fadeSamples = 0;
	_curFadeSamples = 0;

	_compositionSamples = 0;
	_compositionSampleCount = 0;
	_compositionPos = -1;

	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_handle,
			this, -1, 255, 0, false, true);
}

SoundBlaster::~SoundBlaster() {
	_mixer->stopHandle(_handle);
}

bool SoundBlaster::isPlaying() const {
	return !_end;
}

char SoundBlaster::getPlayingSound() const {
	return _playingSound;
}

void SoundBlaster::stopSound(int16 fadeLength, SoundDesc *sndDesc) {
	Common::StackLock slock(_mutex);

	if (sndDesc && (sndDesc != _curSoundDesc))
		return;

	if (fadeLength <= 0) {
		_data = 0;
		_end = true;
		_playingSound = 0;
		_curSoundDesc = 0;
		return;
	}

	_fade = true;
	_fadeVol = 65536;
	_fadeSamples = (int) (fadeLength * (((double) _rate) / 10.0));
	_fadeVolStep = MAX((int32) 1, (int32) (65536 / _fadeSamples));
	_curFadeSamples = 0;
}

void SoundBlaster::setRepeating(int32 repCount) {
	Common::StackLock slock(_mutex);

	_repCount = repCount;
}

void SoundBlaster::stopComposition() {
	if (_compositionPos != -1) {
		stopSound(0);
		_compositionPos = -1;
	}
}

void SoundBlaster::endComposition() {
	_compositionPos = -1;
}

void SoundBlaster::nextCompositionPos() {
	int8 slot;

	while ((++_compositionPos < 50) &&
			((slot = _composition[_compositionPos]) != -1)) {
		if ((slot >= 0) && (slot < _compositionSampleCount)) {
			SoundDesc &sample = _compositionSamples[slot];
			if (!sample.empty() && (sample.getType() == SOUND_SND)) {
				setSample(sample, 1, 0, 0);
				return;
			}
		}
		if (_compositionPos == 49)
			_compositionPos = -1;
	}
	_compositionPos = -1;
}

void SoundBlaster::playComposition(int16 *composition, int16 freqVal,
		SoundDesc *sndDescs, int8 sndCount) {

	_compositionSamples = sndDescs;
	_compositionSampleCount = sndCount; 

	int i = -1;
	do {
		i++;
		_composition[i] = composition[i];
	} while ((i < 50) && (composition[i] != -1));

	_compositionPos = -1;
	nextCompositionPos();
}

void SoundBlaster::setSample(SoundDesc &sndDesc, int16 repCount, int16 frequency,
		int16 fadeLength) {

	if (frequency <= 0)
		frequency = sndDesc._frequency;

	_curSoundDesc = &sndDesc;
	sndDesc._repCount = repCount - 1;
	sndDesc._frequency = frequency;

	_data = (int8 *) sndDesc.getData();
	_length = sndDesc.size();
	_freq = frequency;

	_repCount = repCount;
	_end = false;
	_playingSound = 1;

	_offset = 0;
	_offsetFrac = 0;
	_offsetInc = (_freq << FRAC_BITS) / _rate;

	_last = _cur;
	_cur = _data[0];

	_curFadeSamples = 0;
	if (fadeLength == 0) {
		_fade = false;
		_fadeVol = 65536;
		_fadeSamples = 0;
		_fadeVolStep = 0;
	} else {
		_fade = true;
		_fadeVol = 0;
		_fadeSamples = (int) (fadeLength * (((double) _rate) / 10.0));
		_fadeVolStep = - MAX((int32) 1, (int32) (65536 / _fadeSamples));
	}
}

void SoundBlaster::playSample(SoundDesc &sndDesc, int16 repCount, int16 frequency,
		int16 fadeLength) {
	Common::StackLock slock(_mutex);

	if (!_end)
		return;

	setSample(sndDesc, repCount, frequency, fadeLength);
}

void SoundBlaster::checkEndSample() {
	if (_compositionPos != -1)
		nextCompositionPos();
	else if ((_repCount == -1) || (--_repCount > 0)) {
		_offset = 0;
		_offsetFrac = 0;
		_end = false;
		_playingSound = 1;
	} else {
		_end = true;
		_playingSound = 0;
	}
}

int SoundBlaster::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock slock(_mutex);

	for (int i = 0; i < numSamples; i++) {
		if (!_data)
			return i;
		if (_end || (_offset >= _length))
			checkEndSample();
		if (_end)
			return i;

		// Linear interpolation. See sound/rate.cpp

		int16 val = (_last + (((_cur - _last) * _offsetFrac +
					FRAC_HALF) >> FRAC_BITS)) << 8;
		*buffer++ = (val * _fadeVol) >> 16;

		_offsetFrac += _offsetInc;

		// Was there an integral change?
		if (fracToInt(_offsetFrac) > 0) {
			_last = _cur;
			_cur = _data[_offset];
			_offset += fracToInt(_offsetFrac);
			_offsetFrac &= FRAC_LO_MASK;
		}

		if (_fade) {

			if (++_curFadeSamples >= _fadeSamples) {
				if (_fadeVolStep > 0) {
					_data = 0;
					_end = true;
					_playingSound = 0;
					_compositionPos = -1;
					_curSoundDesc = 0;
				} else {
					_fadeVol = 65536;
					_fade = false;
				}
			} else
				_fadeVol -= _fadeVolStep;

			if (_fadeVol < 0)
				_fadeVol = 0;

		}
	}
	return numSamples;
}

} // End of namespace Gob
