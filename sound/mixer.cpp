/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"

#include "common/file.h"
#include "common/util.h"
#include "common/system.h"

#include "sound/mixer.h"
#include "sound/rate.h"
#include "sound/audiostream.h"
#include "sound/flac.h"
#include "sound/mp3.h"
#include "sound/vorbis.h"


namespace Audio {

#pragma mark -
#pragma mark --- Channel classes ---
#pragma mark -


/**
 * Channels used by the sound mixer.
 */
class Channel {
public:
	const Mixer::SoundType	_type;
	SoundHandle _handle;
private:
	Mixer *_mixer;
	bool _autofreeStream;
	bool _permanent;
	byte _volume;
	int8 _balance;
	bool _paused;
	int _id;
	uint32 _samplesConsumed;
	uint32 _samplesDecoded;
	uint32 _mixerTimeStamp;

protected:
	RateConverter *_converter;
	AudioStream *_input;

public:

	Channel(Mixer *mixer, Mixer::SoundType type, int id = -1);
	Channel(Mixer *mixer, Mixer::SoundType type, AudioStream *input, bool autofreeStream, bool reverseStereo = false, int id = -1, bool permanent = false);
	virtual ~Channel();

	void mix(int16 *data, uint len);

	bool isPermanent() const {
		return _permanent;
	}
	bool isFinished() const {
		return _input->endOfStream();
	}
	void pause(bool paused) {
		_paused = paused;
	}
	bool isPaused() {
		return _paused;
	}
	void setVolume(const byte volume) {
		_volume = volume;
	}
	void setBalance(const int8 balance) {
		_balance = balance;
	}
	int getId() const {
		return _id;
	}
	uint32 getElapsedTime();
};


#pragma mark -
#pragma mark --- Mixer ---
#pragma mark -


Mixer::Mixer() {
	_syst = &OSystem::instance();

	_handleSeed = 0;

	_premixChannel = 0;
	int i = 0;

	for (i = 0; i < ARRAYSIZE(_volumeForSoundType); i++)
		_volumeForSoundType[i] = kMaxMixerVolume;

	_paused = false;
	
	for (i = 0; i != NUM_CHANNELS; i++)
		_channels[i] = 0;

	_mixerReady = _syst->setSoundCallback(mixCallback, this);
	_outputRate = (uint)_syst->getOutputSampleRate();

	if (_outputRate == 0)
		error("OSystem returned invalid sample rate");

	debug(1, "Output sample rate: %d Hz", _outputRate);
}

Mixer::~Mixer() {
	_syst->clearSoundCallback();
	stopAll(true);

	delete _premixChannel;
	_premixChannel = 0;
}

bool Mixer::isPaused() {
	return _paused;
}

void Mixer::setupPremix(AudioStream *stream, SoundType type) {
	Common::StackLock lock(_mutex);

	delete _premixChannel;
	_premixChannel = 0;
	
	if (stream == 0)
		return;

	// Create the channel
	_premixChannel = new Channel(this, type, stream, false);
}

void Mixer::insertChannel(SoundHandle *handle, Channel *chan) {

	int index = -1;
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] == 0) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		warning("Mixer::out of mixer slots");
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

void Mixer::playRaw(SoundHandle *handle, void *sound, uint32 size, uint rate, byte flags,
			int id, byte volume, int8 balance, uint32 loopStart, uint32 loopEnd, SoundType type) {
	Common::StackLock lock(_mutex);

	// Prevent duplicate sounds
	if (id != -1) {
		for (int i = 0; i != NUM_CHANNELS; i++)
			if (_channels[i] != 0 && _channels[i]->getId() == id) {
				if ((flags & Mixer::FLAG_AUTOFREE) != 0)
					free(sound);
				return;
			}
	}

	// Create the input stream
	AudioStream *input;
	if (flags & Mixer::FLAG_LOOP) {
		if (loopEnd == 0) {
			input = makeLinearInputStream(rate, flags, (byte *)sound, size, 0, size);
		} else {
			assert(loopStart < loopEnd && loopEnd <= size);
			input = makeLinearInputStream(rate, flags, (byte *)sound, size, loopStart, loopEnd - loopStart);
		}
	} else {
		input = makeLinearInputStream(rate, flags, (byte *)sound, size, 0, 0);
	}

	// Create the channel
	Channel *chan = new Channel(this, type, input, true, (flags & Mixer::FLAG_REVERSE_STEREO) != 0, id);
	chan->setVolume(volume);
	chan->setBalance(balance);
	insertChannel(handle, chan);
}

void Mixer::playInputStream(SoundType type, SoundHandle *handle, AudioStream *input,
			int id, byte volume, int8 balance, bool autofreeStream, bool permanent) {
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
	Channel *chan = new Channel(this, type, input, autofreeStream, false, id, permanent);
	chan->setVolume(volume);
	chan->setBalance(balance);
	insertChannel(handle, chan);
}

void Mixer::mix(int16 *buf, uint len) {
	Common::StackLock lock(_mutex);

	//  zero the buf
	memset(buf, 0, 2 * len * sizeof(int16));

	if (!_paused) {
		if (_premixChannel)
			_premixChannel->mix(buf, len);

		// now mix all channels
		for (int i = 0; i != NUM_CHANNELS; i++)
			if (_channels[i]) {
				if (_channels[i]->isFinished()) {
					delete _channels[i];
					_channels[i] = 0;
				} else if (!_channels[i]->isPaused())
					_channels[i]->mix(buf, len);
			}
	}
}

void Mixer::mixCallback(void *s, byte *samples, int len) {
	assert(s);
	assert(samples);
	// Len is the number of bytes in the buffer; we divide it by
	// four to get the number of samples (stereo 16 bit).
	((Mixer *)s)->mix((int16 *)samples, len >> 2);
}

