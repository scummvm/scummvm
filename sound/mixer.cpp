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

#include "common/util.h"
#include "common/system.h"

#include "sound/mixer_intern.h"
#include "sound/rate.h"
#include "sound/audiostream.h"
#include "sound/timestamp.h"


namespace Audio {

#pragma mark -
#pragma mark --- Channel classes ---
#pragma mark -


/**
 * Channels used by the sound mixer.
 */
class Channel {
public:
	Channel(Mixer *mixer, Mixer::SoundType type, int id, bool permanent);
	virtual ~Channel() {}

	virtual void mix(int16 *data, uint len) = 0;

	virtual bool isFinished() const = 0;

	bool isPermanent() const { return _permanent; }

	int getId() const { return _id; }

	void pause(bool paused);
	bool isPaused() const { return (_pauseLevel != 0); }

	void setVolume(const byte volume) { _volume = volume; }
	void setBalance(const int8 balance) { _balance = balance; }

	uint32 getElapsedTime();

public:
	const Mixer::SoundType _type;
	SoundHandle _handle;

private:
	bool _permanent;
	int _pauseLevel;
	int _id;

protected:
	Mixer *_mixer;
	byte _volume;
	int8 _balance;

	uint32 _samplesConsumed;
	uint32 _samplesDecoded;
	uint32 _mixerTimeStamp;
	uint32 _pauseStartTime;
	uint32 _pauseTime;
};

class SimpleChannel : public Channel {
private:
	bool _autofreeStream;
	RateConverter *_converter;
	AudioStream *_input;

public:
	SimpleChannel(Mixer *mixer, Mixer::SoundType type, AudioStream *input, bool autofreeStream, bool reverseStereo = false, int id = -1, bool permanent = false);
	~SimpleChannel();

	void mix(int16 *data, uint len);

	bool isFinished() const {
		return _input->endOfStream();
	}
};


#pragma mark -
#pragma mark --- Mixer ---
#pragma mark -


MixerImpl::MixerImpl(OSystem *system)
	: _syst(system), _sampleRate(0), _mixerReady(false), _handleSeed(0) {

	int i;

	for (i = 0; i < ARRAYSIZE(_volumeForSoundType); i++)
		_volumeForSoundType[i] = kMaxMixerVolume;

	for (i = 0; i != NUM_CHANNELS; i++)
		_channels[i] = 0;
}

MixerImpl::~MixerImpl() {
	for (int i = 0; i != NUM_CHANNELS; i++)
		delete _channels[i];
}

void MixerImpl::setReady(bool ready) {
	_mixerReady = ready;
}

uint MixerImpl::getOutputRate() const {
	return _sampleRate;
}

void MixerImpl::setOutputRate(uint sampleRate) {
	if (_sampleRate != 0 && _sampleRate != sampleRate)
		error("Changing the Audio::Mixer output sample rate is not supported");
	_sampleRate = sampleRate;
}

void MixerImpl::insertChannel(SoundHandle *handle, Channel *chan) {
	int index = -1;
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] == 0) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		warning("MixerImpl::out of mixer slots");
		delete chan;
		return;
	}

	_channels[index] = chan;
	 chan->_handle._val = index + (_handleSeed * NUM_CHANNELS);
	_handleSeed++;
	if (handle) {
		*handle = chan->_handle;
	}
}

void MixerImpl::playRaw(
			SoundType type,
			SoundHandle *handle,
			void *sound,
			uint32 size, uint rate, byte flags,
			int id, byte volume, int8 balance,
			uint32 loopStart, uint32 loopEnd) {

	// Create the input stream
	AudioStream *input = makeLinearInputStream((byte *)sound, size, rate, flags, loopStart, loopEnd);

	// Play it
	playInputStream(type, handle, input, id, volume, balance, true, false, ((flags & Mixer::FLAG_REVERSE_STEREO) != 0));
}

void MixerImpl::playInputStream(
			SoundType type,
			SoundHandle *handle,
			AudioStream *input,
			int id, byte volume, int8 balance,
			bool autofreeStream,
			bool permanent,
			bool reverseStereo) {
	Common::StackLock lock(_mutex);

	if (input == 0) {
		warning("input stream is 0");
		return;
	}

	// Prevent duplicate sounds
	if (id != -1) {
		for (int i = 0; i != NUM_CHANNELS; i++)
			if (_channels[i] != 0 && _channels[i]->getId() == id) {
				if (autofreeStream)
					delete input;
				return;
			}
	}

	// Create the channel
	SimpleChannel *chan = new SimpleChannel(this, type, input, autofreeStream, reverseStereo, id, permanent);
	chan->setVolume(volume);
	chan->setBalance(balance);
	insertChannel(handle, chan);
}

