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

#ifdef ENABLE_EOB

#include "kyra/sound/drivers/pc_base.h"
#include "audio/audiostream.h"
#include "common/mutex.h"

namespace Kyra {

class PCSpeakerDriver : public PCSoundDriver, public Audio::AudioStream {
public:
	PCSpeakerDriver(Audio::Mixer *mixer, bool pcJRMode);
	~PCSpeakerDriver() override;

	void initDriver() override;
	void setSoundData(uint8 *data, uint32 size) override;
	void startSound(int id, int) override;
	bool isChannelPlaying(int channel) const override;
	void stopAllChannels() override;

	void setMusicVolume(uint8 volume) override;
	void setSfxVolume(uint8) override {}

	void update();

	// AudioStream interface
	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override { return false; }
	int getRate() const override { return _outputRate; }
	bool endOfData() const override { return false; }

private:
	void noteOn(int chan, uint16 period);
	void chanOff(int chan);
	void generateSamples(int16 *buffer, int numSamples);

	struct Channel {
		Channel(uint8 attnDB) : curSample(32767.0 / pow(2.0, (double)attnDB / 6.0)),
			dataPtr(0), timer(0), timerScale(0), repeatCounter1(0), repeatCounter2(0), period(-1), samplesLeft(0) {}
		const uint8 *dataPtr;
		int16 timer;
		uint8 timerScale;
		uint8 repeatCounter1;
		uint8 repeatCounter2;
		int32 period;
		int32 curSample;
		uint32 samplesLeft;
	};

	Channel **_channels;
	int _numChannels;

	const uint8 *_newTrackData;
	const uint8 *_trackData;

	Common::Mutex _mutex;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;

	uint _outputRate;
	int _samplesUpdateIntv;
	int _samplesUpdateIntvRem;
	int _samplesUpdateTmr;
	int _samplesUpdateTmrRem;

	int _masterVolume;
	bool _ready;

	const int _clock;
	const int _updateRate;
	const bool _pcJR;
	const int _periodDiv;
	const int _levelAdjust;
	const uint16 * const _periodsTable;