void Mixer::stopAll(bool force) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i] != 0) {
			if (force || !_channels[i]->isPermanent()) {
				delete _channels[i];
				_channels[i] = 0;
			}
		}
}

void Mixer::stopID(int id) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != 0 && _channels[i]->getId() == id) {
			delete _channels[i];
			_channels[i] = 0;
		}
	}
}

void Mixer::stopHandle(SoundHandle handle) {
	Common::StackLock lock(_mutex);

	// Simply ignore stop requests for handles of sounds that already terminated
	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->_handle._val != handle._val)
		return;

	delete _channels[index];
	_channels[index] = 0;
}

void Mixer::setChannelVolume(SoundHandle handle, byte volume) {
	Common::StackLock lock(_mutex);

	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->_handle._val != handle._val)
		return;

	_channels[index]->setVolume(volume);
}

void Mixer::setChannelBalance(SoundHandle handle, int8 balance) {
	Common::StackLock lock(_mutex);

	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->_handle._val != handle._val)
		return;

	_channels[index]->setBalance(balance);
}

uint32 Mixer::getSoundElapsedTimeOfSoundID(int id) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i] && _channels[i]->getId() == id)
			return _channels[i]->getElapsedTime();
	return 0;
}

uint32 Mixer::getSoundElapsedTime(SoundHandle handle) {
	Common::StackLock lock(_mutex);

	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->_handle._val != handle._val)
		return 0;

	return _channels[index]->getElapsedTime();
}

void Mixer::pauseAll(bool paused) {
	_paused = paused;
}

void Mixer::pauseID(int id, bool paused) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != 0 && _channels[i]->getId() == id) {
			_channels[i]->pause(paused);
			return;
		}
	}
}

void Mixer::pauseHandle(SoundHandle handle, bool paused) {
	Common::StackLock lock(_mutex);

	// Simply ignore pause/unpause requests for handles of sound that alreayd terminated
	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->_handle._val != handle._val)
		return;

	_channels[index]->pause(paused);
}

bool Mixer::isSoundIDActive(int id) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i] && _channels[i]->getId() == id)
			return true;
	return false;
}

int Mixer::getSoundID(SoundHandle handle) {
	Common::StackLock lock(_mutex);
	const int index = handle._val % NUM_CHANNELS;
	if (_channels[index] && _channels[index]->_handle._val == handle._val)
		return _channels[index]->getId();
	return 0;
}

bool Mixer::isSoundHandleActive(SoundHandle handle) {
	Common::StackLock lock(_mutex);
	const int index = handle._val % NUM_CHANNELS;
	return _channels[index] && _channels[index]->_handle._val == handle._val;
}

bool Mixer::hasActiveChannelOfType(SoundType type) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i] && _channels[i]->_type == type)
			return true;
	return false;
}

void Mixer::setVolumeForSoundType(SoundType type, int volume) {
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

int Mixer::getVolumeForSoundType(SoundType type) const {
	assert(0 <= type && type < ARRAYSIZE(_volumeForSoundType));
	
	return _volumeForSoundType[type];
}


#pragma mark -
#pragma mark --- Channel implementations ---
#pragma mark -


Channel::Channel(Mixer *mixer, Mixer::SoundType type, int id)
	: _type(type), _mixer(mixer), _autofreeStream(true),
	  _volume(Mixer::kMaxChannelVolume), _balance(0), _paused(false), _id(id), _samplesConsumed(0),
	  _samplesDecoded(0), _mixerTimeStamp(0), _converter(0), _input(0) {
	assert(mixer);
}

Channel::Channel(Mixer *mixer, Mixer::SoundType type, AudioStream *input,
				bool autofreeStream, bool reverseStereo, int id, bool permanent)
	: _type(type), _mixer(mixer), _autofreeStream(autofreeStream),
	  _volume(Mixer::kMaxChannelVolume), _balance(0), _paused(false), _id(id), _samplesConsumed(0),
	  _samplesDecoded(0), _mixerTimeStamp(0), _converter(0), _input(input), _permanent(permanent) {
	assert(mixer);
	assert(input);

	// Get a rate converter instance
	_converter = makeRateConverter(_input->getRate(), mixer->getOutputRate(), _input->isStereo(), reverseStereo);
}

Channel::~Channel() {
	delete _converter;
	if (_autofreeStream)
		delete _input;
}

/* len indicates the number of sample *pairs*. So a value of
   10 means that the buffer contains twice 10 sample, each
   16 bits, for a total of 40 bytes.
 */
void Channel::mix(int16 *data, uint len) {
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

		_converter->flow(*_input, data, len, vol_l, vol_r);

		_samplesDecoded += len;
	}
}

uint32 Channel::getElapsedTime() {
	if (_mixerTimeStamp == 0)
		return 0;

	// Convert the number of samples into a time duration. To avoid
	// overflow, this has to be done in a somewhat non-obvious way.

	uint rate = _mixer->getOutputRate();

	uint32 seconds = _samplesConsumed / rate;
	uint32 milliseconds = (1000 * (_samplesConsumed % rate)) / rate;

	uint32 delta = g_system->getMillis() - _mixerTimeStamp;

	// In theory it would seem like a good idea to limit the approximation
	// so that it never exceeds the theoretical upper bound set by
	// _samplesDecoded. Meanwhile, back in the real world, doing so makes
	// the Broken Sword cutscenes noticeably jerkier. I guess the mixer
	// isn't invoked at the regular intervals that I first imagined.

	// FIXME: This won't work very well if the sound is paused.
	return 1000 * seconds + milliseconds + delta;
}


} // End of namespace Audio