void MixerImpl::mixCallback(byte *samples, uint len) {
	assert(samples);

	Common::StackLock lock(_mutex);

	int16 *buf = (int16 *)samples;
	len >>= 2;

	// Since the mixer callback has been called, the mixer must be ready...
	_mixerReady = true;

	//  zero the buf
	memset(buf, 0, 2 * len * sizeof(int16));

	// mix all channels
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i]) {
			if (_channels[i]->isFinished()) {
				delete _channels[i];
				_channels[i] = 0;
			} else if (!_channels[i]->isPaused())
				_channels[i]->mix(buf, len);
		}
}

void MixerImpl::stopAll() {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != 0 && !_channels[i]->isPermanent()) {
			delete _channels[i];
			_channels[i] = 0;
		}
	}
}

void MixerImpl::stopID(int id) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != 0 && _channels[i]->getId() == id) {
			delete _channels[i];
			_channels[i] = 0;
		}
	}
}

void MixerImpl::stopHandle(SoundHandle handle) {
	Common::StackLock lock(_mutex);

	// Simply ignore stop requests for handles of sounds that already terminated
	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->_handle._val != handle._val)
		return;

	delete _channels[index];
	_channels[index] = 0;
}

void MixerImpl::setChannelVolume(SoundHandle handle, byte volume) {
	Common::StackLock lock(_mutex);

	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->_handle._val != handle._val)
		return;

	_channels[index]->setVolume(volume);
}

void MixerImpl::setChannelBalance(SoundHandle handle, int8 balance) {
	Common::StackLock lock(_mutex);

	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->_handle._val != handle._val)
		return;

	_channels[index]->setBalance(balance);
}

uint32 MixerImpl::getSoundElapsedTime(SoundHandle handle) {
	Common::StackLock lock(_mutex);

	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->_handle._val != handle._val)
		return 0;

	return _channels[index]->getElapsedTime();
}

void MixerImpl::pauseAll(bool paused) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != 0) {
			_channels[i]->pause(paused);
		}
	}
}

void MixerImpl::pauseID(int id, bool paused) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != 0 && _channels[i]->getId() == id) {
			_channels[i]->pause(paused);
			return;
		}
	}
}

void MixerImpl::pauseHandle(SoundHandle handle, bool paused) {
	Common::StackLock lock(_mutex);

	// Simply ignore (un)pause requests for sounds that already terminated
	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->_handle._val != handle._val)
		return;

	_channels[index]->pause(paused);
}

bool MixerImpl::isSoundIDActive(int id) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i] && _channels[i]->getId() == id)
			return true;
	return false;
}

int MixerImpl::getSoundID(SoundHandle handle) {
	Common::StackLock lock(_mutex);
	const int index = handle._val % NUM_CHANNELS;
	if (_channels[index] && _channels[index]->_handle._val == handle._val)
		return _channels[index]->getId();
	return 0;
}

bool MixerImpl::isSoundHandleActive(SoundHandle handle) {
	Common::StackLock lock(_mutex);
	const int index = handle._val % NUM_CHANNELS;
	return _channels[index] && _channels[index]->_handle._val == handle._val;
}

bool MixerImpl::hasActiveChannelOfType(SoundType type) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i] && _channels[i]->_type == type)
			return true;
	return false;
}

void MixerImpl::setVolumeForSoundType(SoundType type, int volume) {
	assert(0 <= type && type < ARRAYSIZE(_volumeForSoundType));

	// Check range
	if (volume > kMaxMixerVolume)
		volume = kMaxMixerVolume;
	else if (volume < 0)
		volume = 0;

	// TODO: Maybe we should do logarithmic (not linear) volume
	// scaling? See also Player_V2::setMasterVolume

	_volumeForSoundType[type] = volume;
}

int MixerImpl::getVolumeForSoundType(SoundType type) const {
	assert(0 <= type && type < ARRAYSIZE(_volumeForSoundType));

	return _volumeForSoundType[type];
}


