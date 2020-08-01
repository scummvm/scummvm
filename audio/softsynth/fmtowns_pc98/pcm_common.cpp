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

#include "audio/softsynth/fmtowns_pc98/pcm_common.h"
#include "audio/mixer.h"

PCMChannel_Base::PCMChannel_Base() : _vol(0), _data(0), _dataEnd(0), _loopLen(0), _pos(0), _loopStart(0), _step(0), _panLeft(7), _panRight(7), _activeOutput(false)  {
}

PCMChannel_Base::~PCMChannel_Base() {
	clear();
}

void PCMChannel_Base::clear() {
	_vol = 0;
	_data = 0;
	_dataEnd = 0;
	_loopLen = 0;
	_pos = 0;
	_loopStart = 0;
	_step = 0;
	_panLeft = _panRight = 7;
	_activeOutput = false;
}

void PCMChannel_Base::updateOutput() {
	if (!isPlaying())
		return;

	_pos += _step;

	if (_pos >= _dataEnd) {
		if (_loopStart != _dataEnd) {
			_pos = _loopStart;
			_dataEnd = _loopStart + _loopLen;
		} else {
			_pos = 0;
			stopInternal();
		}
	}
}

int32 PCMChannel_Base::currentSampleLeft() {
	return (isActive() && _panLeft) ? (((_data[_pos >> 11] * _vol) * _panLeft) >> 3) : 0;
}

int32 PCMChannel_Base::currentSampleRight() {
	return (isActive() && _panRight) ? (((_data[_pos >> 11] * _vol) * _panRight) >> 3) : 0;
}

bool PCMChannel_Base::isActive() const {
	return _activeOutput;
}

void PCMChannel_Base::activate() {
	_activeOutput = true;
}

void PCMChannel_Base::deactivate() {
	_activeOutput = false;
}

void PCMChannel_Base::setData(const int8 *data, uint32 dataEnd, uint32 dataStart) {
	_data = data;
	_dataEnd = dataEnd;
	_pos = dataStart;
}

void PCMChannel_Base::setVolume(uint8 vol) {
	_vol = vol;
}

void PCMChannel_Base::setPanPos(uint8 pan) {
	_panLeft = pan & 0x0f;
	_panRight = pan >> 4;
}

void PCMChannel_Base::setupLoop(uint32 loopStart, uint32 loopLen) {
	_loopStart = loopStart << 11;
	_loopLen = loopLen << 11;
}

void PCMChannel_Base::setRate(uint16 rate) {
	_step = rate;
}

PCMDevice_Base::PCMDevice_Base(int samplingRate, int deviceVolume, int numChannels) : _numChannels(numChannels), _deviceVolume(deviceVolume), _intRate((7670454 << 8) / samplingRate),
	_extRate(72 << 8), _timer(0), _musicVolume(Audio::Mixer::kMaxMixerVolume), _sfxVolume(Audio::Mixer::kMaxMixerVolume), _pcmSfxChanMask(0) {
	_channels = new PCMChannel_Base*[numChannels];
}

PCMDevice_Base::~PCMDevice_Base() {
	delete[] _channels;
}

void PCMDevice_Base::assignChannel(uint8 id, PCMChannel_Base *const chan) {
	assert(id < _numChannels);
	_channels[id] = chan;
}

void PCMDevice_Base::setMusicVolume(uint16 vol) {
	_musicVolume = vol;
}

void PCMDevice_Base::setSfxVolume(uint16 vol) {
	_sfxVolume = vol;
}

void PCMDevice_Base::setSfxChanMask(int mask) {
	_pcmSfxChanMask = mask;
}

void PCMDevice_Base::readBuffer(int32 *buffer, uint32 bufferSize) {
	for (uint32 i = 0; i < bufferSize; i++) {
		_timer += _extRate;
		while (_timer >= _intRate) {
			_timer -= _intRate;

			for (int ii = 0; ii < 8; ii++)
				_channels[ii]->updateOutput();
		}

		int32 finOutL = 0;
		int32 finOutR = 0;

		for (int ii = 0; ii < _numChannels; ii++) {
			if (_channels[ii]->isActive()) {
				int32 oL = _channels[ii]->currentSampleLeft();
				int32 oR = _channels[ii]->currentSampleRight();
				if ((1 << ii) & (~_pcmSfxChanMask)) {
					oL = (oL * _musicVolume) / Audio::Mixer::kMaxMixerVolume;
					oR = (oR * _musicVolume) / Audio::Mixer::kMaxMixerVolume;
				}
				if ((1 << ii) & _pcmSfxChanMask) {
					oL = (oL * _sfxVolume) / Audio::Mixer::kMaxMixerVolume;
					oR = (oR * _sfxVolume) / Audio::Mixer::kMaxMixerVolume;
				}
				finOutL += oL;
				finOutR += oR;

				if (!_channels[ii]->isPlaying())
					_channels[ii]->deactivate();
			}
		}

		buffer[i << 1] += ((finOutL * _deviceVolume) >> 7);
		buffer[(i << 1) + 1] += ((finOutR * _deviceVolume) >> 7);
	}
}
