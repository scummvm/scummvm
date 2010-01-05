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
 * Generic Channel interface used by the default Mixer implementation.
 */
class Channel {
public:
	Channel(Mixer *mixer, Mixer::SoundType type, int id, bool permanent);
	virtual ~Channel() {}

	/**
	 * Mixes the channel's samples into the given buffer.
	 *
	 * @param data buffer where to mix the data
	 * @param len  number of sample *pairs*. So a value of
	 *             10 means that the buffer contains twice 10 sample, each
	 *             16 bits, for a total of 40 bytes.
	 */
	virtual void mix(int16 *data, uint len) = 0;

	/**
	 * Queries whether the channel is still playing or not.
	 */
	virtual bool isFinished() const = 0;

	/**
	 * Queries whether the channel is a permanent channel.
	 * A permanent channel is not affected by a Mixer::stopAll
	 * call.
	 */
	bool isPermanent() const { return _permanent; }

	/**
	 * Returns the id of the channel.
	 */
	int getId() const { return _id; }

	/**
	 * Pauses or unpaused the channel in a recursive fashion.
	 *
	 * @param paused true, when the channel should be paused.
	 *               false when it should be unpaused.
	 */
	void pause(bool paused);

	/**
	 * Queries whether the channel is currently paused.
	 */
	bool isPaused() const { return (_pauseLevel != 0); }

	/**
	 * Sets the channel's own volume.
	 *
	 * @param volume new volume
	 */
	void setVolume(const byte volume);

	/**
	 * Sets the channel's balance setting.
	 *
	 * @param balance new balance
	 */
	void setBalance(const int8 balance);

	/**
	 * Notifies the channel that the global sound type
	 * volume settings changed.
	 */
	void notifyGlobalVolChange() { updateChannelVolumes(); }

	/**
	 * Queries how many milliseconds the channel has
	 * been playing.
	 */
	uint32 getElapsedTime();

	/**
	 * Queries the channel's sound type.
	 */
	Mixer::SoundType getType() const { return _type; }

	/**
	 * Sets the channel's sound handle.
	 *
	 * @param handle new handle
	 */
	void setHandle(const SoundHandle handle) { _handle = handle; }

	/**
	 * Queries the channel's sound handle.
	 */
	SoundHandle getHandle() const { return _handle; }

private:
	const Mixer::SoundType _type;
	SoundHandle _handle;
	bool _permanent;
	int _pauseLevel;
	int _id;

	byte _volume;
	int8 _balance;

	void updateChannelVolumes();
	st_volume_t _volL, _volR;

protected:
	/**
	 * Queries the volume of the left output channel.
	 */
	st_volume_t getLeftVolume() const { return _volL; }

	/**
	 * Queries the volume of the right output channel.
	 */
	st_volume_t getRightVolume() const { return _volR; }

	Mixer *_mixer;

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
	SimpleChannel(Mixer *mixer, Mixer::SoundType type, AudioStream *input, bool autofreeStream, bool reverseStereo, int id, bool permanent);
	~SimpleChannel();

	void mix(int16 *data, uint len);

	bool isFinished() const {
		return _input->endOfStream();
	}
};

class LoopingChannel : public Channel {
public:
	LoopingChannel(Mixer *mixer, Mixer::SoundType type, SeekableAudioStream *input, uint loopCount, Timestamp loopStart, Timestamp loopEnd, bool autofreeStream, bool reverseStereo, int id, bool permanent);
	~LoopingChannel();

	void mix(int16 *data, uint len);
	bool isFinished() const;
private:
	uint _loopCount;
	Timestamp _loopStart;
	Timestamp _loopEnd;

	bool _autofreeStream;
	RateConverter *_converter;
	SeekableAudioStream *_input;
	Timestamp _pos;
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

	SoundHandle chanHandle;
	chanHandle._val = index + (_handleSeed * NUM_CHANNELS);

	chan->setHandle(chanHandle);
	_handleSeed++;
	if (handle)
		*handle = chanHandle;
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
	Channel *chan = new SimpleChannel(this, type, input, autofreeStream, reverseStereo, id, permanent);
	chan->setVolume(volume);
	chan->setBalance(balance);
	insertChannel(handle, chan);
}

void MixerImpl::playInputStreamLooping(
			SoundType type,
			SoundHandle *handle,
			SeekableAudioStream *input,
			uint loopCount,
			Timestamp loopStart, Timestamp loopEnd,
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

	if (loopEnd.msecs() == 0)
		loopEnd = input->getLength();

	// Create the channel
	Channel *chan = new LoopingChannel(this, type, input, loopCount,
	                                   loopStart.convertToFramerate(getOutputRate()),
	                                   loopEnd.convertToFramerate(getOutputRate()),
	                                   autofreeStream, reverseStereo, id, permanent);
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
	if (!_channels[index] || _channels[index]->getHandle()._val != handle._val)
		return;

	delete _channels[index];
	_channels[index] = 0;
}

void MixerImpl::setChannelVolume(SoundHandle handle, byte volume) {
	Common::StackLock lock(_mutex);

	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->getHandle()._val != handle._val)
		return;

	_channels[index]->setVolume(volume);
}