	static const uint16 _periodsPCSpk[96];
	static const uint16 _periodsPCjr[96];
};

PCSpeakerDriver::PCSpeakerDriver(Audio::Mixer *mixer, bool pcJRMode) : PCSoundDriver(), _mixer(mixer), _samplesUpdateIntv(0), _samplesUpdateIntvRem(0),
	_outputRate(0), _samplesUpdateTmr(0), _samplesUpdateTmrRem(0), _newTrackData(0), _trackData(0), _pcJR(pcJRMode), _numChannels(pcJRMode ? 3 : 1), _channels(0),
		_clock(pcJRMode ? 111860 : 1193180), _updateRate(292), _masterVolume(63), _periodsTable(pcJRMode ? _periodsPCjr : _periodsPCSpk), _periodDiv(pcJRMode ? 2 : 2),
	_levelAdjust(pcJRMode ? 1 : 0), _ready(false) {
	_outputRate = _mixer->getOutputRate();
	_samplesUpdateIntv = _outputRate / _updateRate;
	_samplesUpdateIntvRem = _outputRate % _updateRate;

	_channels = new Channel*[_numChannels];
	assert(_channels);
	for (int i = 0; i < _numChannels; ++i) {
		_channels[i] = new Channel(i * 10);
		assert(_channels[i]);
	}
}

PCSpeakerDriver::~PCSpeakerDriver() {
	_ready = false;
	_mixer->stopHandle(_handle);

	if (_channels) {
		for (int i = 0; i < _numChannels; ++i)
			delete _channels[i];
		delete[] _channels;
	}
}

void PCSpeakerDriver::initDriver() {
	if (_ready)
		return;
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
	_ready = true;
}

void PCSpeakerDriver::setSoundData(uint8 *data, uint32 size) {
	Common::StackLock lock(_mutex);
	if (!_ready)
		return;

	if (_soundData) {
		delete[] _soundData;
		_soundData = 0;
	}

	_soundData = data;
	_soundDataSize = size;
}

void PCSpeakerDriver::startSound(int id, int) {
	Common::StackLock lock(_mutex);
	if (!_ready)
		return;
	_newTrackData = getProgram(id & 0x7F);
}

bool PCSpeakerDriver::isChannelPlaying(int channel) const {
	Common::StackLock lock(_mutex);
	if (!_ready)
		return false;
	return _trackData;
}

void PCSpeakerDriver::stopAllChannels() {
	Common::StackLock lock(_mutex);
	if (!_ready)
		return;
	for (int i = 0; i < _numChannels; ++i)
		chanOff(i);
	_trackData = 0;
}

void PCSpeakerDriver::setMusicVolume(uint8 volume) {
	Common::StackLock lock(_mutex);
	_masterVolume = volume >> 2;
}

void PCSpeakerDriver::update() {
	Common::StackLock lock(_mutex);
	if (!_ready)
		return;

	if (_newTrackData) {
		_trackData = _newTrackData;
		_newTrackData = 0;

		for (int i = _numChannels - 1; i >= 0; --i) {
			_channels[i]->dataPtr = _trackData;
			_channels[i]->timer = i * 35;
			_channels[i]->timerScale = 1;
		}
	}

	for (int i = _numChannels - 1; i >= 0; --i) {
		const uint8 *pos = _channels[i]->dataPtr;
		if (!pos)
			continue;

		for (bool runloop = true; runloop; ) {
			if (--_channels[i]->timer > -1)
				break;
			_channels[i]->timer = 0;

			int8 cmd = (int8)*pos++;
			if (cmd >= 0) {
				if (cmd > 95)
					cmd = 0;

				noteOn(i, _periodsTable[cmd]);
				uint8 nextTimer = 1 + *pos++;
				_channels[i]->timer = _channels[i]->timerScale * nextTimer;

			} else {
				switch (cmd) {
				case -23: {
					uint16 ts = _channels[i]->timerScale + *pos++;
					_channels[i]->timerScale = (uint8)MIN<uint16>(ts, 0xFF);
				} break;

				case -24: {
					int16 ts = _channels[i]->timerScale - *pos++;
					_channels[i]->timerScale = (uint8)MAX<int16>(ts, 1);
				} break;

				case -26: {
					uint16 prd = _clock / READ_LE_UINT16(pos);
					if (_pcJR && prd >= 0x400)
						prd = 0x3FF;
					pos += 2;
					noteOn(i, prd);
					uint8 nextTimer = 1 + *pos++;
					_channels[i]->timer = _channels[i]->timerScale * nextTimer;
				} break;

				case -30: {
					_channels[i]->timerScale = *pos++;
					if (!_channels[i]->timerScale)
						_channels[i]->timerScale = 1;
				} break;

				case -46: {
					if (--_channels[i]->repeatCounter2)
						pos -= *pos;
					else
						pos += 2;
				} break;

				case -47: {
					_channels[i]->repeatCounter2 = *pos++;
					if (!_channels[i]->repeatCounter2)
						_channels[i]->repeatCounter2 = 1;
				} break;

				case -50: {
					if (--_channels[i]->repeatCounter1)
						pos -= *pos;
					else
						pos += 2;
				} break;

				case -51: {
					_channels[i]->repeatCounter1 = *pos++;
					if (!_channels[i]->repeatCounter1)
						_channels[i]->repeatCounter1 = 1;
				} break;

				default:
					chanOff(i);
					pos = 0;
					runloop = false;
				}
			}
		}

		_channels[i]->dataPtr = pos;
	}
}

int PCSpeakerDriver::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);
	if (!_ready)
		return 0;

	int render = 0;

	for (int samplesLeft = numSamples; samplesLeft; samplesLeft -= render) {
		if (_samplesUpdateTmr <= 0) {
			_samplesUpdateTmr += _samplesUpdateIntv;
			update();
		}

		_samplesUpdateTmrRem += _samplesUpdateIntvRem;
		while (_samplesUpdateTmrRem >= _updateRate) {
			_samplesUpdateTmr++;
			_samplesUpdateTmrRem -= _updateRate;
		}

		render = MIN<int>(_samplesUpdateTmr, samplesLeft);
		_samplesUpdateTmr -= render;

		generateSamples(buffer, render);
		buffer += render;
	}

	return numSamples;
}

void PCSpeakerDriver::noteOn(int chan, uint16 period) {
	if (chan >= _numChannels)
		return;

	if (period == 0) {
		chanOff(chan);
		return;
	}

	uint32 p = (_outputRate << 10) / ((_clock << 10) / period);
	if (_channels[chan]->period == -1 || _channels[chan]->samplesLeft == 0)
		_channels[chan]->samplesLeft = p / _periodDiv;
	_channels[chan]->period = p & 0xFFFF;
}