#pragma mark -
#pragma mark --- Channel implementations ---
#pragma mark -

Channel::Channel(Mixer *mixer, Mixer::SoundType type, int id, bool permanent)
    : _type(type), _mixer(mixer), _id(id), _permanent(permanent), _volume(Mixer::kMaxChannelVolume),
      _balance(0), _pauseLevel(0), _samplesConsumed(0), _samplesDecoded(0), _mixerTimeStamp(0),
      _pauseStartTime(0), _pauseTime(0) {
}

void Channel::pause(bool paused) {
	//assert((paused && _pauseLevel >= 0) || (!paused && _pauseLevel));

	if (paused) {
		_pauseLevel++;

		if (_pauseLevel == 1)
			_pauseStartTime = g_system->getMillis();
	} else if (_pauseLevel > 0) {
		_pauseLevel--;

		if (!_pauseLevel) {
			_pauseTime = (g_system->getMillis() - _pauseStartTime);
			_pauseStartTime = 0;
		}
	}
}

uint32 Channel::getElapsedTime() {
	if (_mixerTimeStamp == 0)
		return 0;

	const uint32 rate = _mixer->getOutputRate();
	uint32 delta = 0;

	if (isPaused())
		delta = _pauseStartTime - _mixerTimeStamp;
	else
		delta = g_system->getMillis() - _mixerTimeStamp - _pauseTime;

	// Convert the number of samples into a time duration.

	Audio::Timestamp ts(0, rate);

	ts = ts.addFrames(_samplesConsumed);
	ts = ts.addMsecs(delta);

	// In theory it would seem like a good idea to limit the approximation
	// so that it never exceeds the theoretical upper bound set by
	// _samplesDecoded. Meanwhile, back in the real world, doing so makes
	// the Broken Sword cutscenes noticeably jerkier. I guess the mixer
	// isn't invoked at the regular intervals that I first imagined.

	return ts.msecs();
}

SimpleChannel::SimpleChannel(Mixer *mixer, Mixer::SoundType type, AudioStream *input,
				bool autofreeStream, bool reverseStereo, int id, bool permanent)
	: Channel(mixer, type, id, permanent), _autofreeStream(autofreeStream), _converter(0),
	  _input(input) {
	assert(mixer);
	assert(input);

	// Get a rate converter instance
	_converter = makeRateConverter(_input->getRate(), mixer->getOutputRate(), _input->isStereo(), reverseStereo);
}

SimpleChannel::~SimpleChannel() {
	delete _converter;
	if (_autofreeStream)
		delete _input;
}

/* len indicates the number of sample *pairs*. So a value of
   10 means that the buffer contains twice 10 sample, each
   16 bits, for a total of 40 bytes.
 */
void SimpleChannel::mix(int16 *data, uint len) {
	assert(_input);

	if (_input->endOfData()) {
		// TODO: call drain method
	} else {
		assert(_converter);

		// From the channel balance/volume and the global volume, we compute
		// the effective volume for the left and right channel. Note the
		// slightly odd divisor: the 255 reflects the fact that the maximal
		// value for _volume is 255, while the 127 is there because the
		// balance value ranges from -127 to 127.  The mixer (music/sound)
		// volume is in the range 0 - kMaxMixerVolume.
		// Hence, the vol_l/vol_r values will be in that range, too

		int vol = _mixer->getVolumeForSoundType(_type) * _volume;
		st_volume_t vol_l, vol_r;

		if (_balance == 0) {
			vol_l = vol / Mixer::kMaxChannelVolume;
			vol_r = vol / Mixer::kMaxChannelVolume;
		} else if (_balance < 0) {
			vol_l = vol / Mixer::kMaxChannelVolume;
			vol_r = ((127 + _balance) * vol) / (Mixer::kMaxChannelVolume * 127);
		} else {
			vol_l = ((127 - _balance) * vol) / (Mixer::kMaxChannelVolume * 127);
			vol_r = vol / Mixer::kMaxChannelVolume;
		}

		_samplesConsumed = _samplesDecoded;
		_mixerTimeStamp = g_system->getMillis();
		_pauseTime = 0;
		_samplesDecoded += _converter->flow(*_input, data, len, vol_l, vol_r);
	}
}


} // End of namespace Audio