void MixerImpl::setChannelBalance(SoundHandle handle, int8 balance) {
	Common::StackLock lock(_mutex);

	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->getHandle()._val != handle._val)
		return;

	_channels[index]->setBalance(balance);
}

uint32 MixerImpl::getSoundElapsedTime(SoundHandle handle) {
	Common::StackLock lock(_mutex);

	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->getHandle()._val != handle._val)
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
	if (!_channels[index] || _channels[index]->getHandle()._val != handle._val)
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
	if (_channels[index] && _channels[index]->getHandle()._val == handle._val)
		return _channels[index]->getId();
	return 0;
}

bool MixerImpl::isSoundHandleActive(SoundHandle handle) {
	Common::StackLock lock(_mutex);
	const int index = handle._val % NUM_CHANNELS;
	return _channels[index] && _channels[index]->getHandle()._val == handle._val;
}

bool MixerImpl::hasActiveChannelOfType(SoundType type) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i] && _channels[i]->getType() == type)
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

	Common::StackLock lock(_mutex);
	_volumeForSoundType[type] = volume;

	for (int i = 0; i != NUM_CHANNELS; ++i) {
		if (_channels[i] && _channels[i]->getType() == type)
			_channels[i]->notifyGlobalVolChange();
	}
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
	updateChannelVolumes();
}

void Channel::setVolume(const byte volume) {
	_volume = volume;
	updateChannelVolumes();
}

void Channel::setBalance(const int8 balance) {
	_balance = balance;
	updateChannelVolumes();
}

void Channel::updateChannelVolumes() {
	// From the channel balance/volume and the global volume, we compute
	// the effective volume for the left and right channel. Note the
	// slightly odd divisor: the 255 reflects the fact that the maximal
	// value for _volume is 255, while the 127 is there because the
	// balance value ranges from -127 to 127.  The mixer (music/sound)
	// volume is in the range 0 - kMaxMixerVolume.
	// Hence, the vol_l/vol_r values will be in that range, too

	int vol = _mixer->getVolumeForSoundType(_type) * _volume;

	if (_balance == 0) {
		_volL = vol / Mixer::kMaxChannelVolume;
		_volR = vol / Mixer::kMaxChannelVolume;
	} else if (_balance < 0) {
		_volL = vol / Mixer::kMaxChannelVolume;
		_volR = ((127 + _balance) * vol) / (Mixer::kMaxChannelVolume * 127);
	} else {
		_volL = ((127 - _balance) * vol) / (Mixer::kMaxChannelVolume * 127);
		_volR = vol / Mixer::kMaxChannelVolume;
	}
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

void SimpleChannel::mix(int16 *data, uint len) {
	assert(_input);

	if (_input->endOfData()) {
		// TODO: call drain method
	} else {
		assert(_converter);

		_samplesConsumed = _samplesDecoded;
		_mixerTimeStamp = g_system->getMillis();
		_pauseTime = 0;
		_samplesDecoded += _converter->flow(*_input, data, len, getLeftVolume(), getRightVolume());
	}
}

LoopingChannel::LoopingChannel(Mixer *mixer, Mixer::SoundType type, SeekableAudioStream *input, uint loopCount,
                               Timestamp loopStart, Timestamp loopEnd, bool autofreeStream, bool reverseStereo,
                               int id, bool permanent)
    : Channel(mixer, type, id, permanent), _loopCount(loopCount), _loopStart(loopStart), _loopEnd(loopEnd),
      _autofreeStream(autofreeStream), _converter(0), _input(input), _pos(0, mixer->getOutputRate()) {
	_input->seek(loopStart);
	// Get a rate converter instance
	_converter = makeRateConverter(_input->getRate(), mixer->getOutputRate(), _input->isStereo(), reverseStereo);
}

LoopingChannel::~LoopingChannel() {
	delete _converter;
	if (_autofreeStream)
		delete _input;
}

void LoopingChannel::mix(int16 *data, uint len) {
	Timestamp newPos = _pos.addFrames(len);
	int frameDiff = newPos.frameDiff(_loopEnd);
	bool needLoop = false;

	assert(frameDiff <= (int)len);

	if (frameDiff >= 0) {
		len -= frameDiff;
		needLoop = true;
	}

	_samplesConsumed = _samplesDecoded;
	_mixerTimeStamp = g_system->getMillis();
	_pauseTime = 0;
	uint samplesRead = _converter->flow(*_input, data, len, getLeftVolume(), getRightVolume());
	_samplesDecoded += samplesRead;
	_pos = _pos.addFrames(samplesRead);

	if (needLoop) {
		if (!_loopCount || _loopCount > 1) {
			if (_loopCount > 1)
				--_loopCount;

			_input->seek(_loopStart);
			samplesRead = _converter->flow(*_input, data + len * 2, frameDiff, getLeftVolume(), getRightVolume());
			_samplesDecoded += samplesRead;
			_pos = _loopStart.addFrames(samplesRead);
		}
	}
}

bool LoopingChannel::isFinished() const {
	return (_loopCount == 1) && (_pos == _loopEnd);
}

} // End of namespace Audio