void PCSpeakerDriver::chanOff(int chan) {
	if (chan >= _numChannels)
		return;
	_channels[chan]->period = -1;
}

void PCSpeakerDriver::generateSamples(int16 *buffer, int numSamples) {
	int render = 0;
	for (int samplesLeft = numSamples; samplesLeft; samplesLeft -= render) {
		render = samplesLeft;

		for (int i = _numChannels - 1; i >= 0; --i)
			if (_channels[i]->period != -1)
				render = MIN<int>(render, _channels[i]->samplesLeft);

		int32 smp = 0;
		for (int i = _numChannels - 1; i >= 0; --i)
			if (_channels[i]->period != -1)
				smp += _channels[i]->curSample;
		smp = (smp * _masterVolume) >> (8 + _levelAdjust);

		Common::fill<int16*, int16>(buffer, &buffer[render], smp);
		buffer += render;

		for (int i = _numChannels - 1; i >= 0; --i) {
			if (_channels[i]->period == -1)
				continue;

			_channels[i]->samplesLeft -= render;
			if (_channels[i]->samplesLeft == 0) {
				_channels[i]->samplesLeft = _channels[i]->period / _periodDiv;
				_channels[i]->curSample = ~_channels[i]->curSample;
			}
		}
	}
}

const uint16 PCSpeakerDriver::_periodsPCSpk[96] = {
	0x0000, 0xfdff, 0xefa2, 0xe241, 0xd582, 0xc998, 0xbe3d, 0xb38a,
	0xa97c, 0x9ff2, 0x96fc, 0x8e89, 0x8683, 0x7ef7, 0x77d9, 0x7121,
	0x6ac7, 0x64c6, 0x5f1f, 0x59ca, 0x54be, 0x4ffd, 0x4b7e, 0x4742,
	0x4342, 0x3f7b, 0x3bdb, 0x388f, 0x3562, 0x3263, 0x2f8f, 0x2ce4,
	0x2a5f, 0x27fe, 0x25c0, 0x23a1, 0x21a1, 0x1fbe, 0x1df6, 0x1c48,
	0x1ab1, 0x1932, 0x17c8, 0x1672, 0x1530, 0x13ff, 0x12e0, 0x11d1,
	0x10d1, 0x0fdf, 0x0efb, 0x0e24, 0x0d59, 0x0c99, 0x0be4, 0x0b39,
	0x0a98, 0x0a00, 0x0970, 0x08e8, 0x0868, 0x07f0, 0x077e, 0x0712,
	0x06ac, 0x064c, 0x05f2, 0x059d, 0x054c, 0x0500, 0x04b8, 0x0474,
	0x0434, 0x03f8, 0x03bf, 0x0382, 0x0356, 0x0326, 0x02f9, 0x02ce,
	0x02a6, 0x0280, 0x025c, 0x023a, 0x021a, 0x01fc, 0x01df, 0x01c4,
	0x01ab, 0x0193, 0x017c, 0x0167, 0x0153, 0x0140, 0x012e, 0x011d
};

const uint16 PCSpeakerDriver::_periodsPCjr[96] = {
	0x0000, 0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff,
	0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff,
	0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff,
	0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff, 0x03ff,
	0x03f9, 0x03c0, 0x038a, 0x0357, 0x0327, 0x02fa, 0x02cf, 0x02a7,
	0x0281, 0x025d, 0x023b, 0x021b, 0x01fc, 0x01e0, 0x01c5, 0x01ac,
	0x0194, 0x017d, 0x0168, 0x0153, 0x0140, 0x012e, 0x011d, 0x010d,
	0x00fe, 0x00f0, 0x00e2, 0x00d6, 0x00ca, 0x00be, 0x00b4, 0x00aa,
	0x00a0, 0x0097, 0x008f, 0x0087, 0x007f, 0x0078, 0x0071, 0x006b,
	0x0065, 0x005f, 0x005a, 0x0054, 0x0050, 0x004c, 0x0047, 0x0043,
	0x0040, 0x003c, 0x0039, 0x0035, 0x0032, 0x0030, 0x002d, 0x002a,
	0x0028, 0x0026, 0x0024, 0x0022, 0x0020, 0x001e, 0x001c, 0x001b
};

PCSoundDriver *PCSoundDriver::createPCSpk(Audio::Mixer *mixer, bool pcJRMode) {
	return new PCSpeakerDriver(mixer, pcJRMode);
}

} // End of namespace Kyra

#endif
